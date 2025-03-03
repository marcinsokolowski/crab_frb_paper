#!/bin/bash

template="/data_archive/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec/"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

local_dir=`pwd`

ssh aavs@nimbus4 "ls -d ${template}" > single_pulses_dir_list.txt

for remote_dir in `cat single_pulses_dir_list.txt`
do
   subdir=`echo $remote_dir | awk '{i=index($0,"20");print substr($0,i);}'`   
   
   if [[ -s ${subdir}/PULSE_ARFILES.txt ]]; then
      echo "INFO : data in ${subdir} already copied"
   else  
      echo "INFO : getting data from $remote_dir"
      
      mkdir -p ${subdir}
      
      cd ${subdir}
      pwd
      echo "rsync -avP aavs@nimbus4:${remote_dir}/PULSE_ARFILES.txt ."
      rsync -avP aavs@nimbus4:${remote_dir}/PULSE_ARFILES.txt .
      
      if [[ -s PULSE_ARFILES.txt ]]; then
         pulse_arfile=`cat PULSE_ARFILES.txt`
         pulse_arfile_base=${pulse_arfile%%.ar}
         
         echo "rsync -avP aavs@nimbus4:${remote_dir}/${pulse_arfile_base}* ."
         rsync -avP aavs@nimbus4:${remote_dir}/${pulse_arfile_base}* .
         
         echo "cp ~/github/crab_frb_paper/scripts/root/plot_psr_profile_nonorm.C ."
         cp ~/github/crab_frb_paper/scripts/root/plot_psr_profile_nonorm.C .
         
         vap -c "name nbin mjd freq period length bw" -n ${pulse_arfile}
         
         length_ms=`vap -c "name nbin mjd freq period length bw" -n ${pulse_arfile} | awk '{print $7*1000;}'`
         echo "Length = $length_ms [ms]"
         
         cat ${pulse_arfile_base}.txt | grep -v File | awk -v length_ms=${length_ms} '{print $3*(length_ms/1024)" "$4;}'  > ${pulse_arfile_base}_2col.txt
         
         echo "ssh aavs@nimbus4 \"ls -d ${remote_dir}/channel_0_1_*\""
         chdir=`ssh aavs@nimbus4 "ls -d ${remote_dir}/channel_0_1_*"`
         outline=`echo $chdir | awk '{i=index($0,"channel_0_1");print substr($1,i+12)" "180;}'`
         
         echo -n $outline > point.txt
                  
         echo "root -l plot_psr_profile_nonorm.C(\"${pulse_arfile_base}_2col.txt\")"
         root -l "plot_psr_profile_nonorm.C(\"${pulse_arfile_base}_2col.txt\")"
         
         cat point.txt >> ../../../../new_arfiles.txt	
      else
         echo "ERROR : file PULSE_ARFILES.txt not found -> dataset skipped ( $remote_dir )"
      fi
 
      cd ${local_dir}
   fi
done
