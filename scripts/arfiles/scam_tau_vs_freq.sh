#!/bin/bash

template="/media/msok/f5ce6064-9dc3-40c3-bc9c-7314e8594519/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec/"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

model="iso"
outdir="scamp"
if [[ -n "$2" && "$2" != "-" ]]; then
   model="$2"
   
   # only overwritten when model is provided otherwise default outdir is scamp to be consistent
   outdir="scamp_${model}"
fi

# DEFULAT is above : only overwritten when model is provided otherwise default outdir is scamp to be consistent
if [[ -n "$3" && "$3" != "-" ]]; then
   outdir="$3"
fi


path=`pwd`

for dataset in `ls -d $template`
do
   cd $dataset
   if [[ -d ${outdir} ]]; then
      echo "Dataset $dataset already processed -> skipped. Remove directory ${outdir} to reprocessed otherwise there will be ERRORS"
   else   
      mkdir -p ${outdir}
      cd ${outdir}
      pwd
   
      echo "cp ../*.ar ."
      cp ../*.ar .
   
      arfile=`ls *.ar | tail -1`
      ln -s /home/msok/github/psrchive/More/python/.libs/_psrchive.so
   
      echo "python ~/github/SCAMP_I/create_config_ascii_from_archive.py -f $arfile -d ./"
      python ~/github/SCAMP_I/create_config_ascii_from_archive.py -f $arfile -d ./
   
      ascii_file=${arfile}.4ch.ascii
      csv_file=J0534+2200_config_4ch.csv
   
      echo "~/github/SCAMP_I/run.sh $ascii_file $csv_file $model"
      ~/github/SCAMP_I/run.sh $ascii_file $csv_file $model
   
   fi

   read -p "Do you want continue with next dataset : [y/n] " answer
   if [[ $answer == "y" || $answer == "Y" ]]; then
      echo "Continuing to next dataset"
   else
      echo "Stopping script now"
      exit
   fi
   
   cd $path
done
