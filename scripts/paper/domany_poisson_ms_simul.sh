#!/bin/bash

lambda=0.00004
if [[ -n "$1" && "$1" != "-" ]]; then
   lambda=$1
fi

n=10

rm -f time_diff_Lambda_${lambda}000_MANY.txt

i=0
while [[ $i -lt $n ]];
do
   echo
   echo "i = $i"
   echo "python ~/github/crab_frb_paper/scripts/paper/mc_gp_poisson.py $lambda 4"
   python ~/github/crab_frb_paper/scripts/paper/mc_gp_poisson.py $lambda 4

   cat time_diff_Lambda_0.00005000.txt >> time_diff_Lambda_${lambda}000_MANY.txt   

   i=$(($i+1))
done

ls  all_time_diff_snr10.txt time_diff_Lambda_${lambda}000_MANY.txt > many.list
root -l "histofiles.C(\"many.list\",0,0,10,1000,99)"
