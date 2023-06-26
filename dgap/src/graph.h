// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#ifndef GRAPH_H_
#define GRAPH_H_

#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <stdio.h>
#include <cstdlib>
#include <assert.h>
#include <inttypes.h>
#include <vector>
#include <map>
#include <cstring>
#include <omp.h>
#include <climits>

#include <fstream>
#include <string>

#include <libpmemobj.h>
#include <libpmem.h>
#include <xmmintrin.h>

#include <condition_variable>
#include <mutex>
#include "platform_atomics.h"

#include "pvector.h"
#include "util.h"

using namespace std;

#define MAX_LOG_ENTRIES 170   // ~1.99 KB (2040 Bytes)
#define MAX_ULOG_ENTRIES 512  // 2KB

/*
Simple container for graph in DGAP format
 - Intended to be constructed by a Builder
*/

// Used to hold node & weight, with another node it makes a weighted edge
struct LogEntry {
  int32_t u;
  int32_t v;
  int32_t prev_offset;

  LogEntry() {}

  LogEntry(int32_t u, int32_t v) : u(u), v(v), prev_offset(-1) {}

  LogEntry(int32_t u, int32_t v, int32_t prev_offset) : u(u), v(v), prev_offset(prev_offset) {}

  bool operator<(const LogEntry &rhs) const {
    return (u < rhs.u);
  }

  bool operator==(const LogEntry &rhs) const {
    return (u == rhs.u && v == rhs.v);
  }
};

struct PMALeafSegment {
  mutex lock;
  condition_variable cv;
  int32_t on_rebalance;

  PMALeafSegment() {
    on_rebalance = 0;
  }

  void wait_for_rebalance(unique_lock <mutex> &ul) {
    this->cv.wait(ul, [this] { return !on_rebalance; });
  }
};

// Used to hold node & weight, with another node it makes a weighted edge
//template <typename NodeID_, typename WeightT_, typename TimestampT_>
template<typename NodeID_>
struct NodeWeight {
  NodeID_ v;

  NodeWeight() {}

  NodeWeight(NodeID_ v) : v(v) {}

  bool operator<(const NodeWeight &rhs) const {
    return v < rhs.v;
  }

  bool operator==(const NodeWeight &rhs) const {
    return v == rhs.v;
  }

  bool operator==(const NodeID_ &rhs) const {
    return v == rhs;
  }

  operator NodeID_() {
    return v;
  }
};

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
typedef int64_t TimestampT;
typedef EdgePair<SGID> SGEdge;
typedef int64_t SGOffset;

// structure for the vertices
struct vertex_element {
  int64_t index;
  int32_t degree;
  int32_t offset;
};

/* the root object */
struct Base {
  uint64_t pool_uuid_lo;

  PMEMoid vertices_oid_;
  PMEMoid edges_oid_;
  PMEMoid ulog_oid_;              // oid of undo logs
  PMEMoid oplog_oid_;             // oid of operation logs
  PMEMoid log_segment_oid_;       // oid of logs per segment
  PMEMoid log_segment_idx_oid_;   // oid of current insert-index of logs per segment

  PMEMoid segment_edges_actual_oid_;  // actual number of edges stored in the region of a binary-tree node
  PMEMoid segment_edges_total_oid_;   // total number of edges assigned in the region of a binary-tree node

  /* General graph fields */
  int64_t num_vertices;   // Number of vertices
  int64_t num_edges_;     // Number of edges

  /* General PMA fields */
  int64_t elem_capacity; // size of the edges_ array
  int64_t segment_count; // number of pma leaf segments
  int32_t segment_size;  // size of a pma leaf segment
  int32_t tree_height;   // height of the pma tree

  bool directed_;
  bool backed_up_;
  char padding_[6];    //6 Bytes
} __attribute__ ((aligned (8)));

template<class NodeID_, class DestID_ = NodeID_, bool MakeInverse = true>
class CSRGraph {
  // Used for *non-negative* offsets within a neighborhood
  typedef std::make_unsigned<std::ptrdiff_t>::type OffsetT;
  typedef EdgePair<NodeID_, DestID_> Edge;
  typedef pvector<Edge> EdgeList;

  // Used to access neighbors of vertex, basically sugar for iterators
  class Neighborhood {
    DestID_ *seg_base_ptr;
    struct vertex_element *src_v;
    struct LogEntry *log_p;
    int32_t start_offset;
    int32_t onseg_edges, log_index = -1;

    DestID_ *begin_ptr;
    DestID_ *end_ptr;
    bool onseg = false, onlog = false;

  public:
    Neighborhood(DestID_ *seg_base_ptr_, struct vertex_element *src_v_, OffsetT start_offset_, struct LogEntry *log_p_,
                 int32_t onseg_edges_) :
        seg_base_ptr(seg_base_ptr_), src_v(src_v_), log_p(log_p_), onseg_edges(onseg_edges_) {

      start_offset = std::min((int32_t) start_offset_, src_v->degree);
      if (src_v->degree > start_offset) {  // have data to iterate
        if (onseg_edges_ > start_offset) { // have data to iterate from segment
          begin_ptr = (DestID_ *) (seg_base_ptr + src_v->index + start_offset);
          log_index = src_v_->offset;
          onseg = true;
        } else {  // no data left on the segment; need to iterate over logs
          log_index = src_v_->offset;
          start_offset_ = start_offset - onseg_edges_;

          while (start_offset_ != 0) {
            log_index = log_p[log_index].prev_offset;
            start_offset_ -= 1;
          }
          begin_ptr = new DestID_();
          begin_ptr->v = log_p[log_index].v;
          log_index = log_p[log_index].prev_offset;

          onlog = true;
        }
      } else begin_ptr = nullptr;
      end_ptr = nullptr;
    }

    class iterator {
    public:
      int32_t onseg_edges;
      struct vertex_element *src_v;
      struct LogEntry *log_p;
      int32_t log_index, iterator_index;
      bool onseg, onlog;

      iterator() {
        ptr = NULL;
        iterator_index = 1;
        onseg = false;
        onlog = false;
      }

      iterator(DestID_ *p) {
        ptr = p;
        iterator_index = 1;
        onseg = false;
        onlog = false;
      }

      iterator(DestID_ *p, int32_t onseg_edges_, struct vertex_element *src_v_, struct LogEntry *log_p_,
               int32_t log_index_, int32_t iterator_index_, bool onseg_, bool onlog_) :
          onseg_edges(onseg_edges_), src_v(src_v_), log_p(log_p_), log_index(log_index_),
          iterator_index(iterator_index_), onseg(onseg_), onlog(onlog_) {
        ptr = p;
      }

      iterator &operator++() {
        iterator_index += 1;

        if (onseg) { // on-seg
          // if iterator_index goes beyond the onseg_edges
          if (iterator_index > onseg_edges) {
            // if degree > onseg_edges: switch to onlog
            if (src_v->degree > onseg_edges) {
              // this re-initialization of ptr is very crucial; otherwise it will overwrite the existing data
              ptr = new DestID_();
              ptr->v = log_p[log_index].v;

              log_index = log_p[log_index].prev_offset;
              onseg = false;
              onlog = true;
            } else {  // stop
              ptr = nullptr;
            }
          } else {
            ptr = (DestID_ *) ptr + 1;
          }
        } else { // onlog
          // if the prev_offset of current iteration is -1; stop
          if (log_index != -1) {
            ptr->v = log_p[log_index].v;
            log_index = log_p[log_index].prev_offset;
          } else {  // stop
            ptr = nullptr;
          }
        }

        return *this;
      }

