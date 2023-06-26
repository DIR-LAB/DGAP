//
// Created by Islam, Abdullah Al Raqibul on 10/1/22.
//

#ifndef GRAPHONE_GAP_BC_H
#define GRAPHONE_GAP_BC_H

#include "gapbs/pvector.h"
#include "gapbs/bitmap.h"
#include "gapbs/sliding_queue.h"
#include "gapbs/platform_atomics.h"

#include <math.h>
#include <vector>

typedef float ScoreT;
typedef int64_t SGOffset;

/***********************************************************************************************/
/**                              BC Algorithm                                                 **/
/***********************************************************************************************/

static void ParallelPrefixSum(const pvector<int32_t> &degrees, pvector<SGOffset> &prefix) {
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

static void ParallelLoadDegrees(XPGraph* snaph, pvector<int32_t> &degrees) {
#pragma omp parallel for schedule(dynamic, 16384)
  for (vid_t u = 0; u < snaph->get_vcount(); u++) {
    degrees[u] = snaph->get_out_degree(u);
  }
}

inline int64_t GetEdgeId(const pvector<SGOffset> &prefix, vid_t u, vid_t local_edge_id) {
//  if(u == 0) return local_edge_id;
  return prefix[u] + local_edge_id;
}

void PBFS(XPGraph* snaph, vid_t source, pvector<vid_t> &path_counts,
          Bitmap &succ, vector<SlidingQueue<vid_t>::iterator> &depth_index,
          SlidingQueue<vid_t> &queue, const pvector<SGOffset> &prefix) {
  pvector<vid_t> depths(snaph->get_vcount(), -1);
  depths[source] = 0;
  path_counts[source] = 1;
  queue.push_back(source);
  depth_index.push_back(queue.begin());
  queue.slide_window();
//  const vid_t* g_out_start = snaph->out_neigh(0).begin().ptr;
#pragma omp parallel
  {
    vid_t depth = 0;
    QueueBuffer<vid_t> lqueue(queue);
    while (!queue.empty()) {
#pragma omp single
      depth_index.push_back(queue.begin());
      depth++;
#pragma omp for schedule(dynamic, 64)
      for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
        vid_t u = *q_iter;
        vid_t local_edge_id = 0;

        sid_t sid;
        degree_t nebr_count = 0;
        degree_t local_degree = 0;
        vid_t* local_adjlist;

        nebr_count = snaph->get_out_degree(u);
        if (0 == nebr_count) continue;

        local_adjlist = new vid_t[nebr_count];
        local_degree = snaph->get_out_nebrs(u, local_adjlist);
        assert(local_degree == nebr_count);

        for (index_t j = 0; j < local_degree; ++j) {
          sid = local_adjlist[j];
          if ((depths[sid] == -1) &&
              (compare_and_swap(depths[sid], static_cast<vid_t>(-1), depth))) {
            lqueue.push_back(sid);
          }
          if (depths[sid] == depth) {
//            succ.set_bit_atomic(&v - g_out_start);
            succ.set_bit_atomic(GetEdgeId(prefix, u, local_edge_id));
            fetch_and_add(path_counts[sid], path_counts[u]);
          }
          local_edge_id += 1;
        }
        delete [] local_adjlist;

//        sid_t sid;
//        degree_t      delta_degree = 0;
//        degree_t nebr_count = 0;
//        degree_t local_degree = 0;
//        delta_adjlist_t<dst_id_t>* delta_adjlist;
//        dst_id_t* local_adjlist = 0;
//        delta_adjlist = snaph->get_nebrs_archived_out(u);
//        if (0 == delta_adjlist) continue;
//        nebr_count = snaph->get_out_degree(u);
//
//        //traverse the delta adj list
//        delta_degree = nebr_count;
//        while (delta_adjlist != 0 && delta_degree > 0) {
//          local_adjlist = delta_adjlist->get_adjlist();
//          local_degree = delta_adjlist->get_nebrcount();
//          degree_t i_count = std::min(local_degree, delta_degree);
//          for (degree_t i = 0; i < i_count; ++i) {
//            sid = get_sid(local_adjlist[i]);
//
////        for (vid_t &v : snaph->out_neigh(u)) {
////            for (vid_t v : snaph->out_neigh(u)) {
//              if ((depths[sid] == -1) &&
//                  (compare_and_swap(depths[sid], static_cast<vid_t>(-1), depth))) {
//                lqueue.push_back(sid);
//              }
//              if (depths[sid] == depth) {
////            succ.set_bit_atomic(&v - g_out_start);
//                succ.set_bit_atomic(GetEdgeId(prefix, u, local_edge_id));
//                fetch_and_add(path_counts[sid], path_counts[u]);
//              }
//              local_edge_id += 1;
////            }
//          }
//          delta_adjlist = delta_adjlist->get_next();
//          delta_degree -= local_degree;
//        }
      }
      lqueue.flush();
#pragma omp barrier
#pragma omp single
      queue.slide_window();
    }
  }
  depth_index.push_back(queue.begin());
}


