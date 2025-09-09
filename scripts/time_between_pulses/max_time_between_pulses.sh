#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/analysis_final/merged"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

outdir="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/crab_full_analysis/time_between_pulses"
rm -f ${outdir}/all_good_time_diff.txt ${outdir}/all_time_diff.txt
rm -f ${outdir}/all_good_time_between_pulses_fit_results.txt ${outdir}/all_time_between_pulses_fit_results.txt

root_options="-l -b -q" # -b -q 

curr_path=`pwd`
for datadir in `ls -d ${template}`
do
   dir_up=`dirname $datadir`

   echo
   cd $datadir
   good_time=-1
   if [[ -s ${dir_up}/TotalGoodTimeInSec.txt ]]; then
      good_time=`cat ${dir_up}/TotalGoodTimeInSec.txt | awk '{printf("%d\n",$1);}'`
   else
      echo "ERROR : unknown good time -> dataset skipped"
      exit;
   fi
   
   uxtime=-1
   if [[ -s ${dir_up}/../J0534+2200_flagants_ch40_ch256/256/UNIXTIME.txt ]]; then
      uxtime=`cat ${dir_up}/../J0534+2200_flagants_ch40_ch256/256/UNIXTIME.txt`
   else
      echo "ERROR : could not find file ${dir_up}/../J0534+2200_flagants_ch40_ch256/256/UNIXTIME.txt"
      exit
   fi
   ut=`date2date -ux2ut=${uxtime}`
   
   pwd
   if [[ $good_time -ge 2900 ]]; then
      max_time=`awk '{if($1!="#"){print $5;}}' presto.cand_merged |sort -n | awk -v prev_time=0 '{if(NR>1){print $1-prev_time;}prev_time=$1;}' | sort -n -r | head -1`
      echo "GOOD TIME = $good_time [sec] -> $datadir -> max_time_interval = $max_time"
   else
      echo "GOOD TIME = $good_time [sec] -> too short -> skipped ( max_time_interval = $max_time )"
   fi

   cd $curr_path
done

