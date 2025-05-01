#!/bin/bash

root_options="-l"


mkdir -p images
for file in `ls pulse*.txt`
do
   root ${root_options} "plot_psr_profile.C(\"${file}\",2,1)"   
done

cp ~/github/crab_frb_paper/scripts/root/histotau.C .
cat *.refit | awk '{print $9;}' > tau.txt
root ${root_options} "histotau.C(\"tau.txt\",0,1,0,0.01)"


