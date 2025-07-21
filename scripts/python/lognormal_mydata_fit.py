import numpy as np
from scipy.stats import lognorm
import matplotlib.pyplot as plt
import math
from scipy.optimize import curve_fit

def gaussian(x, amplitude, mean, sigma):
   return amplitude * np.exp(-(x - mean)**2 / (2 * sigma**2))


def read_data_file( filename="all_time_diff_snr10.txt", errors=True ) :
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
# WARNING : NOT THIS HERE - see below there is log_data = np.log(data) there !                  
#         x_arr.append( math.log10(x) )
         x_arr.append(x) 
         cnt += 1

         if x > max_x :
            max_x = x
         if x < min_x :
            min_x = x

         print("DEBUG : %.4f" % (x))   

#   print("DEBUG : returning with errors : len() = %d, %d, %d, %d" % (len(x_arr),len(x_err_arr),len(y_arr),len(y_err_arr)))
   print("DEBUG : returning array of %d values min = %.8f , max = %.8f" % (len(x_arr),min_x,max_x))
   return (np.array(x_arr),min_x,max_x)

(data,min_x,max_x) = read_data_file("all_time_diff_snr10.txt")

# Generate log-normally distributed data
# s = 0.5  # Shape parameter
# scale = np.exp(2) # Scale parameter (e^mu)
# data = lognorm.rvs(s, scale=scale, size=1000)

# Option 1: Equal-width bins
num_bins = 50
bins_equal = np.linspace(data.min(), data.max(), num_bins + 1)

# Option 2: Logarithmically spaced bins (useful for log-normal data)
# Transform data to log-scale first
log_data = np.log10(data)
num_log_bins = 30
# bins_log = np.exp(np.linspace(log_data.min(), log_data.max(), num_log_bins + 1))
# bins_log = np.linspace(log_data.min(), log_data.max(), num_log_bins + 1)
bins_log = np.linspace(-2,3,num_log_bins + 1)


plt.figure(figsize=(10, 10))
plt.hist(log_data, bins=bins_equal, edgecolor='black', alpha=0.7)
plt.title('Histogram of Log-Normal Data (Equal-Width Bins)')
plt.xlabel('Wait Times [sec]')
plt.ylabel('Count')
# plt.xscale('log')
# plt.yscale('log')
plt.grid(True)
plt.show()

plt.figure(figsize=(10, 10))
counts, bin_edges, _ = plt.hist(log_data, bins=bins_log, edgecolor='black', alpha=0.7)
# 3. Calculate bin centers
bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2
# 4. Calculate error values (e.g., Poisson error for counts)
# For unweighted histograms, statistical error is often sqrt(counts)
errors = np.sqrt(counts)

# 5. Plot error bars
plt.errorbar(bin_centers, counts, yerr=errors, fmt='o', color='black', capsize=3, label='Error Bars')

# plt.title('Histogram of Log-Normal Data (Logarithmically Spaced Bins)')
plt.title('Histogram of $Log_{10}$( Wait times $\Delta$T )')
plt.xlabel('Wait Times [$Log_{10}$(seconds)]')
plt.ylabel('Count')

dofit=True
# Perform the fit
if dofit :
   # Initial guesses for amplitude, mean, and sigma
   # max(counts) as amplitude, mean of data as mean, std of data as sigma
   p0 = [np.max(counts), np.mean(log_data), np.std(log_data)]

   params, covariance = curve_fit(gaussian, bin_centers, counts, p0=p0)
   params_err = np.sqrt(np.diag(covariance)) # Standard errors of the parameters
   amplitude_fit, mean_fit, sigma_fit = params
   plt.plot(bin_centers, gaussian(bin_centers, *params), color='red', label='Gaussian Fit')
   
   a=10
   param_text = f"Normalisation = {params[0]:.2f} ± {params_err[0]:.2f}\n" \
                f"Mean($Log_{{10}}$( $\Delta$ T ) = {params[1]:.2f} ± {params_err[1]:.2f}\n" \
                f"$\sigma$ = {params[2]:.2f} ± {params_err[2]:.2f}"
   plt.text(0.05, 0.95, param_text, transform=plt.gca().transAxes, fontsize=15, verticalalignment='top', bbox=dict(boxstyle='round,pad=0.5', fc='wheat', alpha=0.5))
   
# plt.xscale('log') # Set x-axis to log scale for better visualization with log bins
plt.grid(True)
plt.show()
