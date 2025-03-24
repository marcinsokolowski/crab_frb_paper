#!/bin/bash

curr_path=`pwd`

template="20??_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/"
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

prepare_data=1
if [[ -n "$5" && "$5" != "-" ]]; then
   prepare_data=$5
fi



for path in `ls -d ${template}`
do
   echo 
   echo "Processing $path"
   cd ${path}
   pwd
   if [[ -s $datfile ]]; then     
      if [[ $prepare_data -gt 0 ]] ;then
         echo "~/github/crab_frb_paper/scripts/calib/process_presto_candidates.sh $datfile - \"${root_options}\""
         ~/github/crab_frb_paper/scripts/calib/process_presto_candidates.sh $datfile - "${root_options}"
      
         cat _DM56.72.singlepulse | grep -v "#" | awk '{if($1>=55 && $1<=59){print $1" "$2" "$3" "$4" "$5;}}' > all_crab_gps.singlepulse
      
         echo "cp ../../../../../analysis_final/rfi.ranges ."
         cp ../../../../../analysis_final/rfi.ranges .
      
         echo "python ~/github/crab_frb_paper/scripts/python/exclude_ranges.py all_crab_gps.singlepulse --rfi_file=rfi.ranges --presto --outfile=all_crab_gps_norfi.singlepulse"
         python ~/github/crab_frb_paper/scripts/python/exclude_ranges.py all_crab_gps.singlepulse --rfi_file=rfi.ranges --presto --outfile=all_crab_gps_norfi.singlepulse
      
         mkdir merged
         cd merged
         echo "~/github/crab_frb_paper/scripts/calib/presto2cand.sh ../all_crab_gps_norfi.singlepulse"
         ~/github/crab_frb_paper/scripts/calib/presto2cand.sh ../all_crab_gps_norfi.singlepulse

         echo "ln -s ../detrended_normalised_${b}.txt"
         ln -s ../detrended_normalised_${b}.txt
      
         cp ~/github/crab_frb_paper/scripts/root/plot_samples_with_candidates.C .
      
         awk '{if($1!="#"){print $3" "$1;}}' presto.cand > presto.txt
         awk '{if($1!="#"){print $3" "$2;}}' presto.cand_normal.sorted > presto_merged_sorted.txt
         
         pwd
         TotalTimeInHours=1.00
         if [[ -s ../../../../../../analysis_final/TotalGoodTimeInSec.txt ]]; then
            TotalTimeInHours=`cat ../../../../../../analysis_final/TotalGoodTimeInSec.txt` 
            echo "TotalTimeInHours = $TotalTimeInHours (from file ../../../../../../analysis_final/TotalGoodTimeInSec.txt)"
         else
            echo "ERROR : file ../../../../../../analysis_final/TotalGoodTimeInSec.txt not found"            
         fi

         mkdir -p images/
         root ${root_options} "plot_samples_with_candidates.C(\"detrended_normalised_${b}.txt\",\"presto.txt\",NULL,NULL,\"presto_merged_sorted.txt\")"
         
         echo "~/github/crab_frb_paper/scripts/calib/snr2jy.sh presto.cand_normal $mean_sefd | awk '{print $3;}' > presto_norfi_fluxcal.cand_normal"
         ~/github/crab_frb_paper/scripts/calib/snr2jy.sh presto.cand_normal $mean_sefd | awk '{print $3;}' > presto_norfi_fluxcal.cand_normal

         # plot distribution of calibrated mean peak flux density :
         cp  ~/github/crab_frb_paper/scripts/root/FluRatePerHourPowerLaw.C .
         root -l "FluRatePerHourPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\",${TotalTimeInHours})"

         cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
         root -l "SpectralLuminosity_DistrPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\",${TotalTimeInHours})"

         # plots SNR distribution 
         cat presto.cand_normal | awk '{if($1!="#"){print $2;}}' > presto.cand_normal_snr
         cp  ~/github/crab_frb_paper/scripts/root/SNRRatePerHourPowerLaw.C .
         root -l "SNRRatePerHourPowerLaw.C(\"presto.cand_normal_snr\",${TotalTimeInHours})"

      else
         echo "Data preparation skipped"
         cd merged
      fi
         
      pwd
      echo "cat ../../../../../../analysis*/MEAN_SEFD.txt | tail -1"      
      MEAN_SEFD=`cat ../../../../../../analysis*/MEAN_SEFD.txt | tail -1`
      
      # use maximum SNR 
      echo "~/github/crab_frb_paper/scripts/calib/fit_profiles.sh ${MEAN_SEFD} ${SNR_THRESHOLD} - \"${root_options}\" 1" # ${SNR_THRESHOLD}
      ~/github/crab_frb_paper/scripts/calib/fit_profiles.sh ${MEAN_SEFD} ${SNR_THRESHOLD} - "${root_options}" 1 # ${SNR_THRESHOLD}
      
      cd ..
   else
      echo "WARNING/ERROR : data file $datfile does not exist in:"
      pwd
   fi
   cd ${curr_path}
done
