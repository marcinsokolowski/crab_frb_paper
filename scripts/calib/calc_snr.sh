#!/bin/bash

fits_file=mergedcand00233_start4370931_end4373816.fits
if [[ -n "$1" && "$1" != "-" ]]; then
   fits_file="$1"
fi

root_options="-l"

outdir=NoNorm/

echo "~/github/frb_search/build_20250313/dedisperse_fits $fits_file test.fits -o ${outdir}"
~/github/frb_search/build_20250313/dedisperse_fits $fits_file test.fits -o ${outdir}

cd ${outdir}/

timeseries_file=timeseries.txt
median_file=median_rmsiqr.txt

awk '{print $1" "$2+$3;}' ${median_file} > up.txt
awk '{print $1" "$2-$3;}' ${median_file} > down.txt

echo "cp ~/github/crab_frb_paper/scripts/root/plot_timeseries_list.C ."
cp ~/github/crab_frb_paper/scripts/root/plot_timeseries_list.C .

echo ${timeseries_file} > list
echo ${median_file} >> list
echo up.txt >> list
echo down.txt >> list

mkdir -p images/
root ${root_options} "plot_timeseries_list.C+(\"list\")"

cp ~/github/crab_frb_paper/scripts/root/plot_psr_snr.C .
root ${root_options} "plot_psr_snr.C(\"snr_vs_time.txt\",1.00,0,true,NULL)"
