# Blocked Adjacency-List (BAL)

This is the reference implementation of our proposed framework DGAP in persistent memory.
Current DGAP implementation is based on PMDK 1.12 (we use the PMDK library only for memory management/allocation purposes).
Before running DGAP on larger datasets, Please [check these instructions](https://github.com/DIR-LAB/DGAP/blob/main/README.md#changes-in-pmdk) to install PMDK from [our provided source](https://github.com/DIR-LAB/DGAP/tree/main/pmdk).

## Quick Start

Build the project:

```
> make
```

Insert Orkut graph and run PageRank:

```
> ./pr -B ${DATA_PATH}/orkut_10/com-orkut.ungraph.base.el -D ${DATA_PATH}/orkut_10/com-orkut.ungraph.dynamic.el -f ${PMEM_PATH}/orkut.pmem -r 1 -n 5 -a
```

Meaning of the command line flags:
+ `-B base.el` load base-graph from file base.el
+ `-D dynamic.el` insert dynamic-graph from file dynamic.el
+ `-f graph.pmem` store the graph in graph.pmem
+ `-r r` source vertex-id, start from node r
+ `-n n` perform n trials
+ `-a` output analysis of last run

## Graph Kernels Included
For a fair comparison, we integrate the following graph algorithms from the GAP Benchmark Suite (GAPBS) into DGAP.
+ Breadth-First Search (BFS) - direction optimizing
+ PageRank (PR) - iterative method in pull direction
+ Connected Components (CC) - Shiloach-Vishkin
+ Betweenness Centrality (BC) - Brandes

## Executing the Benchmark

We provide a script to automate executing the benchmark on all the input graphs we used in our paper. Before running the benchmark, please follow [this directory structure](https://github.com/DIR-LAB/DGAP/blob/main/PREPROCESS.md) to store the input graphs.

__*Warning:*__ A full run of this benchmark might take a couple of hours to finish. If you want to skip some input files, please comment them in the [benchmark script](https://github.com/DIR-LAB/DGAP/blob/main/dgap/benchmark_dgap.sh).

Execute the benchmark:
```
> ./benchmark_dgap.sh > dgap.out
```
