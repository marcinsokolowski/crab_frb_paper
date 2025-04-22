// cd ~/Desktop/SKA/papers/2024/EDA2_FRBs/PAPER/DM_vs_TIME
// root [0] .x plot_tau_multifiles.C("McKee2018_Tau610MHz_Err.txt","tau610MHz.txt_histo.txt")
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

// always verify line :
// gROOT->Reset();

#define MAX_ROWS 20000

int g_show_freq=0;
const char* szDescFreq = "Freq [MHz]";
int gDb=1;

// frequency range :
double gMinFreq=-100.00;
double gMaxFreq=1e20;

char gOutResidFile[1024],gOutResidRatioFile[1024],gOutFitFileBEDLAM[1024];

double gBedlamQualityLimitDBM=-37; // see document logbook/201403/tone_test_vs_ORBCOMM.pdf (-100dBm + 63dB gain ~= -37dBm at the BEDLAM input see also bedlam_calibration_tone.odt/pdf )
// The more strict limit would be -110dBm which is super safe then -110 + 63 = -47dBm :
// double gBedlamQualityLimitDBM=-47;

int gNormX=0;
int gNormY=0;

double gMeanFreq=0.00;
TString gOutFileName;
char gOutFitFile[512];

double power(double x,double y)
{
   return exp(y*log(x));
}                    
                     
double dbm2mW( double in_dbm )
{
   double l = in_dbm/10.00;
   double mW = power(10,l);

   return mW;
}

const char* getbasename_new(const char* name,char* out)
{
  char outtmp[1024];
  int i=0;
  memset(outtmp,'\0',1024);
  out[0] = '\0';


  i = strlen(name)-1;
  while( name[i]!='.' && i>=0 ){
    i--;
  }
  if( i>=0 ){
    strncpy(outtmp,name,i);
    strcpy(out,outtmp);
  }

  return out;
}

const char* change_ext(const char* name,const char* new_ext,char* out)
{
   char tmp_file[1024];
   getbasename_new(name,tmp_file);

        sprintf(out,"%s.%s",tmp_file,new_ext);

        return out;
}



Double_t poly( Double_t freq, Double_t* y, Double_t n_par_count )
{
   Double_t ret=0.00;
//   int n = (n_par_count-1)/2;
   int n = n_par_count;

   double pow=1.00;
   for(int i=0;i<n;i++){
      double next = pow*y[i];

      ret += next;
      pow = pow*freq;
   }

   return ret;
}

Double_t RadTest( Double_t* x, Double_t* y )
{
   return y[0]/sqrt(x[0]);
}


#define N_PAR_COUNT 10
Double_t spectrum_model( Double_t freq )
{
   Double_t par[50];
   for(int i=0;i<50;i++){
      par[i] = 0.00;
   }

/* par[0]                        = -3.52364e+12   ; // +/-    1.041e+12     
   par[1]                        =  4.13935e+11   ; // +/-    6.51364e+10   
   par[2]                        = -1.02092e+10   ; // +/-    1.7242e+09    
   par[3]                        =  1.23619e+08   ; // +/-    2.53996e+07   
   par[4]                        =      -775448   ; // +/-    230140        
   par[5]                        =      2199.82   ; // +/-    1334.28       
   par[6]                        =     0.427662   ; // +/-    4.96585       
   par[7]                        =   -0.0193973   ; // +/-    0.0114757     
   par[8]                        =  4.62558e-05   ; // +/-    1.49857e-05   
   par[9]                        = -3.58946e-08   ; // +/-    8.44799e-09 */

// cd /media/BighornsData/labtest/tone_test/20140311/ZVL_comparison_100MHz_60sec/ZVL_100mhz_avg10000
// modelspec_poly acc32000_ch0_20140311_023648_000005.ZVL_CALIB_FIT
par[0] = 4722515362706.85351562500000000000000000000000000000000000000000;
par[1] = -95027966093.02354431152343750000000000000000000000000000000000;
par[2] = 2858485415.97554826736450195312500000000000000000000000000000;
par[3] = -60083923.19348197430372238159179687500000000000000000000000;
par[4] = 788743.02736626658588647842407226562500000000000000000000;
par[5] = -6199.39397118529541330644860863685607910156250000000000;
par[6] = 28.97252980372121555774356238543987274169921875000000;
par[7] = -0.07877571963827538492619595444921287707984447479248;
par[8] = 0.00011499439127208088760705162467701256900909356773; 
par[9] = -0.00000006966022305926903079173408313284898696338132;


   Double_t ret=0.00;  
   int n = N_PAR_COUNT;

   double pow=1.00;
   for(int i=0;i<n;i++){
      double next = pow*par[i];
      
      ret += next;   
      pow = pow*freq;
   }

   return ret;
}


