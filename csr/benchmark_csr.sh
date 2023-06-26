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
  taskset --cpu-list 0-70:2 ./pr -f ${DATA_PATH}/single-file/com-orkut.ungraph.el -d ${PMEM_PATH}/orkut_csr.db -r 1 -n 5 -a

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -f ${DATA_PATH}/single-file/com-orkut.ungraph.el -d ${PMEM_PATH}/orkut_csr.db -r 1 -n 5 -a

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -f ${DATA_PATH}/single-file/com-orkut.ungraph.el -d ${PMEM_PATH}/orkut_csr.db -r 1 -n 5 -a -i 1

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -f ${DATA_PATH}/single-file/com-orkut.ungraph.el -d ${PMEM_PATH}/orkut_csr.db -r 1 -n 5 -a

  ((threads*=2))
  echo 'Done'
done

  rm ${PMEM_PATH}/orkut_csr.db

threads=1
while [ $threads -le 16 ]
do
  export OMP_NUM_THREADS=$threads
  echo
  echo
  echo '===================================================================='
  echo 'concurrent threads: ' $((OMP_NUM_THREADS))

  # Dataset: livejournal
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: LIVEJOURNAL>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./pr -f ${DATA_PATH}/single-file/soc-LiveJournal1.el -d ${PMEM_PATH}/lj_csr.db -r 0 -n 5 -a

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -f ${DATA_PATH}/single-file/soc-LiveJournal1.el -d ${PMEM_PATH}/lj_csr.db -r 0 -n 5 -a

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -f ${DATA_PATH}/single-file/soc-LiveJournal1.el -d ${PMEM_PATH}/lj_csr.db -r 0 -n 5 -a -i 1

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -f ${DATA_PATH}/single-file/soc-LiveJournal1.el -d ${PMEM_PATH}/lj_csr.db -r 0 -n 5 -a

  ((threads*=2))
  echo 'Done'
done

  rm ${PMEM_PATH}/lj_csr.db

threads=1
while [ $threads -le 16 ]
do
  export OMP_NUM_THREADS=$threads
  echo
  echo
  echo '===================================================================='
  echo 'concurrent threads: ' $((OMP_NUM_THREADS))

  # Dataset: citation
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: CITATION>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./pr -f ${DATA_PATH}/single-file/cit-Patents.el -d ${PMEM_PATH}/cit_csr.db -r 1 -n 5 -a

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -f ${DATA_PATH}/single-file/cit-Patents.el -d ${PMEM_PATH}/cit_csr.db -r 1 -n 5 -a

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -f ${DATA_PATH}/single-file/cit-Patents.el -d ${PMEM_PATH}/cit_csr.db -r 1 -n 5 -a -i 1

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -f ${DATA_PATH}/single-file/cit-Patents.el -d ${PMEM_PATH}/cit_csr.db -r 1 -n 5 -a

  ((threads*=2))
  echo 'Done'
done

  rm ${PMEM_PATH}/cit_csr.db

threads=1
while [ $threads -le 16 ]
do
  export OMP_NUM_THREADS=$threads
  echo
  echo
  echo '===================================================================='
  echo 'concurrent threads: ' $((OMP_NUM_THREADS))

  # Dataset: twitter
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Twitter>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./pr -f ${DATA_PATH}/single-file/twitter-unique-undir.el -d ${PMEM_PATH}/twitter.pmem -r 12 -n 2 -a

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -f ${DATA_PATH}/single-file/twitter-unique-undir.el -d ${PMEM_PATH}/twitter.pmem -r 12 -n 2 -a

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -f ${DATA_PATH}/single-file/twitter-unique-undir.el -d ${PMEM_PATH}/twitter.pmem -r 12 -n 2 -a -i 1

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -f ${DATA_PATH}/single-file/twitter-unique-undir.el -d ${PMEM_PATH}/twitter.pmem -r 12 -n 2 -a

  ((threads*=2))
  echo 'Done'
done

  rm ${PMEM_PATH}/twitter.pmem

threads=1
while [ $threads -le 16 ]
do
  export OMP_NUM_THREADS=$threads
  echo
  echo
  echo '===================================================================='
  echo 'concurrent threads: ' $((OMP_NUM_THREADS))

  # Dataset: friendster
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Friendster>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./pr -f ${DATA_PATH}/single-file/com-friendster.el -d ${PMEM_PATH}/friend.pmem -r 101 -n 2 -a

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -f ${DATA_PATH}/single-file/com-friendster.el -d ${PMEM_PATH}/friend.pmem -r 101 -n 2 -a

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -f ${DATA_PATH}/single-file/com-friendster.el -d ${PMEM_PATH}/friend.pmem -r 101 -n 2 -a -i 1

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -f ${DATA_PATH}/single-file/com-friendster.el -d ${PMEM_PATH}/friend.pmem -r 101 -n 2 -a

  ((threads*=2))
  echo 'Done'
done

  rm ${PMEM_PATH}/friend.pmem

threads=1
while [ $threads -le 16 ]
do
  export OMP_NUM_THREADS=$threads
  echo
  echo
  echo '===================================================================='
  echo 'concurrent threads: ' $((OMP_NUM_THREADS))

  # Dataset: protein
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Protein>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  echo "<<<<<<<<<ALGO: PAGERANK>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./pr -f ${DATA_PATH}/single-file/protein.el -d ${PMEM_PATH}/protein.pmem -r 35 -n 2 -a

  echo
  echo "<<<<<<<<<ALGO: BFS>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bfs -f ${DATA_PATH}/single-file/protein.el -d ${PMEM_PATH}/protein.pmem -r 35 -n 2 -a

  echo
  echo "<<<<<<<<<ALGO: BC>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./bc -f ${DATA_PATH}/single-file/protein.el -d ${PMEM_PATH}/protein.pmem -r 35 -n 2 -a -i 1

  echo
  echo "<<<<<<<<<ALGO: CC_SV>>>>>>>>>"
  taskset --cpu-list 0-70:2 ./cc_sv -f ${DATA_PATH}/single-file/protein.el -d ${PMEM_PATH}/protein.pmem -r 35 -n 2 -a

  ((threads*=2))
  echo 'Done'
done

  rm ${PMEM_PATH}/protein.pmem