      operator DestID_ *() const {
        return ptr;
      }

      DestID_ *operator->() {
        return ptr;
      }

      DestID_ &operator*() {
        return (*ptr);
      }

      bool operator==(const iterator &rhs) const {
        return ptr == rhs.ptr;
      }

      bool operator!=(const iterator &rhs) const {
        return (ptr != rhs.ptr);
      }

    private:
      DestID_ *ptr;
    };

    iterator begin() {
      return iterator(begin_ptr, onseg_edges, src_v, log_p, log_index, start_offset + 1, onseg, onlog);
    }

    iterator end() {
      return iterator(end_ptr);
    }
  };

  void ReleaseResources() {
    if (vertices_ != nullptr) free(vertices_);
    if (log_ptr_ != nullptr) free(log_ptr_);
    if (log_segment_idx_ != nullptr) free(log_segment_idx_);
    if (segment_edges_actual != nullptr) free(segment_edges_actual);
    if (segment_edges_total != nullptr) free(segment_edges_total);
  }


public:
  PMEMobjpool *pop;
  PMEMoid base_oid;
  struct Base *bp;

  ~CSRGraph() {
    memcpy((struct vertex_element *) pmemobj_direct(bp->vertices_oid_), vertices_,
           num_vertices * sizeof(struct vertex_element));
    flush_clwb_nolog((struct vertex_element *) pmemobj_direct(bp->vertices_oid_),
                     num_vertices * sizeof(struct vertex_element));

    memcpy((int32_t *) pmemobj_direct(bp->log_segment_idx_oid_), log_segment_idx_, segment_count * sizeof(int32_t));
    flush_clwb_nolog((int32_t *) pmemobj_direct(bp->log_segment_idx_oid_), segment_count * sizeof(int32_t));

    memcpy((int64_t *) pmemobj_direct(bp->segment_edges_actual_oid_), segment_edges_actual,
           sizeof(int64_t) * segment_count * 2);
    flush_clwb_nolog((int64_t *) pmemobj_direct(bp->segment_edges_actual_oid_), sizeof(int64_t) * segment_count * 2);

    memcpy((int64_t *) pmemobj_direct(bp->segment_edges_total_oid_), segment_edges_total,
           sizeof(int64_t) * segment_count * 2);
    flush_clwb_nolog((int64_t *) pmemobj_direct(bp->segment_edges_total_oid_), sizeof(int64_t) * segment_count * 2);

    bp->num_vertices = num_vertices;  // Number of vertices
    flush_clwb_nolog(&bp->num_vertices, sizeof(int64_t));

    bp->num_edges_ = num_edges_;  // Number of edges
    flush_clwb_nolog(&bp->num_edges_, sizeof(int64_t));

    bp->elem_capacity = elem_capacity;
    flush_clwb_nolog(&bp->elem_capacity, sizeof(int64_t));

    bp->segment_count = segment_count;
    flush_clwb_nolog(&bp->segment_count, sizeof(int64_t));

    bp->segment_size = segment_size;
    flush_clwb_nolog(&bp->segment_size, sizeof(int64_t));

    bp->tree_height = tree_height;
    flush_clwb_nolog(&bp->tree_height, sizeof(int64_t));

    // write flag indicating data has been backed up properly before shutting down
    bp->backed_up_ = true;
    flush_clwb_nolog(&bp->backed_up_, sizeof(bool));

    ReleaseResources();
  }

