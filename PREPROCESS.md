# Prepare Datasets

DGAP provides interfaces to ingest graph data from edge list files. In the paper, we reported DGAP’s performance on the following six input graphs:
1. [Orkut](https://snap.stanford.edu/data/com-Orkut.html)
2. [LiveJournal](https://snap.stanford.edu/data/soc-LiveJournal1.html)
3. [CitPatents](https://snap.stanford.edu/data/cit-Patents.html)
4. [Twitter](https://github.com/ANLAB-KAIST/traces/releases/tag/twitter_rv.net)
5. [Friendster](https://snap.stanford.edu/data/com-Friendster.html)
6. [Protein](https://www.dropbox.com/scl/fi/kx883cz7d5w8p8n346nr0/protein.adj?dl=0&rlkey=6voyszorfex9lrb2edja4w65y)

Many competitors expect graphs into different formats. Hence, user requires to preprocess the datasets before running test on them.
For your convenience, we shared all the preprocessed files of Orkut graph to run tests on DGAP and all the competitors in the `data` directory. We also left the empty directories so that user can fill them with the preprocessing scripts that we provided in [preprocess directory](https://github.com/DIR-LAB/DGAP/tree/main/preprocess).
The `data` directory currently looks like this:

```
data/
├── cit-Patents_10
├── live-journal_10
├── llama
│   ├── citation
│   │   └── llama_d1
│   ├── friend
│   │   └── llama_d1
│   ├── livej
│   │   ├── llama_d1
│   ├── orkut
│   │   ├── com-orkut.ungraph.base.net
│   │   └── llama_d1
│   │       ├── com-orkut.ungraph.dynamic__10_1.net
│   │       ├── ... (89 more files)
│   ├── protein
│   │   └── llama_d1
│   └── twitter_10
│       └── llama_d1
├── orkut_10
│   ├── com-orkut.ungraph.base.el
│   └── com-orkut.ungraph.dynamic.el
├── single-file
│   ├── com-orkut.ungraph.el
└── xpgraph
    ├── citation
    ├── friend
    ├── livej
    ├── orkut
    │   ├── com-orkut.ungraph.base.bin
    │   └── com-orkut.ungraph.dynamic.bin
    ├── protein
    └── twitter
```

Here we shared the step-by-step process to preprocess the datasets:

**Step 1:** Set the `DATA_PATH` to link the root directory of input graph data. Example:
```
> export DATA_PATH=[path-to-dgap]/DGAP/data
```

**Step 2:** Download all the raw input files and place them at `DATA_PATH`.

```
> mkdir $DATA_PATH/raw_graph_files/
> cd $DATA_PATH/raw_graph_files/

## download the raw files under the current directory
```

**Step 3:** Build the processing scripts. Example:

```
> cd [path-to-dgap]/DGAP/preprocess
> make
```

**Step 4:** Preprocess the raw files for CSR. Example:

```
## if the raw graph in edge graph format
> ./shuffle_dataset [path-to-input-file]/input.el [path-to-output-file]/output.el [number-of-lines-in-inout-file]
```

**Step 5:** Preprocess the raw files for DGAP/BAL/GraphOne. Example:

```
## if the raw graph in edge graph format
> ./split_dataset [path-to-input-file]/input.el [path-to-base-graph-output-file]/output.base.el [path-to-dynamic-graph-output-file]/output.dynamic.el [number-of-lines-in-inout-file]

## if the raw graph in adjacency graph format
> ./adj_to_el_converter [path-to-input-file]/input.el [path-to-base-graph-output-file]/output.base.el [path-to-dynamic-graph-output-file]/output.dynamic.el
```

**Step 5:** Preprocess the raw files for DGAP/BAL/GraphOne. Example:

```
## if the raw graph in edge graph format
> ./split_dataset [path-to-input-file]/input.el [path-to-base-graph-output-file]/output.base.el [path-to-dynamic-graph-output-file]/output.dynamic.el [number-of-lines-in-inout-file]

## if the raw graph in adjacency graph format
> ./adj_to_el_converter [path-to-input-file]/input.el [path-to-base-graph-output-file]/output.base.el [path-to-dynamic-graph-output-file]/output.dynamic.el
```

**Step 6:** Preprocess the dynamic graph files for LLAMA. Here we will use the dynamic graph files generated in `Step 5` as the input of the preprocessor. Example:

```
> ./create_llama_dataset [path-to-input-dynamic-graph-file]/input.dynamic.el [directory-path-to-output-file]/ [number-of-lines-in-inout-file] [number-of-splits]
```

**Step 7:** Preprocess files for XPGraph. Here we will use the base and dynamic graph files generated in `Step 5` as the input of the preprocessor. Example:

```
> ./text2bin [path-to-input-text-file]/data.el [path-to-output-binary-file]/data.bin
```

