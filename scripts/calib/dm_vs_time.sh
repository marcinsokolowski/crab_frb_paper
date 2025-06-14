#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

start_dm=56.600
end_dm=56.800
step_dm=0.001
outdir=ScanDM_NEW/
force=0

echo "-----------------------------------------------------"
echo "List of data to process"
echo "-----------------------------------------------------"
ls -d ${template}
echo "-----------------------------------------------------"
sleep 5

curr_path=`pwd`
for dir in `ls -d ${template}`
do
   echo
   cd $dir   
   candfile=`ls merged_channels_??????????.cand_merged | tail -1`
   ux=`echo $candfile | cut -b 17-26`
   maxsnr_id=`awk -v maxsnr=-1 -v id=0 '{if($1!="#"){if($3>maxsnr){maxsnr=$3;id=$1;}}}END{print id;}' ${candfile}`
   echo "MAX SNR candidate ID = $maxsnr_id"
   cd candidates_fits
   pwd
   echo "gzip -d mergedcand${maxsnr_id}*.fits.gz"
   gzip -d mergedcand${maxsnr_id}*.fits.gz
   maxsnr_fits=`ls mergedcand${maxsnr_id}*.fits`
   
   if [[ -s ${maxsnr_fits} ]]; then
      if [[ -d ${outdir} && $force -le 0 ]]; then
         echo "De-dispersion (DM)-scan already performed, set force=1 to repeat"
      else
         echo "INFO : running de-dispersion scan now"
         echo "~/github/frb_search/build_20250313/dedisperse_fits $maxsnr_fits test.fits -o ${outdir} -S ${start_dm} -E ${end_dm} -s ${step_dm} -U $ux > dm_scan.out 2>&1"
         sleep 1
         ~/github/frb_search/build_20250313/dedisperse_fits $maxsnr_fits test.fits -o ${outdir} -S ${start_dm} -E ${end_dm} -s ${step_dm} -U $ux > dm_scan.out 2>&1
      fi
      
      cd ${outdir}
      echo "~/github/crab_frb_paper/scripts/calib/fit_fwhm_all.sh $ux"
      ~/github/crab_frb_paper/scripts/calib/fit_fwhm_all.sh $ux
      cd ..
   else
      echo "WARNING : max SNR FITS file $maxsnr_fits not found -> skipped"
   fi
   cd ..
   cd $curr_path
done

echo "PROGRESS : creating final plots - this may take a while to generate ..."

cat ${template}/candidates_fits/DM.txt | awk '{print $1" 0 "$2" 0";}'  > dm_vs_time.txt
cp ~/github/crab_frb_paper/scripts/root/plot_dm_vs_time.C .
root -l "plot_dm_vs_time.C(\"dm_vs_time.txt\")"

cat ${template}/candidates_fits/ScanDM_NEW/DM_FWHM.txt > dm_vs_time_fwhm.txt
cp ~/github/crab_frb_paper/scripts/root/plot_dm_vs_time.C .
root -l "plot_dm_vs_time.C(\"dm_vs_time_fwhm.txt\")"

cat ${template}/candidates_fits/ScanDM_NEW/TAU_FWHM_TIME.txt | awk '{print $1" "$2" "$3*1000.00" "$4*1000.00;}' > taufwhm_vs_time.txt
cp ~/github/crab_frb_paper/scripts/root/plot_scattau_vs_time.C .
root -l "plot_scattau_vs_time.C(\"taufwhm_vs_time.txt\")"