pvector<ScoreT> run_bc(XPGraph* snaph, index_t _edge_count, sid_t root, vid_t num_iters) {
//  Timer t;
//  t.Start();
  pvector<ScoreT> scores(snaph->get_vcount(), 0);
  pvector<vid_t> path_counts(snaph->get_vcount());
  Bitmap succ(_edge_count);
  vector<SlidingQueue<vid_t>::iterator> depth_index;
  SlidingQueue<vid_t> queue(snaph->get_vcount());

  pvector<int32_t> degrees(snaph->get_vcount());
  ParallelLoadDegrees(snaph, degrees);
  pvector<SGOffset> prefix(degrees.size() + 1);
  ParallelPrefixSum(degrees, prefix);

//  std::cout << "num-iters: " << num_iters << std::endl;

//  t.Stop();
//  PrintStep("a", t.Seconds());
//  const vid_t* g_out_start = snaph->out_neigh(0).begin().ptr;
//  std::cout << "first element: " << *g_out_start << std::endl;
  for (vid_t iter=0; iter < num_iters; iter++) {
    vid_t source = root;
//    std::cout << "source: " << source << std::endl;
//    t.Start();
    path_counts.fill(0);
    depth_index.resize(0);
    queue.reset();
    succ.reset();
    PBFS(snaph, source, path_counts, succ, depth_index, queue, prefix);
//    t.Stop();
//    PrintStep("b", t.Seconds());
    pvector<ScoreT> deltas(snaph->get_vcount(), 0);
//    t.Start();
    for (int d=depth_index.size()-2; d >= 0; d--) {
#pragma omp parallel for schedule(dynamic, 64)
      for (auto it = depth_index[d]; it < depth_index[d+1]; it++) {
        vid_t u = *it;
        ScoreT delta_u = 0;
        vid_t local_edge_id = 0;

        sid_t sid;
        degree_t nebr_count = 0;
        degree_t local_degree = 0;
        vid_t* local_adjlist;
        nebr_count = snaph->get_out_degree(u);
        if (0 == nebr_count) continue;

        local_adjlist = new vid_t[nebr_count];
        local_degree = snaph->get_out_nebrs(u, local_adjlist);
        assert(local_degree == nebr_count);

        for (index_t j = 0; j < local_degree; ++j){
          sid = local_adjlist[j];
          if (succ.get_bit(GetEdgeId(prefix, u, local_edge_id))) {
            delta_u += static_cast<ScoreT>(path_counts[u]) /
                       static_cast<ScoreT>(path_counts[sid]) * (1 + deltas[sid]);
          }
          local_edge_id += 1;
        }
        delete [] local_adjlist;

//        sid_t sid;
//        degree_t      delta_degree = 0;
//        degree_t nebr_count = 0;
//        degree_t local_degree = 0;
//        delta_adjlist_t<dst_id_t>* delta_adjlist;
//        dst_id_t* local_adjlist = 0;
//        delta_adjlist = snaph->get_nebrs_archived_out(u);
//        if (0 == delta_adjlist) continue;
//        nebr_count = snaph->get_out_degree(u);
//
//        //traverse the delta adj list
//        delta_degree = nebr_count;
//        while (delta_adjlist != 0 && delta_degree > 0) {
//          local_adjlist = delta_adjlist->get_adjlist();
//          local_degree = delta_adjlist->get_nebrcount();
//          degree_t i_count = std::min(local_degree, delta_degree);
//          for (degree_t i = 0; i < i_count; ++i) {
//            sid = get_sid(local_adjlist[i]);
//
////        for (vid_t &v : snaph->out_neigh(u)) {
////            for (vid_t v : snaph->out_neigh(u)) {
////          if (succ.get_bit(&v - g_out_start)) {
//              if (succ.get_bit(GetEdgeId(prefix, u, local_edge_id))) {
//                delta_u += static_cast<ScoreT>(path_counts[u]) /
//                           static_cast<ScoreT>(path_counts[sid]) * (1 + deltas[sid]);
//              }
//              local_edge_id += 1;
////            }
//          }
//          delta_adjlist = delta_adjlist->get_next();
//          delta_degree -= local_degree;
//        }

        deltas[u] = delta_u;
        scores[u] += delta_u;
      }
    }
//    t.Stop();
//    PrintStep("p", t.Seconds());
  }
  // normalize scores
  ScoreT biggest_score = 0;
#pragma omp parallel for reduction(max : biggest_score)
  for (vid_t n=0; n < snaph->get_vcount(); n++)
    biggest_score = std::max(biggest_score, scores[n]);
#pragma omp parallel for
  for (vid_t n=0; n < snaph->get_vcount(); n++)
    scores[n] = scores[n] / biggest_score;
  return scores;
}


