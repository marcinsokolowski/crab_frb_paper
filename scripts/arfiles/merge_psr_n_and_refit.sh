#!/bin/bash

template=/media/msok/f5ce6064-9dc3-40c3-bc9c-7314e8594519/eda2/2025_02_10_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_1612.883543sec_interval4sec
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

root_options="-l -b -q"
if [[ -n "$2" && "$2" != "-" ]]; then
   root_options="$2"
fi

n=4
if [[ -n "$3" && "$3" != "-" ]]; then
   n="$3"
fi

do_replot=0
if [[ -n "$4" && "$4" != "-" ]]; then
   do_replot=$4
fi


subdir=`basename ${template}`


outdir=${subdir}_merged${n}

cd ${template}/

echo "mkdir -p ../${outdir}"
mkdir -p ../${outdir}

ls -al ../${outdir}/


echo "~/github/crab_frb_paper/scripts/arfiles/merge_psr_n.sh ${n} ../${outdir}/"
~/github/crab_frb_paper/scripts/arfiles/merge_psr_n.sh ${n} ../${outdir}/

echo "cp *.rf ../${outdir}/"
cp *.rf ../${outdir}/

echo "cp UNIXTIME.txt ../${outdir}/"
cp UNIXTIME.txt ../${outdir}/

cd ../${outdir}/

# ~/github/crab_frb_paper/scripts/arfiles/dm_from_gp.sh `ls *.rf|tail -1` 0 "leading_edge" "-l -b -q" - 0 0

if [[ $do_replot -gt 0 ]]; then
   ~/github/crab_frb_paper/scripts/arfiles/dm_from_gp_replot.sh `ls *.rf|tail -1` 0 "pulse" "${root_options}" - - - - NEW_FIT
else 
   ~/github/crab_frb_paper/scripts/arfiles/dm_from_gp.sh `ls *.rf|tail -1` 0 "pulse" "${root_options}" - 0 0
fi   


cd ..
echo "DM_SLOPE_pulse*txt"
cat ${subdir}*/DM_SLOPE_p*txt 
echo "------------------------------"
echo "DM_RISE*txt"
cat ${subdir}*/DM_RISE*txt 
echo "------------------------------"
echo "DM_SLOPE_leading*txt"
cat ${subdir}*/DM_SLOPE_leading*txt 





