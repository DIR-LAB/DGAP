/*
 * bc_adj.h
 * LLAMA Graph Analytics
 *
 * Copyright 2014
 *      The President and Fellows of Harvard College.
 *
 * Copyright 2014
 *      Oracle Labs.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#ifndef LL_GENERATED_CPP_BC_BRANDES_H
#define LL_GENERATED_CPP_BC_BRANDES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>
#include <omp.h>

#include "llama/ll_writable_graph.h"
#include "benchmarks/benchmark.h"
#include "gapbs/pvector.h"
#include "gapbs/bitmap.h"
#include "gapbs/sliding_queue.h"
#include "gapbs/timer.h"

typedef float ScoreT;
typedef int64_t SGOffset;

/**
 * Betweenness Centrality - Brandes
 */
template<class Graph>
class ll_b_gap_bc : public ll_benchmark<Graph> {
  node_t root;
  int32_t num_iters;
  float *G_BC;
  pvector<ScoreT> scores;


public:

  /**
   * Create the benchmark
   */
  ll_b_gap_bc(node_t r, int32_t num_iters_) : ll_benchmark<Graph>("Betweenness Centrality - Brandes") {
    this->create_auto_array_for_nodes(G_BC);
    root = r;
    num_iters = num_iters_;
  }


  /**
   * Destroy the benchmark
   */
  virtual ~ll_b_gap_bc(void) {
  }


  /**
   * Run the benchmark
   *
   * @return the numerical result, if applicable
   */
  virtual double run(void) {

    Graph &g = *this->_graph;

//    Timer t;
//    t.Start();

    pvector<node_t> degrees(g.max_nodes());
    ParallelLoadDegrees(degrees);
    pvector<SGOffset> prefix(degrees.size() + 1);
    ParallelPrefixSum(degrees, prefix);

    scores.resize(g.max_nodes(), 0);
    pvector<node_t> path_counts(g.max_nodes());
//    Bitmap succ(g.num_edges_directed());
//    Bitmap succ(g.max_edges(g.num_levels()));
    Bitmap succ(prefix[degrees.size()]);
    std::vector<SlidingQueue<node_t>::iterator> depth_index;
    SlidingQueue<node_t> queue(g.max_nodes());

//    for(int i=0; i<g.num_levels(); i+=1) {
//      std::cout << "level " << i << "# " << g.max_edges(i) << std::endl;
//    }
//    std::cout << "num-edges: " << g.max_edges(g.num_levels()-1) << ", prefix-sum: " << prefix[degrees.size()] << std::endl;
//    assert(g.max_edges(g.num_levels()) == prefix[degrees.size()] && "prefix sum does not matches with the graph-api!");
//    std::cout << "num-iters: " << num_iters << std::endl;

//    t.Stop();
//  PrintStep("a", t.Seconds());
//  const node_t* g_out_start = g.out_neigh(0).begin().ptr;
//  cout << "first element: " << *g_out_start << endl;
    for (node_t iter=0; iter < num_iters; iter++) {
      node_t source = root;
//      std::cout << "source: " << source << std::endl;
//      t.Start();
      path_counts.fill(0);
      depth_index.resize(0);
      queue.reset();
      succ.reset();
      PBFS(source, path_counts, succ, depth_index, queue, prefix);
//      t.Stop();
//    PrintStep("b", t.Seconds());
      pvector<ScoreT> deltas(g.max_nodes(), 0);
//      t.Start();
      for (int d=depth_index.size()-2; d >= 0; d--) {
#pragma omp parallel for schedule(dynamic, 64)
        for (auto it = depth_index[d]; it < depth_index[d+1]; it++) {
          node_t u = *it;
          ScoreT delta_u = 0;
          node_t local_edge_id = 0;
//          node_t v;
//        for (node_t &v : g.out_neigh(u)) {
//          for (node_t v : g.out_neigh(u)) {
          ll_foreach_out(v, g, u) {
//          if (succ.get_bit(&v - g_out_start)) {
            if (succ.get_bit(GetEdgeId(prefix, u, local_edge_id))) {
              delta_u += static_cast<ScoreT>(path_counts[u]) /
                         static_cast<ScoreT>(path_counts[v]) * (1 + deltas[v]);
            }
            local_edge_id += 1;
          }
          deltas[u] = delta_u;
          scores[u] += delta_u;
        }
      }
//      t.Stop();
//    PrintStep("p", t.Seconds());
    }
    // normalize scores
    ScoreT biggest_score = 0;
#pragma omp parallel for reduction(max : biggest_score)
    for (node_t n=0; n < g.max_nodes(); n++)
      biggest_score = std::max(biggest_score, scores[n]);
#pragma omp parallel for
    for (node_t n=0; n < g.max_nodes(); n++)
      scores[n] = scores[n] / biggest_score;
//    return scores;
    return biggest_score;
  }


  /**
   * Finalize the benchmark
   *
   * @return the updated numerical result, if applicable
   */
  virtual double finalize(void) {
    float max = 0;

//    for (node_t n = 0; n < this->_graph->max_nodes(); n++) {
//      if (G_BC[n] > max) max = G_BC[n];
//    }

    return max;
  }


