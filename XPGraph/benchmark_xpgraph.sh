#!/bin/bash

./../validate.sh || exit -1

echo 'pmem path: ' $PMEM_PATH
echo 'data path: ' $DATA_PATH

## Make libary file
make lib

## Set head file and LD_LIBRARY_PATH
sudo cp ./src/api/libxpgraph.h /usr/include/
export LD_LIBRARY_PATH=./src/api/lib/:$LD_LIBRARY_PATH

## Make executable file
make main

## Create directory to store db files in PMEM
mkdir ${PMEM_PATH}/xpgraph-db/

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
  taskset --cpu-list 0-70:2 ./bin/main -f ${DATA_PATH}/xpgraph/orkut/ -p0 ${PMEM_PATH}/xpgraph-db/XPGraph0/ --recovery ${PMEM_PATH}/xpgraph-db/XPGraphRecovery/ --source 1 -v 3072627 -q 5 -j 2 -t $threads

  rm -rf ${PMEM_PATH}/xpgraph-db/*

  # Dataset: livejournal
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: LIVEJOURNAL>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  taskset --cpu-list 0-70:2 ./bin/main -f ${DATA_PATH}/xpgraph/livej/ -p0 ${PMEM_PATH}/xpgraph-db/XPGraph0/ --recovery ${PMEM_PATH}/xpgraph-db/XPGraphRecovery/ --source 0 -v 4847571 -q 5 -j 2 -t $threads

  rm -rf ${PMEM_PATH}/xpgraph-db/*

  # Dataset: citation
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: CITATION>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  taskset --cpu-list 0-70:2 ./bin/main -f ${DATA_PATH}/xpgraph/citation/ -p0 ${PMEM_PATH}/xpgraph-db/XPGraph0/ --recovery ${PMEM_PATH}/xpgraph-db/XPGraphRecovery/ --source 1 -v 6009555 -q 5 -j 2 -t $threads

  rm -rf ${PMEM_PATH}/xpgraph-db/*

  # Dataset: twitter
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Twitter>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  taskset --cpu-list 0-70:2 ./bin/main -f ${DATA_PATH}/xpgraph/twitter/ -p0 ${PMEM_PATH}/xpgraph-db/XPGraph0/ --recovery ${PMEM_PATH}/xpgraph-db/XPGraphRecovery/ --source 12 -v 61578415 -q 5 -j 2 -t $threads

  rm -rf ${PMEM_PATH}/xpgraph-db/*

  # Dataset: friendster
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Friendster>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  taskset --cpu-list 0-70:2 ./bin/main -f ${DATA_PATH}/xpgraph/friend/ -p0 ${PMEM_PATH}/xpgraph-db/XPGraph0/ --recovery ${PMEM_PATH}/xpgraph-db/XPGraphRecovery/ --source 101 -v 124836180 -q 5 -j 2 -t $threads

  rm -rf ${PMEM_PATH}/xpgraph-db/*

  # Dataset: protein
  echo "~~~~~~~~~~~~~~~~~~~~~~~~~<DATASET: Protein>~~~~~~~~~~~~~~~~~~~~~~~~~"
  echo
  taskset --cpu-list 0-70:2 ./bin/main -f ${DATA_PATH}/xpgraph/protein/ -p0 ${PMEM_PATH}/xpgraph-db/XPGraph0/ --recovery ${PMEM_PATH}/xpgraph-db/XPGraphRecovery/ --source 35 -v 8745543 -q 5 -j 2 -t $threads

  rm -rf ${PMEM_PATH}/xpgraph-db/*

  ((threads*=2))
  echo 'Done'
done