#!/bin/bash

echo "cp ~/github/crab_frb_paper/scripts/root/plot_psr_snr_width.C ."
cp ~/github/crab_frb_paper/scripts/root/plot_psr_snr_width.C .

mkdir -p images/
for snrfile in `ls snr*.txt`
do
   root -b -l -q "plot_psr_snr_width.C(\"${snrfile}\",1.00,0,false,\"pulse\")"
done

