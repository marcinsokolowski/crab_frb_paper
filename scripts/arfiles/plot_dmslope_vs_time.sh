#!/bin/bash

template="202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec/DM_SLOPE_pulse.txt"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

use_all=0
if [[ -n "$2" && "$2" != "-" ]]; then
   use_all=$2
fi

outdir=crab_paper/
if [[ -n "$3" && "$3" != "-" ]]; then
   outdir="$3"
fi



mkdir -p ${outdir}/
rm -f ${outdir}/DM_SLOPE_pulse_vs_ux.txt ${outdir}/DM_RISE_TIME_vs_ux.txt ${outdir}/DM_LEADING_EDGE_vs_ux.txt

for datafile in `ls -d ${template}`
do
   dir=`dirname $datafile`
   
   echo
   echo
   echo $datafile
   if [[ -s ${dir}/good.txt || $use_all -gt 0 ]]; then   
      if [[ ! -s ${dir}/bad.txt || $use_all -gt 10 ]]; then 
         ux=`cat ${dir}/UNIXTIME.txt`
   
         awk -v ux=$ux '{if(ux>0){print ux" 0 "$1" "$2;}}' ${datafile} >> ${outdir}/DM_SLOPE_pulse_vs_ux.txt
         awk -v ux=$ux '{if(ux>0){print ux" 0 "$1" "$2;}}' ${dir}/DM_RISE_TIME.txt >> ${outdir}/DM_RISE_TIME_vs_ux.txt
         awk -v ux=$ux '{if(ux>0){print ux" 0 "$1" "$2;}}' ${dir}/DM_SLOPE_leading_edge.txt >> ${outdir}/DM_LEADING_EDGE_vs_ux.txt
      else
         echo "Skipped - flagged as bad data (set use_all > 10 to include even bad data)"
      fi
   else
      echo "Skipped - no good.txt file (not marked as good/ready dataset to use)"
   fi
done


cd ${outdir}/
cp ~/github/crab_frb_paper/scripts/root/plot_dm_vs_time.C .

root -l "plot_dm_vs_time.C(\"DM_SLOPE_pulse_vs_ux.txt\")"
root -l "plot_dm_vs_time.C(\"DM_RISE_TIME_vs_ux.txt\")"
root -l "plot_dm_vs_time.C(\"DM_LEADING_EDGE_vs_ux.txt\")"


