#!/bin/bash

analysis_dir=analysis_final
if [[ -n "$1" && "$1" != "-" ]]; then
   analysis_dir="$1"
fi

outdir=crab_full_${analysis_dir}
if [[ -n "$2" && "$2" != "-" ]]; then
   outdir="$2"
fi

min_good_time=1300 # or perhaps should be 3000 seconds ? Allow 2024-12-14 to be used !
# TotalGoodTimeInSec.txt
if [[ -n "$3" && "$3" != "-" ]]; then
   min_good_time="$3"
fi

base_path=/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/
if [[ -n "$4" && "$4" != "-" ]]; then
   base_path="$4"
fi

fluence_completness_threshold=1400 # Jy ms see 20250405_tests_of_completness_threshold.odt Figures 1 and 4 
flux_completness_threshold=500 # Jy see 20250405_tests_of_completness_threshold.odt Figure 3 
snr_completness_threshold=10

echo "###################################"
echo "PARAMETERS:"
echo "###################################"
echo "analysis_dir = $analysis_dir"
echo "outdir    = $outdir"
echo "min_good_time = $min_good_time"
echo "base_path = $base_path"
echo "Completness thresholds for fitting power laws to distributions :"
echo "    Fluence >= $fluence_completness_threshold [Jy ms]"
echo "    Flux    >= flux_completness_threshold [Jy]"
echo "    SNR     >= $snr_completness_threshold"
echo "###################################"



# cd /media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/

echo "List of observations:"
ls -ald 202?_??_??_pulsars_msok/${analysis_dir}/updated.hdr
# sleep 5

# calculate total time :
total_time_sec=`grep "Time per file" 202?_??_??_pulsars_msok/${analysis_dir}/updated.hdr | awk -v sum=0.00 '{sum+=$7;}END{print sum;}'`
total_time_hour=`grep "Time per file" 202?_??_??_pulsars_msok/${analysis_dir}/updated.hdr | awk -v sum=0.00 '{sum+=$7;}END{print sum/3600.00;}'`

echo "Total observing time = $total_time_hour [hours]"
echo "PLEASE DOUBLE CHECK THIS"
sleep 5

mkdir -p ${outdir}/merged/

for dataset in `ls -d 202?_??_??_pulsars_msok/${analysis_dir}`
do
   cd ${dataset}
   echo
   merged_dir=`ls -d ../J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged | tail -1`
   pwd
   echo "ln -sf ${merged_dir}" 
   ln -sf ${merged_dir}
   cd -   
done



cat 202?_??_??_pulsars_msok/${analysis_dir}/all_crab_gps_norfi_fluxcal.singlepulse > ${outdir}/all_crab_gps_norfi_fluxcal.singlepulse
# cat 202?_??_??_pulsars_msok/${analysis_dir}/all_crab_gps_norfi.snr > ${outdir}/all_crab_gps_norfi.snr 
# cat 202?_??_??_pulsars_msok/${analysis_dir}/presto.cand_normal_snr ${outdir}/presto.cand_normal_snr
# cat 202?_??_??_pulsars_msok/${analysis_dir}/all_crab_gps_norfi_fluxcal.singlepulse > ${outdir}/all_crab_gps_norfi_fluxcal.singlepulse

# merged :
cat 202?_??_??_pulsars_msok/${analysis_dir}/merged/presto_norfi_fluxcal.cand_normal > ${outdir}/merged/presto_norfi_fluxcal.cand_normal
cat 202?_??_??_pulsars_msok/${analysis_dir}/merged/presto_norfi_fluxcal.cand_normal > ${outdir}/merged/presto_norfi_fluxcal.cand_normal
cat 202?_??_??_pulsars_msok/${analysis_dir}/merged/presto.cand_normal_snr > ${outdir}/merged/presto.cand_normal_snr

mkdir -p ${outdir}/merged/
echo "# UXTIME  FLUX_NORM FLUX_POWER_LAW_IDX NORM_ERR IDX_ERR - - OBSTIME" > ${outdir}/merged/flux_vs_time.txt
echo "# UXTIME  LUMIN_NORM LUMIN_POWER_LAW_IDX NORM_ERR IDX_ERR - - OBSTIME" > ${outdir}/merged/lumin_vs_time.txt
echo "# UXTIME  SNR_NORM SNR_POWER_LAW_IDX NORM_ERR IDX_ERR - - OBSTIME" > ${outdir}/merged/snr_vs_time.txt

