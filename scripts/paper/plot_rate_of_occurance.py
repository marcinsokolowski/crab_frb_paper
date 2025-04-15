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
   mp_filename="mp_fluence_all_JyS.txt"
   if len(sys.argv) > 1:
      mp_filename = sys.argv[1]
      
   ip_filename="ip_fluence_all_JyS.txt"
   if len(sys.argv) > 2:
      ip_filename = sys.argv[2]
      

   (options, args) = parse_options(2)      

   (mp_arr,min_mp,max_mp) = read_data_file_onecol( mp_filename )   
   (ip_arr,min_ip,max_mp) = read_data_file_onecol( ip_filename )   
      
   PulsarPeriods = (options.obstime*3600.00)/options.pulsar_period_sec;
   print("Number of pulsar rotations = %.4f" % (PulsarPeriods));

      
   print("READ %d MPs from file %s" % (len(mp_arr),mp_filename))
   print("READ %d IPs from file %s" % (len(ip_arr),ip_filename))
#   tau1_arr = tau1_arr - 5.00

   # MPs :
   mp_counts,mp_bin_edges = np.histogram(mp_arr,options.nbins,range=(0.09,50))
   mp_bin_centres = (mp_bin_edges[:-1] + mp_bin_edges[1:])/2.
   mp_err=np.sqrt(mp_counts)
   print("DEBUG : %d vs. %d vs. %d vs. %d" % (len(mp_arr),len(mp_bin_centres),len(mp_counts),len(mp_err)))   

   mp_fit_results = powerlaw.Fit( mp_arr , xmin=options.fit_min )
#   mp_fit_results.power_law.plot_pdf( mp_arr )
   print(mp_fit_results.power_law.alpha)  
   print(mp_fit_results.power_law.xmin)
   R_mp, p_mp = mp_fit_results.distribution_compare('power_law', 'lognormal')

   # IPs :
   ip_counts,ip_bin_edges = np.histogram(ip_arr,options.nbins,range=(0.09,50))
   ip_bin_centres = (ip_bin_edges[:-1] + ip_bin_edges[1:])/2.
   ip_err=np.sqrt(ip_counts)
   print("DEBUG : %d vs. %d vs. %d vs. %d" % (len(ip_arr),len(ip_bin_centres),len(ip_counts),len(ip_err)))
   ip_fit_results = powerlaw.Fit( ip_arr , xmin=options.fit_min )
   print(ip_fit_results.power_law.alpha)  
   print(ip_fit_results.power_law.xmin)
   R_ip, p_ip = ip_fit_results.distribution_compare('power_law', 'lognormal')



   
#   pp.errorbar(mp_bin_centres, mp_counts, yerr=mp_err, fmt='o', log=True)

   fig, ax1 = plt.subplots()
   
   plt.yscale('log') # nonposy='clip')
   plt.xscale('log') # nonposy='clip')
   
   plt.xlim((1e-3,1e3))
#   plt.ylim((1e-8,0.1))

   color = 'red'
   ax1.set_xlabel('Fluence [Jy s]',fontsize=20)
   ax1.set_ylabel('Rate of Occurance (per rotation)', color='black', fontsize=20)
   
   # scaled by number of rotations -> to make it per rotation
   ax1.errorbar( mp_bin_centres, mp_counts/PulsarPeriods, yerr=mp_err/PulsarPeriods, fmt='o', color=color, label="GPs from main pulse" )
#   print(optimization.curve_fit( my_power_law, mp_bin_centres, mp_counts/PulsarPeriods, [1e-7,-3], mp_err/PulsarPeriods))
#   print(optimization.curve_fit( my_power_law, mp_bin_centres, mp_counts, [1.00,-3], mp_err )) # , bounds=(2,10000000.0)))
#   print(mp_counts)
   
   ax1.errorbar( ip_bin_centres, ip_counts/PulsarPeriods, yerr=ip_err/PulsarPeriods, fmt='o', color='blue' , label="GPs from inter-pulse")
#   ax1.errorbar( mp_bin_centres, mp_counts, yerr=mp_err, fmt='o', color=color )
#   ax1.errorbar( ip_bin_centres, ip_counts, yerr=ip_err, fmt='o', color='blue' )


#   ax1.plot( jb_mjd, jb_dm, marker='*', color=color, markersize=12 )
   ax1.tick_params(axis='y', labelcolor='black')
#   ax1.yaxis.get_label().set_fontsize(40)
   #adjust position of x-axis label
   ax1.yaxis.set_label_coords(-0.1, .5)
   
#   mp_fit_results.power_law.plot_pdf( mp_arr, ax=ax1 )

   # add lines at different rates:
   intervals=[1,10,60,600,3600,36000] # per second
   for interval in intervals :
      y=options.pulsar_period_sec/interval
      plt.axhline(y=y, color='grey', linestyle='--')


      if interval == 1 :
         s = "1 / sec"      
      elif interval == 60 :
         s = "1 / min"
      elif interval == 600 :
         s = "1 / 10min"
      elif interval == 3600 :
         s = "1 / hr"
      elif interval == 36000 :
         s = "1 / 10 hr"
      else :
         s=("1 / %d sec" % interval)
         
      plt.text(1100,y,s)

   # over plot fitted power laws see : page 2 in /home/msok/Desktop/SKA/papers/2024/EDA2_FRBs/20250411_rate_of_occurance_PAPER.odt 
   # fits are in ROOT because they give proper errors, while I cannot see it in package powerlaw ...
   # MP fit:
   # /media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/crab_full_analysis_final/merged/PAPER_PLOTS/Fluence_distrib/RateOfOccurance
   # root [2] .x FluenceRateOfOccurancePowerLaw_PAPER.C("mp_fluence_all_JyS.txt",66.8028,0,1.40,100.000)
   # 1  p0           3.98003e-06   1.18788e-07  -1.18578e-07   1.18990e-07
   # 2  p1          -2.80297e+00   2.16837e-02  -2.18970e-02   2.14783e-02
   plt.plot( mp_bin_centres[1:], my_power_law(mp_bin_centres[1:],3.98003e-06,-2.80297), color="red" )
   
   # IP fit:
   # page 3 in /home/msok/Desktop/SKA/papers/2024/EDA2_FRBs/20250411_rate_of_occurance_PAPER-ONE-PLOT.odt
   # /media/msok/5508b34c-040a-4dce-a8ff-2c4510a5d1a3/eda2/crab_full_analysis_final/merged/PAPER_PLOTS/Fluence_distrib/RateOfOccurance
   # root [5] .x FluenceRateOfOccurancePowerLaw_PAPER.C("ip_fluence_all_JyS.txt",66.8028,0,1.40,100.000)
   # 1  p0           2.22470e-07   2.59854e-08  -2.53392e-08   2.66137e-08
   # 2  p1          -3.38870e+00   7.95878e-02  -8.16860e-02   7.76838e-02
   plt.plot( ip_bin_centres[1:], my_power_law(ip_bin_centres[1:],2.22470e-07,-3.38870), color="blue" )



   # matplotlib histogram
#   plt.hist( mp_arr, color = 'blue', edgecolor = 'black', bins = int(180/5), log=True)
   
   # seaborn histogram
#   sns.distplot( mp_arr, hist=True, kde=False, bins=int(180/5), color = 'blue', hist_kws={'edgecolor':'black'} )
   
   plt.title(' Fluence of Giant Pulses (GPs) in main pulse (MP) and inter-pulse (IP)')

#   plt.legend(['GPs from main pulse', 'GPs from interpulse'], loc='best')
   plt.legend(loc='upper left')
   plt.show()

if __name__ == "__main__":   
   main()
   