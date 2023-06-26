// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#ifndef GRAPH_H_
#define GRAPH_H_

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <assert.h>
#include <cstring>

#include <libpmemobj.h>
#include <libpmem.h>
#include <xmmintrin.h>

#include <mutex>
#include "platform_atomics.h"

#include "pvector.h"
#include "util.h"

using namespace std;

#define BLOCK_SIZE 1022   // 4K

/*
Simple container for graph in BAL format
 - Intended to be constructed by a Builder
*/

// Used to hold destination vertex-id, with another vertex it makes an unweighted edge
template<typename NodeID_>
struct NodeUnweight {
  NodeID_ v;

  NodeUnweight() {}

  NodeUnweight(NodeID_ v) : v(v) {}

  bool operator<(const NodeUnweight &rhs) const {
    return v < rhs.v;
  }

  bool operator==(const NodeUnweight &rhs) const {
    return v == rhs.v;
  }

  bool operator==(const NodeID_ &rhs) const {
    return v == rhs;
  }

  operator NodeID_() {
    return v;
  }
};

// Used to hold destination vertex-id & weight, with another vertex it makes a weighted edge
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
// It is not relevant to DGAP evaluation, kept this to support backward compatibility
typedef int32_t SGID;
typedef EdgePair<SGID> SGEdge;
typedef int64_t SGOffset;
typedef int32_t NodeID;
typedef int32_t WeightT;

// structure for the vertices
struct vertex_element {
  PMEMmutex mlock;
  uint64_t head_offset;
  uint64_t tail_offset;
  uint32_t degree;
};

// blocks of edges
struct edge_block {
  NodeID block[BLOCK_SIZE];   // fixed-length edge-list block
  uint64_t next_offset;       // offset of the next edge_block
};

/* root pmem object */
struct Base {
  uint64_t pool_uuid_lo_;

  int64_t num_nodes_;
  int64_t num_edges_;

  PMEMoid vertices_oid_;

  bool directed_;
  char padding_[7];    //7 Bytes
} __attribute__ ((aligned (8)));

template<class NodeID_, class DestID_ = NodeID_, bool MakeInverse = true>
class CSRGraph {
  // Used for *non-negative* offsets within a neighborhood
  typedef std::make_unsigned<std::ptrdiff_t>::type OffsetT;
  typedef EdgePair<NodeID_, DestID_> Edge;
  typedef pvector<Edge> EdgeList;

  // Used to access neighbors of vertex, basically sugar for iterators
  class Neighborhood {
    struct edge_block *curr_edge_block_;
    uint32_t degree_, curr_idx_;
    uint64_t pool_uuid_lo_;
    DestID_ *begin_ptr_;
    DestID_ *end_ptr_;
  public:
    Neighborhood(struct edge_block *curr_edge_block, OffsetT start_offset, uint32_t degree, uint64_t pool_uuid_lo) :
        curr_edge_block_(curr_edge_block), degree_(degree), curr_idx_(start_offset), pool_uuid_lo_(pool_uuid_lo) {
      if (start_offset >= degree) begin_ptr_ = nullptr;
      else begin_ptr_ = &(curr_edge_block_->block[start_offset]);

      end_ptr_ = nullptr;
    }

    class iterator {
    public:
      struct edge_block *curr_edge_block_;
      uint32_t curr_idx_, degree_;
      uint64_t pool_uuid_lo_;

      iterator() {
        g_index_ = nullptr;
        curr_edge_block_ = nullptr;
        curr_idx_ = 0;
        degree_ = 0;
      }

      iterator(DestID_ *g_index) {
        g_index_ = g_index;
        curr_edge_block_ = nullptr;
        curr_idx_ = 0;
        degree_ = 0;
      }

      iterator(DestID_ *g_index, struct edge_block *curr_edge_block, uint32_t curr_idx, uint32_t degree,
               uint64_t pool_uuid_lo) {
        g_index_ = g_index;
        curr_edge_block_ = curr_edge_block;
        curr_idx_ = curr_idx;
        degree_ = degree;
        pool_uuid_lo_ = pool_uuid_lo;
      }

      iterator &operator++() {
        curr_idx_ += 1;
        if (curr_idx_ == degree_) g_index_ = nullptr;
        else {
          if (curr_idx_ % BLOCK_SIZE == 0) {
            PMEMoid last_full_block_oid = {pool_uuid_lo_, curr_edge_block_->next_offset};
            curr_edge_block_ = (struct edge_block *) pmemobj_direct(last_full_block_oid);
          }
          g_index_ = &(curr_edge_block_->block[curr_idx_ % BLOCK_SIZE]);
        }
        return *this;
      }

      operator DestID_ *() const {
        return g_index_;
      }

      DestID_ *operator->() {
        return g_index_;
      }

