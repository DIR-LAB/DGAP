# LLAMA

This is the reference sourcecode of LLAMA that we [forked from here](https://github.com/goatdb/llama) [2].

LLAMA [1] uses a multi-versioned CSR structure to enable fast graph analysis and graph mutations.
The graph updates are conducted in batch and organized as multiple immutable snapshots in LLAMA.
To avoid creating too many snapshots in LLAMA, in our evaluation, we only create a snapshot after inserting `1%` of the graph, which ranges from 330K edges to 36M edges, depending on the chosen graph dataset.
In total, we will create 90 snapshots for each graph (the first `10%` warm-up is a single snapshot).
Because graph analysis in LLAMA can not read the latest graph unless the snapshot is created, these large snapshots mean its graph analysis tasks may miss as many as 36 million edges, which might not be acceptable in some applications.
We port LLAMA to persistent memory by changing the location of its snapshot files to PMs space, which shows a naive way of moving existing graph data structure to persistent memory.

## Quick Start

Build the project:

```
> make TASK=bfs_gap
> make TASK=pagerank_gap
> make TASK=bc_gap
> make TASK=cc_gap
```

Insert Orkut graph and run PageRank:

```
> ./bin/bench-pagerank_gap-persistent \
  ${DATA_PATH}/llama/orkut/com-orkut.ungraph.base.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__1_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__1_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__1_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__1_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__1_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__2_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__2_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__2_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__2_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__2_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__3_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__3_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__3_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__3_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__3_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__4_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__4_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__4_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__4_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__4_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__5_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__5_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__5_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__5_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__5_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__6_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__6_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__6_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__6_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__6_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__7_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__7_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__7_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__7_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__7_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__8_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__8_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__8_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__8_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__8_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__9_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__9_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__9_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__9_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__9_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__10_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__10_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__10_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__10_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__10_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__11_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__11_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__11_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__11_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__11_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__12_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__12_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__12_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__12_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__12_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__13_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__13_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__13_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__13_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__13_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__14_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__14_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__14_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__14_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__14_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__15_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__15_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__15_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__15_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__15_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__16_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__16_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__16_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__16_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__16_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__17_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__17_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__17_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__17_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__17_5.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__18_1.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__18_2.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__18_3.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__18_4.net \
  ${DATA_PATH}/llama/orkut/llama_d1/com-orkut.ungraph.dynamic__18_5.net \
  -L -d ${PMEM_PATH}/llama-orkut-d1/ -v -t 1
```

Meaning of the command line flags:
+ `-L` load the input files into the database
+ `-d dir` store the graph in directory dir
+ `-v` enable verbose output
+ `-t t` use t concurrent threads

## Graph Kernels Included
For a fair comparison, we integrate the following graph algorithms from the GAP Benchmark Suite (GAPBS) into LLAMA.
+ Breadth-First Search (BFS) - direction optimizing
+ PageRank (PR) - iterative method in pull direction
+ Connected Components (CC) - Shiloach-Vishkin
+ Betweenness Centrality (BC) - Brandes

## Executing the Benchmark

We provide a script to automate executing the benchmark on all the input graphs we used in our paper. Before running the benchmark, please follow [this directory structure](https://github.com/DIR-LAB/DGAP/blob/main/PREPROCESS.md) to store the input graphs.

__*Warning:*__ A full run of this benchmark might take a couple of hours to finish. If you want to skip some input files, please comment them in the [benchmark script](https://github.com/DIR-LAB/DGAP/blob/main/llama/benchmark_llama.sh).

Execute the benchmark:
```
> ./benchmark_llama.sh > llama.out
```

## Reference
1. Peter Macko, Virendra J Marathe, Daniel W Margo, and Margo I Seltzer. 2015. Llama: Efficient graph analytics using large multiversioned arrays. In 2015 IEEE 31st International Conference on Data Engineering. IEEE, 363–374.
2. Peter Macko. LLAMA. https://github.com/goatdb/llama. Accessed Oct. 21, 2020.