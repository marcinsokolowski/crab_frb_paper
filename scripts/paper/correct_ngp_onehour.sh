#!/bin/bash

cd /media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2

file=TEST.txt

while read line
do
   ux=`echo $line | awk '{print $1;}'`
   tau=`echo $line | awk '{print $2;}'`
   ux2=`echo $line | awk '{print $3;}'`
   ngp=`echo $line | awk '{print $4;}'`
      
   dir=`date -d "1970-01-01 UTC $ux seconds" +"%Y_%m_%d_pulsars_msok"`

   if [[ -s $dir/analysis_final/TotalGoodTimeInSec.txt ]]; then   
      obstime=`cat $dir/analysis_final/TotalGoodTimeInSec.txt`
      ngp_corr=`echo $ngp" "$obstime | awk '{printf("%.6f\n",($1)/($2/3600.00));}'`
   else
      ngp_corr=$ngp
   fi
   
   echo "$tau $ngp_corr $ngp $obstime $ux $dir" 
   
done < $file