/*
 * pagerank.h
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


#ifndef LL_PAGERANK_H
#define LL_PAGERANK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <iostream>

#include "llama/ll_writable_graph.h"
#include "benchmarks/benchmark.h"
#include "gapbs/pvector.h"


/**
 * The GAP PageRank benchmark
 */
template <class Graph, typename value_t>
class gap_b_pagerank_pull_ext : public ll_benchmark<Graph> {

	value_t d;
	int32_t max;

  pvector<value_t> scores;

//	value_t* G_pg_rank;

public:
	/**
	 * Create the benchmark
	 */
	gap_b_pagerank_pull_ext(int32_t max, value_t d=0.85)
		: ll_benchmark<Graph>(
				/* assuming that value_t is either a float or a double */
				sizeof(value_t) == sizeof(float)
					? "GapPageRank<float> - Pull"
					: "GapPageRank<double> - Pull") {

		assert(d > 0 && d < 1);

		this->d = d;
		this->max = max;

		// this->create_auto_array_for_nodes(G_pg_rank);
	}

	/**
	 * Destroy the benchmark
	 */
	virtual ~gap_b_pagerank_pull_ext(void) {
	}

	/**
	 * Run the benchmark
	 *
	 * @return the numerical result, if applicable
	 */
	virtual double run(void) {
		Graph& G = *this->_graph;
		value_t epsilon = 1e-4;
		// printf("gapbs max_iters: %d, gapbs epsilon: %lf\n", max, epsilon);
		// cout << "gapbs max_iters: " << max << endl;
		// cout << "gapbs epsilon: " << epsilon << endl;
		const value_t init_score = 1.0f / G.max_nodes();
		const value_t base_score = (1.0f - d) / G.max_nodes();
//    std::cout << "init-score: " << init_score << ", base-score: " << base_score << std::endl;
		scores.resize(G.max_nodes(), init_score);
		pvector<value_t> outgoing_contrib(G.max_nodes());
		for (int iter=0; iter < max; iter++) {
      double _t_start = ll_get_time_ms();
			double error = 0;
			#pragma omp parallel for
			for (node_t n=0; n < G.max_nodes(); n++)
				outgoing_contrib[n] = scores[n] / G.out_degree(n);
			
			#pragma omp parallel for reduction(+ : error) schedule(dynamic, 64)
			for (node_t u=0; u < G.max_nodes(); u++) {
				value_t incoming_total = 0;
				// for (node_t v : g.in_neigh(u))
//				if(iter == 0 && u == 1) printf("%ld:", u);
//        std::cout << "Source vertex# " << u << "(" << G.in_degree(u) << "): ";
				ll_foreach_in(v, G, u) {
					incoming_total += outgoing_contrib[v];
//          std::cout << v << " ";
//					if(iter == 0 && u == 1) printf(" %ld", v);
				}
				//if(iter == 0 && u == 1) printf("\n");
//        std::cout << std::endl;
//        if(u >= 10) exit(0);
				value_t old_score = scores[u];
				scores[u] = base_score + d * incoming_total;
				error += fabs(scores[u] - old_score);
			}
      double _t_taken = ll_get_time_ms() - _t_start;
//			printf(" %2d    %lf    %lf ms\n", iter, error, _t_taken);
//			if (error < epsilon)
//				break;
		}
		return 0;
	}

	/**
	 * Finalize the benchmark
	 *
	 * @return the updated numerical result, if applicable
	 */
	virtual double finalize(void) {
		// todo: update later ...
		return 0.0;
	}


	/**
	 * Print the results
	 * 
	 * @param f the output file
	 */
	virtual void print_results(FILE* f) {
    PrintTopScores(scores);
		// todo: update later ...
	}

private:
  // Returns k pairs with largest values from list of key-value pairs
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
    Graph& g = *this->_graph;
    std::vector<std::pair<node_t, ScoreT>> score_pairs(g.max_nodes());
    for (node_t n=0; n < g.max_nodes(); n++) {
      score_pairs[n] = std::make_pair(n, scores[n]);
    }
    int k = 5;
    std::vector<std::pair<ScoreT, node_t>> top_k = TopK(score_pairs, k);
    k = std::min(k, static_cast<int>(top_k.size()));
    for (auto kvp : top_k)
      std::cout << kvp.second << ":" << kvp.first << std::endl;
  }
};