  CSRGraph(const char *file, const EdgeList &edge_list, bool is_directed, int64_t n_edges, int64_t n_vertices) {
    bool is_new = false;
    num_threads = omp_get_max_threads();

    /* file already exists */
    if (file_exists(file) == 0) {
      if ((pop = pmemobj_open(file, LAYOUT_NAME)) == NULL) {
        fprintf(stderr, "[%s]: FATAL: pmemobj_open error: %s\n", __FUNCTION__, pmemobj_errormsg());
        exit(0);
      }
    } else {
      if ((pop = pmemobj_create(file, LAYOUT_NAME, DB_POOL_SIZE, CREATE_MODE_RW)) == NULL) {
        fprintf(stderr, "[%s]: FATAL: pmemobj_create error: %s\n", __FUNCTION__, pmemobj_errormsg());
        exit(0);
      }
      is_new = true;
    }

    base_oid = pmemobj_root(pop, sizeof(struct Base));
    bp = (struct Base *) pmemobj_direct(base_oid);
    check_sanity(bp);

    // newly created file
    if (is_new) {
      // ds initialization (for dram-domain)
      num_edges_ = n_edges;
      num_vertices = n_vertices;
      max_valid_vertex_id = n_vertices;
      directed_ = is_directed;
      compute_capacity();

      // array-based compete tree structure
      segment_edges_actual = (int64_t *) calloc(segment_count * 2, sizeof(int64_t));
      segment_edges_total = (int64_t *) calloc(segment_count * 2, sizeof(int64_t));

      tree_height = floor_log2(segment_count);
      delta_up = (up_0 - up_h) / tree_height;
      delta_low = (low_h - low_0) / tree_height;

      // ds initialization (for pmem-domain)
      bp->pool_uuid_lo = base_oid.pool_uuid_lo;
      bp->num_vertices = num_vertices;
      bp->num_edges_ = num_edges_;
      bp->directed_ = directed_;
      bp->elem_capacity = elem_capacity;
      bp->segment_count = segment_count;
      bp->segment_size = segment_size;
      bp->tree_height = tree_height;

      // allocate memory for vertices and edges in pmem-domain
      if (pmemobj_zalloc(pop, &bp->vertices_oid_, num_vertices * sizeof(struct vertex_element), VERTEX_TYPE)) {
        fprintf(stderr, "[%s]: FATAL: vertex array allocation failed: %s\n", __func__, pmemobj_errormsg());
        abort();
      }

      if (pmemobj_zalloc(pop, &bp->edges_oid_, elem_capacity * sizeof(DestID_), EDGE_TYPE)) {
        fprintf(stderr, "[%s]: FATAL: edge array allocation failed: %s\n", __func__, pmemobj_errormsg());
        abort();
      }

      if (pmemobj_zalloc(pop, &bp->log_segment_oid_, segment_count * MAX_LOG_ENTRIES * sizeof(struct LogEntry),
                         SEG_LOG_PTR_TYPE)) {
        fprintf(stderr, "[%s]: FATAL: per-segment log array allocation failed: %s\n", __func__, pmemobj_errormsg());
        abort();
      }

      if (pmemobj_zalloc(pop, &bp->ulog_oid_, num_threads * MAX_ULOG_ENTRIES * sizeof(DestID_), ULOG_PTR_TYPE)) {
        fprintf(stderr, "[%s]: FATAL: u-log array allocation failed: %s\n", __func__, pmemobj_errormsg());
        abort();
      }

      if (pmemobj_zalloc(pop, &bp->oplog_oid_, num_threads * sizeof(int64_t), OPLOG_PTR_TYPE)) {
        fprintf(stderr, "[%s]: FATAL: op-log array allocation failed: %s\n", __func__, pmemobj_errormsg());
        abort();
      }

      if (pmemobj_zalloc(pop, &bp->log_segment_idx_oid_, segment_count * sizeof(int32_t), SEG_LOG_IDX_TYPE)) {
        fprintf(stderr, "[%s]: FATAL: per-segment log index allocation failed: %s\n", __func__, pmemobj_errormsg());
        abort();
      }

      if (pmemobj_zalloc(pop, &bp->segment_edges_actual_oid_, sizeof(int64_t) * segment_count * 2,
                         PMA_TREE_META_TYPE)) {
        fprintf(stderr, "[%s]: FATAL: pma metadata allocation failed: %s\n", __func__, pmemobj_errormsg());
        abort();
      }

      if (pmemobj_zalloc(pop, &bp->segment_edges_total_oid_, sizeof(int64_t) * segment_count * 2, PMA_TREE_META_TYPE)) {
        fprintf(stderr, "[%s]: FATAL: pma metadata allocation failed: %s\n", __func__, pmemobj_errormsg());
        abort();
      }

      flush_clwb_nolog(bp, sizeof(struct Base));

      // retrieving pmem-pointer from pmem-oid (for pmem domain)
      edges_ = (DestID_ *) pmemobj_direct(bp->edges_oid_);
      vertices_ = (struct vertex_element *) malloc(num_vertices * sizeof(struct vertex_element));
      memcpy(vertices_, (struct vertex_element *) pmemobj_direct(bp->vertices_oid_),
             num_vertices * sizeof(struct vertex_element));

      log_base_ptr_ = (struct LogEntry *) pmemobj_direct(bp->log_segment_oid_);
      log_ptr_ = (struct LogEntry **) malloc(segment_count * sizeof(struct LogEntry *));  // 8-byte

      // save pointer in the log_ptr_[sid]
      for (int sid = 0; sid < segment_count; sid += 1) {
        log_ptr_[sid] = (struct LogEntry *) (log_base_ptr_ + (sid * MAX_LOG_ENTRIES));
      }
      log_segment_idx_ = (int32_t *) calloc(segment_count, sizeof(int32_t));

      ulog_base_ptr_ = (DestID_ *) pmemobj_direct(bp->ulog_oid_);
      ulog_ptr_ = (DestID_ **) malloc(num_threads * sizeof(DestID_ *)); // 8-byte

      // save pointer in the ulog_ptr_[tid]
      for (int tid = 0; tid < num_threads; tid += 1) {
        /// assignment of per-seg-edge-log from a single large pre-allocated log
        ulog_ptr_[tid] = (DestID_ *) (ulog_base_ptr_ + (tid * MAX_ULOG_ENTRIES));
      }

      oplog_ptr_ = (int64_t *) pmemobj_direct(bp->oplog_oid_);

      // leaf segment concurrency primitives
      leaf_segments = new PMALeafSegment[segment_count];

      /// insert base-graph edges
      Timer t;
      t.Start();
      insert(edge_list);
      t.Stop();
      cout << "base graph insert time: " << t.Seconds() << endl;

      bp->backed_up_ = false;
      flush_clwb_nolog(&bp->backed_up_, sizeof(bool));
    } else {
      Timer t_reboot;
      t_reboot.Start();

      cout << "how was last backup? Good? " << bp->backed_up_ << endl;

      /// ds initialization (for dram-domain)
      elem_capacity = bp->elem_capacity;
      segment_count = bp->segment_count;
      segment_size = bp->segment_size;
      tree_height = bp->tree_height;

      num_vertices = bp->num_vertices;
      num_edges_ = bp->num_edges_;
      avg_degree = ceil_div(num_edges_, num_vertices);
      directed_ = bp->directed_;

      // array-based compete tree structure
      segment_edges_actual = (int64_t *) calloc(segment_count * 2, sizeof(int64_t));
      segment_edges_total = (int64_t *) calloc(segment_count * 2, sizeof(int64_t));

      delta_up = (up_0 - up_h) / tree_height;
      delta_low = (low_h - low_0) / tree_height;

      // retrieving pmem-pointer from pmem-oid (for pmem domain)
      edges_ = (DestID_ *) pmemobj_direct(bp->edges_oid_);
      vertices_ = (struct vertex_element *) malloc(num_vertices * sizeof(struct vertex_element));

      log_base_ptr_ = (struct LogEntry *) pmemobj_direct(bp->log_segment_oid_);
      log_ptr_ = (struct LogEntry **) malloc(segment_count * sizeof(struct LogEntry *));  // 8-byte
      log_segment_idx_ = (int32_t *) malloc(segment_count * sizeof(int32_t)); // 4-byte

      for (int sid = 0; sid < segment_count; sid += 1) {
        // save pointer in the log_ptr_[sid]
        log_ptr_[sid] = (struct LogEntry *) (log_base_ptr_ + (sid * MAX_LOG_ENTRIES));
      }

      // last shutdown was properly backed up
      if (bp->backed_up_) {
        memcpy(vertices_, (struct vertex_element *) pmemobj_direct(bp->vertices_oid_),
               num_vertices * sizeof(struct vertex_element));
        memcpy(log_segment_idx_, (int32_t *) pmemobj_direct(bp->log_segment_idx_oid_),
               segment_count * sizeof(int32_t));  // 4-byte
        memcpy(segment_edges_actual, (int64_t *) pmemobj_direct(bp->segment_edges_actual_oid_),
               sizeof(int64_t) * segment_count * 2); // 8-byte
        memcpy(segment_edges_total, (int64_t *) pmemobj_direct(bp->segment_edges_total_oid_),
               sizeof(int64_t) * segment_count * 2); // 8-byte
      } else {
        Timer t;
        t.Start();
        recovery();
        t.Stop();
        cout << "graph recovery time: " << t.Seconds() << endl;

        // persisting number of edges
        bp->num_edges_ = num_edges_;
        flush_clwb_nolog(&bp->num_edges_, sizeof(int64_t));
      }

      t_reboot.Stop();
      cout << "graph reboot time: " << t_reboot.Seconds() << endl;
    }
  }

  void recovery() {
    /// rebuild vertices_ by scanning edges_
    /// recount num_edges_ and save it to bp->num_edges_
    /// reconstruct segment_edges_actual, segment_edges_total
    num_edges_ = 0;

    // need to translate the vertex-ids of the graphs which have vertex-id 0
    int64_t st_idx = 0;
    int32_t seg_id = 0;
    NodeID_ vid = 0;
    NodeID_ mx_vid = 0;
    if (edges_[st_idx].v == 0) {
      seg_id = get_segment_id(0);
      vertices_[0].index = st_idx;
      vertices_[0].degree = 1;
      st_idx += 1;
      segment_edges_actual[seg_id] += 1;
      num_edges_ += 1;
      while (edges_[st_idx].v != 0) {
        st_idx += 1;
        vertices_[0].degree += 1;
        segment_edges_actual[seg_id] += 1;
        num_edges_ += 1;
      }
    }
    for (int64_t i = st_idx; i < elem_capacity; i += 1) {
      if (edges_[i].v < 0) {
        vid = -edges_[i].v;
        mx_vid = max(mx_vid, vid);
        seg_id = get_segment_id(vid);
        vertices_[vid].index = i;
        vertices_[vid].degree = 1;
        segment_edges_actual[seg_id] += 1;
        num_edges_ += 1;
      }
      if (edges_[i].v > 0) {
        vertices_[vid].degree += 1;
        segment_edges_actual[seg_id] += 1;
        num_edges_ += 1;
      }
    }

    cout << "max vertex-id retrieved: " << mx_vid << endl;
    cout << "segment_count: " << segment_count << endl;

    // reconstruct segment_edges_total
    recount_segment_total();

    /// initialize log_segment_idx_ by scanning the log_ptr_[sid]
    int32_t total_log_entries = 0;
    for (int sid = 0; sid < segment_count; sid++) {
      for (int i = 0; i < MAX_LOG_ENTRIES; i += 1) {
        if (log_ptr_[sid][i].u && log_ptr_[sid][i].v && log_ptr_[sid][i].prev_offset) {
          log_segment_idx_[sid] = i;
        } else break;
      }
      total_log_entries += log_segment_idx_[sid];
    }
    cout << "total log entries found: " << total_log_entries << endl;
  }

