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

dtm=`date +%y%m%d%H%M`
outdir="PeakFit_${dtm}"
if [[ -n "$6" && "$6" != "-" ]]; then
   outdir="$6"
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

#      if [[ -s DM_SLOPE_leading_edge.txt && $force -le 0 ]]; then
#          echo "ALREADY PROCESSED leading_edge : $path"
#      else
#          echo "~/github/crab_frb_paper/scripts/arfiles/dm_from_gp_replot.sh $rffile $dodedisp leading_edge \"${root_options}\" - ${force}"
#          ~/github/crab_frb_paper/scripts/arfiles/dm_from_gp_replot.sh $rffile $dodedisp "leading_edge" "${root_options}" - ${force}
#      fi   
   
   echo "~/github/crab_frb_paper/scripts/arfiles/dm_from_gp_replot.sh $rffile 0 pulse \"${root_options}\" - ${force} 0 1 ${outdir}"
   ~/github/crab_frb_paper/scripts/arfiles/dm_from_gp_replot.sh $rffile 0 "pulse" "${root_options}" - ${force} 0 1 ${outdir}
   
   cd $curr_path
   
   sleep 5
done
