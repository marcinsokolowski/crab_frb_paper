#!/bin/bash

n=4
if [[ -n "$1" && "$1" != "-" ]]; then
   n=$1
fi

outdir=merged_${n}/
if [[ -n "$2" && "$2" != "-" ]]; then
   outdir="$2"
fi

mkdir -p ${outdir}
for psrfile in `ls *.psr`
do
   echo "python ~/github/crab_frb_paper/scripts/python/average_n_points.py $psrfile ${outdir}/${psrfile} --n_merge=${n} --no_errors"
   python ~/github/crab_frb_paper/scripts/python/average_n_points.py $psrfile ${outdir}/${psrfile} --n_merge=${n} --no_errors
done
