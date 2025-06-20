#!/bin/bash

do_tau=0
if [[ -n "$1" && "$1" != "-" ]]; then
   do_tau=$1
fi

# MAX SNR : pulse000000_snr113.8_time1215.333504sec.txt
template="pulse??????_snr??.?_time*sec.txt"
if [[ -n "$2" && "$2" != "-" ]]; then
   template="$2"
fi


root_options="-l"

echo "mv plot_psr_profile.C plot_psr_profile.C.OLD"
mv plot_psr_profile.C plot_psr_profile.C.OLD

echo "cp ~/github/crab_frb_paper/scripts/root/plot_psr_profile_tau.C ."
cp ~/github/crab_frb_paper/scripts/root/plot_psr_profile_tau.C .

mkdir -p images
for file in `ls ${template}` # only snr>=10
do
   root ${root_options} "plot_psr_profile_tau.C(\"${file}\",2,1)"   
done

if [[ $do_tau -gt 0 ]]; then
   cp ~/github/crab_frb_paper/scripts/root/histotau.C .
   cat pulse*.refit | awk '{print $9;}' > tau.txt
   root ${root_options} "histotau.C(\"tau.txt\",0,1,-0.05,0.05)"
else
   echo "INFO : plotting histogram of Tau-s skipped"
fi


