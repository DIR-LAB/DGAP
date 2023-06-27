// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#ifndef GRAPH_H_
#define GRAPH_H_

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <libpmemobj.h>
#include <libpmem.h>

#include "pvector.h"
#include "util.h"


/*
Simple container for graph in CSR format
 - Intended to be constructed by a Builder
*/


// Used to hold node & weight, with another node it makes a weighted edge
template<typename NodeID_, typename WeightT_>
struct NodeWeight {
  NodeID_ v;
  WeightT_ w;

  NodeWeight() {}

  NodeWeight(NodeID_ v) : v(v), w(1) {}

  NodeWeight(NodeID_ v, WeightT_ w) : v(v), w(w) {}

  bool operator<(const NodeWeight &rhs) const {
    return v == rhs.v ? w < rhs.w : v < rhs.v;
  }

  // doesn't check WeightT_s, needed to remove duplicate edges
  bool operator==(const NodeWeight &rhs) const {
    return v == rhs.v;
  }

  // doesn't check WeightT_s, needed to remove self edges
  bool operator==(const NodeID_ &rhs) const {
    return v == rhs;
  }

  operator NodeID_() {
    return v;
  }
};

template<typename NodeID_, typename WeightT_>
std::ostream &operator<<(std::ostream &os,
                         const NodeWeight<NodeID_, WeightT_> &nw) {
  os << nw.v << " " << nw.w;
  return os;
}

template<typename NodeID_, typename WeightT_>
std::istream &operator>>(std::istream &is, NodeWeight<NodeID_, WeightT_> &nw) {
  is >> nw.v >> nw.w;
  return is;
}


// Syntatic sugar for an edge
template<typename SrcT, typename DstT = SrcT>
struct EdgePair {
  SrcT u;
  DstT v;

  EdgePair() {}

  EdgePair(SrcT u, DstT v) : u(u), v(v) {}
};

// SG = serialized graph, these types are for writing graph to file
typedef int32_t SGID;
typedef EdgePair<SGID> SGEdge;
typedef int64_t SGOffset;


/* the root object */
struct Base {
  uint64_t pool_uuid_lo;

  int64_t num_nodes_;
  int64_t num_edges_;

  PMEMoid oid_out_index_;
  PMEMoid oid_out_neighbors_;
  PMEMoid oid_in_index_;
  PMEMoid oid_in_neighbors_;

  bool directed_;
  bool weighted_;
  char padding_[6];    //6 Bytes
} __attribute__ ((aligned (8)));

template<class NodeID_, class DestID_ = NodeID_, bool MakeInverse = true>
class CSRGraph {
  // Used for *non-negative* offsets within a neighborhood
  typedef std::make_unsigned<std::ptrdiff_t>::type OffsetT;

  // Used to access neighbors of vertex, basically sugar for iterators
  class Neighborhood {
    DestID_ *st_index_;
    DestID_ *nd_index_;
    OffsetT start_offset_;
  public:
    Neighborhood(DestID_ *st_index_, DestID_ *nd_index_, OffsetT start_offset) :
        st_index_(st_index_), nd_index_(nd_index_), start_offset_(0) {
      OffsetT max_offset = end() - begin();
      start_offset_ = std::min(start_offset, max_offset);
    }

    typedef DestID_ *iterator;

    iterator begin() { return st_index_ + start_offset_; }

    iterator end() { return nd_index_; }
  };

  void ReleaseResources() {
    //pmemobj_close(pop);
  }


public:
  PMEMobjpool *pop;
  PMEMoid base_oid;
  struct Base *bp;

  CSRGraph() : directed_(false), num_nodes_(-1), num_edges_(-1),
               out_index_(nullptr), out_neighbors_(nullptr),
               in_index_(nullptr), in_neighbors_(nullptr) {}

