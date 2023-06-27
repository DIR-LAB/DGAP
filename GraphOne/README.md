# GraphOne

This is the reference sourcecode of GraphOne that we [forked from here](https://github.com/the-data-lab/GraphOne) [2].

GraphOne [1] is an in-memory graph analysis framework with an extra durability guarantee using external non-volatile devices.
New data is first stored in an in-DRAM edge list in an append-only manner. Background threads incrementally move this data to non-volatile memory for persistence.
To port GraphOne to persistent memory, we change the location of **durable phase** to the PM space and require it to flush DRAM data after each `2^16` insertions to reduce the chances of losing data.
We do not limit the DRAM usage of GraphOne during graph analysis. Hence for some graphs, the graph data may be completely cached in DRAM.
Due to these settings, we name this baseline as **GraphOne-FD**, indicating GraphOne Flushing-DRAM, in our paper.

## Quick Start

Build the project:

```
> mkdir build
> cd build
> cmake ../
> make
> cd ../
```

Insert Orkut graph and run PageRank:

```
> ./build/graphone_test -b ${DATA_PATH}/orkut_10/com-orkut.ungraph.base.el -d ${DATA_PATH}/orkut_10/com-orkut.ungraph.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 3072627 -j 0 -s 1 -t 1
```

Meaning of the command line flags:
+ `-b base.el` load base-graph from file base.el
+ `-d dynamic.el` insert dynamic-graph from file dynamic.el
+ `-o dir` store the graph in directory dir
+ `-v n` there are n vertices in the graph
+ `-j j` perform j job (`0` for PageRank)
+ `-s s` source vertex-id, start from node s
+ `-t t` use t concurrent threads

## Graph Kernels Included
For a fair comparison, we integrate the following graph algorithms from the GAP Benchmark Suite (GAPBS) into GraphOne.
+ Breadth-First Search (BFS) - direction optimizing
+ PageRank (PR) - iterative method in pull direction
+ Connected Components (CC) - Shiloach-Vishkin
+ Betweenness Centrality (BC) - Brandes

## Executing the Benchmark

We provide a script to automate executing the benchmark on all the input graphs we used in our paper. Before running the benchmark, please follow [this directory structure](https://github.com/DIR-LAB/DGAP/blob/main/PREPROCESS.md) to store the input graphs.

__*Warning:*__ A full run of this benchmark might take a couple of hours to finish. If you want to skip some input files, please comment them in the [benchmark script](https://github.com/DIR-LAB/DGAP/blob/main/GraphOne/benchmark_graphone.sh).

Execute the benchmark:
```
> ./benchmark_graphone.sh > graphone.out
```

## Reference
1. Pradeep Kumar and H Howie Huang. 2019. Graphone: A data store for real-time analytics on evolving graphs. In 17th {USENIX} Conference on File and Storage Technologies ({FAST} 19). 249–263.
2. Pradeep Kumar. GraphOne. https://github.com/the-data-lab/GraphOne. Accessed Sep. 30, 2022.