      DestID_ &operator*() {
        return (*g_index_);
      }

      bool operator==(const iterator &rhs) const {
        return g_index_ == rhs.g_index_;
      }

      bool operator!=(const iterator &rhs) const {
        return (g_index_ != rhs.g_index_);
      }

    private:
      DestID_ *g_index_;
    };

    iterator begin() {
      return iterator(begin_ptr_, curr_edge_block_, curr_idx_, degree_, pool_uuid_lo_);
    }

    iterator end() {
      return iterator(end_ptr_);
    }
  };

  void ReleaseResources() {
    if (vertices_ != nullptr) free(vertices_);
  }


public:
  PMEMobjpool *pop;
  PMEMoid base_oid;
  struct Base *bp;

  CSRGraph(const char *dbfilename, EdgeList &base_edge_list, bool is_directed, uint64_t n_edges, uint64_t n_vertices) {
    bool is_new = false;

    if (file_exists(dbfilename) == 0) {   // file already exists
      if ((pop = pmemobj_open(dbfilename, LAYOUT_NAME)) == NULL) {
        fprintf(stderr, "[%s]: FATAL: pmemobj_open error: %s\n", __FUNCTION__, pmemobj_errormsg());
        exit(0);
      }
    } else {
      if ((pop = pmemobj_create(dbfilename, LAYOUT_NAME, DB_POOL_SIZE, CREATE_MODE_RW)) == NULL) {
        fprintf(stderr, "[%s]: FATAL: pmemobj_create error: %s\n", __FUNCTION__, pmemobj_errormsg());
        exit(0);
      }
      is_new = true;
    }

    base_oid = pmemobj_root(pop, sizeof(struct Base));
    bp = (struct Base *) pmemobj_direct(base_oid);
    check_sanity(bp);

    if (is_new) {
      // initialize dram cache
      num_edges_ = n_edges;
      num_nodes_ = n_vertices;
      directed_ = is_directed;
      pool_uuid_lo_ = base_oid.pool_uuid_lo;

      // initialize memory for the base-graph
      bp->pool_uuid_lo_ = base_oid.pool_uuid_lo;
      bp->num_edges_ = n_edges;
      bp->num_nodes_ = n_vertices;
      bp->directed_ = is_directed;

      if (pmemobj_zalloc(pop, &bp->vertices_oid_, n_vertices * sizeof(struct vertex_element), VERTEX_TYPE)) {
        fprintf(stderr, "[%s]: FATAL: edge array allocation failed: %s\n", __func__, pmemobj_errormsg());
        abort();
      }

      // persisting changes in the base-pointer
      flush_clwb_nolog(bp, sizeof(struct Base));

      vertices_pmem_ = (struct vertex_element *) pmemobj_direct(bp->vertices_oid_);
      vertices_ = (struct vertex_element *) malloc(num_nodes_ * sizeof(struct vertex_element));
      memcpy(vertices_, vertices_pmem_, num_nodes_ * sizeof(struct vertex_element));

      /// inserting base-graph edges
      Timer t;
      t.Start();
#pragma omp parallel for
      for (int i = 0; i < num_edges_; i++) {
        if (pmemobj_mutex_lock(pop, &vertices_[base_edge_list[i].u].mlock) != 0) {
          cout << "something went wrong in acquiring lock! Abort!!!" << endl;
          exit(-1);
        }

        uint32_t t_src = base_edge_list[i].u;

        if (vertices_[t_src].degree == 0) {
          // initialize a new edge-list block and update head_offset/tail_offset in the vertex structure
          PMEMoid block_oid;
          if (pmemobj_zalloc(pop, &block_oid, sizeof(struct edge_block), EDGE_TYPE)) {
            fprintf(stderr, "[%s]: FATAL: edge array allocation failed: %s\n", __func__, pmemobj_errormsg());
            abort();
          }
          struct edge_block *curr_block = (struct edge_block *) pmemobj_direct(block_oid);

          curr_block->block[0] = base_edge_list[i].v;
          flush_clwb_nolog(&curr_block->block[0], sizeof(DestID_));

          vertices_[t_src].head_offset = (uint64_t) curr_block;
          vertices_[t_src].tail_offset = (uint64_t) curr_block;
        } else {
          if (vertices_[t_src].degree % BLOCK_SIZE == 0) {
            // as the current block is full, need to create a new block
            PMEMoid block_oid;
            if (pmemobj_zalloc(pop, &block_oid, sizeof(struct edge_block), EDGE_TYPE)) {
              fprintf(stderr, "[%s]: FATAL: edge array allocation failed: %s\n", __func__, pmemobj_errormsg());
              abort();
            }
            struct edge_block *curr_block = (struct edge_block *) pmemobj_direct(block_oid);

            curr_block->block[0] = base_edge_list[i].v;
            flush_clwb_nolog(&curr_block->block[0], sizeof(DestID_));

            // linking current-block at the next pointer of the current tail
            struct edge_block *last_full_block = (struct edge_block *) vertices_[t_src].tail_offset;
            last_full_block->next_offset = block_oid.off;

            flush_clwb_nolog(&last_full_block->next_offset, sizeof(uint64_t));

            // update pointer for the tail block
            vertices_[t_src].tail_offset = (uint64_t) curr_block;
          } else {
            // we have enough space in current block
            struct edge_block *curr_block = (struct edge_block *) vertices_[t_src].tail_offset;
            int32_t curr_idx = vertices_[t_src].degree % BLOCK_SIZE;

            curr_block->block[curr_idx] = base_edge_list[i].v;
            flush_clwb_nolog(&curr_block->block[curr_idx], sizeof(DestID_));
          }
        }
        vertices_[t_src].degree += 1;
        pmemobj_mutex_unlock(pop, &vertices_[base_edge_list[i].u].mlock);
      }

      t.Stop();
      cout << "B-Graph Build Time: " << t.Seconds() << " seconds." << endl;

      memcpy(vertices_pmem_, vertices_, n_vertices * sizeof(struct vertex_element));
      flush_clwb_nolog(vertices_pmem_, n_vertices * sizeof(struct vertex_element));
    } else {
      bp->pool_uuid_lo_ = base_oid.pool_uuid_lo;
      flush_clwb_nolog(bp, sizeof(struct Base));

      directed_ = bp->directed_;
      num_nodes_ = bp->num_nodes_;
      num_edges_ = bp->num_edges_;
      pool_uuid_lo_ = bp->pool_uuid_lo_;

      vertices_pmem_ = (struct vertex_element *) pmemobj_direct(bp->vertices_oid_);
      vertices_ = (struct vertex_element *) malloc(num_nodes_ * sizeof(struct vertex_element));
      memcpy(vertices_, vertices_pmem_, num_nodes_ * sizeof(struct vertex_element));
    }
  }

