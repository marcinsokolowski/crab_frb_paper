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
   filename1="eda2.txt"
   if len(sys.argv) > 1 and sys.argv[1] != "-":
      filename1 = sys.argv[1]
      
   filename2=None
   if len(sys.argv) > 2 and sys.argv[2] != "-" :
      filename2 = sys.argv[2]
      

   (options, args) = parse_options(2)      

   
   (x_arr1,x_arr1_err,y_arr1,y_arr1_err,y_arr1_min,y_arr1_max,color1) = read_data_file( filename1 )   
   print("READ %d MPs from file %s (color=%s)" % (len(x_arr1),filename1,color1))
   
   x_arr2 = None
   if filename2 is not None :
      (x_arr2,x_arr2_err,y_arr2,y_arr2_err,y_arr2_min,y_arr2_max,color2) = read_data_file( filename2 )   
      print("READ %d IPs from file %s (color=%s)" % (len(x_arr2),filename2,color2))
      
   fig, ax1 = plt.subplots()
   
   plt.yscale('log') # nonposy='clip')
   plt.xscale('log') # nonposy='clip')
   
   plt.xlim((10,10000))
   plt.ylim((0.01,300))


   # color = 'tab:black'
   ax1.set_xlabel( options.x_axis,fontsize=20)
   ax1.set_ylabel( options.y_axis,fontsize=20)
#   ax1.plot(t, data1, color=color, marker='+', linestyle='None' )
   ax1.errorbar( x_arr1, y_arr1, yerr=y_arr1_err, fmt='*', color="black", label="This work" )

   ##############################################################################################################################
   # Bhat et al - MWA DEMO : see page 1 in /home/msok/Desktop/SKA/papers/2024/EDA2_FRBs/20250417_digitisation_and_plots_from_MWA-DEMO.odt
   # upper limit at 100 MHz is 5 kJy 
   x_bhat = np.array([100,200])
   x_bhat_err = np.array([3,3])
   y_bhat = np.array([5,50])
   y_bhat_err = np.array([0.00,1.1])
   ax1.errorbar( x_bhat[0:1], y_bhat[0:1], yerr=y_bhat_err[0:1], fmt='v', color="green" )
   # point a
   ax1.errorbar( x_bhat[1:], y_bhat[1:], yerr=y_bhat_err[1:], fmt='s', color="green", label="Bhat et al., ApJ, 2007" )
   ##############################################################################################################################
   
   ##############################################################################################################################
   # Popov, M. V., et al. 2006, Astronomy Reports, 50, 562, see page 2 in /home/msok/Desktop/SKA/papers/2024/EDA2_FRBs/20250417_digitisation_and_plots_from_MWA-DEMO.odt
   x_popov = np.array([23.23,111,600])
   x_popov_err = np.array([0,0,0])
   y_popov = np.array([0.15,0.7,34.00])
   y_popov_err = np.array([0.15,0.7,34.00])*0.3 # <30% - they say on page 5 in Popov ( see ~/Desktop/SKA/papers/2024/EDA2_FRBs/references/Ramesh_Bhat/MWA_demonstrator )
   ax1.errorbar( x_popov, y_popov, yerr=y_popov_err, fmt='^', color="red", label="Popov et al., ARep, 2006" )

   ##############################################################################################################################      
   # Ramesh ATCA paper :
   # Bhat et al, The Astrophysical Journal, Volume 676, Issue 2, pp. 1200-1209 (2008)., https://ui.adsabs.harvard.edu/abs/2008ApJ...676.1200B/abstract
   x_bhat = np.array([1300])
   x_bhat_err = np.array([0])
   y_bhat = np.array([45])
   y_bhat_err = np.array([0.00])
   ax1.errorbar( x_bhat, y_bhat, yerr=y_bhat_err, fmt='P', color="purple", label="Bhat et al., ApJ, 2008" )
   
   ##############################################################################################################################
   # Cordes et al, ApJ, The Astrophysical Journal, Volume 612, Issue 1, pp. 375-388.
   # https://ui.adsabs.harvard.edu/abs/2004ApJ...612..375C/abstract
   x_cordes = np.array([430,1475,2330,2850,8800])
   x_cordes_err = np.array([0,0,0,0,0])
   y_cordes = np.array([155,1.03,0.086,0.089,0.880])
   y_cordes_err = np.array([0.00,0.00,0.00,0.00,0.00])
   ax1.errorbar( x_cordes, y_cordes, yerr=y_cordes_err, fmt='o', color="red", label="Cordes et al., ApJ, 2004" )

   
   
   # TODO : Apurba, Doskoch 

   
#   if x_arr2 is not None :
#      ax1.errorbar( x_arr2, y_arr2, yerr=y_arr2_err, fmt='*', color="black", label="This work" )
   
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
   