  bool directed() const {
    return directed_;
  }

  int64_t num_nodes() const {
    return num_vertices;
  }

  int64_t num_edges() const {
    return num_edges_;
  }

  int64_t num_edges_directed() const {
    return directed_ ? num_edges_ : 2 * num_edges_;
  }

  int64_t out_degree(NodeID_ v) const {
    return vertices_[v].degree - 1;
  }

  int64_t in_degree(NodeID_ v) const {
    static_assert(MakeInverse, "Graph inversion disabled but reading inverse");
    return vertices_[v].degree - 1;
  }

  Neighborhood out_neigh(NodeID_ n, OffsetT start_offset = 0) const {
    int32_t onseg_edges = vertices_[n].degree;
    int64_t next_vertex_boundary = (n >= num_vertices - 1) ? (elem_capacity - 1) : vertices_[n + 1].index - 1;
    if (vertices_[n].offset != -1) onseg_edges = next_vertex_boundary - vertices_[n].index + 1;
    return Neighborhood(edges_, (struct vertex_element *) (vertices_ + n), start_offset + 1, log_ptr_[n / segment_size],
                        onseg_edges);
  }

  Neighborhood in_neigh(NodeID_ n, OffsetT start_offset = 0) const {
    static_assert(MakeInverse, "Graph inversion disabled but reading inverse");
    int32_t onseg_edges = vertices_[n].degree;
    int64_t next_vertex_boundary = (n >= num_vertices - 1) ? (elem_capacity - 1) : vertices_[n + 1].index - 1;
    if (vertices_[n].offset != -1) onseg_edges = next_vertex_boundary - vertices_[n].index + 1;
    return Neighborhood(edges_, (struct vertex_element *) (vertices_ + n), start_offset + 1, log_ptr_[n / segment_size],
                        onseg_edges);
  }

  void PrintStats() const {
    std::cout << "Graph has " << num_vertices << " nodes and "
              << num_edges_ << " ";
    if (!directed_)
      std::cout << "un";
    std::cout << "directed edges for degree: ";
    std::cout << num_edges_ / num_vertices << std::endl;
  }

  void PrintTopology() const {
    for (NodeID_ i = 0; i < num_vertices; i++) {
      if (i && i % 10000000 == 0) std::cout << i / 1000000 << " million vertices processed." << endl;
      for (DestID_ j: out_neigh(i)) {
        volatile DestID_ x = j;
      }
    }
  }

  void PrintTopology(NodeID_ src) const {
    std::cout << vertices_[src].index << " " << vertices_[src].degree << " " << vertices_[src].offset << std::endl;
    std::cout << src << ": ";
    for (DestID_ j: out_neigh(src)) {
      std::cout << j << " ";
    }
    std::cout << std::endl;
  }

  pvector<SGOffset> VertexOffsets(bool in_graph = false) const {
    pvector<SGOffset> offsets(num_vertices + 1);
    for (NodeID_ n = 0; n < num_vertices + 1; n++)
      offsets[n] = vertices_[n].index - vertices_[0].index;
    return offsets;
  }

  Range<NodeID_> vertices() const {
    return Range<NodeID_>(num_nodes());
  }

  inline void print_vertices() {
    for (int i = 0; i < num_vertices; i++) {
      printf("(%d)|%llu,%d| ", i, vertices_[i].index, vertices_[i].degree);
    }
    printf("\n");
  }

  inline void print_vertices(int32_t from, int32_t to) {
    for (int32_t i = from; i < to; i++) {
      printf("(%d)|%llu,%d| ", i, vertices_[i].index, vertices_[i].degree);
    }
    printf("\n");
  }

  inline void print_vertices(int32_t segment_id) {
    cout << "Print Vertices: ";
    int32_t from = (segment_id) * segment_size;
    int32_t to = (segment_id + 1) * segment_size;
    cout << from << " " << to << endl;
    print_vertices(from, to);
  }

  inline void print_vertex(int32_t vid) {
    cout << "vertex-id: " << vid << "# index: " << vertices_[vid].index;
    cout << ", degree: " << vertices_[vid].degree << ", log-offset: " << vertices_[vid].offset << endl;
  }

  inline void print_edges() {
    cout << "Print Edges: ";
    for (int i = 0; i < elem_capacity; i++) {
      printf("%u ", edges_[i].v);
    }
    printf("\n");
  }

  inline void print_segment() {
    cout << "Print Segments: ";
    for (int i = 0; i < segment_count * 2; i++) {
      printf("(%d)|%llu / %llu| ", i, segment_edges_actual[i], segment_edges_total[i]);
    }
    printf("\n");
  }

  inline void print_segment(int segment_id) {
    printf("Segment (%d): |%llu / %llu|\n", segment_id, segment_edges_actual[segment_id],
           segment_edges_total[segment_id]);
  }

  inline void print_pma_meta() {
    cout << "max_size: " << max_size << ", num_edges: " << num_edges_ << ", num_vertices: " << num_vertices
         << ", avg_degree: " << avg_degree << ", elem_capacity: " << elem_capacity << endl;
    cout << "segment_count: " << segment_count << ", segment_size: " << segment_size << ", tree_height: " << tree_height
         << endl;
  }

  inline void edge_list_boundary_sanity_checker() {
    for (int32_t curr_vertex = 1; curr_vertex < num_vertices; curr_vertex += 1) {
      if (vertices_[curr_vertex - 1].index + vertices_[curr_vertex - 1].degree > vertices_[curr_vertex].index) {
        cout << "**** Invalid edge-list boundary found at vertex-id: " << curr_vertex - 1 << " index: "
             << vertices_[curr_vertex - 1].index;
        cout << " degree: " << vertices_[curr_vertex - 1].degree << " next vertex start at: "
             << vertices_[curr_vertex].index << endl;
      }
      assert(vertices_[curr_vertex - 1].index + vertices_[curr_vertex - 1].degree <= vertices_[curr_vertex].index &&
             "Invalid edge-list boundary found!");
    }
    assert(vertices_[num_vertices - 1].index + vertices_[num_vertices - 1].degree <= elem_capacity &&
           "Invalid edge-list boundary found!");
  }