  ~CSRGraph() {
    memcpy((struct vertex_element *) pmemobj_direct(bp->vertices_oid_),
           vertices_,num_nodes_ * sizeof(struct vertex_element));
    flush_clwb_nolog((struct vertex_element *) pmemobj_direct(bp->vertices_oid_),
                     num_nodes_ * sizeof(struct vertex_element));

    bp->num_nodes_ = num_nodes_;  // Number of vertices
    flush_clwb_nolog(&bp->num_nodes_, sizeof(int64_t));

    bp->num_edges_ = num_edges_;  // Number of edges
    flush_clwb_nolog(&bp->num_edges_, sizeof(int64_t));

    // todo: need to take backup for the vertices_ => vertices_pmem_

    ReleaseResources();
  }

  /// insert dynamic edges
  void insert(uint32_t src, uint32_t dst) {
    if (pmemobj_mutex_lock(pop, &vertices_[src].mlock) != 0) {
      cout << "something went wrong in acquiring lock! Abort!!!" << endl;
      exit(-1);
    }

    if (vertices_[src].degree == 0) {
      // initialize a new edge-list block and update head_offset/tail_offset in the vertex structure
      PMEMoid block_oid;

      if (pmemobj_zalloc(pop, &block_oid, sizeof(struct edge_block), EDGE_TYPE)) {
        fprintf(stderr, "[%s]: FATAL: edge array allocation failed: %s\n", __func__, pmemobj_errormsg());
        abort();
      }

      struct edge_block *curr_block = (struct edge_block *) pmemobj_direct(block_oid);

      curr_block->block[0] = dst;
      flush_clwb_nolog(&curr_block->block[0], sizeof(DestID_));

      vertices_[src].head_offset = (uint64_t) curr_block;
      vertices_[src].tail_offset = (uint64_t) curr_block;
    }
    else {
      if (vertices_[src].degree % BLOCK_SIZE == 0) {
        // as the current block is full, need to create a new block
        PMEMoid block_oid;

        if (pmemobj_zalloc(pop, &block_oid, sizeof(struct edge_block), EDGE_TYPE)) {
          fprintf(stderr, "[%s]: FATAL: edge array allocation failed: %s\n", __func__, pmemobj_errormsg());
          abort();
        }

        struct edge_block *curr_block = (struct edge_block *) pmemobj_direct(block_oid);
        curr_block->block[0] = dst;
        flush_clwb_nolog(&curr_block->block[0], sizeof(DestID_));

        // linking current-block at the next pointer of the current tail
        struct edge_block *last_full_block = (struct edge_block *) vertices_[src].tail_offset;
        last_full_block->next_offset = block_oid.off;
        flush_clwb_nolog(&last_full_block->next_offset, sizeof(uint64_t));

        // update pointer for the tail block
        vertices_[src].tail_offset = (uint64_t) curr_block;
      }
      else {
        // we have enough space in current block
        struct edge_block *curr_block = (struct edge_block *) vertices_[src].tail_offset;
        int32_t curr_idx = vertices_[src].degree % BLOCK_SIZE;

        curr_block->block[curr_idx] = dst;
        flush_clwb_nolog(&curr_block->block[curr_idx], sizeof(DestID_));
      }
    }
    vertices_[src].degree += 1;

    pmemobj_mutex_unlock(pop, &vertices_[src].mlock);

    // updating num_edges_ using CAS atomic instruction
    int64_t old_val, new_val;
    do {
      old_val = num_edges_;
      new_val = old_val + 1;
    } while (!compare_and_swap(num_edges_, old_val, new_val));
  }

