import math

const_ms=52542.00
d_pc=1.0 # distance to screen
a_pc=0.0017  # size/width of the screen

delta_dm=0.02
delta_ne=delta_dm/a_pc

f_mhz=(231.25+200.00)/2.00

tau_s = const_ms*(delta_ne*delta_ne)*(d_pc/a_pc)*d_pc*(1.00/(f_mhz*f_mhz*f_mhz*f_mhz))


print("tau_s = %.8f [ms]" % (tau_s))


