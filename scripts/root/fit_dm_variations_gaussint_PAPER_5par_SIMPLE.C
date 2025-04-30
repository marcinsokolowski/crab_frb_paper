// 
// Script for fitting pulse profile to scattered pulsar profiles like VELA:
// There are two options for the onset of the pulses:
//   - linear (fast up)
//   - gaussian
// 
// this is followed by exponetial decay 
// 

// always verify line :
// gROOT->Reset();
#include <TROOT.h>
#include <TF1.h>
#include <TH1F.h>
#include <TPaveText.h>
#include <TGraph.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <TGraphErrors.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TMath.h>
#include <TComplex.h>
#include <TFile.h>

double gStartX = -1;


double gFinalChi2 = -1.00;
int gLog=0;
int gVerb=0;
int gNormaliseInputData=1;
double gPulsarPeriodMS = 89.328385024;
double gOriginalRMS=1.00;
double gOriginalMean=0.00;
double gSigmaSimulated=1.00;
double gNoiseStart=0.00;
double gNoiseEnd=1e20;
bool   gUseFitResidualsRMS=false;
char   gInputFileName[256];

#define MAX_ROWS 10000000

Double_t Amplitude = 0.00;

// original before any re-scaling 
Double_t gMinX = 1e20;
Double_t gMaxX = -1e20;
Double_t gMinY = 1e20;

Double_t gXaxisNormFactor = 1.00;
Double_t gYaxisNormFactor = 1.00;
Double_t gYaxisOffset = 0.00;
Double_t gXaxisOffset = 0.00;

// fitted parameters re-scaled back to physical units:
double gScatteringTimeTauMS = -1000;
double gPulseWidthMS = -1000;
Double_t gFittedParameters[100];
Double_t gFittedParametersErrors[100];
Double_t gFittedParametersOriginalScaling[100];
Double_t gFittedParametersOriginalScalingErrors[100];
int      gFittedParametersN=5;

// No scattering tau = 0.050
// Scattering tau = 2 seconds ?
Double_t Pulse_with_linear_onset( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
   Double_t t0 = y[1];           // start of the pulse
   Double_t t_peak = y[2];
   Double_t peak_flux = y[3];
   Double_t tau = y[4];

   double flux = offset;
   if( t > t0 ){
      if( t < t_peak ){
         double a = peak_flux / ( t_peak - t0 );
         flux = a*(t - t0 );
      }else{
         // exponential decay
         flux = peak_flux*exp( - (t-t_peak) / tau );
      }
   }


   return flux;

   // FRB171020 , 200 Jy ms @ 1.4 GHz 
   //             1500 Jy ms @ 185 MHz and spectral index  = -1
   //             11400 Jy ms @ 185 MHz and spectral index = -2
   //             7600 Jy ms @ 185 MHz and spectral index  = -1.8
/*   Double_t fluence = 7.60;  // Jy x sec assuming spectral index  = -1.8

   Double_t T   = 5;          // seconds - duration
   Double_t tau = y[1];      // 50ms - WARNING, when <0.050 - the integral value is incorrect !!!
   Amplitude = fluence / tau; // assuming pulse : Amplitude x exp( - t / tau )

   if( t < t0 ){
     return 0;
   }

   double pulse = Amplitude* exp( - (t-t0) / tau );
   return pulse;*/
}


Double_t Pulse_with_gauss_onset( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
//   Double_t t0 = y[1];           // start of the pulse
   Double_t t_peak = y[1];
   Double_t peak_flux = y[2];
   Double_t sigma = y[3];
   Double_t tau = y[4];


   double flux = offset;
//   if( t > t0 ){
//      double gaussian = (1.00/sqrt(sigma*2*TMath::Pi()))*exp(-0.5*(x - t_peak)*(x - t_peak)/(sigma*sigma) )

   double norm = peak_flux*(1.00/sqrt(sigma*2*TMath::Pi()));
   double gaussian = norm*exp(-0.5*(t - t_peak)*(t - t_peak)/(sigma*sigma) );

   if( t < t_peak ){
      flux = gaussian;
   }else{
      // exponential decay
      flux = norm*exp( - (t-t_peak) / tau );
   }
//   }


   return flux + offset;
}

TF1* pConvolution = NULL;

Double_t Gauss_convolved_with_exptail( Double_t* x, Double_t* y )
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

Double_t gauss_exp_convol( Double_t* x, Double_t* y )
{
   Double_t t_prim = x[0];

   Double_t offset = y[0];
   Double_t t_peak = y[1];
   Double_t peak_flux = y[2];
   Double_t sigma = y[3];
   Double_t tau = y[4];
   Double_t t = y[5];

   double flux = offset;
   double norm = peak_flux*(1.00/sqrt(sigma*2*TMath::Pi()));
   double gaussian = norm*exp(-0.5*(t_prim - t_peak)*(t_prim - t_peak)/(sigma*sigma) );
   double exp_val = exp( - (t-t_prim) / tau );   

  
   Double_t ret = gaussian*exp_val;

   printf("DEBUG : ret = %.8f ( %.8f %.8f ) : parameters %.8f, %.8f %.8f %.8f %.8f %.8f\n",ret,gaussian,exp_val,y[0],y[1],y[2],y[3],y[4],y[5]);
// exit;
   return ret;
}


Double_t Convolution( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
   Double_t t_peak = y[1];
   Double_t peak_flux = y[2];
   Double_t sigma = y[3];
   Double_t tau = y[4];

   TF1* pConvolution =  new TF1("convolution",gauss_exp_convol,t_peak-1.10,t_peak+1.00,6);
   Double_t par[6];
   par[0] = y[0];
   par[1] = y[1];
   par[2] = y[2];
   par[3] = y[3];
   par[4] = y[4];
   par[5] = t;
   pConvolution->SetParameters(par);
   pConvolution->FixParameter(0,0.00);
 
   double ret = pConvolution->Integral( t_peak-1.00, t , 1e-9 );
   delete pConvolution;

   return ret;   
}



