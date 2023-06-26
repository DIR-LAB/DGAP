#include <iostream>
#include <string>
#include <fstream>

#include <libxpgraph.h>
#include "apps/graph_benchmarks.hpp"

int main(int argc, const char ** argv)
{
    XPGraph *xpgraph = new XPGraph(argc, argv);
    xpgraph->import_graph_by_config();

    // test_graph_benchmarks(xpgraph);
    test_gapbs_graph_benchmarks(xpgraph);
    xpgraph->save_gragh();
    delete xpgraph;
    return 0;
}