Double_t HorizontalLine( Double_t* x, Double_t* y )
{
   return y[0];
}

Double_t Line( Double_t* x, Double_t* y )
{
   Double_t a = y[1];
   Double_t b = y[0];

   //a = (483.269-33121.78)/(68.97-841.87);
   //b = 483.269 - a*68.97;

   return (x[0]*a+b);
   // return (x[0]*y[0]);
}

TGraphErrors* DrawGraph( Double_t* x_values, Double_t* y_values, Double_t* x_values_errors, Double_t* y_values_errors,
         int numVal, 
         long q, TPad* pPad, const char* fit_func_name=NULL, 
         double min_y=-1e9, double max_y=-1e9,
         const char* szStarName="", const char* fname="default",
         int bLog=0, const char* szDescX=NULL, const char* szDescY=NULL,
         double fit_min_x=-100000, double fit_max_x=-100000,
         const char* szOPT="AP", int ColorNum = kBlack, int MarkerType = 20 )
{
    Double_t z,sigma_z,const_part;
    TF1 *line = NULL;
    TF1 *line_draw = NULL;
   TF1 *part1 = NULL;
   TF1 *part2 = NULL;
    Double_t maxX=-100000,maxY=-100000;
    Double_t minX=100000,minY=100000;


    TGraphErrors* pGraph = new TGraphErrors(q);
    for(int i=0;i<numVal;i++){
//        printf("q=%d %f %f\n",q, x_values[i], y_values[i] );

        double x_val = x_values[i];
//        if( g_show_freq ){
//           x_val = (480.00/4096)*x_values[i];
//        }

        pGraph->SetPoint( i, x_val, y_values[i] );
        if( y_values_errors ){
           double x_err = 0;
           if ( x_values_errors ){
              x_err = x_values_errors[i];
           }
           pGraph->SetPointError( i, x_err, y_values_errors[i] );
        }


        if(x_val>maxX)
            maxX = x_val;
        if(y_values[i]>maxY)
            maxY = y_values[i];
      
        if(x_val<minX)
            minX = x_val;
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
/*
   TH1F*  histo = new TH1F("adu_histo","adu_histo",50,minY,maxY);
   for(int i=0;i<numVal;i++){
      histo->Fill( y_values[i] );
   }
   histo->Fit("gaus","0");
   norm = histo->GetFunction("gaus")->GetParameter(0);
   mean = histo->GetFunction("gaus")->GetParameter(1);
   sigma = histo->GetFunction("gaus")->GetParameter(2);
   r = (sigma/mean);
                                                                                
   printf("Gauss_norm = %.8f\n",norm);
   printf("Gauss_sigma = %.8f\n",sigma);
   printf("Gauss_mean = %.8f\n",mean);
   printf("sigma/mean = %8f\n",(sigma/mean));
*/


    pGraph->SetMarkerStyle(MarkerType);
    pGraph->SetMarkerColor(ColorNum);
    pGraph->SetLineColor(ColorNum);
    pGraph->SetMarkerSize(2);
    if( min_y>-1e9 && max_y>-1e9 ){
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
//    pGraph->Draw("pA");
    pGraph->Draw(szOPT);

   if( fit_min_x<=-100000 ){
      fit_min_x = minX;
   }
   if( fit_max_x<=-100000 ){
      fit_max_x = maxX;
   }

   printf("fitting in x range (%.2f,%.2f)\n",fit_min_x,fit_max_x);

   int n_params=0;
   int local_func=0;
   if( fit_func_name && strlen(fit_func_name) ){
      if( strcmp( fit_func_name, "horline" )==0 || fit_func_name[0]=='h' ){
         printf("Fitting horizontal line\n");
         line = new TF1("fit_func",HorizontalLine,fit_min_x,fit_max_x,1);
         line_draw = new TF1("fit_func2",HorizontalLine,minX,maxX,1);
         local_func=1;         
         n_params=1;
      }
      if( strcmp( fit_func_name, "line" )==0 || fit_func_name[0]=='l' ){
         printf("Fitting straight line\n");
         line = new TF1("fit_func",Line,fit_min_x,fit_max_x,2);
         line_draw = new TF1("fit_func2",Line,minX,maxX,2);
         local_func=1;
         n_params=2;
      }
      if( strcmp( fit_func_name, "radtest" )==0 || fit_func_name[0]=='r' ){
         printf("Fitting straight line\n");
         line = new TF1("fit_func",RadTest,fit_min_x,fit_max_x,1);
         line_draw = new TF1("fit_func2",RadTest,minX,maxX,1);
         local_func=1;
         n_params=1;
      }


   }


   Double_t par[10];
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
               int pol_fit_order=-1;
               double chi2=0;

               if( strcmp(fit_func_name,"pol0")==0  ){
                   pGraph->Fit("pol0");
//                   par[0]=pol0->GetParameter(0);
                   chi2 = pGraph->GetFunction("pol0")->GetChisquare();
                    pol_fit_order=0;
                }

               if( strcmp(fit_func_name,"pol1")==0  ){
                   pGraph->Fit("pol1");
//                   par[0]=pol1->GetParameter(0);
//                   par[1]=pol1->GetParameter(1);   
                   chi2 = pGraph->GetFunction("pol1")->GetChisquare();
                   pol_fit_order=1;
                }

              if( strcmp(fit_func_name,"pol2")==0  ){
                   pGraph->Fit("pol2");
//                   par[0]=pol2->GetParameter(0);
//                   par[1]=pol2->GetParameter(1);
//                   par[2]=pol2->GetParameter(2);
                   chi2 = pGraph->GetFunction("pol2")->GetChisquare();
                   pol_fit_order=2;
                }

              if( strcmp(fit_func_name,"pol3")==0  ){
                   pGraph->Fit("pol3");
                   par[0]=pGraph->GetFunction("pol3")->GetParameter(0);
                   par[1]=pGraph->GetFunction("pol3")->GetParameter(1);
                   par[2]=pGraph->GetFunction("pol3")->GetParameter(2);
                   par[3]=pGraph->GetFunction("pol3")->GetParameter(3);
                   chi2 = pGraph->GetFunction("pol3")->GetChisquare();
                   pol_fit_order=3; 
                }

              if( strcmp(fit_func_name,"pol4")==0  ){
                   pGraph->Fit("pol4");
                   par[0]=pGraph->GetFunction("pol4")->GetParameter(0);
                   par[1]=pGraph->GetFunction("pol4")->GetParameter(1);
                   par[2]=pGraph->GetFunction("pol4")->GetParameter(2);
                   par[3]=pGraph->GetFunction("pol4")->GetParameter(3);
                   par[4]=pGraph->GetFunction("pol4")->GetParameter(4);
                   chi2 = pGraph->GetFunction("pol4")->GetChisquare();
                   pol_fit_order=4;
                }


              if( strcmp(fit_func_name,"pol5")==0  ){
                   pGraph->Fit("pol5");
                   par[0]=pGraph->GetFunction("pol5")->GetParameter(0);
                   par[1]=pGraph->GetFunction("pol5")->GetParameter(1);
                   par[2]=pGraph->GetFunction("pol5")->GetParameter(2);
                   par[3]=pGraph->GetFunction("pol5")->GetParameter(3);
                   par[4]=pGraph->GetFunction("pol5")->GetParameter(4);
                   par[5]=pGraph->GetFunction("pol5")->GetParameter(5);
                   chi2 = pGraph->GetFunction("pol5")->GetChisquare();
                   pol_fit_order=5; 
                }

                if( pol_fit_order < 0 && strncmp(fit_func_name,"pol",3)==0 ){
                   pol_fit_order=atol(fit_func_name+3);
                   pGraph->Fit( fit_func_name );
                   for(int i=0;i<=pol_fit_order;i++){
                      par[i]=pGraph->GetFunction(fit_func_name)->GetParameter(0);
                   }
                   chi2 = pGraph->GetFunction(fit_func_name)->GetChisquare();
                  }


                if( pol_fit_order >=0 ){
                   pGraph->GetFunction(fit_func_name)->GetParameters(par);
                   printf("Parameters of fitted polynomial:\n"); for(int
                   i=0;i<(pol_fit_order+1);i++){
                      printf("\t%.20f\n",par[i]);
                   }

                   printf("ROOT FUNC : \n");
                   printf("-----------------------------\n");
                   printf("TF1* pFunc2 = new TF1(\"user2\",\"");
                   for(int i=0;i<(pol_fit_order+1);i++){
                        printf(" %+.50f * x^%d",(double)(par[i]),i);
                   }
                   printf("\",%.2f,%.2f);",minX,maxX);
                   printf("\n");  


                   double sum=0.00;
                   double sum2=0.00;
                   FILE* out_file = fopen( gOutResidFile,"w");                  
                   FILE* out_file2 = fopen( gOutResidRatioFile,"w");
                   FILE* out_fit_bedlam = fopen( gOutFitFileBEDLAM , "w" );
                   for(int i=0;i<numVal;i++){
                      double freq = x_values[i];
                      double real_val = y_values[i];
                      double fit_val = poly(freq,par,(pol_fit_order+1));
                      double diff = (real_val-fit_val);
                      double diff_ratio = diff/fit_val;
                      fprintf(out_file,"%.8f %.20f %.20f %.20f %.20f\n",freq,diff,real_val,fit_val,diff_ratio);
                      fprintf(out_file2,"%.8f %.20f\n",freq,diff_ratio);
  
                      sum += diff;
                      sum2 += diff*diff;
                   }
                   for(int i=0;i<4096;i++){
                      double freq = i*(480.00/4096);
                      double fit_val = poly(freq,par,(pol_fit_order+1));

                      fprintf(out_fit_bedlam,"%.8f %.20f\n",freq,fit_val);
                   }
                   fclose(out_file);
                   fclose(out_file2);
                   fclose(out_fit_bedlam);
                   printf("Fit results written to output files : %s and %s and %s\n",gOutResidFile,gOutResidRatioFile,gOutFitFileBEDLAM);

                  double rms = sqrt(sum2/numVal - (sum/numVal)*(sum/numVal));
                  FILE* out_chifile = fopen( "out_chi2.txt" ,"a+" );
                  fprintf(out_chifile,"%s %.8f %.8f %.8f\n",gOutResidFile,chi2,(sum/numVal),rms);
                  printf("FILE/CHI2/MEAN/RMS = %s %.8f %.8f %.8f\n",gOutResidFile,chi2,(sum/numVal),rms);
                  fclose(out_chifile);
                }

//      if( strstr(fit_func_name,"line") || fit_func_name[0]=='l' || fit_func_name[0]=='L'
//          || fit_func_name[0]=='h' || fit_func_name[0]=='H' ){
      if( fit_func_name && strlen(fit_func_name) ){      
         pGraph->Fit("fit_func","R");


         Double_t fit_params[10];
         line->GetParameters(fit_params);
         FILE* outfit = fopen(gOutFitFile,"w");    
         for(int i=0;i<n_params;i++){
            fprintf(outfit,"%.8f %.8f\n",fit_params[i],line->GetParError(i));
         }
         fclose(outfit);
         printf("INFO : fit results written to file %s\n",gOutFitFile);
      }

      if( local_func ){
         line->GetParameters(par);
//         line->Draw("same");
         line_draw->SetParameters(par);
         line_draw->Draw("same");
      }


      if( strlen(gOutFileName.Data()) && strcmp( fit_func_name, "radtest" )==0 ){
         FILE* out_f = fopen(gOutFileName.Data(),"a");
         fprintf(out_f,"%.8f %.8f\n",gMeanFreq,par[0]);
         fclose(out_f);
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

//        lat.DrawLatex(minX+(maxX-minX)*0.2,minY+(maxY-minY)*0.7,szDesc1.Data());

      szDesc2 = "b = ";
      szDesc2 += par[1];
//        lat.DrawLatex(minX+(maxX-minX)*0.2,minY+(maxY-minY)*0.6,szDesc2.Data());

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
   pGraph->GetYaxis()->SetTitleOffset(1.20);
   pGraph->GetXaxis()->SetTitleSize(0.05);
   pGraph->GetXaxis()->SetLabelSize(0.05);
   pGraph->GetYaxis()->SetTitleSize(0.05);
   pGraph->GetYaxis()->SetLabelSize(0.05);


   TLatex lat;
   lat.SetTextAlign(23);
   lat.SetTextSize(0.05);
                                                                                
   char szDesc[255];
   sprintf(szDesc,"sigma/mean = %.8f\n",r);
//   lat.DrawLatex( (minX+maxX)/2 , minY+(maxY-minY)/5 , szDesc);

   if( numVal>=10 ){
      TString szOutFile=fname;
      szOutFile += "_sigma.out";
      FILE* out_file = fopen( szOutFile.Data(),"w");
      fprintf(out_file,"%.8f %.8f %.8f %.8f\n",norm,mean,sigma,r);
      fclose(out_file);
   }

   
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
//         printf("ptr = %s\n",ptr);

         if( col == x_col ){
            x_val = atof( ptr );
            ncols++;
         }
         if( col == y_col ){
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
     // printf("%s\n",buff);
     
     x_values[all] = x_val;

     if( x_val < gMinFreq || x_val > gMaxFreq )
        continue;

     if( gDb ){
        y_values[all] = 10.00*TMath::Log10(y_val);
     }else{
        y_values[all] = y_val;
     }

     if( gNormX > 0 ){
        x_values[all] = x_val / 480.00;
     }
     if( gNormY > 0 ){
        y_values[all] = y_val / 300;
     }

//     printf("values : %f %f\n",x_val,y_val);

     all++;
   }
   fclose(fcd);

//   exit(0);   
   return all;
}  

const char* parse_legend( const char* basename, char* szLegend )
{
   int month;
   strcpy(szLegend,basename);
   if( sscanf(szLegend,"%d_tau_vs_avglocal.txt",&month)==1 || sscanf(szLegend,"%d_tau_vs_avglocal.txt.SAV",&month)==1 ){
      sprintf(szLegend,"%d",month);
   }   
   return szLegend;
}


void plot_tau_multifiles( const char* basename="McKee2018_Tau610MHz_Err.txt", 
               const char* basename2="tau610MHz.txt_histo.txt",
               const char* basename3=NULL,
               const char* basename4=NULL,
               const char* fit_func_name=NULL, 
               int sleep_time=-1,
               int bShowQualityLimits=0,
               double min_x=0.00, double max_x=0.7,
               double min_y=0, 
               double max_y=0.5 , int bDb=0,
      const char* szDescX="Tau [ms]",const char* szDescY="Fraction", const char* szTitle=NULL,
      const char* szExtraInfo=NULL,
      double fit_min_x=-100000, double fit_max_x=-100000,
      int x_col=0, int y_col=2, 
      double dMeanFreq=0.00, const char* szOutFileName=NULL, int bLogXY=0,
      const char* szOutImageType="png", const char* szOutDir="images/" )
{
   change_ext(basename,"resid",gOutResidFile);
   change_ext(basename,"resid_ratio",gOutResidRatioFile);
   change_ext(basename,"fit_bedlam",gOutFitFileBEDLAM);
   change_ext(basename,"fit_params",gOutFitFile);
   gMinFreq = min_x;
   gMaxFreq = max_x;
   if( szOutFileName ){
      gOutFileName = szOutFileName;
   }
   gMeanFreq = dMeanFreq;

   TString gPlotTitle="Data from file : ";
   if( !szTitle){
      gPlotTitle = gPlotTitle + basename;
      szTitle = gPlotTitle.Data();
   }
   gDb = bDb;
//   if( show_freq ){
//      szDescX = szDescFreq;
//   }
   
   // gROOT->Reset();
   // const char* basename = "s_vs_sigma_g_sqr";

   TCanvas* c1 = new TCanvas("c1","xxxx",200,10,700,500);
   c1->SetGridx();
   c1->SetGridy();
   c1->SetFillColor(0);
   c1->SetFillStyle(0);
   if( bLogXY ){
      c1->SetLogy(1);
      c1->SetLogx(1);
   }
//   if( !gDb ){
//      szDescY="Power [?]";
//   }
   
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

   


//   const Int_t MAX_ROWS = 200000;
   Double_t x_value1[MAX_ROWS],x_value2[MAX_ROWS],x_value3[MAX_ROWS],x_value4[MAX_ROWS],x_value5[MAX_ROWS],x_value6[MAX_ROWS];
   Double_t y_value1[MAX_ROWS],y_value2[MAX_ROWS],y_value3[MAX_ROWS],y_value4[MAX_ROWS],y_value5[MAX_ROWS],y_value6[MAX_ROWS];
   Double_t x_value1_err[MAX_ROWS],y_value1_err[MAX_ROWS];
   Double_t x_value2_err[MAX_ROWS],y_value2_err[MAX_ROWS];
   Double_t x_value3_err[MAX_ROWS],y_value3_err[MAX_ROWS];
   Double_t x_value4_err[MAX_ROWS],y_value4_err[MAX_ROWS];
   Double_t maxX=-100000,maxY=-100000;   
   Double_t minX=100000,minY=100000;
   Int_t ncols;

   TMultiGraph *mg = new TMultiGraph();
   int g=0;
   TGraphErrors* graphs[1000];
// mg->SetMinimum(1);
   if( min_y > -10000 ){
      mg->SetMinimum( min_y );
   }
   if( max_y > -10000 ){
      mg->SetMaximum( max_y );
   }

   TLegend *legend = new TLegend(.05,0.8,0.55,0.95);
   legend->SetTextFont(72);
   legend->SetTextSize(0.05); // 0.05 - large for eps-es to papers



   // file 1 required :   
   int lq1 = ReadResultsFile( basename, x_value1, y_value1, -1, -1, x_col, y_col ); 
   Int_t lq1_err = ReadResultsFile( basename, x_value1_err, y_value1_err, -1, -1, x_col+1, y_col+1 );
   printf("Read %d points and %d errors from file %s\n",lq1,lq1_err,basename);
   if( lq1 != lq1_err ){
      printf("ERROR : unequal numbers of points and errors (%d != %d) -> exiting script now !\n",lq1,lq1_err);
      return;
   }   
   // drawing background graphs here :
   TGraphErrors* pGraph1 = DrawGraph( x_value1, y_value1, x_value1_err, y_value1_err, lq1, 1, NULL, 
              fit_func_name, min_y, max_y, szTitle,
            basename, 0, szDescX, szDescY, fit_min_x,
            fit_max_x, "AP", kBlack, 20 );
   mg->Add(pGraph1);
	char szLegend[64];
	parse_legend( basename, szLegend );
   legend->AddEntry(pGraph1,szLegend,"P");// basename

   if( basename2 ){
      // file 1 required :   
      int lq2 = ReadResultsFile( basename2, x_value2, y_value2, -1, -1, x_col, y_col ); 
      Int_t lq2_err = ReadResultsFile( basename2, x_value2_err, y_value2_err, -1, -1, x_col+1, y_col+1 );
      printf("Read %d points and %d errors from file %s\n",lq2,lq2_err,basename2);
      if( lq2 != lq2_err ){
         printf("ERROR : unequal numbers of points and errors (%d != %d) -> exiting script now !\n",lq2,lq2_err);
         return;
      }   
      // drawing background graphs here :
      TGraphErrors* pGraph2 = DrawGraph( x_value2, y_value2, x_value2_err, y_value2_err, lq2, 1, NULL, 
              fit_func_name, min_y, max_y, szTitle,
            basename, 0, szDescX, szDescY, fit_min_x,
            fit_max_x, "P,same", kBlue, 21 );
      mg->Add(pGraph2);
      parse_legend( basename2, szLegend );
      legend->AddEntry(pGraph2,szLegend,"P");// basename
   }

   if( basename3 ){
      // file 1 required :   
      int lq3 = ReadResultsFile( basename3, x_value3, y_value3, -1, -1, x_col, y_col ); 
      Int_t lq3_err = ReadResultsFile( basename3, x_value3_err, y_value3_err, -1, -1, x_col+1, y_col+1 );
      printf("Read %d points and %d errors from file %s\n",lq3,lq3_err,basename3);
      if( lq3 != lq3_err ){
         printf("ERROR : unequal numbers of points and errors (%d != %d) -> exiting script now !\n",lq3,lq3_err);
         return;
      }   
      // drawing background graphs here :
      TGraphErrors* pGraph3 = DrawGraph( x_value3, y_value3, x_value3_err, y_value3_err, lq3, 1, NULL, 
              fit_func_name, min_y, max_y, szTitle,
            basename, 0, szDescX, szDescY, fit_min_x,
            fit_max_x, "P,same", kGreen, 22 );
      mg->Add(pGraph3);
      parse_legend( basename3, szLegend );
      legend->AddEntry(pGraph3,szLegend,"P");// basename
   }

   if( basename4 ){
      // file 1 required :   
      int lq4 = ReadResultsFile( basename4, x_value4, y_value4, -1, -1, x_col, y_col ); 
      Int_t lq4_err = ReadResultsFile( basename4, x_value4_err, y_value4_err, -1, -1, x_col+1, y_col+1 );
      printf("Read %d points and %d errors from file %s\n",lq4,lq4_err,basename4);
      if( lq4 != lq4_err ){
         printf("ERROR : unequal numbers of points and errors (%d != %d) -> exiting script now !\n",lq4,lq4_err);
         return;
      }   
      // drawing background graphs here :
      TGraphErrors* pGraph4 = DrawGraph( x_value4, y_value4, x_value4_err, y_value4_err, lq4, 1, NULL, 
              fit_func_name, min_y, max_y, szTitle,
            basename, 0, szDescX, szDescY, fit_min_x,
            fit_max_x, "P,same", kRed, 23 );
      mg->Add(pGraph4);
      parse_legend( basename4, szLegend );
      legend->AddEntry(pGraph4,szLegend,"P");// basename
   }
   legend->Draw();


   // calculation of BEDLAM safety limit :
   Double_t bedlam_quality_limit[MAX_ROWS],bedlam_quality_limit_safe[MAX_ROWS];
   double gBedlamQualityLimit_mW = dbm2mW(gBedlamQualityLimitDBM);
   double average_quality_limit_bedlam_units = 0.00;
   double average_calib_constant = 0.00;
   int average_quality_limit_counter=0;
   double max_value=-100000.00;
   for(int i=0;i<lq1;i++){   
      double freq_mhz = x_value1[i];
      double calib_const = spectrum_model( freq_mhz); 
      bedlam_quality_limit[i] = gBedlamQualityLimit_mW * calib_const;
      bedlam_quality_limit_safe[i] = bedlam_quality_limit[i]/10.00;

      if( y_value1[i] > max_value ){
         max_value = y_value1[i];
      }

      if( 40<=freq_mhz && freq_mhz<=350 ){
         average_quality_limit_bedlam_units += bedlam_quality_limit[i];
         average_calib_constant += calib_const;
         average_quality_limit_counter++;
      }
   }
   average_quality_limit_bedlam_units = average_quality_limit_bedlam_units / average_quality_limit_counter;
   double average_quality_limit_bedlam_units_safe = average_quality_limit_bedlam_units/10.00; // 10dBm less is super safe limit 
   printf("Average bedlam quality limits %e - %e [BEDLAM UNITS]\n",average_quality_limit_bedlam_units_safe,average_quality_limit_bedlam_units);
   printf("Average calib constant = %e\n",(average_calib_constant/average_quality_limit_counter));

   if( szExtraInfo && strlen(szExtraInfo) ){
      TLatex lat;
      lat.SetTextAlign(23);
      lat.SetTextSize(0.05);
      lat.DrawLatex(250,4900,szExtraInfo);
   }
   
   c1->Update();

   char szPngFile[1024];
   change_ext(basename,szOutImageType,szPngFile);   
   TString szPngName1;
   if( szOutDir && strlen(szOutDir) ){
      szPngName1 = szOutDir;
   }
   szPngName1 += szPngFile;
   c1->Print(szPngName1.Data());

//   if( sleep_time > 0 ){
//      gSystem->Sleep(1000*sleep_time);
//   }   
}


