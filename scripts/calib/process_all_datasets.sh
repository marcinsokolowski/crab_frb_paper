#!/bin/bash

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

   cd $dir
   ux=`ls uxtime*txt | awk -v sum=0 -v count=0 '{ux=substr($1,7,10);sum+=ux;count+=1;}END{print sum/count;}'`
   cd -

   count=`cat $snrfile | awk '{if($1!="#"){print $0;}}' | wc -l`
   count_merged=`cat $dir/merged/presto.cand_normal_snr | awk '{if($1!="#"){print $0;}}' | wc -l`

   echo "$ux $count" >> ${outdir}/ngps_vs_uxtime.txt
   echo "$ux $count_merged" >> ${outdir}/merged/merged-ngps_vs_uxtime.txt
done

# PLOTTING :
# NON-MERGED :
cd ${outdir}
# plot distribution of calibrated mean peak flux density :
cp  ~/github/crab_frb_paper/scripts/root/FluDistrPowerLaw.C .
root -l "FluDistrPowerLaw.C(\"all_crab_gps_norfi_fluxcal.singlepulse\")"

cp  ~/github/crab_frb_paper/scripts/root/SNRDistrPowerLaw.C .
root -l "SNRDistrPowerLaw.C(\"all_crab_gps_norfi.snr\")"

# plot disitrbution of luminosity :
cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
root -l "SpectralLuminosity_DistrPowerLaw.C(\"all_crab_gps_norfi_fluxcal.singlepulse\")"

# plot N GPs vs. time :
cp ~/github/crab_frb_paper/scripts/root/plot_ngps_vs_time.C .
root -l "plot_ngps_vs_time.C(\"ngps_vs_uxtime.txt\")"


# PLOTTING MERGED :
cd merged/
# plot distribution of calibrated mean peak flux density :
cp  ~/github/crab_frb_paper/scripts/root/FluDistrPowerLaw.C .
root -l "FluDistrPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\")"

cp ~/github/crab_frb_paper/scripts/root/SpectralLuminosity_DistrPowerLaw.C .
root -l "SpectralLuminosity_DistrPowerLaw.C(\"presto_norfi_fluxcal.cand_normal\")"

# plots SNR distribution 
cp  ~/github/crab_frb_paper/scripts/root/SNRDistrPowerLaw.C .
root -l "SNRDistrPowerLaw.C(\"presto.cand_normal_snr\")"


# plot N GPs vs. time :
cp ~/github/crab_frb_paper/scripts/root/plot_ngps_vs_time.C .
root -l "plot_ngps_vs_time.C(\"merged-ngps_vs_uxtime.txt\")"

