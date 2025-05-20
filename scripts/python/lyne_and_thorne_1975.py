import math

pc = 3.0857e13 # km
c = 300000 # km/s 
D = 2000.00*3.0857*1e13 # km to be in the same units as c 

def screen_parameters( a, tau0, label ) :
   b = 1.00 - a 

   theta0 = math.sqrt( c * tau0 / (a*b*D) )
   theta0_arcsec = theta0 * 206265.00
   phi0 = 2.0*a*theta0
   phi0_arcsec = 2.0*a*theta0_arcsec
   
   r0_km = math.sqrt(2*D*a*b*c*tau0) 
   r0_pc = r0_km/pc

   print("%s: " % (label))
   print("Theta0 = %.6f [arcsec] = %e [rad] (%s)" % (theta0_arcsec,theta0,label))
   print("Phi0   = %.6f [arcsec] = %e [rad] (%s)" % (phi0_arcsec,phi0,label))
   print("r0     = %e [pc]" % (r0_pc))
   print("")


if __name__ == "__main__":

   # Lyne & Thorne 
   tau_lyne_and_thorne = 0.003 # 3ms
   
   # screen midway between pulsar and earth :
   screen_parameters( 0.5, tau_lyne_and_thorne, "midway" )

   # screen midway between pulsar and earth :
   screen_parameters( 5e-5, tau_lyne_and_thorne, "Crab Nebula" )
 

   tau_msok = 0.005 
   # screen midway between pulsar and earth :
   screen_parameters( 0.5, tau_msok, "midway" )

   # screen midway between pulsar and earth :
   screen_parameters( 5e-5, tau_msok, "Crab Nebula" )
