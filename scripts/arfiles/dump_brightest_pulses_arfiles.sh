#!/bin/bash

do_scp=0
if [[ -n "$1" && "$1" != "-" ]]; then
   do_scp=$1
fi

# was presto.txt in merged/
presto_pulse_file=_DM56.72.singlepulse

export PATH=~/github/crab_frb_paper/scripts/arfiles:$PATH

for merged_dir in `ls -d /media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/20??_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms100_dmstep0.01/`
do
   uxtime=`echo $merged_dir | awk '{i=index($1,"merged_channels");print substr($1,i+16,10);}'`

   echo
   echo
   if [[ -d $merged_dir ]]; then
      cd $merged_dir
      pwd
      if [[ -s ${presto_pulse_file} ]]; then
         max_line=`awk -v max_snr=-1 '{if($1!="#" && $2>max_snr){max_snr=$2;line=$0;}}END{print line;}' $presto_pulse_file`      
         echo $max_line > MAX_SNR_LINE.txt

         pwd
         final_threshold=50
         echo "~/github/crab_frb_paper/scripts/arfiles/find_bright_presto_pulses.sh ${presto_pulse_file} 50 > MAX_SNR_LINES.txt"      
         ~/github/crab_frb_paper/scripts/arfiles/find_bright_presto_pulses.sh ${presto_pulse_file} 50 > MAX_SNR_LINES.txt      
         
         count=`cat MAX_SNR_LINES.txt | wc -l`
         if [[ $count -le 1 ]]; then
            echo "WARNING : no pulses above SNR = 50 -> dumping SNR >= 30 pulses :"
            echo "~/github/crab_frb_paper/scripts/arfiles/find_bright_presto_pulses.sh ${presto_pulse_file} 30 > MAX_SNR_LINES.txt"      
            ~/github/crab_frb_paper/scripts/arfiles/find_bright_presto_pulses.sh ${presto_pulse_file} 30 > MAX_SNR_LINES.txt                                                   
            final_threshold=30
            
            count=`cat MAX_SNR_LINES.txt | wc -l`
         fi                  
         
         echo "cat MAX_SNR_LINES.txt (SNR >= $final_threshold ) :"
         cat MAX_SNR_LINES.txt
         
         echo "cat MAX_SNR_LINE.txt"
         cat MAX_SNR_LINE.txt
                  
      
         # copy to nimbus4 :
         subdir=`echo $merged_dir | awk '{i=index($0,"20");print substr($0,i,59);}'`
      
         if [[ $do_scp -gt 0 ]]; then
            echo "rsync -avP MAX_SNR_LINE*txt aavs@nimbus4:/data_archive/${subdir}/"
            rsync -avP MAX_SNR_LINE*txt aavs@nimbus4:/data_archive/${subdir}/
         else
            echo "INFO : copying to nimubs4 is not required"
         fi
      
      else
         echo "ERROR : file ${presto_pulse_file} does not exist"
      fi
   
      cd -
   else
      echo "WARNING : directory $merged_dir does not exist"
   fi
done


