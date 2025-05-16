#!/bin/bash

template="/media/msok/f5ce6064-9dc3-40c3-bc9c-7314e8594519/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec_merged4"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

root_options="-l -b -q"
if [[ -n "$2" && "$2" != "-" ]]; then
   root_options="$2"
fi

path=`pwd`

for dataset in `ls -d ${template}`
do
   cd ${dataset}

   ~/github/crab_frb_paper/scripts/arfiles/dm_from_gp_replot.sh `ls *.rf|tail -1` 0 "pulse" "${root_options}" - - - - NEW_FIT "-l"
   
   cd ${path}
done

