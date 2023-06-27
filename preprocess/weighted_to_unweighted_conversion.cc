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

int main(int argc, char *argv[]) {
  string input, output;

  if(argc < 5) exit(-1);

  input = argv[1];
  output = argv[2];

  std::ifstream infile(input);
  if (!infile.is_open()) {
    std::cout << "Couldn't open file " << input << std::endl;
    std::exit(-2);
  }

  ofstream outfile(output);
  if(!outfile) {
    cout << "Cannot open the " << output << "-th output file!" << endl;
    exit(-3);
  }

  int64_t line = 0;
  int32_t u, v;
  int64_t w;

  while (infile >> u >> v >> w) {
    outfile << u << " " << v << "\n";
    line += 1;
//    if(line % MILLION == 0)) {
//      cout << "[DONE] writing " << split_count << "-th file; going to open the next one." << endl;
//    }
  }
  outfile.close();
  outfile.clear();
  infile.close();

  return 0;
}
