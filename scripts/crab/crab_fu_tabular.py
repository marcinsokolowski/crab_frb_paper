import time
import sys
import math

# ~/Desktop/EDA2/papers/2024/EDA2_FRBs/Crab_period
# awk '{printf("%.8f,",$1);}' fu_vs_time_last10.txt
# awk '{printf("%.8f,",$3);}' fu_vs_time_last10.txt
uxtimes=[1713139200.00000000,1715731200.00000000,1718409600.00000000,1721001600.00000000,1723680000.00000000,1726358400.00000000,1728950400.00000000,1731628800.00000000,1734220800.00000000,1736899200.00000000]
fus=[29.56202062,29.56107026,29.56008827,29.55913804,29.55815625,29.55717451,29.55622452,29.55524293,29.55429310,29.55331170]
n=len(uxtimes)

def getfu( ux ) :
   best_i = -1
   best_ux=-1
   best_distance=1e20
   for i in range(0,n-1):
      distance = math.fabs(ux - uxtimes[i])
      if distance < best_distance :
         best_distance = distance
         best_ux = uxtimes[i]
         best_i = i
         
   return best_i     

ux = time.time()
if len(sys.argv) > 1 and sys.argv[1] != "-" :
   ux = float(sys.argv[1])

period=False
if len(sys.argv) > 2 :
   period = ( int(sys.argv[2]) > 0 )

if ux < uxtimes[0] or ux>uxtimes[n-1] :
   print("ERROR : unixtime outside range")
   sys.exit(-1)

best_i = getfu(ux)
best_ux = uxtimes[best_i]
fu = fus[best_i]

if period :
   print("%.20f" % (1.00/fu))
else :
   print("%.20f" % (fu))
  