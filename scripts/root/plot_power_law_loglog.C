// always verify line :
// gROOT->Reset();
#include <TROOT.h>
#include <TF1.h>
#include <TH1F.h>
#include <TPaveText.h>
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


int gLog=0;
int gVerb=0;
double gUNIXTIME = -1;
int gFreqErrorsZero=0;
Double_t gFitParams[2];
Double_t gFreqRef = 215.00;

#define MAX_ROWS 10000000

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

Double_t power_law( Double_t* x, Double_t* y )
{
    Double_t freq_mhz=x[0];
    Double_t freq_ref = gFreqRef;

    Double_t ret = y[0]*TMath::Power( (freq_mhz/freq_ref) , (y[1]) );
//    printf("ret = %.8f\n",ret); 

    return ret;
}


TGraphErrors* DrawGraph( Double_t* x_values, Double_t* y_values, int numVal, 
         long q, TPad* pPad, const char* fit_func_name=NULL, 
         double min_y=-10000, double max_y=-10000,
         const char* szStarName="", const char* fname="default",
         int bLog=0, const char* szDescX=NULL, const char* szDescY=NULL,
         double fit_min_x=2.278753601, double fit_max_x=2.380211242, Double_t* x_values_err=NULL,  Double_t* y_values_err=NULL,
         int ColorNum = kRed, const char* szOPT="AP", bool bShowError=true  )
{
    int MarkerType = 20;
//    fit_min_x=0.00;
//    fit_max_x=1000.00;

    Double_t z,sigma_z,const_part;
    TF1 *line = NULL;
    TF1 *line_draw = NULL;
   TF1 *part1 = NULL;
   TF1 *part2 = NULL;
    Double_t maxX=-100000,maxY=-100000;
    Double_t minX=100000,minY=100000;


    TGraphErrors* pGraph = new TGraphErrors(q);
    for(int i=0;i<numVal;i++){
        if( gVerb || 1 ){
           printf("q=%d %f %f\n",i, x_values[i], y_values[i] );
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

        if( y_values_err ){
           double x_err = 0.00;
           pGraph->SetPointError( i, x_err, y_values_err[i] ); 
           printf("\terror = %.8f\n",y_values_err[i]);
        }
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
    pGraph->SetMarkerColor(ColorNum);
    pGraph->SetTitle( szStarName );
    pGraph->Draw( szOPT );

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
         line_draw = new TF1("fit_func2",Line,fit_min_x,fit_max_x,2);
         local_func=1;
      }
      if( strcmp( fit_func_name, "powerlaw" )==0 || strcmp( fit_func_name, "power_law" )==0 || fit_func_name[0]=='p' ){
         printf("Fitting power law\n");
         line = new TF1("fit_func",power_law,fit_min_x,fit_max_x,2);
         line_draw = new TF1("fit_func2",power_law,fit_min_x,fit_max_x,2);
         local_func=1;
      }
   }


   Double_t par[4];
   par[0] = 2.0;
   par[1] = -3.00; // -3.50;
   par[2] = 0.0;
   par[3] = 0.0;

   if( local_func ){
      line->SetParameters(par);
   }

   if( fit_func_name && strlen(fit_func_name) ){
      printf("fitting function : %s\n",fit_func_name);

      if( strcmp(fit_func_name,"gaus")==0 || fit_func_name[0]=='g' || fit_func_name[0]=='G' ){
         pGraph->Fit("gaus");
      }

      if( strstr(fit_func_name,"line") || fit_func_name[0]=='l' || fit_func_name[0]=='L'
          || fit_func_name[0]=='h' || fit_func_name[0]=='H' || fit_func_name[0]=='p' ){
         printf("DEBUG : fitting here ???\n");
         pGraph->Fit("fit_func","R,F,E,M,V");                
//         pGraph->Fit("fit_func","R");

         line->GetParameters(par);
         gFitParams[0] = par[0];
         gFitParams[1] = par[1];
      }

      if( local_func ){
         line->GetParameters(par);
//         line->Draw("same");
         line_draw->SetParameters(par);
         line_draw->Draw("same");
 
         double par0_err = line->GetParError(0);
         double par1_err = line->GetParError(1);

         double power_law_norm = TMath::Power(10.0,par[1])*TMath::Power(gFreqRef,par[0]);
         double power_law_norm_err = 0.00; // TODO :  needs to be propagated if needed !!!
         double power_law_index = par[0];
         double power_law_index_err = par0_err;
         

         FILE* outf = fopen("FIT.txt","w");
         fprintf(outf,"%.8f 0.00 %.8f %.8f %.8f %.8f\n",gUNIXTIME,power_law_index,power_law_index_err,power_law_norm,power_law_norm_err);
         fclose(outf);         
      }


      TLatex lat;
      lat.SetTextAlign(23);
      lat.SetTextSize(0.05);

      TString szA,szB,szConst,szDesc1,szDesc2;

      printf("minX=%.2f maxX=%.2f minY=%.2f maxY=%.2f\n",minX,maxX,minY,maxY);
//      if( strcmp(fit_func_name,"line")==0 || fit_func_name[0]=='l' || fit_func_name[0]=='L'){



//      }
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
                                                                                
   char szDesc[255];
   sprintf(szDesc,"sigma/mean = %.8f\n",r);
   
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
   double max_val=-100000;

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
     
     // sec -> ms
     y_val = y_val*1000.00;

     x_values[all] = x_val;
     y_values[all] = y_val; // *1000.00;
     if( y_val > max_val ){
       max_val = y_val;
     }
     if( gVerb || 1 ){
           printf("values : %f %f\n",x_val,y_val);
          }

     all++;
   }
   fclose(fcd);


   printf("max_val = %.4f\n",max_val);
   double prev=y_values[0];
   for(int i=0;i<all;i++){
      if( y_values[i]<=(max_val-3) && prev>=(max_val-3) ){ 
         if( gVerb>0 ){
            printf("%s : 3dB Point (%.4f - 3dB) is %.4f dB at %.4f [MHz]\n",fname,max_val,y_values[i],x_values[i]);
         }
      }
      prev = y_values[i];
   }

//   exit(0);   
   return all;
}  

