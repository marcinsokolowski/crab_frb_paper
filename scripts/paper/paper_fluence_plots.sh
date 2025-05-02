#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged/pulses_snr5_calibrated/"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

file="calibrated_fluence.txt.fit_results_fit_range_800.00-10000.00"

all_file=all_${file}

cat ${template}/${file}  > ${all_file}

# plot fit norm (par[0] vs. time)
mkdir -p crab_paper/PAPER/
cat ${all_file} | awk '{if($1!="#"){print $7" 43200 "$1" "$3;}}' > crab_paper/PAPER/all_fluence_fit_norm_vs_uxtime.txt
cat ${all_file} | awk '{if($1!="#"){print $7" 43200 "$2" "$4;}}' > crab_paper/PAPER/all_fluence_fit_plindex_vs_uxtime.txt
cat ${all_file} | awk '{if($1!="#"){print $7" 43200 "$8" 200";}}' > crab_paper/PAPER/all_fluence_turnoverfluence_vs_uxtime.txt
cat ${all_file} | awk '{if($1!="#"){print $7" 43200 "$11" 200";}}' > crab_paper/PAPER/all_fluence_onehourfluence_vs_uxtime.txt
cat ${all_file} | awk '{if($1!="#"){print $7" "$10;}}' > crab_paper/PAPER/all_fluence_ngps_vs_uxtime.txt

cd crab_paper/PAPER/
cp ~/github/crab_frb_paper/scripts/root/plot_ngps_vs_time_error_PAPER.C .
root -l "plot_ngps_vs_time_error_PAPER.C(\"all_fluence_ngps_vs_uxtime.txt\")"

cp ~/github/crab_frb_paper/scripts/root/plot_plidx_vs_time_error.C .
root -l "plot_plidx_vs_time_error.C(\"all_fluence_fit_plindex_vs_uxtime.txt\")"

cp ~/github/crab_frb_paper/scripts/root/plot_plnorm_vs_time_error.C .
root -l "plot_plnorm_vs_time_error.C(\"all_fluence_fit_norm_vs_uxtime.txt\")"

cp ~/github/crab_frb_paper/scripts/root/plot_fluenceinfo_vs_time_error_PAPER.C .
root -l "plot_fluenceinfo_vs_time_error_PAPER.C(\"all_fluence_onehourfluence_vs_uxtime.txt\",\"One hour fluence [Jy ms]\")"

root -l "plot_fluenceinfo_vs_time_error_PAPER.C(\"all_fluence_turnoverfluence_vs_uxtime.txt\",\"Turn-over fluence (incompletness) [Jy ms]\",200,2000)"






