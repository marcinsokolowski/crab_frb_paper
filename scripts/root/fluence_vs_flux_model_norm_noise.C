// always verify line :
// gROOT->Reset();
#include <TROOT.h>
#include <TF1.h>
#include <TH1F.h>
#include <TPaveText.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TMath.h>
#include <TComplex.h>
#include <TFile.h>
#include <TRandom.h>


double gSEFD = 7930.00; // Jy 
double gTimeResolution = (1.08*64*14)/1000000.00;
double gBW = 29629629.00; // Hz 

double sigma_noise( double sefd, double delta_time, double delta_freq, int n_pol=1 )
{
   double sigma_n = sefd/sqrt(n_pol*delta_time*delta_freq);

   return sigma_n;
}

double gFluenceRef = 1000.00;

Double_t Fluence_distrib_powerlaw( Double_t* x, Double_t* y )
{
   Double_t fluence = x[0];

   return y[0]*TMath::Power( (fluence/gFluenceRef) , y[1] );
}

Double_t Pulse_with_gauss_onset( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
   Double_t t_peak = y[1];
   Double_t peak_flux = y[2];
   Double_t sigma = y[3];
   Double_t tau = y[4];


   double flux = offset;
   double norm = peak_flux*(1.00/sqrt(sigma*2*TMath::Pi()));
   double gaussian = norm*exp(-0.5*(t - t_peak)*(t - t_peak)/(sigma*sigma) );

   if( t < t_peak ){
      flux = gaussian;
   }else{
      // exponential decay
      flux = norm*exp( - (t-t_peak) / tau );
   }

   return flux + offset;
}

Double_t Pulse_with_gauss_onset_NORM( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
   Double_t t_peak = y[1];
   Double_t peak_flux = y[2];
   Double_t sigma = y[3];
   Double_t tau = y[4];


   double flux = offset;
   double norm = peak_flux*(1.00/sqrt(sigma*2*TMath::Pi()));
//   double norm = 1.00; // have just no normalisation here at all - but the numbers for peak flux are too high then ???
   double gaussian = norm*exp(-0.5*(t - t_peak)*(t - t_peak)/(sigma*sigma) );

   if( t < t_peak ){
      flux = gaussian;
   }else{
      // exponential decay
      flux = norm*exp( - (t-t_peak) / tau );
   }

   return flux + offset;
}

double calc_max( TF1* pulse_norm, double t0, double t1 )
{
   double max = -1e20, t_max = -1e20;
   double dt = 1e-6;
   double t = t0;
   while( t <= t1 ){
      double val = pulse_norm->Eval(t);

      if( val > max ){
         max = val;
         t_max = t;
      }

      t += dt;
   }

//   printf("# DEBUG : maximum %.6f [Jy] found at time = %.8f [sec]\n",max,t_max);

   return max;
}

double calc_max_real( TF1* pulse_norm, double sigma_n, double t0, double t1 )
{
   double max = -1e20, t_max = -1e20;
   double dt = gTimeResolution; // 1e-6;
   double t = t0;
   while( t <= t1 ){
      double val = pulse_norm->Eval(t);
      double noise = gRandom->Gaus( 0.00, sigma_n );
      val = val + noise;

      if( val > max ){
         max = val;
         t_max = t;
      }

      t += dt;
   }

//   printf("# DEBUG : maximum %.6f [Jy] found at time = %.8f [sec]\n",max,t_max);

   return max;
}



// count_gps( fluence_distrib, F_min, fluence_bin );
double count_gps( TF1* fluence_distrib, double F_min, double fluence_bin )
{
   double N_gp = 0;
   double F = F_min;
   while( F < 10000 ){
      double F_c = F + fluence_bin/2.00;
      double n = fluence_distrib->Eval( F_c );
      N_gp += n;
//      printf("DEBUG count_gps : + %.8f = %.8f\n",n,N_gp);

      F += fluence_bin;
   }

   return N_gp;
}

