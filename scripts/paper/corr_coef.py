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
         
         if errors:
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
   filename="DM_vs_TAU.txt"
   if len(sys.argv) > 1:
      filename = sys.argv[1]
      
   tau_filename1="taumean_vs_time.txt"
   if len(sys.argv) > 2:
      tau_filename1 = sys.argv[2]

      
      
   (x_arr,y_arr,min_x,max_x) = read_data_file( filename, errors=False )
   
   fig, ax1 = plt.subplots()

   color = 'tab:red'
   ax1.set_xlabel('MJD [days]',fontsize=20)
   ax1.set_xlabel('DM - 56.705825 [pc/cm$^3$]', fontsize=20)
   ax1.set_ylabel(r'Scattering Time ($\tau$) [ms]',fontsize=20)
   ax1.plot( x_arr, y_arr, color=color, marker='+', linestyle='None' )
   
   corr_pearsons, pvalue = pearsonr( x_arr, y_arr )    
   print('Pearsons correlation: %.3f, pvalue = %.3f' % (corr_pearsons,pvalue))
   
   corr_spearmans, _ = spearmanr( x_arr, y_arr )
   print('Spearmans correlation: %.3f' % corr_spearmans)
   
   covariance = cov( x_arr, y_arr )
   print(covariance)
   
   fig.tight_layout()  # otherwise the right y-label is slightly clipped
   plt.show()

if __name__ == "__main__":
   main()
   