Double_t Convolution_MyIntegral( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
   Double_t t_peak = y[1];
   Double_t peak_flux = y[2];
   Double_t sigma = y[3];
   Double_t tau = y[4];

   double norm = peak_flux*(1.00/sqrt(sigma*2*TMath::Pi()));
   double sigma2 = sigma*sigma;


   Double_t infinity_value = 0.1;
   Double_t t_prim = t_peak - infinity_value;
   Double_t dt = 1e-5; // -9 ???
   Double_t pi = TMath::Pi();
   Double_t tau_d = TMath::Power( tau, 3.00/5.00 );
 

   // Pulse_with_gauss_onset
   // Pulse_with_linear_onset

   // integral from -INF to t 
   double calka = 0.00;
   while( t_prim <= t ){
//      double onset = exp(-0.5*(t_prim - t_peak)*(t_prim - t_peak)/(sigma2) );
//      double exp_tail = exp( - (t-t_prim) / tau );   
      
//      double exp_tail = sqrt( pi*tau*tau*tau / 4.00 )* TMath::Power( (t-t_prim) , 5.00/2.00 )*exp( -(pi*pi)*tau/(4.00*(t-t_prim)) );
//      double exp_tail = TMath::Power( tau_d/(t-t_prim) , 5.00/2.00 )*exp( -(pi*pi)*tau/(4.00*(t-t_prim)) );

      // Gaussian onset :
      calka += exp( -0.5*(t_prim - t_peak)*(t_prim - t_peak)/(sigma2) - (t-t_prim) / tau ); // 1/tau here is absorbed into norm or rather peak_flux (y[2]) parameter 

      t_prim += dt;
   }
   calka = norm*calka*dt;

   return calka + offset;   
}

Double_t ConvolutionB_MyIntegral( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
   Double_t t_peak = y[1];
   Double_t peak_flux = y[2];
   Double_t sigma = y[3];
   Double_t tau = y[4];

   double norm = peak_flux*(1.00/sqrt(sigma*2*TMath::Pi()));
   double sigma2 = sigma*sigma;


   Double_t infinity_value = 0.1;
   Double_t t_prim = t_peak - infinity_value;
   Double_t dt = 1e-5; // -9 ???
   Double_t pi = TMath::Pi();
   Double_t tau_d = TMath::Power( tau, 3.00/5.00 );
 

   // Pulse_with_gauss_onset
   // Pulse_with_linear_onset

   // integral from -INF to t 
   double calka = 0.00;
   while( t_prim <= t ){
      double onset = exp(-0.5*(t_prim - t_peak)*(t_prim - t_peak)/(sigma2) );
//      double exp_tail = exp( - (t-t_prim) / tau );   
      
//      double exp_tail = sqrt( pi*tau*tau*tau / 4.00 )* TMath::Power( (t-t_prim) , 5.00/2.00 )*exp( -(pi*pi)*tau/(4.00*(t-t_prim)) );
      double exp_tail = TMath::Power( tau_d/(t-t_prim) , 5.00/2.00 )*exp( -(pi*pi)*tau/(4.00*(t-t_prim)) );

      // Gaussian onset :
//      calka += exp( -0.5*(t_prim - t_peak)*(t_prim - t_peak)/(sigma2) - (t-t_prim) / tau ); // 1/tau here is absorbed into norm or rather peak_flux (y[2]) parameter 
      calka += onset * exp_tail;

      t_prim += dt;
   }
   calka = norm*calka*dt;

   return calka + offset;   
}



Double_t LinearPulse( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
   Double_t t0 = y[1];           // start of the pulse
   Double_t t_peak = y[2];
   Double_t peak_flux = y[3];
   Double_t tau = y[4];

   double delta_t = fabs(t_peak - t0);
   double t1 = t_peak + delta_t;

   double flux = 0.00;
   if( fabs(t-t_peak) <= delta_t ){
      if( t<t_peak ){
         double a = peak_flux / ( t_peak - t0 );
         flux = a*( t - t0 );  
      }else{
//         double a = peak_flux / ( t1 - t_peak );
//         flux = a*( t1 - t );
         flux = 0.00;
      }
   }

   return flux + offset;
}

Double_t Convolution_MyIntegral_LinearOnset( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
   Double_t t0 = y[1];           // start of the pulse
   Double_t t_peak = y[2];
   Double_t peak_flux = y[3];
   Double_t tau = y[4];

//   double norm = peak_flux*(1.00/sqrt(sigma*2*TMath::Pi()));
//   double sigma2 = sigma*sigma;


   Double_t infinity_value = 0.1;
   Double_t t_prim = t_peak - infinity_value;
   Double_t dt = 1e-4; // -9 ???
 

   // Pulse_with_gauss_onset
   // Pulse_with_linear_onset

// WORKING :
/*   if( t > t0 ){
      if( t <= t_peak ){
         double a = peak_flux / ( t_peak - t0 );
         double flux = a*(t - t0 );      

         return flux + offset;
      }else{
         double exp_tail = peak_flux*exp( - (t-t_peak) / tau );
         return exp_tail + offset;
      }
   }*/
  
/*   if( t <= t0 ){ 
      return offset;
   }*/

//   return LinearPulse( x, y );

   if( t <= t_peak ){
//      double a = peak_flux / ( t_peak - t0 );
//      double flux = a*(t - t0 );

//      return flux + offset;
      return LinearPulse( x, y );
   }

   // integral from -INF to t 
   double calka = 0.00;
   while( t_prim <= t_peak ){
//      double a = peak_flux / ( t_peak - t0 );
//      double flux = a*(t_prim - t0 );  
     
//      calka += flux*exp( - (t-t_prim) / tau );
      
      double flux = LinearPulse( x, y );
      calka += flux*exp( - (t-t_prim) / tau );

      t_prim += dt;
   }
   calka = calka*dt;

   return calka;   
}




