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
char gOutHistoFile[128];

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
 


double min_val = 1000000;
double max_val = -100000;



TH1F* histofile( const char* fname, int column=0,
                int dofit=1, double low=0, double up=100000, int bin_no=100,
                int bLog=0, const char* szTitleX="", const char* szTitleY="",
                int DoBorder=1, const char* szTitle=NULL, const char* szOutFile=NULL,
                const char* szOutPostfix="_histo", int unix_time=0, int bNormalise=1 )
{
   sprintf(gOutHistoFile,"%s_histo.txt",fname);

   FILE *fcd = fopen(fname,"r");
   char buff[2048];
   int lSize=2048;
   int val[4];
   double sum=0;
   double sum2=0;
   int count=0;


   printf("low=%.2f up=%.2f bin=%d (min_val=%.2f, max_val=%.2f)\n",low,up,bin_no,min_val,max_val);


   char szHistoTitle[256];
   sprintf(szHistoTitle,"Histo_%s",fname);

   int ncols=0;
//   double border=(up-low)/10.00;
//   if( DoBorder <= 0 )
   double border=0;
   TH1F*  histo = new TH1F(szHistoTitle,szTitle,bin_no,low-border,up+border);
   TH1F* h_px = histo;
   int rejected=0;
   while (1) {
      if(fgets(buff,lSize,fcd)==0)
         break;
      double valx,valy;

      char* ptr=NULL;
      char* search_ptr=buff;
      int col=0;
      while( (ptr = strtok(search_ptr," \t")) ){
         search_ptr = NULL;
         if( gVerb ){
              printf("ptr = %s\n",ptr);
         }

         if( col == column ){
            if( strstr(ptr,"nan") ){
               break;
            }
            valx = atof( ptr );
            ncols++;
         }
         col++;
      }

      sum += valx;
      sum2 += valx*valx;
      count++;

/*      if( valx > -35 ){
         rejected++;
      }*/

      printf("DEBUG : %.8f\n",valx);
      histo->Fill( valx );
   }

   Double_t integral = histo->Integral();
   printf("Integral = %.8f\n",integral);
   if( bNormalise > 0 ){
     if (integral > 0){
        histo->Scale(1/integral);
     }
     // check integral:
     double norm_integral = histo->Integral();
     printf("Integral after normalisation = %.8f\n",norm_integral);

   }
   printf("Number of rejected (Power > -35 dBm) = %d out of %.2f = %.8f \%\n",rejected,integral,100.00*((double)rejected)/((double)integral));

   if( dofit ){
      histo->Fit("gaus");

      double par[3];
      histo->GetFunction("gaus")->GetParameters(par);
   
      TLatex lat;
      lat.SetTextAlign(23);
      lat.SetTextSize(0.05);
      
      int max_bin = histo->GetMaximumBin();
      int max_value = histo->GetBinContent( max_bin );
      printf("max_value = %d\n",max_value);
      char szDesc[256];
      sprintf(szDesc,"FIT_mean  = %.6f",par[1]);
      lat.DrawLatex( low+(up-low)*0.2,max_value*0.95,szDesc);
      sprintf(szDesc,"FIT_sigma = %.6f",par[2]);
      lat.DrawLatex( low+(up-low)*0.2,max_value*0.89,szDesc);


      double mean = sum/count;
      double mean2 = (sum2/count);
      double rms = sqrt( mean2 - mean*mean );

      FILE* out = fopen("sigma.txt","a+");
      // filename SIGMA MEAN NORMALIZATION AVG RMS
      fprintf(out,"%s %.8f %.8f %.8f %.8f %.8f\n",fname,par[2],par[1],par[0],mean,rms);
      fclose(out);

      printf("GAUSS MEAN  = %.8f\n",par[1]);
      printf("GAUSS SIGMA = %.8f\n",par[2]);
   }

   FILE* outf = fopen(gOutHistoFile,"w");
   for(int ch=0;ch<histo->GetNbinsX();ch++){
      double val_histo = histo->GetBinContent(ch);

      if( val_histo > 0 ){
         fprintf(outf,"%.8f %.8f\n",histo->GetBinCenter(ch),val_histo);
         printf("DEBUG : %.8f %.8f\n",histo->GetBinCenter(ch),val_histo);
      }
   }
   fclose(outf);   
   printf("Histogram exported to file %s\n",gOutHistoFile);


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
   }
   if( szTitleY && strlen(szTitleY) ){
      histo->SetYTitle( szTitleY );   
   }
   

   if( unix_time > 0 ){
      FILE* of = fopen( "max_vs_time.txt", "a" );
      fprintf(of,"%d %.2f %.2f\n",unix_time,min_val,max_val);
      fclose(of);
   }



   return histo;
}


