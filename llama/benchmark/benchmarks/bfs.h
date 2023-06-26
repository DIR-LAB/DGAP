/*
 * bfs.h
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


#ifndef LL_BFS_H
#define LL_BFS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <cmath>
#include <algorithm>
#include <omp.h>

#include "llama/ll_bfs_template.h"
#include "llama/ll_writable_graph.h"
#include "benchmarks/benchmark.h"

#include "gapbs/bitmap.h"
#include "gapbs/platform_atomics.h"
#include "gapbs/pvector.h"
#include "gapbs/sliding_queue.h"
#include "gapbs/timer.h"

// BFS/DFS definitions for the procedure
template <class Graph>
class bfs_bfs : public ll_bfs_template
    <Graph, short, true, false, false, false>
{
public:
    bfs_bfs(Graph& _G, node_t& _root, int32_t& _count)
    : ll_bfs_template<Graph, short, true, false, false, false>(_G),
    G(_G), root(_root), count(_count){}

private:  // list of varaibles
    Graph& G;
    node_t& root;
    int32_t& count;

protected:
    virtual void visit_fw(node_t v) 
    {
        ATOMIC_ADD<int32_t>(&count, 1);
    }

    virtual void visit_rv(node_t v) {}
    virtual bool check_navigator(node_t v, edge_t v_idx) {return true;}


};


/**
 * Count vertices in the given BFS traversal
 */
template <class Graph>
class ll_b_bfs : public ll_benchmark<Graph> {

	node_t root;


public:

	/**
	 * Create the benchmark
	 *
	 * @param r the root
	 */
	ll_b_bfs(node_t r)
		: ll_benchmark<Graph>("BFS - Count") {

		root = r;
	}


	/**
	 * Destroy the benchmark
	 */
	virtual ~ll_b_bfs(void) {
	}


	/**
	 * Run the benchmark
	 *
	 * @return the numerical result, if applicable
	 */
	virtual double run(void) {

		Graph& G = *this->_graph;
		int32_t count = 0;

		bfs_bfs<Graph> _BFS(G, root, count);
		_BFS.prepare(root);
		_BFS.do_bfs_forward();

		return count; 
	}
};

/**
 * The GAP BFS benchmark
 */
template <class Graph>
class gap_b_bfs_ext : public ll_benchmark<Graph> {

	node_t root;
	int32_t count;
  pvector<node_t> parent;

public:
	/**
	 * Create the benchmark
	 *
	 * @param r the root
	 */
	gap_b_bfs_ext(node_t r)
		: ll_benchmark<Graph>("BFS - Direction Optimized") {
		//printf("BFS - Direction Optimized\n");
		this->root = r;
	}

	/**
	 * Destroy the benchmark
	 */
	virtual ~gap_b_bfs_ext(void) {
	}

	/**
	 * Run the benchmark
	 *
	 * @return the numerical result, if applicable
	 */
	virtual double run(void) {
		Graph& G = *this->_graph;
		//printf("going to run bfs with root: %d\n", root);
		int alpha = 15;
    int beta = 18;

		//PrintStep("Source", static_cast<int64_t>(root));
		//printf("max_nodes: %d\n", G.max_nodes());
		//printf("here 1\n");
		Timer t;
		t.Start();
		//printf("here 2\n");
//		pvector<node_t> parent = InitParent(G);
		InitParent(G);
		t.Stop();
		//printf("so far here ...\n");
		//PrintStep("i", t.Seconds());
		parent[root] = root;
		SlidingQueue<node_t> queue(G.max_nodes());
		queue.push_back(root);
		queue.slide_window();
		Bitmap curr(G.max_nodes());
		curr.reset();
		Bitmap front(G.max_nodes());
		front.reset();

		// todo: need to check whether there are other ways to get the number of edges in the graph
		int64_t edges_to_check = 0;
		for (size_t i = 0; i < G.num_levels(); i++) edges_to_check += G.max_edges(i);

		int64_t scout_count = G.out_degree(root);
		while (!queue.empty()) {
			if (scout_count > edges_to_check / alpha) {
				int64_t awake_count, old_awake_count;
				TIME_OP(t, QueueToBitmap(queue, front));
				//PrintStep("e", t.Seconds());
				awake_count = queue.size();
				queue.slide_window();
				do {
					t.Start();
					old_awake_count = awake_count;
					awake_count = BUStep(G, parent, front, curr);
					front.swap(curr);
					t.Stop();
					//PrintStep("bu", t.Seconds(), awake_count);
				} while ((awake_count >= old_awake_count) ||
						(awake_count > G.max_nodes() / beta));
				TIME_OP(t, BitmapToQueue(G, front, queue));
				//PrintStep("c", t.Seconds());
				scout_count = 1;
			} else {
				t.Start();
				edges_to_check -= scout_count;
				scout_count = TDStep(G, parent, queue);
				queue.slide_window();
				t.Stop();
				//PrintStep("td", t.Seconds(), queue.size());
			}
		}
		#pragma omp parallel for
		for (node_t n = 0; n < G.max_nodes(); n++) {
			if (parent[n] < -1) {
				parent[n] = -1;
			}
			else ATOMIC_ADD<int32_t>(&count, 1);
		}

		return count;
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
    PrintBFSStats(parent);
		// todo: update later ...
	}

private:
	int64_t BUStep(Graph& G, pvector<node_t> &parent, Bitmap &front, Bitmap &next) {
		int64_t awake_count = 0;
		next.reset();
		#pragma omp parallel for reduction(+ : awake_count) schedule(dynamic, 1024)
		for (node_t u=0; u < G.max_nodes(); u++) {
			if (parent[u] < 0) {
				//for (node_t v : g.in_neigh(u)) {
				ll_foreach_in(v, G, u) {
					if (front.get_bit(v)) {
						parent[u] = v;
						awake_count++;
						next.set_bit(u);
						break;
					}
				}
			}
		}
		return awake_count;
	}


