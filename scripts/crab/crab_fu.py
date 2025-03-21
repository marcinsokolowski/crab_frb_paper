import time
import sys

# fit paramters from running :
# ~/github/crab_frb_paper/scripts/crab/crab_period_vs_time.sh
# starting from : 1497484800.00000000 43200 29.64135341 0.00000000000000300000
# ~/Desktop/SKA/papers/2024/EDA2_FRBs/Crab_period
# .x plot_fu_vs_time_error.C("fu_vs_time_last.txt")
# a = -0.00000000036771662859
# b = 29.64130320431149456795
a = -0.00000000036771663202
b = 29.64130320452460054526

# .x plot_fu_vs_time_error.C("fu_vs_time_last10.txt")
# a = -0.00000000036654568348 
# b = 29.56202015361811064054
# .x plot_fu_vs_time_error.C("fu_vs_time_last200.txt")
# a = -0.00000000036654568348 
# b = 29.56202015361811064054

ux0=1497484800.00000000

ux = time.time()
if len(sys.argv) > 1 and sys.argv[1] != "-" :
   ux = float(sys.argv[1])

period=False
if len(sys.argv) > 2 :
   period = ( int(sys.argv[2]) > 0 )

fu=b + a*(ux-ux0)

if period :
   print("%.20f" % (1.00/fu))
else :
   print("%.20f" % (fu))
   