  /**
   * Print the results
   *
   * @param f the output file
   */
  virtual void print_results(FILE *f) {
    PrintTopScores(scores);
//    print_results_part(f, this->_graph, G_BC);
  }

private:
  template<typename KeyT, typename ValT>
  std::vector<std::pair<ValT, KeyT>> TopK(
      const std::vector<std::pair<KeyT, ValT>> &to_sort, size_t k) {
    std::vector<std::pair<ValT, KeyT>> top_k;
    ValT min_so_far = 0;
    for (auto kvp : to_sort) {
      if ((top_k.size() < k) || (kvp.second > min_so_far)) {
        top_k.push_back(std::make_pair(kvp.second, kvp.first));
        std::sort(top_k.begin(), top_k.end(),
                  std::greater<std::pair<ValT, KeyT>>());
        if (top_k.size() > k)
          top_k.resize(k);
        min_so_far = top_k.back().first;
      }
    }
    return top_k;
  }

  void PrintTopScores(const pvector<ScoreT> &scores) {
    Graph &g = *this->_graph;
    std::vector<std::pair<node_t, ScoreT>> score_pairs(g.max_nodes());
    for (node_t n = 0; n < g.max_nodes(); n++)
      score_pairs[n] = std::make_pair(n, scores[n]);
    int k = 5;
    std::vector<std::pair<ScoreT, node_t>> top_k = TopK(score_pairs, k);
    for (auto kvp : top_k)
      std::cout << kvp.second << ":" << kvp.first << std::endl;
  }

  void PrintStep(const std::string &s, double seconds, int64_t count = -1) {
    if (count != -1)
      printf("%5s%11" PRId64 "  %10.5lf\n", s.c_str(), count, seconds);
    else
    printf("%5s%23.5lf\n", s.c_str(), seconds);
  }

  static void ParallelPrefixSum(const pvector<node_t> &degrees, pvector<SGOffset> &prefix) {
    const size_t block_size = 1 << 20;
    const size_t num_blocks = (degrees.size() + block_size - 1) / block_size;
    pvector<SGOffset> local_sums(num_blocks);
#pragma omp parallel for
    for (size_t block = 0; block < num_blocks; block++) {
      SGOffset lsum = 0;
      size_t block_end = std::min((block + 1) * block_size, degrees.size());
      for (size_t i = block * block_size; i < block_end; i++) lsum += degrees[i];
      local_sums[block] = lsum;
    }
    pvector<SGOffset> bulk_prefix(num_blocks + 1);
    SGOffset total = 0;
    for (size_t block = 0; block < num_blocks; block++) {
      bulk_prefix[block] = total;
      total += local_sums[block];
    }
    bulk_prefix[num_blocks] = total;
#pragma omp parallel for
    for (size_t block = 0; block < num_blocks; block++) {
      SGOffset local_total = bulk_prefix[block];
      size_t block_end = std::min((block + 1) * block_size, degrees.size());
      for (size_t i = block * block_size; i < block_end; i++) {
        prefix[i] = local_total;
        local_total += degrees[i];
      }
    }
    prefix[degrees.size()] = bulk_prefix[num_blocks];
//  return prefix;
  }

  void ParallelLoadDegrees(pvector<node_t> &degrees) {
    Graph &g = *this->_graph;
#pragma omp parallel for schedule(dynamic, 16384)
    for (node_t u = 0; u < g.max_nodes(); u++) {
      degrees[u] = g.out_degree(u);
    }
  }

  inline int64_t GetEdgeId(const pvector<SGOffset> &prefix, node_t u, node_t local_edge_id) {
//  if(u == 0) return local_edge_id;
    return prefix[u] + local_edge_id;
  }

  void PBFS(node_t source, pvector<node_t> &path_counts,
            Bitmap &succ, std::vector<SlidingQueue<node_t>::iterator> &depth_index,
            SlidingQueue<node_t> &queue, const pvector<SGOffset> &prefix) {

//    std::cout << "entered PBFS" << std::endl;
    Graph &g = *this->_graph;

//    std::cout << "graph instance taken ..." << std::endl;
//    std::cout << "number of nodes: " << g.max_nodes() << std::endl;

    pvector<node_t> depths(g.max_nodes(), -1);
    depths[source] = 0;
    path_counts[source] = 1;
    queue.push_back(source);
    depth_index.push_back(queue.begin());
    queue.slide_window();
//  const node_t* g_out_start = g.out_neigh(0).begin().ptr;
#pragma omp parallel
    {
      node_t depth = 0;
      QueueBuffer<node_t> lqueue(queue);
      while (!queue.empty()) {
#pragma omp single
        depth_index.push_back(queue.begin());
        depth++;
#pragma omp for schedule(dynamic, 64)
        for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
          node_t u = *q_iter;
//          std::cout << "u: " << u << std::endl;
//          if(u == 369684) std::cout << "degree: " << g.out_degree(u) << std::endl;
          node_t local_edge_id = 0;
//          node_t v;
//        for (node_t &v : g.out_neigh(u)) {
//          for (node_t v : g.out_neigh(u)) {
          ll_foreach_out(v, g, u) {
//            if(u == 369684) std::cout << "v: " << v << ", edge-id: " << GetEdgeId(prefix, u, local_edge_id) << std::endl;
            if ((depths[v] == -1) &&
                (compare_and_swap(depths[v], static_cast<node_t>(-1), depth))) {
              lqueue.push_back(v);
            }
            if (depths[v] == depth) {
//            succ.set_bit_atomic(&v - g_out_start);
              succ.set_bit_atomic(GetEdgeId(prefix, u, local_edge_id));
              fetch_and_add(path_counts[v], path_counts[u]);
            }
            local_edge_id += 1;
          }
//          std::cout << "u: " << u << std::endl;
        }
        lqueue.flush();
#pragma omp barrier
#pragma omp single
        queue.slide_window();
      }
    }
    depth_index.push_back(queue.begin());
//    std::cout << "exiting PBFS" << std::endl;
  }
};

#endif