  /*****************************************************************************
   *                                                                           *
   *   PMA                                                                     *
   *                                                                           *
   *****************************************************************************/
  /// Double the size of the "edges_" array
  void resize_V1() {
    elem_capacity *= 2;
    int64_t gaps = elem_capacity - num_edges_;
    int64_t *new_indices = calculate_positions(0, num_vertices, gaps, num_edges_);

    PMEMoid new_edges_oid_ = OID_NULL;
    if (pmemobj_zalloc(pop, &new_edges_oid_, elem_capacity * sizeof(DestID_), EDGE_TYPE)) {
      fprintf(stderr, "[%s]: FATAL: edge array allocation failed: %s\n", __func__, pmemobj_errormsg());
      abort();
    }
    DestID_ *new_edges_ = (DestID_ *) pmemobj_direct(new_edges_oid_);

    int64_t write_index;
    int32_t curr_off, curr_seg, onseg_num_edges;
    int64_t next_vertex_boundary;
    for (NodeID_ vi = 0; vi < num_vertices; vi += 1) {
      next_vertex_boundary = (vi == num_vertices - 1) ? (elem_capacity - 1) : vertices_[vi + 1].index - 1;

      // count on-segment number of edges for vertex-vi
      if (vertices_[vi].offset != -1) onseg_num_edges = next_vertex_boundary - vertices_[vi].index + 1;
      else onseg_num_edges = vertices_[vi].degree;

      memcpy((new_edges_ + new_indices[vi]), (edges_ + (vertices_[vi].index)), onseg_num_edges * sizeof(DestID_));

      // if vertex-vi have edges in the log, move it to on-segment
      if (vertices_[vi].offset != -1) {
        curr_off = vertices_[vi].offset;
        curr_seg = get_segment_id(vi) - segment_count;

        write_index = new_indices[vi] + vertices_[vi].degree - 1;
        while (curr_off != -1) {
          new_edges_[write_index].v = log_ptr_[curr_seg][curr_off].v;
          curr_off = log_ptr_[curr_seg][curr_off].prev_offset;
          write_index--;
        }
      }

      // update the index to the new position
      vertices_[vi].index = new_indices[vi];
      vertices_[vi].offset = -1;
    }

    flush_clwb_nolog(new_edges_, elem_capacity * sizeof(DestID_));

    pmemobj_free(&bp->edges_oid_);
    bp->edges_oid_ = OID_NULL;
    bp->edges_oid_ = new_edges_oid_;
    flush_clwb_nolog(&bp->edges_oid_, sizeof(PMEMoid));

    edges_ = (DestID_ *) pmemobj_direct(bp->edges_oid_);
    recount_segment_total();
    free(new_indices);
    new_indices = nullptr;

    bp->elem_capacity = elem_capacity;
    flush_clwb_nolog(&bp->elem_capacity, sizeof(int64_t));

    int32_t st_seg = segment_count, nd_seg = 2 * segment_count;
    for (int32_t i = st_seg; i < nd_seg; i += 1) {
      release_log(i - segment_count);
    }
  }

  inline int32_t get_segment_id(int32_t vertex_id) {
    return (vertex_id / segment_size) + segment_count;
  }

  void reconstruct_pma_tree() {
    recount_segment_actual();
    recount_segment_total();
  }

  // Expected to run in single thread
  void recount_segment_actual() {
    // count the size of each segment in the tree
    num_edges_ = 0;
    memset(segment_edges_actual, 0, sizeof(int64_t) * segment_count * 2);
    for (int seg_id = 0; seg_id < segment_count; seg_id++) {
      int32_t start_vertex = (seg_id * segment_size);
      int32_t end_vertex = min(((seg_id + 1) * segment_size), num_vertices);

      int32_t segment_actual_p = 0;
      for (int32_t vid = start_vertex; vid < end_vertex; vid += 1) {
        segment_actual_p += vertices_[vid].degree;
        num_edges_ += vertices_[vid].degree;
      }

      int32_t j = seg_id + segment_count;  //tree leaves
      segment_edges_actual[j] = segment_actual_p;
    }
  }

  // Expected to run in single thread
  void recount_segment_total() {
    // count the size of each segment in the tree
    memset(segment_edges_total, 0, sizeof(int64_t) * segment_count * 2);
    for (int i = 0; i < segment_count; i++) {
      int64_t next_starter = (i == (segment_count - 1)) ? (elem_capacity) : vertices_[(i + 1) * segment_size].index;
      int64_t segment_total_p = next_starter - vertices_[i * segment_size].index;
      int32_t j = i + segment_count;  //tree leaves
      segment_edges_total[j] = segment_total_p;
    }
  }

  // Expected to run in single thread
  void recount_segment_total(int32_t start_vertex, int32_t end_vertex) {
    int32_t start_seg = get_segment_id(start_vertex) - segment_count;
    int32_t end_seg = get_segment_id(end_vertex) - segment_count;

    for (int32_t i = start_seg; i < end_seg; i += 1) {
      int64_t next_starter = (i == (segment_count - 1)) ? (elem_capacity) : vertices_[(i + 1) * segment_size].index;
      int64_t segment_total_p = next_starter - vertices_[i * segment_size].index;
      int32_t j = i + segment_count;  //tree leaves
      segment_edges_total[j] = segment_total_p;
    }
  }

  /// Insert base-graph. Assume all the edges of a vertex comes together (COO format).
  void insert(const EdgeList &edge_list) {
    NodeID_ last_vid = -1;
    int64_t ii = 0;
    for (int i = 0; i < num_edges_; i++) {
      int32_t t_src = edge_list[i].u;
      int32_t t_dst = edge_list[i].v.v;

      int32_t t_segment_id = get_segment_id(t_src);

      int32_t t_degree = vertices_[t_src].degree;
      if (t_degree == 0) {
        if (t_src != last_vid + 1) {
          for (NodeID_ vid = last_vid + 1; vid < t_src; vid += 1) {
            edges_[ii].v = -vid;
            vertices_[vid].degree = 1;
            vertices_[vid].index = ii;
            ii += 1;

            segment_edges_actual[get_segment_id(vid)] += 1;
          }
        }
        edges_[ii].v = -t_src;
        vertices_[t_src].degree = 1;
        vertices_[t_src].index = ii;
        ii += 1;

        segment_edges_actual[t_segment_id] += 1;
        last_vid = t_src;
      }

      edges_[ii].v = t_dst;
      ii += 1;
      vertices_[t_src].degree += 1;

      // update the actual edges in each segment of the tree
      segment_edges_actual[t_segment_id] += 1;
    }

    // correct the starting of the vertices with 0 degree in the base-graph
    for (int i = 0; i < num_vertices; i++) {
      if (vertices_[i].degree == 0) {
        assert(i > last_vid && "Something went wrong! We should not leave some zero-degree vertices before last_vid!");
        edges_[ii].v = -i;
        vertices_[i].degree = 1;
        vertices_[i].index = ii;
        ii += 1;

        segment_edges_actual[get_segment_id(i)] += 1;
      }
      vertices_[i].offset = -1;
    }

    num_edges_ += num_vertices;
    spread_weighted(0, num_vertices);
    flush_clwb_nolog(edges_, sizeof(DestID_) * elem_capacity);
  }

  bool have_space_onseg(int32_t src, int64_t loc) {
    if ((src == (num_vertices - 1) && elem_capacity > loc)
        || (src < (num_vertices - 1) && vertices_[src + 1].index > loc))
      return true;
    return false;
  }

