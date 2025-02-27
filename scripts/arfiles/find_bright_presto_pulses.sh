#!/bin/bash

pulsefile=_DM57.00.singlepulse
if [[ -n "$1" && "$1" != "-" ]]; then
   pulsefile="$1"
fi

snr_threshold=50
if [[ -n "$2" && "$2" != "-" ]]; then
   snr_threshold=$2
fi

b=${pulsefile%%.singlepulse}
outfile=pulses_snr${snr_threshold}_${b}.txt
if [[ -n "$3" && "$3" != "-" ]]; then
   outfile="$3"
fi

cat ${pulsefile} | awk -v snr_threshold=${snr_threshold} '{if($1!="#" && $2>snr_threshold){print $0;}}' > ${outfile}


echo "List of bright pulses above threshold = $snr_threshold:"
cat ${outfile}




