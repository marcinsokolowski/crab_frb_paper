#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/merged/pulses_snr??.??_calibrated/"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

rm -f crab_tau_vs_time.txt
for fitfile in `ls ${template}/pulse*.txt.refit`
do
   uxtime=`echo $fitfile | awk '{i=index($1,"merged_channels");print substr($1,i+16,10);}'`
   
   tau=`cat $fitfile | awk '{print $9*1000.00;}'`
   tau_err=`cat $fitfile | awk '{print $10*1000.00;}'`
   
   echo "$uxtime 0.00 $tau $tau_err" >> crab_tau_vs_time.txt
done


if [[ ! -s plot_scattau_vs_time.C ]]; then
   echo "cp ~/github/crab_frb_paper/scripts/root/plot_scattau_vs_time.C ."
   cp ~/github/crab_frb_paper/scripts/root/plot_scattau_vs_time.C .
fi

mkdir -p images/
root -l "plot_scattau_vs_time.C(\"crab_tau_vs_time.txt\")"
