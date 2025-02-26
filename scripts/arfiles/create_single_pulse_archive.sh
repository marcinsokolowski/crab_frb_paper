#!/bin/bash

object=J0534+2200

mkdir single_pulse_archives_4004-4008/
cd single_pulse_archives_4004-4008/
psrcat -e ${object} > ${object}.eph

for dada in `ls ../*.dada`  
do
   outfile=${dada_file%%dada}_single_pulse.ar 
   dada_file=`basename $dada`
   
   dada_dir=${dada_file%%.dada}
   mkdir -p $dada_dir
   cd $dada_dir
   
   psrcat -e ${object} > ${object}.eph
   
   ln -s ../../${dada_file}

   # use -cuda 0 when there is GPU     
   # -S 3000 -T 2000 - time range 
   echo "dspsr -F1024:D -b1024 -E ${object}.eph -turns 1 -a PSRFITS -minram=256 -B 0.925925926 -S 4004 -T 4 ${dada_file}"
   dspsr -F1024:D -b1024 -E ${object}.eph -turns 1 -a PSRFITS -minram=256 -B 0.925925926 -S 4004 -T 4 ${dada_file} 
   cd ..
done


