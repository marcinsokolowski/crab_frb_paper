#!/bin/bash

rm -f dm_vs_ux.txt

for dt in `ls -d 202?_??_??`
do
   cd $dt
   rm -f dms.txt   
   for bestfile in `ls *.best`
   do
      line=`tail -3 $bestfile | head -1`      
      dm=`echo $line | awk '{print $1;}'`
      dm_err=`echo $line | awk '{print $3;}'`
   
      echo "$dm $dm_err" >> dms.txt
   done
   
   dm_mean=`cat dms.txt | awk -v sum=0 -v cnt=0 '{sum+=$1;cnt+=1;}END{print sum/cnt;}'`
   dm_err=`cat dms.txt | awk -v sum=0 -v cnt=0 '{sum+=($2*$2);cnt+=1;}END{print sqrt(sum)/cnt;}'`
   
   year=`echo $dt | cut -b 1-4`
   mon=`echo $dt | cut -b 6-7`
   day=`echo $dt | cut -b 9-11`
   
   dtm=${year}${mon}${day}_220000
   echo "date2date -local2ux=${dtm}"
   ux=`date2date -local2ux=${dtm}`
   cd ..
   
   echo "$ux 43200 $dm_mean $dm_err" >> dm_vs_ux.txt
done

cp ~/github/crab_frb_paper/scripts/root/plot_dm_vs_time.C .
root -l "plot_dm_vs_time.C(\"dm_vs_ux.txt\")"

