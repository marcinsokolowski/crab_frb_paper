#!/bin/bash

file=pulse_31489249519_sum.dm56.722.psr
if [[ -n "$1" && "$1" != "-" ]]; then
   file="$1"
fi

unwraped_file=${file%%.psr}_UNWRAPED.psr
if [[ -n "$2" && "$2" != "-" ]]; then
   unwraped_file="$2"
fi


n=500
if [[ -n "$3" && "$3" != "-" ]]; then
   n=$3
fi


max_phase=`cat $file | tail -1 | awk '{print $1;}'`
delta_phase=`awk -v prev=0 '{if(NR==2){print $1-prev;}prev=$1;}' $file`

echo "DEBUG : max_phase = $max_phase, delta_phase = $delta_phase"

cat ${file} | awk -v n=${n} -v delta_phase=0.0000326098 '{bin=(NR-1);amp=$2;if(NR<=n){bin=1024+bin;}printf("%.8f %.8f\n",(bin-n)*delta_phase,amp);}' |sort -n > ${unwraped_file}



