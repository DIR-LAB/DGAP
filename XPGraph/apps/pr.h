//
// Created by Islam, Abdullah Al Raqibul on 10/1/22.
//

#ifndef GRAPHONE_GAP_PR_H
#define GRAPHONE_GAP_PR_H

#include "gapbs/pvector.h"
#include "gapbs/platform_atomics.h"

#include <math.h>

typedef float ScoreT;
const float kDamp = 0.85;

/***********************************************************************************************/
/**                              PageRank Algorithm                                           **/
/***********************************************************************************************/

/// this code copied from: plaingraph_manager_t<T>::run_pr
//template <class T>
pvector<ScoreT> run_pr(XPGraph* snaph, int max_iters, double epsilon = 0)
{
//  pgraph_t<T>* pgraph = (pgraph_t<T>*)get_plaingraph();
//  snap_t<T>* snaph = create_static_view(pgraph, STALE_MASK|V_CENTRIC);

//  mem_pagerank<dst_id_t>(snaph, 20);
//  delete_static_view(snaph);

  const ScoreT init_score = 1.0f / snaph->get_vcount();
  const ScoreT base_score = (1.0f - kDamp) / snaph->get_vcount();
  pvector<ScoreT> scores(snaph->get_vcount(), init_score);
  pvector<ScoreT> outgoing_contrib(snaph->get_vcount());
  for (int iter=0; iter < max_iters; iter++) {
    double error = 0;
#pragma omp parallel for
    for (vid_t n=0; n < snaph->get_vcount(); n++)
      outgoing_contrib[n] = scores[n] / snaph->get_out_degree(n);
#pragma omp parallel for reduction(+ : error) schedule(dynamic, 64)
    for (vid_t u=0; u < snaph->get_vcount(); u++) {
      ScoreT incoming_total = 0;

      sid_t sid;
      degree_t nebr_count = 0;
      degree_t local_degree = 0;
      vid_t* local_adjlist;
      nebr_count = snaph->get_in_degree(u);
      if (0 == nebr_count) continue;

      local_adjlist = new vid_t[nebr_count];
      local_degree = snaph->get_in_nebrs(u, local_adjlist);
      assert(local_degree == nebr_count);

      // traverse the delta adj list
      for (index_t j = 0; j < local_degree; ++j){
        sid = local_adjlist[j];
        incoming_total += outgoing_contrib[sid];
      }
      delete [] local_adjlist;


//      sid_t sid;
//      degree_t      delta_degree = 0;
//      degree_t nebr_count = 0;
//      degree_t local_degree = 0;
//      delta_adjlist_t<dst_id_t>* delta_adjlist;
//      dst_id_t* local_adjlist = 0;
//      delta_adjlist = snaph->get_nebrs_archived_in(u);
//      if (0 == delta_adjlist) continue;
//      nebr_count = snaph->get_in_degree(u);
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
//          incoming_total += outgoing_contrib[sid];
//        }
//        delta_adjlist = delta_adjlist->get_next();
//        delta_degree -= local_degree;
//      }

//      for (vid_t v : snaph->in_neigh(u))
//        incoming_total += outgoing_contrib[v];
      ScoreT old_score = scores[u];
      scores[u] = base_score + kDamp * incoming_total;
      error += fabs(scores[u] - old_score);
    }
//    printf(" %2d    %lf\n", iter, error);
//    if (error < epsilon)
//      break;
  }
  return scores;
}

void PrintTopPRScores(XPGraph* snaph, const pvector<ScoreT> &scores) {
  vector<std::pair<vid_t, ScoreT>> score_pairs(snaph->get_vcount());
  for (vid_t n=0; n < snaph->get_vcount(); n++) {
    score_pairs[n] = std::make_pair(n, scores[n]);
  }
  int k = 5;
  vector<std::pair<ScoreT, vid_t>> top_k = TopK(score_pairs, k);
  k = std::min(k, static_cast<int>(top_k.size()));
  for (auto kvp : top_k)
    std::cout << kvp.second << ":" << kvp.first << std::endl;
}

#endif //GRAPHONE_GAP_PR_H
