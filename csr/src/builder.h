// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#ifndef BUILDER_H_
#define BUILDER_H_

#include <algorithm>
#include <cinttypes>
#include <fstream>
#include <functional>
#include <type_traits>
#include <utility>

#include "command_line.h"
#include "generator.h"
#include "graph.h"
#include "platform_atomics.h"
#include "pvector.h"
#include "reader.h"
#include "timer.h"
#include "util.h"


/*
GAP Benchmark Suite
Class:  BuilderBase
Author: Scott Beamer

Given arguements from the command line (cli), returns a built graph
 - MakeGraph() will parse cli and obtain edgelist and call
   MakeGraphFromEL(edgelist) to perform actual graph construction
 - edgelist can be from file (reader) or synthetically generated (generator)
 - Common case: BuilderBase typedef'd (w/ params) to be Builder (benchmark.h)
*/


template<typename NodeID_, typename DestID_ = NodeID_,
    typename WeightT_ = NodeID_, bool invert = true>
class BuilderBase {
  typedef EdgePair<NodeID_, DestID_> Edge;
  typedef pvector<Edge> EdgeList;

  const CLBase &cli_;
  bool symmetrize_;
  bool needs_weights_;
  int64_t num_nodes_ = -1;

public:
  explicit BuilderBase(const CLBase &cli) : cli_(cli) {
    symmetrize_ = cli_.symmetrize();
    needs_weights_ = !std::is_same<NodeID_, DestID_>::value;
  }

  DestID_ GetSource(EdgePair<NodeID_, NodeID_> e) {
    return e.u;
  }

  DestID_ GetSource(EdgePair<NodeID_, NodeWeight<NodeID_, WeightT_>> e) {
    return NodeWeight<NodeID_, WeightT_>(e.u, e.v.w);
  }

  NodeID_ FindMaxNodeID(const EdgeList &el) {
    NodeID_ max_seen = 0;
#pragma omp parallel for reduction(max : max_seen)
    for (auto it = el.begin(); it < el.end(); it++) {
      Edge e = *it;
      max_seen = std::max(max_seen, e.u);
      max_seen = std::max(max_seen, (NodeID_) e.v);
    }
    return max_seen;
  }

  pvector<NodeID_> CountDegrees(const EdgeList &el, bool transpose) {
    pvector<NodeID_> degrees(num_nodes_, 0);
#pragma omp parallel for
    for (auto it = el.begin(); it < el.end(); it++) {
      Edge e = *it;
      if (symmetrize_ || (!symmetrize_ && !transpose)) {
        fetch_and_add(degrees[e.u], 1);
      }
    }
    return degrees;
  }

  static
  pvector<SGOffset> PrefixSum(const pvector<NodeID_> &degrees) {
    pvector<SGOffset> sums(degrees.size() + 1);
    SGOffset total = 0;
    for (size_t n = 0; n < degrees.size(); n++) {
      sums[n] = total;
      total += degrees[n];
    }
    sums[degrees.size()] = total;
    return sums;
  }

  static
  pvector<SGOffset> ParallelPrefixSum(const pvector<NodeID_> &degrees) {
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
    pvector<SGOffset> prefix(degrees.size() + 1);
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
    return prefix;
  }

  // Removes self-loops and redundant edges
  // Side effect: neighbor IDs will be sorted
  void SquishCSR(const CSRGraph<NodeID_, DestID_, invert> &g, bool transpose,
                 DestID_ ***sq_index, DestID_ **sq_neighs) {
    pvector<NodeID_> diffs(g.num_nodes());
    DestID_ *n_start, *n_end;
#pragma omp parallel for private(n_start, n_end)
    for (NodeID_ n = 0; n < g.num_nodes(); n++) {
      if (transpose) {
        n_start = g.in_neigh(n).begin();
        n_end = g.in_neigh(n).end();
      } else {
        n_start = g.out_neigh(n).begin();
        n_end = g.out_neigh(n).end();
      }
      // std::sort(n_start, n_end);
      DestID_ *new_end = std::unique(n_start, n_end);   // remove redundant edges
      new_end = std::remove(n_start, new_end, n);       // remove self-loops
      diffs[n] = new_end - n_start;
    }
    pvector<SGOffset> sq_offsets = ParallelPrefixSum(diffs);
    *sq_neighs = new DestID_[sq_offsets[g.num_nodes()]];
    *sq_index = CSRGraph<NodeID_, DestID_>::GenIndex(sq_offsets, *sq_neighs);
#pragma omp parallel for private(n_start)
    for (NodeID_ n = 0; n < g.num_nodes(); n++) {
      if (transpose) n_start = g.in_neigh(n).begin();
      else n_start = g.out_neigh(n).begin();
      std::copy(n_start, n_start + diffs[n], (*sq_index)[n]);
    }
  }

