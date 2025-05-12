#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec/*.ar"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

dodedisp=1
if [[ -n "$2" && "$2" != "-" ]]; then
   dodedisp=$2
fi

func_name="leading_edge"
if [[ -n "$3" && "$3" != "-" ]]; then
   func_name="$3"
fi

root_options="-l"
if [[ -n "$4" && "$4" != "-" ]]; then
   root_options="$4"
fi

force=0
if [[ -n "$5" && "$5" != "-" ]]; then
   force=$5
fi


curr_path=`pwd`

for arfile_path in `ls ${template}`
do
   path=`dirname $arfile_path`
   cd $path
   echo
   echo
   pwd
   
   arfile=`ls *.ar | tail -1`
   backup_file=${arfile}.backup
   rffile=${arfile%%ar}rf # PSR FITS 

   if [[ -s $backup_file ]]; then   
      echo "Backup file $backup_file already exists -> no need to recreate"
   else
      echo "cp $arfile $backup_file"
      cp $arfile $backup_file
   fi
   
   if [[ -s $rffile ]]; then
      echo "PSRFITS file $rffile already exists -> no need to recreate"
   else
      echo "psrconv -o PSRFITS ${arfile}"
      psrconv -o PSRFITS ${arfile}   
   fi
   
   if [[ -s DM_SLOPE_leading_edge.txt && $force -le 0 ]]; then
       echo "ALREADY PROCESSED leading_edge : $path"
   else
       echo "~/github/crab_frb_paper/scripts/arfiles/dm_from_gp.sh $rffile $dodedisp leading_edge \"${root_options}\""
       ~/github/crab_frb_paper/scripts/arfiles/dm_from_gp.sh $rffile $dodedisp "leading_edge" "${root_options}"
   fi   
   
   if [[ -s DM_SLOPE_pulse.txt && $force -le 0 ]]; then
       echo "ALREADY PROCESSED pulse : $path"
   else
       echo "~/github/crab_frb_paper/scripts/arfiles/dm_from_gp.sh $rffile 0 pulse \"${root_options}\""
       ~/github/crab_frb_paper/scripts/arfiles/dm_from_gp.sh $rffile 0 "pulse" "${root_options}"
   fi   
   
   cd $curr_path
   sleep 5
done