Double_t Pulse_with_gauss_onset_original( Double_t* x, Double_t* y )
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


Double_t Pulse_gauss( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t offset = y[0];
   Double_t t_peak = y[1];
   Double_t peak_flux = y[2];
   Double_t sigma = y[3];

   double flux = offset;
   double norm = peak_flux*(1.00/sqrt(sigma*2*TMath::Pi()));
   double gaussian = norm*exp(-0.5*(t - t_peak)*(t - t_peak)/(sigma*sigma) );

   flux = gaussian;

   return flux;
}

Double_t gauss( Double_t* x, Double_t* y )
{
   Double_t t = x[0];
   Double_t t_peak = (60.00-32.00);
   Double_t sigma = y[1];
   Double_t peak_ne = y[0];

   double norm = peak_ne*(1.00/sqrt(sigma*2*TMath::Pi()));
   double gaussian = norm*exp(-0.5*(t - t_peak)*(t - t_peak)/(sigma*sigma) );

   return gaussian;
}

Double_t dm_vs_mjd( Double_t* x, Double_t* y )
{ 
   // parameters : x[0] = ne, x[1] = amax, x[2] = alpha (v0 = alpha*v_crab )  
   // t - time
   Double_t t = x[0];

   Double_t dDM = y[0];
   Double_t T = y[1];
   Double_t t_peak = y[2];
   Double_t dm0 = y[3];

   Double_t delta_dm = dDM*exp(-((t-t_peak)*(t-t_peak))/(2.00*(T*T))); // was : peak_ne*exp(-(yy*yy)/(2.00*sigma*sigma))
   Double_t dm = dm0 +  delta_dm; // *sqrt(sigma)
   return dm;
}




Double_t HorizontalLine( Double_t* x, Double_t* y )
{
   return y[0];
}

Double_t Line( Double_t* x, Double_t* y )
{
   Double_t a = y[0];
   Double_t b = y[1];

   //a = (483.269-33121.78)/(68.97-841.87);
   //b = 483.269 - a*68.97;

   return (x[0]*a+b);
   // return (x[0]*y[0]);
}

