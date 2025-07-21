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
#include <TFile.h>
#include <TMinuit.h>

int gVerb=0;

int g_dDbm2DbmPerHz=1;
int gChannel=0;
double gFreqBinHz=(480.00/4096.00)*1e6;
double gFixedRefPower=-160;

double dbm2mW( double in_dbm )
{
   double l = in_dbm/10.00;
   double mW = TMath::Power(10,l);

   return mW;
}

double gFlux0 = 10.00; // better to have normalisation at SNR=10 than 100000.00 ...

Double_t power_law_distrib( Double_t* x, Double_t* y )
{
    Double_t power_dbm=x[0];
    Double_t power_mW = x[0];
    Double_t power_mW0 = gFlux0;

    return y[0]*TMath::Power( (power_mW/power_mW0) , y[1] );
}

Double_t exponential_distrib( Double_t* x, Double_t* y )
{
    Double_t time_diff=x[0];
    Double_t lambda = y[0];   

    return lambda*exp(-lambda*time_diff);
}

// https://en.wikipedia.org/wiki/Weibull_distribution
Double_t weibull_distrib( Double_t* x, Double_t* y )
{
    Double_t time_diff=x[0];
    Double_t lambda = y[0];   
    Double_t k = y[1];

//    return lambda*exp(-lambda*time_diff);
    Double_t ret = (k/lambda)*TMath::Power( (time_diff/lambda) , (k-1) )*exp(- TMath::Power( (time_diff/lambda), k ) );

    return ret;
}


// LOG-NORMAL : 
Double_t Gaussian( Double_t* x, Double_t* y )
{
   Double_t t = x[0];

   Double_t t_peak = y[0];
   Double_t n0 = y[1];
   Double_t sigma = y[2];

   double tt = TMath::Power(10,t);
   double norm = n0*(1.00/(tt*sigma*sqrt(2*TMath::Pi())));
   double gaussian = norm*exp(-0.5*(t - t_peak)*(t - t_peak)/(2.00*sigma*sigma) );

   return gaussian;
}




int is_digit( char znak )
{
   if( znak=='0' || znak=='1' || znak=='2' || znak=='3' || znak=='4' ||
       znak=='5' || znak=='6' || znak=='7' || znak=='8' || znak=='9' ){
      return 1;
   }
   return 0; 
}



