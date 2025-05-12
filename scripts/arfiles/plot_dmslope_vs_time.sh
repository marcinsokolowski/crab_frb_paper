#!/bin/bash

template="202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec/DM_SLOPE_pulse.txt"

mkdir -p crab_paper/
rm -f crab_paper/DM_SLOPE_pulse_vs_ux.txt crab_paper/DM_RISE_TIME_vs_ux.txt crab_paper/DM_LEADING_EDGE_vs_ux.txt

for datafile in `ls -d ${template}`
do
   dir=`dirname $datafile`
   
   ux=`cat ${dir}/UNIXTIME.txt`
   
   awk -v ux=$ux '{if(ux>0){print ux" 0 "$1" "$2;}}' ${datafile} >> crab_paper/DM_SLOPE_pulse_vs_ux.txt
   awk -v ux=$ux '{if(ux>0){print ux" 0 "$1" "$2;}}' ${dir}/DM_RISE_TIME.txt >> crab_paper/DM_RISE_TIME_vs_ux.txt
   awk -v ux=$ux '{if(ux>0){print ux" 0 "$1" "$2;}}' ${dir}/DM_SLOPE_leading_edge.txt >> crab_paper/DM_LEADING_EDGE_vs_ux.txt
done


cp ~/github/crab_frb_paper/scripts/root/plot_dm_vs_time.C .

cd crab_paper/
root -l "plot_dm_vs_time.C(\"DM_SLOPE_pulse_vs_ux.txt\")"
root -l "plot_dm_vs_time.C(\"DM_RISE_TIME_vs_ux.txt\")"
root -l "plot_dm_vs_time.C(\"DM_LEADING_EDGE_vs_ux.txt\")"