void histofiles_tau( const char* file_list, int column=0,
                int dofit=1, double low=0, double up=100000, int bin_no=100,
                int bLog=1, const char* szTitleX="Scattering time at 610 MHz [ms]", const char* szTitleY="Number of entries", 
                int DoBorder=1, const char* szTitle=NULL, const char* szOutFile=NULL,
                const char* szOutPostfix="_histo", int unix_time=0, int bNormalise=1 )
{
//   if( !szTitle ){
//      szTitle=file_list;
//   }

   TFile*  f = new TFile("histofile.root","RECREATE");
   

   TCanvas* c1 = new TCanvas("c1","plot",10,10,1800,850);
   c1->SetFillColor(0);
   c1->SetFillStyle(0);
   gStyle->SetPadTopMargin(0.03);
   gStyle->SetPadBottomMargin(1.2);  
   gStyle->SetPadLeftMargin(0.15);
   gStyle->SetPadRightMargin(0.04);
   gStyle->SetTitleXOffset(1.12);
   gStyle->SetTitleYOffset(1.2);
   gStyle->SetFillColor(0);
   gStyle->SetFillStyle(0);
 
   TPad* p1 = new TPad("p1","p1",0,0,1,1,-1);
   if(bLog)
      p1->SetLogy(1);
   p1->SetFillColor(0);
   p1->SetFillStyle(0);
   p1->Draw();
   p1->cd();

   cFileDesc2 list[1000];
   int file_count = ReadListFile( file_list, list );

   TLegend *legend = new TLegend(.2,0.8,0.45,0.95);
   legend->SetTextFont(72);
   legend->SetTextSize(0.04);
   TH1F* histo_tab[20];

   const char* fname = list[0].fname;
   FILE *fcd = fopen(fname,"r");
   if(!fcd){
       printf("Could not open file : %s\n",fname);
       return;
   }
   char buff[2048];
   int lSize=2048;
   int val[4];

   int ncols=0;
   double valx;
   double sum=0;
   double sum2=0;
   int count=0;   
   while (1) {
         if(fgets(buff,lSize,fcd)==0)
            break;

//         printf("%s\n",buff);

           char* ptr=NULL;
           char* search_ptr=buff;
           int col=0;
           while( (ptr = strtok(search_ptr," \t")) ){
              search_ptr = NULL;
              if( gVerb ){
                   printf("ptr = %s\n",ptr);
              }

              if( col == column ){
                 if( strstr(ptr,"nan") ){
                    break;
                 }
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
   }
   fclose( fcd );

   if (low==0 && up==100000 ){
      low = min_val;
      up = max_val;

   }


   for(int i=0;i<file_count;i++){
      histo_tab[i] = histofile( list[i].fname , column, dofit, low, up, bin_no, bLog, szTitleX, szTitleY, DoBorder, szTitle, szOutFile, szOutPostfix, unix_time, bNormalise );
      TH1F* histo = histo_tab[i];
      histo->SetLineColor( basic_kolors[i] );
      histo->SetLineWidth(2);
      
      if( i == 0 ){
         histo->Draw();
      }else{
         histo->Draw("same");
      }

/*      TLatex*  lat = new TLatex();
      lat->SetTextAlign(23);
      lat->SetTextSize(0.05);
      char szText[128];
      sprintf(szText,"Min Value = %d, Max Value = %d\n",(int)min_val,(int)max_val);
      lat->DrawLatex( min_val+(max_val-min_val)/3,  histo->GetBinContent(histo->GetMaximumBin()) * 0.9 , szText);
      printf("lat at (%.2f,%.2f) = %s\n",min_val+(max_val-min_val)/3,  histo->GetBinContent(histo->GetMaximumBin()) * 0.9,szText);*/


      char szLegend[64];
      strcpy(szLegend,"Unknown data");
      if( strstr( list[i].fname , "tau610MHz.txt" ) ){
         strcpy(szLegend,"EDA2 #tau_{215MHz} scaled to 610 MHz");
      }
      legend->AddEntry(histo,szLegend,"lp");

      c1->Update();
      // gSystem->Sleep(5000);

   }
   legend->Draw();

   TLatex*  lat = new TLatex();
   lat->SetTextAlign(23);
   lat->SetTextSize(0.05);
   char szText[128];
   sprintf(szText,"Min Value = %d, Max Value = %d\n",(int)min_val,(int)max_val);
//   lat->DrawLatex( min_val+(max_val-min_val)/3,  histo->GetBinContent(histo->GetMaximumBin()) * 0.9 , szText);
//   printf("lat at (%.2f,%.2f) = %s\n",min_val+(max_val-min_val)/3,  histo->GetBinContent(histo->GetMaximumBin()) * 0.9,szText);

/*   TString szEpsName=file_list;
   szEpsName += szOutPostfix;
   szEpsName += ".eps";
   c1->Print(szEpsName.Data());*/

   TString szPngName=file_list;
   szPngName += szOutPostfix;
   szPngName += ".png";
   c1->Print(szPngName.Data());
}

