
template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged/pulses_snr5_calibrated/mp_ip/"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

TotalTimeHours=1.00
if [[ -n "$2" && "$2" != "-" ]]; then
   TotalTimeHours=$2
fi

fluence_completness_threshold=1400 # Jy ms see : 20250405_tests_of_completness_threshold.odt
if [[ -n "$3" && "$3" != "-" ]]; then
   fluence_completness_threshold=$3
fi

echo "cat ${template}/mp_fluence.txt > mp_fluence_all.txt"
cat ${template}/mp_fluence.txt > mp_fluence_all.txt

echo "cat ${template}/ip_fluence.txt > ip_fluence_all.txt"
cat ${template}/ip_fluence.txt > ip_fluence_all.txt

cp ~/github/crab_frb_paper/scripts/root/FluenceRatePerHourPowerLaw.C .
mkdir -p images/
root ${root_options} "FluenceRatePerHourPowerLaw.C(\"mp_fluence_all.txt\",${TotalTimeHours},0,${fluence_completness_threshold},100000)"
root ${root_options} "FluenceRatePerHourPowerLaw.C(\"ip_fluence_all.txt\",${TotalTimeHours},0,${fluence_completness_threshold},100000)"

