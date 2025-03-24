#!/bin/bash

files="_DM*.singlepulse"
if [[ -n "$1" && "$1" != "-" ]]; then
   files="$1"
fi

sefd=7911
if [[ -n "$2" && "$2" != "-" ]]; then
   sefd=$2
fi

# was 31250000 0.001
# cat ${files} | grep -v "#" | awk -v sefd=${sefd} -v npol=2 -v bw_hz=29629629 -v timeres_sec=0.00096768 '{snr=$2;downfact=$5;inttime_sec=timeres_sec*downfact;sigma_sim=sefd/sqrt(npol*bw_hz*inttime_sec);flu=snr*sigma_sim;print snr" -> "flu;}'

# My derivations and why it is ok not to use downfact just use sigma (in fact S/N) as is in PRESTO output file is here : /home/msok/Desktop/EDA2/papers/2024/EDA2_FRBs/20250212_Scott_Ransom_downsampling.odt
# basically this is because sigma = S/N = CONSTANT and corresponds to S/N calculated as peak_flux/sigma_native , where sigma_native is std. dev. of noise in the native time resolution dt. 
# when downsampling is applied, it stays the same because of 1/SQRT(BOXCAR) "as long as the pulse is still resolved" - i.e. BOXCAR*dt <= PULSE_WIDTH
# see also link below and Scott's comment copy pasted below here:
# After discussion with Pratik and here:
# https://github.com/scottransom/presto/blob/master/bin/single_pulse_search.py
# if RMS is independent of downsampling I can just multiply by my original sigma_simulation - without taking into account downfact, hence downfact=1 below (vs. downfact=$5 in code above).
# 
# Scott Ransom :
# The definition of "sigma" used is possibly slightly different from that used in other codes for S/N:
#
#    sigma = sum(signal-bkgd_level)/RMS/sqrt(boxcar_width)
#
# where the bkgd_level is typically 0 after detrending and RMS=1 after normalization. This definition has the advantage that you will get (basically) the same sigma for any pulse no
# matter how much the input time series has been downsampled as long as the pulse is still resolved.
# 

# If SEFD = SEFD_I - already Stokes I SEFD this means that there is not need for npol=2 and I should just use npol=1 !!!
# My original results of calibrated flux density were by factor SQRT(2) underestimated (too low)
cat ${files} | grep -v "#" | awk -v sefd=${sefd} -v npol=1 -v bw_hz=29629629 -v timeres_sec=0.00096768 '{snr=$2;inttime_sec=timeres_sec;sigma_sim=sefd/sqrt(npol*bw_hz*inttime_sec);flu=snr*sigma_sim;print snr" -> "flu;}'
