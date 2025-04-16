import math


if __name__ == "__main__":
   #if len(sys.argv) > 1:   
   #    = sys.argv[1]


   sefd_i = 7930 # Jy
   delta_time = (1.08*64*14)/1e6 # sec
   delta_freq = 31.25*1e6 # Hz
   
   sigma = sefd_i / math.sqrt( delta_time*delta_freq )
   
   print("Sigma noise = %.6f Jy" % (sigma))
   print("5sigma = %.6f Jy" % (sigma*5.0))   
   print("10sigma = %.6f Jy" % (sigma*10.0))   
   