#!/bin/bash

curr_path=`pwd`

template="20??_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged/"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

SNR_THRESHOLD=-40 # <40 -> use the max SNR one 
if [[ -n "$2" && "$2" != "-" ]]; then
   SNR_THRESHOLD=$2
fi


datfile=_DM56.72.dat
if [[ -n "$3" && "$3" != "-" ]]; then
   datfile="$3"
fi
b=${datfile%%.dat}

root_options="-l"
if [[ -n "$4" && "$4" != "-" ]]; then
   root_options="$4"
fi

replot_only=0
if [[ -n "$5" && "$5" != "-" ]]; then
   replot_only=$5
fi


for path in `ls -d ${template}`
do
   echo 
   echo "Processing $path"
   cd ${path}
   pwd

   echo "cat ../../../../../../analysis*/MEAN_SEFD.txt | tail -1"      
   MEAN_SEFD=`cat ../../../../../../analysis*/MEAN_SEFD.txt | tail -1`

   echo "~/github/crab_frb_paper/scripts/calib/fluence.sh ${MEAN_SEFD} 5 - \"${root_options}\" ${replot_only}"
   ~/github/crab_frb_paper/scripts/calib/fluence.sh ${MEAN_SEFD} 5 - "${root_options}" ${replot_only}

   cd ${curr_path}
done
