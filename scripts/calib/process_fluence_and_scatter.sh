#!/bin/bash

# Fluence distributions :
curr_dir=`pwd`
pwd
path=`ls -d J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/`

dataset=`basename $curr_dir`
dat_count=`ls ${path}/_DM56.7?.dat |wc -l`
if [[ -d $path && $dat_count -gt 0 ]]; then
   echo "INFO : $path and data _DM56.7?.dat exist -> continuing"
else
   echo "INFO : $path does not exist -> copying data now ..."   
   cd ..
   pwd
 
   echo "~/github/crab_frb_paper/scripts/scp/scp_exect_dm_dat.sh ${dataset}"
   sleep 1
   ~/github/crab_frb_paper/scripts/scp/scp_exect_dm_dat.sh ${dataset}
   cd $curr_dir
   
   path=`ls -d J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/`
   pwd
fi   


# fit profile to peak SNR pulse:
echo "~/github/crab_frb_paper/scripts/calib/fit_profiles_all.sh ${path} -40"
~/github/crab_frb_paper/scripts/calib/fit_profiles_all.sh ${path} -40

# calculate fluence for all pulses :
echo "~/github/crab_frb_paper/scripts/calib/fluence_all.sh ${path}/merged - - - 1"
~/github/crab_frb_paper/scripts/calib/fluence_all.sh ${path}/merged - - - 1 

# fits to all pulse profiles :
echo "~/github/crab_frb_paper/scripts/calib/fit_all_profiles_all_dates.sh ${path}/merged/pulses_snr5_calibrated/"
~/github/crab_frb_paper/scripts/calib/fit_all_profiles_all_dates.sh ${path}/merged/pulses_snr5_calibrated/

echo "Separating GPs from MPs and IPs ..."
echo "~/github/crab_frb_paper/scripts/calib/fluence_mp_and_ip_all.sh ${path}/merged/pulses_snr5_calibrated/"
~/github/crab_frb_paper/scripts/calib/fluence_mp_and_ip_all.sh ${path}/merged/pulses_snr5_calibrated/
