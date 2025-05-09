#!/bin/bash

tau_files="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged/pulses_snr5_calibrated/tau.txt"

merge_n=2

outdir=~/Desktop/SKA/papers/2024/EDA2_FRBs/PAPER/TAU_MERGED
cd ${outdir}
cp ~/github/crab_frb_paper/scripts/root/histotau.C .

echo "mv taufit.txt taufit.txt.old"
mv taufit.txt taufit.txt.old

rm -f curr.txt ux.txt 
prev_ux=-1
merged=0
for tau_file in `ls ${tau_files}`
do
  echo  
  dir=`dirname ${tau_file}`
  dataset=`echo $dir |  awk '{i=index($1,"/eda2/2");print substr($1,i+6,23);}'`
  echo "Processing $dataset ( $dir )"
  uxtime=`cat ${dir}/taufit.txt | awk '{if($1!="#"){print $7;}}'`
  diff=`echo $uxtime" "$prev_ux | awk '{printf("%d\n",($1-$2));}'`
  
  echo "DEBUG : diff = $diff"

  finish=0
  if [[ $merged -le 0 ]]; then
     echo "cat ${tau_file} > curr.txt"
     cat ${tau_file} > curr.txt

     pwd
     echo "echo $uxtime > ux.txt"
     echo $uxtime > ux.txt
     merged=$(($merged+1))
  else
     if [[ $diff -lt 86400 ]]; then
        echo "IF ???"
        echo "cat ${tau_file} >> curr.txt"
        cat ${tau_file} >> curr.txt   
        
        pwd
        echo "echo $uxtime >> ux.txt"
        echo $uxtime >> ux.txt
        pwd
        merged=$(($merged+1))
     else 
        # too long break -> fit now and start new average :
        finish=1
     fi     
  fi

  if [[ $merged -ge $merge_n || $finish -gt 0 ]]; then
     pwd
     unixtime=`cat ux.txt | awk -v sum=0 -v count=0 '{sum+=$1;count+=1;}END{printf("%.8f\n",sum/count);}'`
     echo "Number of merged = $merge_n reached ($merged) -> fitting Gaussian now (unixtime = $unixtime)"     
     root -l "histotau.C(\"curr.txt\",0,1,0,0.08,100,0,\"Scattering Time [sec]\", \"Number of pulses fitted\", 0, \"${dataset}\", NULL, \"_histo\", $unixtime, \"${dataset}\" )"
     # histogram/fit 
     echo
     echo
     if [[ $merged -ge $merge_n ]]; then
        echo "rm -f curr.txt ux.txt"
        rm -f curr.txt ux.txt
        merged=0
        
        sleep 5
     else
        # new dataset was after too long break and starts new average:
        echo "cat ${tau_file} > curr.txt"
        cat ${tau_file} > curr.txt

        echo "echo $uxtime > ux.txt"
        echo $uxtime > ux.txt        
        
        merged=1
     fi
#     exit
  fi
  prev_ux=${uxtime}
done