TGraphErrors* DrawGraph( Double_t* x_values, Double_t* y_values, int numVal, 
         long q, TPad* pPad, const char* fit_func_name=NULL, 
         double min_y=-10000, double max_y=-10000,
         const char* szStarName="", const char* fname="default",
         int bLog=0, const char* szDescX=NULL, const char* szDescY=NULL,
         double fit_min_x=-100000, double fit_max_x=-100000, 
         Double_t* y_values_errors=NULL,
         Double_t* init_params=NULL,
         double bIgnorePeak=0.2 )
{
    int MarkerType = 20;
    int ColorNum = kRed;

    Double_t z,sigma_z,const_part;
    TF1 *line = NULL;
    TF1 *line_draw = NULL;
   TF1 *part1 = NULL;
   TF1 *part2 = NULL;
    Double_t maxX=-100000,maxY=-100000;
    Double_t minX=100000,minY=100000;
    Double_t maxYarg=-1;


    TGraphErrors* pGraph = new TGraphErrors(q);
    for(int i=0;i<numVal;i++){
        if( gVerb ){
           printf("q=%d %f %f\n",(int)q, x_values[i], y_values[i] );
        }

        pGraph->SetPoint( i, x_values[i], y_values[i] );

        if( y_values_errors ){
           pGraph->SetPointError( i, 0.00, y_values_errors[i] );
        }

        if(x_values[i]>maxX)
            maxX = x_values[i];
        if(y_values[i]>maxY){
            maxY = y_values[i];
            maxYarg = x_values[i];
        }
      
        if(x_values[i]<minX)
            minX = x_values[i];
        if(y_values[i]<minY)
            minY = y_values[i];
    }
    printf("Found min_x=%.2f , max_x=%.2f\n",minX,maxX);
    printf("Found min_y=%.2f , max_y=%.2f\n",minY,maxY);
    Double_t stepX = (maxX-minX)/10.00;
    Double_t stepY = (maxY-minY)/10.00;
    printf("Found  stepX=%.2f , stepY=%.2f\n",stepX,stepY);

    // pPad->DrawFrame(minX-stepX,minY-stepY,maxX+stepX,maxY+stepY);
   if( min_y!=-1 && max_y!=-1 ){
      // pPad->DrawFrame(minX-stepX,min_y,maxX+stepX,max_y);
   }else{
      // pPad->DrawFrame(minX-stepX,0,maxX+stepX,maxY+stepY);
   }
  //    pPad->SetGrid();   


   double r=1.00;
   double norm = 1;
   double mean = 1;
   double sigma = 1;


    pGraph->SetMarkerStyle(MarkerType);
    pGraph->SetMarkerColor(ColorNum);
    if( min_y>-10000 && max_y>-10000 ){
       pGraph->SetMinimum( min_y );
       pGraph->SetMaximum( max_y );
       printf("Set Min/Max = (%.2f,%.2f)\n",min_y,max_y);
    }else{      
       double minVal = minY-stepY;
       if( bLog ){
          if( minVal<0 ){
             minVal = 1;
          }
       }

       pGraph->SetMinimum( minVal );
       pGraph->SetMaximum( maxY+stepY );
       printf("Set Min/Max = (%.2f,%.2f)\n",minVal,(maxY+stepY));
    }
//    pGraph->SetMinimum( -90 );
//    pGraph->SetMaximum( 90 );
    pGraph->SetTitle( szStarName );
// WORKS 
//    pGraph->GetXaxis()->SetLimits(0,1);
    pGraph->Draw("AP");

   if( fit_min_x<=-100000 ){
      fit_min_x = minX;
   }
   if( fit_max_x<=-100000 ){
      fit_max_x = maxX;
   }

   printf("fitting in x range (%.2f,%.2f)\n",fit_min_x,fit_max_x);

   Double_t par[10];
   par[0] = 0;
   par[1] = 11.00;
   par[2] = 0.0;
   par[3] = 0.0;

   int local_func=0;
   if( fit_func_name && strlen(fit_func_name) ){
      if( strcmp( fit_func_name, "dm_vs_mjd" )==0 ){
         printf("Fitting pulsar DM vs. MJD in the range : %.6f - %.6f\n",fit_min_x,fit_max_x);
         line = new TF1("fit_func",dm_vs_mjd,fit_min_x,fit_max_x,4);
         line_draw = new TF1("fit_func2",dm_vs_mjd,minX,maxX,4);
         local_func=1;

// WORKING:
         Double_t km2pc = 3.240e-14;
         Double_t v_crab = 120000; // km/s = 120km/s (Kaplan at al 2008, ApJ, 677, 1201) - transverse Crab velocity 
         Double_t v_crab_pc = 3.36e-7; // pc/day // 3.888e-9;
         Double_t v0 = v_crab_pc;         

         par[0] = 0.04;
         par[1] = 1e-5/v0;
         par[2] = 62.66;
         par[3] = 0.005; // alpha
 
         line->SetParName(0,"#Delta DM");
         line->SetParName(1,"#Tau_{#sigma}");
         line->SetParName(2,"t_peak [days]");
         line->SetParName(3,"DM_{offset} [pc/cm^{3}]");
//         line->SetParName(4,"#alpha (v_{0} = #alpha v_{crab)})");
//         line->SetParName(4,"#alpha (v_{0} #approx #alpha v_{crab})");
         line->SetParName(4,"#alpha");

//         line->SetParLimits(1,0.00,1000.00);
//         line->FixParameter(1,1.00);
         line->SetParLimits(2,0.00,10000.00);

      }


      if( strcmp( fit_func_name, "pulse" )==0 ){
         printf("Fitting pulsar profile in the range : %.6f - %.6f\n",fit_min_x,fit_max_x);
         line = new TF1("fit_func",Pulse_with_linear_onset,fit_min_x,fit_max_x,5);
         line_draw = new TF1("fit_func2",Pulse_with_linear_onset,minX,maxX,5);
         local_func=1;

         par[0] = 0;
         par[1] = maxYarg - 0.005;
         par[2] = maxYarg;
         par[3] = maxY*0.5;
         par[4] = 2.00; // very long decay ...

/*         if( gNormaliseInputData ){
            par[1] = par[1] / numVal;
            par[2] = par[2] / numVal;
         }else{
            par[0] = 15685190000000.000000;
            par[3] = 15.692*1e12;
         }*/

         line->SetParName(0,"Offset");
         line->SetParName(1,"#t_{start}");
         line->SetParName(2,"#t_{peak}");
         line->SetParName(3,"Peak_flux");
         line->SetParName(4,"#tau");
      }

      if( strcmp( fit_func_name, "pulse_gauss" )==0 ){
         printf("Fitting pulsar pulse_gauss profile in the range : %.6f - %.6f\n",fit_min_x,fit_max_x);
         line = new TF1("fit_func",Pulse_with_gauss_onset,fit_min_x,fit_max_x,5);
         line_draw = new TF1("fit_func2",Pulse_with_gauss_onset,minX,maxX,5);
         local_func=1;

         par[0] = 0.00; // offset (mean off-pulse)
         par[1] = 0.5; // t_peak  0.53 for the MWA data
         par[2] = 1; // peak flux 
         par[3] = 0.1; // sigma 
         par[4] = 0.1; // very long decay ... - 10 for MWA fit 
         par[5] = 0.1; // tau 

         if( init_params ){
            par[0] = init_params[0];
            par[1] = init_params[1];
            par[2] = init_params[2];
            par[3] = init_params[3];
            par[4] = init_params[4];
         }

         line->SetParName(0,"SNR offset");
         line->SetParName(1,"t_{peak}");
         line->SetParName(2,"f_{peak}");
         line->SetParName(3,"#sigma_{gauss}");
         line->SetParName(4,"#tau");

      }

      if( strcmp( fit_func_name, "convolution" )==0 ){
         printf("Fitting pulsar Gauss-convolved-exp profile in the range : %.6f - %.6f\n",fit_min_x,fit_max_x);
         line = new TF1("fit_func",Convolution_MyIntegral,fit_min_x,fit_max_x,5);
         line_draw = new TF1("fit_func2",Convolution_MyIntegral,minX,maxX,5);
         local_func=1;

         // Gaussian 
         par[0] = 0.00; // offset (mean off-pulse)
         par[1] = maxYarg; // t_peak  0.53 for the MWA data
         par[2] = maxY; // peak flux 
         par[3] = 0.001; // sigma 
         par[4] = 0.002; // very long decay ... - 10 for MWA fit 

         // Linear onset : 
/*         par[0] = 0.00; // offset (mean off-pulse)
         par[1] = maxYarg - 0.001/2.00;
         par[2] = maxYarg;        
         par[3] = maxY;
         par[4] = 0.002;*/
 

/*         if( init_params ){
            par[0] = init_params[0];
            par[1] = init_params[1];
            par[2] = init_params[2];
            par[3] = init_params[3];
            par[4] = init_params[4];
         }*/

         line->SetParName(0,"SNR offset");
         line->SetParName(1,"t_{peak}");
         line->SetParName(2,"f_{peak}");
         line->SetParName(3,"#sigma_{gauss}");
         line->SetParName(4,"#tau");

      }

      if( strcmp( fit_func_name, "convolution_b" )==0 ){
         printf("Fitting pulsar Gauss-convolved-exp profile in the range : %.6f - %.6f\n",fit_min_x,fit_max_x);
         line = new TF1("fit_func",ConvolutionB_MyIntegral,fit_min_x,fit_max_x,5);
         line_draw = new TF1("fit_func2",ConvolutionB_MyIntegral,minX,maxX,5);
         local_func=1;

         // Gaussian 
         par[0] = 0.00; // offset (mean off-pulse)
         par[1] = maxYarg; // t_peak  0.53 for the MWA data
         par[2] = maxY; // peak flux 
         par[3] = 0.001; // sigma 
         par[4] = 0.002; // very long decay ... - 10 for MWA fit 

         // Linear onset : 
/*         par[0] = 0.00; // offset (mean off-pulse)
         par[1] = maxYarg - 0.001/2.00;
         par[2] = maxYarg;        
         par[3] = maxY;
         par[4] = 0.002;*/
 

/*         if( init_params ){
            par[0] = init_params[0];
            par[1] = init_params[1];
            par[2] = init_params[2];
            par[3] = init_params[3];
            par[4] = init_params[4];
         }*/

         line->SetParName(0,"SNR offset");
         line->SetParName(1,"t_{peak}");
         line->SetParName(2,"f_{peak}");
         line->SetParName(3,"#sigma_{gauss}");
         line->SetParName(4,"#tau");

      }

      if( strcmp( fit_func_name, "convolution_lin_onset" )==0 ){
         printf("Fitting pulsar Gauss-convolved-exp profile in the range : %.6f - %.6f\n",fit_min_x,fit_max_x);
         line = new TF1("fit_func",Convolution_MyIntegral_LinearOnset,fit_min_x,fit_max_x,5);
         line_draw = new TF1("fit_func2",Convolution_MyIntegral_LinearOnset,minX,maxX,5);
         local_func=1;

         // Linear onset : 
         par[0] = 0;
         par[1] = maxYarg - 0.005;
         par[2] = maxYarg;
         par[3] = maxY*0.5;
         par[4] = 0.002; // very long decay ...

 

/*         if( init_params ){
            par[0] = init_params[0];
            par[1] = init_params[1];
            par[2] = init_params[2];
            par[3] = init_params[3];
            par[4] = init_params[4];
         }*/

         line->SetParName(0,"SNR offset");
         line->SetParName(1,"t_0");
         line->SetParName(2,"t_{peak}");
         line->SetParName(3,"f_{peak}");
         line->SetParName(4,"#tau");

      }



      if( strcmp( fit_func_name, "pulse_gauss_only" )==0 ){
         printf("Fitting pulsar pulse_gauss profile in the range : %.6f - %.6f ( %.8f - %.8f )\n",fit_min_x,fit_max_x,fit_min_x,fit_max_x);
         line = new TF1("fit_func",Pulse_gauss,fit_min_x,fit_max_x,4);
         line_draw = new TF1("fit_func2",Pulse_gauss,minX,maxX,4);
         local_func=1;
         
         double phase_max=-1, max_value=-1;
         for(int i=0;i<numVal;i++){
            if( y_values[i] > max_value ){
                max_value = y_values[i];
                phase_max = x_values[i];
            }
         }

         par[0] = 0.00; // offset (mean off-pulse)
         par[1] = phase_max;
         par[2] = int(max_value);
//         par[1] = 0.75; // t_peak  0.53 for the MWA data
//         par[2] = 8; // peak flux 
         par[3] = 0.003333333; // sigma - good for pulse width of J1752-2806 (very narrow spike)
         printf("DEBUG : initial values for pulse_gauss_only fitting : %.4f, %.4f, %.4f, %.4f\n",par[0],par[1],par[2],par[3]);

/*         if( gNormaliseInputData ){
            par[1] = par[1] / numVal;
            par[2] = par[2] / numVal;
         }else{
            par[0] = 15685190000000.000000;
            par[3] = 15.692*1e12;
         }*/

         line->SetParName(0,"SNR offset");
         line->SetParName(1,"t_{peak}");
         line->SetParName(2,"f_{peak}");
         line->SetParName(3,"#sigma_{gauss}");

//         line->SetParName(1,"T_start");
//         line->SetParName(4,"#tau");

      }



      if( strcmp( fit_func_name, "horline" )==0 || fit_func_name[0]=='h' ){
         printf("Fitting horizontal line\n");
         line = new TF1("fit_func",HorizontalLine,fit_min_x,fit_max_x,1);
         line_draw = new TF1("fit_func2",HorizontalLine,minX,maxX,1);
         local_func=1;
      }
      if( strcmp( fit_func_name, "line" )==0 || fit_func_name[0]=='l' ){
         printf("Fitting straight line\n");
         line = new TF1("fit_func",Line,fit_min_x,fit_max_x,2);
         line_draw = new TF1("fit_func2",Line,minX,maxX,2);
         local_func=1;
      }
   }


   if( local_func ){
      line->SetParameters(par);
   }
   if( fit_func_name && strlen(fit_func_name) ){
      printf("fitting function : %s",fit_func_name);

      if( strcmp(fit_func_name,"gaus")==0 || fit_func_name[0]=='g' || fit_func_name[0]=='G' ){
         pGraph->Fit("gaus");
      }

      if( strstr(fit_func_name,"line") || fit_func_name[0]=='l' || fit_func_name[0]=='L'
          || fit_func_name[0]=='h' || fit_func_name[0]=='H' || strstr(fit_func_name,"pulse") || fit_func_name[0]=='p' || strstr(fit_func_name,"pulse_gauss") || strstr(fit_func_name,"convolution") || strstr(fit_func_name,"pulse_gauss_only")
          || strstr(fit_func_name,"dm_vs_mjd")
        ){
         pGraph->Fit("fit_func","R");
         printf("DEBUG : fitted fit_func (%s)\n",fit_func_name);
      }

      if( local_func ){
         line->GetParameters(par);
//         line->Draw("same");
         line_draw->SetParameters(par);
         line_draw->Draw("same");
         line_draw->SetLineColor(kBlack);

         printf("Fitted paramaters:\n");
         printf("\tpar[0] = %.8f +/- %.8f\n",par[0],line->GetParError(0));
         printf("\tpar[1] = %.8f +/- %.8f\n",par[1],line->GetParError(1));
         printf("\tpar[2] = %.8f +/- %.8f\n",par[2],line->GetParError(2));
         printf("\tpar[3] = %.8f +/- %.8f\n",par[3],line->GetParError(3));
         printf("\tpar[4] = %.8f +/- %.8f\n",par[4],line->GetParError(4));
         printf("TEST VALUE = %.8f vs. %.8f\n",line->Eval(0.5),line_draw->Eval(0.5));

         gFittedParametersN = 5;
         gFittedParameters[0] = par[0];
         gFittedParameters[1] = par[1];
         gFittedParameters[2] = par[2];
         gFittedParameters[3] = par[3];
         gFittedParameters[4] = par[4];

         gFittedParametersErrors[0] = line->GetParError(0);
         gFittedParametersErrors[1] = line->GetParError(1);
         gFittedParametersErrors[2] = line->GetParError(2);
         gFittedParametersErrors[3] = line->GetParError(3);
         gFittedParametersErrors[4] = line->GetParError(4);
      }

      double chi2 = line->GetChisquare();
      double ndf = line->GetNDF();
      double chi2_ndf = chi2/ndf;
      double par0_err = line->GetParError(0);
      printf("Chi2 = %.8f , ndf = %.1f , chi2_ndf = %.8f , isnan(par[0] error) = %d\n",chi2,ndf,chi2_ndf,isnan(par0_err));
      gFinalChi2 = chi2_ndf;

      double sum_resid=0.00;
      double sum_resid2=0.00;
      int count_resid=0;
      FILE* out_f = fopen("fit.txt","w"); 
      for(int i=0;i<numVal;i++){
         bool bInclude=true;
         double x = x_values[i];
         double y = y_values[i];

         if( bIgnorePeak>0 ){
            double dist_from_peak = fabs( x - par[1] );
            if( dist_from_peak <= bIgnorePeak ){
               printf("WARNING : ignoring phase bin %.8f - too close to the peak at %.8f\n",x,par[1]);
               bInclude=false;
            }
         }

         double fit = line_draw->Eval(x);
         double resid = (y-fit);
         fprintf(out_f,"%.8f %.8f %.8f %.8f\n",x,fit,y,resid);

         if( bInclude ){
            sum_resid += resid;
            sum_resid2 += resid*resid;
            count_resid += 1; 
         }
      }
      fclose(out_f);     

      // T (parameter 1) = Sigma_blob / velocity , test varius values of
      // Sigma_blob for given velocities <= 1000 km/s 
      printf("Fitted T = %.6f -> see corresponding sigma_blob values for different velocities:\n",par[1]);
      double pc_in_km = 3.0857e13; // km
      double v0 = 10.00;
      double pi = TMath::Pi();
      while (v0 <= 1000){
         double sigma_blob_km = (v0*86400)*par[1];
         double sigma_blob_pc = sigma_blob_km/pc_in_km;

         // Normalisation = SQRT(2PI)*n_peak*sigma_blob 
         double n_peak = par[0]/(sqrt(2*pi)*sigma_blob_pc);
         printf("v0 = %.4f km/s -> sigma_blob = %e [pc] = %e [km] , n_peak = %e [e-/cm^3]\n",v0,sigma_blob_pc,sigma_blob_km,n_peak);
         
         if( v0 < 300 ){
            v0 += 10;
         }else{
            v0 += 50;
         }
      }

   }
   pGraph->GetXaxis()->SetTitleOffset(1.00);
   pGraph->GetYaxis()->SetTitleOffset(1.00);
   pGraph->GetXaxis()->SetTitleSize(0.05);
   pGraph->GetXaxis()->SetLabelSize(0.05);
   pGraph->GetYaxis()->SetTitleSize(0.05);
   pGraph->GetYaxis()->SetLabelSize(0.05);


   if( szDescX && szDescX[0] ){
      pGraph->GetHistogram()->SetXTitle( szDescX );
   }else{
      pGraph->GetHistogram()->SetXTitle("Channel");
   }
   if( szDescY && szDescY[0] ){
       pGraph->GetHistogram()->SetYTitle( szDescY );
   }else{
      pGraph->GetHistogram()->SetYTitle("Power");
   }
   // pGraph->GetHistogram()->SetYTitle("sigmaG1_homeo/SigmaG1");

   TLatex lat;
   lat.SetTextAlign(23);
   lat.SetTextSize(0.05);
                                                                                
//   char szDesc[255];
//   sprintf(szDesc,"sigma/mean = %.8f\n",r);
   
   return pGraph;
}

