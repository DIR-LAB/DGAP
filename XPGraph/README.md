# XPGraph

This is the reference sourcecode of XPGraph that we [forked from here](https://github.com/ISCS-ZJU/XPGraph) [2].

XPGraph [1] is a PM-based dynamic graph system.
It is based on GraphOne but extends it with new designs for persistent memory.
Specifically, XPGraph stores both the edge list and adjacency list in persistent memory to guarantee data persistence and leverages the DRAM as a cache to batch data into the adjacency list.
Similar to GraphOne, XPGraph also transfers data to DRAM for graph analysis. In our evaluations, we use the default parameter settings of XPGraph for comparisons.

## Quick Start

Build the project:

```
## make libary file
$ make lib

## set head file and LD_LIBRARY_PATH
$ sudo cp ./src/api/libxpgraph.h /usr/include/
$ export LD_LIBRARY_PATH=./src/api/lib/:$LD_LIBRARY_PATH

## make executable file
$ make main
```

Insert Orkut graph and run different graph analysis:

```
> ./bin/main -f ${DATA_PATH}/xpgraph/orkut/ -p0 ${PMEM_PATH}/xpgraph-db/XPGraph0/ --recovery ${PMEM_PATH}/xpgraph-db/XPGraphRecovery/ --source 1 -v 3072627 -q 5 -j 2 -t $threads
```

Meaning of the command line flags:
+ `-f dir` read input graphs from directory dir
+ `-p0 dir` store the graph in directory dir
+ `--recovery dir` directory to store data used for recovery
+ `--source s` source vertex-id, start from node s
+ `-v n` there are n vertices in the graph
+ `-q q` run each graph analysis for q times
+ `-j j` perform j job (`2` for archive and running analysis)
+ `-t t` use t concurrent threads

## Graph Kernels Included
For a fair comparison, we integrate the following graph algorithms from the GAP Benchmark Suite (GAPBS) into XPGraph.
+ Breadth-First Search (BFS) - direction optimizing
+ PageRank (PR) - iterative method in pull direction
+ Connected Components (CC) - Shiloach-Vishkin
+ Betweenness Centrality (BC) - Brandes

## Executing the Benchmark

We provide a script to automate executing the benchmark on all the input graphs we used in our paper. Before running the benchmark, please follow [this directory structure](https://github.com/DIR-LAB/DGAP/blob/main/PREPROCESS.md) to store the input graphs.

__*Warning:*__ A full run of this benchmark might take a couple of hours to finish. If you want to skip some input files, please comment them in the [benchmark script](https://github.com/DIR-LAB/DGAP/blob/main/XPGraph/benchmark_xpgraph.sh).

Execute the benchmark:
```
> ./benchmark_xpgraph.sh > xpgraph.out
```

## Reference
1. Rui Wang, Shuibing He, Weixu Zong, Yongkun Li, and Yinlong Xu. 2022. XP-Graph: XPline-Friendly Persistent Memory Graph Stores for Large-Scale Evolving Graphs. In 2022 55th IEEE/ACM International Symposium on Microarchitecture (MICRO). IEEE, 1308–1325.
2. Rui Wang. XPGraph. https://github.com/ISCS-ZJU/XPGraph. Accessed Mar. 31, 2023.