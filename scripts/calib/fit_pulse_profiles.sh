#!/bin/bash

# just do max snr one 
snr_threshold=$max_snr
if [[ -n "$1" && "$1" != "-" && $2 -gt 0 ]]; then
   snr_threshold=$1
fi

root_options="-l"
if [[ -n "$2" && "$2" != "-" ]]; then
   root_options="$2"
fi

cp ~/github/crab_frb_paper/scripts/root/plot_psr_profile.C .

mkdir -p images
for file in `ls pulse*.txt`
do
  root ${root_options} "plot_psr_profile.C(\"${file}\",2,1)"   
done

cp ~/github/crab_frb_paper/scripts/root/histotau.C .
cat *.refit | awk '{print $9;}' > tau.txt
root ${root_options} "histotau.C(\"tau.txt\",0,1,0,0.01)"
