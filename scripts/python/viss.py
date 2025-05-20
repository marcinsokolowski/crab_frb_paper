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


# solve velocity equation :
v_crab = 120 # km/s 
v_earth = 30 # km/s
v_screen = 15 # km/s - just a test/guess value

x = (viss_kmpersec - (v_earth-v_screen)) / (v_crab - v_screen)

print("x = %.4f" % (x))

# x = Dos/Dps -> (1+x)Dps = D = l 
Dps = l/(1+x)
Dos = Dps*x

print("Distance pulsar-screen Dps = %.4f pc" % (Dps))
print("Distance observer-screen Dos = %.4f pc" % (Dos))

