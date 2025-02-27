#!/bin/bash

object=J0534+2200

pulse_time=4006.013276
if [[ -n "$1" && "$1" != "-" ]]; then
   pulse_time=$1
fi

delta_time=0.033 # Crab period 
# delta_time=1.5 # dispersive delay of Crab at 230 - 200 MHz band is ~1.5 seconds : python ./dispersion2.py 56.7 231.25 200 0 0 2232 0.014 0.014 
if [[ -n "$2" && "$2" != "-" ]]; then
   delta_time=$2
fi

start_time=`echo $pulse_time" "$delta_time | awk '{print $1-$2;}'`
end_time=`echo $pulse_time" "$delta_time | awk '{print $1+$2;}'`
interval=`echo $delta_time | awk '{print 2*$1;}'`

outdir=single_pulse_archive_pulse_${pulse_time}sec_interval${interval}sec
if [[ -n "$3" && "$3" != "-" ]]; then
   outdir=$3
fi

mkdir ${outdir}/
cd ${outdir}/
psrcat -e ${object} > ${object}.eph

arcount=`ls -d channel_0_?_*/*.ar | wc -l`

if [[ $arcount -gt 0 && $force -le 0 ]]; then
   echo "INFO : $arcount .ar files already exist -> no need to re-create (unless force option used)"
else
   echo "INFO : creating single pulse archives:"

   for dada in `ls ../*.dada`  
   do
      # outfile=${dada_file%%dada}_single_pulse_${pulse_time}.ar 
      dada_file=`basename $dada`
   
      dada_dir=${dada_file%%.dada}
      mkdir -p $dada_dir
      cd $dada_dir
   
      psrcat -e ${object} > ${object}.eph
   
      ln -s ../../${dada_file}

      # use -cuda 0 when there is GPU     
      # -S 3000 -T 2000 - time range 
      echo "dspsr -F1024:D -b1024 -E ${object}.eph -turns 1 -a PSRFITS -minram=256 -B 0.925925926 -S ${start_time} -T ${interval} ${dada_file}"
      dspsr -F1024:D -b1024 -E ${object}.eph -turns 1 -a PSRFITS -minram=256 -B 0.925925926 -S ${start_time} -T ${interval} ${dada_file} 
   
      cd ..
    done
fi

# sum only last 5 from channel 0 this is where the pulse will seat :
for first_arfile in `ls channel_0_?_*/*.ar | tail -5`
do
   base_arfile=`basename $first_arfile`
   
   arlist=`ls channel_?_?_*/${base_arfile} | awk '{printf("%s ",$1);}END{printf("\n");}'`
   arlist2=`ls channel_??_?_*/${base_arfile} | awk '{printf("%s ",$1);}END{printf("\n");}'`
   
   arcount=`ls ${arlist} ${arlist2} | wc -l`
   
   if [[ $arcount -ge 40 ]]; then
      outfile=${base_arfile%%.ar}_sum.ar
      echo "psradd -E J0534+2200.eph -R -o ${outfile} ${arlist} ${arlist2}"
      psradd -E J0534+2200.eph -R -o ${outfile} ${arlist} ${arlist2}
   else
      echo "Non-full band pulse $base_arfile skipped"
   fi
done
