import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

def gaussian(x, amplitude, mean, sigma):
   return amplitude * np.exp(-(x - mean)**2 / (2 * sigma**2))
   
# Example: Generate some normally distributed data
data = np.random.normal(loc=0, scale=1, size=1000)

# Create histogram and get bin information
counts, bin_edges, _ = plt.hist(data, bins=30, density=False) # density=False for raw counts
bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2   

# Initial guesses for amplitude, mean, and sigma
# max(counts) as amplitude, mean of data as mean, std of data as sigma
p0 = [np.max(counts), np.mean(data), np.std(data)]

# Perform the fit
params, covariance = curve_fit(gaussian, bin_centers, counts, p0=p0)
amplitude_fit, mean_fit, sigma_fit = params

plt.plot(bin_centers, gaussian(bin_centers, *params), color='red', label='Gaussian Fit')
plt.xlabel('Value')
plt.ylabel('Frequency')
plt.title('Histogram with Gaussian Fit')
plt.legend()
plt.show()
