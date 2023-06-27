# Compressed Sparse Row (CSR)

This is the reference implementation for the Compressed Sparse Row (CSR) in persistent memory.
CSR is the baseline for graph analysis evaluations since 1) it can not be updated and 2) it represents the best graph analysis performance for its extremely compact memory layout.

## Quick Start

Build the project:

```
> make
```

Load Orkut graph and run PageRank:

```
> ./pr -f ${DATA_PATH}/single-file/com-orkut.ungraph.el -d ${PMEM_PATH}/orkut_csr.db -r 1 -n 5 -a
```

Meaning of the command line flags:
+ `-f graph.el` load graph from file graph.el
+ `-d graph.pmem` store the graph in graph.pmem
+ `-r r` source vertex-id, start from node r
+ `-n n` perform n trials
+ `-a` output analysis of last run

## Graph Kernels Included
For a fair comparison, we integrate the following graph algorithms from the GAP Benchmark Suite (GAPBS) into CSR.
+ Breadth-First Search (BFS) - direction optimizing
+ PageRank (PR) - iterative method in pull direction
+ Connected Components (CC) - Shiloach-Vishkin
+ Betweenness Centrality (BC) - Brandes

## Executing the Benchmark

We provide a script to automate executing the benchmark on all the input graphs we used in our paper. Before running the benchmark, please follow [this directory structure](https://github.com/DIR-LAB/DGAP/blob/main/PREPROCESS.md) to store the input graphs.

__*Warning:*__ A full run of this benchmark might take a couple of hours to finish. If you want to skip some input files, please comment them in the [benchmark script](https://github.com/DIR-LAB/DGAP/blob/main/csr/benchmark_csr.sh).

Execute the benchmark:
```
> ./benchmark_csr.sh > csr.out
```
