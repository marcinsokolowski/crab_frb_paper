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


Double_t tau_func( Double_t* x, Double_t* y )
{
   Double_t a_kpc = x[0]/1000.00;
   Double_t d_kpc = y[0];

   Double_t const_ms=52542.00;

//   d_pc=1.0; // distance to screen
//   a_pc=0.0017; // size/width of the screen

   Double_t delta_dm=0.02;
   Double_t delta_ne=delta_dm/a_kpc;

   Double_t f_mhz=(231.25+200.00)/2.00;

   Double_t tau_s = const_ms*(delta_ne*delta_ne)*(d_kpc/a_kpc)*d_kpc*(1.00/(f_mhz*f_mhz*f_mhz*f_mhz));

   return tau_s;
}

void tau_theo()
{
   Double_t par[5];

   TCanvas* c1 = new TCanvas("c1","c1",200,10,2000,1000);
   c1->SetLogx(1);
   c1->SetLogy(1);
   c1->cd();
   TLegend *legend = new TLegend(.4,0.75,0.9,0.85);
   legend->SetTextFont(72);
   legend->SetTextSize(0.02);

   double d_kpc = 0.1;
   double delta_d_kpc = 0.1;
   double d_crab = 2.00;

   int line_type = 1;
   int colors[10] = { kBlack,kRed,kCyan,kBlue,kGreen,kOrange,kYellow,kGray,kPink,kMagenta };

   int i=0;
   int color_index=0;
   while( d_kpc <= d_crab ){
      char szFuncName[64];
      sprintf(szFuncName,"d_kpc_%.4f_kpc",d_kpc);
      TF1* pTauFunc = new TF1(szFuncName,tau_func,1,20.00,1);
      par[0] = d_kpc;
      pTauFunc->SetParameters(par);

      if( i==0 ){
         pTauFunc->Draw();
      }else{
         pTauFunc->Draw("same");   
      }
      pTauFunc->SetLineColor(colors[color_index]);
      pTauFunc->SetLineStyle(line_type);

      if( i==0 ){
         pTauFunc->SetMinimum(0.01);
         pTauFunc->SetMaximum(7.00);
         pTauFunc->GetHistogram()->SetXTitle( "a [pc]" );
         pTauFunc->GetHistogram()->SetYTitle( "Scatter broadening time #Tau [ms]" );
      }
      char szLineDesc[64];
      sprintf(szLineDesc,"d = %.2f [pc]",d_kpc);
      legend->AddEntry( pTauFunc , szLineDesc, "L" );

      color_index++;
      if( color_index >= 10 ){
         line_type++;
         color_index=0;
      }
      i++;
      d_kpc += delta_d_kpc;
   }

 
   legend->Draw();
}

