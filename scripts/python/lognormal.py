import numpy as np
from scipy.stats import lognorm
import matplotlib.pyplot as plt

# Generate log-normally distributed data
s = 0.5  # Shape parameter
scale = np.exp(2) # Scale parameter (e^mu)
data = lognorm.rvs(s, scale=scale, size=1000)

# Option 1: Equal-width bins
num_bins = 50
bins_equal = np.linspace(data.min(), data.max(), num_bins + 1)

# Option 2: Logarithmically spaced bins (useful for log-normal data)
# Transform data to log-scale first
log_data = np.log(data)
num_log_bins = 30
bins_log = np.exp(np.linspace(log_data.min(), log_data.max(), num_log_bins + 1))

plt.figure(figsize=(10, 5))
plt.hist(data, bins=bins_equal, edgecolor='black', alpha=0.7)
plt.title('Histogram of Log-Normal Data (Equal-Width Bins)')
plt.xlabel('Value')
plt.ylabel('Frequency')
plt.grid(True)
plt.show()

plt.figure(figsize=(10, 5))
plt.hist(data, bins=bins_log, edgecolor='black', alpha=0.7)
plt.title('Histogram of Log-Normal Data (Logarithmically Spaced Bins)')
plt.xlabel('Value')
plt.ylabel('Frequency')
plt.xscale('log') # Set x-axis to log scale for better visualization with log bins
plt.grid(True)
plt.show()
