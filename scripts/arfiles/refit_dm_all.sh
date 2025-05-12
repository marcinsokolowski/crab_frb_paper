#!/bin/bash

template="202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec/*.ar"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

path=`pwd`

for arfile_path in `ls -d ${template}`
do
   echo
   echo
   
   dir=`dirname $arfile_path`
   cd $dir
   pwd
   read -p "Do you want to re-fit for these data  : [y/n] " answer

   if [[ $answer == "y" || $answer == "Y" ]]; then
      echo "~/github/crab_frb_paper/scripts/arfiles/refit_dm.sh"
      ~/github/crab_frb_paper/scripts/arfiles/refit_dm.sh 
   else
      echo "Fit is not required"
   fi
   cd $path
done