  CSRGraph(int64_t num_nodes, DestID_ **index, DestID_ *neighs) :
      directed_(false), num_nodes_(num_nodes),
      out_index_(index), out_neighbors_(neighs),
      in_index_(index), in_neighbors_(neighs) {
    num_edges_ = (out_index_[num_nodes_] - out_index_[0]) / 2;
  }

  CSRGraph(int64_t num_nodes, DestID_ **out_index, DestID_ *out_neighs,
           DestID_ **in_index, DestID_ *in_neighs) :
      directed_(true), num_nodes_(num_nodes),
      out_index_(out_index), out_neighbors_(out_neighs),
      in_index_(in_index), in_neighbors_(in_neighs) {
    num_edges_ = out_index_[num_nodes_] - out_index_[0];
  }

  /**
   * Constructor for PMEM graph, only retrieve the pmemobj_root object
   * */
  CSRGraph(const char *dbfilename) {
    /* dbfile already exists */
    if (file_exists(dbfilename) == 0) {
      if ((pop = pmemobj_open(dbfilename, LAYOUT_NAME)) == NULL) {
        fprintf(stderr, "[%s]: FATAL: pmemobj_open error: %s\n", __FUNCTION__, pmemobj_errormsg());
        exit(0);
      }
    } else {
      if ((pop = pmemobj_create(dbfilename, LAYOUT_NAME, DB_POOL_SIZE, CREATE_MODE_RW)) == NULL) {
        fprintf(stderr, "[%s]: FATAL: pmemobj_create error: %s\n", __FUNCTION__, pmemobj_errormsg());
        exit(0);
      }
    }

    base_oid = pmemobj_root(pop, sizeof(struct Base));
    bp = (struct Base *) pmemobj_direct(base_oid);
    check_sanity(bp);

    directed_ = bp->directed_;
    num_nodes_ = bp->num_nodes_;
    num_edges_ = bp->num_edges_;
  }

  CSRGraph(CSRGraph &&other) : pop(other.pop), base_oid(other.base_oid), bp(other.bp),
                               directed_(other.directed_),
                               num_nodes_(other.num_nodes_), num_edges_(other.num_edges_),
                               out_index_(other.out_index_), out_neighbors_(other.out_neighbors_),
                               in_index_(other.in_index_), in_neighbors_(other.in_neighbors_) {
    other.num_edges_ = -1;
    other.num_nodes_ = -1;
    other.out_index_ = nullptr;
    other.out_neighbors_ = nullptr;
    other.in_index_ = nullptr;
    other.in_neighbors_ = nullptr;

    other.pop = nullptr;
    other.base_oid = OID_NULL;
    other.bp = nullptr;
  }

  ~CSRGraph() {
    ReleaseResources();
  }

  CSRGraph &operator=(CSRGraph &&other) {
    if (this != &other) {
      ReleaseResources();
      directed_ = other.directed_;
      num_edges_ = other.num_edges_;
      num_nodes_ = other.num_nodes_;
      out_index_ = other.out_index_;
      out_neighbors_ = other.out_neighbors_;
      in_index_ = other.in_index_;
      in_neighbors_ = other.in_neighbors_;
      pop = other.pop;
      base_oid = other.base_oid;
      bp = other.bp;

      other.num_edges_ = -1;
      other.num_nodes_ = -1;
      other.out_index_ = nullptr;
      other.out_neighbors_ = nullptr;
      other.in_index_ = nullptr;
      other.in_neighbors_ = nullptr;
      other.pop = nullptr;
      other.base_oid = OID_NULL;
      other.bp = nullptr;
    }
    return *this;
  }

  void update_dram_cache(bool directed, int64_t num_nodes, int64_t num_edges) {
    directed_ = directed;
    num_nodes_ = num_nodes;
    num_edges_ = num_edges;
  }

  bool directed() const {
    return bp->directed_;
  }

  int64_t num_nodes() const {
    return bp->num_nodes_;
  }

  int64_t num_edges() const {
    return bp->num_edges_;
  }