void PrintTopScores(XPGraph* snaph, const pvector<ScoreT> &scores) {
  vector<std::pair<vid_t, ScoreT>> score_pairs(snaph->get_vcount());
  for (vid_t n=0; n<snaph->get_vcount(); n+=1)
    score_pairs[n] = std::make_pair(n, scores[n]);
  int k = 5;
  vector<std::pair<ScoreT, vid_t>> top_k = TopK(score_pairs, k);
  for (auto kvp : top_k)
    std::cout << kvp.second << ":" << kvp.first << std::endl;
}


// Still uses Brandes algorithm, but has the following differences:
// - serial (no need for atomics or dynamic scheduling)
// - uses vector for BFS queue
// - regenerates farthest to closest traversal order from depths
// - regenerates successors from depths
bool BCVerifier(XPGraph* snaph, sid_t root, int32_t num_iters,
                const pvector<ScoreT> &scores_to_test) {
  pvector<ScoreT> scores(snaph->get_vcount(), 0);
  for (int iter=0; iter < num_iters; iter++) {
    vid_t source = root;
    // BFS phase, only records depth & path_counts
    pvector<int> depths(snaph->get_vcount(), -1);
    depths[source] = 0;
    vector<vid_t> path_counts(snaph->get_vcount(), 0);
    path_counts[source] = 1;
    vector<vid_t> to_visit;
    to_visit.reserve(snaph->get_vcount());
    to_visit.push_back(source);
    for (auto it = to_visit.begin(); it != to_visit.end(); it++) {
      vid_t u = *it;

      sid_t sid;
      degree_t nebr_count = 0;
      degree_t local_degree = 0;
      vid_t* local_adjlist;
      nebr_count = snaph->get_out_degree(u);
      if (0 == nebr_count) continue;

      local_adjlist = new vid_t[nebr_count];
      local_degree = snaph->get_out_nebrs(u, local_adjlist);
      assert(local_degree == nebr_count);

      for (index_t j = 0; j < local_degree; ++j){
        sid = local_adjlist[j];
        if (depths[sid] == -1) {
          depths[sid] = depths[u] + 1;
          to_visit.push_back(sid);
        }
        if (depths[sid] == depths[u] + 1)
          path_counts[sid] += path_counts[u];
      }
      delete [] local_adjlist;

//      sid_t sid;
//      degree_t      delta_degree = 0;
//      degree_t nebr_count = 0;
//      degree_t local_degree = 0;
//      delta_adjlist_t<dst_id_t>* delta_adjlist;
//      dst_id_t* local_adjlist = 0;
//      delta_adjlist = snaph->get_nebrs_archived_out(u);
//      if (0 == delta_adjlist) continue;
//      nebr_count = snaph->get_out_degree(u);
//
//      //traverse the delta adj list
//      delta_degree = nebr_count;
//      while (delta_adjlist != 0 && delta_degree > 0) {
//        local_adjlist = delta_adjlist->get_adjlist();
//        local_degree = delta_adjlist->get_nebrcount();
//        degree_t i_count = std::min(local_degree, delta_degree);
//        for (degree_t i = 0; i < i_count; ++i) {
//          sid = get_sid(local_adjlist[i]);
////          rank += prior_rank_array[sid];
////            incoming_total += outgoing_contrib[sid];
////          for (vid_t v : snaph->out_neigh(u)) {
//            if (depths[sid] == -1) {
//              depths[sid] = depths[u] + 1;
//              to_visit.push_back(sid);
//            }
//            if (depths[sid] == depths[u] + 1)
//              path_counts[sid] += path_counts[u];
////          }
////            }
//        }
//        delta_adjlist = delta_adjlist->get_next();
//        delta_degree -= local_degree;
//      }
    }
    // Get lists of vertices at each depth
    vector<vector<vid_t>> verts_at_depth;
    for (vid_t n=0; n<snaph->get_vcount(); n+=1) {
      if (depths[n] != -1) {
        if (depths[n] >= static_cast<int>(verts_at_depth.size()))
          verts_at_depth.resize(depths[n] + 1);
        verts_at_depth[depths[n]].push_back(n);
      }
    }
    // Going from farthest to clostest, compute "depencies" (deltas)
    pvector<ScoreT> deltas(snaph->get_vcount(), 0);
    for (int depth=verts_at_depth.size()-1; depth >= 0; depth--) {
      for (vid_t u : verts_at_depth[depth]) {
        sid_t sid;
        degree_t nebr_count = 0;
        degree_t local_degree = 0;
        vid_t* local_adjlist;
        nebr_count = snaph->get_out_degree(u);
        if (0 == nebr_count) continue;

        local_adjlist = new vid_t[nebr_count];
        local_degree = snaph->get_out_nebrs(u, local_adjlist);
        assert(local_degree == nebr_count);

        for (index_t j = 0; j < local_degree; ++j){
          sid = local_adjlist[j];
          if (depths[sid] == depths[u] + 1) {
            deltas[u] += static_cast<ScoreT>(path_counts[u]) /
                         static_cast<ScoreT>(path_counts[sid]) * (1 + deltas[sid]);
          }
        }
        delete [] local_adjlist;

//        sid_t sid;
//        degree_t      delta_degree = 0;
//        degree_t nebr_count = 0;
//        degree_t local_degree = 0;
//        delta_adjlist_t<dst_id_t>* delta_adjlist;
//        dst_id_t* local_adjlist = 0;
//        delta_adjlist = snaph->get_nebrs_archived_out(u);
//        if (0 == delta_adjlist) continue;
//        nebr_count = snaph->get_out_degree(u);
//
//        //traverse the delta adj list
//        delta_degree = nebr_count;
//        while (delta_adjlist != 0 && delta_degree > 0) {
//          local_adjlist = delta_adjlist->get_adjlist();
//          local_degree = delta_adjlist->get_nebrcount();
//          degree_t i_count = std::min(local_degree, delta_degree);
//          for (degree_t i = 0; i < i_count; ++i) {
//            sid = get_sid(local_adjlist[i]);
////          rank += prior_rank_array[sid];
////            incoming_total += outgoing_contrib[sid];
////            for (vid_t v : snaph->out_neigh(u)) {
//              if (depths[sid] == depths[u] + 1) {
//                deltas[u] += static_cast<ScoreT>(path_counts[u]) /
//                             static_cast<ScoreT>(path_counts[sid]) * (1 + deltas[sid]);
//              }
////            }
//          }
//          delta_adjlist = delta_adjlist->get_next();
//          delta_degree -= local_degree;
//        }
        scores[u] += deltas[u];
      }
    }
  }
  // Normalize scores
  ScoreT biggest_score = *std::max_element(scores.begin(), scores.end());
  for (vid_t n=0; n < snaph->get_vcount(); n+=1)
    scores[n] = scores[n] / biggest_score;
  // Compare scores
  bool all_ok = true;
  for (vid_t n=0; n < snaph->get_vcount(); n+=1) {
    if (scores[n] != scores_to_test[n]) {
      std::cout << n << ": " << scores[n] << " != " << scores_to_test[n] << std::endl;
      all_ok = false;
    }
  }
  return all_ok;
}

#endif //GRAPHONE_GAP_BC_H
