#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/analysis_final/merged"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

snr_threshold=5.00
if [[ -n "$2" && "$2" != "-" ]]; then
   snr_threshold=$2
fi


outdir="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/crab_full_analysis/time_between_pulses"
rm -f ${outdir}/all_good_time_diff_snr${snr_threshold}.txt ${outdir}/all_time_diff_snr${snr_threshold}.txt
rm -f ${outdir}/all_good_time_between_pulses_fit_results_snr${snr_threshold}.txt ${outdir}/all_time_between_pulses_fit_results_snr${snr_threshold}.txt

root_options="-l -b -q" # -b -q 

curr_path=`pwd`
for datadir in `ls -d ${template}`
do
   dir_up=`dirname $datadir`

   echo
   cd $datadir
   good_time=-1
   if [[ -s ${dir_up}/TotalGoodTimeInSec.txt ]]; then
      good_time=`cat ${dir_up}/TotalGoodTimeInSec.txt | awk '{printf("%d\n",$1);}'`
   else
      echo "ERROR : unknown good time -> dataset skipped"
      exit;
   fi
   
   uxtime=-1
   if [[ -s ${dir_up}/../J0534+2200_flagants_ch40_ch256/256/UNIXTIME.txt ]]; then
      uxtime=`cat ${dir_up}/../J0534+2200_flagants_ch40_ch256/256/UNIXTIME.txt`
   else
      echo "ERROR : could not find file ${dir_up}/../J0534+2200_flagants_ch40_ch256/256/UNIXTIME.txt"
      exit
   fi
   ut=`date2date -ux2ut=${uxtime}`
   
   pwd
   awk -v snr_threshold=${snr_threshold} '{if($1!="#"){if($3>=snr_threshold){print $5;}}}' presto.cand_merged |sort -n | awk -v prev_time=0 '{if(NR>1){print $1-prev_time;}prev_time=$1;}' > ${ut}_time_diff_snr${snr_threshold}.txt

   mkdir -p images   
   cp ~/github/crab_frb_paper/scripts/root/histo_time_between_pulses.C .
   root ${root_options} "histo_time_between_pulses.C(\"${ut}_time_diff.txt\")"
        
   if [[ $good_time -ge 3000 ]]; then
      # only dates with good amount of good time (no RFI -> otherwise wrong distribution of intervals when RFI times are removed):
      cat ${ut}_time_diff_snr${snr_threshold}.txt >> ${outdir}/all_good_time_diff_snr${snr_threshold}.txt
      cat time_between_pulses_fit_results.txt | awk -v good_time=${good_time} -v uxtime=${uxtime} '{print uxtime" "$0" "good_time;}' >> ${outdir}/all_good_time_between_pulses_fit_results_snr${snr_threshold}.txt
   fi
   
   # all pulses go here (including good and bad nights)
   cat ${ut}_time_diff_snr${snr_threshold}.txt >> ${outdir}/all_time_diff_snr${snr_threshold}.txt
   cat time_between_pulses_fit_results.txt | awk -v good_time=${good_time} -v uxtime=${uxtime} '{print uxtime" "$0" "good_time;}' >> ${outdir}/all_time_between_pulses_fit_results_snr${snr_threshold}.txt            
   
#   sleep 10
   
   cd $curr_path
done

cd ${outdir}/
# cat ${template}/time_diff.txt > all_time_diff.txt
cp ~/github/crab_frb_paper/scripts/root/histo_time_between_pulses.C .
mkdir -p images   
root -l "histo_time_between_pulses.C(\"all_time_diff_snr${snr_threshold}.txt\")"
root -l "histo_time_between_pulses.C(\"all_good_time_diff_snr${snr_threshold}.txt\")"

cat all_good_time_between_pulses_fit_results_snr${snr_threshold}.txt | awk '{print $1" "43200" "$3" "$5;}' > good_time_powerlaw_index_vs_time_snr${snr_threshold}.txt
root -l "plot_timeplidx_vs_time_error.C(\"good_time_powerlaw_index_vs_time_snr${snr_threshold}.txt\")"

cat all_good_time_between_pulses_fit_results_snr${snr_threshold}.txt | awk '{print $1" "43200" "$2" "$4;}' > good_time_powerlaw_norm_vs_time_snr${snr_threshold}.txt
root -l "plot_timeplidx_vs_time_error.C(\"good_time_powerlaw_norm_vs_time_snr${snr_threshold}.txt\",0,50,\"Power law norm\")"
