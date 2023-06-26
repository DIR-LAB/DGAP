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


#ifndef LL_GENERATED_CPP_CC_SV_H
#define LL_GENERATED_CPP_CC_SV_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <unordered_map>
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
class ll_b_gap_cc : public ll_benchmark<Graph> {
  pvector<node_t> comp;

public:

  /**
   * Create the benchmark
   */
  ll_b_gap_cc() : ll_benchmark<Graph>("Connected Components (CC) - Shiloach-Vishkin") {
  }


  /**
   * Destroy the benchmark
   */
  virtual ~ll_b_gap_cc(void) {
  }


  /**
   * Run the benchmark
   *
   * @return the numerical result, if applicable
   */
  virtual double run(void) {
    Graph &g = *this->_graph;
    comp.resize(g.max_nodes());
#pragma omp parallel for
    for (node_t n=0; n < g.max_nodes(); n++)
      comp[n] = n;
    bool change = true;
    int num_iter = 0;
    while (change) {
      change = false;
      num_iter++;
#pragma omp parallel for
      for (node_t u=0; u < g.max_nodes(); u++) {
//        for (node_t v : g.out_neigh(u)) {
        ll_foreach_out(v, g, u) {
          node_t comp_u = comp[u];
          node_t comp_v = comp[v];
          if (comp_u == comp_v) continue;
          // Hooking condition so lower component ID wins independent of direction
          node_t high_comp = comp_u > comp_v ? comp_u : comp_v;
          node_t low_comp = comp_u + (comp_v - high_comp);
          if (high_comp == comp[high_comp]) {
            change = true;
            comp[high_comp] = low_comp;
          }
        }
      }
#pragma omp parallel for
      for (node_t n=0; n < g.max_nodes(); n++) {
        while (comp[n] != comp[comp[n]]) {
          comp[n] = comp[comp[n]];
        }
      }
    }
    std::cout << "Shiloach-Vishkin took " << num_iter << " iterations" << std::endl;
//    return comp;
    return 0;
  }


  /**
   * Finalize the benchmark
   *
   * @return the updated numerical result, if applicable
   */
  virtual double finalize(void) {
    float max = 0;
//    PrintCompStats(comp);
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
    PrintCompStats(comp);
//    print_results_part(f, this->_graph, G_BC);
  }

private:
  static std::vector<std::pair<node_t, node_t>> TopK(const std::vector<std::pair<node_t, node_t>> &to_sort, size_t k) {
    std::vector<std::pair<node_t, node_t>> top_k;
    node_t min_so_far = 0;
    for (auto kvp : to_sort) {
      if ((top_k.size() < k) || (kvp.second > min_so_far)) {
        top_k.push_back(std::make_pair(kvp.second, kvp.first));
        std::sort(top_k.begin(), top_k.end(), std::greater<std::pair<node_t, node_t>>());
        if (top_k.size() > k)
          top_k.resize(k);
        min_so_far = top_k.back().first;
      }
    }
    return top_k;
  }

  void PrintCompStats(const pvector<node_t> &comp) {
    std::cout << std::endl;
    std::unordered_map<node_t, node_t> count;
    for (node_t comp_i : comp)
      count[comp_i] += 1;
    int k = 5;
    std::vector<std::pair<node_t, node_t>> count_vector;
    count_vector.reserve(count.size());
    for (auto kvp : count)
      count_vector.push_back(kvp);
    std::vector<std::pair<node_t, node_t>> top_k = TopK(count_vector, k);
    k = std::min(k, static_cast<int>(top_k.size()));
    std::cout << k << " biggest clusters" << std::endl;
    for (auto kvp : top_k)
      std::cout << kvp.second << ":" << kvp.first << std::endl;
    std::cout << "There are " << count.size() << " components" << std::endl;
  }


// Verifies CC result by performing a BFS from a vertex in each component
// - Asserts search does not reach a vertex with a different component label
// - If the graph is directed, it performs the search as if it was undirected
// - Asserts every vertex is visited (degree-0 vertex should have own label)
  bool CCVerifier(const pvector<node_t> &comp) {
    Graph &g = *this->_graph;

    std::unordered_map<node_t, node_t> label_to_source;
    for (node_t n : g.max_nodes())
      label_to_source[comp[n]] = n;
    Bitmap visited(g.max_nodes());
    visited.reset();
    std::vector<node_t> frontier;
    frontier.reserve(g.max_nodes());
    for (auto label_source_pair : label_to_source) {
      node_t curr_label = label_source_pair.first;
      node_t source = label_source_pair.second;
      frontier.clear();
      frontier.push_back(source);
      visited.set_bit(source);
      for (auto it = frontier.begin(); it != frontier.end(); it++) {
        node_t u = *it;
//        for (node_t v : g.out_neigh(u)) {
        ll_foreach_out(v, g, u) {
          if (comp[v] != curr_label)
            return false;
          if (!visited.get_bit(v)) {
            visited.set_bit(v);
            frontier.push_back(v);
          }
        }
//        if (g.directed()) {
//          for (node_t v : g.in_neigh(u)) {
//            if (comp[v] != curr_label)
//              return false;
//            if (!visited.get_bit(v)) {
//              visited.set_bit(v);
//              frontier.push_back(v);
//            }
//          }
//        }
      }
    }
    for (node_t n=0; n < g.max_nodes(); n++)
      if (!visited.get_bit(n))
        return false;
    return true;
  }
};

#endif

