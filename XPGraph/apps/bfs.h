//
// Created by Islam, Abdullah Al Raqibul on 10/1/22.
//

#ifndef GRAPHONE_GAP_BFS_H
#define GRAPHONE_GAP_BFS_H

#include "gapbs/pvector.h"
#include "gapbs/gapbs_bitmap.h"
#include "gapbs/timer.h"
#include "gapbs/sliding_queue.h"
#include "gapbs/platform_atomics.h"
#include <libxpgraph.h>

#include <math.h>
#include <iostream>

/***********************************************************************************************/
/**                              BFS Algorithm                                                **/
/***********************************************************************************************/

int64_t BUStep(XPGraph* snaph, pvector<vid_t> &parent, GapbsBitmap &front, GapbsBitmap &next) {
  int64_t awake_count = 0;
  next.reset();
#pragma omp parallel for reduction(+ : awake_count) schedule(dynamic, 1024)
  for (vid_t u=0; u < snaph->get_vcount(); u++) {
    if (parent[u] < 0) {
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
        if (front.get_bit(sid)) {
          parent[u] = sid;
          awake_count++;
          next.set_bit(u);
          break;
        }
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
//      nebr_count = snaph->get_degree_in(u);
//
//      //traverse the delta adj list
//      delta_degree = nebr_count;
//      while (delta_adjlist != 0 && delta_degree > 0) {
//        local_adjlist = delta_adjlist->get_adjlist();
//        local_degree = delta_adjlist->get_nebrcount();
//        degree_t i_count = min(local_degree, delta_degree);
//        for (degree_t i = 0; i < i_count; ++i) {
//          sid = get_sid(local_adjlist[i]);
//          if (front.get_bit(sid)) {
//            parent[u] = sid;
//            awake_count++;
//            next.set_bit(u);
//            break;
//          }
//        }
//        delta_adjlist = delta_adjlist->get_next();
//        delta_degree -= local_degree;
//      }

//      for (vid_t v : snaph->in_neigh(u)) {
//        if (front.get_bit(v)) {
//          parent[u] = v;
//          awake_count++;
//          next.set_bit(u);
//          break;
//        }
//      }
    }
  }
  return awake_count;
}


int64_t TDStep(XPGraph* snaph, pvector<vid_t> &parent,
               SlidingQueue<vid_t> &queue) {
  int64_t scout_count = 0;
#pragma omp parallel
  {
    QueueBuffer<vid_t> lqueue(queue);
#pragma omp for reduction(+ : scout_count)
    for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
      vid_t u = *q_iter;

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
        vid_t curr_val = parent[sid];
        if (curr_val < 0) {
          if (compare_and_swap(parent[sid], curr_val, u)) {
            lqueue.push_back(sid);
            scout_count += -curr_val;
          }
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
//      nebr_count = snaph->get_out_degree(u);
//
//      //traverse the delta adj list
//      delta_degree = nebr_count;
//      while (delta_adjlist != 0 && delta_degree > 0) {
//        local_adjlist = delta_adjlist->get_adjlist();
//        local_degree = delta_adjlist->get_nebrcount();
//        degree_t i_count = min(local_degree, delta_degree);
//        for (degree_t i = 0; i < i_count; ++i) {
//          sid = get_sid(local_adjlist[i]);
//          vid_t curr_val = parent[sid];
//          if (curr_val < 0) {
//            if (compare_and_swap(parent[sid], curr_val, u)) {
//              lqueue.push_back(sid);
//              scout_count += -curr_val;
//            }
//          }
//        }
//        delta_adjlist = delta_adjlist->get_next();
//        delta_degree -= local_degree;
//      }

//      for (vid_t v : snaph->out_neigh(u)) {
//        vid_t curr_val = parent[v];
//        if (curr_val < 0) {
//          if (compare_and_swap(parent[v], curr_val, u)) {
//            lqueue.push_back(v);
//            scout_count += -curr_val;
//          }
//        }
//      }
    }
    lqueue.flush();
  }
  return scout_count;
}


void QueueToBitmap(const SlidingQueue<vid_t> &queue, GapbsBitmap &bm) {
#pragma omp parallel for
  for (auto q_iter = queue.begin(); q_iter < queue.end(); q_iter++) {
    vid_t u = *q_iter;
    bm.set_bit_atomic(u);
  }
}

void BitmapToQueue(XPGraph* snaph, const GapbsBitmap &bm,
                   SlidingQueue<vid_t> &queue) {
#pragma omp parallel
  {
    QueueBuffer<vid_t> lqueue(queue);
#pragma omp for
    for (vid_t n=0; n < snaph->get_vcount(); n++)
      if (bm.get_bit(n))
        lqueue.push_back(n);
    lqueue.flush();
  }
  queue.slide_window();
}