	int64_t TDStep(Graph& G, pvector<node_t> &parent, SlidingQueue<node_t> &queue) {
		int64_t scout_count = 0;
		#pragma omp parallel
		{
			QueueBuffer<node_t> lqueue(queue);
			#pragma omp for reduction(+ : scout_count)
			for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
				node_t u = *q_iter;
				//for (node_t v : g.out_neigh(u)) {
				ll_foreach_out(v, G, u) {
					node_t curr_val = parent[v];
					if (curr_val < 0) {
						if (compare_and_swap(parent[v], curr_val, u)) {
							lqueue.push_back(v);
							scout_count += -curr_val;
						}
					}
				}
			}
			lqueue.flush();
		}
		return scout_count;
	}


	void QueueToBitmap(const SlidingQueue<node_t> &queue, Bitmap &bm) {
		#pragma omp parallel for
		for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
			node_t u = *q_iter;
			bm.set_bit_atomic(u);
		}
	}

	void BitmapToQueue(Graph& G, const Bitmap &bm, SlidingQueue<node_t> &queue) {
		#pragma omp parallel
		{
			QueueBuffer<node_t> lqueue(queue);
			#pragma omp for
			for (node_t n=0; n < G.max_nodes(); n++)
			{
				if (bm.get_bit(n))
				{
					lqueue.push_back(n);
				}
			}
			lqueue.flush();
		}
		queue.slide_window();
	}

	void InitParent(Graph& G) {
		//printf("InitParent ->\n");
		//printf("max_nodes: %d\n", G.max_nodes());
		parent.resize(G.max_nodes());
		
		#pragma omp parallel for
		for (node_t n=0; n < G.max_nodes(); n++)
		{
			parent[n] = G.out_degree(n) != 0 ? (-1) * G.out_degree(n) : -1;
		}
		
		//printf("InitParent <-\n");
//		return parent;
	}

  void PrintBFSStats(const pvector<node_t> &bfs_tree) {
    Graph& g = *this->_graph;
    int64_t tree_size = 0;
    int64_t n_edges = 0;
    for (node_t n=0; n<g.max_nodes(); n+=1) {
      if (bfs_tree[n] >= 0) {
        n_edges += g.out_degree(n);
        tree_size++;
      }
    }
    std::cout << "BFS Tree has " << tree_size << " nodes and ";
    std::cout << n_edges << " edges" << std::endl;
  }
};

/**
 * The BFS benchmark - Direction-Optimizing Breadth-First Search
 */
template <class Graph>
class ll_b_gap_bfs
	: public gap_b_bfs_ext<Graph> {

public:

	/**
	 * Create the benchmark
	 *
	 * @param r the root
	 */
	ll_b_gap_bfs(node_t r)
		: gap_b_bfs_ext<Graph>(r) {}
};

#endif
