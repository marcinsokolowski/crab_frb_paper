#!/bin/bash

export PATH=~/github/crab_frb_paper/scripts/calib/:$PATH

# get log file :
curr_dir=`pwd`
dataset=`basename $curr_dir`
echo "rsync -avP aavs2:/data/${dataset}/J0534+2200_40channels_*sec_flagants_sepdada_ch256.out ."
rsync -avP aavs2:/data/${dataset}/J0534+2200_40channels_*sec_flagants_sepdada_ch256.out .

count=`ls J0534+2200_40channels_*sec_flagants_sepdada_ch256.out | wc -l`
if [[ $count -le 0 ]]; then
   echo "WARNING : no file J0534+2200_40channels_*sec_flagants_sepdada_ch256.out found trying all locations ..."
   dt=`echo ${dataset} | cut -b 1-11`
   
   echo "rsync -avP aavs2:/data/${dt}*/J0534+2200_40channels_*sec_flagants_sepdada_ch256.out ."
   rsync -avP aavs2:/data/${dt}*/J0534+2200_40channels_*sec_flagants_sepdada_ch256.out .
   
   count=`ls J0534+2200_40channels_*sec_flagants_sepdada_ch256.out | wc -l`
   if [[ $count -le 0 ]]; then
      echo "ERROR : could not find log file J0534+2200_40channels_*sec_flagants_sepdada_ch256.out anywhere -> cannot continue, manual verification is required"
   else 
      echo "OK : file J0534+2200_40channels_*sec_flagants_sepdada_ch256.out found OK -> proceeding with processing"
   fi
fi


curr_path=`pwd`
# presto_dir=`ls -d ${curr_path}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5*/ | tail -1`

# obs_script=doitnoweda2
logfile=`ls J0534+2200_40channels_*sec_flagants_sepdada_ch256.out | tail -1`
if [[ -n "$1" && "$1" != "-" ]]; then
   logfile="$1"
fi

if [[ -s ${logfile} ]]; then
   echo "INFO : log file $logfile exists -> OK"
else
   echo "ERROR : log file $logfile not found -> exiting now"   
#   exit;
fi

outdir=analysis_final
prev_analysis="../analysis"
if [[ -n "$2" && "$2" != "-" ]]; then
   outdir="$2"
fi
if [[ -d ${outdir} ]]; then
   echo "WARNING : directory ${outdir} already exists -> moving"
   echo "mv ${outdir} ${outdir}_prev"
   echo "Waiting 2 seconds (press Ctrl+C to cancel) ..."
   echo "mv ${outdir} ${outdir}_prev"
   sleep 2
   mv ${outdir} ${outdir}_prev
   
   prev_analysis="../${outdir}_prev"
   echo "prev_analysis = $prev_analysis"
fi

calc_sefd=1
if [[ -n "$3" && "$3" != "-" ]]; then
   calc_sefd=$3
fi

if [[ -n "$4" && "$4" != "-" ]]; then
   prev_analysis="$4"
fi

force_calc_sefd=0
if [[ -n "$5" && "$5" != "-" ]]; then
   force_calc_sefd=$5
fi

sefds_only=0
if [[ -n "$6" && "$6" != "-" ]]; then
   sefds_only=$6
fi

# presto_sps_thresh5_numdms100_dmstep1
presto_dir_template="presto_sps_thresh5_numdms100_dmstep1"
if [[ -n "$7" && "$7" != "-" ]]; then
   presto_dir_template="$7"
fi
presto_dir=`ls -d ${curr_path}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/${presto_dir_template}/ | tail -1`
echo "DEBUG : presto_dir = $presto_dir"
if [[ ! -d $presto_dir ]]; then
   presto_dir_template="presto_sps_thresh5_numdms???_dmstep1"
   presto_dir=`ls -d ${curr_path}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/${presto_dir_template}/ | tail -1`
   echo "DEBUG2 : presto_dir = $presto_dir" 
fi
# sleep 20

crude_dm_plots=0
if [[ -n "$8" && "$8" != "-" ]]; then
   crude_dm_plots=$8
fi


echo "###################################################"
echo "PARAMETERS:"
echo "###################################################"
echo "outdir = $outdir"
echo "prev_analysis = $prev_analysis"
echo "calc_sefd = $calc_sefd"
echo "force_calc_sefd = $force_calc_sefd"
echo "sefds_only = $sefds_only"
echo "crude_dm_plots = $crude_dm_plots"
echo "###################################################"


header_file=`ls ${curr_path}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/updated.hdr`
if [[ ! -s ${header_file} ]]; then
   echo "ERROR : make sure to copy updated.hdr file from aavs2-server (generate with : readfile updated.fil > updated.hdr)"
   exit;
fi

