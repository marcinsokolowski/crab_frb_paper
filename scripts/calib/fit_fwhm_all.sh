#!/bin/bash

unixtime=-1
if [[ -n "$1" && "$1" != "-" ]]; then
   unixtime=$1
fi

echo "cp ~/github/crab_frb_paper/scripts/root/plot_psr_snr_width.C ."
cp ~/github/crab_frb_paper/scripts/root/plot_psr_snr_width.C .

echo "mv FWHM.txt FWHM.txt.old"
mv FWHM.txt FWHM.txt.old

echo "mv TAU_FWHM.txt TAU_FWHM.txt.old"
mv TAU_FWHM.txt TAU_FWHM.txt.old

mkdir -p images/
for snrfile in `ls snr*.txt`
do
   maxx=`awk -v maxy=-1 -v maxx=-1 '{if($2>maxy){maxy=$2;maxx=$1;}}END{print maxx}' ${snrfile}`
   maxy=`awk -v maxy=-1 -v maxx=-1 '{if($2>maxy){maxy=$2;maxx=$1;}}END{print maxy}' ${snrfile}`
   
   start_x=`echo $maxx | awk '{print $1-20*0.001}'`
   end_x=`echo $maxx | awk '{print $1+40*0.001}'`

   root -b -l -q "plot_psr_snr_width.C+(\"${snrfile}\",${start_x},${end_x})"

   pwd
done

mv FWHM.txt FWHM_tmp.txt
cat FWHM_tmp.txt | awk '{gsub("snr_vs_time_dm","",$0);gsub(".txt","",$0);print $0;}' > FWHM.txt

mkdir -p images/
cp ~/github/crab_frb_paper/scripts/root/plot_fwhm_vs_dm.C .
root -b -l -q "plot_fwhm_vs_dm.C(\"FWHM.txt\",${unixtime})"

# get scattering time (Tau) corresponding to optimal DM 
echo "cat DM_FWHM.txt | awk"   
dm_fwhm=`cat DM_FWHM.txt | awk '{print substr($3,1,9);}'`
echo "grep $dm_fwhm TAU_FWHM.txt | awk"
tau_fwhm=`grep $dm_fwhm TAU_FWHM.txt | awk '{print $2;}'`
tau_fwhm_err=`grep $dm_fwhm TAU_FWHM.txt | awk '{print $3;}'`
echo "$unixtime 43200 $tau_fwhm $tau_fwhm_err" > TAU_FWHM_TIME.txt      