void plot_power_law_loglog( const char* basename="sigmaG1_vs_lapSigmaG1_for_root", const char* modelfile=NULL, double unixtime=-1,
               int set_zero_freq_errors=1,
               const char* fit_func_name="powerlaw", double min_y=0.00, 
               double max_y=1.00, int bLog=0, const char* szDescX="Frequency [MHz]",
      const char* szDescY="Scattering Time #tau [ms]", const char* szTitle=NULL,
      double fit_min_x=2.278753601, double fit_max_x=2.380211242,
      int x_col=0, int y_col=2, const char* outpngfile=NULL )
{
   if( !szTitle){
      szTitle = basename;
   }
   gLog = bLog;
   gUNIXTIME = unixtime;
   gFreqErrorsZero = set_zero_freq_errors;
   
   // gROOT->Reset();
   // const char* basename = "s_vs_sigma_g_sqr";

   TCanvas* c1_log = new TCanvas("c1_log","c1_log",200,10,700,500);
   c1_log->SetGridx();
   c1_log->SetGridy();
   c1_log->SetFillColor(0);
   c1_log->SetFillStyle(0);

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



   lq1 = ReadResultsFile( basename, x_value1, y_value1, -1, -1, x_col, y_col ); 
   int lq1_err = ReadResultsFile( basename, x_value1_err, y_value1_err, -1, -1, x_col+1, y_col+1 );

   Double_t* x_value1_log = new Double_t[MAX_ROWS];
   Double_t* y_value1_log = new Double_t[MAX_ROWS];
   Double_t* y_value1_err_log = new Double_t[MAX_ROWS];

   for(int i=0;i<lq1;i++){
      x_value1_log[i] = TMath::Log10( x_value1[i] );
      y_value1_log[i] = TMath::Log10( y_value1[i] );
      y_value1_err_log[i] = fabs((1.00/TMath::Log(10.00))*(1.00/y_value1[i])*y_value1_err[i]);
   }

   c1_log->cd();
   // drawing background graphs here :
   TGraphErrors* pGraphLogLog = DrawGraph( x_value1_log, y_value1_log, lq1, 1, NULL, 
              "line", min_y, max_y, szTitle,
            basename, 0, szDescX, szDescY, fit_min_x, fit_max_x, NULL, y_value1_err_log );

  c1_log->Update();

/*   TString szEpsName1=basename;
   szEpsName1 += ".eps";
   c1->Print(szEpsName1.Data());*/

   TString szPngName1="images/";
   if( outpngfile ){
      szPngName1 += outpngfile;
   }else{
      szPngName1 += basename;
      szPngName1 += ".png";
   }
   c1_log->Print(szPngName1.Data());
}