int ReadResultsFile( const char* fname, Double_t* x_values, Double_t* y_values,
                     int CondCol=-1, int CondValue=-1, int x_col=0, int y_col=0 )
{
   const int lSize=1000;
   char buff[1000];
   
   FILE *fcd = fopen(fname,"r");
   if(!fcd){
       printf("Could not open file : %s\n",fname);
       return -1;
   }

   printf("x_col=%d y_col=%d\n",x_col,y_col);

   Int_t all = 0;
   Double_t fval1,fval2,fval3,fval4,fval5,fval6,mag,x,y;
   long lval1,lval2,lval3,lval4;
   double max_val=-1e20, min_val = 1e20;
   double max_x = -1e20;
   double min_x = 1e20;

   all=0;
   int ncols=-1;
   while (1) {
      if(fgets(buff,lSize,fcd)==0)
         break;
      if(buff[0]=='#')
         continue;      
// what if nan is in the column that we are not interested in 
//      if(strstr(buff,"nan"))
//         continue;

      //  9.39  377.000000  8.000000 328.757587  77.088256   298.312992 65.223146   44.506926
      // ncols = sscanf(buff,"%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f",&mag,&x,&y,&fval1,&fval2,&fval3,&fval4,&fval5);
      Double_t x_val;
      Double_t y_val;
//      ncols = sscanf(buff,"%f\t%f\n",&x_val,&y_val);

      int ncols=0;
      char* ptr=NULL;
      char* search_ptr=buff;
      int col=0;
      while( (ptr = strtok(search_ptr," \t")) ){
         search_ptr = NULL;
         if( gVerb ){
              printf("ptr = %s\n",ptr);
                        }

         if( col == x_col ){
            if( strstr(ptr,"nan") ){
               break;
            }
            x_val = atof( ptr );
            ncols++;
         }
         if( col == y_col ){
            if( strstr(ptr,"nan") ){
               break;
            }
            y_val = atof( ptr );
            ncols++;
         }
         col++;
      }


      if( ncols < 2 ){
         // changed due to stupid plots , single column rows are skiped
         printf("ERROR - less then 2 columns, line skiped !!!\n");
         printf("%s\n",buff);
         continue;
         // y_val = x_val;
         // x_val = all;
      }

      if (ncols < 0) break;
     
     if(CondCol>=0){
          if(lval1!=CondValue)
            continue;                                        
     }
//     printf("%s\n",buff);

     if( gStartX < 0 ){
         gStartX = x_val;
     }
     
     x_values[all] = x_val;
     y_values[all] = y_val;
     if( y_val > max_val ){
        max_val = y_val;
     }
     if( y_val < min_val ){
        min_val = y_val;
     }
     if( x_val > max_x ){
        max_x = x_val;
     }
     if( x_val < min_x ){
        min_x = x_val;
     }
     if( gVerb || 0 ){
        printf("values : %f %f\n",x_val,y_val);
     }

     all++;
   }
   fclose(fcd);

   gMinX = min_x;
   gMaxX = max_x;
   gMinY = min_val;

 
   printf("max_val = %.4f\n",max_val);
   return all;
}  