/**
 * The PageRank benchmark
 */
template <class Graph, typename value_t>
class ll_b_pagerank_pull_ext : public ll_benchmark<Graph> {

	value_t d;
	int32_t max;

	value_t* G_pg_rank;


public:

	/**
	 * Create the benchmark
	 */
	ll_b_pagerank_pull_ext(int32_t max, value_t d=0.85)
		: ll_benchmark<Graph>(
				/* assuming that value_t is either a float or a double */
				sizeof(value_t) == sizeof(float)
					? "PageRank<float> - Pull"
					: "PageRank<double> - Pull") {

		assert(d > 0 && d < 1);

		this->d = d;
		this->max = max;

		this->create_auto_array_for_nodes(G_pg_rank);
	}


	/**
	 * Destroy the benchmark
	 */
	virtual ~ll_b_pagerank_pull_ext(void) {
	}


	/**
	 * Run the benchmark
	 *
	 * @return the numerical result, if applicable
	 */
	virtual double run(void) {

		Graph& G = *this->_graph;
		ll_memory_helper m;

		value_t diff = 0.0 ;
		value_t tolerance = 1e-4 ;
		int32_t cnt = 0 ;
		value_t N = 0.0 ;
		value_t* G_pg_rank_nxt = m.allocate<value_t>(G.max_nodes());

		cnt = 0 ;
		N = (value_t)(G.max_nodes()) ;

#pragma omp parallel for
		for (node_t t0 = 0; t0 < G.max_nodes(); t0 ++) 
			G.set_node_prop(G_pg_rank, t0, 1 / N);

		this->progress_init(max);

		do
		{
			diff = 0.000000 ;
#pragma omp parallel
			{
				value_t diff_prv = 0.0 ;

				diff_prv = 0.000000 ;

#pragma omp for nowait schedule(dynamic,4096)
				for (node_t t = 0; t < G.max_nodes(); t ++) 
				{
					value_t val = 0.0 ;
					value_t __S1 = 0.0 ;

					__S1 = 0.000000 ;

					/*ll_edge_iterator iter = G.in_iter_begin(t);
					  for (edge_t w_idx = G.in_iter_next(iter);
					  w_idx != LL_NIL_EDGE;
					  w_idx = G.in_iter_next(iter)) {
					  node_t w = LL_ITER_IN_NEXT_EDGE(G, iter, w_idx);

					  __S1 = __S1 + G_pg_rank[w] / ((value_t)((G.out_degree(w)))) ;
					  }*/

#ifdef TEST_CXX_ITER
//          std::cout << "Source vertex: " << t << std::endl;
					typename Graph::iterator iter;
					typename Graph::iterator end = G.end();
					for (iter = G.in_begin(t); iter != end; ++iter) {
						__S1 = __S1 + G_pg_rank[*iter] / ((value_t)((G.out_degree(*iter)))) ;
//            std::cout << *iter << std::endl;
					}
//          exit(0);
#else
					/*ll_edge_iterator iter;
					G.inm_iter_begin(iter, t);
					for (node_t w = G.inm_iter_next(iter);
							w != LL_NIL_NODE;
							w = G.inm_iter_next(iter)) {

						__S1 = __S1 + G_pg_rank[w] / ((value_t)((G.out_degree(w)))) ;
					}*/

//          std::cout << "here it come" << std::endl;
//          std::cout << "Source vertex# " << t << "(" << G.out_degree(t) << "): ";
					ll_foreach_in(w, G, t) {
						__S1 = __S1 + G_pg_rank[w] / ((value_t)((G.out_degree(w)))) ;
//            std::cout << w << " ";
					}
//          ll_edge_iterator iter;
//          G.out_iter_begin(iter, t);
//          for (edge_t s_idx = G.out_iter_next(iter);
//               s_idx != LL_NIL_EDGE;
//               s_idx = G.out_iter_next(iter)) {
//            node_t s = LL_ITER_OUT_NEXT_NODE(G, iter, s_idx);
//            std::cout << s << " ";
//          }
//          std::cout << std::endl;
//          if(t >= 10) exit(0);
#endif

					val = (1 - d) / N + d * __S1 ;
					diff_prv = diff_prv +  std::abs((val - G_pg_rank[t]))  ;
					G.set_node_prop(G_pg_rank_nxt, t, val);
				}
				ATOMIC_ADD<value_t>(&diff, diff_prv);
			}

#pragma omp parallel for
			for (node_t i3 = 0; i3 < G.max_nodes(); i3 ++) 
				G.set_node_prop(G_pg_rank, i3, G_pg_rank_nxt[i3]);
			
//			printf(" %2d    %lf\n", cnt, diff);
//			if(diff < tolerance) break;

			cnt = cnt + 1 ;
			this->progress_update(cnt);
		}
		while (cnt < max);
		this->progress_clear();

		return 0;
	}


