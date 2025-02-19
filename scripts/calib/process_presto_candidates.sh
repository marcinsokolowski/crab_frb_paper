#!/bin/bash

datfile=_DM57.00.dat
if [[ -n "$1" && "$1" != "-" ]]; then
   datfile="$1"
fi

options=""
if [[ -n "$2" && "$2" != "-" ]]; then
   options="$2"
fi

root_options="-l"

b=${datfile%%.dat}

# original de-dispersed timeseries:
timeseries_file=timeseries_${b}.txt
echo "~/github/presto_tools/build/presto_data_reader $datfile -o ${timeseries_file} ${options}"
~/github/presto_tools/build/presto_data_reader $datfile -o ${timeseries_file} ${options}

# running median, de-trending and normalisation by RMS_IQR :
running_median_file=running_median_${b}.txt
rmqiqr_file=rmsiqr_${b}.txt
detrendnorm_file=detrended_normalised_${b}.txt

echo "~/github/presto_tools/build/presto_data_reader $datfile -o ${detrendnorm_file} -r 100 -R ${running_median_file} -I ${rmqiqr_file} ${options}"
~/github/presto_tools/build/presto_data_reader $datfile -o ${detrendnorm_file} -r 100 -R ${running_median_file} -I ${rmqiqr_file} ${options}

awk '{print $1" "$3+$4;}' ${detrendnorm_file} > up.txt
awk '{print $1" "$3-$4;}' ${detrendnorm_file} > down.txt

echo "cp ~/github/crab_frb_paper/scripts/root/plot_timeseries_list.C ."
cp ~/github/crab_frb_paper/scripts/root/plot_timeseries_list.C .

echo ${timeseries_file} > list
echo ${running_median_file} >> list
echo up.txt >> list
echo down.txt >> list

mkdir -p images/
root ${root_options} "plot_timeseries_list.C+(\"list\")"



