#!/bin/bash

arfile=pulse_31425593048_sum.rf
if [[ -n "$1" && "$1" != "-" ]]; then
   arfile=$1     
fi
b=${arfile%%.rf}

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


echo "cp ~/github/crab_frb_paper/scripts/root/fit_leading_edge.C ."
cp ~/github/crab_frb_paper/scripts/root/fit_leading_edge.C .

echo "cp ~/github/crab_frb_paper/scripts/root/plotslope_err.C ."
cp ~/github/crab_frb_paper/scripts/root/plotslope_err.C .

echo "cp ~/github/crab_frb_paper/scripts/root/plotrisetime_err.C ."
cp ~/github/crab_frb_paper/scripts/root/plotrisetime_err.C .


mkdir -p images/

for dm in `echo 56.700 56.702 56.704 56.706 56.708 56.710 56.712 56.714 56.716 56.718 56.720 56.722 56.724 56.726 56.728 56.730 56.732 56.734 56.736 56.738 56.740 56.742 56.744 56.746 56.748 56.750 56.760 56.770 56.780 56.790 56.800`
do
   
   dmfile=${arfile%%rf}dm${dm}
   txtfile=${dmfile}.txt
   psrfile=${txtfile%%txt}psr

   if [[ $dodedisp -gt 0 ]]; then   
      echo "pam ${arfile} -d ${dm} -e "dm${dm}""
      pam ${arfile} -d ${dm} -e "dm${dm}"
   
      pdv -FTtp ${dmfile} | awk '{if(NF==4){print $0;}}' > ${txtfile}   
   else
      echo "Dedisp skipped"
   fi
   
   awk '{print $3*(0.0333924123/1024.00)" "$4;}' ${txtfile} > ${psrfile}
   root ${root_options} "fit_leading_edge.C(\"${psrfile}\",${dm},\"${func_name}\")" 

#   rm -f ${dmfile}   
   echo "rm -f ${dmfile}"   
#   sleep 1
done

# cat pulse*dm??.psr*.fit | awk '{print NR" "$7/($5-$3);}'  > slope_vs_index.txt
if [[ $func_name == "leading_edge" ]]; then
   cat ${b}.dm*.psr.fit > slope_vs_dm_leading_edge.txt
   root -l "plotslope_err.C(\"slope_vs_dm_leading_edge.txt\",\"poly2\",56.7,56.74,\"leading_edge\")"
else 
   cat pulse*dm*.psr*.fit | awk '{print $1" 0 "$9/($7-$5)" 0 ";}' |sort -n  > slope_vs_index_pulse.txt
   cat pulse*dm*.psr*.fit | awk '{print $1" 0 "$7-$5" 0";}' |sort -n > risetime_vs_index.txt

   root -l "plotslope_err.C(\"slope_vs_index_pulse.txt\",\"poly2\",56.7,56.74,\"pulse\")"
   root -l "plotrisetime_err.C(\"risetime_vs_index.txt\",\"poly2\",56.7,56.74)"
fi
