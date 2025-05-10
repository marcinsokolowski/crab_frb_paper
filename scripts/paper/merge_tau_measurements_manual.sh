#!/bin/bash

cd ~/Desktop/SKA/papers/2024/EDA2_FRBs/PAPER/TAU_MERGED

tau_list=all_tau_files.txt
merged=0
dataset="unknown"

echo "mv taufit.txt taufit.txt.old"
mv taufit.txt taufit.txt.old

echo "rm -f curr.txt ux.txt"
rm -f curr.txt ux.txt

mkdir -p images/
cp ~/github/crab_frb_paper/scripts/root/histotau.C .

while read line
do
   tau_file=${line}
   echo 
   echo "Processing tau_file = $tau_file"
   n_items=`echo ${tau_file} | awk '{print NF;}'`
   
   echo "DEBUG : n_items = $n_items, merged = $merged"
   
   if [[ $n_items -le 0 ]]; then
      if [[ $merged -gt 0 ]]; then
         unixtime=`cat ux.txt | awk -v sum=0 -v count=0 '{sum+=$1;count+=1;}END{printf("%.8f\n",sum/count);}'`
         echo "Number of merged = $merge_n reached ($merged) -> fitting Gaussian now (unixtime = $unixtime)"
         echo "Histograming dataset $dataset curr.txt :"
         cat curr.txt
         echo "-----------------------------------"
         
         echo "DEBUG : keeping merged tau-s in file tau_ux${unixtime}.txt"
         echo "cp curr.txt tau_ux${unixtime}_${dataset}.txt"
         cp curr.txt tau_ux${unixtime}_${dataset}.txt
         
         echo "cp ux.txt ux${unixtime}_${dataset}.txt"
         cp ux.txt ux${unixtime}_${dataset}.txt
  
         root -l -q -b "histotau.C(\"tau_ux${unixtime}_${dataset}.txt\",0,1,0,0.08,100,0,\"Scattering Time [sec]\", \"Number of pulses fitted\", 0, \"${dataset}\", NULL, \"_histo\", $unixtime, \"${dataset}\" )"
         echo "-----------------------------------"

      fi
   
      echo "rm -f curr.txt ux.txt"
      rm -f curr.txt ux.txt
   else
      echo  
      echo "DEBUG : here ???"
      dir=`dirname ${tau_file}`
      dataset=`echo $dir |  awk '{i=index($1,"/eda2/2");print substr($1,i+6,23);}'`
      echo "Processing $dataset ( $dir )"
      uxtime=`cat ${dir}/taufit.txt | awk '{if($1!="#"){print $7;}}'`
      
      echo "cat ${tau_file} >> curr.txt"
      cat ${tau_file} >> curr.txt   

      pwd
      echo "echo $uxtime >> ux.txt"
      echo $uxtime >> ux.txt

      merged=$(($merged+1))
   fi
   
#   sleep 5
done < ${tau_list}


cp ~/github/crab_frb_paper/scripts/root/plot_scattau_vs_time.C .
awk '{if($1!="#"){print $7" 43200 "$3*1000.0" "$2*1000.0;}}' taufit.txt > fit_tau_vs_time.txt
awk '{if($1!="#"){print $7" 43200 "$5*1000.0" "$6*1000.0;}}' taufit.txt > mean_tau_vs_time.txt

root -l "plot_scattau_vs_time.C(\"taumean_vs_time.txt\",\"fit_tau_vs_time.txt\")"
root -l "plot_scattau_vs_time.C(\"taumean_vs_time.txt\",\"mean_tau_vs_time.txt\")"