double calc_rms( Double_t* x_values, Double_t* y_values, int cnt, double start=-1, double end=-1 )
{
   double sum=0,sum2=0;
   double maxX=-1000000000;
   int count=0;

   for(int i=0;i<cnt;i++){
      double x_val = x_values[i];

      if( (start<0 || x_val >= start) && (end<0 || x_val <= end) ){
         sum += y_values[i];
         sum2 += (y_values[i])*(y_values[i]);
         count++;
      }

      if( x_val > maxX ){
         maxX = x_val;
      }
   }

   double rms = sqrt( sum2/count - (sum/count)*(sum/count) );
   double mean = (sum/count);
   printf("MEAN = %e = %e/%d\n",mean,sum,count);
   printf("RMS  = %e\n",rms);
  
   return rms;
}

double calc_rms_bins( Double_t* x_values, Double_t* y_values, int cnt, int start_bin=0, int end_bin=10 )
{
   double sum=0,sum2=0;
   double maxX=-1000000000;
   int count=0;

   for(int i=0;i<cnt;i++){
      double x_val = x_values[i];

      if( i>=start_bin && i<=end_bin ){
         sum += y_values[i];
         sum2 += (y_values[i])*(y_values[i]);
         count++;
      }

      if( x_val > maxX ){
         maxX = x_val;
      }
   }

   double rms = sqrt( sum2/count - (sum/count)*(sum/count) );
   double mean = (sum/count);
   printf("MEAN = %e = %e/%d\n",mean,sum,count);
   printf("RMS  = %e\n",rms);
  
   return rms;
}



