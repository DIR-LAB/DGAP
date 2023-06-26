#!/bin/bash

./../validate.sh || exit -1

echo 'pmem path: ' $PMEM_PATH
echo 'data path: ' $DATA_PATH

make clean && make

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
  taskset --cpu-list 0-70:2 ./pr -B ${DATA_PATH}/orkut_10/com-orkut.ungraph.base.el -D ${DATA_PATH}/orkut_10/com-orkut.ungraph.dynamic.el -f ${PMEM_PATH}/orkut.db -r 1 -n 5 -a

  rm ${PMEM_PATH}/orkut.db

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -B ${DATA_PATH}/orkut_10/com-orkut.ungraph.base.el -D ${DATA_PATH}/orkut_10/com-orkut.ungraph.dynamic.el -f ${PMEM_PATH}/orkut.db -r 1 -n 5 -a

  rm ${PMEM_PATH}/orkut.db

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -B ${DATA_PATH}/orkut_10/com-orkut.ungraph.base.el -D ${DATA_PATH}/orkut_10/com-orkut.ungraph.dynamic.el -f ${PMEM_PATH}/orkut.db -r 1 -n 5 -a -i 1

  rm ${PMEM_PATH}/orkut.db

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -B ${DATA_PATH}/orkut_10/com-orkut.ungraph.base.el -D ${DATA_PATH}/orkut_10/com-orkut.ungraph.dynamic.el -f ${PMEM_PATH}/orkut.db -r 1 -n 5 -a

  rm ${PMEM_PATH}/orkut.db

  # Dataset: livejournal
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: LIVEJOURNAL>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./pr -B ${DATA_PATH}/live-journal_10/soc-LiveJournal1.base.el -D ${DATA_PATH}/live-journal_10/soc-LiveJournal1.dynamic.el -f ${PMEM_PATH}/lj.db -r 0 -n 5 -a

  rm ${PMEM_PATH}/lj.db

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -B ${DATA_PATH}/live-journal_10/soc-LiveJournal1.base.el -D ${DATA_PATH}/live-journal_10/soc-LiveJournal1.dynamic.el -f ${PMEM_PATH}/lj.db -r 0 -n 5 -a

  rm ${PMEM_PATH}/lj.db

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -B ${DATA_PATH}/live-journal_10/soc-LiveJournal1.base.el -D ${DATA_PATH}/live-journal_10/soc-LiveJournal1.dynamic.el -f ${PMEM_PATH}/lj.db -r 0 -n 5 -a -i 1

  rm ${PMEM_PATH}/lj.db

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -B ${DATA_PATH}/live-journal_10/soc-LiveJournal1.base.el -D ${DATA_PATH}/live-journal_10/soc-LiveJournal1.dynamic.el -f ${PMEM_PATH}/lj.db -r 0 -n 5 -a

  rm ${PMEM_PATH}/lj.db

  # Dataset: citation
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: CITATION>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./pr -B ${DATA_PATH}/cit-Patents_10/cit-Patents.base.el -D ${DATA_PATH}/cit-Patents_10/cit-Patents.dynamic.el -f ${PMEM_PATH}/cit.db -r 1 -n 5 -a

  rm ${PMEM_PATH}/cit.db

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -B ${DATA_PATH}/cit-Patents_10/cit-Patents.base.el -D ${DATA_PATH}/cit-Patents_10/cit-Patents.dynamic.el -f ${PMEM_PATH}/cit.db -r 1 -n 5 -a

  rm ${PMEM_PATH}/cit.db

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -B ${DATA_PATH}/cit-Patents_10/cit-Patents.base.el -D ${DATA_PATH}/cit-Patents_10/cit-Patents.dynamic.el -f ${PMEM_PATH}/cit.db -r 1 -n 5 -a -i 1

  rm ${PMEM_PATH}/cit.db

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -B ${DATA_PATH}/cit-Patents_10/cit-Patents.base.el -D ${DATA_PATH}/cit-Patents_10/cit-Patents.dynamic.el -f ${PMEM_PATH}/cit.db -r 1 -n 5 -a

  rm ${PMEM_PATH}/cit.db


  # Dataset: twitter
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Twitter>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./pr -B ${DATA_PATH}/twitter_10/twitter-unique-undir.base.el -D ${DATA_PATH}/twitter_10/twitter-unique-undir.dynamic.el -f ${PMEM_PATH}/twitter.pmem -r 12 -n 2 -a

  rm ${PMEM_PATH}/twitter.pmem

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -B ${DATA_PATH}/twitter_10/twitter-unique-undir.base.el -D ${DATA_PATH}/twitter_10/twitter-unique-undir.dynamic.el -f ${PMEM_PATH}/twitter.pmem -r 12 -n 2 -a

  rm ${PMEM_PATH}/twitter.pmem

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -B ${DATA_PATH}/twitter_10/twitter-unique-undir.base.el -D ${DATA_PATH}/twitter_10/twitter-unique-undir.dynamic.el -f ${PMEM_PATH}/twitter.pmem -r 12 -n 2 -a -i 1

  rm ${PMEM_PATH}/twitter.pmem

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -B ${DATA_PATH}/twitter_10/twitter-unique-undir.base.el -D ${DATA_PATH}/twitter_10/twitter-unique-undir.dynamic.el -f ${PMEM_PATH}/twitter.pmem -r 12 -n 2 -a

  rm ${PMEM_PATH}/twitter.pmem

  # Dataset: friendster
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Friendster>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./pr -B ${DATA_PATH}/friend_10/com-friendster.base.el -D ${DATA_PATH}/friend_10/com-friendster.dynamic.el -f ${PMEM_PATH}/friend.pmem -r 101 -n 2 -a

  rm ${PMEM_PATH}/friend.pmem

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -B ${DATA_PATH}/friend_10/com-friendster.base.el -D ${DATA_PATH}/friend_10/com-friendster.dynamic.el -f ${PMEM_PATH}/friend.pmem -r 101 -n 2 -a

  rm ${PMEM_PATH}/friend.pmem

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -B ${DATA_PATH}/friend_10/com-friendster.base.el -D ${DATA_PATH}/friend_10/com-friendster.dynamic.el -f ${PMEM_PATH}/friend.pmem -r 101 -n 2 -a -i 1

  rm ${PMEM_PATH}/friend.pmem

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -B ${DATA_PATH}/friend_10/com-friendster.base.el -D ${DATA_PATH}/friend_10/com-friendster.dynamic.el -f ${PMEM_PATH}/friend.pmem -r 101 -n 2 -a

  rm ${PMEM_PATH}/friend.pmem

  # Dataset: protein
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Protein>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./pr -B ${DATA_PATH}/protein/protein.base.el -D ${DATA_PATH}/protein/protein.dynamic.el -f ${PMEM_PATH}/protein.pmem -r 35 -n 2 -a

  rm ${PMEM_PATH}/protein.pmem

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -B ${DATA_PATH}/protein/protein.base.el -D ${DATA_PATH}/protein/protein.dynamic.el -f ${PMEM_PATH}/protein.pmem -r 35 -n 2 -a

  rm ${PMEM_PATH}/protein.pmem

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -B ${DATA_PATH}/protein/protein.base.el -D ${DATA_PATH}/protein/protein.dynamic.el -f ${PMEM_PATH}/protein.pmem -r 35 -n 2 -a -i 1

  rm ${PMEM_PATH}/protein.pmem

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -B ${DATA_PATH}/protein/protein.base.el -D ${DATA_PATH}/protein/protein.dynamic.el -f ${PMEM_PATH}/protein.pmem -r 35 -n 2 -a

  rm ${PMEM_PATH}/protein.pmem

  ((threads*=2))
  echo 'Done'
done