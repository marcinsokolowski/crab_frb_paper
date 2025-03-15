#!/bin/bash

echo "cp ~/github/crab_frb_paper/scripts/root/plot_psr_snr_width.C ."
cp ~/github/crab_frb_paper/scripts/root/plot_psr_snr_width.C .

echo "mv FWHM.txt FWHM.txt.old"
mv FWHM.txt FWHM.txt.old

mkdir -p images/
for snrfile in `ls snr*.txt`
do
   maxx=`awk -v maxy=-1 -v maxx=-1 '{if($2>maxy){maxy=$2;maxx=$1;}}END{print maxx}' ${snrfile}`
   maxy=`awk -v maxy=-1 -v maxx=-1 '{if($2>maxy){maxy=$2;maxx=$1;}}END{print maxy}' ${snrfile}`
   
   start_x=`echo $maxx | awk '{print $1-20*0.001}'`
   end_x=`echo $maxx | awk '{print $1+40*0.001}'`

   root -b -l -q "plot_psr_snr_width.C(\"${snrfile}\",${start_x},${end_x})"
done

cp ~/github/crab_frb_paper/scripts/root/plot_fwhm_vs_dm.C .
