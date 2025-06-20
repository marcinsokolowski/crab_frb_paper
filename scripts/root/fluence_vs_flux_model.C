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


double gSEFD = 7930.00; // Jy 
double gTimeResolution = (1.08*64*14)/1000000.00;
double gBW = 29629629.00; // Hz 

double sigma_noise( double sefd, double delta_time, double delta_freq, int n_pol=1 )
{
   double sigma_n = sefd/sqrt(n_pol*delta_time*delta_freq);

   return sigma_n;
}

Double_t Fluence_distrib_powerlaw( Double_t* x, Double_t* y )
{
   Double_t fluence = x[0];

   return y[0]*TMath::Power( (fluence/1000.00) , y[1] );
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

Double_t Pulse_with_gauss_onset_NONORM( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
   Double_t t_peak = y[1];
   Double_t peak_flux = y[2];
   Double_t sigma = y[3];
   Double_t tau = y[4];


   double flux = offset;
//   double norm = (1.00/sqrt(sigma*2*TMath::Pi()));
   double norm = 1.00; // have just no normalisation here at all - but the numbers for peak flux are too high then ???
   double gaussian = norm*exp(-0.5*(t - t_peak)*(t - t_peak)/(sigma*sigma) );

   if( t < t_peak ){
      flux = gaussian;
   }else{
      // exponential decay
      flux = norm*exp( - (t-t_peak) / tau );
   }

   return flux + offset;
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

void fluence_vs_flux_model()
{
   double sigma_n = sigma_noise( gSEFD, gTimeResolution, gBW , 1 );

   printf("Noise in data = %.6f [Jy]\n",sigma_n);

   Double_t par[5];

   TCanvas* c1 = new TCanvas("c1","c1",200,10,2000,1000);
   c1->SetLogx(1);
   c1->SetLogy(1);
   c1->cd();

   double fluence_bin = (5000-90)/100.00; 
   TF1* fluence_distrib = new TF1("Fluence_distrib_powerlaw",Fluence_distrib_powerlaw,90,5000,2);
   par[0] = 18.00;
   par[1] = -3.132;
   fluence_distrib->SetParameters(par);
   fluence_distrib->Draw();   

   TCanvas* c2 = new TCanvas("c2","c2",200,10,2000,1000);
//   c1->SetLogx(1);
//   c1->SetLogy(1);
   c2->cd();


//   TF1* pulse = new TF1("Pulse_with_gauss_onset",Pulse_with_gauss_onset,-0.02,0.02,5);
   TF1* pulse = new TF1("Pulse_with_gauss_onset_NONORM",Pulse_with_gauss_onset_NONORM,-0.02,0.02,5);
   par[0] = 0.000;
   par[1] = 0.000;
   par[2] = 500.00;
   par[3] = 0.0008;
   par[4] = 0.004;

   pulse->SetParameters(par);
   pulse->Draw();   

   double calka = pulse->Integral( -0.02, +0.02, 1e-10 ); // in Jy seconds because all the time parameters are in seconds !!!   
   double calka_Jyms = calka*1000.00;
   printf("Integral = %.8f [Jy s] = %.8f [Jy ms]\n",calka,calka_Jyms);

   FILE* outf = fopen("peakflux_vs_tau_1000Jyms.txt","w");
   fprintf(outf,"# Tau[ms]   Peak_flux[Jy]  Integral   Fluence_min[Jy ms] N_gp\n");
   double F0 = 1000; // fluence is constant in Jy*ms 
   double tau = 0.0005;
   double tau_step = 0.00001;
   double threshold = 5*sigma_n;  

   while( tau <= 0.010 ){
      TF1* pulse_nonorm = new TF1("Pulse_with_gauss_onset_NONORM",Pulse_with_gauss_onset_NONORM,-0.02,0.02,5);
      par[4] = tau;
      pulse_nonorm->SetParameters(par);
      double calka = pulse_nonorm->Integral( -0.02, +0.02, 1e-10 )*1000.00; // *1000 to make it Jy ms 
      double peak_flux = F0/calka;       
      double F_min = threshold*calka;

      double N_gp = count_gps( fluence_distrib, F_min, fluence_bin );

      printf("Tau = %.6f [ms] : calka = %.6f [Jy ms] -> peak flux = %.8f [Jy] -> N_gp = %.4f\n",tau*1000.00,calka,peak_flux,N_gp);
//      pulse_nonorm->Draw();
//      break;
      
      fprintf(outf,"%.8f %.8f %.8f %.8f %.8f\n",tau*1000.00,peak_flux,calka,F_min,N_gp);
      tau += tau_step;
   }

   fclose(outf);
}

