import numpy as np
from astropy.time import Time
import sys

mjd=0
if len(sys.argv) > 1:
   mjd = float( sys.argv[1] )

times = [mjd]
t = Time(times, format='mjd')
print("%.8f" % (t.to_value('unix')[0]))

