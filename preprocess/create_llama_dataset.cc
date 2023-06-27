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

#define FILE_EXTENSION ".net"

string get_filename(string pref, int id) {
  return (pref + "__" + to_string(id) + FILE_EXTENSION);
}

int main(int argc, char *argv[]) {
  string input, output_base, filename;
  int64_t in_lines;
  int32_t num_splits;

  if(argc < 5) exit(-1);

  input = argv[1];
  output_base = argv[2];
  in_lines = atoll(argv[3]);
  num_splits = atoi(argv[4]);

  std::ifstream infile(input);
  if (!infile.is_open()) {
    std::cout << "Couldn't open file " << input << std::endl;
    std::exit(-2);
  }

  int64_t line = 0;
  int64_t num_edges_per_split = (int64_t) ceil((double) in_lines / (double) num_splits);
  int32_t split_count = 1;
  int32_t u, v;
  filename = get_filename(output_base, split_count);

  ofstream outfile(filename);
  if(!outfile) {
    cout << "Cannot open the " << split_count << "-th output file!" << endl;
    exit(-3);
  }

  while (infile >> u >> v) {
    if(line && (line % num_edges_per_split == 0)) {
      outfile.close();
      outfile.clear();
      cout << "[DONE] writing " << split_count << "-th file; going to open the next one." << endl;

      split_count += 1;
      filename = get_filename(output_base, split_count);
      outfile.open(filename);
      if(!outfile) {
        cout << "Cannot open the " << split_count << "-th output file!" << endl;
        exit(-3);
      }
    }

    outfile << u << " " << v << "\n";
    line += 1;
  }
  outfile.close();
  outfile.clear();

  return 0;
}
