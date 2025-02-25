#!/bin/bash

sefd=7911
if [[ -n "$1" && "$1" != "-" ]]; then
   sefd=$1
fi

snr_threshold=40
if [[ -n "$2" && "$2" != "-" ]]; then
   snr_threshold=$2
fi


datfile=../_DM56.72.dat
if [[ -n "$3" && "$3" != "-" ]]; then
   datfile=$3
fi


echo "~/github/presto_tools/build/extract_pulses ${datfile} presto.txt -X 7937.64 -C -t $snr_threshold -P pulses_snr${snr_threshold}_calibrated/ -r 100 -U 1"
~/github/presto_tools/build/extract_pulses ${datfile} presto.txt -X 7937.64 -C -t $snr_threshold -P pulses_snr${snr_threshold}_calibrated/ -r 100 -U 1

cd pulses_snr${snr_threshold}_calibrated/

cp ~/github/crab_frb_paper/scripts/root/plot_psr_profile.C .

mkdir -p images
for file in `ls pulse*.txt`
do
   root -l "plot_psr_profile.C(\"${file}\",2,1)"   
done

cat *.refit | awk '{print $9;}' > tau.txt
root -l "histofile.C(\"tau.txt\",0,1,0,0.01)"