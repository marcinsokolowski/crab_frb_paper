
// see /home/msok/Desktop/EDA2/papers/2024/EDA2_FRBs/20250531_number_of_galaxies.odt
void gp_rates_extrapolation( double l=1e38, int n_crabs_per_galaxy=1, double n_galaxies=1e8 )
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


   // z <= 0.1 universe :
   double wait_time_from_local_universe = wait_time_for_one_years/(n_galaxies*n_crabs_per_galaxy);
   double wait_time_from_local_universe_hours = wait_time_from_local_universe*(365.00*24.00);
   double wait_time_from_local_universe_months = wait_time_from_local_universe*12;
   printf("Wait times in the local universe (z<0.1) = %e years = %.4f hours = %.4f months\n",wait_time_from_local_universe,wait_time_from_local_universe_hours,wait_time_from_local_universe_months);

}