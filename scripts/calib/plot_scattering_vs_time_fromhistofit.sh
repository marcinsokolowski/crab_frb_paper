#!/bin/bash

rm -f crab_taufit_vs_time.txt
for fitfile in `ls /media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/20??_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/merged/pulses_snr30_calibrated/taufit.txt`
do
   uxtime=`echo $fitfile | awk '{i=index($1,"merged_channels");print substr($1,i+16,10);}'`
   
   tau=`cat $fitfile | grep -v "#" | awk '{print $5*1000.00;}' | tail -1`
   tau_err=`cat $fitfile | grep -v "#" | awk '{print $6*1000.00;}' | tail -1`
   
   echo "$uxtime 0.00 $tau $tau_err" >> crab_taufit_vs_time.txt
done


if [[ ! -s plot_scattau_vs_time.C ]]; then
   echo "cp ~/github/crab_frb_paper/scripts/root/plot_scattau_vs_time.C ."
   cp ~/github/crab_frb_paper/scripts/root/plot_scattau_vs_time.C .
fi

mkdir -p images/
root -l "plot_scattau_vs_time.C(\"crab_taufit_vs_time.txt\")"