double count_gps_with_noise( TF1* fluence_distrib, double F_min, double fluence_bin, double sigma_n, double snr_threshold, Double_t* par )
{
   double N_gp = 0;
   double N_gp_nonoise=0;

   double F = F_min;

   Double_t par_local[5];
   par_local[0] = par[0];
   par_local[1] = par[1];
   par_local[2] = par[2];
   par_local[3] = par[3];
   par_local[4] = par[4];

  
   while( F < 10000 ){
      double F_c = F + fluence_bin/2.00;
      double n_double = fluence_distrib->Eval( F_c );
      int n = int(round(n_double));
  
      // calculate peak flux density for a given value of fluence F_c 
      TF1* pulse_no_norm = new TF1("Pulse_with_gauss_onset_NONORM",Pulse_with_gauss_onset_NORM,-0.02,0.02,5);
      par_local[2] = 1.00;
      pulse_no_norm->SetParameters(par_local);
      double F_test = pulse_no_norm->Integral( -0.02, +0.02, 1e-10  )*1000.00;
      double N = F_c / F_test;
      par_local[2] = N;
      pulse_no_norm->SetParameters(par_local);
      F_test = pulse_no_norm->Integral( -0.02, +0.02, 1e-10 )*1000.00;
       if( fabs(F_test-F_c) >= 0.001 ){
         printf("ERROR : in function count_gps_with_noise fluence F_c = %.6f [Jy ms] != %.6f [Jy ms] - while they should be the same after re-normalisation\n",F_c,F_test);
         return -1;
      }
//      printf("DEBUG : F_c = %.8f vs. F_test = %.8f [Jy ms]\n",F_c,F_test);

      // apply effect of noise to detection :
      for(int i=0;i<n;i++){
           double peak_flux = calc_max_real( pulse_no_norm,  sigma_n, -0.02, +0.02 );

//           double peak_flux = calc_max( pulse_no_norm, -0.02, +0.02 );
//           double noise = gRandom->Gaus( 0.00, sigma_n );  
//           peak_flux += noise;

         double snr = peak_flux / sigma_n;
         if( snr >= snr_threshold ){
            N_gp = N_gp + double(1.00); // only those added which satisfy condition SNR>=THRESHOLD (DETECTION IN THE PRESENCE OF NOISE SIMULATED);
         }
      }

      N_gp_nonoise = N_gp_nonoise + n_double; // all detected
//      N_gp = N_gp + double(n);

//      printf("DEBUG count_gps : + %.8f = %.8f\n",n,N_gp);

      F += fluence_bin;
   }

   printf("DEBUG : N_gp = %.1f vs. N_gp_nonoise = %.1f\n",N_gp,N_gp_nonoise);

   return N_gp;
}




