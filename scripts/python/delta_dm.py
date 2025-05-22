import math
import sys

# using mean_dt_dm see : page 1 in 20250522_simple_DM_modelling.odt
# cd ~/Desktop/SKA/papers/2024/EDA2_FRBs/PAPER/DM_vs_TIME
# .x fit_dm_variations_gaussint_PAPER_5par_SIMPLE.C("DM_vs_UX.txt")

d_dm=0.01034637 # pc as from Figure 11 in the paper 
if len(sys.argv) > 1:
   d_dm = float( sys.argv[1] )
   
v0=120 # km/s 
if len(sys.argv) > 2:
   v0 = float( sys.argv[2] )

dt = 80 # in days
if len(sys.argv) > 2:
   dt = float( sys.argv[3] )


pc=3.0857*1e13 # km
pc_m = pc*1000.00 # meters 
pc_cm = pc_m*100.00 # cm
v0_pc = v0/pc


dl = v0_pc*dt*86400
ne = d_dm/dl


print("dL = %e [pc]" % (dl))
print("ne = %e e-/cm^3" % (ne))

# FROM FIT :
print("Parameters based on the fit:")
T_sigma = 12.46*86400
sigma_blob = v0_pc*T_sigma
delta_DM = 0.027
ne_peak = delta_DM/(math.sqrt(2*math.pi)*sigma_blob)

print("Sigma_blob = %e [pc]" % (sigma_blob))
print("ne_peak    = %e [e-/cm^3]" % (ne_peak))

   
   
   


