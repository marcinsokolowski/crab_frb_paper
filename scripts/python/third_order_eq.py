import math
import cmath
import sys
import numpy

def solve_cubic_equation(a, b, c, d):
    delta = (18*a*b*c*d) - (4*(b**3)*d) + ((b**2)*(c**2)) - (4*a*(c**3)) - (27*(a**2)*(d**2))
    if delta > 0:
        num_roots = 3
    elif delta == 0:
        num_roots = 2
    else:
        num_roots = 1

    if num_roots == 3:
        print("3 roots")
        y1 = (((-b**3) / (27*(a**3))) + ((b*c) / (6*(a**2))) - (d / (2*a))) + cmath.sqrt((((-b**3) / (27*(a**3))) + ((b*c) / (6*(a**2))) - (d / (2*a)))**2 + ((c / (3*a)) - ((b**2) / (9*(a**2))))**3)
        y2 = (((-b**3) / (27*(a**3))) + ((b*c) / (6*(a**2))) - (d / (2*a))) - cmath.sqrt((((-b**3) / (27*(a**3))) + ((b*c) / (6*(a**2))) - (d / (2*a)))**2 + ((c / (3*a)) - ((b**2) / (9*(a**2))))**3)
        
        x1 = (y1**(1/3)) + (y2**(1/3)) - (b / (3*a))
        x2 = ((-1/2)*((y1**(1/3)) + (y2**(1/3)))) - (b / (3*a)) + ((1/2)*1j*cmath.sqrt(3)*((y1**(1/3))-(y2**(1/3))))
        x3 = ((-1/2)*((y1**(1/3)) + (y2**(1/3)))) - (b / (3*a)) - ((1/2)*1j*cmath.sqrt(3)*((y1**(1/3))-(y2**(1/3))))
        return x1, x2, x3

    if num_roots == 2:
        print("2 roots")
        x1 = ((-b + cmath.sqrt(b**2-(4*a*c))) / (2*a))
        x2 = ((-b - cmath.sqrt(b**2-(4*a*c))) / (2*a))
        return x1, x2
    
    if num_roots == 1:
        print("1 root")
        x1 = (-d/c)
        return x1
        
if __name__ == '__main__':        
   t_ms=5.00
   if len(sys.argv) > 1:
      t_ms = float( sys.argv[1] )
   
   t_riss=51 # days
   if len(sys.argv) > 2:
      t_riss = float( sys.argv[2] )

   v_sc=15
   if len(sys.argv) > 3:
      v_sc = float( sys.argv[3] )

   D=2000 # pc
   if len(sys.argv) > 4:
      D = float( sys.argv[4] )
   
   v_psr = 120
   v_e = 30

   alpha = 1574.848324697*math.sqrt(t_ms)/t_riss
   delta1 = v_psr - v_sc
   delta2 = v_e - v_sc

   a = alpha 
   b = (delta1 - delta2)
   c = -alpha*D
   d = D*delta2

   # solve equation : ay^3 + by^2 + cy + d = 0 
   print("a = %.8f , b = %.8f, c = %.8f , d = %.8f" % (a,b,c,d))
   list=[]
   try :
      (x1,x2,x3) = solve_cubic_equation(a, b, c, d)
      print(f"The roots of the cubic equation are:")
      print(f"x1 = {x1}")
      print(f"x2 = {x2}")
      print(f"x3 = {x3}")
      list = (x1,x2,x3)
   except :       
      (x1) = solve_cubic_equation(a, b, c, d)
      list = (x1,)

   print("\n\nPhysical solutions:")   
   good=0
   physical_solutions=[]
   for x in list :   
      x_im = numpy.imag(x)
      x_re = numpy.real(x)   
      d  = 0.00
      if math.fabs(x_im) < 1e-10 and x_re >= 0 :    
         d = x_re*x_re
         x_d = d/(D-d)
         d_km = d*3.0857*1e13 # km
         good = good + 1
         physical_solutions.append(d)
         
         theta_arcsec = 0.909545262*math.sqrt(t_ms/d)
         v_riss = 1731.46779*d*theta_arcsec/t_riss
         
         print("Physical solution %d = %.8f pc = %e [mln km] (x = %.8f) -> theta = %.6f [arcsec] , V_riss = %.8f [km/s]" % (good,d,d_km/1e6,x_d,theta_arcsec,v_riss))
      



   
   