double round_to_n_digits(double x, int& n, int is_err=0)
{ 
    char buff[32];

    if( is_err > 0 ){
       n=1;

       sprintf(buff,"%.20f",x);
       char first_digit = -1;  
       int after_dot=-1;
       for(int i=0;i<strlen(buff);i++){
         if( is_digit( buff[i] ) > 0 ){
            if( after_dot >= 0 ){
               after_dot++;
            }  
            if( first_digit < 0 && buff[i]!='0' ){
               first_digit = buff[i];
               break;
            }
         }else{
            if( buff[i] == '.' ){
               after_dot = 0;
            }
         }   
       }     

       double rounded = x;
//     after_dot++;
// COMMENT IFS for a)
       if( after_dot < 0 ){
          after_dot = 0;   
       }
       if( first_digit != '1' ){
          rounded = ceil(x*TMath::Power(10,after_dot))/TMath::Power(10,after_dot);
       }
       printf("First digit in %s is %c, after dot = %d -> rounded = %.20f\n",buff,first_digit,after_dot,rounded);
       if( first_digit == '1' ){
          n = 2;
       }

//     double scale = TMath::Power(10.0, ceil(log10(fabs(x))) + n);
//     return round(x * scale) / scale;
       if( after_dot > 0 ){
          sprintf(buff, "%.*g", n, rounded);
       }else{
          sprintf(buff, "%.0f",rounded);
       }
       printf("Buff = %s\n",buff);
    }else{
       sprintf(buff, "%.*g", n, x);
    }

    return atof(buff);
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


void histo_time_between_pulses_log_normal_nooffset( const char* fname, 
                int column=0,
                double fit_min_x=-10, double fit_max_x=10, 
                int dofit=1, int dDbm2DbmPerHz=1,
                double low=-5, double up=5, int bin_no=1000, double unixtime=-1,
                const char* szExtDesc=NULL, int shift_text=0, int channel=0,
                int bLog=0, const char* szTitleX="Wait Times [Log_{10}(seconds)]", const char* szTitleY="Counts", 
                int DoBorder=1, const char* szTitle=NULL, const char* szOutFile=NULL,
                const char* szOutPostfix="_histo", int unix_time=0, const char* flag=NULL,
                int bNormalise=1, int bPrintHeader=1 )
{
    gChannel = channel;
    dDbm2DbmPerHz = 0;
    char gOutHistoFile[1024];
    change_ext(fname,"histo",gOutHistoFile);

    if( !flag ){
       flag = fname;
     }

     if( !szTitle ){
       szTitle=fname;
     }

   TFile* f = new TFile("histofile.root","RECREATE");
   
   FILE *fcd = fopen(fname,"r");
   if(!fcd){
       printf("Could not open file : %s\n",fname);
       return;
   }

   char buff[2048];
   int lSize=2048;
   int val[4];

   double sum=0;
   double sum2=0;
   int count=0,cnt=0;
   double min_val = 1000000;
   double max_val = -100000;

   
   while (1) {
         if(fgets(buff,lSize,fcd)==0)
            break;
         if(buff[0]=='#' || strstr(buff,"inf"))
            continue;      


//         printf("%s\n",buff);

         double valx,valy;
         double tmp_val,tmp_val1,tmp_val2,tmp_val3;

         int ncols=0;
         char* ptr=NULL;
         char* search_ptr=buff;
         int col=0;
         while( ptr = strtok(search_ptr," \t") ){
            search_ptr = NULL;
            if( gVerb ){
               printf("ptr = %s\n",ptr);
            }

            if( col == column ){
               valx = atof( ptr );
               valx = TMath::Log10(valx); // logarithm
               ncols++;
            }
            col++;
         }

//         printf("%.2f\n",valx);

         if( valx < min_val ){
            min_val = valx;
         }
         if( valx > max_val ){
            max_val = valx;
         }
         cnt++;
   }
   fclose( fcd );

   printf("DEBUG : ok 1 ???\n");
   fcd = fopen(fname,"r");      

   if (low<=-1000000 && up<=-1000000 ){
      low = min_val;
      up = max_val;

   }

   printf("low=%.2f up=%.2f bin=%d (min_val=%.2f, max_val=%.2f)\n",low,up,bin_no,min_val,max_val);

  
   char szHistoTitle[256];
//   sprintf(szHistoTitle,"Histo_%s",fname);
   sprintf(szHistoTitle,"Log_{10}( WaitTimes )");
   int rejected=0;

//   double border=(up-low)/10.00;
//   if( DoBorder <= 0 )
   double start_x = 1000;
   double   border=0;
   printf("DEBUG : histo bin_no = %d, range = %.6f - %.6f",bin_no,low-border,up+border);
   TH1F*  histo = new TH1F(szHistoTitle,szTitle,bin_no,low,up);
//   TH1F*  histo = new TH1F(szHistoTitle,szTitle,bin_no,start_x-border,up+border);        
//   TH1F*  histo = new TH1F(szHistoTitle,szTitle,bin_no,5,110);
   histo->Sumw2(); //

   TH1F* h_px = histo;
   double sum_test=0.00;
   while (1) {
      if(fgets(buff,lSize,fcd)==0)
         break;
      if(buff[0]=='#')
         continue;      

      double valx,valy;
      double tmp_val,tmp_val1,tmp_val2,tmp_val3;

            int ncols=0;
            char* ptr=NULL;
            char* search_ptr=buff;
            int col=0;
            while( ptr = strtok(search_ptr," \t") ){
               search_ptr = NULL;
               if( gVerb ){
                  printf("ptr = %s\n",ptr);
               }

               if( col == column ){
                  valx = atof( ptr );
                  valx = TMath::Log10(valx); // logarithm
                  ncols++;
               }
               col++;
            }

            sum += valx;
            sum2 += valx*valx;
            count++;

            histo->Fill( valx );
            sum_test += 1.00;
   }

   printf("DEBUG : sum_test = %.8f\n",sum_test);

   // for correct errors after normalisation:   
//   histo->Sumw2(); // https://root.cern.ch/root/roottalk/roottalk99/2843.html

   if( bNormalise > 0 ){
     Double_t integral = histo->Integral();
     printf("Integral = %.8f\n",integral);
     if (integral > 0){
              histo->Scale(1/integral);
          }
   }

// normalise to have number of pulses per hour
//   histo->Scale(1/TotalTimeInHours);


   TCanvas* c1 = new TCanvas("c1","plot",10,10,1200,1200);
   c1->SetFillColor(0);
   c1->SetFillStyle(0);
   gStyle->SetPadTopMargin(0.03);
   gStyle->SetPadBottomMargin(1.2);  
   gStyle->SetPadLeftMargin(0.15);
   gStyle->SetPadRightMargin(0.04);
   gStyle->SetTitleXOffset(1.12);
   gStyle->SetTitleYOffset(1.5);
   gStyle->SetFillColor(0);
   gStyle->SetFillStyle(0);
   gStyle->SetOptFit(111);
   gStyle->SetOptStat("ne");
//   c1->SetLogx(1);
//   c1->SetLogy(1);

 

   // X axis 
   histo->GetXaxis()->SetTitleOffset(1.00);
   histo->GetXaxis()->SetTitleSize(0.048);
   histo->GetXaxis()->SetLabelSize(0.05);

   // Y axis 
   histo->GetYaxis()->SetTitleOffset(0.60);
   histo->GetYaxis()->SetTitleSize(0.05);
   histo->GetYaxis()->SetLabelSize(0.05);
   histo->SetTitle("");
//   histo->SetMaximum(0.3);
//   histo->

// TEST:
//   histo->Sumw2();

   histo->Draw();
// return; 
   

   // export data:
   Double_t* x_values = new Double_t[histo->GetNbinsX()];
   Double_t* y_values = new Double_t[histo->GetNbinsX()];
   int graph_count=0;

   FILE* outf = fopen(gOutHistoFile,"w");
   for(int ch=0;ch<histo->GetNbinsX();ch++){
      double val_histo = histo->GetBinContent(ch);

      if( val_histo > 0 ){
         fprintf(outf,"%.8f %.8f\n",histo->GetBinCenter(ch),TMath::Log10(val_histo));
         printf("DEBUG : %.8f %.8f\n",histo->GetBinCenter(ch),TMath::Log10(val_histo));

         x_values[graph_count] = histo->GetBinCenter(ch);
         y_values[graph_count] = val_histo; // dbm2mW(val_histo);
         graph_count++;
      }
   }
   fclose(outf);   
   printf("Histogram exported to file %s\n",gOutHistoFile);

   double par[5];
   Double_t parerrors=NULL;
   Double_t fit_lambda=0.00,fit_exp=0.00;
   Double_t fit_lambda_err=0.00,fit_exp_err=0.00;
   if( dofit ){
      int n_par = 3;
      TF1* pFitFunc = new TF1("Gaussian",Gaussian,fit_min_x,fit_max_x,n_par);

      par[0] = 1.00;
      par[1] = 0.008;
      par[2] = 0.5;
 
      pFitFunc->SetParameters(par);
      pFitFunc->SetParName(0,"<log_{10}(T_{0})>");
      pFitFunc->SetParName(1,"Norm");
      pFitFunc->SetParName(2,"#sigma");
//      pFitFunc->FixParameter(1,1.00);

      TFitResultPtr pFitRes = histo->Fit("Gaussian","E,V,R,S","",fit_min_x,fit_max_x);

//      TFitResultPtr pFitRes = histo->Fit(pFitFunc,"E,V,R,S","",fit_min_x,fit_max_x);
      TMatrixDSym cov = pFitRes->GetCovarianceMatrix(); 
      cov.Print();
      TMatrixD matrix0(n_par,n_par);
      gMinuit->mnemat(matrix0.GetMatrixArray(),n_par);
      matrix0.Print();

// printf("HERE ???\n"); 

//      histo->GetFunction("power_law_distrib")->SetParameters(par);
//      histo->Fit("power_law_distrib","E,V","",fit_min_x,fit_max_x);
      histo->GetFunction("Gaussian")->GetParameters(par);
//      parerrors=histo->GetFunction("power_law_distrib")->GetParErrors();
//      fit_lambda_err = histo->GetFunction("weibull_distrib")->GetParError(0);

// printf("HERE ???\n"); 


      Double_t x_test[1],y_test[1];
      x_test[0] = 10.00;
      y_test[0] = 0.01;
      printf("TEST : f(10) = %e vs. %e\n",pFitFunc->Eval(10.00),weibull_distrib(x_test,y_test));
   
      TLatex lat;
      lat.SetTextAlign(23);
      lat.SetTextSize(0.07);
      
      char szDesc[256];
//      sprintf(szDesc,"Fitted formula: N #times #left(#frac{Power[mW]}{10^{%d}mW}#right)^{#alpha}",base);
//      lat.DrawLatex( -149.8+shift_text,0.05,szDesc);
//      lat.DrawLatex( text_x, text_y,"Fit results:");

//      int n_digits=2;
//      sprintf(szDesc,"P(%.0f) = %.5f #pm %.5f",gFlux0,fit_norm,round_to_n_digits(fit_norm_err,n_digits,1));
//      lat.DrawLatex( text_x, text_y/2,szDesc);      
//      sprintf(szDesc,"#alpha = %.2f #pm 0.02",fit_exp,round_to_n_digits(fit_exp_err,n_digits,1));
//      lat.DrawLatex( text_x, text_y/4,szDesc);



      double mean = sum/count;
      double mean2 = (sum2/count);
      double rms = sqrt( mean2 - mean*mean );

      FILE* out = fopen("time_between_pulses_fit_results_EXP.txt","w");
      // filename SIGMA MEAN NORMALIZATION AVG RMS
//    if( bPrintHeader > 0 ){
//       fprintf(out,"# FILE FIT_SIGMA FIT_MEAN FIT_NORM MEAN RMS\n");
//    }
      fit_lambda = par[0];      
      fprintf(out,"%.8f %.20f %.20f\n",unixtime,fit_lambda,fit_lambda_err);
      fclose(out);      


      printf("GAUSS MEAN  = %.8f\n",par[1]);
      printf("GAUSS SIGMA = %.8f\n",par[2]);
      printf("Fit results written to file sigma.txt\n");
   }else{
      FILE* out = fopen("sigma.txt","a+");
      // filename SIGMA MEAN NORMALIZATION AVG RMS
      fprintf(out,"%s %.8f %.8f %.8f %.8f %.8f %d\n",flag,0,histo->GetMean(),histo->GetRMS(),histo->GetMean(),histo->GetRMS(),unix_time);
      fclose(out);
   }



   if( szOutFile ){   
      FILE* out_file=NULL;
      out_file = fopen( szOutFile, "w" );

      for(int i=0;i<bin_no;i++){
         int value = histo->GetBinContent( i );
         printf("BIN(%d) = %d\n",i,value);

         if( value > 0 ){
            fprintf(out_file,"%d\n",value);         
         }
      }

      fclose( out_file );
   }

   if( szTitleX && strlen(szTitleX) ){
      histo->SetXTitle( szTitleX );
      histo->GetXaxis()->SetTitleSize(0.05);
      histo->GetXaxis()->SetTitleOffset(1.0);
      histo->GetXaxis()->SetLabelSize(0.05);
   }
   if( szTitleY && strlen(szTitleY) ){
      histo->SetYTitle( szTitleY );   
      histo->GetYaxis()->SetTitleSize(0.05);
      histo->GetYaxis()->SetTitleOffset(1.5);
      histo->GetYaxis()->SetLabelSize(0.05);
   }
   
   TLatex lat;
   lat.SetTextAlign(23);
   lat.SetTextSize(0.07);
   char szText[128];
   sprintf(szText,"Min Value = %d, Max Value = %d\n",(int)min_val,(int)max_val);
//   lat.DrawLatex( min_val+(max_val-min_val)/3,  histo->GetBinContent(histo->GetMaximumBin()) * 0.9 , szText);
//   printf("lat at (%.2f,%.2f) = %s\n",min_val+(max_val-min_val)/3,  histo->GetBinContent(histo->GetMaximumBin()) * 0.9,szText);

   if( unix_time > 0 ){
      FILE* of = fopen( "max_vs_time.txt", "a" );
      fprintf(of,"%d %.2f %.2f\n",unix_time,min_val,max_val);
      fclose(of);
   }
   
//   TLatex lat;
   lat.SetTextAlign(23);
   lat.SetTextSize(0.068);
   if( szExtDesc ){
//      lat.DrawLatex( -160 , 0.2 , szExtDesc);
      lat.DrawLatex( (low+up)/2.00 , 0.25 , szExtDesc); 
   }


   c1->Update();
   // gSystem->Sleep(5000);


   

//   TString szPngName=fname;
//   szPngName += szOutPostfix;
//   szPngName += ".png";
//   c1->Print(szPngName.Data());

   TString szPngName;
   szPngName="images/";
   szPngName += fname;
   szPngName += "_time_between_pulses";
   szPngName += ".png";
   c1->Print(szPngName.Data());
}