void normalise_x( Double_t* x_values, int cnt )
{
   double minX = 1e20;
   double maxX = -1e20;

   for(int i=0;i<cnt;i++){
      double x_val = x_values[i];

      if( x_val > maxX ){
         maxX = x_val;
      }
      if( x_val < minX ){
         minX = x_val;
      }
   }

   gXaxisNormFactor = (maxX-minX);
   gXaxisOffset = minX;
   for(int i=0;i<cnt;i++){
      x_values[i] = (x_values[i] - gXaxisOffset)/gXaxisNormFactor;
   }   
}


double normalise_y_meanrms( Double_t* x_values, Double_t* y_values, int cnt, double start=0.00, double end=0.4 )
{
   double sum=0,sum2=0;
   double minY=1e20;
   double maxY=-1e20;
   int count=0;

   for(int i=0;i<cnt;i++){
      double x_val = x_values[i];

      if( (start<0 || x_val >= start) && (end<0 || x_val <= end) ){
         sum += y_values[i];
         sum2 += (y_values[i])*(y_values[i]);
         count++;
      }    

      if( y_values[i] > maxY ){
         maxY = y_values[i];
      }
      if( y_values[i] < minY ){
         minY = y_values[i];
      }
   }

   double rms = sqrt( sum2/count - (sum/count)*(sum/count) );
   double mean = (sum/count);
   printf("MEAN = %e (count = %d)\n",mean,count);
   printf("RMS  = %e (count = %d)\n",rms,count);
   printf("Y-axis range : %.8f - %.8f\n",minY,maxY);

   for(int i=0;i<cnt;i++){
      y_values[i] = (y_values[i] - mean)/rms;
   }

   gOriginalMean = mean;
   gOriginalRMS = rms;
   gYaxisOffset = mean;
   gYaxisNormFactor = rms;

   return rms;
}

