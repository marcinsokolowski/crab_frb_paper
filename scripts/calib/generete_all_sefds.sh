#!/bin/bash

export PATH=~/github/crab_frb_paper/scripts/calib:$PATH

datalist=2025_crab.list
if [[ -n "$1" && "$1" != "-" ]]; then
   datalist="$1"
fi

for dataset in `cat $datalist`
do
   cd ${dataset}/
   echo 
   echo "Processing dataset $dataset ..."
   echo "process_dataset.sh - - 1 - 0 1 > analysis.out 2>&1"
   process_dataset.sh - - 1 - 0 1 > analysis.out 2>&1
   
   cd ..
done
