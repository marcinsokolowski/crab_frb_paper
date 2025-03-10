#!/bin/bash

cd /media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2

# Scattering vs. time :
~/github/crab_frb_paper/scripts/calib/scattering_time_from_all_fits.sh - - - "-l -q -b"

# all vs. time :
~/github/crab_frb_paper/scripts/calib/process_all_datasets.sh

# fluence for all datasets and power law index vs. time :
~/github/crab_frb_paper/scripts/calib/replot_fluence_vs_time.sh




