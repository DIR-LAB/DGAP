# DGAP: Efficient Dynamic Graph Analysis on Persistent Memory
[//]: # (DGAP: Efficient Dynamic Graph Analysis on Persistent Memory [SC'23])

This repo includes the source code of DGAP along with other state-of-the-art dynamic graph processing systems (LLAMA [2], GraphOne [3], and XP-Graph [4]) and data structures (CSR, and BAL), which are ported to run on persistent memory. It also includes PMDK (v1.12.1) with necessary changes required in DGAP (please check in [changes in PMDK](#changes-in-pmdk) for more details).

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.8087836.svg)](https://doi.org/10.5281/zenodo.8087836)

## Citing DGAP:

```
@inproceedings{islam2023dgap,
  title={DGAP: Efficient Dynamic Graph Analysis on Persistent Memory},
  author={Islam, Abdullah Al Raqibul and Dai, Dong},
  booktitle={SC'23: Proceedings of the International Conference for High Performance Computing, Networking, Storage and Analysis},
  year={2023},
}
```
[//]: # (pages={71-80},)
[//]: # (doi={10.1109/CCGrid54584.2022.00016})
[//]: # (organization={ACM})

### Table of Contents
- **[What is DGAP?](#what-is-dgap)**<br>
- **[Directory Structure](#directory-structure)**<br>
- **[Workflow](#workflow)**<br>
    - **[1. Build Binaries](#build-binaries)**<br>
    - **[2. Input Graph Datasets](#input-graph-datasets)**<br>
    - **[3. Run the Benchmarks](#run-the-benchmarks)**<br>
- **[Contribution](#contribution)**<br>
- **[Contact](#contact)**<br>
- **[Reference](#reference)**<br>

## What is DGAP?
DGAP is a framework for efficient dynamic graph analysis on persistent memory. DGAP utilizes the existing PMA-based mutable Compressed Sparse Row (CSR) graph structure with extensive new designs for persistent memory. We demonstrate the benefits of DGAP in graph update and analysis by comparing it to state-of-the-art dynamic graph frameworks on persistent memory, such as XPGraph, LLAMA, and GraphOne.

DGAP is implemented using the PMDK library. The core data structure is approximately [1,500 lines of C++ code](https://github.com/DIR-LAB/DGAP/blob/main/dgap/src/graph.h). We benchmark DGAP and all the competitors on six real-world graphs with synthetic graph insertion patterns. For a fair comparison, we integrate different graph algorithms from the GAP Benchmark Suite (GAPBS) into all the competitors. So, this artifact contains the competitors’ code along with the DGAP. The build/run command and benchmark scripts are also provided individually.

## Directory Structure
At the high level, DGAP repository structure looks like this:

```
.
├── bal/        Blocked Adjacency-List (BAL) implementation on persistent memory
├── csr/        Compressed Sparse Row (CSR) implementation on persistent memory [1]
├── dgap/       DGAP implementation on persistent memory
├── data/       test graph data
├── GraphOne/   GraphOne [3] implementation with graph algorithms from GAPBS [1]
├── llama/      LLAMA [2] implementation with graph algorithms from GAPBS [1]
├── pmdk/       PMDK including our modification (see details in later section)
└── XPGraph/    XP-Graph [4] implementation with graph algorithms from GAPBS [1]
```

# Workflow
## 1. Build Binaries
The code has been implemented in C++ and tested on Ubuntu 20.04 with CMake 3.13.4 and gcc 9.4.0.

### 1.1 Software Prerequisites
* C++11 compiler
* OpenMP
* CMake 3.13.4
* GNU Make 4.2.1
* PMDK (v1.12.1 or later with changes, please check section [Changes in PMDK](#changes-in-pmdk) for details)
* Python 3.8.10

### 1.2 Hardware Prerequisites
We evaluated DGAP with the following machine configuration.
* Server: Dell R740 rack server
* CPU: 2nd generation Intel Xeon Scalable Processor (Gold 6254 @ 3.10 GHz) with 18 physical cores
* Memory: 6 DRAM DIMMs with 32 GB each (totaling 192 GB)
* PMs: 6 Optane DC DIMMs with 128 GB each (768 GB in total)
* Storage: SSD (~512 GB would require to store and preprocess all the input dataset files)
* OS: Ubuntu 20.04, utilizing a Linux kernel version 4.15.0.

DGAP is not yet optimized for multi-socket processors, so we recommend using a single-socket machine or a similar setting to reproduce similar results in the paper. Also, while DGAP runs on any persistent memory device, its high performance is best judged when running on Intel Optane DC Persistent Memory. If desired, we will provide proper guidelines to access our testbed.

### 1.3 Changes in PMDK
Current DGAP implementation is based on PMDK 1.12 (we use the PMDK library only for memory management/allocation purposes).
PMDK has a known limitation in single object memory allocation larger than *0x3FFDFFFC0* bytes (around 16 GB), as the current implementation `does not support cross-zone allocation`.
In DGAP, the *edge array* is allocated as a single object and might require larger space than is allowed by PMDK (~16 GB) for larger graphs.
For such reason, we modified the PMDK by increasing the zone size (as par suggested by PMDK developers).
We provide the forked PMDK repository with our change (as a single commit).

The user is required to install/re-install PMDK to test DGAP for larger graphs (e.g., Twitter, Friendster, Protein, etc.).
It is worth noting that, user can run tests on DGAP with smaller graphs (e.g., Orkut, LiveJournal, CitPatents, etc.) without installing our changes in PMDK.

Installing PMDK using our source repository:
```
## install prerequisites: to build the PMDK libraries on Linux, you may need to install the following required packages on the build system
> sudo apt install autoconf automake pkg-config libglib2.0-dev libfabric-dev pandoc libncurses5-dev

## libndctl is required only by selected PMDK components or features, install ndctl and daxctl packages on Linux
> sudo apt install ndctl daxctl

## to build the master branch run the make utility in the root directory
> cd pmdk
> make

## to install the libraries to the default /usr/local location
> sudo make install prefix=/usr/local
```

### 1.4 Build & Run
**Step 1:** Clone the repository:
```
> git clone https://github.com/DIR-LAB/DGAP.git
```

**Step 2:** Set the `PMEM_PATH` and `DATA_PATH` to link the root directory of PMEM and input graph data respectively. Example:
```
> export PMEM_PATH=/mnt/pmem
> export DATA_PATH=./data
```

**Step 3:** Download and preprocess the datasets by following [these instructions](#prepare-datasets).

**Step 4:** To build and run test on DGAP or any of the competitors, please follow instructions included in the corresponding sub-directories:

1. [Build & Run DGAP](https://github.com/DIR-LAB/DGAP/tree/main/dgap)
2. [Build & Run BAL](https://github.com/DIR-LAB/DGAP/tree/main/bal)
3. [Build & Run CSR](https://github.com/DIR-LAB/DGAP/tree/main/csr)
4. [Build & Run LLAMA](https://github.com/DIR-LAB/DGAP/tree/main/llama)
5. [Build & Run GraphOne](https://github.com/DIR-LAB/DGAP/tree/main/GraphOne)
6. [Build & Run XPGraph](https://github.com/DIR-LAB/DGAP/tree/main/XPGraph)

Please note that, to test concurrent graph insertion and analysis performance, user need to set the number of concurrent threads by overwriting the `OpenMP` environment variable `"OMP_NUM_THREADS"`. Example:
```
> export OMP_NUM_THREADS=16
```

**Step 5:** Reproduce results that we included in our paper. (TBA)

### 1.5 Graph Kernels Included
For a fair comparison, we integrate the following graph algorithms from the GAP Benchmark Suite (GAPBS) into DGAP and all the competitors.

+ Breadth-First Search (BFS) - direction optimizing
+ PageRank (PR) - iterative method in pull direction
+ Connected Components (CC) - Shiloach-Vishkin
+ Betweenness Centrality (BC) - Brandes

[//]: # (## Design and Implementations)
[//]: # (This repository contains the implementation of the following data structures)
[//]: # (* CSR: Baseline for graph analysis performance.)
[//]: # (* VCSR: Our proposed mutable CSR storage format.)
[//]: # (* Blocked Adjacency List &#40;BAL&#41;: Fixed number of edges &#40;e.g., 512 edges in our case&#41; stored per block.)
[//]: # (* [Packed CSR &#40;PCSR&#41;]&#40;https://github.com/wheatman/Packed-Compressed-Sparse-Row&#41; [1]: Another mutable CSR format that also leverages PMA.)

## 2. Input Graph Datasets

Since different graph processing frameworks read dynamic graphs differently. To compare them, we have to do series of data preprocessing first. 

The details are described in a separate page: [Graph Datasets Pre-processing](https://github.com/DIR-LAB/DGAP/blob/main/PREPROCESS.md).

Before moving to that page, we discuss some basic concepts here.

### 2.1 Graph Properties
* **Direction of the graph:** Currently DGAP and all the competitors only store the out-going edges of the graph. Few graph algorithms implemented in the [GAP Benchmark Suite (GAPBS)](https://github.com/sbeamer/gapbs) expect to access both the in and out-going edges.
    * We solve this problem by inserting the inverse edges for the directed graph datasets.
* **Weighted/Property of the graph:** Currently DGAP and all the competitors stores the unweighted graphs. We plan to add support for weighted/property graphs in the future.

### 2.2 Graph Data Format and Conversion
All the dynamic graph processing systems (included in this repository) expect the input graphs in the **edge-list format**. So, for example, the data will look like the following:

```
...
1015    1017
1017    1015
14736   14752
14752   14736
1080    1531
...
```

The GAPBS framework expects `.el` as the default file extension for the unweighted input graphs. So, we used `.el` file extension for DGAP, BAL, CSR, and GraphOne.
LLAMA and XPGraph expect the file extension `.net` and `.bin` respectively.

1. **Prepare Datasets for CSR**: CSR expect a single input graph file in edge graph format. Users just need to convert any ordered edge graph datasets to a randomly shuffled dataset. We provide `shuffle_dataset` command to do that.
2. **Prepare Datasets for DGAP/BAL/GraphOne**. These systems expect two input files (base graph and dynamic graph) for each of the graph datasets. The dataset is in edge graph format, and these files collectively represent the whole graph. Users need to convert any edge graph datasets into two files: the base-graph-output and dynamic-graph-output. Depends on the original graph file format (edge format or adjaccency format), we provide two different commands: `split_dataset` and `adj_to_el_converter`

- **.base.el** (10% base graph file)
- **.dynamic.el** (dynamic graph file).

3. **Prepare Datasets for LLAMA**. LLAMA uses a multi-versioned CSR structure to enable fast graph analysis and graph mutations. The graph updates are conducted in batch and organized as multiple immutable snapshots in LLAMA. LLAMA expect multiple input files in edge graph format. For each of these input files, LLAMA creates a single snapshot.
In our evaluation, we create a snapshot after inserting each 1% of the dynamic graphs. Hence users need to split their dynamic graph files (that we prepared for DGAP, BAL, and GraphOne) into multiple pieces to evaluate LLAMA. We provide a command `create_llama_dataset` for this purpose.

4. **Prepare Datasets for XPGraph**. XPGraph reads edge graph files in binary format. The authors of XPGraph provided a script to convert the input data from text format to binary format. We used their script (`text2bin`) to convert our edge graphs from text format to binary format. 

Again, more details can be seen at [Graph Datasets Pre-processing](https://github.com/DIR-LAB/DGAP/blob/main/PREPROCESS.md) page.

## 3. Run the Benchmarks

One of the key motivations of this project is to allow others to reuse our DGAP implementation for performance comparison. We provide detailed build and run instructions for DGAP and all the competitors in the corresponding sub-directories. Before running the benchmark, please follow [this directory structure](https://github.com/DIR-LAB/DGAP/blob/main/PREPROCESS.md) to store the input graphs. Then, run `benchmark_xxx.sh` scripts in each directory to reproduce the results of DGAP and all the competitors.

## Contribution

If you would like to contribute to this project, we would certainly appreciate your help! Here are some of the ways you can contribute:

* Bug fixes, whether for performance or correctness of the existing data structure and graph algorithm implementation.
* Improvement of the existing documentation.
* Add additional persistent data structure implementations for dynamic graphs; we are open to adding more data structures in this repo. However, please keep in mind that we will only accept new data structures if you either integrate them in the [GAP Benchmark Suite (GAPBS)](https://github.com/sbeamer/gapbs) or use the graph analysis code implemented at GAPBS. In this way, we will be able to make comparable performance analyses in the future.

Our future goal is to provide a set of portable, high-performance data structure baselines for dynamic graphs. For code contributions, please focus on code simplicity and readability. If you open a pull request, we will do a quick sanity check and respond to you as soon as possible.

## Contact
* [Abdullah Al Raqibul Islam (UNC Charlotte)](http://biqar.github.io)
* [Dong Dai (UNC Charlotte)](http://daidong.github.io)

## Reference
1. Scott Beamer. GAP Benchmark Suite. https://github.com/sbeamer/gapbs. Accessed July. 30, 2021.
2. Peter Macko, Virendra J Marathe, Daniel W Margo, and Margo I Seltzer. 2015. Llama: Efficient graph analytics using large multiversioned arrays. In 2015 IEEE 31st International Conference on Data Engineering. IEEE, 363–374.
3. Pradeep Kumar and H Howie Huang. 2019. Graphone: A data store for real-time analytics on evolving graphs. In 17th {USENIX} Conference on File and Storage Technologies ({FAST} 19). 249–263.
4. Rui Wang, Shuibing He, Weixu Zong, Yongkun Li, and Yinlong Xu. 2022. XP-Graph: XPline-Friendly Persistent Memory Graph Stores for Large-Scale Evolving Graphs. In 2022 55th IEEE/ACM International Symposium on Microarchitecture (MICRO). IEEE, 1308–1325.
5. PMEM. PMDK: Persistent Memory Development Kit. https://github.com/pmem/pmdk. Accessed [v1.12.1](https://github.com/pmem/pmdk/releases/tag/1.12.1).
