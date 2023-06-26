//
// Created by Islam, Abdullah Al Raqibul on 10/1/22.
//

#ifndef GRAPHONE_EXAMPLE_H
#define GRAPHONE_EXAMPLE_H

//#include "mem_iterative_analytics.h"
//#include "gapbs/pvector.h"
//#include "gapbs/bitmap.h"
//#include "gapbs/sliding_queue.h"
//#include "gapbs/platform_atomics.h"

//#include <math.h>
#include <vector>
//#include <unordered_map>

typedef float ScoreT;

//template <class T>
void waitfor_archive_durable(pgraph_t<dst_id_t> *pgraph, double start)
{
//  pgraph_t<T>* pgraph = (pgraph_t<T>*)get_plaingraph();
  blog_t<dst_id_t>* blog = pgraph->blog;
  index_t marker = blog->blog_head;
  if (marker != blog->blog_marker) {
    pgraph->create_marker(marker);
  }
  double end = 0;
  bool done_making = false;
  bool done_persisting = false;
  while (!done_making || !done_persisting) {
    if (blog->blog_tail == blog->blog_head && !done_making) {
      end = mywtime();
//      cout << "Make Graph Time = " << end - start << endl;
      done_making = true;
    }
    if (blog->blog_wtail == blog->blog_head && !done_persisting) {
      end = mywtime();
//      cout << "Durable Graph Time = " << end - start << endl;
      done_persisting = true;
    }
    usleep(1);
  }
}

/***********************************************************************************************/
/**                              GAP Utility Functions                                        **/
/***********************************************************************************************/

// Returns k pairs with largest values from list of key-value pairs
template<typename KeyT, typename ValT>
std::vector<std::pair<ValT, KeyT>> TopK(
    const std::vector<std::pair<KeyT, ValT>> &to_sort, size_t k) {
  std::vector<std::pair<ValT, KeyT>> top_k;
  ValT min_so_far = 0;
  for (auto kvp : to_sort) {
    if ((top_k.size() < k) || (kvp.second > min_so_far)) {
      top_k.push_back(std::make_pair(kvp.second, kvp.first));
      std::sort(top_k.begin(), top_k.end(),
                std::greater<std::pair<ValT, KeyT>>());
      if (top_k.size() > k)
        top_k.resize(k);
      min_so_far = top_k.back().first;
    }
  }
  return top_k;
}

#endif //GRAPHONE_EXAMPLE_H
