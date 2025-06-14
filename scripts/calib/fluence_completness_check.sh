#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged/pulses_snr5_calibrated/calibrated_pulses.txt"
if [[ -n "$1" && "$1" != "-" ]]; then 
   template="$1"
fi

outdir=/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/crab_full_analysis/fluence_completness
if [[ -n "$2" && "$2" != "-" ]]; then
   outdir="$2"
fi

mkdir -p ${outdir}
cd ${outdir}

cat ${template} > all_calibrated_pulses.txt

cp ~/github/crab_frb_paper/scripts/root/plot_snr_vs_fluence.C .
cp ~/github/crab_frb_paper/scripts/root/plot_fluence_vs_snr.C .

awk '{if($1!="#"){print $3" "$6;}}' all_calibrated_pulses.txt > all_prestosnr_vs_fluence.txt
awk '{if($1!="#"){print $6" "$3;}}' all_calibrated_pulses.txt > all_fluence_vs_prestosnr.txt

mkdir -p images/
root -l "plot_snr_vs_fluence.C(\"all_prestosnr_vs_fluence.txt\")"
root -l "plot_fluence_vs_snr.C(\"all_fluence_vs_prestosnr.txt\")"

# completness in flux density :
cp ~/github/crab_frb_paper/scripts/root/plot_snr_vs_flux.C .
awk '{if($1!="#"){print $7" "$6;}}' all_calibrated_pulses.txt > all_prestosnr_vs_fluxdensity.txt
root -l "plot_snr_vs_flux.C(\"all_prestosnr_vs_fluxdensity.txt\")"

cp ~/github/crab_frb_paper/scripts/root/plot_flux_vs_snr.C .
awk '{if($1!="#"){print $6" "$7;}}' all_calibrated_pulses.txt > all_fluxdensity_vs_prestosnr.txt
root -l "plot_flux_vs_snr.C(\"all_fluxdensity_vs_prestosnr.txt\")"


# Time[sec] MAX_FLUX[Jy] FLUENCE[Jy ms] SNR SNR_FLUX[Jy] PRESTO_SNR PRESTO_FLUX[Jy]
# 205.61942016 492633280.00000000 11228.58761628 66.65437317 3120.68264347 83.84000000 3925.29415493
# Fluence vs. flux density :
awk '{if($1!="#"){print $7" "$3;}}' all_calibrated_pulses.txt > prestofluxdensity_vs_fluence.txt
cp ~/github/crab_frb_paper/scripts/root/plot_fluence_vs_flux.C .
root -l "plot_fluence_vs_flux.C(\"prestofluxdensity_vs_fluence.txt\")"