void fluence_vs_flux_model_norm_noise( double F0 = 8000, double noise_multiplier=1.00 ) // fluence is constant in Jy*ms as in Figure 7 in the paper at rate 1-GP/hour :
{
   double sigma_n = sigma_noise( gSEFD, gTimeResolution, gBW , 1 )*noise_multiplier; // noise_multiplier is here in case I underestimate the noise

   printf("Noise in data = %.6f [Jy]\n",sigma_n);

   Double_t par[5];

   TCanvas* c1 = new TCanvas("c1","c1",200,10,2000,1000);
   c1->SetLogx(1);
   c1->SetLogy(1);
   c1->cd();

   // TODO : F0 must enter here too !!! - re-normalise so that F0 has 1/hour rate
   double fluence_bin = (5000-90)/100.00; 
   TF1* fluence_distrib0 = new TF1("Fluence_distrib_powerlaw0",Fluence_distrib_powerlaw,90,5000,2);
   // page 1 in : /home/msok/Desktop/EDA2/papers/2024/EDA2_FRBs/20250401_modelling_number_of_GPs_REPEAT.odt
   /* FCN=98.3902 FROM MINOS     STATUS=SUCCESSFUL     21 CALLS         181 TOTAL
                     EDM=4.5615e-07    STRATEGY= 1      ERROR MATRIX ACCURATE 
      EXT PARAMETER                  PARABOLIC         MINOS ERRORS        
       NO.   NAME      VALUE            ERROR      NEGATIVE      POSITIVE   
      1  p0           3.05335e+01   8.31785e-01  -8.21527e-01   8.42031e-01
      2  p1          -2.97932e+00   3.41557e-02  -3.43199e-02   3.39862e-02
   */
   par[0] = 30.5335; // was 18.00;
   par[1] = -2.97932; // was -3.132;
//   par[1] = -3.5;
   fluence_distrib0->SetParameters(par); 
   fluence_distrib0->Draw();   

   // re-normalise to make F0 a fluence with 1 / hour rate:
   double F_1hour = gFluenceRef*TMath::Power( (1.00/par[0]) , (1.00/par[1]) );
   printf("DEBUG : 1 hour fluence (fluence with rate 1 GP / hour) = %.6f [Jy ms]\n",F_1hour);
   par[0] = 1.00/(TMath::Power( (F0/gFluenceRef) , par[1] ));
   printf("DEBUG : par[0] := %.8f\n",par[0]);
   TF1* fluence_distrib = new TF1("Fluence_distrib_powerlaw",Fluence_distrib_powerlaw,90,5000,2);
   fluence_distrib->SetParameters(par); 
   fluence_distrib->Draw("same");   
   fluence_distrib->SetLineColor(kBlack);
   F_1hour = gFluenceRef*TMath::Power( (1.00/par[0]) , (1.00/par[1]) );
   printf("DEBUG : 1 hour fluence (fluence with rate 1 GP / hour) = %.6f [Jy ms] (after re-normalisation)\n",F_1hour);


   

   TCanvas* c2 = new TCanvas("c2","c2",200,10,2000,1000);
//   c1->SetLogx(1);
//   c1->SetLogy(1);
   c2->cd();


//   TF1* pulse = new TF1("Pulse_with_gauss_onset",Pulse_with_gauss_onset,-0.02,0.02,5);
   TF1* pulse = new TF1("Pulse_with_gauss_onset_NORM",Pulse_with_gauss_onset_NORM,-0.02,0.02,5);
/* for norm=1 :  
   par[0] = 0.000;
   par[1] = 0.000;
   par[2] = 500.00;
   par[3] = 0.0008;
   par[4] = 0.004;*/

   // norm!=1 :
   // see page 2 in /home/msok/Desktop/EDA2/papers/2024/EDA2_FRBs/20250424_single_pulse_model_for_Ngp_vs_Tau_modelling.odt
   par[0] = 0.00; // we do not need offset : 95.32519547;
   par[1] = 0.00; // original fit here 4230.48637346 but we need pulse to beat time = 0 
   par[2] = 1469.65451453;
   par[3] = 0.00081161;
   par[4] = 0.00194341;

   pulse->SetParameters(par);
   pulse->Draw();   
   pulse->SetLineColor(kBlack);

   double calka = pulse->Integral( -0.02, +0.02, 1e-10 ); // in Jy seconds because all the time parameters are in seconds !!!   
   double calka_Jyms = calka*1000.00;
   printf("Integral = %.8f [Jy s] = %.8f [Jy ms]\n",calka,calka_Jyms);

   TF1* pulse2 = new TF1("Pulse_with_gauss_onset_NORM2",Pulse_with_gauss_onset_NORM,-0.02,0.02,5);
   par[2] = par[2]*(F0/calka_Jyms);
   pulse2->SetParameters(par);
   pulse2->Draw("same");   
   pulse2->SetLineColor(kGreen);

   calka = pulse2->Integral( -0.02, +0.02, 1e-10 ); // in Jy seconds because all the time parameters are in seconds !!!   
   calka_Jyms = calka*1000.00;
   printf("Renormalised Integral = %.8f [Jy s] = %.8f [Jy ms]\n",calka,calka_Jyms);


// return;

   char szOutFile[128];
   sprintf(szOutFile,"peakflux_vs_tau_%dJyms_noise%.3fJy.txt",int(F0),sigma_n);
   FILE* outf = fopen(szOutFile,"w");
   fprintf(outf,"# Tau[ms]   Peak_flux[Jy]  Integral   Fluence_min[Jy ms] N_gp\n");
   double tau = 0.0005;
   double tau_step = 0.00001*10.00;// *10.00 to get it faster
   double threshold = 5*sigma_n;  
//   sleep(5);

   while( tau <= 0.010 ){
      printf("\n\n\nTau = %.4f [ms]\n",tau*1000.00);

      TF1* pulse_norm = new TF1("Pulse_with_gauss_onset_NORM3",Pulse_with_gauss_onset_NORM,-0.02,0.02,5);
      par[4] = tau;
      pulse_norm->SetParameters(par);
      double calka0 = pulse_norm->Integral( -0.02, +0.02, 1e-10 )*1000.00; // *1000 to make it Jy ms 
      delete pulse_norm;
  
      // re-normalise pulse function to have fluence F0 (constant !!!)
      par[2] = par[2]*(F0/calka0);
      pulse_norm = new TF1("Pulse_with_gauss_onset_NORM4",Pulse_with_gauss_onset_NORM,-0.02,0.02,5);
      pulse_norm->SetParameters(par);
      double calka = pulse_norm->Integral( -0.02, +0.02, 1e-10 )*1000.00; // *1000 to make it Jy ms 
      printf("DEBUG : calka = %.8f [Jy ms] vs. before re-normalisation %.8f  [Jy ms]\n",calka,calka0);
 

      // peak flux corresponding to fluence F0 
      // double peak_flux = F0/calka;  
      // rather find peak flux as the maximum of the pulse_norm curve 
      double peak_flux = calc_max( pulse_norm, -0.02, 0.02 );
     
      // what is the minimum detection threshold corresponding to our peak flux threshold (SNR) SNR >= 5 ?
      // double F_min = threshold*calka;
      // This is done by calculation normalisation at which f_peak = 5sigma
      // F_min is the minimum fluence that can be detected, which corresponds to the peak flux density = Threshold (SNR=peak_flux/sigma_noise >= 5);
      // we just need to find peak_flux_5sigma - peak flux at SNR=5
      // then calculate fluence corresponding to this exact peak flux and
      // count number of GPs with fluence above this threshold !
      // . So set par[2] (norm) = 1 -> 
       
      Double_t par_norm_one[5];
      par_norm_one[0] = par[0];
      par_norm_one[1] = par[1];
      par_norm_one[2] = 1.00;
      par_norm_one[3] = par[3];
      par_norm_one[4] = par[4];
      TF1* pulse_no_norm = new TF1("Pulse_with_gauss_onset_NONORM",Pulse_with_gauss_onset_NORM,-0.02,0.02,5);
      pulse_no_norm->SetParameters(par_norm_one);
      double max_nonorm = calc_max( pulse_no_norm, -0.02, 0.02 );
      printf("MAX_NONORM = %.8f vs. 5*Sigma_noise = %.8f\n",max_nonorm,5.00*sigma_n);
      par_norm_one[2] = 5.00*sigma_n / max_nonorm;
      printf("NEW NORM = %.8f\n",par_norm_one[2]);
      pulse_no_norm->SetParameters(par_norm_one);
      double max_nonorm2 = calc_max( pulse_no_norm, -0.02, 0.02 );
      printf("NEW MAX_NONORM = %.8f vs. 5*Sigma_noise = %.8f\n",max_nonorm2,5.00*sigma_n);

      // calculate miminum detected Fluence (fluence threshold) corresponding to peak_flux/sigma_noise = 5 (i.e. peak flux threshold):
      double F_min = pulse_no_norm->Integral( -0.02, +0.02, 1e-10 )*1000.00;
      printf("DEBUG : F_min = %.8f [Jy ms]\n",F_min);
      
      

//      double N_gp = count_gps( fluence_distrib, F_min, fluence_bin );

      // NEW - including noise :
      // double count_gps_with_noise( TF1* fluence_distrib, double F_min, double fluence_bin, double sigma_n, double snr_threshold, Double_t* par )
      double N_gp = count_gps_with_noise( fluence_distrib, F_min, fluence_bin, sigma_n, 5, par_norm_one);

      printf("Tau = %.6f [ms] : calka = %.6f [Jy ms] -> peak flux = %.8f [Jy] -> N_gp = %.4f\n",tau*1000.00,calka,peak_flux,N_gp);
//      pulse_no_norm->Draw();
// return;
//      break;
      
      fprintf(outf,"%.8f %.8f %.8f %.8f %.8f\n",tau*1000.00,peak_flux,calka,F_min,N_gp);
      tau += tau_step;

      delete pulse_norm;
   }

   fclose(outf);
   printf("N_gps vs. Tau saved to file %s\n",szOutFile);
}