# TotalTimeInHours=0.998768602
echo "cat ${curr_path}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/updated.hdr"
TotalTimeInHours=`cat ${curr_path}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/updated.hdr | grep "Time per file" | awk '{print $6/3600.00}'`
TotalTimeInSeconds=`cat ${curr_path}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/updated.hdr | grep "Time per file" | awk '{print $6}'`
if [[ -n "$4" && "$4" != "-" ]]; then
   TotalTimeInHours=$4
   TotalTimeInSeconds=`echo $TotalTimeInHours | awk '{print $1*3600;}'`
#else 
#   echo "ERROR : make sure to provide parameter 4 - Total observing time, use : readfile FILTERBANK"
#   exit;
fi

echo "ANALYSIS DATA FROM $TotalTimeInHours [hours] observation"

mkdir ${outdir}
cd ${outdir}
echo "cp ${curr_path}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/updated.hdr ."
cp ${curr_path}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/updated.hdr .

ln -s ../${logfile}

if [[ -d ${prev_analysis} && $force_calc_sefd -le 0 ]]; then
   # SEFDs were already generated and can be re-used if no set force_calc_sefd=1 (5th parameter)

   echo "Creating symbolic links to SEFD files ${prev_analysis}/*.sefd"
#   for file in `ls ${prev_analysis}/*.out`; do    ln -s $file; done
#   for file in `ls ${prev_analysis}/*.txt`; do    ln -s $file; done
   ln -s ${prev_analysis}/sefd_vs_time.txt .
   for file in `ls ${prev_analysis}/*.sefd`; do    ln -s $file; done
   for txt in `ls ../analysis/uxtime*txt`; do     ln -s $txt; done
else
   # Generate lists and execute SEFD simulations :
   if [[ $calc_sefd -gt 0 ]]; then
      if [[ ! -s MEAN_SEFD.txt || $force_calc_sefd -gt 0 ]]; then
         echo "~/github/station_beam/scripts/psrflux/generete_sefd_calc_list.sh ${logfile} - 1 "
         ~/github/station_beam/scripts/psrflux/generete_sefd_calc_list.sh ${logfile} - 1 
      
         # listfile=`ls -tr *_startux*_*sec_startfreq256_40chan.txt | tail -1`
         # echo "~/github/station_beam/scripts/psrflux/calculate_sefd_for_pointings.sh $listfile - 300 0.78125"
         # ~/github/station_beam/scripts/psrflux/calculate_sefd_for_pointings.sh $listfile - 300 0.78125

         # SEFD in the centre of the band :
         grep "215.625" *.sefd | awk -v start_ux=-1 '{if(start_ux < 0){start_ux=$4;}print ($4-start_ux)" "$8;}' > sefd_vs_time.txt
         # echo "SEFD AT THE CENTRE CHANNEL = $sefd_centre_ch [Jy]"
      
         mean_sefd=`awk -v sum=0 -v cnt=0 '{sum+=$2;cnt+=1;}END{print sum/cnt;}' sefd_vs_time.txt`
         echo "$mean_sefd" > MEAN_SEFD.txt
      else
         echo "INFO : SEFDs already generated earlier"
      fi
   else
      echo "WARNING : calculation of SEFD is not required"
   fi   
fi

mean_sefd=`awk -v sum=0 -v cnt=0 '{sum+=$2;cnt+=1;}END{print sum/cnt;}' sefd_vs_time.txt`
echo "MEAN SEFD OVER TIME = $mean_sefd [Jy]"

if [[ $sefds_only -gt 0 ]]; then
   echo "WARNING : only generation of SEFDs has been requested exiting now"
   exit
fi

# fit to SEFD over time :
# cat *.sefd | awk -v start_ux=-1 '{if($1!="#"){if(start_ux < 0){start_ux=$4;}print ($4-start_ux)" "$8;}}' > sefd_vs_time.txt
cp ~/github/crab_frb_paper/scripts/root/plotsefd_vs_time.C .
root -l "plotsefd_vs_time.C(\"sefd_vs_time.txt\")"


# PRESTO single pulses analysis :
echo "ln -s ${presto_dir} presto_5sigma_pulses"
ln -s ${presto_dir} presto_5sigma_pulses
cat presto_5sigma_pulses/_DM*.singlepulse | grep -v "#" | awk '{if($1<20){print $3;}}' > rfitimes.txt
cat presto_5sigma_pulses/_DM*.singlepulse | grep -v "#" | awk '{if($1>=55 && $1<=59){print $3;}}' > gptimes.txt
cp ~/github/crab_frb_paper/scripts/root/rfifind.C .

# 
time_rounded_to_ten=`echo $TotalTimeInSeconds | awk '{print int($1/10.00)*10.00+10}'`
n_bins=`echo $time_rounded_to_ten | awk '{print $1/10;}'`

