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

double gDM=-1;
double gFinalChi2 = -1.00;
double gCalConstant = 33.03678886; // Jy 
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

double gMinX_Allowed = -1e20;
double gMaxX_Allowed = +1e20;

// double gMinX_Allowed = 0.87;
// double gMaxX_Allowed = 0.93;


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

   double flux = 0.00;
   if( t > t0 ){
      if( t < t_peak ){
         double a = peak_flux / ( t_peak - t0 );
         flux = a*(t - t0 );
      }else{
         // exponential decay
         flux = peak_flux*exp( - (t-t_peak) / tau );
      }
   }


   return flux + offset;

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
         double bIgnorePeak=0.2, bool bSaveFit=false )
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
        if(y_values[i]>maxY)
            maxY = y_values[i];
      
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

   double phase_max=-1, max_value=-1;
   int phase_max_i = -1;

   int local_func=0;
   if( fit_func_name && strlen(fit_func_name) ){
      if( strcmp( fit_func_name, "pulse" )==0 ){
         printf("Fitting pulsar profile in the range : %.6f - %.6f\n",fit_min_x,fit_max_x);
         line = new TF1("fit_func",Pulse_with_linear_onset,fit_min_x,fit_max_x,5);
         line_draw = new TF1("fit_func2",Pulse_with_linear_onset,minX,maxX,5);
         local_func=1;

         for(int i=0;i<numVal;i++){
            if( y_values[i] > max_value ){
                max_value = y_values[i];
                phase_max = x_values[i];
                phase_max_i = i;
            }
         }

printf("DEBUG : phase_max = %.8f\n",phase_max);
         double ts = phase_max - 0.001; // 2*(x_values[1]-x_values[0]);
         double tp = phase_max;
         double alpha = max_value / (tp - ts);

         par[0] = 0;
         par[1] = ts;
         par[2] = tp;
         par[3] = max_value;
         par[4] = 0.2; // very long decay ...         

         FILE* parf = fopen("last.fit","r");
         if( parf ){
            char szLine[256];
            fgets(szLine,256,parf);
            if( sscanf(szLine,"%lf %lf %lf %lf %lf\n",&(par[0]),&(par[1]),&(par[2]),&(par[3]),&(par[4])) == 5 ){
               printf("Read parameters from file last.fit : %.8f %.8f %.8f %.8f %.8f\n",par[0],par[1],par[2],par[3],par[4]);
            }else{
               printf("ERROR : could not read 5 parameters from file last.fit\n");
            }
            fclose(parf);
         }else{
            printf("WARNING : file last.fit with fitted parameteres does not exist\n");
         }

/*         if( gNormaliseInputData ){
            par[1] = par[1] / numVal;
            par[2] = par[2] / numVal;
         }else{
            par[0] = 15685190000000.000000;
            par[3] = 15.692*1e12;
         }*/

         printf("Initial parameters:\n");
         printf("   Offset    = %.8f\n",par[0]);
         printf("   t_start   = %.8f\n",par[1]);
         printf("   t_peak    = %.8f\n",par[2]);
         printf("   Peak_flux = %.8f\n",par[3]);
         printf("   Tau       = %.8f\n",par[4]);

         line->SetParName(0,"Offset");
         line->SetParName(1,"#t_{start}");
         line->SetParName(2,"#t_{peak}");
         line->SetParName(3,"Peak_flux");
         line->SetParName(4,"#tau");

         line->SetParLimits(1,0.00,0.04);
         line->SetParLimits(2,0.00,0.04);
         line->SetParLimits(3,0.00,1.00);
         line->SetParLimits(4,0.00,1.00);

      }

      if( strcmp( fit_func_name, "leading_edge" )==0 ){
         int n_slope_points = 4;
         printf("Fitting leading edge using %d points\n",n_slope_points);

         for(int i=0;i<numVal;i++){
            if( y_values[i] > max_value ){
                max_value = y_values[i];
                phase_max = x_values[i];
                phase_max_i = i;
            }
         }

         double height1=0.5*max_value;
         double prev_value = max_value;
         int i_height1 = -1;
         for(int i=phase_max_i;i>0;i--){
             if( prev_value > height1 && y_values[i] < height1){
                i_height1 = i;
                break;
             }
             prev_value = y_values[i];
         }

         double height2=0.25*max_value;
         prev_value = max_value;
         int i_height2 = -1;
         for(int i=phase_max_i;i>0;i--){
             if( prev_value > height2 && y_values[i] < height2){
                i_height2 = i;
                break;
             }
             prev_value = y_values[i];
         }

         double res = x_values[1] - x_values[0]; 
         double x_height1 = x_values[i_height1];
         double x_height2 = x_values[i_height2];

//         double fit_start = x_height2-5*res;
//         double fit_end = x_height1+5*res;
         double fit_start = x_height2-n_slope_points*res;
         double fit_end = x_height2+n_slope_points*res;
         printf("DEBUG : phase_max = %.8f , phase rnage = %.8f - %.8f, fiting in range %.8f - %.8f , res = %.8f\n",phase_max,x_height2,x_height1,fit_start,fit_end,res);

         line = new TF1("fit_func",Line,fit_start,fit_end,2);
         line_draw = new TF1("fit_func2",Line,fit_start,fit_end,2);
         local_func=1;

         par[0] = (y_values[i_height2 + n_slope_points] - y_values[i_height2 - n_slope_points])/(x_values[i_height2 + n_slope_points] - x_values[i_height2 - n_slope_points]);
         par[1] = y_values[i_height2] - par[0]*x_values[i_height2];

         FILE* parf = fopen("last.fit","r");
         if( parf ){
            char szLine[256];
            fgets(szLine,256,parf);
            if( sscanf(szLine,"%lf %lf\n",&(par[0]),&(par[1])) ==  2 ){
               printf("Read parameters from file last.fit : %.8f %.8f\n",par[0],par[1]);
            }else{
               printf("ERROR : could not read 5 parameters from file last.fit\n");
            }
            fclose(parf);
         }else{
            printf("WARNING : file last.fit with fitted parameteres does not exist\n");
         }


         line->SetParName(0,"Slope");
         line->SetParName(1,"Intercept");
      }


      if( strcmp( fit_func_name, "pulse_gauss" )==0 ){
         printf("Fitting pulsar pulse_gauss profile in the range : %.6f - %.6f\n",fit_min_x,fit_max_x);
         line = new TF1("fit_func",Pulse_with_gauss_onset,fit_min_x,fit_max_x,5);
         line_draw = new TF1("fit_func2",Pulse_with_gauss_onset,minX,maxX,5);
         local_func=1;

         par[0] = 0.00; // offset (mean off-pulse)
         par[1] = phase_max; // t_peak  0.53 for the MWA data
         par[2] = 0.005; // peak flux 
         par[3] = 0.01; // sigma 
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

      if( strcmp( fit_func_name, "pulse_gauss_only" )==0 ){
         printf("Fitting pulsar pulse_gauss profile in the range : %.6f - %.6f ( %.8f - %.8f )\n",fit_min_x,fit_max_x,fit_min_x,fit_max_x);
         line = new TF1("fit_func",Pulse_gauss,fit_min_x,fit_max_x,4);
         line_draw = new TF1("fit_func2",Pulse_gauss,minX,maxX,4);
         local_func=1;
         
         for(int i=0;i<numVal;i++){
            if( y_values[i] > max_value ){
                max_value = y_values[i];
                phase_max = x_values[i];
                phase_max_i = i;
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



/*      if( strcmp( fit_func_name, "horline" )==0 || fit_func_name[0]=='h' ){
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
      }*/
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
          || fit_func_name[0]=='h' || fit_func_name[0]=='H' || strstr(fit_func_name,"pulse") || fit_func_name[0]=='p' || strstr(fit_func_name,"pulse_gauss") || strstr(fit_func_name,"pulse_gauss_only")
        ){
         // options : https://root.cern.ch/doc/master/classTGraph.html#a61269bcd47a57296f0f1d57ceff8feeb
         pGraph->Fit("fit_func","R,F,E,M,V");
         printf("DEBUG : fitted fit_func (%s)\n",fit_func_name);
      }

      if( local_func ){
         line->GetParameters(par);
//         line->Draw("same");
         line_draw->SetParameters(par);
         line_draw->Draw("same");
         line_draw->SetLineColor(kBlack);

         printf("Fitted paramaters:\n");
         printf("\tpar[0] = %.8f\n",par[0]);
         printf("\tpar[1] = %.8f\n",par[1]);
         printf("\tpar[2] = %.8f\n",par[2]);
         printf("\tpar[3] = %.8f\n",par[3]);
         printf("\tpar[4] = %.8f\n",par[4]);
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

         if( strcmp( fit_func_name, "pulse" )==0 ){
            double slope = par[3]/(par[2]-par[1]);
            double risetime = (par[2] - par[1]);

            printf("SLOPE = %.8f\n",slope);
            printf("RISETIME = %.8f\n",risetime);


            double ts = par[1];
            double tp = par[2];
            double l = (tp-ts);
            double fp = par[3];
            double t1 = ts + 0.1*l; // 0.2
            double t2 = ts + 0.5*l; // 0.6

            TF1* rise = new TF1("Line",Line,t1,t2,2);
            Double_t parl[2];
            parl[0] = fp/(tp-ts);
            parl[1] = -parl[0]*ts;
            pGraph->Fit("Line","R,F,E,M,V");
//            rise->SetLineColor(kGreen);
            char szFittedFile[128];
            sprintf(szFittedFile,"%s.rise",gInputFileName); 
            FILE* outf = fopen(szFittedFile,"w");
            rise->GetParameters(parl);
            fprintf(outf,"%.8f 0.00 %.8f %.8f %.8f %.8f\n",gDM,parl[0],rise->GetParError(0),parl[1],rise->GetParError(1));
            fclose(outf);

            sprintf(szFittedFile,"%s.peak",gInputFileName); 
            outf = fopen(szFittedFile,"w");
            rise->GetParameters(parl);
            fprintf(outf,"%.8f 0.00 %.8f %.8f %.8f %.8f\n",gDM,(fp-par[0]),gFittedParametersErrors[3]);
            fclose(outf);
         }

      }


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
//               printf("WARNING : ignoring phase bin %.8f - too close to the peak at %.8f\n",x,par[1]);
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

      double t_peak = x_values[phase_max_i];
      int pre_rise_i = phase_max_i - 10;
      if( pre_rise_i < 0 ){
         pre_rise_i = 0;
      }
      int post_rise_i = phase_max_i + 50;
      if( post_rise_i >= numVal ){
         post_rise_i = numVal;
      }
      double start_x = x_values[pre_rise_i];
      double end_x = x_values[phase_max_i]+50*gFittedParameters[4];
      double dt = 0.000001;
      if( end_x > x_values[numVal-1] ){
         end_x = x_values[numVal-1];
      }

      double x_left = -1;
      double x_right = -1;      
      double t = start_x;
      while( t < end_x ){
         double fitval = line_draw->Eval( t );
//         printf("SEARCH : %.6f %.8f\n",t,fitval);

         if(x_left < 0 ){
            if( fitval >= max_value/2 ){
               x_left = t;
            }
         }else{
            if( t > t_peak ){
                if( x_right < 0 ){
                   if( fitval <= (max_value/2) ){
                      x_right = t;
                      break; 
                   }
                }
            }
         }

         t += dt;
      }
      double fwhm = (x_right-x_left)*1000.0;
      printf("PULSE FWHM %.8f - %.8f -> width = %.8f [ms]\n",x_left,x_right,fwhm);

      FILE* outf = fopen("FWHM.txt","a+"); 
      fprintf(outf,"%s %.8f\n",gInputFileName,fwhm);
      fclose(outf);


      outf = fopen("TAU_FWHM.txt","a+"); 
      fprintf(outf,"%s %.8f %.8f\n",gInputFileName,gFittedParameters[4],gFittedParametersErrors[4]);
      fclose(outf);



   pGraph->GetXaxis()->SetTitleOffset(1.00);
   pGraph->GetYaxis()->SetTitleOffset(0.60);
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
                                                                                
   char szDesc[255];
   sprintf(szDesc,"sigma/mean = %.8f\n",r);
   
   return pGraph;
       

   }   
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
   double sum = 0.00;
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

      if( x_val < gMinX_Allowed || x_val > gMaxX_Allowed ){
         continue; 
      }
     
     if(CondCol>=0){
          if(lval1!=CondValue)
            continue;                                        
     }
//     printf("%s\n",buff);
     
     x_values[all] = x_val;
     y_values[all] = y_val;     
     sum += y_val;
 

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
   printf("Manual integration in ReadResults (sum) = %.8f\n",sum);
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
   printf("SNR = 1 / rms = %.4f\n",1/rms);
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
   printf("SNR = 1 / rms = %.4f\n",(maxY-minY)/rms);
   printf("Y-axis range : %.8f - %.8f\n",minY,maxY);

   sum2=0;
   sum=0;
   count=0;
   gYaxisNormFactor = (maxY-minY);
   for(int i=0;i<cnt;i++){
      double x_val = x_values[i];
      y_values[i] = (y_values[i] - minY)/gYaxisNormFactor;

      if( (start<0 || x_val >= start) && (end<0 || x_val <= end) ){
        sum += y_values[i];
        sum2 += y_values[i]*y_values[i];
        count++;
      }
   }

   double rms_norm = sqrt( sum2/count - (sum/count)*(sum/count) );
   printf("AFTER NORMALISATION:\n");
   printf("RMS  = %e (count = %d)\n",rms_norm,count);
   printf("SNR = 1 / rms = %.4f\n",1.00/rms_norm);


   gOriginalMean = mean;
   gOriginalRMS = rms;
   gYaxisOffset = minY;

   return rms;
}



void fit_leading_edge( const char* basename, double dm, const char* fit_func_name="leading_edge",
                         double range_start=-1e20, double range_end=+1e20,
                         double cal_constant=1.00,
                   int bNormaliseInputData=0, bool bShowOriginalDataWithFit=false,
                       double noise_start=0, double noise_end=0.4, 
                       double sigma_simulated=0.1120, // simulated sigma of noise in Jy , sigma_Stokes_I - for the entire duration of the observation !!!
                       bool bIsSigmaSimulPerPhaseBin=false, // if this is true no need to multiply sigma_simulated by sqrt(n_bins)
                       bool bUseFitResidualsRMS=false, // use residuals of the FIT to calculate Sigma_obs which may be slightly different than 1.00 (after normalisation)
                       double min_y=-1000000,  double max_y=-1000000, int bLog=0,
      const char* szDescX="Phase",
      const char* szDescY="Signal to Noise Ratio (SNR)", 
      const char* szTitle=NULL,
      double fit_min_x=-100000, double fit_max_x=-100000,
      int x_col=0, int y_col=1, const char* outpngfile=NULL )
{
   gDM = dm;
   gMinX_Allowed = range_start;
   gMaxX_Allowed = range_end;

   // snr002315_snr5.7_time3120.549304sec.txt
   int index;
   double snr,time;
   if( sscanf(basename,"snr%d_snr%lf_time%lfsec.txt",&index,&snr,&time) == 3){
      printf("SSCANF OK : %d , %.1f %.8f\n",index,snr,time);
   }

   if( !szTitle){
      szTitle = basename;
   }
   gCalConstant = cal_constant;
   gLog = bLog;
   gNormaliseInputData = bNormaliseInputData;
   gNoiseStart = noise_start;
   gNoiseEnd   = noise_end;
   gUseFitResidualsRMS = bUseFitResidualsRMS;
   strcpy(gInputFileName,basename);
   
   // gROOT->Reset();
   // const char* basename = "s_vs_sigma_g_sqr";

   TCanvas* c1 = new TCanvas("c1","xxxx",200,10,1800,1200);
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
   Double_t* x_value1_original = new Double_t[MAX_ROWS];
   Double_t* y_value1_original = new Double_t[MAX_ROWS];
   Double_t* y_value1_err = new Double_t[MAX_ROWS];
   Double_t maxX=-100000,maxY=-100000;   
   Double_t minX=100000,minY=100000;

   Int_t ncols;
   Int_t lq1=0,lq2=0,lq3=0,lq5=0,lq9=0,lq25=0;



   lq1 = ReadResultsFile( basename, x_value1, y_value1, -1, -1, x_col, y_col ); 
   for(int i=0;i<lq1;i++){
      x_value1_original[i] = x_value1[i];
      y_value1_original[i] = y_value1[i];
   }
   
   int n_bins = lq1;
   printf("DEBUG : number of bins = %d\n",n_bins);     
   double rms = calc_rms_bins( x_value1, y_value1, lq1, 0 , 12 );
   double rms_original = rms;

   // normalisation of X-axis to [0,1] range is required always to make it
   // easier to find peak of the flux (~0.5)
//   normalise_x( x_value1, lq1 );

   // normalise by Y-MEAN/RMS
/*   if( bNormaliseInputData == 1 ){
      rms = normalise_y_meanrms( x_value1, y_value1, lq1, 0.00, 0.4  );
   }else{
      if( bNormaliseInputData == 2 ){
         rms = normalise_y_minmax( x_value1, y_value1, lq1, 0.00, 0.4  );
      }else{
         printf("UNKNOWN VALUE of bNormaliseInputData = %d\n",bNormaliseInputData);
      }
   }*/
   
   printf("Control RMS after re-scaling:\n");
   double rms_new = calc_rms( x_value1, y_value1, lq1, noise_start, noise_end );
   printf("RMS after rescaling = %.6f (vs. old %.6f) in range %.6f - %.6f\n",rms_new,rms_original,noise_start,noise_end);

   for(int i=0;i<lq1;i++){
     y_value1_err[i] = rms_original;
   }


   
   // drawing background graphs here :
   TGraphErrors* pGraph1 = DrawGraph( x_value1, y_value1, lq1, 1, NULL, fit_func_name, min_y, max_y, szTitle,
                                      basename, bLog, szDescX, szDescY, fit_min_x, fit_max_x, y_value1_err );
   
   if( gNormaliseInputData > 0 ){
      double scattering_time_sec = gFittedParameters[4]*gXaxisNormFactor;
      gScatteringTimeTauMS = scattering_time_sec*1000.00;
      printf("Scattering time par[4] = %.8f -> re-scaled back = %.8f [sec] = %.8f [ms]\n",gFittedParameters[4],scattering_time_sec,gScatteringTimeTauMS);

      double pulse_width_sec = gFittedParameters[3]*gXaxisNormFactor; 
      gPulseWidthMS = pulse_width_sec*1000.00;
      printf("Pulse width par[3] = %.8f -> re-scaled back = %.8f [sec] = %.8f [ms]\n",gFittedParameters[3],pulse_width_sec,gPulseWidthMS);
   }

   c1->Update();

   double dm_err=0.00;
   char szFittedFile[128];
   sprintf(szFittedFile,"%s.fit",basename);
   FILE* outf = fopen(szFittedFile,"w");
   if( strstr(basename,"leading_edge") ){
      fprintf(outf,"%.8f %.8f %.8f %.8f %.8f %.8f\n",dm,dm_err,gFittedParameters[0],gFittedParametersErrors[0],gFittedParameters[1],gFittedParametersErrors[1]);
   }else{
      fprintf(outf,"%.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f\n",dm,dm_err,gFittedParameters[0],gFittedParametersErrors[0],gFittedParameters[1],gFittedParametersErrors[1],gFittedParameters[2],gFittedParametersErrors[2],gFittedParameters[3],gFittedParametersErrors[3],gFittedParameters[4],gFittedParametersErrors[4]);
   }
   fclose(outf);

   outf = fopen("last.fit","w");
   if( strstr(basename,"leading_edge") ){
      fprintf(outf,"%.8f %.8f\n",gFittedParameters[0],gFittedParameters[1]);
   }else{
      fprintf(outf,"%.8f %.8f %.8f %.8f %.8f\n",gFittedParameters[0],gFittedParameters[1],gFittedParameters[2],gFittedParameters[3],gFittedParameters[4]);
   }
   fclose(outf);



//   normalise_x( x_value1_original, lq1 );
//   normalise_y_minmax( x_value1_original, y_value1_original, lq1, 0.00, 0.4  );

   gFittedParametersOriginalScaling[0] = gFittedParameters[0]*gYaxisNormFactor; // + gYaxisOffset;
   gFittedParametersOriginalScaling[1] = gFittedParameters[1]*gXaxisNormFactor + gXaxisOffset;
   gFittedParametersOriginalScaling[2] = gFittedParameters[2]*gYaxisNormFactor/5;
   gFittedParametersOriginalScaling[3] = gFittedParameters[3]*gXaxisNormFactor;
   gFittedParametersOriginalScaling[4] = gFittedParameters[4]*gXaxisNormFactor;

   gFittedParametersOriginalScalingErrors[0] = gFittedParametersErrors[0]*gYaxisNormFactor;
   gFittedParametersOriginalScalingErrors[1] = gFittedParametersErrors[1]*gXaxisNormFactor;
   gFittedParametersOriginalScalingErrors[2] = gFittedParametersErrors[2]*gYaxisNormFactor;
   gFittedParametersOriginalScalingErrors[3] = gFittedParametersErrors[3]*gXaxisNormFactor;
   gFittedParametersOriginalScalingErrors[4] = gFittedParametersErrors[4]*gXaxisNormFactor;

   printf("PARAMETERS SCALED BACK:\n");
   printf("par[0] = %.8f (SNR_offset)\n",gFittedParametersOriginalScaling[0]);
   printf("par[1] = %.8f [sec] (peak time)\n",gFittedParametersOriginalScaling[1]);
   printf("par[2] = %.8f (peak flux)\n",gFittedParametersOriginalScaling[2]);
   printf("par[3] = %.8f [sec] (sigma_gauss = pulse width)\n",gFittedParametersOriginalScaling[3]);
   printf("par[4] = %.8f [sec] (scattering time, decay tau)\n",gFittedParametersOriginalScaling[4]);

   sprintf(szFittedFile,"%s.fit_scaled_back",basename);
   outf = fopen(szFittedFile,"w");
   fprintf(outf,"%.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f\n",gFittedParametersOriginalScaling[0],gFittedParametersOriginalScalingErrors[0],gFittedParametersOriginalScaling[1],gFittedParametersOriginalScalingErrors[1],gFittedParametersOriginalScaling[2],gFittedParametersOriginalScalingErrors[2],gFittedParametersOriginalScaling[3],gFittedParametersOriginalScalingErrors[3],gFittedParametersOriginalScaling[4],gFittedParametersOriginalScalingErrors[4]);
   fclose(outf);

   printf("Scaling factors for:\n");
   printf("X-axis %.8f\n",gXaxisNormFactor);
   printf("Y-axis %.8f\n",gYaxisNormFactor);

   if( bShowOriginalDataWithFit ){
      // plot original data:
      TCanvas* c2 = new TCanvas("c2","xxxx",200,10,700,500);
      c2->SetGridx();
      c2->SetGridy();
      c2->SetFillColor(0);
      c2->SetFillStyle(0);
      if( bLog ){
         c2->SetLogy(1);
      }
      // just subtract baselines :
      for(int i=0;i<lq1;i++){
         y_value1_original[i] = y_value1_original[i] - gYaxisOffset;
      }

      for(int i=0;i<lq1;i++){
         y_value1_err[i] = rms_original;
      }

      printf("DEBUG : using rms_original = %.8f\n",rms_original);
      TGraphErrors* pGraph2 = DrawGraph( x_value1_original, y_value1_original, lq1, 1, NULL, fit_func_name, 0.00, max_y, szTitle,
                                         basename, bLog, szDescX, szDescY, fit_min_x, fit_max_x, y_value1_err, gFittedParametersOriginalScaling, 0.01/2.00, true ); // gFittedParametersOriginalScaling[4]*2.00 );

      TF1* line_original_data = new TF1("fit_func_test",Pulse_with_gauss_onset_original,gMinX,gMaxX,gFittedParametersN);
//   TF1* line_original_data = new TF1("fit_func_test",Pulse_with_gauss_onset_original,0,1,gFittedParametersN);
      line_original_data->SetParameters(gFittedParametersOriginalScaling);
      printf("DEBUG : %.8f\n",line_original_data->Eval( x_value1_original[0] ));
      line_original_data->Draw("same");
      c2->Update();

      sprintf(szFittedFile,"%s.refit",basename);
      outf = fopen(szFittedFile,"w");
      fprintf(outf,"%.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.8f %.1f %.2f\n",gFittedParameters[0],gFittedParametersErrors[0],gFittedParameters[1],gFittedParametersErrors[1],gFittedParameters[2],gFittedParametersErrors[2],gFittedParameters[3],gFittedParametersErrors[3],gFittedParameters[4],gFittedParametersErrors[4],snr,gFinalChi2);
      fclose(outf);

      TString szPngName1="images/";
      if( outpngfile ){
         szPngName1 += outpngfile;
      }else{
         szPngName1 += basename;
         szPngName1 += ".png";
      }
   }else{
      TString szPngName1="images/";
      if( outpngfile ){
         szPngName1 += outpngfile;
      }else{
         szPngName1 += basename;
         szPngName1 += ".png";
      }
      c1->Print(szPngName1.Data());  
   }
}