  CSRGraph<NodeID_, DestID_, invert> SquishGraph(const CSRGraph<NodeID_, DestID_, invert> &g) {
    DestID_ **out_index, *out_neighs, **in_index, *in_neighs;
    SquishCSR(g, false, &out_index, &out_neighs);
    if (g.directed()) {
      if (invert) SquishCSR(g, true, &in_index, &in_neighs);
      return CSRGraph<NodeID_, DestID_, invert>(g.num_nodes(), out_index,
                                                out_neighs, in_index,
                                                in_neighs);
    } else {
      return CSRGraph<NodeID_, DestID_, invert>(g.num_nodes(), out_index,
                                                out_neighs);
    }
  }

  /*
  Graph Bulding Steps (for CSR):
    - Read edgelist once to determine vertex degrees (CountDegrees)
    - Determine vertex offsets by a prefix sum (ParallelPrefixSum)
    - Allocate storage and set points according to offsets (GenIndex)
    - Copy edges into storage
  */
  void MakeCSR(const EdgeList &el, bool transpose, DestID_ ***index, DestID_ **neighs) {
    pvector<NodeID_> degrees = CountDegrees(el, transpose);
    pvector<SGOffset> offsets = ParallelPrefixSum(degrees);
    *neighs = new DestID_[offsets[num_nodes_]];
    *index = CSRGraph<NodeID_, DestID_>::GenIndex(offsets, *neighs);
#pragma omp parallel for
    for (auto it = el.begin(); it < el.end(); it++) {
      Edge e = *it;
      if (symmetrize_ || (!symmetrize_ && !transpose)) {
        (*neighs)[fetch_and_add(offsets[e.u], 1)] = e.v;
      }
    }
  }

  /*
  Graph Bulding Steps (for CSR):
    - Vertex degrees and offsets (by prefix sum) already calculated and passed as a parameter in this method
    - Allocate in-memory storage and set points according to offsets (GenIndex)
    - Copy edges into im-memory storage
    - Side effect: neighbor IDs will be sorted
  */
  void MakeCSR(const EdgeList &el, bool transpose, int64_t **index, DestID_ **neighs,
               const pvector<NodeID_> &degrees, pvector<SGOffset> &offsets) {
    *neighs = new DestID_[offsets[num_nodes_]];
    *index = new int64_t[offsets.size()];

    NodeID_ index_length = offsets.size();
    for (NodeID_ n = 0; n < index_length; n++) {
      (*index)[n] = offsets[n];
    }

#pragma omp parallel for
    for (auto it = el.begin(); it < el.end(); it++) {
      Edge e = *it;
      if (symmetrize_ || (!symmetrize_ && !transpose)) {
        (*neighs)[fetch_and_add(offsets[e.u], 1)] = e.v;
      }
    }
  }

  CSRGraph<NodeID_, DestID_, invert> MakeGraphFromEL(EdgeList &el) {
    DestID_ **index = nullptr, **inv_index = nullptr;
    DestID_ *neighs = nullptr, *inv_neighs = nullptr;
    Timer t;
    t.Start();
    if (num_nodes_ == -1) num_nodes_ = FindMaxNodeID(el) + 1;
    if (needs_weights_) Generator<NodeID_, DestID_, WeightT_>::InsertWeights(el);

    //making the out_neighbors
    MakeCSR(el, false, &index, &neighs);
    //for directed graph, making the in_neighbors
    if (!symmetrize_ && invert) MakeCSR(el, true, &inv_index, &inv_neighs);

    t.Stop();
    PrintTime("Build Time", t.Seconds());
    if (symmetrize_) {
      return CSRGraph<NodeID_, DestID_, invert>(num_nodes_, index, neighs);
    } else {
      return CSRGraph<NodeID_, DestID_, invert>(num_nodes_, index, neighs, inv_index, inv_neighs);
    }
  }

