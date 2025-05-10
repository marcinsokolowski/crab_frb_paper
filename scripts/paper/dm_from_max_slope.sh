#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/candidates_fits/ScanDM_NEW"

curr_path=`pwd`
for dir in `ls -d ${template}`
do
   cd $dir
   
   cp ~/github/crab_frb_paper/scripts/root/plotslope.C .
   cp ~/github/crab_frb_paper/scripts/root/plotrisetime.C .
   cp ~/github/crab_frb_paper/scripts/root/plot_fwhm_vs_dm.C .
   
   cat *.fit | awk '{print NR" "$7/($5-$3);}'  > slope_vs_index.txt
   cat *.fit | awk '{print NR" "$5-$3;}'  > risetime_vs_index.txt
   
   mkdir -p images/
   root -l -b -q "plotslope.C(\"slope_vs_index.txt\",\"poly2\",56.700,56.76)"
   root -l -b -q "plotrisetime.C(\"risetime_vs_index.txt\",\"poly2\",56.700,56.76)"
   
   ux = 0
   if [[ -s DM_FWHM.txt ]]; then
      ux=`cat DM_FWHM.txt | awk '{print $1;}'`
   fi
   root -l -b -q "plot_fwhm_vs_dm.C(\"FWHM.txt\",$ux)"   
   
   
#   sleep 1
   
   cd $curr_path   
done
