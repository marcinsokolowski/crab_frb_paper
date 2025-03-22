#!/bin/bash

path="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/2024_12_14_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/merged/pulses_snr5_calibrated"
if [[ -n "$1" && "$1" != "-" ]]; then
   path="$1"
fi

cd $path
ux=`echo $path | awk '{idx=index($0,"merged_channels_");print substr($0,idx+16,10);}'`

root_options="-l"

mp_phase_start=0.2
if [[ -n "$2" && "$2" != "-" ]]; then
  mp_phase_start=$2
fi

mp_phase_end=0.44
if [[ -n "$3" && "$3" != "-" ]]; then
   mp_phase_end=$3
fi

ip_phase_start=0.6
if [[ -n "$4" && "$4" != "-" ]]; then
   ip_phase_start=$4
fi

ip_phase_end=0.84
if [[ -n "$5" && "$5" != "-" ]]; then
   ip_phase_end=$5
fi


fluence_completness_threshold=1000 # Jy ms see : ~/Desktop/EDA2/papers/2024/EDA2_FRBs/20250319_tests_of_completness_threshold.odt

min_snr=5

infile=calibrated_pulses.txt
# format :
# Time[sec] MAX_FLUX[Jy] FLUENCE[Jy ms] SNR SNR_FLUX[Jy] PRESTO_SNR PRESTO_FLUX[Jy]
# 31.68087552 427207168.00000000 4834.99434657 41.87648773 1383.46468329 56.78000000 1875.82883099


# TODO : add date dependence !!! Implemnet some simple Python script with interpolation included 
# period=0.033836032
period=`python ~/github/crab_frb_paper/scripts/crab/crab_fu.py $ux 1`
# period=`python ~/github/crab_frb_paper/scripts/crab/crab_fu_tabular.py $ux 1`
echo "Period of Crab for ux = $ux is $period [seconds] vs. default value 0.033836032 from 15 DEC 2024 ( https://www.jb.man.ac.uk/pulsar/crab/crab2.txt )"
sleep 5


mkdir -p mp_ip/

# check distribution of phases
cat ${infile} | awk -v period=${period} '{if($1!="#"){time=$1;phase=( time % period)/period;print phase;}}' > mp_ip/gp_phase.txt
cd mp_ip/
mkdir -p images/
root ${root_options} "histofile_raw.C(\"gp_phase.txt\",0,0,0,1,50)"
cd ..

read -p "Do you want to overwrite phase windows for Mainpulse (MP) and Interpulse (IP) : [y/n] " answer
if [[ $answer == "y" || $answer == "Y" ]]; then
   read -p "MP start phase [default $mp_phase_start]: " mp_phase_start
   read -p "MP end phase [default $mp_phase_end]: " mp_phase_end
   read -p "IP start phase [default $ip_phase_start]: " ip_phase_start
   read -p "IP end phase [default $ip_phase_end]: " ip_phase_end
else
   echo "Using default pulse windows: MP $mp_phase_start - $mp_phase_end , IP : $ip_phase_start - $ip_phase_end"   
fi

# select only MPs :
cat ${infile} | awk -v period=${period} -v mp_start=${mp_phase_start} -v mp_end=${mp_phase_end} '{if($1!="#"){time=$1;phase=( time % period)/period;if( (mp_end>mp_start && phase>=mp_start && phase<=mp_end) || (mp_end < mp_start && (phase>=mp_start || phase<=mp_end)) ){print $3;}}}' > mp_ip/mp_fluence.txt

# select only IPs :
cat ${infile} | awk -v period=${period} -v mp_start=${ip_phase_start} -v mp_end=${ip_phase_end} '{if($1!="#"){time=$1;phase=( time % period)/period;if( (mp_end>mp_start && phase>=mp_start && phase<=mp_end) || (mp_end < mp_start && (phase>=mp_start || phase<=mp_end)) ){print $3;}}}' > mp_ip/ip_fluence.txt

if [[ -s ../../../../../../../analysis_final/TotalGoodTimeInSec.txt ]]; then
   TotalTimeHours=`cat ../../../../../../../analysis_final/TotalGoodTimeInSec.txt | awk '{print $1/3600.00}'`
else
   echo "ERROR : file ../../../../../../../analysis_final/TotalGoodTimeInSec.txt not found -> cannot continue"
   exit
fi

# plot and fit distributions using completness threshold:
cd mp_ip/

cp ~/github/crab_frb_paper/scripts/root/FluenceRatePerHourPowerLaw.C .
root ${root_options} "FluenceRatePerHourPowerLaw.C(\"mp_fluence.txt\",${TotalTimeHours},0,1000,100000)"
root ${root_options} "FluenceRatePerHourPowerLaw.C(\"ip_fluence.txt\",${TotalTimeHours},0,1000,100000)"


# TODO : per rotation (see Brad's paper) :