root -l "rfifind.C(\"gptimes.txt\",100000000000000.00,0,${time_rounded_to_ten},${n_bins})" # do not save RFI ranges (that's why such a high RFI threshold)

# 
root -l "rfifind.C(\"rfitimes.txt\",10,0,${time_rounded_to_ten},${n_bins})"
# 2025-04-21 : temporary disabled :
# echo "mv rfi.ranges rfi.ranges.backup"
# mv rfi.ranges rfi.ranges.backup

# Create an empty file rfi.ranges :
echo "touch rfi.ranges"
touch rfi.ranges

if [[ -s TotalGoodTimeInSec.txt ]]; then
   TotalTimeInHours=`cat TotalGoodTimeInSec.txt | awk '{print $1/3600.00}'`
   echo "INFO : TotalTimeInHours updated to $TotalTimeInHours [hours] after RFI flagging"
else
   echo "ERROR : script rfifind.C should produce local file TotalGoodTimeInSec.txt with number of seconds after RFI excision -> cannot reliably continue, please verify"
   exit;
fi

# exclude RFI 
cat presto_5sigma_pulses/_DM*.singlepulse | grep -v "#" | awk '{if($1>=55 && $1<=59){print $1" "$2" "$3" "$4" "$5;}}' > all_crab_gps.singlepulse
echo "Number of single pulses in DM range : 55 - 59 pc/cm^3 - BEFORE RFI excision:"
cat all_crab_gps.singlepulse |wc

# TODO : calculate time after excluding some parts of data !!!
echo "python ~/github/crab_frb_paper/scripts/python/exclude_ranges.py all_crab_gps.singlepulse --rfi_file=rfi.ranges --presto --outfile=all_crab_gps_norfi.singlepulse"
python ~/github/crab_frb_paper/scripts/python/exclude_ranges.py all_crab_gps.singlepulse --rfi_file=rfi.ranges --presto --outfile=all_crab_gps_norfi.singlepulse

echo "Number of single pulses in DM range : 55 - 59 pc/cm^3 - AFTER RFI excision:"
cat all_crab_gps_norfi.singlepulse|wc

# calibrate SNR -> mean of peak flux density (see notes):
echo "~/github/crab_frb_paper/scripts/calib/snr2jy.sh all_crab_gps_norfi.singlepulse $mean_sefd | awk '{print $3;}' > all_crab_gps_norfi_fluxcal.singlepulse"
~/github/crab_frb_paper/scripts/calib/snr2jy.sh all_crab_gps_norfi.singlepulse $mean_sefd | awk '{print $3;}' > all_crab_gps_norfi_fluxcal.singlepulse

# 
if [[ -s presto_5sigma_pulses/_DM57.00.dat ]]; then
   echo "File presto_5sigma_pulses/_DM57.00.dat already exists"
else
   echo "File presto_5sigma_pulses/_DM57.00.dat not found -> copying :"
   echo "rsync -avP aavs2:/data/${dataset}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5*/_DM57.00.dat presto_5sigma_pulses/"
   rsync -avP aavs2:/data/${dataset}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5*/_DM57.00.dat presto_5sigma_pulses/
   
   sleep 5 
fi   

