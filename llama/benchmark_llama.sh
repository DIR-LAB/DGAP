#!/bin/bash

./../validate.sh || exit -1

echo 'pmem path: ' $PMEM_PATH
echo 'data path: ' $DATA_PATH

make TASK=bfs_gap
make TASK=pagerank_gap
make TASK=bc_gap
make TASK=cc_gap

threads=1
while [ $threads -le 16 ]
do
  export OMP_NUM_THREADS=$threads
  echo
  echo
  echo '===================================================================='
  echo 'concurrent threads: ' $((OMP_NUM_THREADS))

  # Dataset: orkut
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: ORKUT>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"

  taskset --cpu-list 0-70:2 ./bin/bench-pagerank_gap-persistent \
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
  -L -d ${PMEM_PATH}/llama-orkut-d1/ -v -t $threads

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bfs_gap-persistent -d ${PMEM_PATH}/llama-orkut-d1/ -v -R 1 -c 5 -t $threads

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bc_gap-persistent -d ${PMEM_PATH}/llama-orkut-d1/ -v -R 1 -c 5 -t $threads

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-cc_gap-persistent -d ${PMEM_PATH}/llama-orkut-d1/ -v -R 1 -c 5 -t $threads

  rm -rf ${PMEM_PATH}/llama-orkut-d1/

  # Dataset: livejournal
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: LIVEJOURNAL>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"

  taskset --cpu-list 0-70:2 ./bin/bench-pagerank_gap-persistent \
  ${DATA_PATH}/llama/livej/soc-LiveJournal1.base.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__1_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__1_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__1_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__1_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__1_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__2_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__2_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__2_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__2_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__2_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__3_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__3_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__3_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__3_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__3_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__4_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__4_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__4_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__4_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__4_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__5_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__5_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__5_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__5_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__5_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__6_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__6_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__6_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__6_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__6_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__7_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__7_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__7_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__7_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__7_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__8_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__8_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__8_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__8_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__8_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__9_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__9_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__9_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__9_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__9_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__10_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__10_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__10_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__10_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__10_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__11_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__11_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__11_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__11_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__11_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__12_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__12_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__12_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__12_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__12_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__13_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__13_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__13_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__13_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__13_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__14_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__14_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__14_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__14_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__14_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__15_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__15_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__15_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__15_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__15_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__16_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__16_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__16_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__16_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__16_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__17_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__17_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__17_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__17_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__17_5.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__18_1.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__18_2.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__18_3.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__18_4.net \
  ${DATA_PATH}/llama/livej/llama_d1/soc-LiveJournal1.dynamic__18_5.net \
  -L -d ${PMEM_PATH}/llama-lj-d1/ -v -t $threads

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bfs_gap-persistent -d ${PMEM_PATH}/llama-lj-d1/ -v -R 0 -c 5 -t $threads

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bc_gap-persistent -d ${PMEM_PATH}/llama-lj-d1/ -v -R 0 -c 5 -t $threads

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-cc_gap-persistent -d ${PMEM_PATH}/llama-lj-d1/ -v -R 0 -c 5 -t $threads

  rm -rf ${PMEM_PATH}/llama-lj-d1/

  # Dataset: citation
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: CITATION>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"

  taskset --cpu-list 0-70:2 ./bin/bench-pagerank_gap-persistent \
  ${DATA_PATH}/llama/citation/cit-Patents.base.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__1_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__1_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__1_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__1_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__1_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__2_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__2_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__2_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__2_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__2_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__3_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__3_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__3_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__3_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__3_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__4_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__4_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__4_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__4_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__4_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__5_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__5_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__5_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__5_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__5_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__6_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__6_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__6_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__6_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__6_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__7_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__7_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__7_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__7_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__7_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__8_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__8_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__8_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__8_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__8_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__9_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__9_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__9_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__9_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__9_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__10_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__10_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__10_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__10_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__10_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__11_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__11_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__11_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__11_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__11_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__12_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__12_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__12_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__12_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__12_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__13_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__13_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__13_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__13_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__13_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__14_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__14_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__14_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__14_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__14_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__15_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__15_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__15_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__15_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__15_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__16_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__16_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__16_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__16_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__16_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__17_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__17_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__17_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__17_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__17_5.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__18_1.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__18_2.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__18_3.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__18_4.net \
  ${DATA_PATH}/llama/citation/llama_d1/cit-Patents.dynamic__18_5.net \
  -L -d ${PMEM_PATH}/llama-cit-d1/ -v -t $threads

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bfs_gap-persistent -d ${PMEM_PATH}/llama-cit-d1/ -v -R 1 -c 5 -t $threads

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bc_gap-persistent -d ${PMEM_PATH}/llama-cit-d1/ -v -R 1 -c 5 -t $threads

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-cc_gap-persistent -d ${PMEM_PATH}/llama-cit-d1/ -v -R 1 -c 5 -t $threads

  rm -rf ${PMEM_PATH}/llama-cit-d1/

  # Dataset: Twitter
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Twitter>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"

  taskset --cpu-list 0-70:2 ./bin/bench-pagerank_gap-persistent \
  ${DATA_PATH}/llama/twitter_10/twitter-unique-undir.base.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__1.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__2.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__3.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__4.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__5.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__6.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__7.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__8.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__9.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__10.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__11.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__12.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__13.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__14.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__15.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__16.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__17.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__18.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__19.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__20.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__21.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__22.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__23.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__24.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__25.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__26.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__27.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__28.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__29.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__30.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__31.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__32.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__33.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__34.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__35.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__36.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__37.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__38.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__39.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__40.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__41.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__42.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__43.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__44.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__45.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__46.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__47.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__48.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__49.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__50.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__51.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__52.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__53.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__54.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__55.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__56.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__57.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__58.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__59.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__60.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__61.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__62.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__63.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__64.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__65.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__66.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__67.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__68.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__69.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__70.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__71.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__72.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__73.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__74.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__75.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__76.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__77.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__78.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__79.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__80.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__81.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__82.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__83.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__84.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__85.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__86.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__87.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__88.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__89.net \
  ${DATA_PATH}/llama/twitter_10/llama_d1/twitter-unique-undir.dynamic__90.net \
  -L -d ${PMEM_PATH}/llama-twitter-d1/ -v -t $threads

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bfs_gap-persistent -d ${PMEM_PATH}/llama-twitter-d1/ -v -R 12 -c 2 -t $threads

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bc_gap-persistent -d ${PMEM_PATH}/llama-twitter-d1/ -v -R 12 -c 2 -t $threads

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-cc_gap-persistent -d ${PMEM_PATH}/llama-twitter-d1/ -v -R 12 -c 2 -t $threads

  rm -rf ${PMEM_PATH}/llama-twitter-d1/

  # Dataset: friendster
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Friendster>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"

  taskset --cpu-list 0-70:2 ./bin/bench-pagerank_gap-persistent \
  ${DATA_PATH}/friend/com-friendster.base.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__1.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__2.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__3.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__4.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__5.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__6.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__7.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__8.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__9.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__10.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__11.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__12.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__13.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__14.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__15.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__16.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__17.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__18.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__19.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__20.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__21.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__22.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__23.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__24.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__25.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__26.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__27.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__28.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__29.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__30.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__31.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__32.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__33.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__34.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__35.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__36.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__37.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__38.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__39.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__40.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__41.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__42.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__43.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__44.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__45.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__46.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__47.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__48.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__49.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__50.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__51.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__52.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__53.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__54.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__55.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__56.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__57.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__58.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__59.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__60.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__61.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__62.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__63.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__64.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__65.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__66.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__67.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__68.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__69.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__70.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__71.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__72.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__73.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__74.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__75.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__76.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__77.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__78.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__79.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__80.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__81.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__82.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__83.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__84.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__85.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__86.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__87.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__88.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__89.net \
  ${DATA_PATH}/friend/llama_d1/com-friendster.dynamic__90.net \
  -L -d ${PMEM_PATH}/llama-friend-d1/ -v -t $threads

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bfs_gap-persistent -d ${PMEM_PATH}/llama-friend-d1/ -v -R 101 -c 1 -t $threads

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bc_gap-persistent -d ${PMEM_PATH}/llama-friend-d1/ -v -R 101 -c 1 -t $threads

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-cc_gap-persistent -d ${PMEM_PATH}/llama-friend-d1/ -v -R 101 -c 1 -t $threads

  rm -rf ${PMEM_PATH}/llama-friend-d1/

  # Dataset: protein
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Protein>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"

  taskset --cpu-list 0-70:2 ./bin/bench-pagerank_gap-persistent \
  ${DATA_PATH}/protein/protein.base.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__1.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__2.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__3.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__4.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__5.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__6.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__7.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__8.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__9.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__10.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__11.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__12.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__13.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__14.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__15.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__16.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__17.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__18.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__19.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__20.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__21.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__22.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__23.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__24.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__25.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__26.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__27.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__28.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__29.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__30.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__31.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__32.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__33.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__34.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__35.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__36.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__37.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__38.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__39.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__40.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__41.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__42.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__43.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__44.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__45.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__46.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__47.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__48.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__49.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__50.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__51.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__52.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__53.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__54.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__55.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__56.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__57.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__58.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__59.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__60.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__61.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__62.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__63.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__64.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__65.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__66.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__67.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__68.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__69.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__70.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__71.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__72.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__73.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__74.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__75.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__76.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__77.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__78.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__79.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__80.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__81.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__82.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__83.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__84.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__85.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__86.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__87.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__88.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__89.net \
  ${DATA_PATH}/protein/llama_d1/protein.dynamic__90.net \
  -L -d ${PMEM_PATH}/llama-protein-d1/ -v -t $threads

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bfs_gap-persistent -d ${PMEM_PATH}/llama-protein-d1/ -v -R 35 -c 1 -t $threads

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-bc_gap-persistent -d ${PMEM_PATH}/llama-protein-d1/ -v -R 35 -c 1 -t $threads

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bin/bench-cc_gap-persistent -d ${PMEM_PATH}/llama-protein-d1/ -v -R 35 -c 1 -t $threads

  rm -rf ${PMEM_PATH}/llama-protein-d1/

  ((threads*=2))
  echo 'Done'
done