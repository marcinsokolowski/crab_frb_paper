#!/bin/bash

arfile=pulse_31425593048_sum.rf
if [[ -n "$1" && "$1" != "-" ]]; then
   arfile=$1     
fi

echo "cp ~/github/crab_frb_paper/scripts/root/fit_leading_edge.C ."
cp ~/github/crab_frb_paper/scripts/root/fit_leading_edge.C .

echo "cp ~/github/crab_frb_paper/scripts/root/plotslope.C ."
cp ~/github/crab_frb_paper/scripts/root/plotslope.C .

mkdir -p images/

for dm in `echo 56.70 56.702 56.704 56.706 56.708 56.710 56.712 56.714 56.716 56.718 56.720 56.722 56.724 56.726 56.728 56.730 56.732 56.734 56.736 56.738 56.740 56.742 56.744 56.746 56.748 56.750 56.76 56.77 56.78 56.79 56.80`
do
   
   dmfile=${arfile%%rf}dm${dm}
   txtfile=${dmfile}.txt
   psrfile=${txtfile%%txt}psr
   
   echo "pam ${arfile} -d ${dm} -e "dm${dm}""
   pam ${arfile} -d ${dm} -e "dm${dm}"
   
   pdv -FTtp ${dmfile} | awk '{if(NF==4){print $0;}}' > ${txtfile}
   
   awk '{print $3*(0.0333924123/1024.00)" "$4;}' ${txtfile} > ${psrfile}
   
   root -l "fit_leading_edge.C(\"${psrfile}\",${dm})" 

#   rm -f ${dmfile}   
   echo "rm -f ${dmfile}"   
   sleep 1
done

# cat pulse*dm??.psr*.fit | awk '{print NR" "$7/($5-$3);}'  > slope_vs_index.txt