  /// Insert dynamic-graph edges.
  void insert(int32_t src, int32_t dst) {
    int32_t current_segment = get_segment_id(src);
    int32_t left_segment = current_segment - segment_count, right_segment = current_segment - segment_count + 1;

    // taking the lock for the current segment
    unique_lock <mutex> ul(leaf_segments[current_segment - segment_count].lock);
    leaf_segments[current_segment - segment_count].wait_for_rebalance(ul);

    // insert the current edge
    do_insertion(src, dst, current_segment);

    int32_t left_index = src, right_index = src;
    int32_t window = current_segment;

    double density = (double) (segment_edges_actual[current_segment]) / (double) segment_edges_total[current_segment];
    int32_t height = 0;
    if (density >= up_0) {
      // unlock the current segment
      leaf_segments[current_segment - segment_count].on_rebalance += 1;
      ul.unlock();

      double up_height = up_0 - (height * delta_up);
      pair <int64_t, int64_t> seg_meta;

      while (window > 0) {
        // Go one level up in our conceptual PMA tree
        window /= 2;
        height += 1;

        int32_t window_size = segment_size * (1 << height);
        left_index = (src / window_size) * window_size;
        right_index = min(left_index + window_size, num_vertices);

        left_segment = get_segment_id(left_index) - segment_count;
        right_segment = get_segment_id(right_index) - segment_count;

        seg_meta = lock_in_batch(left_segment, right_segment, current_segment - segment_count);
        up_height = up_0 - (height * delta_up);
        density = (double) seg_meta.first / (double) seg_meta.second;

        if (density < up_height) {
          break;
        } else {
          unlock_in_batch(left_segment, right_segment, current_segment - segment_count);
        }
      }

      if (!height) {
        cout << "This should not happen! Aborting!" << endl;
        exit(-1);
      }

      if (window) {
        // Found a window within threshold
        int32_t window_size = segment_size * (1 << height);
        left_index = (src / window_size) * window_size;
        right_index = min(left_index + window_size, num_vertices);

        left_segment = get_segment_id(left_index) - segment_count;
        right_segment = get_segment_id(right_index) - segment_count;

        density = (double) (segment_edges_actual[current_segment]) / (double) segment_edges_total[current_segment];
        if (density >= up_0) rebalance_weighted(left_index, right_index, seg_meta.first, seg_meta.second, src, dst);
      }
      else {
        // Rebalance not possible without increasing the underlying array size, need to resize the size of "edges_" array
        left_segment = 0;
        right_segment = segment_count;

        density = (double) (segment_edges_actual[current_segment]) / (double) segment_edges_total[current_segment];
        if (density < up_0) return;
        seg_meta = lock_in_batch(left_segment, right_segment, current_segment - segment_count);
        resize_V1();
      }

      unlock_in_batch(left_segment, right_segment);
    }
    else {
      ul.unlock();
    }
  }

  inline void do_insertion(int32_t src, int32_t dst, int32_t src_segment) {
    int64_t loc = vertices_[src].index + vertices_[src].degree;

    // if there is empty space, make the insertion
    if (have_space_onseg(src, loc)) {
      edges_[loc].v = dst;
      flush_clwb_nolog(&edges_[loc], sizeof(DestID_));
    }
    else {  // else add it to the log
      // check if the log is full
      if (log_segment_idx_[src_segment - segment_count] >= MAX_LOG_ENTRIES) {
        int32_t left_index = (src / segment_size) * segment_size;
        int32_t right_index = min(left_index + segment_size, num_vertices);

        rebalance_weighted(left_index, right_index,
                           segment_edges_actual[src_segment], segment_edges_total[src_segment],
                           src, dst);
      }

      loc = vertices_[src].index + vertices_[src].degree;
      if (have_space_onseg(src, loc)) {
        edges_[loc].v = dst;
        flush_clwb_nolog(&edges_[loc], sizeof(DestID_));
      } else {
        insert_into_log(src_segment - segment_count, src, dst);
      }
    }

    // updating metadata
    vertices_[src].degree += 1;
    segment_edges_actual[src_segment] += 1;
    int64_t old_val, new_val;
    do {
      old_val = num_edges_;
      new_val = old_val + 1;
    } while (!compare_and_swap(num_edges_, old_val, new_val));
  }

  /// Insert an edge into edge-log.
  inline void insert_into_log(int32_t segment_id, int32_t src, int32_t dst) {
    assert(log_segment_idx_[segment_id] < MAX_LOG_ENTRIES &&
           "logs are full, need to perform a rebalance first");
    assert(vertices_[src].offset < MAX_LOG_ENTRIES &&
           "vertex offset is beyond the log range, should not happen this for sure!");
    assert((src >= (segment_id * segment_size) && src < ((segment_id * segment_size) + segment_size)) &&
           "src vertex is not for this segment-id");

    // insert into log
    struct LogEntry *log_ins_ptr = (struct LogEntry *) (log_ptr_[segment_id] + log_segment_idx_[segment_id]);
    log_ins_ptr->u = src;
    log_ins_ptr->v = dst;
    log_ins_ptr->prev_offset = vertices_[src].offset;

    flush_clwb_nolog(log_ins_ptr, sizeof(struct LogEntry));

    vertices_[src].offset = log_segment_idx_[segment_id];
    log_segment_idx_[segment_id] += 1;
  }

  /// Print all the edges of vertex @vid stored in the per-seg-logs.
  inline void print_per_vertex_log(int32_t vid) {
    int32_t segment_id = get_segment_id(vid) - segment_count;
    int32_t current_offset = vertices_[vid].offset;
    if (current_offset == -1) {
      cout << "vertex " << vid << ": do not have any log" << endl;
      return;
    }
    cout << "vertex " << vid << ":";
    while (current_offset != -1) {
      cout << " " << log_ptr_[segment_id][current_offset].v;
      current_offset = log_ptr_[segment_id][current_offset].prev_offset;
    }
    cout << endl;
  }

  /// Print all the edges of segment @segment_id stored in the per-seg-logs.
  inline void print_log(int32_t segment_id) {
    if (log_segment_idx_[segment_id] == 0) {
      cout << "segment " << segment_id << ": do not have any log" << endl;
      return;
    }
    cout << "vertex range from: " << segment_id * segment_size << " to: " << (segment_id * segment_size) + segment_size
         << endl;
    cout << "segment " << segment_id << ":";
    for (int i = 0; i < log_segment_idx_[segment_id]; i += 1) {
      cout << " <" << log_ptr_[segment_id][i].u << " " << log_ptr_[segment_id][i].v << " "
           << log_ptr_[segment_id][i].prev_offset << ">";
    }
    cout << endl;
  }

  /// Releasing per-seg-logs by mem-setting to 0.
  inline void release_log(int32_t segment_id) {
    if (log_segment_idx_[segment_id] == 0) return;
    memset(log_ptr_[segment_id], 0, sizeof(struct LogEntry) * log_segment_idx_[segment_id]);

    flush_clwb_nolog(&log_ptr_[segment_id], sizeof(struct LogEntry) * log_segment_idx_[segment_id]);
    log_segment_idx_[segment_id] = 0;
  }

  /// Calculate PMA-parameters.
  void compute_capacity() {
    segment_size = ceil_log2(num_vertices); // Ideal segment size
    segment_count = ceil_div(num_vertices, segment_size); // Ideal number of segments

    // The number of segments has to be a power of 2, though.
    segment_count = hyperfloor(segment_count);
    // Update the segment size accordingly
    segment_size = ceil_div(num_vertices, segment_count);

    num_vertices = segment_count * segment_size;
    avg_degree = ceil_div(num_edges_, num_vertices);

    elem_capacity = (num_edges_ + num_vertices) * max_sparseness;
  }

