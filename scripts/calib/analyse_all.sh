#!/bin/bash

#!/bin/bash

for dir in `ls -d 202?_??_??_pulsars_msok`
do
   echo 
   fulldir=${dir}/J0534+2200_flagants_ch40_ch256/256/filterbank_msok_64ch 
   if [[ -d ${fulldir} ]]; then
      cd ${dir}
      if [[ -d analysis_final ]]; then
         echo "Data $dir alread processed -> skipped"
      else
         pwd
         echo "Data $dir not processed yet -> processing now"
         
         echo "mv analysis! analysis_old!"
         mv analysis! analysis_old!
         
         echo "~/github/crab_frb_paper/scripts/calib/process_dataset.sh > analysis_final.out 2>&1" > analysis!
         chmod +x analysis!
         echo "./analysis!"
         ./analysis!
      fi
      cd ..
   else
      echo "WARNING : dataset $dir is incomplete (directory $fulldir does not exist)"
   fi
done