for dir in `ls -d 202?_??_??_pulsars_msok/${analysis_dir}/`
do
#   dir=`dirname $snrfile`
   dataset=`dirname $dir`
   total_good_time=`cat ${dir}/TotalGoodTimeInSec.txt | awk '{print int($1);}'`

   if [[ $total_good_time -gt $min_good_time ]]; then   
      echo 
      echo "Processing dataset = $dataset, dir = $dir"
      obsduration_hours=`cat ${dir}/updated.hdr | grep "Time per file" | awk '{print $6/3600.00;}'`
 
      cd $dir
      uxfile_count=`ls uxtime*txt 2>&1 | wc -l`
      if [[ $uxfile_count -le 0 ]]; then
         for uxfile in `ls ../analysis_final_prev/uxtime*txt`
         do
            echo "ln -s ${uxfile}"
            ln -s ${uxfile}
         done
      fi
      
      ux=`ls uxtime*txt | awk -v sum=0 -v count=0 '{ux=substr($1,7,10);sum+=ux;count+=1;}END{print sum/count;}'`
      cd -

      count=`cat ${dir}/merged/presto.cand_normal_snr | awk '{if($1!="#"){print $0;}}' | wc -l`
      flux_fit_line=`cat $dir/merged/flux_fit_results.txt | tail -1`
      lumin_fit_line=`cat $dir/merged/lumin_fit_results.txt | tail -1`
      snr_fit_line=`cat $dir/merged/snr_fit_results.txt | tail -1`
      
      echo "DEBUG: count = $count"
      echo "flux_fit_line = $flux_fit_line"
      echo "lumin_fit_line = $lumin_fit_line"
      echo "snr_fit_line = $snr_fit_line"
      
      
      # original (before merging):
      echo "$ux $count" | awk -v obsduration=${obsduration_hours} '{print $1" "$2/obsduration;}' >> ${outdir}/merged/ngps_vs_uxtime.txt
      echo "$ux $flux_fit_line" | awk -v obsduration=${obsduration_hours} '{print $0" "obsduration;}' >> ${outdir}/merged/flux_vs_time.txt
      echo "$ux $snr_fit_line" | awk -v obsduration=${obsduration_hours} '{print $0" "obsduration;}' >> ${outdir}/merged/snr_vs_time.txt
      echo "$ux $lumin_fit_line" | awk -v obsduration=${obsduration_hours} '{print $0" "obsduration;}' >> ${outdir}/merged/lumin_vs_time.txt
   
#      echo "DEBUG : $ux $snr_fit_line" 
   
      # after merging :
#      flux_fit_line=`cat $dir/merged/flux_fit_results.txt | tail -1`
#      lumin_fit_line=`cat $dir/merged/lumin_fit_results.txt | tail -1`
#      snr_fit_line=`cat $dir/merged/snr_fit_results.txt | tail -1`
#      echo "$ux $count_merged" | awk -v obsduration=${obsduration_hours} '{print $1" "$2/obsduration;}' >> ${outdir}/merged/merged-ngps_vs_uxtime.txt
#      echo "$ux $flux_fit_line" | awk -v obsduration=${obsduration_hours} '{print $0" "obsduration;}' >> ${outdir}/merged/flux_vs_time.txt
#      echo "$ux $snr_fit_line" | awk -v obsduration=${obsduration_hours} '{print $0" "obsduration;}' >> ${outdir}/merged/snr_vs_time.txt
#      echo "$ux $lumin_fit_line" | awk -v obsduration=${obsduration_hours} '{print $0" "obsduration;}' >> ${outdir}/merged/lumin_vs_time.txt
   else
      echo "WARNING : dataset $dir skipped. Total good time = $total_good_time < minimum required good time $min_good_time"
   fi
done


# PLOTTING :
# NON-MERGED :
cd ${outdir}
mkdir -p images/

# PLOTTING MERGED :
cd merged/
mkdir -p images/
# plot distribution of calibrated mean peak flux density :
cp  ~/github/crab_frb_paper/scripts/root/FluRatePerHourPowerLaw.C .
root -l "FluRatePerHourPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\",${total_time_hour},0,${flux_completness_threshold},100000)"

cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
root -l "SpectralLuminosity_DistrPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\",${total_time_hour},1.5e23,2e25,100,2e24,2e25,true,${flux_completness_threshold})"

# plots SNR distribution 
cp  ~/github/crab_frb_paper/scripts/root/SNRRatePerHourPowerLaw.C .
root -l "SNRRatePerHourPowerLaw.C(\"presto.cand_normal_snr\",${total_time_hour},0,${snr_completness_threshold},500)"

# plot N GPs vs. time :
cp ~/github/crab_frb_paper/scripts/root/plot_ngps_vs_time_error.C .
root -l "plot_ngps_vs_time_error.C(\"ngps_vs_uxtime.txt\")"

# TODO : plot Lumin, SNR and Flux norm and index vs. time :
# plots of fitted values vs. time :
pwd
cp ~/github/crab_frb_paper/scripts/root/plot_norm_vs_time_error.C .
root -l "plot_norm_vs_time_error.C(\"snr_vs_time.txt\",0,0.05,\"SNR : fitted power law norm vs. time (Rate at SNR=10)\")"
pwd
root -l "plot_norm_vs_time_error.C(\"flux_vs_time.txt\",0,0.0025,\"Peak flux density : fitted power law norm vs. time (Rate at 1000 Jy)\")"

root -l "plot_norm_vs_time_error.C(\"lumin_vs_time.txt\",0,2,\"Spectral Luminosity : fitted power law norm vs. time (Rate at 7*10^{23} erg/s/Hz)\")"

cp ~/github/crab_frb_paper/scripts/root/plot_plidx_vs_time_error.C .
root -l "plot_plidx_vs_time_error.C(\"snr_vs_time.txt\",-6,0,\"SNR : fitted power law index vs. time\")"
root -l "plot_plidx_vs_time_error.C(\"lumin_vs_time.txt\",-6,0,\"Spectral luminosity : fitted power law index vs. time\")"
root -l "plot_plidx_vs_time_error.C(\"flux_vs_time.txt\",-6,0,\"Peak flux density : fitted power law index vs. time\")"

# Plot all fluence and total distribution of fluence from all observations :
# TODO : add question if show all plots (default YES) :
root_options="-l"
read -p "Show plots of fluence distribution for every night ? [y/n]: " answer
if [[ $answer == "n" || $answer == "N" ]]; then
   root_options="-l -q -b"
fi

echo "~/github/crab_frb_paper/scripts/calib/replot_fluence_vs_time.sh - \"${root_options}\" - ${fluence_completness_threshold}"
~/github/crab_frb_paper/scripts/calib/replot_fluence_vs_time.sh - "${root_options}" - ${fluence_completness_threshold}


# Plot max fluence vs. time :
echo "~/github/crab_frb_paper/scripts/calib/plot_maxfluence_vs_time.sh"
~/github/crab_frb_paper/scripts/calib/plot_maxfluence_vs_time.sh

# MAX of fluence calculated from integral of fitted pulse profiles:
echo "~/github/crab_frb_paper/scripts/calib/plot_fitted_maxfluence_vs_time.sh"
~/github/crab_frb_paper/scripts/calib/plot_fitted_maxfluence_vs_time.sh


# Scattering vs. time :
# cd ${base_path}/
cd ${base_path}/${outdir}/merged
echo "~/github/crab_frb_paper/scripts/calib/scattering_time_from_all_fits.sh"
~/github/crab_frb_paper/scripts/calib/scattering_time_from_all_fits.sh 

# scattering vs. number of GPs :
awk '{print $1" "$3}' taumean_vs_time.txt > taumean_vs_time_2col.txt
correlate_files taumean_vs_time_2col.txt ngps_vs_uxtime.txt Ngps_vs_taufit.txt -i 3600 -c 2
root -l "plot_ngps_vs_tau.C(\"Ngps_vs_taufit.txt\")"

# MPs and IPs :
echo "~/github/crab_frb_paper/scripts/calib/plot_fluence_mp_and_ip_all.sh - ${total_time_hour} ${fluence_completness_threshold}"
~/github/crab_frb_paper/scripts/calib/plot_fluence_mp_and_ip_all.sh - ${total_time_hour} ${fluence_completness_threshold}

