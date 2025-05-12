#!/bin/bash

cp ~/github/crab_frb_paper/scripts/root/plotslope_err.C .
cp ~/github/crab_frb_paper/scripts/root/plotrisetime_err.C .

root -l "plotslope_err.C(\"slope_vs_dm_leading_edge.txt\",\"poly2\",56.62,56.8,\"leading_edge\")"
root -l "plotslope_err.C(\"slope_vs_index_pulse.txt\",\"poly2\",56.62,56.8,\"pulse\")"
root -l "plotrisetime_err.C(\"risetime_vs_index.txt\",\"poly2\",56.59,56.8)"

