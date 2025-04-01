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


void fluence_vs_flux_model()
{
   double sigma_n = sigma_noise( gSEFD, gTimeResolution, gBW , 1 );

   printf("Noise in data = %.6f [Jy]\n",sigma_n);

//   TF1* pulse = new TF1("Pulse_with_gauss_onset",Pulse_with_gauss_onset,-0.02,0.02,5);
   TF1* pulse = new TF1("Pulse_with_gauss_onset_NONORM",Pulse_with_gauss_onset_NONORM,-0.02,0.02,5);
   Double_t par[5];
   par[0] = 0.000;
   par[1] = 0.000;
   par[2] = 500.00;
   par[3] = 0.0008;
   par[4] = 0.004;

   pulse->SetParameters(par);
   pulse->Draw();   

   double calka = pulse->Integral( -0.02, +0.02, 1e-10 );
   printf("Integral = %.8f [Jy ms]\n",calka);

   FILE* outf = fopen("peakflux_vs_tau_30Jyms.txt","w");
   double F0 = 30; // fluence is constant 
   double tau = 0.0005;
   double tau_step = 0.00001;
   double threshold = 5*sigma_n;  

   while( tau <= 0.010 ){
      TF1* pulse_nonorm = new TF1("Pulse_with_gauss_onset_NONORM",Pulse_with_gauss_onset_NONORM,-0.02,0.02,5);
      par[4] = tau;
      pulse_nonorm->SetParameters(par);
      double calka = pulse_nonorm->Integral( -0.02, +0.02, 1e-10 );
      double peak_flux = F0/calka;       
      double F_min = threshold*calka;

      printf("Tau = %.6f [ms] : calka = %.6f [Jy ms] -> peak flux = %.8f [Jy]\n",tau*1000.00,calka,peak_flux);
//      pulse_nonorm->Draw();
//      break;
      
      fprintf(outf,"%.8f %.8f %.8f %.8f\n",tau*1000.00,peak_flux,calka,F_min);
      tau += tau_step;
   }

   fclose(outf);
}

