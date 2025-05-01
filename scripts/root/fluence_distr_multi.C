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


int gVerb=0;

double gPulsarPeriods=1.00;
double gPulsarPeriodInSec=0.0333924123;
double gUnixTime=-1;
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

double gFlux0 = 10.00;

Double_t power_law_distrib( Double_t* x, Double_t* y )
{
    Double_t power_dbm=x[0];
    Double_t power_mW = x[0];
    Double_t power_mW0 = gFlux0;

    return y[0]*TMath::Power( (power_mW/power_mW0) , y[1] );
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

struct cFileDesc2
{
   char fname[128];
   char comment[512];
};


int basic_kolors[] = {kBlue,kBlack,4,5,6,7,8,9,11,12,20,28,29,30,36,38,39,40,41,42,43,44,45,46,47,48,49,10,13,14,15,16,17,18,19,1,2,3,4,5,6,7,8,9};

int ReadListFile( const char* fname , cFileDesc2* file_list )
{
   const int lSize=1000;
   char buff[1000];
   
   FILE *fcd = fopen(fname,"r");
   if(!fcd){
       printf("Could not open file : %s\n",fname);
       return -1;
   }

   Int_t all = 0;
   Double_t fval1,fval2,fval3,fval4,fval5,fval6,mag,x,y;
   long lval1,lval2,lval3,lval4;

   double sum20mhz=0.00;
   double total_sum=0.00;

   all=0;
   int ncols=0;
   while (1) {
      if(fgets(buff,lSize,fcd)==0)
         break;
      if(buff[0]=='#')
         continue;
      if(strstr(buff,"nan"))
         continue;

      if( buff[strlen(buff)-1] == '\n' ){
           buff[strlen(buff)-1] = '\0';  
      }

      cFileDesc2 file_desc;
      strcpy(file_desc.comment,buff);
      char* search_ptr = buff;
      const char* ptr = strtok(search_ptr," \t");
      strcpy( file_desc.fname, ptr );
//      ptr = strtok(search_ptr," \t");
//      if( ptr ){
//      if( strlen(buff+strlen(ptr)) > 0 ){
//         strcpy(file_desc.comment,buff); 
//      }

      file_list[all] = file_desc;
      all++;
   }
   fclose(fcd);

   return all;
}



TH1F* histogram( const char* fname, int column, int low, int up, int bin_no, const char* szTitle, double& min_val, double& max_val, int color )
{
   FILE *fcd = fopen(fname,"r");
   if(!fcd){
       printf("Could not open file : %s\n",fname);
       return NULL;
   }

   char buff[2048];
   int lSize=2048;
   int val[4];

   double sum=0;
   double sum2=0;
   int count=0,cnt=0;
   min_val = 1000000;
   max_val = -100000;

   
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
         while( (ptr = strtok(search_ptr," \t")) ){
            search_ptr = NULL;
            if( gVerb ){
               printf("ptr = %s\n",ptr);
            }

            if( col == column ){
               valx = atof( ptr );
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
   sprintf(szHistoTitle,"Histo_%s",fname);
   int rejected=0;

//   double border=(up-low)/10.00;
//   if( DoBorder <= 0 )
   double start_x = 1;
   double   border=0;
   printf("DEBUG : histo bin_no = %d, range = %.6f - %.6f",bin_no,low-border,up+border);
   TH1F*  histo = new TH1F(szHistoTitle,szTitle,bin_no,low-border,up+border);        
//   TH1F*  histo = new TH1F(szHistoTitle,szTitle,bin_no,5,110);
   histo->Sumw2(); //
   histo->SetLineColor(color);

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
            while( (ptr = strtok(search_ptr," \t")) ){
               search_ptr = NULL;
               if( gVerb ){
                  printf("ptr = %s\n",ptr);
               }

               if( col == column ){
                  valx = atof( ptr );
                  ncols++;
               }
               col++;
            }
 
            if( valx < 0.500 ){
               continue;
            }

            sum += valx;
            sum2 += valx*valx;
            count++;

            histo->Fill( valx );
            sum_test += 1.00;
   }

   printf("DEBUG : sum_test = %.8f\n",sum_test);

   fclose(fcd);

   return histo;
}

double fit_histogram( TH1F* histo, const char* fname, const char* gOutHistoFile, double fit_min_x, double fit_max_x, int dofit=1, int unix_time=0 )
{
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

   double par[3];
   Double_t parerrors=0.00;
   Double_t fit_norm=0.00,fit_exp=0.00;
   Double_t fit_norm_err=0.00,fit_exp_err=0.00;


   int max_bin = 0;
   double max_value = 0.00;
   if( dofit ){
      TF1* pFitFunc = new TF1("power_law_distrib",power_law_distrib,fit_min_x,fit_max_x,2);
      par[0] = 1e-6;
      par[1] = -3.0;
 
      pFitFunc->SetParameters(par);
      histo->Fit("power_law_distrib","E,V","",fit_min_x,fit_max_x);
//      histo->GetFunction("power_law_distrib")->SetParameters(par);
//      histo->Fit("power_law_distrib","E,V","",fit_min_x,fit_max_x);
      histo->GetFunction("power_law_distrib")->GetParameters(par);
//      parerrors=histo->GetFunction("power_law_distrib")->GetParErrors();
      fit_norm_err = histo->GetFunction("power_law_distrib")->GetParError(0);
      fit_exp_err  = histo->GetFunction("power_law_distrib")->GetParError(1);            

      histo->GetFunction("power_law_distrib")->SetParName(0,"N_{ref}");
      histo->GetFunction("power_law_distrib")->SetParName(1,"#alpha");
 
      printf("FITTED POWER LAW = %.8f * (f/%.8f)^(%.8f)\n",par[0],gFlux0,par[1]);
   
      TLatex lat;
      lat.SetTextAlign(23);
      lat.SetTextSize(0.03);
      
      max_bin = histo->GetMaximumBin();
      max_value = histo->GetBinContent( max_bin );
      printf("max_value = %.8f\n",max_value);
      char szDesc[256];
      fit_norm = par[0];
      fit_exp  = par[1];
      int base = gFixedRefPower/10;
      double text_x = 200;
      double text_y = 400;


//      double mean = sum/count;
//      double mean2 = (sum2/count);
//      double rms = sqrt( mean2 - mean*mean );

      char szFitFile[128];
      sprintf(szFitFile,"%s.fit_results_fit_range_%.2f-%.2f",fname,fit_min_x,fit_max_x);
      FILE* out = fopen(szFitFile,"a+");
      // filename SIGMA MEAN NORMALIZATION AVG RMS
//    if( bPrintHeader > 0 ){
//       fprintf(out,"# FILE FIT_SIGMA FIT_MEAN FIT_NORM MEAN RMS\n");
//    }
      fprintf(out,"%.20f %.20f %.20f %.20f %.4f %d %.8f\n",fit_norm,fit_exp,fit_norm_err,fit_exp_err,((double)gChannel)*(480.00/4096.00),gChannel,gUnixTime);
      fclose(out);      


      printf("GAUSS MEAN  = %.8f\n",par[1]);
      printf("GAUSS SIGMA = %.8f\n",par[2]);
      printf("Fit results written to file sigma.txt\n");
   }else{
      FILE* out = fopen("sigma.txt","a+");
      // filename SIGMA MEAN NORMALIZATION AVG RMS
      fprintf(out,"%s %.8f %.8f %.8f %.8f %.8f %d\n",fname,0.00,histo->GetMean(),histo->GetRMS(),histo->GetMean(),histo->GetRMS(),unix_time);
      fclose(out);
   }

   delete [] x_values;
   delete [] y_values;

   double fluence_of_oneperhour_pulse = gFlux0 / TMath::Power( par[0] , (1.00/par[1]) );

   return fluence_of_oneperhour_pulse;
}

void fluence_distr_multi( const char* file_list, double TotalTimeInHours=0.998768602,
                int column=0,
                double fit_min_x=1400, double fit_max_x=100000.00, 
                double unixtime=-1,
                int dofit=1, int dDbm2DbmPerHz=1,
                double low=90, double up=50000.000, int bin_no=100,
                const char* szExtDesc=NULL, int shift_text=0, int channel=0,
                int bLog=1, const char* szTitleX="Fluence [Jy s]", const char* szTitleY="Rate of occurance (per rotation)", 
                int DoBorder=1, const char* szTitle=NULL, const char* szOutFile=NULL,
                const char* szOutPostfix="_histo", int unix_time=0, const char* flag=NULL,
                int bNormalise=0, int bPrintHeader=1, int bPlotExtrapolation=0 )
{
    gPulsarPeriods = (TotalTimeInHours*3600.00)/gPulsarPeriodInSec;
    printf("Number of pulsar rotations = %.4f\n",gPulsarPeriods);

    gUnixTime = unixtime;
    gChannel = channel;
    dDbm2DbmPerHz = 0;
    char gOutHistoFile[1024];
    change_ext(file_list,"histo",gOutHistoFile);

    if( !flag ){
       flag = file_list;
     }

     if( !szTitle ){
       szTitle=file_list;
     }

   cFileDesc2 list[1000];
   int file_count = ReadListFile( file_list, list );

   TFile* f = new TFile("histofile.root","RECREATE");

   TCanvas* c1 = new TCanvas("c1","plot",10,10,3500,1200);
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
   c1->SetLogx(1);
   c1->SetLogy(1);

   TLegend *legend = new TLegend(.05,0.8,0.55,0.95);
   legend->SetTextFont(72);
   legend->SetTextSize(0.02); // 0.05 - large for eps-es to papers

   for(int i=0;i<file_count;i++){        
      double min_val = 1000000;
      double max_val = -100000;
      int kolor = basic_kolors[i];

      TH1F* histo = histogram( list[i].fname, column, low, up, bin_no, szTitle, min_val, max_val, kolor );
//      histo->Scale(1/gPulsarPeriods);



      // X axis 
      histo->GetXaxis()->SetTitleOffset(0.60);
      histo->GetXaxis()->SetTitleSize(0.048);
      histo->GetXaxis()->SetLabelSize(0.05);

      // Y axis 
      histo->GetYaxis()->SetTitleOffset(0.60);
      histo->GetYaxis()->SetTitleSize(0.05);
      histo->GetYaxis()->SetLabelSize(0.05);
      histo->SetTitle("");
//   histo_mp->SetMaximum(0.3);
//   histo_mp->

// TEST:
//   histo_mp->Sumw2();

     if( i > 0 ){
        histo->Draw("L,same");
     }else{
        histo->Draw("AL");
     }
     char szLegend[64];
     strcpy(szLegend,list[i].fname);
     legend->AddEntry(histo,szLegend,"lp");// basename




/*     double fluence_of_oneperhour_pulse_mp = fit_histogram( histo, list[i].fname, gOutHistoFile, fit_min_x, fit_max_x, dofit, unix_time );

     if( szOutFile && NULL ){   
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
     }*/

   }
   legend->Draw();
   c1->Update();
//   gSystem->Sleep(1);


   TString szPngName;
   szPngName="images/";
   szPngName += file_list;
   szPngName += ".png";
   c1->Print(szPngName.Data());

   printf("PWD : %s\n",gSystem->pwd());   
}

