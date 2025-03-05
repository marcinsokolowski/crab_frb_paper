#!/bin/bash

sefd=7911
count=`ls ../../../../../../analysis*/MEAN_SEFD.txt |wc -l`
if [[ $count -gt 0 ]]; then
   echo "cat ../../../../../../analysis*/MEAN_SEFD.txt | tail -1"      
   sefd=`cat ../../../../../../analysis*/MEAN_SEFD.txt | tail -1`
fi   
if [[ -n "$1" && "$1" != "-" ]]; then
   sefd=$1
fi

# max_snr=`awk -v maxsnr=-1 '{if($2>maxsnr){maxsnr=$2;}}END{print maxsnr-0.01;}' presto.txt`
# just do max snr one 
snr_threshold=5
if [[ -n "$2" && "$2" != "-" && $2 -gt 0 ]]; then
   snr_threshold=$2
fi


datfile=../_DM56.72.dat
if [[ -n "$3" && "$3" != "-" ]]; then
   datfile=$3
fi

root_options="-l"
if [[ -n "$4" && "$4" != "-" ]]; then
   root_options="$4"
fi

replot_only=0
if [[ -n "$5" && "$5" != "-" ]]; then
   replot_only=$5
fi


outdir=pulses_snr${snr_threshold}_calibrated/

if [[ ! -d pulses_snr${snr_threshold}_calibrated/ ]]; then
   running_median_file=running_median.txt
   rmqiqr_file=running_rmsiqr.txt
   detrendnorm_file=detrended_normalised_${b}.txt
   calibrated_pulses_file=calibrated_pulses.txt
      
   mkdir -p ${outdir}/
   
   if [[ $replot_only -le 0 ]]; then
      timeseries_file=timeseries.txt
      echo "~/github/presto_tools/build/presto_data_reader $datfile -o ${outdir}/${timeseries_file} ${options}"
      ~/github/presto_tools/build/presto_data_reader $datfile -o ${outdir}/${timeseries_file} ${options}
   
      if [[ ! -s presto_merged.txt ]]; then
         awk '{if($1!="#"){print $3" "$2;}}' presto.cand_normal > presto_merged.txt
      fi

      echo "~/github/presto_tools/build/extract_pulses_new ${datfile} presto_merged.txt -X ${sefd} -C -t $snr_threshold -P pulses_snr${snr_threshold}_calibrated/ -r 100 -U 1 -R ${outdir}/${running_median_file} -I ${outdir}/${rmqiqr_file} -o ${outdir}/${detrendnorm_file} -O ${outdir}/${calibrated_pulses_file} > extract_pulses_new.out 2>&1"
      ~/github/presto_tools/build/extract_pulses_new ${datfile} presto_merged.txt -X ${sefd} -C -t $snr_threshold -P pulses_snr${snr_threshold}_calibrated/ -r 100 -U 1 -R ${outdir}/${running_median_file} -I ${outdir}/${rmqiqr_file} -o ${outdir}/${detrendnorm_file} -O ${outdir}/${calibrated_pulses_file} > extract_pulses_new.out 2>&1
   else
      echo "INFO : replotting only -> no need to extrac pulses again"
   fi   
   
   cd ${outdir}/
   awk '{if($1!="#"){print $2;}}' calibrated_pulses.txt > calibrated_flux.txt
   awk '{if($1!="#"){print $3;}}' calibrated_pulses.txt > calibrated_fluence.txt
   awk '{if($1!="#"){print $4;}}' calibrated_pulses.txt > calibrated_snr.txt

   TotalTimeHours=`cat ../../../../../../../analysis_final/TotalGoodTimeInSec.txt | awk '{print $1/3600.00}'`
   
   cp ~/github/crab_frb_paper/scripts/root/FluenceRatePerHourPowerLaw.C .
   root ${root_options} "FluenceRatePerHourPowerLaw.C(\"calibrated_fluence.txt\",${TotalTimeHours},0,800,10000)"
   root ${root_options} "FluenceRatePerHourPowerLaw.C(\"calibrated_fluence.txt\",${TotalTimeHours},0,600,10000)"
   
   echo "PARAMETERS : SEFD = $sefd, TotalTimeHours=$TotalTimeHours"
   
   cd -
else
   echo "INFO : Pulses already dumped"
fi   

