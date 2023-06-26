// Copyright (c) 2015, The Regents of the University of California (Regents)
// See LICENSE.txt for license details

#include <iostream>

#include "benchmark.h"
#include "builder.h"
#include "command_line.h"
#include "graph.h"
#include "reader.h"
#include "writer.h"

using namespace std;

int main(int argc, char* argv[]) {
  CLConvert cli(argc, argv, "converter");
  cli.ParseArgs();
  if (cli.out_weighted()) {
    WeightedBuilder bw(cli);
    WGraph wg = bw.MakeGraph();
    wg.PrintStats();
    WeightedWriter ww(wg);
    ww.WriteHammerGraph(cli.out_filename(), cli.out_sg());
  } else {
    std::cout << "Un-weighted graph is not supported here!" << std::endl;
    std::exit(-1);
//    Builder b(cli);
//    Graph g = b.MakeGraph();
//    g.PrintStats();
//    Writer w(g);
//    w.WriteHammerGraph(cli.out_filename(), cli.out_sg());
  }
  return 0;
}
