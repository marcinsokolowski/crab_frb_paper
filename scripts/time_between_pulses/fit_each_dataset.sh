#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/analysis_final/merged"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

outdir=~/Desktop/EDA2/papers/2024/EDA2_FRBs/PAPER/Time_between_pulses/images/

root_options="-l" # -b -q 

curr_path=`pwd`
for datadir in `ls -d ${template}`
do
   dir_up=`dirname $datadir`

   echo 
   echo
   echo  
   cd $datadir
   
   uxtime=-1
   if [[ -s ${dir_up}/../J0534+2200_flagants_ch40_ch256/256/UNIXTIME.txt ]]; then
      uxtime=`cat ${dir_up}/../J0534+2200_flagants_ch40_ch256/256/UNIXTIME.txt`
   else
      echo "ERROR : could not find file ${dir_up}/../J0534+2200_flagants_ch40_ch256/256/UNIXTIME.txt"
      exit
   fi
   ut=`date2date -ux2ut=${uxtime}`

   cp ~/github/crab_frb_paper/scripts/root/histo_time_between_pulses_pdfexp.C .

   mkdir -p images   
   root ${root_options} "histo_time_between_pulses_pdfexp.C(\"${ut}_time_diff.txt\",0,0,1000,1,1,0,1000,1000,$uxtime)"

   pwd 
   p=`pwd`  
   ls -al ${p}/images/${ut}_time_diff.txt_time_between_pulses.png
   
   echo "----------------------------------------------------------------------------"
   echo "cp ${p}/images/${ut}_time_diff.txt_time_between_pulses.png ${outdir}"
   cp ${p}/images/${ut}_time_diff.txt_time_between_pulses.png ${outdir}
   echo "----------------------------------------------------------------------------"
   
   cd $curr_path
#   exit
done


cat ${template}/time_between_pulses_fit_results_EXP.txt > ALL_time_between_pulses_fit_results_EXP_FIT.txt
awk '{print $1" "43200" "$2" "$3;}' ALL_time_between_pulses_fit_results_EXP_FIT.txt > ALL_time_between_pulses_fit_results_EXP_FIT_ERR.txt

mkdir -p images/
root -l "plot_lambda_vs_time.C(\"ALL_time_between_pulses_fit_results_EXP_FIT_ERR.txt\")"
