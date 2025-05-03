import math

v_crab = 120000 # m/s 
v_crab_pc = 3.36e-7 # pc/day

duration = 75.00 # days 

v0 = v_crab_pc
print("L = 1pc -> v_crab = 120km/s :")
size = v0*duration
print("Size = %e [pc]" % (size))

print("\n\n")

v0 = v_crab_pc*0.5
print("L = 1000pc -> v_crab = 60km/s :")
size = v0*duration
print("Size = %e [pc]" % (size))


