#!/bin/bash

# cd /media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/

# calculate total time :
total_time_sec=`grep "Time per file" 202?_??_??_pulsars_msok/analysis/updated.hdr | awk -v sum=0.00 '{sum+=$7;}END{print sum;}'`
total_time_hour=`grep "Time per file" 202?_??_??_pulsars_msok/analysis/updated.hdr | awk -v sum=0.00 '{sum+=$7;}END{print sum/3600.00;}'`

outdir=crab_full_analysis

mkdir -p ${outdir}/merged/

cat 202?_??_??_pulsars_msok/analysis/all_crab_gps_norfi_fluxcal.singlepulse > ${outdir}/all_crab_gps_norfi_fluxcal.singlepulse
cat 202?_??_??_pulsars_msok/analysis/all_crab_gps_norfi.snr > ${outdir}/all_crab_gps_norfi.snr 
# cat 202?_??_??_pulsars_msok/analysis/presto.cand_normal_snr ${outdir}/presto.cand_normal_snr
# cat 202?_??_??_pulsars_msok/analysis/all_crab_gps_norfi_fluxcal.singlepulse > ${outdir}/all_crab_gps_norfi_fluxcal.singlepulse

# merged :
cat 202?_??_??_pulsars_msok/analysis/merged/presto_norfi_fluxcal.cand_normal > ${outdir}/merged/presto_norfi_fluxcal.cand_normal
cat 202?_??_??_pulsars_msok/analysis/merged/presto_norfi_fluxcal.cand_normal > ${outdir}/merged/presto_norfi_fluxcal.cand_normal
cat 202?_??_??_pulsars_msok/analysis/merged/presto.cand_normal_snr > ${outdir}/merged/presto.cand_normal_snr

# GPs vs time for both NON-merged and merged PRESTO GPs:
echo "# UXTIME    #GPs" > ${outdir}/ngps_vs_uxtime.txt
echo "# UXTIME    #GPs" > ${outdir}/merged-ngps_vs_uxtime.txt

for snrfile in `ls 202?_??_??_pulsars_msok/analysis/all_crab_gps_norfi.snr`
do
   dir=`dirname $snrfile`
   obsduration_hours=`cat ${dir}/updated.hdr | grep "Time per file" | awk '{print $6/3600.00;}'`

   cd $dir
   ux=`ls uxtime*txt | awk -v sum=0 -v count=0 '{ux=substr($1,7,10);sum+=ux;count+=1;}END{print sum/count;}'`
   cd -

   count=`cat $snrfile | awk '{if($1!="#"){print $0;}}' | wc -l`
   count_merged=`cat $dir/merged/presto.cand_normal_snr | awk '{if($1!="#"){print $0;}}' | wc -l`

   echo "$ux $count" | awk -v obsduration=${obsduration_hours} '{print $1" "$2/obsduration;}' >> ${outdir}/ngps_vs_uxtime.txt
   echo "$ux $count_merged" | awk -v obsduration=${obsduration_hours} '{print $1" "$2/obsduration;}' >> ${outdir}/merged/merged-ngps_vs_uxtime.txt
done

# PLOTTING :
# NON-MERGED :
cd ${outdir}
# plot distribution of calibrated mean peak flux density :
cp  ~/github/crab_frb_paper/scripts/root/FluRatePerHourPowerLaw.C .
root -l "FluRatePerHourPowerLaw.C(\"all_crab_gps_norfi_fluxcal.singlepulse\",${total_time_hour})"

cp  ~/github/crab_frb_paper/scripts/root/SNRRatePerHourPowerLaw.C .
root -l "SNRRatePerHourPowerLaw.C(\"all_crab_gps_norfi.snr\",${total_time_hour})"

# plot disitrbution of luminosity :
cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
root -l "SpectralLuminosity_DistrPowerLaw.C(\"all_crab_gps_norfi_fluxcal.singlepulse\",${total_time_hour})"

# plot N GPs vs. time :
cp ~/github/crab_frb_paper/scripts/root/plot_ngps_vs_time.C .
root -l "plot_ngps_vs_time.C(\"ngps_vs_uxtime.txt\")"


# PLOTTING MERGED :
cd merged/
# plot distribution of calibrated mean peak flux density :
cp  ~/github/crab_frb_paper/scripts/root/FluRatePerHourPowerLaw.C .
root -l "FluRatePerHourPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\",${total_time_hour})"

cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
root -l "SpectralLuminosity_DistrPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\",${total_time_hour})"

# plots SNR distribution 
cp  ~/github/crab_frb_paper/scripts/root/SNRRatePerHourPowerLaw.C .
root -l "SNRRatePerHourPowerLaw.C(\"presto.cand_normal_snr\",${total_time_hour})"


# plot N GPs vs. time :
cp ~/github/crab_frb_paper/scripts/root/plot_ngps_vs_time.C .
root -l "plot_ngps_vs_time.C(\"merged-ngps_vs_uxtime.txt\")"

