#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/merged/pulses_snr5_calibrated/MAX_calibrated_fluence.txt"


rm -f maxfluence_vs_time.txt
for file in `ls ${template}`
do
   uxtime=`echo $file | awk '{i=index($1,"merged_channels_");uxtime=substr($1,i+16,10);print uxtime}'`
   
   max_fluence=`cat $file`
   echo "$uxtime $max_fluence" >> maxfluence_vs_time.txt   
done


cp ~/github/crab_frb_paper/scripts/root/plot_maxfluence_vs_time.C .

root -l "plot_maxfluence_vs_time.C(\"maxfluence_vs_time.txt\")"
