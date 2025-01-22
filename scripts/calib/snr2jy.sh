#!/bin/bash

files="_DM*.singlepulse"
if [[ -n "$1" && "$1" != "-" ]]; then
   files="$1"
fi

sefd=7911
if [[ -n "$2" && "$2" != "-" ]]; then
   sefd=$2
fi

cat ${files} | grep -v "#" | awk -v sefd=${sefd} -v npol=2 -v bw_hz=31250000 -v timeres_sec=0.001 '{snr=$2;sigma_sim=sefd/sqrt(npol*bw_hz*timeres_sec);flu=snr*sigma_sim;print snr" -> "flu;}'
