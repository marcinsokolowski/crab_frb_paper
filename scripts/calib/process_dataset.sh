#!/bin/bash


# get log file :
curr_dir=`pwd`
dataset=`basename $curr_dir`
echo "rsync -avP aavs2:/data/${dataset}/J0534+2200_40channels_*sec_flagants_sepdada_ch256.out ."
rsync -avP aavs2:/data/${dataset}/J0534+2200_40channels_*sec_flagants_sepdada_ch256.out .


curr_path=`pwd`
presto_dir=`ls -d ${curr_path}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5*/ | tail -1`

# obs_script=doitnoweda2
logfile=`ls J0534+2200_40channels_*sec_flagants_sepdada_ch256.out | tail -1`
if [[ -n "$1" && "$1" != "-" ]]; then
   logfile="$1"
fi

if [[ -s ${logfile} ]]; then
   echo "INFO : log file $logfile exists -> OK"
else
   echo "ERROR : log file $logfile not found -> exiting now"   
   exit;
fi

outdir=analysis
if [[ -n "$2" && "$2" != "-" ]]; then
   outdir="$2"
fi

calc_sefd=1
if [[ -n "$3" && "$3" != "-" ]]; then
   calc_sefd=$3
fi

mkdir ${outdir}
cd ${outdir}

ln -s ../${logfile}

# Generate lists and execute SEFD simulations :
if [[ $calc_sefd -gt 0 ]]; then
   echo "~/github/station_beam/scripts/psrflux/generete_sefd_calc_list.sh ${logfile} - 1 "
   ~/github/station_beam/scripts/psrflux/generete_sefd_calc_list.sh ${logfile} - 1 
else
   echo "WARNING : calculation of SEFD is not required"
fi   

# listfile=`ls -tr *_startux*_*sec_startfreq256_40chan.txt | tail -1`
# echo "~/github/station_beam/scripts/psrflux/calculate_sefd_for_pointings.sh $listfile - 300 0.78125"
# ~/github/station_beam/scripts/psrflux/calculate_sefd_for_pointings.sh $listfile - 300 0.78125

# SEFD in the centre of the band :
grep "215.625" *.sefd | awk -v start_ux=-1 '{if(start_ux < 0){start_ux=$4;}print ($4-start_ux)" "$8;}' > sefd_vs_time.txt
# echo "SEFD AT THE CENTRE CHANNEL = $sefd_centre_ch [Jy]"

mean_sefd=`awk -v sum=0 -v cnt=0 '{sum+=$2;cnt+=1;}END{print sum/cnt;}' sefd_vs_time.txt`
echo "MEAN SEFD OVER TIME = $mean_sefd [Jy]"

# fit to SEFD over time :
# cat *.sefd | awk -v start_ux=-1 '{if($1!="#"){if(start_ux < 0){start_ux=$4;}print ($4-start_ux)" "$8;}}' > sefd_vs_time.txt
cp ~/github/crab_frb_paper/scripts/root/plotsefd_vs_time.C .
root -l "plotsefd_vs_time.C(\"sefd_vs_time.txt\")"


# PRESTO single pulses analysis :
ln -s ${presto_dir} presto_5sigma_pulses
cat presto_5sigma_pulses/_DM*.singlepulse | grep -v "#" | awk '{if($1<20){print $3;}}' > rfitimes.txt
cat presto_5sigma_pulses/_DM*.singlepulse | grep -v "#" | awk '{if($1>=55 && $1<=59){print $3;}}' > gptimes.txt
cp ~/github/crab_frb_paper/scripts/root/rfifind.C .
root -l "rfifind.C(\"gptimes.txt\",100000000000000.00)" # do not save RFI ranges (that's why such a high RFI threshold)
root -l "rfifind.C(\"rfitimes.txt\")"

# exclude RFI 
cat presto_5sigma_pulses/_DM*.singlepulse | grep -v "#" | awk '{if($1>=55 && $1<=59){print $1" "$2" "$3" "$4" "$5;}}' > all_crab_gps.singlepulse
echo "Number of single pulses in DM range : 55 - 59 pc/cm^3 - BEFORE RFI excision:"
cat all_crab_gps.singlepulse |wc

echo "python ~/github/crab_frb_paper/scripts/python/exclude_ranges.py all_crab_gps.singlepulse --rfi_file=rfi.ranges --presto --outfile=all_crab_gps_norfi.singlepulse"
python ~/github/crab_frb_paper/scripts/python/exclude_ranges.py all_crab_gps.singlepulse --rfi_file=rfi.ranges --presto --outfile=all_crab_gps_norfi.singlepulse

echo "Number of single pulses in DM range : 55 - 59 pc/cm^3 - AFTER RFI excision:"
cat all_crab_gps_norfi.singlepulse|wc

# calibrate SNR -> mean of peak flux density (see notes):
echo "~/github/crab_frb_paper/scripts/calib/snr2jy.sh all_crab_gps_norfi.singlepulse $mean_sefd | awk '{print $3;}' > all_crab_gps_norfi_fluxcal.singlepulse"
~/github/crab_frb_paper/scripts/calib/snr2jy.sh all_crab_gps_norfi.singlepulse $mean_sefd | awk '{print $3;}' > all_crab_gps_norfi_fluxcal.singlepulse



# plot distribution of calibrated mean peak flux density :
cp  ~/github/crab_frb_paper/scripts/root/FluDistrPowerLaw.C .
root -l "FluDistrPowerLaw.C(\"all_crab_gps_norfi_fluxcal.singlepulse\")"

# plots SNR distribution 
cat all_crab_gps_norfi.singlepulse | awk '{if($1!="#"){print $2;}}' > all_crab_gps_norfi.snr
cp  ~/github/crab_frb_paper/scripts/root/SNRDistrPowerLaw.C .
root -l "SNRDistrPowerLaw.C(\"all_crab_gps_norfi.snr\")"

# plot disitrbution of luminosity :
cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
root -l "SpectralLuminosity_DistrPowerLaw.C(\"all_crab_gps_norfi_fluxcal.singlepulse\")"

# Analysis on merged PRESTO candidates :
mkdir -p merged/
cd merged/
echo "~/github/crab_frb_paper/scripts/calib/presto2cand.sh ../all_crab_gps_norfi.singlepulse"
~/github/crab_frb_paper/scripts/calib/presto2cand.sh ../all_crab_gps_norfi.singlepulse

echo "~/github/crab_frb_paper/scripts/calib/snr2jy.sh presto.cand_normal $mean_sefd | awk '{print $3;}' > presto_norfi_fluxcal.cand_normal"
~/github/crab_frb_paper/scripts/calib/snr2jy.sh presto.cand_normal $mean_sefd | awk '{print $3;}' > presto_norfi_fluxcal.cand_normal

# plot distribution of calibrated mean peak flux density :
cp  ~/github/crab_frb_paper/scripts/root/FluDistrPowerLaw.C .
root -l "FluDistrPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\")"

cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
root -l "SpectralLuminosity_DistrPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\")"

# plots SNR distribution 
cat presto.cand_normal | awk '{if($1!="#"){print $2;}}' > presto.cand_normal_snr
cp  ~/github/crab_frb_paper/scripts/root/SNRDistrPowerLaw.C .
root -l "SNRDistrPowerLaw.C(\"presto.cand_normal_snr\")"





# TODO :
# - plot Fluence distribution
# - plot Luminosity distribution , use distance to Crab and fluence to calculate this one 



