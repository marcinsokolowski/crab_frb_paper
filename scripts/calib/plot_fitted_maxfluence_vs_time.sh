#!/bin/bash

postfix=fitmin800.00_fitted_fluence_1col.txt
if [[ -n "$1" && "$1" != "-" ]]; then
   postfix="$1"
fi

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged/pulses_snr5_calibrated/fitted_fluence.txt"
if [[ -n "$2" && "$2" != "-" ]]; then
   template="$2"
fi

rm -f maxfittedfluence_vs_time.txt
rm -f fittedfluenceonehour_vs_time.txt

for file in `ls ${template}`
do
   echo 
   echo $file
   dir=`dirname $file`      
   uxtime=`echo $file | awk '{i=index($1,"merged_channels_");uxtime=substr($1,i+16,10);print uxtime}'`
   
   one_hour=`cat $dir/ONEPERHOUR_FLUENCE_${postfix} | head -1`
   
   max_fluence=`cat $file | awk '{snr=substr($1,14,4);gsub("_","",snr);if(snr>=10){print $2};}' | awk -v max=-1 '{if($1>=max){max=$1;}}END{print max;}'`

   echo "$uxtime $max_fluence" >> maxfittedfluence_vs_time.txt
   echo "$uxtime $one_hour" >> fittedfluenceonehour_vs_time.txt
   
   echo "$uxtime $max_fluence"
   echo "$uxtime $one_hour"   
   sleep 2
done


cat maxfittedfluence_vs_time.txt |wc

sleep 20


cp ~/github/crab_frb_paper/scripts/root/plot_maxfluence_vs_time.C .

root -l "plot_maxfluence_vs_time.C(\"maxfittedfluence_vs_time.txt\")"

root -l "plot_maxfluence_vs_time.C(\"fittedfluenceonehour_vs_time.txt\")"
