#!/bin/bash

fits_file=mergedcand00233_start4370931_end4373816.fits
if [[ -n "$1" && "$1" != "-" ]]; then
   fits_file="$1"
fi

start_dm=56.600
end_dm=56.800
step_dm=0.001
outdir=ScanDM_NEW/

ux=`date +%s`

echo "~/github/frb_search/build_20250313/dedisperse_fits $fits_file test.fits -o ${outdir} -S ${start_dm} -E ${end_dm} -s ${step_dm} -U $ux"
~/github/frb_search/build_20250313/dedisperse_fits $fits_file test.fits -o ${outdir} -S ${start_dm} -E ${end_dm} -s ${step_dm} -U $ux


cd ${outdir}/
