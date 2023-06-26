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


template <typename NodeID_, typename DestID_ = NodeID_,
          typename WeightT_ = NodeID_, typename TimestampT_ = WeightT_, bool invert = true>
class BuilderBase {
  typedef EdgePair<NodeID_, DestID_> Edge;
  typedef pvector<Edge> EdgeList;

  const CLBase &cli_;
  bool symmetrize_;
  bool needs_weights_;
  int64_t num_nodes_ = -1;
  int64_t num_edges_ = 0;
  int64_t base_graph_num_edges_ = 0;

 public:
  explicit BuilderBase(const CLBase &cli) : cli_(cli) {
    symmetrize_ = cli_.symmetrize();
    needs_weights_ = !std::is_same<NodeID_, DestID_>::value;
  }

  DestID_ GetSource(EdgePair<NodeID_, NodeID_> e) {
    return e.u;
  }

  DestID_ GetSource(EdgePair<NodeID_, NodeWeight<NodeID_>> e) {
    return NodeWeight<NodeID_>(e.u);
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

  CSRGraph<NodeID_, DestID_, invert> MakeGraph() {
    bool needs_ingestion_ = true;
    EdgeList el;
    Timer t;

    /* pmem db-file already exists, no needs to ingest data into pmem */
    if (file_exists(cli_.dbfilename().c_str()) == 0) {
      printf("[%s]: pmem db-file already exists, no needs to ingest data into pmem\n", __FUNCTION__);
      needs_ingestion_ = false;
    }
    else {
      if (cli_.base_filename() != "") {
        Reader<NodeID_, DestID_, WeightT_, TimestampT_, invert> r(cli_.base_filename());
        el = r.ReadFile(needs_weights_);
      } else {
        printf("[%s]: graph input-file not exists, abort!!!\n", __FUNCTION__);
        exit(0);
      }

      base_graph_num_edges_ = el.size();
      num_nodes_ = FindMaxNodeID(el) + 1;

      if (symmetrize_) {
        for (int i = 0; i < base_graph_num_edges_; i += 1) {
          el.push_back(EdgePair<NodeID_, DestID_>(static_cast<NodeID_>(el[i].v), GetSource(el[i])));
        }
        base_graph_num_edges_ *= 2;
      }

      std::sort(el.begin(), el.end(), [](Edge &a, Edge &b) {
        return a.u < b.u;
      });
    }

    CSRGraph<NodeID_, DestID_, invert> g(cli_.dbfilename().c_str(), el, !symmetrize_, base_graph_num_edges_, num_nodes_);

    if(needs_ingestion_) {
      el.clear();

      if (cli_.dynamic_filename() != "") {
        Reader<NodeID_, DestID_, WeightT_, TimestampT_, invert> r(cli_.dynamic_filename());
        el = r.ReadFile(needs_weights_);
      } else {
        printf("[%s]: graph input-file not exists, abort!!!\n", __FUNCTION__);
        exit(0);
      }
      size_t dynamic_edges = el.size();
      cout << "total dynamic_edges loaded to memory: " << dynamic_edges << endl;
      t.Start();
//      g.print_pma_meta();

//      #pragma omp parallel for
      #pragma omp parallel for schedule(dynamic, 1048576)   // 1024 * 1024
      for (uint64_t i = 0; i < dynamic_edges; i += 1) {
        g.insert(el[i].u, el[i].v.v);
        if (symmetrize_) {
          g.insert(el[i].v.v, el[i].u);
        }
//        if(i && i % 10000000 == 0) cout << "inserted " << (i/1000000) << " M dynamic edges" << endl;
      }
      t.Stop();
      cout << "D-Graph Build Time: " << t.Seconds() << " seconds." << endl;
    }

    return g;
  }
};

#endif  // BUILDER_H_