  /// Spread the edges based on the degree of source vertices.
  /// Here, end_vertex is excluded, and start_vertex is expected to be 0
  void spread_weighted(int32_t start_vertex, int32_t end_vertex) {
    assert(start_vertex == 0 && "start-vertex is expected to be 0 here.");
    int64_t gaps = elem_capacity - num_edges_;
    int64_t *new_positions = calculate_positions(start_vertex, end_vertex, gaps, num_edges_);

    int64_t read_index, write_index, curr_degree;
    for (int32_t curr_vertex = end_vertex - 1; curr_vertex > start_vertex; curr_vertex -= 1) {
      curr_degree = vertices_[curr_vertex].degree;
      read_index = vertices_[curr_vertex].index + curr_degree - 1;
      write_index = new_positions[curr_vertex] + curr_degree - 1;

      if (write_index < read_index) {
        cout << "current-vertex: " << curr_vertex << ", read: " << read_index << ", write: " << write_index
             << ", degree: " << curr_degree << endl;
      }
      assert(write_index >= read_index && "index anomaly occurred while spreading elements");

      for (int i = 0; i < curr_degree; i++) {
        edges_[write_index] = edges_[read_index];
        write_index--;
        read_index--;
      }

      vertices_[curr_vertex].index = new_positions[curr_vertex];
    }

    // note: we do not need to flush the data in PMEM from here, it is managed from the caller function
    free(new_positions);
    new_positions = nullptr;
    recount_segment_total();
  }

  /// Take the locks of the segments [@left_segment, @right_segment); exclude @src_segment to increment the CV counter
  inline pair <int64_t, int64_t> lock_in_batch(int32_t left_segment, int32_t right_segment, int32_t src_segment) {
    pair <int64_t, int64_t> ret = make_pair(0l, 0l);
    int32_t old_val, new_val;
    for (int32_t seg_id = left_segment; seg_id < right_segment; seg_id += 1) {
      if (seg_id != src_segment) {
        do {
          old_val = leaf_segments[seg_id].on_rebalance;
          new_val = old_val + 1;
        } while (!compare_and_swap(leaf_segments[seg_id].on_rebalance, old_val, new_val));
      }

      leaf_segments[seg_id].lock.lock();
      ret.first += segment_edges_actual[seg_id + segment_count];
      ret.second += segment_edges_total[seg_id + segment_count];
    }
    return ret;
  }

  /// Unlock the locks of the segments [@left_segment, @right_segment)
  inline void unlock_in_batch(int32_t left_segment, int32_t right_segment) {
    int32_t rebal_depend;
    int32_t old_val, new_val;
    for (int32_t seg_id = right_segment - 1; seg_id >= left_segment; seg_id -= 1) {
      do {
        old_val = leaf_segments[seg_id].on_rebalance;
        new_val = old_val - 1;
      } while (!compare_and_swap(leaf_segments[seg_id].on_rebalance, old_val, new_val));

      rebal_depend = leaf_segments[seg_id].on_rebalance;
      leaf_segments[seg_id].lock.unlock();
      if (!rebal_depend) leaf_segments[seg_id].cv.notify_all();
    }
  }

  /// Unlock the locks of the segments [@left_segment, @right_segment); exclude @src_segment to decrement the CV counter
  inline void unlock_in_batch(int32_t left_segment, int32_t right_segment, int32_t src_segment) {
    int32_t rebal_depend;
    int32_t old_val, new_val;
    for (int32_t seg_id = right_segment - 1; seg_id >= left_segment; seg_id -= 1) {
      if (seg_id != src_segment) {
        do {
          old_val = leaf_segments[seg_id].on_rebalance;
          new_val = old_val - 1;
        } while (!compare_and_swap(leaf_segments[seg_id].on_rebalance, old_val, new_val));
      }
      rebal_depend = leaf_segments[seg_id].on_rebalance;
      leaf_segments[seg_id].lock.unlock();
      if (!rebal_depend) leaf_segments[seg_id].cv.notify_all();
    }
  }

  /// Rebalance all the segments.
  void rebalance_all() {
    int32_t left_segment = get_segment_id(0) - segment_count;
    int32_t right_segment = get_segment_id(num_vertices) - segment_count;

    pair <int64_t, int64_t> seg_meta = lock_in_batch(left_segment, right_segment, -1);
    rebalance_weighted(0, num_vertices, seg_meta.first, seg_meta.second);
    unlock_in_batch(left_segment, right_segment);
  }

  /// Calculate the starting index of each vertex for [@start_vertex, @end_vertex) based on the degree of vertices
  /// @total_degree is the sum of degrees of [@start_vertex, @end_vertex)
  /// @gaps are the sum of empty spaces after the per-vertex neighbors of [@start_vertex, @end_vertex)
  int64_t *calculate_positions(int32_t start_vertex, int32_t end_vertex, int64_t gaps, int64_t total_degree) {
    int32_t size = end_vertex - start_vertex;
    int64_t *new_index = (int64_t *) calloc(size, sizeof(int64_t));
    total_degree += size;

    int64_t index_boundary = (end_vertex >= num_vertices) ? elem_capacity : vertices_[end_vertex].index;
    double index_d = vertices_[start_vertex].index;
    double step = ((double) gaps) / total_degree;  //per-edge step
    double precision_pos = 100000000.0;
    assert(((int64_t)(step * precision_pos)) > 0l && "fixed-precision is going to cause problem!");
    step = ((double) ((int64_t)(step * precision_pos))) / precision_pos;
    for (int i = start_vertex; i < end_vertex; i++) {
      new_index[i - start_vertex] = index_d;
      assert(new_index[i - start_vertex] + vertices_[i].degree <= index_boundary && "index calculation is wrong!");

      index_d = new_index[i - start_vertex];
      index_d += (vertices_[i].degree + (step * (vertices_[i].degree + 1)));
//      index_d += (vertices_[i].degree + (step * vertices_[i].degree));
    }

    return new_index;
  }

  void rebalance_weighted(int32_t start_vertex,
                          int32_t end_vertex,
                          int64_t used_space, int64_t total_space, int32_t src = -1, int32_t dst = -1) {
    int64_t from = vertices_[start_vertex].index;
    int64_t to = (end_vertex >= num_vertices) ? elem_capacity : vertices_[end_vertex].index;
    assert(to > from && "Invalid range found while doing weighted rebalance");
    int64_t capacity = to - from;

    assert(total_space == capacity && "Segment capacity is not matched with segment_edges_total");
    int64_t gaps = total_space - used_space;

    // calculate the future positions of the vertices_[i].index
    int32_t size = end_vertex - start_vertex;
    int64_t *new_index = calculate_positions(start_vertex, end_vertex, gaps, used_space);
    int64_t index_boundary = (end_vertex >= num_vertices) ? elem_capacity : vertices_[end_vertex].index;
    assert(new_index[size - 1] + vertices_[end_vertex - 1].degree <= index_boundary &&
           "Rebalance (weighted) index calculation is wrong!");

    rebalance_data_V1(start_vertex, end_vertex, new_index);

    free(new_index);
    new_index = nullptr;
    recount_segment_total(start_vertex, end_vertex);
  }

  /// Manage crash consistent log
  /// First, flush changes in the @edge_array for the interval [@flush_idx_st, @flush_idx_nd)
  /// Then, load data to ulog from the @edge_array for the range of [@load_idx_st, @load_idx_st + @MAX_ULOG_SIZE)
  /// Finally, make an entry in the oplog to track the @edge_array range saved in the ulog
  inline void
  load_into_ulog(int tid, int64_t load_idx_st, int32_t load_sz, int64_t flush_idx_st, int64_t flush_idx_nd) {
    // flush the last entries
    if (flush_idx_st != flush_idx_nd) {
      flush_clwb_nolog(&edges_[flush_idx_st], sizeof(DestID_) * (flush_idx_nd - flush_idx_st + 1));
    }
//    memcpy(ulog_ptr_[tid], edges_+load_idx_st, sizeof(DestID_) * MAX_ULOG_ENTRIES);
    memcpy(ulog_ptr_[tid], edges_ + load_idx_st, sizeof(DestID_) * load_sz);
    flush_clwb_nolog(&ulog_ptr_[tid], sizeof(DestID_) * MAX_ULOG_ENTRIES);
    oplog_ptr_[tid] = load_idx_st;
    flush_clwb_nolog(&oplog_ptr_[tid], sizeof(int64_t));
  }