  int64_t num_edges_directed() const {
    return bp->directed_ ? bp->num_edges_ : 2 * bp->num_edges_;
  }

  int64_t out_degree(NodeID_ v) const {
    int64_t *ptr_out_index = (int64_t *) pmemobj_direct(bp->oid_out_index_);
    return (ptr_out_index[v + 1]) - (ptr_out_index[v]);
  }

  int64_t in_degree(NodeID_ v) const {
    static_assert(MakeInverse, "Graph inversion disabled but reading inverse");
    int64_t *ptr_in_index = (int64_t *) pmemobj_direct(bp->oid_in_index_);
    return (ptr_in_index[v + 1]) - (ptr_in_index[v]);
  }

  Neighborhood out_neigh(NodeID_ n, OffsetT start_offset = 0) const {
    int64_t *ptr_out_index = (int64_t *) pmemobj_direct(bp->oid_out_index_);
    DestID_ *st_index = (DestID_ *) ((DestID_ *) pmemobj_direct(bp->oid_out_neighbors_) + (ptr_out_index[n]));
    DestID_ *nd_index = (DestID_ *) ((DestID_ *) pmemobj_direct(bp->oid_out_neighbors_) + (ptr_out_index[n + 1]));
    return Neighborhood(st_index, nd_index, start_offset);
  }

  Neighborhood in_neigh(NodeID_ n, OffsetT start_offset = 0) const {
    static_assert(MakeInverse, "Graph inversion disabled but reading inverse");
    int64_t *ptr_in_index = (int64_t *) pmemobj_direct(bp->oid_in_index_);
    DestID_ *st_index = (DestID_ *) ((DestID_ *) pmemobj_direct(bp->oid_in_neighbors_) + (ptr_in_index[n]));
    DestID_ *nd_index = (DestID_ *) ((DestID_ *) pmemobj_direct(bp->oid_in_neighbors_) + (ptr_in_index[n + 1]));
    return Neighborhood(st_index, nd_index, start_offset);
  }

  void PrintStats() const {
    std::cout << "Graph has " << bp->num_nodes_ << " nodes and "
              << bp->num_edges_ << " ";
    if (!bp->directed_)
      std::cout << "un";
    std::cout << "directed edges for degree: ";
    std::cout << bp->num_edges_ / bp->num_nodes_ << std::endl;
  }

  void PrintTopology() const {
    for (NodeID_ i = 0; i < bp->num_nodes_; i++) {
      std::cout << i << ": ";
      for (DestID_ j: out_neigh(i)) {
        std::cout << j << " ";
      }
      std::cout << std::endl;
    }
  }

  void PrintTopology(NodeID_ n) const {
    std::cout << n << ": ";
    for (DestID_ j: out_neigh(n)) {
      std::cout << j << " ";
    }
    std::cout << std::endl;
  }

  static DestID_ **GenIndex(const pvector<SGOffset> &offsets, DestID_ *neighs) {
    NodeID_ length = offsets.size();
    DestID_ **index = new DestID_ *[length];
#pragma omp parallel for
    for (NodeID_ n = 0; n < length; n++)
      index[n] = neighs + offsets[n];
    return index;
  }

  pvector<SGOffset> VertexOffsets(bool in_graph = false) const {
    pvector<SGOffset> offsets(num_nodes_ + 1);
    for (NodeID_ n = 0; n < num_nodes_ + 1; n++)
      if (in_graph)
        offsets[n] = in_index_[n] - in_index_[0];
      else
        offsets[n] = out_index_[n] - out_index_[0];
    return offsets;
  }

  Range<NodeID_> vertices() const {
    return Range<NodeID_>(num_nodes());
  }

private:
  bool directed_;
  int64_t num_nodes_;
  int64_t num_edges_;
  DestID_ **out_index_;
  DestID_ *out_neighbors_;
  DestID_ **in_index_;
  DestID_ *in_neighbors_;
};

#endif  // GRAPH_H_
