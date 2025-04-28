# https://numpy.org/doc/2.2/reference/random/generated/numpy.random.poisson.html

import sys
import numpy as np
import matplotlib.pyplot as plt


show=False
debug = False

obstime = 3600.00 # second observations
period = 0.0333924123
# n_periods = obstime/period
time_resolution = (1.08*64*14)/1000000.00 # as time resolution of my data
n_samples = int( obstime / time_resolution )

if __name__ == '__main__':   
   Lambda = period*(1.00/500.00)
   if len(sys.argv) > 1 and sys.argv[1] != "-" :
      Lambda = float(sys.argv[1])
   n_samples = int( obstime / Lambda )    
 
   oversampling_factor = 1
   if len(sys.argv) > 2 and sys.argv[2] != "-" :
      oversampling_factor = float(sys.argv[2])
      
   n_samples = int(n_samples*oversampling_factor)

   print("###################################")
   print("PARAMETERS:")   
   print("###################################")
   print("n_samples = %d" % (n_samples))
   print("###################################")

   outfile=("time_diff_Lambda_%.8f.txt" % (Lambda))
   
   s = np.random.poisson(Lambda, int(n_samples))
   print("DEBUG : s[0] = %.6f , s[1] = %.6f" % (s[0],s[1]))
   count, bins, ignored = plt.hist(s, 10, log=True ) # , density=True)

   if show :
      plt.show()

   # time_between_pulses = np.zeros( n_samples-1 )
   time_between_pulses = []
   non_zero=0
   for i in range(0,n_samples) :
      if s[i] > 0 :
         if debug : 
            print("%d : %.8f" % (i,s[i]))
         non_zero += 1

   print("Number of non-zero bins %d vs. all bins = %d" % (non_zero,len(s)))      

   outf = open(outfile,"w")
   prev_i = -1
   for i in range(0,n_samples) :
      if s[i] > 0 :
         if prev_i >= 0 :         
            diff = (i - prev_i)*time_resolution
            time_between_pulses.append(diff)
            line=("%.8f\n" % diff)
            outf.write(line)
   
         prev_i = i 
   
   outf.close()      
       
   count2, bins2, ignored2 = plt.hist(time_between_pulses, 50, log=True )      
   
   if show :
      plt.show()


# pulse_times = np.zeros(len(s))
# pulse_times[0] = s[0]
# for i in range(1,len(s)) :
#   pulse_times[i] = pulse_times[i-1] + s[i]
#
#time_between_pulses = np.zeros(len(s-1))
#for i in range(1,len(pulse_times)) :
#   diff = pulse_times[i] - pulse_times[i-1]
#   time_between_pulses[i-1] = diff
   
#count2, bins2, ignored2 = plt.hist(time_between_pulses, 10, log=True )   
#plt.show()

