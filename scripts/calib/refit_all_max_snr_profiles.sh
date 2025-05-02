#!/bin/bash


maxsnr_template="/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged/pulses_snr*.??_calibrated/"
if [[ -n "$1" && "$1" != "-" ]]; then
   maxsnr_template="$1"
fi

snr_threshold=20
if [[ -n "$2" && "$2" != "-" ]]; then
   snr_threshold=$2
fi

chi2_max=1.5
if [[ -n "$3" && "$3" != "-" ]]; then
   chi2_max=$3
fi

root_options="-l -q -b"
if [[ -n "$4" && "$4" != "-" ]]; then
   root_options="$4"
fi

refit_all=1
if [[ -n "$5" && "$5" != "-" ]]; then
   refit_all=$5
fi


curr_path=`pwd`

for dir in `ls -d ${maxsnr_template}`
do
     echo
     echo "Fitting data in $dir"
     rm -f taufit.txt

     cd $dir     
     
     if [[ $refit_all -gt 0 ]]; then
        echo "~/github/crab_frb_paper/scripts/calib/refit_tau.sh 0"
        ~/github/crab_frb_paper/scripts/calib/refit_tau.sh 0
     fi
     
     dataset=`echo $dir |  awk '{i=index($1,"/eda2/2");print substr($1,i+6,23);}'`
#     cat *.refit | grep -v nan | awk -v snr_threshold=${snr_threshold} -v chi2_max=${chi2_max} '{if($1>0 && $11>=snr_threshold && $12>chi2_max ){print $9;}}' > tau.txt
     echo "mv tau.txt tau.txt.old"
     mv tau.txt tau.txt.old
     cat *.refit | grep -v nan | awk '{if($1>0){print $9;}}' > tau.txt
     
     unixtime=-1
     if [[ -s ../../../../../UNIXTIME.txt ]]; then
        unixtime=`cat ../../../../../UNIXTIME.txt`
     fi
     
     cp ~/github/crab_frb_paper/scripts/root/histotau.C .

     rm -f taufit.txt     
     root ${root_options} "histotau.C(\"tau.txt\",0,1,0,0.08,100,0,\"Scattering Time [sec]\", \"Number of pulses fitted\", 0, \"${dataset}\", NULL, \"_histo\", $unixtime, \"${dataset}\" )"
     cat taufit.txt        
     cd -
done

cd $curr_path

# path=/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged/pulses_snr5_calibrated/
# cat ${path}/taufit.txt | awk '{if($1!="#"){print  $7" 86400 "$3*1000" "$2*1000;}}' > taugauss_vs_time.txt
# cat ${path}/taufit.txt | awk '{if($1!="#"){print  $7" 86400 "$5*1000" "$6*1000;}}' > taumean_vs_time.txt

maxsnr_path=/media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/202?_??_??_pulsars_msok/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch/merged_channels_??????????/presto_sps_thresh5_numdms10_dmstep0.01/merged/pulses_snr*.??_calibrated/
cat ${maxsnr_path}/taufit.txt | awk '{if($1!="#"){print  $7" 86400 "$3*1000" "$2*1000;}}' > maxsnr_taugauss_vs_time.txt
cat ${maxsnr_path}/taufit.txt | awk '{if($1!="#"){print  $7" 86400 "$5*1000" "$6*1000;}}' > maxsnr_taumean_vs_time.txt

echo "cp ~/github/crab_frb_paper/scripts/root/plot_scattau_vs_time.C ."
cp ~/github/crab_frb_paper/scripts/root/plot_scattau_vs_time.C .

mkdir -p images/
# root -l -b -q "plot_scattau_vs_time.C(\"taugauss_vs_time.txt\")"

echo "TEST?"
if [[ -s "arfiles.txt" ]]; then
   root -l -b -q "plot_scattau_vs_time.C(\"taumean_vs_time.txt\",\"maxsnr_taumean_vs_time.txt\",\"arfiles.txt\")"
else
   root -l -b -q "plot_scattau_vs_time.C(\"taumean_vs_time.txt\",\"maxsnr_taumean_vs_time.txt\")"
fi   



