#!/bin/bash

start=0
if [[ -n "$1" && "$1" != "-" ]]; then
   start=$1
fi

end=0
if [[ -n "$2" && "$2" != "-" ]]; then
   end=$2
fi

outfile=timeseries_${start}sec-${end}sec.txt

awk -v start=${start} -v end=${end} '{if($1>=start && $1<=end){print $0;}}' timeseries.txt > ${outfile}



