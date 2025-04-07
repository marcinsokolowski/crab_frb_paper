#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged/pulses_snr5_calibrated"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

root_options="-l"
if [[ -n "$2" && "$2" != "-" ]]; then
   root_options="$2"
fi

do_fitting=1
if [[ -n "$3" && "$3" != "-" ]]; then
   do_fitting=$3
fi

fluence_completness_threshold=1400 # Jy ms see : 20250405_tests_of_completness_threshold.odt
if [[ -n "$4" && "$4" != "-" ]]; then
   fluence_completness_threshold=$4
fi

min_good_time=1300 # or perhaps should be 3000 seconds ? Allow 2024-12-14 to be used !
# TotalGoodTimeInSec.txt
if [[ -n "$5" && "$5" != "-" ]]; then
   min_good_time="$5"
fi



FullTotalTimeInSec=0

if [[ $do_fitting -gt 0 ]]; then
   rm -f calibrated_fluence.all fitted_fluence.all

   for dir in `ls -d ${template}`
   do
      cd ${dir}
   
      TotalTimeHours=`cat ../../../../../../../analysis_final/TotalGoodTimeInSec.txt | awk '{print $1/3600.00}'`
      TotalTimeSeconds=`cat ../../../../../../../analysis_final/TotalGoodTimeInSec.txt | awk '{printf("%d\n",$1);}'`
      UNIXTIME=`cat ../../../../../UNIXTIME.txt`

      echo "rm -f *.fit_results_fit_range_*"
      rm -f *.fit_results_fit_range_*
   
      cp ~/github/crab_frb_paper/scripts/root/FluenceRatePerHourPowerLaw.C .
      root ${root_options} "FluenceRatePerHourPowerLaw.C(\"fitted_fluence_1col.txt\",${TotalTimeHours},0,${fluence_completness_threshold},10000,${UNIXTIME})"
#      root ${root_options} "FluenceRatePerHourPowerLaw.C(\"fitted_fluence_1col.txt\",${TotalTimeHours},0,600,10000,${UNIXTIME})"

      root ${root_options} "FluenceRatePerHourPowerLaw.C(\"calibrated_fluence.txt\",${TotalTimeHours},0,${fluence_completness_threshold},10000,${UNIXTIME})"
#      root ${root_options} "FluenceRatePerHourPowerLaw.C(\"calibrated_fluence.txt\",${TotalTimeHours},0,600,10000,${UNIXTIME})"

      # use completness threshold = 1000 Jy ms (see /home/msok/Desktop/SKA/papers/2024/EDA2_FRBs/20250319_tests_of_completness_threshold.odt )
#      root ${root_options} "FluenceRatePerHourPowerLaw.C(\"fitted_fluence_1col.txt\",${TotalTimeHours},0,1000,10000,${UNIXTIME})"
#      root ${root_options} "FluenceRatePerHourPowerLaw.C(\"calibrated_fluence.txt\",${TotalTimeHours},0,1000,10000,${UNIXTIME})"
         
      cd -

      if [[ $TotalTimeSeconds -gt $min_good_time ]]; then      
         cat ${dir}/fitted_fluence_1col.txt >> fitted_fluence.all
         cat ${dir}/calibrated_fluence.txt >> calibrated_fluence.all
         
         FullTotalTimeInSec=$(($FullTotalTimeInSec+$TotalTimeSeconds))
         echo "DEBUG : FullTotalTimeInSec = $FullTotalTimeInSec [sec]"
      fi
   done
   
   echo "DEBUG : fitting of all fluence distributions completed, Total Time = $FullTotalTimeInSec [sec]"
   sleep 5
else 
   echo "WARNING : fitting is not required"
fi   


cp ~/github/crab_frb_paper/scripts/root/plot_plidx_vs_time_error.C .

