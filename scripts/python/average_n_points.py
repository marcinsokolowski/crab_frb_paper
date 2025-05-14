# https://matplotlib.org/stable/gallery/subplots_axes_and_figures/two_scales.html
import matplotlib.pyplot as plt
import numpy as np
import sys
from astropy.time import Time
import pylab
import math

from mpl_axes_aligner import align

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
   parser.add_option('--max_errors','--max_err',action="store_true",dest="max_error",default=False, help="Use maximum error value (specifically for DM errors from timing which are correlated and should not be added in quadratures) [default %default]")
   parser.add_option('--no_errors','--no_err',action="store_false",dest="errors",default=True, help="Input file with errors ? [default %default]")
   (options,args)=parser.parse_args(sys.argv[idx:])

   return (options, args)


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

         x_err = 0
         y_err = 0
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
         x_err_arr.append(x_err)
         y_err_arr.append(y_err)
         
         cnt += 1

         if x > max_x :
            max_x = x
         if x < min_x :
            min_x = x
            
         print("DEBUG : %.4f , %.4f , %.4f , %.4f" % (x,x_err,y,y_err))   

   print("DEBUG : returning with errors : len() = %d, %d, %d, %d" % (len(x_arr),len(x_err_arr),len(y_arr),len(y_err_arr)))
   return (np.array(x_arr),np.array(x_err_arr),np.array(y_arr),np.array(y_err_arr),min_x,max_x)

def ux2mjd( ux_arr ) :
   t_arr = Time( ux_arr, format='unix')      
   mjd_arr = t_arr.to_value('mjd')
   
   return mjd_arr

def merge_data( x_arr, x_err, y_arr, y_err, n_merge=2, max_diff=(86400*1.5), max_error=False ) :
  out_x_list=[]
  out_y_list=[]
  out_x_err_list=[]
  out_y_err_list=[]

  new_x_list = []
  new_y_list = []
  new_x_err_list = []
  new_y_err_list = []
  
  prev_ux = 0
  l = len(x_arr)
  i = 0
  while i < l :
     ux_diff = x_arr[i] - prev_ux 
     
     print("DEBUG : i = %d, ux_diff = %.4f [sec], smaller than limit %s" % (i,ux_diff,(ux_diff < max_diff)))
       
     finish = False
     if len(new_x_list) <= 0 or ux_diff < max_diff :
        print("DEBUG : adding new point (i=%d)" % (i))
        
        new_x_list.append( x_arr[i] )
        new_y_list.append( y_arr[i] )
        new_x_err_list.append( x_err[i] )
        new_y_err_list.append( y_err[i] )        
     else :
        if ux_diff >= max_diff :
           # too long time interval :
           print("DEBUG : diff = %.2f [sec] -> broken sequence -> finishing now" % (ux_diff))
           finish = True
                
     if len(new_x_list) == n_merge or finish :
        print("DEBUG : reached %d or finish" % (n_merge))
        new_x_list = np.array(new_x_list)
        new_y_list = np.array(new_y_list)
        new_x_err_list = np.array(new_x_err_list)
        new_y_err_list = np.array(new_y_err_list)
        
        new_x = new_x_list.mean()
        new_y = new_y_list.mean()
        
        sum2_x=0
        sum2_y=0
        count=0
        for k in range(0,len(new_x_err_list)) : 
           sum2_x += new_x_err_list[k]*new_x_err_list[k]
           sum2_y += new_y_err_list[k]*new_y_err_list[k]
           count += 1
           
        new_x_err = math.sqrt(sum2_x)/count    
        new_y_err = math.sqrt(sum2_y)/count    
        
        if max_error : 
           new_x_err = max(new_x_err_list)
           new_y_err = max(new_y_err_list)
        
        out_x_list.append(new_x)
        out_y_list.append(new_y)
        out_x_err_list.append(new_x_err)
        out_y_err_list.append(new_y_err)
     
        new_x_list = []
        new_y_list = []
        new_x_err_list = []
        new_y_err_list = []
        
        if finish :
           # this means there is new point which was not averaged with the previous group and needs to start a new list :
           new_x_list.append( x_arr[i] )
           new_y_list.append( y_arr[i] )
           new_x_err_list.append( x_err[i] )
           new_y_err_list.append( y_err[i] ) 

     else : 
        print("DEBUG : i = %d -> else again ?" % (i))
                

     prev_ux = x_arr[i]  
     i += 1

  print("DEBUG : returing arrays of sizes %d, %d, %d, %d" % (len(out_x_list),len(out_x_err_list),len(out_y_list),len(out_y_err_list)))     
  return (out_x_list,out_x_err_list,out_y_list,out_y_err_list)

def main() :
   filename="taumean_vs_time.txt"
   if len(sys.argv) > 1:
      filename = sys.argv[1]
      
   outfile="taumean_vs_time_2merged.txt"
   if len(sys.argv) > 2:
      outfile = sys.argv[2]

   (options, args) = parse_options(2)

  
   (x_arr,x_err,y_arr,y_err,min_x,max_x) = read_data_file( filename, errors=options.errors )   
#    x_arr_mjd = ux2mjd( x_arr )
   print("READ : %d / %d / %d / %d points" % (len(x_arr),len(x_err),len(y_arr),len(y_err)))

   (out_x_list,out_x_err_list,out_y_list,out_y_err_list) = merge_data( x_arr, x_err, y_arr, y_err, n_merge=options.n_merge, max_error=options.max_error )
   print("READ : %d / %d / %d / %d points" % (len(out_x_list),len(out_x_err_list),len(out_y_list),len(out_y_err_list)))
   
   outf = open(outfile,"w")
   out_count=len(out_x_list)
   for i in range(0,out_count) :
      if options.errors : 
         line = ("%.8f %.8f %.8f %.8f\n" % (out_x_list[i],out_x_err_list[i],out_y_list[i],out_y_err_list[i]))
      else :
         line = ("%.8f %.8f\n" % (out_x_list[i],out_y_list[i]))
      outf.write(line)
   
   outf.close()
      
   
if __name__ == "__main__":
   main()
   