# Usage :
# cd ~/Desktop/SKA/papers/2024/EDA2_FRBs/references/Ramesh_Bhat/MWA_demonstrator
# python ./peak_flux_vs_freq.py - eda2.txt --title="Peak flux density vs. frequency" --x_axis="Frequency [MHz]" --y_axis="Peak flux density [kJy]"
import matplotlib.pyplot as plt
from matplotlib import pyplot as pp
import seaborn as sns
import numpy as np
import sys
from astropy.time import Time
import pylab
import math

import errno
from optparse import OptionParser,OptionGroup
import errno
import getopt
import optparse

# is it useful ???
import powerlaw

import scipy.optimize as optimization

pylab.rc('font', family='serif', size=20)

def parse_options(idx):
   parser=optparse.OptionParser()
   parser.set_usage("""parse_pulsars.py""")
   parser.add_option('--scale_tau',action="store_true",dest="scale_tau",default=True, help="Scale scattering time from one frequency to another set --freq_in and --freq_out [default: %default]")
   parser.add_option("--freq_in","--frequency_input",dest="freq_in",default=215.625,help="Input frequency [default: %default]",type="float")
   parser.add_option("--freq_out","--frequency_output",dest="freq_out",default=610,help="Output frequency [default: %default]",type="float")
   parser.add_option("--tau_index","--index",dest="index",default=-4,help="Output frequency [default: %default] or use -4.4 for Kolomogorov turbulence",type="float")
   
#   parser.add_option("--obstime","--total_time",dest="obstime",default=66.8028,help="Total observing time in hours [default: %default]",type="float")
#   parser.add_option("--period","--pulsar_period_sec","--pp_sec",dest="pulsar_period_sec",default=0.0333924123,help="Pulsar period in seconds [default: %default]",type="float")
#   parser.add_option("--fit_min","--fit_min_x","--xmin",dest="fit_min",default=2.00,help="Minimum of the fit [default: %default]",type="float")
#   parser.add_option("-n","--n_bins","--n_bin","--nbin","--nbins",dest="nbins",default=100,help="Number of histogram bins [default: %default]",type="int")
   
#   parser.add_option("-t","--title",dest="title",default="Interesting plot",help="Plot title [default: %default]")
#   parser.add_option("-x","--x_axis",dest="x_axis",default="X axis [units]",help="X axis title [default: %default]")
#   parser.add_option("-y","--y_axis",dest="y_axis",default="Y axis [units]",help="Y axis title [default: %default]")

   (options,args)=parser.parse_args(sys.argv[idx:])

   return (options, args)


def read_data_file( filename, errors=True ) :
   print("read_data(%s) ..." % (filename))
   file=open(filename,'r')

   # reads the entire file into a list of strings variable data :
   data=file.readlines()
   # print data

   # initialisation of empty lists :
   color="black"
   x_arr=[]
   y_arr=[]
   x_err_arr=[]
   y_err_arr=[]
   min_x=1e20
   max_x=-1e20
   cnt=0
   for line in data : 
      words = line.split(' ')

      if line[0] == '#' or line[0]=='\n' or len(line) <= 0 or len(words)<2 :
         continue
      
      if line[0] != "#" :
#         print line[:-1]
         if errors : 
            x=float(words[0+0])
            x_err=float(words[1+0])
            y=float(words[2+0])
            y_err=float(words[3+0])
            
            if len(words) > 4 and words[4+0]!="\n" :
               color = words[4+0]
         else :
            x=float(words[0+0])
            y=float(words[1+0])
            if len(words) > 2 and words[2+0]!="\n" :
               color = words[2+0]

                        

         x_arr.append(x)
         y_arr.append(y)
         
         if errors : 
            x_err_arr.append(x_err)
            y_err_arr.append(y_err)
         cnt += 1

         if x > max_x :
            max_x = x
         if x < min_x :
            min_x = x

   if errors :
      return (np.array(x_arr),np.array(x_err),np.array(y_arr),np.array(y_err),min_x,max_x,color)
   else :
      return (np.array(x_arr),np.array(y_arr),min_x,max_x,color)

def read_data_file_onecol( filename) :
   print("read_data(%s) ..." % (filename))
   file=open(filename,'r')

   # reads the entire file into a list of strings variable data :
   data=file.readlines()
   # print data

   # initialisation of empty lists :
   x_arr=[]
   min_x=1e20
   max_x=-1e20
   cnt=0
   for line in data : 
      words = line.split(' ')

      if line[0] == '#' or line[0]=='\n' or len(line) <= 0 or len(words)<1 :
         continue
      
      if line[0] != "#" :
#         print line[:-1]
         x=float(words[0+0])
         x_arr.append(x)
         cnt += 1

         if x > max_x :
            max_x = x
         if x < min_x :
            min_x = x

   return (np.array(x_arr),min_x,max_x)


def ux2mjd( ux_arr ) :
   t_arr = Time( ux_arr, format='unix')      
   mjd_arr = t_arr.to_value('mjd')
   
   return mjd_arr

def my_power_law(x, n, a):
    return n*(x/10.00)**a

def main() :
   filename1="taumean_vs_time.txt"
   if len(sys.argv) > 1 and sys.argv[1] != "-":
      filename1 = sys.argv[1]
      
   (options, args) = parse_options(1)      

   
   (x_arr,x_arr_err,y_arr,y_arr_err,min_x,max_x,color) = read_data_file( filename1 )   
   print("READ %d data points from file %s" % (len(y_arr),filename1))
   
   freq_ratio = (options.freq_out / options.freq_in )
   scaling_factor = np.power( freq_ratio , options.index )
   print("# DEBUG : freq_ratio = %.8f -> scaling_factor = %.8f" % (freq_ratio,scaling_factor))
   for i in range(0,len(y_arr)) :
      tau_in = y_arr[i]
      tau_out = tau_in*scaling_factor
      
      print("%.8f" % (tau_out))
      
#      print("%.8f (from %.8f)" % (tau_out,tau_in))
   

if __name__ == "__main__":   
   main()
   