#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/analysis_final/"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

root_options="-l -q -b"
if [[ -n "$2" && "$2" != "-" ]]; then
   root_options="$2"
fi


fluence_completness_threshold=1400 # Jy ms see 20250405_tests_of_completness_threshold.odt Figures 1 and 4 
flux_completness_threshold=500 # Jy see 20250405_tests_of_completness_threshold.odt Figure 3 
snr_completness_threshold=10

curr_path=`pwd`

for dir in `ls -d ${template}`
do
   cd $dir
   mkdir -p images/
   

   total_time_hour=`cat TotalGoodTimeInSec.txt`
   
   cd merged/
   mv flux_fit_results.txt flux_fit_results.txt.old
   mv lumin_fit_results.txt lumin_fit_results.txt.old
   mv snr_fit_results.txt snr_fit_results.txt.old

   
   cp  ~/github/crab_frb_paper/scripts/root/FluRatePerHourPowerLaw.C .
   root ${root_options} "FluRatePerHourPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\",${total_time_hour},0,${flux_completness_threshold},20000,1,1,90,20000)"

   cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
   root ${root_options} "SpectralLuminosity_DistrPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\",${total_time_hour},1.5e23,1e26,100,2e24,1e26,true,${flux_completness_threshold})"

   # plots SNR distribution 
   cp  ~/github/crab_frb_paper/scripts/root/SNRRatePerHourPowerLaw.C .
   root ${root_options} "SNRRatePerHourPowerLaw.C(\"presto.cand_normal_snr\",${total_time_hour},0,${snr_completness_threshold},500,1,1,0,300)"
   
   # sleep 1 
   
   cd $curr_path
done

pwd
echo "gthumb -n ${template}/merged/images/presto_norfi_fluxcal.cand_normal_FluRatePerHourPowerLaw.png &"
gthumb -n ${template}/merged/images/presto_norfi_fluxcal.cand_normal_FluRatePerHourPowerLaw.png &
sleep 1

echo "gthumb -n 202?_??_??_pulsars_msok/analysis_final/merged/images/presto_norfi_fluxcal.cand_normal_SpectralLuminosity_DistrPowerLaw.png &"
gthumb -n 202?_??_??_pulsars_msok/analysis_final/merged/images/presto_norfi_fluxcal.cand_normal_SpectralLuminosity_DistrPowerLaw.png &
sleep 1

echo "gthumb -n 202?_??_??_pulsars_msok/analysis_final/merged/images/presto.cand_normal_snr_SNRRatePerHourPowerLaw.png &"
gthumb -n 202?_??_??_pulsars_msok/analysis_final/merged/images/presto.cand_normal_snr_SNRRatePerHourPowerLaw.png &
sleep 1




