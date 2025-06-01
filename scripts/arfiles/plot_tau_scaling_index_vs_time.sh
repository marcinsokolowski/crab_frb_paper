#!/bin/bash

template="/media/msok/f5ce6064-9dc3-40c3-bc9c-7314e8594519/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec/scamp/"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

set_zero_freq_errors=0
if [[ -n "$2" && "$2" != "-" ]]; then
   set_zero_freq_errors=$2
fi

min_fit_points=2
if [[ -n "$3" && "$3" != "-" ]]; then
   min_fit_points=$3
fi

root_options="-l -q -b"
if [[ -n "$4" && "$4" != "-" ]]; then
   root_options="$4"
fi

sleep_time=0

path=`pwd`


for dataset in `ls -d ${template}`
do
   cd $dataset
   if [[ -d output/results/ ]]; then
      unixtime=`cat ../UNIXTIME.txt`
      cd output/results/

      rm -f FIT.txt
      
      cp ../../../UNIXTIME.txt .
      csv_file=`ls pulse*output.csv | tail -1`
      if [[ -s ${csv_file} ]]; then
         awk -F "," '{if(NR>1){print $9" 3.90625 "$20" "$21;}}' ${csv_file} | awk '{if(NF>2){print $0;}}' > tau_vs_freq.txt
                  
         if [[ -s tau_vs_freq.txt ]]; then
            lines_count=`cat tau_vs_freq.txt | wc -l`

            if [[ $lines_count -ge $min_fit_points ]]; then
               echo "cp ~/github/crab_frb_paper/scripts/root/plot_power_law_loglog.C ."   
               cp ~/github/crab_frb_paper/scripts/root/plot_power_law_loglog.C .
   
   
               root ${root_options} "plot_power_law_loglog.C(\"tau_vs_freq.txt\",NULL,${unixtime},${set_zero_freq_errors})"
            else
               pwd
               echo "WARNING : less than 2 lines in file tau_vs_freq.txt -> skipped"
               root ${root_options}
            fi
            if [[ $sleep_time -gt 0 ]]; then
               echo "sleep $sleep_time"
               sleep $sleep_time
            fi
         else
            echo "WARNING : tau_vs_freq.txt is empty (not good fit)"
         fi
      fi
      cd $path
   else
      echo "WARNING : output/results/ subdirectory does not exist -> ignored"
   fi
done


echo "cat ${template}/output/results/FIT.txt > tau_index_vs_time.txt"
cat ${template}/output/results/FIT.txt > tau_index_vs_time.txt

cp ~/github/crab_frb_paper/scripts/root/plot_tauindex_vs_dm.C .

root -l "plot_tauindex_vs_time.C(\"tau_index_vs_time.txt\")"

# awk '{if($7<=5){print $0;}}' tau_index_vs_time.txt > tau_index_vs_time_chi2ndfLT5.txt
# root -l "plot_tauindex_vs_time.C(\"tau_index_vs_time_chi2ndfLT5.txt\")"

awk '{if($7<=3 && $7>0.5 && $3<0){print $0;}}' tau_index_vs_time.txt > tau_index_vs_time_chi2ndfLT3.txt
root -l "plot_tauindex_vs_time.C(\"tau_index_vs_time_chi2ndfLT3.txt\")"

