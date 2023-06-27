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

pair<int32_t, int32_t> *edge_list;

#define FILE_EXTENSION ".el"

int main(int argc, char *argv[]) {
  string input, output;
  int64_t in_lines;

  if(argc < 4) exit(-1);

  input = argv[1];
  output = argv[2];
  in_lines = atoll(argv[3]);
  in_lines = (in_lines * 2);

  std::ifstream infile(input);
  if (!infile.is_open()) {
    std::cout << "Couldn't open file " << input << std::endl;
    std::exit(-2);
  }

  edge_list = (pair<int32_t, int32_t> *) malloc(sizeof(pair<int32_t, int32_t>) * in_lines);

  int64_t line = 0;
  int32_t u, v;
  while (infile >> u >> v) {
    edge_list[line++] = make_pair(u, v);
    edge_list[line++] = make_pair(v, u);

    if(line % TEN_MILLION == 0) {
      cout << "read " << (line / MILLION) << " M entries." << endl;
    }
  }

  sort(edge_list, edge_list+in_lines);

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

  // obtain a time-based seed:
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//  shuffle(edge_list, edge_list + in_lines, std::default_random_engine(seed));
  shuffle(edge_list, edge_list + out_line, std::default_random_engine(seed));

  ofstream outfile(output);
  if(!outfile) {
    cout << "Cannot open the " << output << " output file!" << endl;
    exit(-3);
  }

//  for(int64_t i=0; i<in_lines; i+=1) {
  for(int64_t i=0; i<out_line; i+=1) {
    outfile << edge_list[i].first << " " << edge_list[i].second << "\n";
  }

  outfile.close();

  return 0;
}