  void MakePMemGraphFromEL(EdgeList &el, CSRGraph<NodeID_, DestID_, invert> &g) {
    Timer t;
    t.Start();
    if (num_nodes_ == -1) num_nodes_ = FindMaxNodeID(el) + 1;
    if (needs_weights_) Generator<NodeID_, DestID_, WeightT_>::InsertWeights(el);

    TX_BEGIN(g.pop)
    {
      pmemobj_tx_add_range_direct(g.bp, sizeof(struct Base));
      g.bp->pool_uuid_lo = g.base_oid.pool_uuid_lo;

      //making the out_neighbors
      pvector<NodeID_> out_degrees = CountDegrees(el, false);
      pvector<SGOffset> out_offsets = ParallelPrefixSum(out_degrees);

      int64_t *mem_out_index = nullptr;
      DestID_ *mem_out_neighs = nullptr;
      NodeID_ out_index_length = out_offsets.size();

      MakeCSR(el, false, &mem_out_index, &mem_out_neighs, out_degrees, out_offsets);

      g.bp->oid_out_index_ = pmemobj_tx_zalloc(sizeof(int64_t) * out_index_length, INDEX_TYPE);
      int64_t *pmem_out_index_ = (int64_t *) pmemobj_direct(g.bp->oid_out_index_);
      pmemobj_tx_add_range_direct(pmem_out_index_, sizeof(NodeID_) * out_index_length);

      g.bp->oid_out_neighbors_ = pmemobj_tx_zalloc(sizeof(DestID_) * out_offsets[num_nodes_], NEIGHBOR_TYPE);
      DestID_ *pmem_out_neighbors_ = (DestID_ *) pmemobj_direct(g.bp->oid_out_neighbors_);
      pmemobj_tx_add_range_direct(pmem_out_neighbors_, sizeof(DestID_) * out_offsets[num_nodes_]);

      memcpy(pmem_out_index_, mem_out_index, sizeof(int64_t) * out_index_length);
      memcpy(pmem_out_neighbors_, mem_out_neighs, sizeof(DestID_) * out_offsets[num_nodes_]);

      g.bp->oid_in_index_ = g.bp->oid_out_index_;
      g.bp->oid_in_neighbors_ = g.bp->oid_out_neighbors_;

      g.bp->num_edges_ = (pmem_out_index_[num_nodes_] - pmem_out_index_[0]);
      g.bp->directed_ = false;

      g.bp->num_nodes_ = num_nodes_;
      g.bp->weighted_ = !std::is_same<NodeID_, DestID_>::value;

      g.update_dram_cache(false, g.bp->num_nodes_, g.bp->num_edges_);
    }
    TX_ONABORT{
        fprintf(stderr, "[%s]: FATAL: transaction aborted: %s\n", __func__, pmemobj_errormsg());
        abort();
    }
    TX_END

    t.Stop();
    PrintTime("PMem Build Time", t.Seconds());
  }

  CSRGraph<NodeID_, DestID_, invert> MakeGraph() {
    if (file_exists(cli_.dbfilename().c_str()) == 0) {
      printf("[%s]: pmem db-file already exists, no needs to ingest data into pmem\n", __FUNCTION__);
      return CSRGraph<NodeID_, DestID_, invert>(cli_.dbfilename().c_str());
    }
    CSRGraph<NodeID_, DestID_, invert> g = CSRGraph<NodeID_, DestID_, invert>(cli_.dbfilename().c_str());
    {  // extra scope to trigger earlier deletion of el (save memory)
      EdgeList el;
      if (cli_.filename() != "") {
        Reader<NodeID_, DestID_, WeightT_, invert> r(cli_.filename());
        if ((r.GetSuffix() == ".sg") || (r.GetSuffix() == ".wsg")) {
          return r.ReadSerializedGraph();
        } else {
          el = r.ReadFile(needs_weights_);
        }
      } else if (cli_.scale() != -1) {
        Generator<NodeID_, DestID_> gen(cli_.scale(), cli_.degree());
        el = gen.GenerateEL(cli_.uniform());
      }
      MakePMemGraphFromEL(el, g);
    }
    // note: we omit the removing of "self-loops" and "redundant edges" from the squishing step
    // sorting neighbor IDs is done in: MakePMemGraphFromEL()->MakeCSR()
    //return SquishGraph(g);
    return g;
  }
};

#endif  // BUILDER_H_
