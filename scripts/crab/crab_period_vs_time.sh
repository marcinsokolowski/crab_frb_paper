#!/bin/bash

infile=crab2.txt
if [[ ! -s $infile ]]; then
   echo "wget https://www.jb.man.ac.uk/pulsar/crab/crab2.txt"
   wget https://www.jb.man.ac.uk/pulsar/crab/crab2.txt
fi

rm -f fu_vs_time.txt period_vs_time.txt

line_counter=0
while read line 
do
   if [[ $line_counter -gt 10 ]]; then
      nf=`echo $line | awk '{print NF;}'`
      
      if [[ $nf -gt 10 ]]; then
         echo "Processing line : $line"            
         mjd=`echo $line | awk '{printf("%d\n",$4);}'`
         period_err=0
         
         if [[ $mjd -gt 55889 ]]; then
            fu=`echo $line | awk '{printf("%.20f\n",$7);}'`
            fu_sigma=`echo $line | awk '{printf("%.20f\n",$8/1e15);}'`
            period=`echo $line | awk '{printf("%.20f\n",1.00/$7);}'`
         else
            fu=`echo $line | awk '{printf("%.20f\n",$8);}'`
            fu_sigma=`echo $line | awk '{printf("%.20f\n",$9/1e15);}'`
            period=`echo $line | awk '{printf("%.20f\n",1.00/$8);}'`
         fi
      
         ux=`python ~/github/crab_frb_paper/scripts/python/mjd2ux.py ${mjd}`
         echo "$ux 43200 $fu $fu_sigma" >> fu_vs_time.txt
         echo "$ux 43200 $period $period_err" >> period_vs_time.txt
       fi
   else 
      echo "First 10 lines skipped : $line"
   fi
   
   line_counter=$(($line_counter+1))
done < ${infile}


tail --lines=100 fu_vs_time.txt > fu_vs_time_last.txt
cp ~/github/crab_frb_paper/scripts/root/plot_fu_vs_time_error.C .
root -l "plot_fu_vs_time_error.C(\"fu_vs_time_last.txt\")"
