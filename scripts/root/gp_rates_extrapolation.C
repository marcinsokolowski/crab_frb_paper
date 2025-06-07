
void gp_rates_extrapolation( double l=1e38 )
{
   double n_ref = 5208;
   double alpha = -3.004;
   double l0 = 7*1e23;

   double hour_rate = n_ref*TMath::Power( (l/l0) , alpha );

   double year_in_hours = 365*24;
   double wait_time_for_one_hours = 1.00 / hour_rate;
   double wait_time_for_one_years = wait_time_for_one_hours/year_in_hours;

   double hubble_time = 14*1e9; // 14 billion years 

   printf("Hour rate for l = %e [erg/Hz/s] = %e [GPs/hour]\n",l,hour_rate);
   printf("1 year in hours = %.8f\n",year_in_hours);
   printf("Wait time = %e years vs. Hubble time = %e years (in hours = %e)\n",wait_time_for_one_years,hubble_time,wait_time_for_one_hours);

}