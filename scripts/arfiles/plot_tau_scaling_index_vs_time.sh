#!/bin/bash

template="/media/msok/f5ce6064-9dc3-40c3-bc9c-7314e8594519/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec/scamp/"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

path=`pwd`


for dataset in `ls -d ${template}`
do
   cd $dataset
   if [[ -d output/results/ ]]; then
      unixtime=`cat ../UNIXTIME.txt`
      cd output/results/
      cp ../../../UNIXTIME.txt .
      awk -F "," '{if(NR>1){print $9" 3.90625 "$20" "$21;}}' pulse_*_sum.ar.4ch.ascii_output.csv | awk '{if(NF>2){print $0;}}' > tau_vs_freq.txt

      echo "cp ~/github/crab_frb_paper/scripts/root/plot_power_law.C ."   
      cp ~/github/crab_frb_paper/scripts/root/plot_power_law.C .
   
   
      root -l "plot_power_law.C(\"tau_vs_freq.txt\",NULL,${unixtime})"
      cd $path
   else
      echo "WARNING : output/results/ subdirectory does not exist -> ignored"
   fi
done


echo "cat ${template}/output/results/FIT.txt > tau_index_vs_time.txt"
cat ${template}/output/results/FIT.txt > tau_index_vs_time.txt

cp ~/github/crab_frb_paper/scripts/root/plot_tauindex_vs_dm.C .

root -l "plot_tauindex_vs_dm.C(\"tau_index_vs_time.txt\")"