	/**
	 * Finalize the benchmark
	 *
	 * @return the updated numerical result, if applicable
	 */
	virtual double finalize(void) {
		value_t s  = 0;
		value_t s2 = 0;
		size_t c  = 0;
		for (node_t n = 0; n < this->_graph->max_nodes(); n++) {
			s  += G_pg_rank[n];
			s2 += G_pg_rank[n] * G_pg_rank[n];
			c++;
		}
		return s;
	}


	/**
	 * Print the results
	 * 
	 * @param f the output file
	 */
	virtual void print_results(FILE* f) {
		print_results_part(f, this->_graph, G_pg_rank);
	}
};


/**
 * The PageRank benchmark - PUSH variant
 */
template <class Graph, typename value_t>
class ll_b_pagerank_push_ext : public ll_benchmark<Graph> {

	value_t d;
	int32_t max;

	value_t* G_pg_rank;


public:

	/**
	 * Create the benchmark
	 */
	ll_b_pagerank_push_ext(int32_t max, value_t d=0.85)
		: ll_benchmark<Graph>(
				/* assuming that value_t is either a float or a double */
				sizeof(value_t) == sizeof(float)
					? "PageRank<float> - Push"
					: "PageRank<double> - Push") {

		assert(d > 0 && d < 1);

		this->d = d;
		this->max = max;

		this->create_auto_array_for_nodes(G_pg_rank);
	}


	/**
	 * Destroy the benchmark
	 */
	virtual ~ll_b_pagerank_push_ext(void) {
	}


	/**
	 * Run the benchmark
	 *
	 * @return the numerical result, if applicable
	 */
	virtual double run(void) {

		Graph& G = *this->_graph;
		ll_memory_helper m;

		int32_t cnt = 0 ;
		value_t N = 0.0 ;
		value_t* G_pg_rank_nxt = m.allocate<value_t>(G.max_nodes());

		cnt = 0 ;
		N = (value_t)(G.max_nodes()) ;

#pragma omp parallel for
		for (node_t t0 = 0; t0 < G.max_nodes(); t0 ++)  {
			G.set_node_prop(G_pg_rank, t0, 1 / N);
			G.set_node_prop(G_pg_rank_nxt, t0, (value_t) 0.0);
		}

		this->progress_init(max);

		do
		{
#pragma omp parallel
			{

#pragma omp for schedule(dynamic,4096)
				for (node_t t = 0; t < G.max_nodes(); t ++) 
				{
					int t_degree = G.out_degree(t);
					if (t_degree == 0) continue;

					value_t t_pg_rank = G_pg_rank[t];
					value_t t_delta = t_pg_rank / t_degree;

#ifdef TEST_CXX_ITER
					typename Graph::iterator iter;
					typename Graph::iterator end = G.end();
					for (iter = G.out_begin(t); iter != end; ++iter) {
						ATOMIC_ADD<value_t>(&G_pg_rank_nxt[*iter], t_delta);
					}
#else
					/*ll_edge_iterator iter;
					G.out_iter_begin(iter, t);
					for (edge_t w_idx = G.out_iter_next(iter);
							w_idx != LL_NIL_EDGE;
							w_idx = G.out_iter_next(iter)) {
						node_t w = LL_ITER_OUT_NEXT_NODE(G, iter, w_idx);
						ATOMIC_ADD<value_t>(&G_pg_rank_nxt[w], t_delta);
					}*/

					ll_foreach_out(w, G, t) {
						ATOMIC_ADD<value_t>(&G_pg_rank_nxt[w], t_delta);
					}
#endif

				}

#pragma omp for schedule(dynamic,4096)
				for (node_t t = 0; t < G.max_nodes(); t ++) 
				{
					G.set_node_prop(G_pg_rank, t, (1 - d) / N + d * G_pg_rank_nxt[t]);
					G.set_node_prop(G_pg_rank_nxt, t, (value_t) 0.0);
				}
			}

			cnt = cnt + 1 ;
			this->progress_update(cnt);
		}
		while (cnt < max);
		this->progress_clear();

		return 0;
	}


