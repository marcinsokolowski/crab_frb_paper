#!/bin/bash

template=/media/msok/f5ce6064-9dc3-40c3-bc9c-7314e8594519/eda2/2025_02_10_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_1612.883543sec_interval4sec
subdir=`basename ${template}`


n=4
outdir=${subdir}_merged${n}

cd ${template}/

echo "mkdir -p ../${outdir}"
mkdir -p ../${outdir}

ls -al ../${outdir}/


echo "~/github/crab_frb_paper/scripts/arfiles/merge_psr_n.sh ${n} ../${outdir}/"
~/github/crab_frb_paper/scripts/arfiles/merge_psr_n.sh ${n} ../${outdir}/

echo "cp *.rf ../${outdir}/"
cp *.rf ../${outdir}/

cd ../${outdir}/

~/github/crab_frb_paper/scripts/arfiles/dm_from_gp.sh `ls *.rf|tail -1` 0 "leading_edge" "-l -b -q" - 0 0

~/github/crab_frb_paper/scripts/arfiles/dm_from_gp.sh `ls *.rf|tail -1` 0 "pulse" "-l -b -q" - 0 0