  bool directed() const {
    return directed_;
  }

  int64_t num_nodes() const {
    return num_nodes_;
  }

  int64_t num_edges() const {
    return num_edges_;
  }

  int64_t num_edges_directed() const {
    return directed_ ? num_edges_ : 2 * num_edges_;
  }

  int64_t out_degree(NodeID_ v) const {
    return vertices_[v].degree;
  }

  int64_t in_degree(NodeID_ v) const {
    static_assert(MakeInverse, "Graph inversion disabled but reading inverse");
    return vertices_[v].degree;
  }

  Neighborhood out_neigh(NodeID_ n, OffsetT start_offset = 0) const {
    struct edge_block *block_ptr = (struct edge_block *) vertices_[n].head_offset;
    return Neighborhood(block_ptr, start_offset, vertices_[n].degree, pool_uuid_lo_);
  }

  Neighborhood in_neigh(NodeID_ n, OffsetT start_offset = 0) const {
    static_assert(MakeInverse, "Graph inversion disabled but reading inverse");
    struct edge_block *block_ptr = (struct edge_block *) vertices_[n].head_offset;
    return Neighborhood(block_ptr, start_offset, vertices_[n].degree, pool_uuid_lo_);
  }

  void PrintStats() const {
    std::cout << "Graph has " << num_nodes_ << " nodes and "
              << num_edges_ << " ";
    if (!directed_)
      std::cout << "un";
    std::cout << "directed edges for degree: ";
    std::cout << num_edges_ / num_nodes_ << std::endl;
  }

  void PrintTopology() const {
    for (NodeID_ i = 0; i < num_nodes_; i++) {
      std::cout << i << ": ";
      for (DestID_ j: out_neigh(i)) {
        std::cout << j << " ";
      }
      std::cout << std::endl;
    }
  }

  void PrintTopology(NodeID_ src) const {
    uint32_t j = 0;
    PMEMoid curr_oid;
    uint64_t curr_ptr = vertices_[src].head_offset;
    std::cout << src << "(" << vertices_[src].degree << "): ";
    while (curr_ptr) {
      struct edge_block *curr_edge_block = (struct edge_block *) curr_ptr;
      cout << curr_edge_block->block[j % BLOCK_SIZE] << " ";
      j += 1;
      if (j == vertices_[src].degree) break;
      if (j % BLOCK_SIZE == 0) {
        curr_oid = {pool_uuid_lo_, curr_edge_block->next_offset};
        curr_ptr = (uint64_t)((struct edge_block *) pmemobj_direct(curr_oid));
      }
    }
    cout << endl;

    std::cout << src << "(" << out_degree(src) << "): ";
    for (DestID_ j: out_neigh(src)) {
      std::cout << j.v << " ";
    }
    std::cout << std::endl << std::endl;
  }

  static DestID_ **GenIndex(const pvector<SGOffset> &offsets, DestID_ *neighs) {
    NodeID_ length = offsets.size();
    DestID_ **index = new DestID_ *[length];
#pragma omp parallel for
    for (NodeID_ n = 0; n < length; n++)
      index[n] = neighs + offsets[n];
    return index;
  }

  // It is not relevant to DGAP evaluation, kept this to support backward compatibility
  pvector<SGOffset> VertexOffsets(bool in_graph = false) const {
    pvector<SGOffset> offsets(num_nodes_ + 1);
    return offsets;
  }

  Range<NodeID_> vertices() const {
    return Range<NodeID_>(num_nodes());
  }

private:
  bool directed_;
  int64_t num_nodes_;
  int64_t num_edges_;
  uint64_t pool_uuid_lo_;
  struct vertex_element *vertices_pmem_;      //underlying storage for vertex list
  struct vertex_element *vertices_;           //underlying storage for vertex list
};

#endif  // GRAPH_H_
