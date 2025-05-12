#!/bin/bash

template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/single_pulse_archive_pulse_*sec_interval*sec"

for dir in `ls -d ${template}`
do
   echo 
   echo
   echo "Copying $dir to "
   new_dir=`echo $dir | awk '{i=index($1,"eda2");print substr($1,i+5);}'`   
   echo $new_dir
   
   mkdir -p $new_dir
   
   echo "cp ${dir}/*.ar ${new_dir}/"
   cp ${dir}/*.ar ${new_dir}/   
done
