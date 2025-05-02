# https://matplotlib.org/stable/gallery/subplots_axes_and_figures/two_scales.html
import matplotlib.pyplot as plt
import numpy as np
import sys
from astropy.time import Time
import pylab

from mpl_axes_aligner import align

pylab.rc('font', family='serif', size=20)

def read_data_file( filename, errors=True ) :
   print("read_data(%s) ..." % (filename))
   file=open(filename,'r')

   # reads the entire file into a list of strings variable data :
   data=file.readlines()
   # print data

   # initialisation of empty lists :
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
         else :
            x=float(words[0+0])
            y=float(words[1+0])

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
      return (np.array(x_arr),np.array(x_err),np.array(y_arr),np.array(y_err),min_x,max_x)
   else :
      return (np.array(x_arr),np.array(y_arr),min_x,max_x)

def ux2mjd( ux_arr ) :
   t_arr = Time( ux_arr, format='unix')      
   mjd_arr = t_arr.to_value('mjd')
   
   return mjd_arr

def main() :
   filename="DM_vs_UX.txt"
   if len(sys.argv) > 1:
      filename = sys.argv[1]
      
   tau_filename1="taumean_vs_time.txt"
   if len(sys.argv) > 2:
      tau_filename1 = sys.argv[2]

   jodrell_bank="jodrell_bank_crab_dm_vs_ux.txt"      
      
   (x_arr,x_err,y_arr,y_err,min_x,max_x) = read_data_file( filename )   
   x_arr_mjd = ux2mjd( x_arr )
   
   (tau1_t_arr,tau1_t_err,tau1_arr,tau1_err,tau1_min,tau1_max) = read_data_file( tau_filename1 )
   tau1_arr_mjd = ux2mjd( tau1_t_arr )
   
   (jb_ux,jb_ux_err,jb_dm,jb_dm_err,jb_dm_max,jb_dm_min) = read_data_file( jodrell_bank, errors=True )
   jb_mjd = ux2mjd( jb_ux )
#   tau1_arr = tau1_arr - 5.00

   # Create some mock data
#   t = np.arange(0.01, 10.0, 0.01)
#   data1 = np.exp(t)
#   data2 = np.sin(2 * np.pi * t)
   t = x_arr
   data1 = y_arr
   data2 = np.sin(2 * np.pi * x_arr ) 
   

   fig, ax1 = plt.subplots()

   color = 'tab:red'
   ax1.set_xlabel('MJD [days]',fontsize=20)
   ax1.set_ylabel('DM - 56.705825 [pc/cm$^3$]', color=color, fontsize=20)
#   ax1.plot(t, data1, color=color, marker='+', linestyle='None' )
   ax1.errorbar( x_arr_mjd, y_arr, yerr=y_err, fmt='o', color=color )
#   if len(jb_mjd) > 0 :
#   ax1.plot( jb_mjd, jb_dm, fmt='o', color='tab:black' )
   color2='tab:red'
   ax1.plot( jb_mjd, jb_dm, marker='*', color=color2, markersize=12 )
   ax1.tick_params(axis='y', labelcolor=color)
#   ax1.yaxis.get_label().set_fontsize(40)
   #adjust position of x-axis label
   ax1.yaxis.set_label_coords(-0.07, .5) # (-0.1, .5)

   ax2 = ax1.twinx()  # instantiate a second Axes that shares the same x-axis

   color = 'tab:blue'
   ax2.set_ylabel(r'Scattering Time ($\tau$) [ms]', color=color, fontsize=20)  # we already handled the x-label with ax1
#   ax2.plot(t, data2, color=color)
   ax2.errorbar( tau1_arr_mjd, tau1_arr, yerr=tau1_err, fmt='v', color=color )
   ax2.tick_params(axis='y', labelcolor=color)
#   ax2.yaxis.get_label().set_fontsize(40)
   ax2.yaxis.set_label_coords(+1.06, .5) # (+1.09, .5)

   # align axis 
   align.yaxes(ax1, 0.00, ax2, 2.00)    
   

   fig.tight_layout()  # otherwise the right y-label is slightly clipped
   plt.show()

if __name__ == "__main__":
   main()
   