double normalise_y_minmax( Double_t* x_values, Double_t* y_values, int cnt, double start=0.00, double end=0.4 )
{
   double sum=0,sum2=0;
   double minY=1e20;
   double maxY=-1e20;
   int count=0;

   for(int i=0;i<cnt;i++){
      double x_val = x_values[i];

      if( (start<0 || x_val >= start) && (end<0 || x_val <= end) ){
         sum += y_values[i];
         sum2 += (y_values[i])*(y_values[i]);
         count++;
      }    

      if( y_values[i] > maxY ){
         maxY = y_values[i];
      }
      if( y_values[i] < minY ){
         minY = y_values[i];
      }
   }

   double rms = sqrt( sum2/count - (sum/count)*(sum/count) );
   double mean = (sum/count);
   printf("MEAN = %e (count = %d)\n",mean,count);
   printf("RMS  = %e (count = %d)\n",rms,count);
   printf("Y-axis range : %.8f - %.8f\n",minY,maxY);

   gYaxisNormFactor = (maxY-minY);
   for(int i=0;i<cnt;i++){
      y_values[i] = (y_values[i] - minY)/gYaxisNormFactor;
   }

   gOriginalMean = mean;
   gOriginalRMS = rms;
   gYaxisOffset = minY;

   return rms;
}



void fit_dm_variations_gaussint_PAPER_5par_SIMPLE( const char* basename="DM_vs_UX.txt", 
                       const char* fit_func_name="dm_vs_mjd", // dm_vs_time
                       double noise_start=0, double noise_end=0.4, 
                       double sigma_simulated=0.1120, // simulated sigma of noise in Jy , sigma_Stokes_I - for the entire duration of the observation !!!
                       bool bIsSigmaSimulPerPhaseBin=false, // if this is true no need to multiply sigma_simulated by sqrt(n_bins)
                       bool bUseFitResidualsRMS=false, // use residuals of the FIT to calculate Sigma_obs which may be slightly different than 1.00 (after normalisation)
                       double min_y=-1000000,  double max_y=-1000000, int bLog=0,
      const char* szDescX="MJD [days]",
      const char* szDescY="DM - 56.705825 [pc/cm^{3}] ", 
      const char* szTitle=NULL,
      double fit_min_x=0, double fit_max_x=100,
      int x_col=0, int y_col=1, const char* outpngfile=NULL )
{
   int index;
   double snr,time;
   if( sscanf(basename,"pulse%d_snr%lf_time%lfsec.txt",&index,&snr,&time) == 3){
      printf("SSCANF OK : %d , %.1f %.8f\n",index,snr,time);
   }

   if( !szTitle){
      szTitle = basename;
   }
   gLog = bLog;
   gNoiseStart = noise_start;
   gNoiseEnd   = noise_end;
   gUseFitResidualsRMS = bUseFitResidualsRMS;
   strcpy(gInputFileName,basename);
   
   // gROOT->Reset();
   // const char* basename = "s_vs_sigma_g_sqr";

   TCanvas* c1 = new TCanvas("c1","xxxx",200,10,700,500);
   c1->SetGridx();
   c1->SetGridy();
   c1->SetFillColor(0);
   c1->SetFillStyle(0);
   if( bLog ){
      c1->SetLogy(1);
   }
   
   gStyle->SetPadTopMargin(0.03);
   gStyle->SetPadBottomMargin(1.2);
   gStyle->SetPadLeftMargin(0.15);
   gStyle->SetPadRightMargin(0.04);
   gStyle->SetTitleXOffset(1.12);
   gStyle->SetTitleYOffset(1.2);
   gStyle->SetFillColor(0);
   gStyle->SetFillStyle(0);
   gStyle->SetOptFit(111);

   // p1 = new TPad("p1","p1",0,0,1,1,-1);
   // p1->Draw();

   


   Double_t* x_value1 = new Double_t[MAX_ROWS];
   Double_t* y_value1 = new Double_t[MAX_ROWS];
   Double_t* x_value1_err = new Double_t[MAX_ROWS];
   Double_t* y_value1_err = new Double_t[MAX_ROWS];
   Double_t maxX=-100000,maxY=-100000;   
   Double_t minX=100000,minY=100000;

   Int_t ncols;
   Int_t lq1=0,lq2=0,lq3=0,lq5=0,lq9=0,lq25=0;



   lq1 = ReadResultsFile( basename, x_value1, y_value1, -1, -1, 0, 2 ); 
   int lq1_err = ReadResultsFile( basename, x_value1_err, y_value1_err, -1, -1, 1, 3 );

   for(int i=0;i<lq1;i++){
      x_value1[i] = (x_value1[i] - gStartX)/86400;      
   }
   
   TGraphErrors* pGraph1 = DrawGraph( x_value1, y_value1, lq1, 1, NULL, fit_func_name, min_y, max_y, szTitle,
                                      basename, bLog, szDescX, szDescY, fit_min_x, fit_max_x, y_value1_err );

   c1->Update();

   char szFittedFile[128];
   sprintf(szFittedFile,"%s.fit",basename);
   FILE* outf = fopen(szFittedFile,"w");
   fprintf(outf,"%.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f\n",gFittedParameters[0],gFittedParametersErrors[0],gFittedParameters[1],gFittedParametersErrors[1],gFittedParameters[2],gFittedParametersErrors[2],gFittedParameters[3],gFittedParametersErrors[3],gFittedParameters[4],gFittedParametersErrors[4]);
   fclose(outf);

   TString szPngName1="images/";
   if( outpngfile ){
      szPngName1 += outpngfile;
   }else{
      szPngName1 += basename;
      szPngName1 += ".png";
   }
   c1->Print(szPngName1.Data());
}