pvector<vid_t> InitParent(XPGraph* snaph) {
  pvector<vid_t> parent(snaph->get_vcount());
#pragma omp parallel for
  for (vid_t n=0; n < snaph->get_vcount(); n++)
    parent[n] = snaph->get_out_degree(n) != 0 ? -snaph->get_out_degree(n) : -1;
  return parent;
}

/// this code copied from: plaingraph_manager_t<T>::run_bfs
//template <class T>
//pvector<vid_t> run_bfs(snap_t<dst_id_t>* snaph, index_t _edge_count, sid_t root, int alpha = 15, int beta = 18)
pvector<vid_t> run_bfs(XPGraph* snaph, index_t _edge_count, sid_t root, int alpha = 15, int beta = 18)
{
  //  PrintStep("Source", static_cast<int64_t>(source));
  Timer t;
  t.Start();
  pvector<vid_t> parent = InitParent(snaph);
  t.Stop();
//  PrintStep("i", t.Seconds());
  parent[root] = root;
  SlidingQueue<vid_t> queue(snaph->get_vcount());
  queue.push_back(root);
  queue.slide_window();
  GapbsBitmap curr(snaph->get_vcount());
  curr.reset();
  GapbsBitmap front(snaph->get_vcount());
  front.reset();
//  int64_t edges_to_check = snaph->num_edges_directed();
  int64_t edges_to_check = _edge_count;
  int64_t scout_count = snaph->get_out_degree(root);
  while (!queue.empty()) {
    if (scout_count > edges_to_check / alpha) {
      int64_t awake_count, old_awake_count;
      TIME_OP(t, QueueToBitmap(queue, front));
      QueueToBitmap(queue, front);
//      PrintStep("e", t.Seconds());
      awake_count = queue.size();
      queue.slide_window();
      do {
        t.Start();
        old_awake_count = awake_count;
        awake_count = BUStep(snaph, parent, front, curr);
        front.swap(curr);
        t.Stop();
//        PrintStep("bu", t.Seconds(), awake_count);
      } while ((awake_count >= old_awake_count) ||
               (awake_count > snaph->get_vcount() / beta));
      TIME_OP(t, BitmapToQueue(snaph, front, queue));
      BitmapToQueue(snaph, front, queue);
//      PrintStep("c", t.Seconds());
      scout_count = 1;
    } else {
      t.Start();
      edges_to_check -= scout_count;
      scout_count = TDStep(snaph, parent, queue);
      queue.slide_window();
      t.Stop();
//      PrintStep("td", t.Seconds(), queue.size());
    }
  }
#pragma omp parallel for
  for (vid_t n = 0; n < snaph->get_vcount(); n++)
    if (parent[n] < -1)
      parent[n] = -1;
  return parent;
//  double start, end;

//  pgraph_t<T>* pgraph1 = (pgraph_t<T>*)get_plaingraph();
//
//  start = mywtime();
//  snap_t<T>* snaph = create_static_view(pgraph1, V_CENTRIC);
//  end = mywtime();
//  cout << "static View creation = " << end - start << endl;

//  uint8_t* level_array = 0;
//  level_array = (uint8_t*) calloc(snaph->get_vcount(), sizeof(uint8_t));
//  start = mywtime();
////  mem_bfs<dst_id_t>(snaph, level_array, root);
//  end = mywtime();
//  cout << "BFS complex = " << end - start << endl;
//
//  free(level_array);
  /*
  level_array = (uint8_t*) calloc(snaph->get_vcount(), sizeof(uint8_t));
  start = mywtime();
  mem_bfs_simple<T>(snaph, level_array, root);
  end = mywtime();
  cout << "BFS simple = " << end - start << endl;
  */
//  delete_static_view(snaph);
}

void PrintBFSStats(XPGraph* snaph, const pvector<vid_t> &bfs_tree) {
  int64_t tree_size = 0;
  int64_t n_edges = 0;
//  for (vid_t n : snaph->get_vcount()) {
  for (vid_t n=0; n < snaph->get_vcount(); n++) {
    if (bfs_tree[n] >= 0) {
      n_edges += snaph->get_out_degree(n);
      tree_size++;
    }
  }
  std::cout << "BFS Tree has " << tree_size << " nodes and ";
  std::cout << n_edges << " edges" << std::endl;
}

#endif //GRAPHONE_GAP_BFS_H
