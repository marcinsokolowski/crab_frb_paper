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

# parsing options :
from optparse import OptionParser,OptionGroup
import errno
import getopt
import optparse


pylab.rc('font', family='serif', size=20)

def parse_options(idx):
   parser=optparse.OptionParser()
   parser.set_usage("""parse_pulsars.py""")
   parser.add_option("-n","--n_merge","--merge_n",dest="n_merge",default=2,help="Merge so many data points [default: %default]",type="int")
   parser.add_option('--errors','--err',action="store_true",dest="errors",default=False, help="Use data errors in calculating correlation coefficient [default %default]")
   (options,args)=parser.parse_args(sys.argv[idx:])

   return (options, args)



pylab.rc('font', family='serif', size=20)

def correlation_with_errors(x, x_err, y, y_err):
    """
    Calculates the weighted Pearson correlation coefficient, 
    considering errors in x and y.

    Args:
        x (array-like): x data.
        x_err (array-like): Errors in x.
        y (array-like): y data.
        y_err (array-like): Errors in y.

    Returns:
        float: Weighted Pearson correlation coefficient.
    """
    if len(x) != len(y) or len(x) != len(x_err) or len(y) != len(y_err):
        raise ValueError("Input arrays must have the same length")

    weights_x = 1.0 / np.array(x_err)**2
    weights_y = 1.0 / np.array(y_err)**2
    weights = weights_x * weights_y

    mean_x = np.average(x, weights=weights_x)
    mean_y = np.average(y, weights=weights_y)

    numerator = np.sum(weights * (x - mean_x) * (y - mean_y))
    denominator = np.sqrt(np.sum(weights * (x - mean_x)**2) * np.sum(weights *(y - mean_y)**2))

    if denominator == 0:
      return 0  #Avoid division by zero

    return numerator / denominator

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
      return (np.array(x_arr),np.array(x_err_arr),np.array(y_arr),np.array(y_err_arr),min_x,max_x)
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

   (options, args) = parse_options(1)
      
#   tau_filename1="taumean_vs_time.txt"
#   if len(sys.argv) > 2:
#      tau_filename1 = sys.argv[2]     
      
   x_arr=None
   x_err=None
   y_arr=None
   y_err=None
   if options.errors : 
      (x_arr,x_err,y_arr,y_err,min_x,max_x) = read_data_file( filename, errors=True )
      print("READ WITH ERRORS : %d / %d / %d / %d elements" % (len(x_arr),len(x_err),len(y_arr),len(y_err)))
   else :
      (x_arr,y_arr,min_x,max_x) = read_data_file( filename, errors=False )
   
   fig, ax1 = plt.subplots()

   color = 'tab:red'
   ax1.set_xlabel('X values',fontsize=20)
   ax1.set_ylabel('Y values', fontsize=20)
#   ax1.set_ylabel(r'Correlation of arbitrary parameters',fontsize=20)
   ax1.errorbar( x_arr, y_arr, yerr=y_err, color=color, marker='+', linestyle='None' )
   
   if options.errors :
      corr_coeff_local = correlation_with_errors( x_arr, x_err, y_arr, y_err )
      print("Correlation coefficient using errors = %.6f" % (corr_coeff_local));
   else :
      corr_pearsons, pvalue = pearsonr( x_arr, y_arr )    
      print('Pearsons correlation: %.3f, pvalue = %e' % (corr_pearsons,pvalue))
   
      corr_spearmans, _ = spearmanr( x_arr, y_arr )
      print('Spearmans correlation: %.3f' % corr_spearmans)
   
      covariance = cov( x_arr, y_arr )
      print(covariance)
   
   fig.tight_layout()  # otherwise the right y-label is slightly clipped
   plt.show()

if __name__ == "__main__":
   main()
   