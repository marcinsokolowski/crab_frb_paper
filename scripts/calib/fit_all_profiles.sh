#!/bin/bash

template="snr*_snr*_time*sec.txt"
root_options="-b -q -l"

echo "rm -f fitted_fluence.txt"
rm -f fitted_fluence.txt

echo "cp ~/github/crab_frb_paper/scripts/root/plot_psr_snr.C ."
cp ~/github/crab_frb_paper/scripts/root/plot_psr_snr.C .

mkdir -p images/
for file in `ls ${template}`
do
   root -b -q -l "plot_psr_snr.C+(\"${file}\")"
done

cat fitted_fluence.txt | awk '{print $2;}' > fitted_fluence_1col.txt

TotalTimeHours=`cat ../../../../../../../analysis_final/TotalGoodTimeInSec.txt | awk '{print $1/3600.00}'`

cp ~/github/crab_frb_paper/scripts/root/FluenceRatePerHourPowerLaw.C .
root -l "FluenceRatePerHourPowerLaw.C(\"fitted_fluence_1col.txt\",${TotalTimeHours},0,800,10000)"
root -l "FluenceRatePerHourPowerLaw.C(\"fitted_fluence_1col.txt\",${TotalTimeHours},0,600,10000)"
