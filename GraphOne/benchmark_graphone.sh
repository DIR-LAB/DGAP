#!/bin/bash

./../validate.sh || exit -1

echo 'pmem path: ' $PMEM_PATH
echo 'data path: ' $DATA_PATH

mkdir $PMEM_PATH/GraphOne/

threads=1
while [ $threads -le 16 ]
do
  export OMP_NUM_THREADS=$threads
  echo
  echo
  echo '===================================================================='
  echo 'concurrent threads: ' $((threads))

  # Dataset: orkut
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: ORKUT>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/orkut_10/com-orkut.ungraph.base.el -d ${DATA_PATH}/orkut_10/com-orkut.ungraph.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 3072627 -j 0 -s 1 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/orkut_10/com-orkut.ungraph.base.el -d ${DATA_PATH}/orkut_10/com-orkut.ungraph.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 3072627 -j 1 -s 1 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/orkut_10/com-orkut.ungraph.base.el -d ${DATA_PATH}/orkut_10/com-orkut.ungraph.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 3072627 -j 2 -s 1 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/orkut_10/com-orkut.ungraph.base.el -d ${DATA_PATH}/orkut_10/com-orkut.ungraph.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 3072627 -j 3 -s 1 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  # Dataset: livejournal
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: LIVEJOURNAL>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/live-journal_10/soc-LiveJournal1.base.el -d ${DATA_PATH}/live-journal_10/soc-LiveJournal1.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 4847571 -j 0 -s 0 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/live-journal_10/soc-LiveJournal1.base.el -d ${DATA_PATH}/live-journal_10/soc-LiveJournal1.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 4847571 -j 1 -s 0 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/live-journal_10/soc-LiveJournal1.base.el -d ${DATA_PATH}/live-journal_10/soc-LiveJournal1.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 4847571 -j 2 -s 0 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/live-journal_10/soc-LiveJournal1.base.el -d ${DATA_PATH}/live-journal_10/soc-LiveJournal1.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 4847571 -j 3 -s 0 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  # Dataset: citation
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: CITATION>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/cit-Patents_10/cit-Patents.base.el -d ${DATA_PATH}/cit-Patents_10/cit-Patents.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 6009555 -j 0 -s 1 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/cit-Patents_10/cit-Patents.base.el -d ${DATA_PATH}/cit-Patents_10/cit-Patents.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 6009555 -j 1 -s 1 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/cit-Patents_10/cit-Patents.base.el -d ${DATA_PATH}/cit-Patents_10/cit-Patents.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 6009555 -j 2 -s 1 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/cit-Patents_10/cit-Patents.base.el -d ${DATA_PATH}/cit-Patents_10/cit-Patents.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 6009555 -j 3 -s 1 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  # Dataset: twitter
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Twitter>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/twitter_10/twitter-unique-undir.base.el -d ${DATA_PATH}/twitter_10/twitter-unique-undir.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 61578415 -j 0 -s 12 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/twitter_10/twitter-unique-undir.base.el -d ${DATA_PATH}/twitter_10/twitter-unique-undir.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 61578415 -j 1 -s 12 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/twitter_10/twitter-unique-undir.base.el -d ${DATA_PATH}/twitter_10/twitter-unique-undir.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 61578415 -j 2 -s 12 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/twitter_10/twitter-unique-undir.base.el -d ${DATA_PATH}/twitter_10/twitter-unique-undir.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 61578415 -j 3 -s 12 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  # Dataset: friendster
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Friendster>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/friend_10/com-friendster.base.el -d ${DATA_PATH}/friend_10/com-friendster.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 124836180 -j 0 -s 101 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/friend_10/com-friendster.base.el -d ${DATA_PATH}/friend_10/com-friendster.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 124836180 -j 1 -s 101 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/friend_10/com-friendster.base.el -d ${DATA_PATH}/friend_10/com-friendster.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 124836180 -j 2 -s 101 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/friend_10/com-friendster.base.el -d ${DATA_PATH}/friend_10/com-friendster.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 124836180 -j 3 -s 101 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  # Dataset: protein
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Protein>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/protein/protein.base.el -d ${DATA_PATH}/protein/protein.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 8745543 -j 0 -s 35 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/protein/protein.base.el -d ${DATA_PATH}/protein/protein.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 8745543 -j 1 -s 35 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/protein/protein.base.el -d ${DATA_PATH}/protein/protein.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 8745543 -j 2 -s 35 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./build/graphone_test -b ${DATA_PATH}/protein/protein.base.el -d ${DATA_PATH}/protein/protein.dynamic.el -o ${PMEM_PATH}/GraphOne/ -v 8745543 -j 3 -s 35 -t $threads

  rm ${PMEM_PATH}/GraphOne/*

  ((threads*=2))
  echo 'Done'
done