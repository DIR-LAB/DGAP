//
// Created by Islam, Abdullah Al Raqibul on 10/1/22.
//

#ifndef GRAPHONE_GAP_CC_H
#define GRAPHONE_GAP_CC_H

#include "gapbs/pvector.h"
#include "gapbs/bitmap.h"
#include "gapbs/sliding_queue.h"
#include "gapbs/platform_atomics.h"

#include <math.h>
#include <vector>
#include <unordered_map>

typedef float ScoreT;

/***********************************************************************************************/
/**                              CC Algorithm                                                 **/
/***********************************************************************************************/

// The hooking condition (comp_u < comp_v) may not coincide with the edge's
// direction, so we use a min-max swap such that lower component IDs propagate
// independent of the edge's direction.
pvector<vid_t> run_cc(XPGraph* snaph) {
  pvector<vid_t> comp(snaph->get_vcount());
#pragma omp parallel for
  for (vid_t n=0; n < snaph->get_vcount(); n++)
    comp[n] = n;
  bool change = true;
  int num_iter = 0;
  while (change) {
    change = false;
    num_iter++;
#pragma omp parallel for
    for (vid_t u=0; u < snaph->get_vcount(); u++) {
      sid_t sid;
      degree_t nebr_count = 0;
      degree_t local_degree = 0;
      vid_t* local_adjlist;

      nebr_count = snaph->get_out_degree(u);
      if (0 == nebr_count) continue;

      local_adjlist = new vid_t[nebr_count];
      local_degree = snaph->get_out_nebrs(u, local_adjlist);
      assert(local_degree == nebr_count);

      // traverse the delta adj list
      for (index_t j = 0; j < local_degree; ++j){
        sid = local_adjlist[j];
        vid_t comp_u = comp[u];
        vid_t comp_v = comp[sid];
        if (comp_u == comp_v) continue;
        // Hooking condition so lower component ID wins independent of direction
        vid_t high_comp = comp_u > comp_v ? comp_u : comp_v;
        vid_t low_comp = comp_u + (comp_v - high_comp);
        if (high_comp == comp[high_comp]) {
          change = true;
          comp[high_comp] = low_comp;
        }
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
//      nebr_count = snaph->get_degree_out(u);
//
//      //traverse the delta adj list
//      delta_degree = nebr_count;
//      while (delta_adjlist != 0 && delta_degree > 0) {
//        local_adjlist = delta_adjlist->get_adjlist();
//        local_degree = delta_adjlist->get_nebrcount();
//        degree_t i_count = std::min(local_degree, delta_degree);
//        for (degree_t i = 0; i < i_count; ++i) {
//          sid = get_sid(local_adjlist[i]);
//
////          for (vid_t v : snaph->out_neigh(u)) {
//            vid_t comp_u = comp[u];
//            vid_t comp_v = comp[sid];
//            if (comp_u == comp_v) continue;
//            // Hooking condition so lower component ID wins independent of direction
//            vid_t high_comp = comp_u > comp_v ? comp_u : comp_v;
//            vid_t low_comp = comp_u + (comp_v - high_comp);
//            if (high_comp == comp[high_comp]) {
//              change = true;
//              comp[high_comp] = low_comp;
//            }
////          }
//        }
//        delta_adjlist = delta_adjlist->get_next();
//        delta_degree -= local_degree;
//      }
    }
#pragma omp parallel for
    for (vid_t n=0; n < snaph->get_vcount(); n++) {
      while (comp[n] != comp[comp[n]]) {
        comp[n] = comp[comp[n]];
      }
    }
  }
  std::cout << "Shiloach-Vishkin took " << num_iter << " iterations" << std::endl;
  return comp;
}


void PrintCompStats(XPGraph* snaph, const pvector<vid_t> &comp) {
  std::cout << std::endl;
  std::unordered_map<vid_t, vid_t> count;
  for (vid_t comp_i : comp)
    count[comp_i] += 1;
  int k = 5;
  vector<std::pair<vid_t, vid_t>> count_vector;
  count_vector.reserve(count.size());
  for (auto kvp : count)
    count_vector.push_back(kvp);
  vector<std::pair<vid_t, vid_t>> top_k = TopK(count_vector, k);
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
bool CCVerifier(XPGraph* snaph, const pvector<vid_t> &comp) {
  std::unordered_map<vid_t, vid_t> label_to_source;
  for (vid_t n=0; n < snaph->get_vcount(); n++)
    label_to_source[comp[n]] = n;
  Bitmap visited(snaph->get_vcount());
  visited.reset();
  vector<vid_t> frontier;
  frontier.reserve(snaph->get_vcount());
  for (auto label_source_pair : label_to_source) {
    vid_t curr_label = label_source_pair.first;
    vid_t source = label_source_pair.second;
    frontier.clear();
    frontier.push_back(source);
    visited.set_bit(source);
    for (auto it = frontier.begin(); it != frontier.end(); it++) {
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

      // traverse the delta adj list
      for (index_t j = 0; j < local_degree; ++j){
        sid = local_adjlist[j];
        if (comp[sid] != curr_label)
          return false;
        if (!visited.get_bit(sid)) {
          visited.set_bit(sid);
          frontier.push_back(sid);
        }
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
//      nebr_count = snaph->get_degree_out(u);
//
//      //traverse the delta adj list
//      delta_degree = nebr_count;
//      while (delta_adjlist != 0 && delta_degree > 0) {
//        local_adjlist = delta_adjlist->get_adjlist();
//        local_degree = delta_adjlist->get_nebrcount();
//        degree_t i_count = std::min(local_degree, delta_degree);
//        for (degree_t i = 0; i < i_count; ++i) {
//          sid = get_sid(local_adjlist[i]);
//
////          for (vid_t v : snaph->out_neigh(u)) {
//            if (comp[sid] != curr_label)
//              return false;
//            if (!visited.get_bit(sid)) {
//              visited.set_bit(sid);
//              frontier.push_back(sid);
//            }
////          }
//        }
//        delta_adjlist = delta_adjlist->get_next();
//        delta_degree -= local_degree;
//      }
//      if (snaph->directed()) {
//        for (vid_t v : snaph->in_neigh(u)) {
//          if (comp[v] != curr_label)
//            return false;
//          if (!visited.get_bit(v)) {
//            visited.set_bit(v);
//            frontier.push_back(v);
//          }
//        }
//      }
    }
  }
  for (vid_t n=0; n < snaph->get_vcount(); n++)
    if (!visited.get_bit(n))
      return false;
  return true;
}

#endif //GRAPHONE_GAP_CC_H
