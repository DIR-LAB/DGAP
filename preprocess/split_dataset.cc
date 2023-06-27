// Note: example coppied from https://pmem.io/pmdk/manpages/linux/v1.2/libpmem.3/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock
#include <climits>
#include <algorithm>
using namespace std;

#define MILLION 1000000
#define TEN_MILLION 10000000
#define HUNDRED_MILLION 100000000

string input, output_b, output_d, output_unique;
int64_t in_lines;

pair<int32_t, int32_t> *edge_list;
pair<int32_t, int32_t> *u_edge_list;

int main(int argc, char *argv[]) {
  if(argc < 5) exit(-1);

  input = argv[1];
  output_b = argv[2];
  output_d = argv[3];
//  output_unique = argv[4];
  in_lines = atoll(argv[4]);
  in_lines = (in_lines * 2);

  edge_list = (pair<int32_t, int32_t> *) malloc(sizeof(pair<int32_t, int32_t>) * in_lines);

  std::ifstream infile(input);
  if (!infile.is_open()) {
    std::cout << "Couldn't open file " << input << std::endl;
    std::exit(-2);
  }

  int64_t line = 0;
  int64_t u_, v_;
  int32_t u, v;
  int32_t mx_u = -(INT_MAX - 1), mn_u = INT_MAX;
  while (infile >> u >> v) {
//  while (infile >> u_ >> v_) {
//    if(u_ > INT_MAX || v_ > INT_MAX) {
//      cout << "Vertex-id is larger than INT_MAX! u: " << u_ << ", v: " << v_ << endl;
//      exit(-4);
//    }
//    u = u_; v = v_;

    mx_u = max(u, mx_u);
    mn_u = min(u, mn_u);

    mx_u = max(v, mx_u);
    mn_u = min(v, mn_u);

    edge_list[line++] = make_pair(u, v);
    edge_list[line++] = make_pair(v, u);

    if(line % HUNDRED_MILLION == 0) {
      cout << "read " << (line / MILLION) << " M entries." << endl;
    }
  }

  cout << "[DONE] reading the dataset." << endl;
  cout << "MAX vertex-id: " << mx_u << ", MIN vertex-id: " << mn_u << endl;

  sort(edge_list, edge_list+in_lines);

  cout << "[DONE] dataset sorted." << endl;

//  u_edge_list = (pair<int32_t, int32_t> *) malloc(sizeof(pair<int32_t, int32_t>) * in_lines);
  int64_t out_line = 0;
  int32_t last_u = -1, last_v = -1;
  for(int64_t i=0; i<in_lines; i+=1) {
    // removing redundant edges
    if(edge_list[i].first == last_u && edge_list[i].second == last_v) continue;
    // removing self loops
//    if(edge_list[i].first == edge_list[i].second) continue;
//    u_edge_list[out_line++] = edge_list[i];
    edge_list[out_line++] = edge_list[i];
    last_u = edge_list[i].first;
    last_v = edge_list[i].second;
  }

  cout << "[DONE] unique edge-list prepared. Total unique edges: " << out_line << endl;

//  free(edge_list);
//  cout << "[DONE] edge-list memory freed." << endl;

  // randomly shuffling the edges
  // obtain a time-based seed:
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//  shuffle(u_edge_list, u_edge_list + (out_line-1), std::default_random_engine(seed));
  shuffle(edge_list, edge_list + (out_line-1), std::default_random_engine(seed));

  cout << "[DONE] unique edge-list shuffled." << endl;
  int64_t out_line_b = (out_line / 10);
  cout << "Base graph will get: " << (out_line_b + 1) << " edges." << endl;
  cout << "Dynamic graph will get: " << (out_line - out_line_b - 1) << " edges." << endl;

  ofstream out_b(output_b);
  if(!out_b) {
    cout << "Cannot open the base-output file!" << endl;
    exit(-3);
  }

  for(int64_t i=0; i<out_line_b; i+=1) {
//    out_b << u_edge_list[i].first << " " << u_edge_list[i].second << "\n";
    out_b << edge_list[i].first << " " << edge_list[i].second << "\n";
  }
//  out_b << u_edge_list[out_line - 1].first << " " << u_edge_list[out_line - 1].second << "\n";
  out_b << edge_list[out_line - 1].first << " " << edge_list[out_line - 1].second << "\n";

  cout << "[DONE] writing the base-datasets." << endl;
  out_b.flush();
  out_b.close();

  ofstream out_d(output_d);
  if(!out_d) {
    cout << "Cannot open the dynamic-output file!" << endl;
    exit(-3);
  }

  for(int64_t i=out_line_b; i<out_line-1; i+=1) {
//    out_d << u_edge_list[i].first << " " << u_edge_list[i].second << "\n";
    out_d << edge_list[i].first << " " << edge_list[i].second << "\n";
  }

  cout << "[DONE] writing the dynamic-datasets." << endl;
  out_d.flush();
  out_d.close();

//  ofstream out_u(output_unique);
//  if(!out_u) {
//    cout << "Cannot open the unique-single-output file!" << endl;
//    exit(-3);
//  }
//
//  for(int64_t i=0; i<out_line; i+=1) {
////    out_u << u_edge_list[i].first << " " << u_edge_list[i].second << "\n";
//    out_u << edge_list[i].first << " " << edge_list[i].second << "\n";
//  }
//
//  cout << "[DONE] writing the unique-single-datasets." << endl;
//  out_u.close();

  return 0;
}