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
#include <TLatex.h>
#include <TStyle.h>
#include <TMath.h>
#include <TComplex.h>
#include <TRandom.h>
#include <TSystem.h>

int gLog=0;
int gVerb=0;
int gDb2Num=0;

// frequency range :
double gMinFreq=-100.00;
double gMaxFreq=1e20;

double gMinAllowedValue=-1e20;

#define MAX_ROWS 10000000

Double_t interpolate(Double_t* ftab, Double_t* ytab, int cnt, Double_t f, int bUseEnds=0 )
{
   for(int i=1;i<cnt;i++){
      double freq = ftab[i];
      double val  = ytab[i];
        
      if( fabs(freq - f )<=0.01 ){
         return val;
      }else{
         if( ftab[i-1]<=f && f<=ftab[i] ){
            double u1 = ftab[i-1];
            double t1 = ytab[i-1];
            double u2 = ftab[i];
            double t2 = ytab[i];

            double interpol_val = t1 + (f-u1)*(t2-t1)/(u2-u1);
//            printf("Out_val = %.2f\n",(interpol_val));
            return (interpol_val);
         }        
      }
   }              

   if( bUseEnds ){
      if( f < ftab[0] ){
         return ytab[0];
      }
      if( f > ftab[cnt-1] ){
         return ytab[cnt-1];
      }
   }

   if( gVerb > 0 ){
      printf("WARNING : could not find value for freq = %f\n",f);
   }

   return -1000;
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


TGraph* DrawGraph( Double_t* x_values, Double_t* y_values, int numVal, 
         long q, TPad* pPad, int ColorNum = kRed, const char* szOpt="pA", int MarkerSize=1.00, int MarkerType = 20,
         const char* fit_func_name=NULL, 
         double min_y=-10000, double max_y=-10000,
         const char* szStarName="", const char* fname="default",
         int bLog=0, const char* szDescX=NULL, const char* szDescY=NULL,
         double fit_min_x=-100000, double fit_max_x=-100000 )
{
   Double_t z,sigma_z,const_part;
   TF1 *line = NULL;
   TF1 *line_draw = NULL;
   TF1 *part1 = NULL;
   TF1 *part2 = NULL;
   Double_t maxX=-100000,maxY=-100000;
   Double_t minX=100000,minY=100000;

   printf("Drawing %d points in color = %d\n",numVal,ColorNum);

   TGraph* pGraph = new TGraph(q);
   for(int i=0;i<numVal;i++){
       if( gVerb ){
           printf("q=%d %f %f\n",q, x_values[i], y_values[i] );
       }

       pGraph->SetPoint( i, x_values[i], y_values[i] );

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

   double r=1.00;
   double norm = 1;
   double mean = 1;
   double sigma = 1;


   pGraph->SetMarkerStyle(MarkerType);
   pGraph->SetMarkerSize(MarkerSize);
   pGraph->SetMarkerColor(ColorNum);
   pGraph->SetLineColor(ColorNum);
   pGraph->SetLineWidth(2);
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
    pGraph->Draw( szOpt );

   if( fit_min_x<=-100000 ){
      fit_min_x = minX;
   }
   if( fit_max_x<=-100000 ){
      fit_max_x = maxX;
   }

   printf("fitting in x range (%.2f,%.2f)\n",fit_min_x,fit_max_x);

   int local_func=0;
   if( fit_func_name && strlen(fit_func_name) ){
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


   Double_t par[4];
   par[0] = 0.1;
   par[1] = 1.00;
   par[2] = 0.0;
   par[3] = 0.0;


   if( local_func ){
      line->SetParameters(par);
   }
   if( fit_func_name && strlen(fit_func_name) ){
      printf("fitting function : %s",fit_func_name);

      if( strcmp(fit_func_name,"gaus")==0 || fit_func_name[0]=='g' || fit_func_name[0]=='G' ){
         pGraph->Fit("gaus");
      }
               if( strcmp(fit_func_name,"pol1")==0  ){
                   pGraph->Fit("pol1");
                   // par[0]=pol1->GetParameter(0);
                   // par[1]=pol1->GetParameter(1);   
                }

              if( strcmp(fit_func_name,"pol2")==0  ){
                   pGraph->Fit("pol2");
                   // par[0]=pol2->GetParameter(0);
                   // par[1]=pol2->GetParameter(1);
                   // par[2]=pol2->GetParameter(2);

                }

      if( strstr(fit_func_name,"line") || fit_func_name[0]=='l' || fit_func_name[0]=='L'
          || fit_func_name[0]=='h' || fit_func_name[0]=='H' ){
         pGraph->Fit("fit_func","R");
      }

      if( local_func ){
         line->GetParameters(par);
//         line->Draw("same");
         line_draw->SetParameters(par);
         line_draw->Draw("same");
      }


      TLatex lat;
      lat.SetTextAlign(23);
      lat.SetTextSize(0.05);

      TString szA,szB,szConst,szDesc1,szDesc2;

      printf("minX=%.2f maxX=%.2f minY=%.2f maxY=%.2f\n",minX,maxX,minY,maxY);
//      if( strcmp(fit_func_name,"line")==0 || fit_func_name[0]=='l' || fit_func_name[0]=='L'){



      szDesc1 = "a = ";
      szDesc1 += par[0];

      if( strcmp(fit_func_name,"pol2")==0  ){
             szDesc1 = "y = ";
            szDesc1 += par[0];
            szDesc1 += " + (";
            szDesc1 += par[1];
            szDesc1 += " *m) + (";
            szDesc1 += par[2];
            szDesc1 += " *m^2) i=";
//            szDesc1 +=  i;                 
      }
      if( strcmp(fit_func_name,"pol1")==0  ){
            szDesc1 = "y = ";
            szDesc1 += par[0];
            szDesc1 += " + ("; 
            szDesc1 += par[1];
            szDesc1 += " *m) i=";
//            szDesc1 += i;
      } 

        lat.DrawLatex(minX+(maxX-minX)*0.2,minY+(maxY-minY)*0.7,szDesc1.Data());

      szDesc2 = "b = ";
      szDesc2 += par[1];
        lat.DrawLatex(minX+(maxX-minX)*0.2,minY+(maxY-minY)*0.6,szDesc2.Data());

//      }
   }


   if( szDescX && szDescX[0] ){
      pGraph->GetHistogram()->SetXTitle( szDescX );
   }else{
      pGraph->GetHistogram()->SetXTitle("Number of field observations");
   }
   if( szDescY && szDescY[0] ){
       pGraph->GetHistogram()->SetYTitle( szDescY );
   }else{
      pGraph->GetHistogram()->SetYTitle("Number of new stars");
   }
   // pGraph->GetHistogram()->SetYTitle("sigmaG1_homeo/SigmaG1");

   pGraph->GetXaxis()->SetTitleOffset(1.00);
   pGraph->GetYaxis()->SetTitleOffset(0.40); // 1.00 for paper
   pGraph->GetXaxis()->SetTitleSize(0.1); // 0.05 for paper
   pGraph->GetXaxis()->SetLabelSize(0.1);
   pGraph->GetYaxis()->SetTitleSize(0.1); // 0.02 for paper
   pGraph->GetYaxis()->SetLabelSize(0.08);


   TLatex lat;
   lat.SetTextAlign(23);
   lat.SetTextSize(0.05);
                                                                                
//   char szDesc[1024];
//   sprintf(szDesc,"sigma/mean = %.8f\n",r);
//   lat.DrawLatex( (minX+maxX)/2 , minY+(maxY-minY)/5 , szDesc);

/*   if( numVal>=10 ){
      TString szOutFile=fname;
      szOutFile += "_sigma.out";
      FILE* out_file = fopen( szOutFile.Data(),"w");
      fprintf(out_file,"%.8f %.8f %.8f %.8f\n",norm,mean,sigma,r);
      fclose(out_file);
   }*/

   
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

   all=0;
   int ncols=-1;
   while (1) {
      if(fgets(buff,lSize,fcd)==0)
         break;

      if(buff[0]=='#')
         continue;      
      if(strstr(buff,"nan"))
         continue;

//      printf("Line = |%s|\n",buff);

      //  9.39  377.000000  8.000000 328.757587  77.088256   298.312992 65.223146   44.506926
      // ncols = sscanf(buff,"%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f",&mag,&x,&y,&fval1,&fval2,&fval3,&fval4,&fval5);
      Double_t x_val;
      Double_t y_val;
//      ncols = sscanf(buff,"%f\t%f\n",&x_val,&y_val);

      int ncols=0;
      char* ptr=NULL;
      char* search_ptr=buff;
      int col=0;
      while( ptr = strtok(search_ptr," \t") ){
         search_ptr = NULL;
         if( gVerb ){
              printf("ptr = %s\n",ptr);
                        }

         if( col == x_col ){
            if( strstr(ptr,"nan") || strstr(ptr,"NAN") ){
               break;
            }

            x_val = atof( ptr );
            ncols++;
         }
         if( col == y_col ){
            if( strstr(ptr,"nan") || strstr(ptr,"NAN") ){
               break;
            }

            y_val = atof( ptr );
            ncols++;
         }
         col++;
      }


      if( ncols < 2 ){
         // changed due to stupid plots , single column rows are skiped
         printf("ERROR - less then 2 columns, line |%s| skiped !!!\n",buff);
         printf("%s\n",buff);
         continue;
         // y_val = x_val;
         // x_val = all;
      }

      if (ncols < 0){
         printf("Line |%s| has no columns -> skipped\n",buff);
         break;
      }
     
     if(CondCol>=0){
          if(lval1!=CondValue)
            continue;                                        
     }
     // printf("%s\n",buff);

     if( x_val < gMinFreq || x_val > gMaxFreq ){
        if( gVerb > 0 || 1 ){
           printf("Frequency %.2f outside range %e - %e\n",x_val,gMinFreq,gMaxFreq);
        }
        continue;
     }
     
     x_values[all] = x_val;
     y_values[all] = y_val;

     if( y_val < gMinAllowedValue ){
        printf("SKIPPED due to y_val < %.4f : %.8f %.8f\n",gMinAllowedValue,x_val,y_val);
        continue;
     }
     if( gDb2Num ){
         y_values[all] = TMath::Power(10.00,(y_val/10.00));
     }
     if( gVerb ){
           printf("values : %f %f\n",x_val,y_val);
          }

     all++;
   }
   fclose(fcd);

//   exit(0);   
   return all;
}  

int find_max( Double_t* x_value, Double_t* y_value, int count, double t0, double dt, int i_start=0 )
{
   double max_val=-1e20;
   int max_val_i = -1;

   for(int i=i_start;i<count;i++){
      double t = x_value[i];
      double diff = fabs(t - t0 );
      if ( diff <= dt ){
         if ( y_value[i] > max_val ){
            max_val = y_value[i];
            max_val_i = i;
         }
      }else{
         if( t > (t0+1.5*dt) ){
            // do not go too far after t0 as it has to be maximum for a
            // specific pulse - not later pulses or other RFI events ...
            break;
         }
      }
   }

   return max_val_i;
}

// assumes that data are rescaled, that is : val_prim = (val-mean)/rms - so
// that the values are expressed in SNR 
int count_detected_pulses( Double_t* x_value, Double_t* y_value, int count, Double_t* pulse_arrival_times, int pulse_count, double threshold=5.00, Double_t* x_cand=NULL, Double_t* y_cand=NULL, int cand_count=0 )
{
   int detected_pulse_count=0;
   int* detected_pulses = new int[pulse_count]; // maximum as many as expected pulses
   double pulse_period = pulse_arrival_times[1] - pulse_arrival_times[0];
   int last_max_i = 0;
   FILE* out_f = fopen("detected_pulses.txt","w");

   printf("INFO : counting detected pulses above threshold SNR >= %.3f\n",threshold);

   double min_time = x_value[10];

   for(int p=0;p<pulse_count;p++){
      if( pulse_arrival_times[p] > min_time && pulse_arrival_times[p] > 1.00  ){ // skip first 5 timesteps as there are strange effects there:
         int max_i = find_max( x_value, y_value, count, pulse_arrival_times[p], 1*pulse_period, last_max_i );

         if( max_i > 0 ){
             last_max_i = max_i;
             if( y_value[max_i] > threshold ){
                printf("\tPulse at %.6f [sec] , SNR = %.2f\n",x_value[max_i],y_value[max_i]);
                fprintf(out_f,"%8f %.4f -1\n",x_value[max_i],y_value[max_i]);
                detected_pulses[detected_pulse_count] = max_i;
                detected_pulse_count++;
             }
         }       
      }else{
        printf("Pulse at %.6f skipped (below %.6f)\n",pulse_arrival_times[p],min_time);
      }
   }

   printf("Detected %d pulses\n",detected_pulse_count);

   if( cand_count > 0 ){
      last_max_i = -1;
      printf("Checking if detected pulses were also found by FREDDA:\n");
      int fredda_detected=0;
      for(int i=0;i<detected_pulse_count;i++){
         double pulse_time = x_value[ detected_pulses[i] ];
         double snr_msok   = y_value[ detected_pulses[i] ];

         printf("Checking FREDDA detection of pulse %d at index = %d , time = %.4f [sec] : ",i,detected_pulses[i],pulse_time);
         int max_i = find_max( x_cand, y_cand, cand_count, pulse_time, 1*pulse_period, last_max_i );

         if( max_i > 0 ){
            last_max_i = max_i;

//            if( y_cand[max_i] > 10.00 ){ // fredda threshold 10 sigma
            fredda_detected++; 
//            }
            double snr_fredda = y_cand[max_i];
            printf(" OK ( snr_msok = %.4f , snr_fredda = %.4f )\n",snr_msok,snr_fredda);
         }else{
            printf("NOT FOUND ( snr_msok = %.4f ) \n",snr_msok);
         }
      }

      printf("FREDDA detected %d / %d pulses (%.2f %%)\n",fredda_detected,detected_pulse_count,100.0*double(fredda_detected)/double(detected_pulse_count));
   }

   fclose(out_f);

   return detected_pulse_count;
}

double find_freq( Double_t* x_value, Double_t* y_value, int count, double norm_freq )
{
   int best_i=-1;
   double min_dist=100000;
   for(int i=0;i<count;i++){
      if( fabs(x_value[i] - norm_freq) < min_dist ){
         min_dist = fabs(x_value[i] - norm_freq);
         int best_i=i;
      }
   }

   if( best_i < count && best_i>=0 ){
      return y_value[best_i];
   }

   printf("ERROR : could not find closest frequency for %.2f [MHz]\n",norm_freq);
   return 1;
}

// norm at 55.7 
void plot_samples_with_candidates( const char* basename="sigmaG1_vs_lapSigmaG1_for_root", 
                          const char* basename2="dm57_cand_vs_time.txt",         
                          const char* basename3=NULL, // "dm56_cand_vs_time.txt",
                          const char* cand_file=NULL, // 1369650000_20230601101942_ch120_02_sorted.cand                          
                          const char* oper=NULL, double min_x=-1e20, double max_x=1e20,
                          const char* fit_func_name=NULL, double min_y=-10000, 
                          double max_y=-10000 , double min_allowed_value=-1e20,
                          int sleep_time=-1,
                int bLog=0,
      const char* szDescX="Time [ms]",const char* szDescY="SNR", const char* szTitle="", const char* szOutDir="images/",
      double fit_min_x=-100000, double fit_max_x=-100000,
      int x_col=0, int y_col=1, double norm_freq=55.7 )
//  double min_x=-1e20, double max_x=1e20 )
{
   int bShowLegend = 0;
   gMinAllowedValue = min_allowed_value;
   if( !szTitle){
      szTitle = basename;
   }
   gLog = bLog;
   gMinFreq = min_x;
   gMaxFreq = max_x;

   
   // gROOT->Reset();
   // const char* basename = "s_vs_sigma_g_sqr";

   TCanvas* c1 = new TCanvas("c1","xxxx",200,10,2000,500);
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

   Double_t* x_value1 = new Double_t[MAX_ROWS];
   Double_t* x_value2 = new Double_t[MAX_ROWS];
   Double_t* x_value3 = new Double_t[MAX_ROWS];
   Double_t* x_value_cand = new Double_t[MAX_ROWS];
   Double_t* y_value1 = new Double_t[MAX_ROWS];
   Double_t* y_value2 = new Double_t[MAX_ROWS];
   Double_t* y_value3 = new Double_t[MAX_ROWS];
   Double_t* y_value_cand = new Double_t[MAX_ROWS];
   Double_t maxX=-100000,maxY=-100000;   
   Double_t minX=100000,minY=100000;

   Int_t ncols;
   Int_t lq1=0,lq2=0,lq3=0,lq5=0,lq9=0,lq25=0;



   lq1 = ReadResultsFile( basename, x_value1, y_value1, -1, -1, x_col, y_col ); 
   printf("Read %d points from file %s\n",lq1,basename);

   TString szTitleFinal = szTitle;
//   szTitleFinal += " / ";   
//   szTitleFinal += basename2;

   TMultiGraph *mg = new TMultiGraph();
   int g=0;
   TGraph* graphs[1000];
// mg->SetMinimum(1);  
   if( min_y > -10000 ){
      mg->SetMinimum( min_y );
   }
   if( max_y > -10000 ){
      mg->SetMaximum( max_y );
   }

   TLegend *legend = new TLegend(.35,0.8,0.65,0.95);
   legend->SetTextFont(72);
   legend->SetTextSize(0.08); // 0.05 - large for eps-es to papers 
   legend->SetFillStyle(1001);



   // drawing background graphs here :
   TGraph* pGraph1 = DrawGraph( x_value1, y_value1, lq1, 1, NULL, kBlack, "AL", 1, kFullCircle,
              fit_func_name, min_y, max_y, szTitleFinal.Data(),
            basename, bLog, szDescX, szDescY, fit_min_x,
            fit_max_x );
   // 
//   pGraph1->GetYaxis()->SetTitleOffset(-0.7);
//   pGraph1->GetYaxis()->SetLabelOffset(-0.03);
   mg->Add(pGraph1);
   legend->AddEntry(pGraph1,"J0837-4135 no pulse","P");// basename


   szTitleFinal = basename;
   szTitleFinal += " / ";
   szTitleFinal += basename2;
//   pad2->cd();
   lq2 = ReadResultsFile( basename2, x_value2, y_value2, -1, -1, x_col, y_col );
   printf("Read %d points from file %s\n",lq2,basename2);

   TGraph* pGraph2 = DrawGraph( x_value2, y_value2, lq2, 1, NULL, kRed, "P,same", 3, kOpenDiamond, // 5, kFullDiamond, // kFullTriangleDown,
              fit_func_name, min_y, max_y, szTitleFinal.Data(),
            basename, bLog, szDescX, szDescY, fit_min_x,
            fit_max_x );

   lq3 = ReadResultsFile( basename3, x_value3, y_value3, -1, -1, x_col, y_col );
   printf("Read %d points from file %s\n",lq3,basename3);

   TGraph* pGraph3 = DrawGraph( x_value3, y_value3, lq2, 1, NULL, kBlue, "P,same", 3, kOpenCircle, // 5, kFullDiamond, // kFullTriangleDown,
              fit_func_name, min_y, max_y, szTitleFinal.Data(),
            basename, bLog, szDescX, szDescY, fit_min_x,
            fit_max_x );


   int cand_count = 0;
   if( cand_file ){
      printf("Reading candidate file %s\n",cand_file);
      cand_count = ReadResultsFile( cand_file, x_value_cand, y_value_cand, -1, -1, 0, 1 );
      printf("Read %d candidates\n",cand_count);     
   }else{
      printf("WARNING : candidate file (3rd parameter) not provided\n");
   }

   // count detected pulses :
   // int count_detected_pulses( Double_t* x_value, Double_t* y_value, int count, Double_t* pulse_arrival_times, int pulse_count, double threshold=5.00 )
//   int pulse_count = count_detected_pulses( x_value1, y_value1, lq1, x_value2, lq2, 5.00, x_value_cand, y_value_cand, cand_count );
   TGraph* pGraph4 = DrawGraph( x_value_cand, y_value_cand, cand_count, 1, NULL, kGreen, "P,same", 3, kMultiply, // 5, kFullDiamond, // kFullTriangleDown,
              fit_func_name, min_y, max_y, szTitleFinal.Data(),
            basename, bLog, szDescX, szDescY, fit_min_x,
            fit_max_x );


   c1->Update();

   TString szEpsName1=szOutDir;
   szEpsName1 += basename;
   szEpsName1 += ".eps";
   c1->Print(szEpsName1.Data());

   TString szPngName1=szOutDir;
   szPngName1 += basename;
   szPngName1 += ".png";
   c1->Print(szPngName1.Data());



   if( sleep_time > 0 ){
      gSystem->Sleep(1000*sleep_time);
   }
}