	/**
	 * Finalize the benchmark
	 *
	 * @return the updated numerical result, if applicable
	 */
	virtual double finalize(void) {
		value_t s  = 0;
		value_t s2 = 0;
		size_t c  = 0;
		for (node_t n = 0; n < this->_graph->max_nodes(); n++) {
			s  += G_pg_rank[n];
			s2 += G_pg_rank[n] * G_pg_rank[n];
			c++;
		}
		return s;
	}


	/**
	 * Print the results
	 * 
	 * @param f the output file
	 */
	virtual void print_results(FILE* f) {
		print_results_part(f, this->_graph, G_pg_rank);
	}
};


/**
 * Adapters
 */


/**
 * The PageRank benchmark - Pull variant, float
 */
template <class Graph>
class ll_b_pagerank_pull_float
	: public ll_b_pagerank_pull_ext<Graph, float> {

public:

	/**
	 * Create the benchmark
	 */
	ll_b_pagerank_pull_float(int32_t max, float d=0.85)
		: ll_b_pagerank_pull_ext<Graph, float>(max, d) {}
};

/**
 * The PageRank benchmark - Pull variant, float
 */
template <class Graph>
class ll_b_gap_pagerank_pull_float
	: public gap_b_pagerank_pull_ext<Graph, float> {

public:

	/**
	 * Create the benchmark
	 */
	ll_b_gap_pagerank_pull_float(int32_t max, float d=0.85)
		: gap_b_pagerank_pull_ext<Graph, float>(max, d) {}
};

/**
 * The PageRank benchmark - Push variant, float
 */
template <class Graph>
class ll_b_pagerank_push_float
	: public ll_b_pagerank_push_ext<Graph, float> {

public:

	/**
	 * Create the benchmark
	 */
	ll_b_pagerank_push_float(int32_t max, float d=0.85)
		: ll_b_pagerank_push_ext<Graph, float>(max, d) {}
};


/**
 * The PageRank benchmark - Pull variant, double
 */
template <class Graph>
class ll_b_pagerank_pull_double
	: public ll_b_pagerank_pull_ext<Graph, double> {

public:

	/**
	 * Create the benchmark
	 */
	ll_b_pagerank_pull_double(int32_t max, double d=0.85)
		: ll_b_pagerank_pull_ext<Graph, double>(max, d) {}
};


/**
 * The PageRank benchmark - Push variant, double
 */
template <class Graph>
class ll_b_pagerank_push_double
	: public ll_b_pagerank_push_ext<Graph, double> {

public:

	/**
	 * Create the benchmark
	 */
	ll_b_pagerank_push_double(int32_t max, double d=0.85)
		: ll_b_pagerank_push_ext<Graph, double>(max, d) {}
};

#endif

