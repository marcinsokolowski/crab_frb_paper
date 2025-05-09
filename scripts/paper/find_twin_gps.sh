#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

for dataset in `ls -d ${template}`
do
   cd ${dataset}
   echo
   echo
   pwd
   if [[ -s detrended_normalised__DM56.72.txt ]]; then
      echo "File detrended_normalised__DM56.72.txt already exists"
   else
      if [[ -s detrended_normalised__DM56.72.txt.gz ]]; then
         echo "gzip -d detrended_normalised__DM56.72.txt.gz"
         gzip -d detrended_normalised__DM56.72.txt.gz
      fi
   fi
   
   if [[ -s detrended_normalised__DM56.72.txt ]]; then
      echo "/home/msok/github/presto_tools/build3/find_twin_gps _DM56.72.singlepulse detrended_normalised__DM56.72.txt > twin_gps.out 2>&1"
      /home/msok/github/presto_tools/build3/find_twin_gps _DM56.72.singlepulse detrended_normalised__DM56.72.txt > twin_gps.out 2>&1
      
      count_twins=`grep "Twin pulse found SNR" twin_gps.out | wc -l`
      echo "Found $count_twins twin GPs"
      echo
   else
      echo "ERROR : file detrended_normalised__DM56.72.txt not found in $dataset"
   fi
   cd -
done
