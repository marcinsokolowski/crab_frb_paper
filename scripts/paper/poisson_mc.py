# https://numpy.org/doc/2.2/reference/random/generated/numpy.random.poisson.html

import numpy as np
import matplotlib.pyplot as plt

s = np.random.poisson(5, 10000)
count, bins, ignored = plt.hist(s, 14, density=True)

plt.show()
