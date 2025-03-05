#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/merged/pulses_snr5_calibrated/"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

curr_path=`pwd`

for dir in `ls -d ${template}`
do
   cd ${dir}
   echo "~/github/crab_frb_paper/scripts/calib/fit_all_profiles.sh"
   ~/github/crab_frb_paper/scripts/calib/fit_all_profiles.sh   
   cd $curr_path
done