  inline void
  rebalance_data_V1(int32_t start_vertex, int32_t end_vertex, int64_t *new_index, bool from_resize = false) {
    int tid = omp_get_thread_num();
    int32_t ii, jj;
    int32_t curr_vertex = start_vertex;
    int64_t read_index, last_read_index, write_index;
    int32_t curr_off, curr_seg, onseg_num_edges;
    int64_t next_vertex_boundary;
    int64_t ulog_st = vertices_[end_vertex].index, ulog_nd = vertices_[end_vertex].index;
    int32_t load_sz = 0;

    // loop over vertex
    while (curr_vertex < end_vertex) {
      for (ii = curr_vertex; ii < end_vertex; ii++) {
        // the following condition will give us the first vertex which have space to its right side
        if (new_index[ii - start_vertex] + vertices_[ii].degree <= vertices_[ii + 1].index) break;
      }

      if (ii == end_vertex) {
        ii -= 1;
      }

      next_vertex_boundary = (ii >= num_vertices - 1) ? (elem_capacity - 1) : vertices_[ii + 1].index - 1;
      // we will shift edges for source-vertex [curr_vertex to ii]
      for (jj = ii; jj >= curr_vertex; jj -= 1) {
        if (vertices_[jj].offset != -1) onseg_num_edges = next_vertex_boundary - vertices_[jj].index + 1;

        // on-segment: do left-shift
        if (new_index[jj - start_vertex] < vertices_[jj].index) {
          read_index = vertices_[jj].index;
          last_read_index = read_index + ((vertices_[jj].offset != -1) ? onseg_num_edges : vertices_[jj].degree);

          write_index = new_index[jj - start_vertex];

          while (read_index < last_read_index) {
            if (write_index < ulog_st || write_index >= ulog_nd) {
              load_sz = ((write_index + MAX_ULOG_ENTRIES) <= elem_capacity)
                        ? MAX_ULOG_ENTRIES : (elem_capacity - write_index);
              load_into_ulog(tid, write_index, load_sz, ulog_st, ulog_nd);
              ulog_st = write_index;
              ulog_nd = ((write_index + MAX_ULOG_ENTRIES) <= elem_capacity)
                        ? (write_index + MAX_ULOG_ENTRIES) : elem_capacity;
            }
            edges_[write_index] = edges_[read_index];
            write_index++;
            read_index++;
          }
        }
        // on-segment: do right-shift
        else if (new_index[jj - start_vertex] > vertices_[jj].index) {
          read_index = vertices_[jj].index + ((vertices_[jj].offset != -1)
                                              ? onseg_num_edges : vertices_[jj].degree) - 1;
          last_read_index = vertices_[jj].index;

          write_index = new_index[jj - start_vertex] + ((vertices_[jj].offset != -1)
                                                        ? onseg_num_edges : vertices_[jj].degree) - 1;

          while (read_index >= last_read_index) {
            if (write_index < ulog_st || write_index >= ulog_nd) {
              load_sz = ((write_index - MAX_ULOG_ENTRIES) >= 0) ? MAX_ULOG_ENTRIES : write_index;
              load_into_ulog(tid, write_index - MAX_ULOG_ENTRIES, load_sz, ulog_st, ulog_nd);
              ulog_st = ((write_index - MAX_ULOG_ENTRIES) >= 0) ? (write_index - MAX_ULOG_ENTRIES) : 0;
              ulog_nd = write_index;
            }
            edges_[write_index] = edges_[read_index];
            write_index--;
            read_index--;
          }
        }

        // if vertex-jj have edges in the log, move it to on-segment
        if (vertices_[jj].offset != -1) {
          curr_off = vertices_[jj].offset;
          curr_seg = get_segment_id(jj) - segment_count;

          write_index = new_index[jj - start_vertex] + vertices_[jj].degree - 1;
          while (curr_off != -1) {
            if (write_index < ulog_st || write_index >= ulog_nd) {
              load_sz = ((write_index - MAX_ULOG_ENTRIES) >= 0) ? MAX_ULOG_ENTRIES : write_index;
              load_into_ulog(tid, write_index - MAX_ULOG_ENTRIES, load_sz, ulog_st, ulog_nd);
              ulog_st = ((write_index - MAX_ULOG_ENTRIES) >= 0) ? (write_index - MAX_ULOG_ENTRIES) : 0;
              ulog_nd = write_index;
            }
            edges_[write_index].v = log_ptr_[curr_seg][curr_off].v;

            curr_off = log_ptr_[curr_seg][curr_off].prev_offset;
            write_index--;
          }
        }

        // update the index to the new position
        next_vertex_boundary = vertices_[jj].index - 1;
        vertices_[jj].index = new_index[jj - start_vertex];
        vertices_[jj].offset = -1;
      }
      curr_vertex = ii + 1;
    }
    // update log for the rebalance segments
    int32_t st_seg = get_segment_id(start_vertex), nd_seg = get_segment_id(end_vertex);
    for (int32_t i = st_seg; i < nd_seg; i += 1) {
      release_log(i - segment_count);
    }
  }

private:
  bool directed_;
  int64_t max_size = (1ULL << 56) - 1ULL;

  /* PMA constants */
  // Height-based (as opposed to depth-based) tree thresholds
  // Upper density thresholds
  static constexpr double up_h = 0.75;    // root
  static constexpr double up_0 = 1.00;    // leaves
  // Lower density thresholds
  static constexpr double low_h = 0.50;   // root
  static constexpr double low_0 = 0.25;   // leaves

  int8_t max_sparseness = 1.0 / low_0;
  int8_t largest_empty_segment = 1.0 * max_sparseness;

  int64_t num_edges_ = 0;                 // Number of edges
  int32_t num_vertices = 0;               // Number of vertices
  int32_t max_valid_vertex_id = 0;        // Max valid vertex-id
  int64_t avg_degree = 0;                 // averge degree of the graph

  /* General PMA fields */
  int64_t elem_capacity;                  // size of the edges_ array
  int32_t segment_size;                   // size of a pma leaf segment
  int64_t segment_count;                  // number of pma leaf segments
  int32_t tree_height;                    // height of the pma tree
  double delta_up;                        // Delta for upper density threshold
  double delta_low;                       // Delta for lower density threshold

  DestID_ *edges_;                        // Underlying storage for edgelist
  struct vertex_element *vertices_;       // underlying storage for vertex list

  struct LogEntry *log_base_ptr_;         // mapping of pmem::log_segment_oid_
  struct LogEntry **log_ptr_;             // pointer of logs per segment
  int32_t *log_segment_idx_;              // current insert-index of logs per segment
  int64_t *segment_edges_actual;          // actual number of edges stored in the region of a binary-tree node
  int64_t *segment_edges_total;           // total number of edges assigned in the region of a binary-tree node

  struct PMALeafSegment *leaf_segments;   // pma-leaf segments to control concurrency

  int num_threads;                        // max (available) number of concurrent write threads
  DestID_ *ulog_base_ptr_;                // base pointer of the undo-log; used to track the group allocation of undo-logs
  DestID_ **ulog_ptr_;                    // array of undo-log pointers (array size is number of write threads)
  int64_t *oplog_ptr_;                    // keeps the start index in the edge array that is backed up in undo-log
};

#endif  // GRAPH_H_
