# https://matplotlib.org/stable/gallery/subplots_axes_and_figures/two_scales.html
import matplotlib.pyplot as plt
import numpy as np
import sys
from astropy.time import Time
import pylab

# https://machinelearningmastery.com/how-to-use-correlation-to-understand-the-relationship-between-variables/
from scipy.stats import pearsonr
from scipy.stats import spearmanr
from numpy import cov

pylab.rc('font', family='serif', size=20)

def main() :
   # Parameters:
   N_low_z_gal = 1e8 # from ???
   N_crabs_per_gal = 1e2
   N_hours_per_day = 24
   

   crab_spectral_lumins = np.array( [1e30,1e34,1e40] )
   crab_rates = np.array( [1.686479e-15,1.624494e-27,1.535763e-45] )
   
   for i in range(0,len(crab_spectral_lumins)) :
      crab_rate = crab_rates[i]
      crab_lumin = crab_spectral_lumins[i]
   
      N_frb_rate = N_low_z_gal*N_crabs_per_gal*N_hours_per_day*crab_rate
      
      print("\n\n")
      print("FRB spectral luminosity = %e erg/Hz/s" % (crab_lumin))
      print("FRB rate = %e / day /  sky" % (N_frb_rate))
      
   
   

if __name__ == "__main__":
   main()
   