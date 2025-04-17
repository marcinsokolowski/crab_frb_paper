# https://matplotlib.org/stable/gallery/subplots_axes_and_figures/two_scales.html
import matplotlib.pyplot as plt
from matplotlib import pyplot as pp
import seaborn as sns
import numpy as np
import sys
from astropy.time import Time
import pylab

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
   parser.add_option("--obstime","--total_time",dest="obstime",default=66.8028,help="Total observing time in hours [default: %default]",type="float")
   parser.add_option("--period","--pulsar_period_sec","--pp_sec",dest="pulsar_period_sec",default=0.0333924123,help="Pulsar period in seconds [default: %default]",type="float")
   parser.add_option("--fit_min","--fit_min_x","--xmin",dest="fit_min",default=2.00,help="Minimum of the fit [default: %default]",type="float")
   parser.add_option("-n","--n_bins","--n_bin","--nbin","--nbins",dest="nbins",default=100,help="Number of histogram bins [default: %default]",type="int")
   
   parser.add_option("-t","--title",dest="title",default="Interesting plot",help="Plot title [default: %default]")
   parser.add_option("-x","--x_axis",dest="x_axis",default="X axis [units]",help="X axis title [default: %default]")
   parser.add_option("-y","--y_axis",dest="y_axis",default="Y axis [units]",help="Y axis title [default: %default]")

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
   filename1="data1.txt"
   if len(sys.argv) > 1:
      filename1 = sys.argv[1]
      
   filename2=None
   if len(sys.argv) > 2:
      filename2 = sys.argv[2]
      

   (options, args) = parse_options(2)      

   (x_arr1,x_arr1_err,y_arr1,y_arr1_err,y_arr1_min,y_arr1_max,color1) = read_data_file( filename1 )   
   print("READ %d MPs from file %s (color=%s)" % (len(x_arr1),filename1,color1))
   
   
   x_arr2 = None
   if filename2 is not None and filename2 != "-" :
      (x_arr2,x_arr2_err,y_arr2,y_arr2_err,y_arr2_min,y_arr2_max,color2) = read_data_file( filename2 )   
      print("READ %d IPs from file %s (color=%s)" % (len(x_arr2),filename2,color2))
      
   fig, ax1 = plt.subplots()
   
   plt.yscale('log') # nonposy='clip')
   plt.xscale('log') # nonposy='clip')

   # color = 'tab:black'
   ax1.set_xlabel( options.x_axis,fontsize=20)
   ax1.set_ylabel( options.y_axis, color=color1, fontsize=20)
#   ax1.plot(t, data1, color=color, marker='+', linestyle='None' )
   ax1.errorbar( x_arr1, y_arr1, yerr=y_arr1_err, fmt='o', color=color1 )
   
   if x_arr2 is not None :
      ax1.errorbar( x_arr2, y_arr2, yerr=y_arr2_err, fmt='o', color=color2 )
   
#   if len(jb_mjd) > 0 :
#   ax1.plot( jb_mjd, jb_dm, fmt='o', color='tab:black' )
#   ax1.plot( jb_mjd, jb_dm, marker='*', color=color, markersize=12 )
#   ax1.tick_params(axis='y', labelcolor=black)

   
   plt.title( options.title )

#   plt.legend(['GPs from main pulse', 'GPs from interpulse'], loc='best')
   plt.legend(loc='upper left')
   plt.get_current_fig_manager().window.showMaximized()
   plt.show()

if __name__ == "__main__":   
   main()
   