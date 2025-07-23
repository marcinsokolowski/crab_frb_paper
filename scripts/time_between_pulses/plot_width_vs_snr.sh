#!/bin/bash

cp time_between_pulses/all_time_diff_snr??.txt .
cp time_between_pulses/all_time_diff_snr???.txt .

for file in `ls all_time_diff_snr*.txt`
do
   echo "python ~/github/crab_frb_paper/scripts/python/lognormal_mydata_fit.py $file"
   sleep 1
   python ~/github/crab_frb_paper/scripts/python/lognormal_mydata_fit.py $file
done

cp ~/github/crab_frb_paper/scripts/root/plot_width_vs_snr.C .
root -l "plot_width_vs_snr.C(\"sigma_vs_snr_SORTED.txt\")"