#!/bin/bash

pulse_time=31.68087552 
if [[ -n "$1" && "$1" != "-" ]]; then
   pulse_time=$1
fi

radius=1
if [[ -n "$2" && "$2" != "-" ]]; then
   radius=$2
fi

start=`echo $pulse_time" "$radius | awk '{print $1-$2;}'`
end=`echo $pulse_time" "$radius | awk '{print $1+$2;}'`

outfile=timeseries_${start}sec-${end}sec.txt

awk -v start=${start} -v end=${end} '{if($1>=start && $1<=end){print $0;}}' timeseries.txt > ${outfile}



