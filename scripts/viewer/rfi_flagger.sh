#!/bin/bash 

read -p "Enter the DM of the source: " DM

for fits_file in *.fits* 
do 
        echo "Processing file: $fits_file"
#        python3 ~/Documents/git/ICRAR/File/RFI_Flagger/WORKING_FINAL/RFI_Signal_Flagger.py "$fits_file" "$DM"
        python3 ~/github/crab_frb_paper/scripts/viewer/RFI_Signal_Flagger.py "$fits_file" "$DM"
done 
