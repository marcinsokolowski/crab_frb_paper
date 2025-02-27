#!/bin/bash

template="202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

curr_dir=`pwd`

for datadir in `ls -d ${template}`
do
   if [[ $datadir == "2024_12_24_pulsars_msok/J0534+2200_flagants_ch40_ch256/256" ||$datadir == "2024_12_14_pulsars_msok/J0534+2200_flagants_ch40_ch256/256" ]]; then
      echo "$datadir : already processed -> skipped"
      continue
   fi

   cd $datadir
   count_dada=`ls *.dada | wc -l`
   
   if [[ $count_dada -gt 0 ]]; then # skip this one as it was already processed
      if [[ -s MAX_SNR_LINE.txt ]]; then   
         pulse_time=`cat MAX_SNR_LINE.txt | awk '{print $3;}'`
         
         echo "~/github/crab_frb_paper/scripts/arfiles/create_single_pulse_archive.sh $pulse_time 1.5"
         ~/github/crab_frb_paper/scripts/arfiles/create_single_pulse_archive.sh $pulse_time 1.5         
      else
         echo "WARNING : no file MAX_SNR_LINE.txt in $datadir or in :"
         pwd
         echo "WARNING : $datadir skipped"
      fi
   else
      echo "WARNING : no .dada files in $datadir"
   fi
   
   # find .ar file with the actual pulse :
   echo "~/github/crab_frb_paper/scripts/arfiles/find_pulse_amongst_arfiles.sh 0.001"
   ~/github/crab_frb_paper/scripts/arfiles/find_pulse_amongst_arfiles.sh 0.001
   
   cd $curr_dir
done

