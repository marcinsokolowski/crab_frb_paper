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

pdv_options="" # add -C to change phase of the peak (move it around - not sure how ...)
if [[ -n "$5" && "$5" != "-" ]]; then
   pdv_options="$5"
fi

force=0
if [[ -n "$6" && "$6" != "-" ]]; then
   force="$6"
fi

unwrap=0
n_unwrap=500
if [[ -n "$7" && "$7" != "-" ]]; then
   unwrap=$7
   
   if [[ $unwrap -gt 1 ]]; then
      n_unwrap=$unwrap
   fi
fi

use_prev_fit=1
if [[ -n "$8" && "$8" != "-" ]]; then
   use_prev_fit=$8
fi

dtm=`date +%y%m%d%H%M`
outdir="PeakFit_${dtm}"
if [[ -n "$9" && "$9" != "-" ]]; then
   outdir="$9"
fi

root_options2="${root_options}"
if [[ -n "${10}" && "${10}" != "-" ]]; then
   root_options2="${10}"
fi


echo "#######################################"
echo "PARAMETERS:"
echo "#######################################"
echo "outdir = $outdir"
echo "#######################################"


slope_version=0

echo "cp ~/github/crab_frb_paper/scripts/root/fit_leading_edge*.C ."
cp ~/github/crab_frb_paper/scripts/root/fit_leading_edge*.C .

echo "cp ~/github/crab_frb_paper/scripts/root/plotslope_err.C ."
cp ~/github/crab_frb_paper/scripts/root/plotslope_err.C .

echo "cp ~/github/crab_frb_paper/scripts/root/plotrisetime_err.C ."
cp ~/github/crab_frb_paper/scripts/root/plotrisetime_err.C .

echo "cp ~/github/crab_frb_paper/scripts/root/plotpeak_err.C ."
cp ~/github/crab_frb_paper/scripts/root/plotpeak_err.C .

echo "cp ~/github/crab_frb_paper/scripts/root/plotrise_err.C ."
cp ~/github/crab_frb_paper/scripts/root/plotrise_err.C .

mkdir -p images/

dtm=`date +%y%m%d%H%M%S`
mv last.fit last.fit.${dtm}
for dm in `echo 56.720 56.722 56.724 56.726 56.728 56.730 56.732 56.734 56.736 56.738 56.740 56.742 56.744 56.746 56.748 56.750 56.760 56.770 56.780 56.790 56.800 56.830 56.850 56.880 56.900`
do   
   dmfile=${arfile%%rf}dm${dm}
   txtfile=${dmfile}.txt
   psrfile=${txtfile%%txt}psr

   if [[ $use_prev_fit -gt 0 ]]; then
      echo "Using last.fit"
   else
      echo "Not using last fit : rm -f last.fit"
      rm -f last.fit   
   fi

   # or fit_leading_edge_slope.C
   if [[ $slope_version -gt 0 ]]; then
      root ${root_options} "fit_leading_edge_slope.C+(\"${psrfile}\",${dm},\"${func_name}\")"
   else
      root ${root_options} "fit_leading_edge.C+(\"${psrfile}\",${dm},\"${func_name}\")" 
   fi

#   rm -f ${dmfile}   
   echo "rm -f ${dmfile}"   
#   sleep 1
done

dtm=`date +%y%m%d%H%M%S`
mv last.fit last.fit.${dtm}
for dm in `echo 56.720 56.718 56.716 56.714 56.712 56.710 56.708 56.706 56.704 56.702 56.700 56.690 56.680 56.670 56.650 56.640 56.630 56.600`
do   
   dmfile=${arfile%%rf}dm${dm}
   txtfile=${dmfile}.txt
   psrfile=${txtfile%%txt}psr

   # or fit_leading_edge_slope.C   
   if [[ $slope_version -gt 0 ]]; then
      # fitting with slope is less stable in measuring RISE TIME :
      root ${root_options} "fit_leading_edge_slope.C+(\"${psrfile}\",${dm},\"${func_name}\")"
   else
      root ${root_options} "fit_leading_edge.C+(\"${psrfile}\",${dm},\"${func_name}\")" 
   fi

#   rm -f ${dmfile}   
   echo "rm -f ${dmfile}"   
#   sleep 1
done



# cat pulse*dm??.psr*.fit | awk '{print NR" "$7/($5-$3);}'  > slope_vs_index.txt
if [[ $func_name == "leading_edge" ]]; then
   cat ${b}.dm*.psr.fit > slope_vs_dm_leading_edge.txt
   root ${root_options} "plotslope_err.C(\"slope_vs_dm_leading_edge.txt\",\"poly2\",56.62,56.8,\"leading_edge\")"
else 
# normal _ts version :

   if [[ $slope_version -gt 0 ]]; then
      # WARNING : slope version is less stable in measuring RISE TIME !!!
      cat pulse*dm*.psr*.fit | awk '{print $1" 0 "$5" "$6;}' > slope_vs_index_pulse.txt 
      cat pulse*dm*.psr*.fit | awk '{p=$9;dp=$10;s=$5;ds=$6;err=(p/s)*sqrt((dp/p)*(dp/p)+(ds/s)*(ds/s));print $1" 0 "dt" "err;}' |sort -n > risetime_vs_index.txt
   else
      cat pulse*dm*.psr*.fit | awk '{err_down=sqrt($6*$6+$8*$8);err_up=$10;up=$9;down=($7-$5);slope=up/down;err=slope*sqrt((err_up/up)*(err_up/up)+(err_down/down)*(err_down/down));print $1" 0 "slope" "err;}' |sort -n  > slope_vs_index_pulse.txt
      cat pulse*dm*.psr*.fit | awk '{err=sqrt($6*$6+$8*$8);print $1" 0 "$7-$5" "err;}' |sort -n > risetime_vs_index.txt      
      cat pulse*dm*.psr*.rise > rise_vs_dm.txt
      cat pulse*dm*.psr*.peak > peak_vs_dm.txt
   fi
   
   mkdir -p ${outdir}
   
   echo "cp UNIXTIME.txt ${outdir}/"
   cp UNIXTIME.txt ${outdir}/   
 
   # was 56.62,56.8,
   root ${root_options2} "plotslope_err.C(\"slope_vs_index_pulse.txt\",\"poly2\",56.66,56.75,\"pulse\",\"${outdir}\")"

   root ${root_options2} "plotrise_err.C(\"rise_vs_dm.txt\",\"poly2\",56.66,56.75,\"pulse\",\"${outdir}\")"
   
   root ${root_options2} "plotpeak_err.C(\"peak_vs_dm.txt\",\"poly2\",56.66,56.8,\"pulse\",\"${outdir}\")"
   
   # was 56.59,56.8
   root ${root_options2} "plotrisetime_err.C(\"risetime_vs_index.txt\",\"poly2\",56.66,56.75,\"${outdir}\")"
fi