# cat ${template}/fitted_fluence_1col.txt.fit_results_fit_range_600.00-10000.00 | awk '{print $7" "1800" "$2" "$4}' > fittedfluence_index_vs_time_fit_range_600-10000.txt
# cat ${template}/fitted_fluence_1col.txt.fit_results_fit_range_800.00-10000.00 | awk '{print $7" "1800" "$2" "$4}' > fittedfluence_index_vs_time_fit_range_800-10000.txt
cat ${template}/fitted_fluence_1col.txt.fit_results_fit_range_${fluence_completness_threshold}.00-10000.00 | awk '{print $7" "1800" "$2" "$4}' > fittedfluence_index_vs_time_fit_range_${fluence_completness_threshold}-10000.txt

# root -l "plot_plidx_vs_time_error.C(\"fittedfluence_index_vs_time_fit_range_1000-10000.txt\",-6,0,\"Fluence index (fit range >1000 Jy ms) [Integral of fit]\")"
# root -l "plot_plidx_vs_time_error.C(\"fittedfluence_index_vs_time_fit_range_600-10000.txt\",-6,0,\"Fluence index (fit range >600 Jy ms) [Integral of fit]\")"
root -l "plot_plidx_vs_time_error.C(\"fittedfluence_index_vs_time_fit_range_${fluence_completness_threshold}-10000.txt\",-6,0,\"Fluence index (fit range >${fluence_completness_threshold} Jy ms) [Integral of fit]\")"

# cat ${template}/calibrated_fluence.txt.fit_results_fit_range_600.00-10000.00 | awk '{print $7" "1800" "$2" "$4}' > calibratedfluence_index_vs_time_fit_range_600-10000.txt
# cat ${template}/calibrated_fluence.txt.fit_results_fit_range_800.00-10000.00 | awk '{print $7" "1800" "$2" "$4}' > calibratedfluence_index_vs_time_fit_range_800-10000.txt
cat ${template}/calibrated_fluence.txt.fit_results_fit_range_${fluence_completness_threshold}.00-10000.00 | awk '{print $7" "1800" "$2" "$4}' > calibratedfluence_index_vs_time_fit_range_${fluence_completness_threshold}-10000.txt

# root -l "plot_plidx_vs_time_error.C(\"calibratedfluence_index_vs_time_fit_range_1000-10000.txt\",-6,0,\"Fluence index (fit range >1000 Jy ms) [fluence from data]\")"
# root -l "plot_plidx_vs_time_error.C(\"calibratedfluence_index_vs_time_fit_range_600-10000.txt\",-6,0,\"Fluence index (fit range >600 Jy ms) [fluence from data]\")"
root -l "plot_plidx_vs_time_error.C(\"calibratedfluence_index_vs_time_fit_range_${fluence_completness_threshold}-10000.txt\",-6,0,\"Fluence index (fit range >${fluence_completness_threshold} Jy ms) [fluence from data]\")"


# plot and fit ditribution of fluence from all nights :
cp ~/github/crab_frb_paper/scripts/root/FluenceRatePerHourPowerLaw.C .

FullTotalTimeInHours=`echo $FullTotalTimeInSec | awk '{print $1/3600.00;}'`
# root -l "FluenceRatePerHourPowerLaw.C(\"fitted_fluence.all\",${FullTotalTimeInHours},0,600,10000,-1)"
root -l "FluenceRatePerHourPowerLaw.C(\"fitted_fluence.all\",${FullTotalTimeInHours},0,${fluence_completness_threshold},10000,-1)"

# root -l "FluenceRatePerHourPowerLaw.C(\"calibrated_fluence.all\",${FullTotalTimeInHours},0,600,10000,-1)"
root -l "FluenceRatePerHourPowerLaw.C(\"calibrated_fluence.all\",${FullTotalTimeInHours},0,${fluence_completness_threshold},10000,-1)"

# root -l "FluenceRatePerHourPowerLaw.C(\"fitted_fluence.all\",${FullTotalTimeInHours},0,1000,10000,-1)"
# root -l "FluenceRatePerHourPowerLaw.C(\"calibrated_fluence.all\",${FullTotalTimeInHours},0,1000,10000,-1)"