if [[ $crude_dm_plots -gt 0 ]]; then
   # dump timeseries from DM57.dat file:
   if [[ -s presto_5sigma_pulses/_DM57.00.dat ]]; then
      ln -s presto_5sigma_pulses/_DM57.00.dat
      echo "~/github/crab_frb_paper/scripts/calib/process_presto_candidates.sh _DM57.00.dat \"-U 1\""
      ~/github/crab_frb_paper/scripts/calib/process_presto_candidates.sh _DM57.00.dat "-U 1"
   else
      echo "WARNING : file presto_5sigma_pulses/_DM57.00.dat not found - cannot dump timeseries -> please verify"
      sleep 10
   fi

   # plot distribution of calibrated mean peak flux density :
   # cp  ~/github/crab_frb_paper/scripts/root/FluDistrPowerLaw.C .
   # root -l "FluDistrPowerLaw.C(\"all_crab_gps_norfi_fluxcal.singlepulse\")"
   mkdir -p images/
   cp  ~/github/crab_frb_paper/scripts/root/FluRatePerHourPowerLaw.C .
   root -l "FluRatePerHourPowerLaw.C(\"all_crab_gps_norfi_fluxcal.singlepulse\",${TotalTimeInHours})"

   # plots SNR distribution 
   cat all_crab_gps_norfi.singlepulse | awk '{if($1!="#"){print $2;}}' > all_crab_gps_norfi.snr
   # cp  ~/github/crab_frb_paper/scripts/root/SNRDistrPowerLaw.C .
   # root -l "SNRDistrPowerLaw.C(\"all_crab_gps_norfi.snr\")"
   cp  ~/github/crab_frb_paper/scripts/root/SNRRatePerHourPowerLaw.C .
   root -l "SNRRatePerHourPowerLaw.C(\"all_crab_gps_norfi.snr\",${TotalTimeInHours})"

   # plot disitrbution of luminosity :
   cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
   root -l "SpectralLuminosity_DistrPowerLaw.C(\"all_crab_gps_norfi_fluxcal.singlepulse\",${TotalTimeInHours})"

   # Analysis on merged PRESTO candidates :
   mkdir -p merged/
   cd merged/
   echo "~/github/crab_frb_paper/scripts/calib/presto2cand.sh ../all_crab_gps_norfi.singlepulse"
   ~/github/crab_frb_paper/scripts/calib/presto2cand.sh ../all_crab_gps_norfi.singlepulse

   echo "~/github/crab_frb_paper/scripts/calib/snr2jy.sh presto.cand_normal $mean_sefd | awk '{print $3;}' > presto_norfi_fluxcal.cand_normal"
   ~/github/crab_frb_paper/scripts/calib/snr2jy.sh presto.cand_normal $mean_sefd | awk '{print $3;}' > presto_norfi_fluxcal.cand_normal
   
   mkdir -p images/
   # plot distribution of calibrated mean peak flux density :
   cp  ~/github/crab_frb_paper/scripts/root/FluRatePerHourPowerLaw.C .
   root -l "FluRatePerHourPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\",${TotalTimeInHours})"
   
   cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
   root -l "SpectralLuminosity_DistrPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\",${TotalTimeInHours})"

   # plots SNR distribution 
   cat presto.cand_normal | awk '{if($1!="#"){print $2;}}' > presto.cand_normal_snr
   cp  ~/github/crab_frb_paper/scripts/root/SNRRatePerHourPowerLaw.C .
   root -l "SNRRatePerHourPowerLaw.C(\"presto.cand_normal_snr\",${TotalTimeInHours})"


   if [[ -s ../detrended_normalised__DM57.00.txt ]]; then
      ln -s ../detrended_normalised__DM57.00.txt
   
   
      cp ~/github/crab_frb_paper/scripts/root/plot_samples_with_candidates.C .
      awk '{if($1!="#"){print $3" "$1;}}' presto.cand > presto.txt
      awk '{if($1!="#"){print $3" "$2;}}' presto.cand_normal.sorted > presto_merged_sorted.txt
   
      root -l "plot_samples_with_candidates.C+(\"detrended_normalised__DM57.00.txt\",\"presto.txt\",NULL,NULL,\"presto_merged_sorted.txt\")"   
   else
      echo "WARNING : file ../timeseries__DM57.00.txt not found -> cannot overplot time series and PRESTO candidates and merged candidates"
   fi   
else
   echo "WARNING : Plotting/analysis of crude DM=57.00 is not requested"
fi

# Fluence distributions :
cd $curr_dir
pwd

echo "~/github/crab_frb_paper/scripts/calib/process_fluence_and_scatter.sh"
~/github/crab_frb_paper/scripts/calib/process_fluence_and_scatter.sh
# path=`ls -d "J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/"`

# fit profile to peak SNR pulse:
# echo "~/github/crab_frb_paper/scripts/calib/fit_profiles_all.sh ${path} -40"
# ~/github/crab_frb_paper/scripts/calib/fit_profiles_all.sh ${path} -40

# calculate fluence for all pulses :
# echo "~/github/crab_frb_paper/scripts/calib/fluence_all.sh ${path}/merged"
# ~/github/crab_frb_paper/scripts/calib/fluence_all.sh ${path}/merged

# fits to all pulse profiles :
# echo "~/github/crab_frb_paper/scripts/calib/fit_all_profiles_all_dates.sh ${path}/merged/pulses_snr5_calibrated/"
# ~/github/crab_frb_paper/scripts/calib/fit_all_profiles_all_dates.sh ${path}/merged/pulses_snr5_calibrated/


# Fluence calculated as an integral of fitted profiles -> distributions :

# TODO :
# - plot Fluence distribution
# - plot Luminosity distribution , use distance to Crab and fluence to calculate this one 
read -p "Do you want to create final plots and vs. time plots ? [y/n]: " answer
if [[ $answer == "y" || $answer == "Y" ]]; then
   echo "INFO : you requested to create final plots, including vs. time plots, this will take a while ..."
   echo "Waiting 10 seconds, if you changed your mind press Ctrl+C to stop script now"
   sleep 10
   
   cd /media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/
   pwd
   echo "~/github/crab_frb_paper/scripts/calib/process_all_datasets.sh"
   ~/github/crab_frb_paper/scripts/calib/process_all_datasets.sh
else
   echo "WARNING : final plots not requested"
   echo "In order to plot all the results vs. time execute:"
   echo "cd /media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/"
   echo "~/github/crab_frb_paper/scripts/calib/process_all_datasets.sh"
fi

