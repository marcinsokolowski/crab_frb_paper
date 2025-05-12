#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec"
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

curr_path=`pwd`

for path in `ls -d ${template}`
do
   cd $path
   
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
   
   echo "~/github/crab_frb_paper/scripts/arfiles/dm_from_gp.sh $rffile $dodedisp $func_name \"${root_options}\""
   ~/github/crab_frb_paper/scripts/arfiles/dm_from_gp.sh $rffile $dodedisp $func_name "${root_options}"

exit;
   
   cd $curr_path
done
