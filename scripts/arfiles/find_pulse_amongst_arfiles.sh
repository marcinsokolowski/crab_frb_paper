#!/bin/bash

threshold=0.001
if [[ -n "$1" && "$1" != "-" ]]; then
   threshold=$1
fi

for arfile in `ls -r *.ar`
do
   txtfile=${arfile%%ar}txt
   
   echo
   if [[ -s ${txtfile} ]]; then
      echo "File ${txtfile} exists -> skipped"
   else
      echo "pdv -FTtp ${arfile} > ${txtfile}"
      pdv -FTtp ${arfile} > ${txtfile}
   fi
   
   max_value=`awk -v max=-1 '{if($1!="File:" && $4>max){max_line=$0;max=$4;}}END{print max_line}' ${txtfile}`   
   is_gp=`echo $max_value | awk -v t=${threshold} '{if($4>t){print 1;}else{print 0;}}'`
   
   echo "Max value = $max_value vs. threshold = $threshold -> is_gp = $is_gp"   
   if [[ $is_gp -gt 0 ]]; then
      echo "GP is in file $arfile"
      
      echo "ls $arfile > PULSE_ARFILES.txt"
      ls $arfile > PULSE_ARFILES.txt
      exit;
   fi
   
done
