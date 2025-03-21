#!/bin/bash

path="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/2024_12_14_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/merged/pulses_snr5_calibrated"
cd $path

root_options="-l"

mp_phase_start=0.22
mp_phase_end=0.44

ip_phase_start=0.64
ip_phase_end=0.84

fluence_completness_threshold=1000 # Jy ms see : ~/Desktop/EDA2/papers/2024/EDA2_FRBs/20250319_tests_of_completness_threshold.odt

min_snr=5

infile=calibrated_pulses.txt
# format :
# Time[sec] MAX_FLUX[Jy] FLUENCE[Jy ms] SNR SNR_FLUX[Jy] PRESTO_SNR PRESTO_FLUX[Jy]
# 31.68087552 427207168.00000000 4834.99434657 41.87648773 1383.46468329 56.78000000 1875.82883099


# TODO : add date dependence !!! Implemnet some simple Python script with interpolation included 
period=0.033836032


mkdir -p mp_ip/

# check distribution of phases
cat ${infile} | awk -v period=${period} '{if($1!="#"){time=$1;phase=( time % period)/period;print phase;}}' > mp_ip/gp_phase.txt

# select only MPs :
cat ${infile} | awk -v period=${period} -v mp_start=${mp_phase_start} -v mp_end=${mp_phase_end} '{if($1!="#"){time=$1;phase=( time % period)/period;if(phase>=mp_start && phase<=mp_end){print $3;}}}' > mp_ip/mp_fluence.txt

# select only IPs :
cat ${infile} | awk -v period=${period} -v mp_start=${ip_phase_start} -v mp_end=${ip_phase_end} '{if($1!="#"){time=$1;phase=( time % period)/period;if(phase>=mp_start && phase<=mp_end){print $3;}}}' > mp_ip/ip_fluence.txt

if [[ -s ../../../../../../../analysis_final/TotalGoodTimeInSec.txt ]]; then
   TotalTimeHours=`cat ../../../../../../../analysis_final/TotalGoodTimeInSec.txt | awk '{print $1/3600.00}'`
else
   echo "ERROR : file ../../../../../../../analysis_final/TotalGoodTimeInSec.txt not found -> cannot continue"
   exit
fi

# plot and fit distributions using completness threshold:
cd mp_ip/
mkdir -p images/
cp ~/github/crab_frb_paper/scripts/root/FluenceRatePerHourPowerLaw.C .
root ${root_options} "FluenceRatePerHourPowerLaw.C(\"mp_fluence.txt\",${TotalTimeHours},0,1000,100000)"
root ${root_options} "FluenceRatePerHourPowerLaw.C(\"ip_fluence.txt\",${TotalTimeHours},0,1000,100000)"


# TODO : per rotation (see Brad's paper) :