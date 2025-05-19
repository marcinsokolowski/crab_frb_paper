import math
import sys

l=2000 # pc
if len(sys.argv) > 1:
   l = float( sys.argv[1] )


# theta_arcsec=0.00965971 # arcsec ~9.6 mas
theta_arcsec2=0.0009331
theta_arcsec=math.sqrt(theta_arcsec2)
theta_rad=theta_arcsec/206265.00
print("theta = %.8f [arcsec] = %e [rad]" % (theta_arcsec,theta_rad))
dt=75*86400 # /2.00 # has to be ~ Half-Peak (see ACF analysis)
pc=3.0857*1e13 # km


viss_kmpersec = (theta_rad*l/dt)*pc


print("V_iss = %.4f km/s" % (viss_kmpersec))
