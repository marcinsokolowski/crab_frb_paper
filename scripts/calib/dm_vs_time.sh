#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

start_dm=56.600
end_dm=56.800
step_dm=0.001
outdir=ScanDM_NEW/


curr_path=`pwd`
for dir in `ls -d ${template}`
do
   echo
   cd $dir   
   candfile=`ls merged_channels_*.cand_merged | tail -1`
   ux=`echo $candfile | cut -b 17-26`
   maxsnr_id=`awk -v maxsnr=-1 -v id=0 '{if($1!="#"){if($3>maxsnr){maxsnr=$3;id=$1;}}}END{print id;}' ${candfile}`
   echo "MAX SNR candidate ID = $maxsnr_id"
   cd candidates_fits
   pwd
   echo "gzip -d mergedcand${maxsnr_id}*.fits.gz"
   gzip -d mergedcand${maxsnr_id}*.fits.gz
   maxsnr_fits=`ls mergedcand${maxsnr_id}*.fits`
   
   if [[ -s ${maxsnr_fits} ]]; then
      echo "~/github/frb_search/build_20250313/dedisperse_fits $maxsnr_fits test.fits -o ${outdir} -S ${start_dm} -E ${end_dm} -s ${step_dm} -U $ux > dm_scan.out 2>&1"
      sleep 1
      ~/github/frb_search/build_20250313/dedisperse_fits $maxsnr_fits test.fits -o ${outdir} -S ${start_dm} -E ${end_dm} -s ${step_dm} -U $ux > dm_scan.out 2>&1
      
   else
      echo "WARNING : max SNR FITS file $maxsnr_fits not found -> skipped"
   fi
   cd ..
   cd $curr_path
done


cat ${template}/candidates_fits/DM.txt 
