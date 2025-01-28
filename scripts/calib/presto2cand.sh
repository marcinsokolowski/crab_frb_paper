#!/bin/bash

# template="_DM*.singlepulse"
template="_DM5?.00.singlepulse"
if [[ -n "$1" && "$1" != "-" ]]; then
   template="$1"
fi

tmp_file1="presto.cand_unsort"
tmp_file="presto.cand"

echo "mv ${tmp_file} ${tmp_file}.old"
mv ${tmp_file} ${tmp_file}.old

echo "rm -f ${tmp_file1}"
rm -f ${tmp_file1}

# # DM      Sigma      Time (s)     Sample    Downfact
#  57.00    5.83      4.869366       5032       9
for dm_file in `ls ${template}`
do
   cat ${dm_file} | awk '{if($1!="#"){print $2" "$4" "$3" 0 0 "$1" 0 -1 -1 -1 -1 -1";}}' >> $tmp_file1
done

# 10.02 58103 56.2251 5 1444 81.98 0 60701.537515777 894485.5980 3812.9171 910674.5244 894378.4712
echo "# S/N, sampno, secs from file start, boxcar, idt, dm, beamno, mjd MEDIAN_TOTAL_POWER RMSIQR_TOTAL_POWER MAX_TOTAL_POWER" > $tmp_file
sort +2 -n ${tmp_file1} >> ${tmp_file}

# 
# path=`which my_friends_of_friends.py`
path="/home/msok/github/crab_frb_paper/scripts/calib/my_friends_of_friends_PRESTO.py"
outfile=${tmp_file%%cand}cand_merged
echo "python $path ${tmp_file} --outfile=${outfile}"
python $path ${tmp_file} --outfile=${outfile}

