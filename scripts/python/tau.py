import sys
import math

c = 3e8 # m/s
pc2m = 3.0857e16 # 1pc in m
kpc = 3.0857e19 # 1kpc in m
D = 2*3.0857e19 # distance to crab pulsar = 2kpc

arcsec2rad = (1.00/3600.00)*(math.pi/180.00)

if __name__ == "__main__":
  l_pc = 2000.00 - 1.00;
  if len(sys.argv) > 1 and sys.argv[1] != "-" :   
    l_pc = float(sys.argv[1])
    
  l_m = l_pc*pc2m  
    
  theta_rms_arcsec = 2.00
  if len(sys.argv) > 2:
     theta_rms_arcsec = float(sys.argv[2])
     
  theta_rms_rad = theta_rms_arcsec*arcsec2rad     
  
  print("###########################################################")
  print("PARAMETERS:")
  print("###########################################################")
  print("l_pc = %.6f [ps]" % (l_pc))
  print("Theta_rms = %.6f [arcsec]" % (theta_rms_arcsec))
  print("###########################################################")
   
   
  tau = l_m*(theta_rms_rad*theta_rms_rad)/(2.00*c)
  tau_ms = tau*1000.00
  
  tau_ms_new = 1.20879*l_pc*(theta_rms_arcsec*theta_rms_arcsec)
  
  print("tau = %e [sec] = %.4f [ms]" % (tau,tau_ms))  
  print("NEW                      = %.4f [ms]" % (tau_ms_new))