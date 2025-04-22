import math

const_ms=52542.00
# d_pc=1.0 # distance to screen
d_pc=2.00 # kpc distance to PSR 
# a_pc=0.0017  # size/width of the screen
a_pc=1e-9 # kpc as from delta_DM fit ....

delta_dm=0.02
# delta_ne=delta_dm/a_pc
delta_ne=0.005

f_mhz=(231.25+200.00)/2.00

tau_s = const_ms*(delta_ne*delta_ne)*(d_pc/a_pc)*d_pc*(1.00/(f_mhz*f_mhz*f_mhz*f_mhz))


print("tau_s = %.8f [ms]" % (tau_s))


