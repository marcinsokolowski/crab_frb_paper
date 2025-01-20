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

void rfifind( const char* fname, double rfi_threshold=10.00, double low=0, double up=1800, int bin_no=180,
              int column=0,
                int dofit=0,
                int bLog=0, const char* szTitleX="", const char* szTitleY="", 
                int DoBorder=1, const char* szTitle=NULL, const char* szOutFile=NULL,
                const char* szOutPostfix="_histo", int unix_time=0, const char* flag=NULL,
                int bNormalise=0, 
                                         int bPrintHeader=1,
                                         const char* szOutDir="images/" )
{
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
         if(buff[0]=='#')
            continue;      
         if(strstr(buff,"inf"))
            continue;


//         printf("%s\n",buff);

         double valx,valy;
         double tmp_val,tmp_val1,tmp_val2,tmp_val3;

/*         if( column <= 3 ){
            switch( column ){
               case 0 :
                  sscanf(buff,"%lf",&valx);
                  break;
               case 1 :
                  sscanf(buff,"%lf %lf",&tmp_val,&valx);
                  break;
               case 2 :
                  sscanf(buff,"%lf %lf %lf",&tmp_val1,&tmp_val2,&valx);
                  break;
               case 3 :
                  sscanf(buff,"%lf %lf %lf %lf",&tmp_val1,&tmp_val2,&tmp_val3,&valx);
                  break;
               default :
                  printf("ERROR : column number %d is too high number :( - script is oversimplistic (no time to fix it !)\n",column);
                  break;
            }
         }else{*/
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
                  ncols++;
               }
               col++;
            }
//         }

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
   fcd = fopen(fname,"r");      

   if (low<=-1000000 && up<=-1000000 ){
      low = min_val;
      up = max_val;

   }

   printf("low=%.2f up=%.2f bin=%d (min_val=%.2f, max_val=%.2f)\n",low,up,bin_no,min_val,max_val);


   char szHistoTitle[256];
   sprintf(szHistoTitle,"Histo_%s",fname);
   int rejected=0;

   double border=0.00;
   TH1F*  histo = new TH1F(szHistoTitle,szTitle,bin_no,low-border,up+border);
   TH1F* h_px = histo;
   while (1) {
      if(fgets(buff,lSize,fcd)==0)
         break;
      if(buff[0]=='#')
         continue;      
      if(strstr(buff,"inf"))
         continue;


      double valx,valy;
      double tmp_val,tmp_val1,tmp_val2,tmp_val3;
/*      switch( column ){
         case 0 :
            sscanf(buff,"%lf",&valx);
            break;
         case 1 :
            sscanf(buff,"%lf %lf",&tmp_val,&valx);
            break;
         case 2 :
            sscanf(buff,"%lf %lf %lf",&tmp_val1,&tmp_val2,&valx);
            break;
         case 3 :
            sscanf(buff,"%lf %lf %lf %lf",&tmp_val1,&tmp_val2,&tmp_val3,&valx);
            break;
         default :
            printf("ERROR : column number %d is too high number :( - script is oversimplistic (no time to fix it !)\n",column);
            break;
      }*/

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
                  ncols++;
               }
               col++;
            }

      sum += valx;
      sum2 += valx*valx;
      count++;

      histo->Fill( valx );


      if( valx > -35 ){
         rejected++;
      }
   }

   if( bNormalise > 0 ){
     Double_t integral = histo->Integral();
     printf("Integral = %.8f\n",integral);
     if (integral > 0){
        histo->Scale(1/integral);
     }
   }

   printf("Number of rejected (Power > -35 dBm) = %d out of %d = %.8f \%\n",rejected,cnt,((double)rejected)/((double)cnt));


   TCanvas* c1 = new TCanvas("c1","plot",200,10,1800,1200);
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
   gStyle->SetOptFit(111);


   gStyle->SetOptStat(1111111);
/*
   // Set stat options                 
   gStyle->SetStatY(0.9);               
   // Set y-position (fraction of pad size)
   gStyle->SetStatX(0.2);               
   // Set x-position (fraction of pad size)
   gStyle->SetStatW(0.4);             
   // Set width of stat-box (fraction of pad size)
   gStyle->SetStatH(0.2);
*/
 
   TPad* p1 = new TPad("p1","p1",0,0,1,1,-1);
   if(bLog)
      p1->SetLogy(1);
   p1->SetFillColor(0);
   p1->SetFillStyle(0);
   p1->Draw();
   p1->cd();

   histo->GetXaxis()->SetTitleOffset(1.00);
   histo->GetYaxis()->SetTitleOffset(1.00);
   histo->GetXaxis()->SetTitleSize(0.05);
   histo->GetXaxis()->SetLabelSize(0.05);
   histo->GetYaxis()->SetTitleSize(0.05);
   histo->GetYaxis()->SetLabelSize(0.05);

   histo->Draw();

   double histo_stddev = histo->GetStdDev();
   double histo_mean   = histo->GetMean();

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
//      lat.DrawLatex( low+(up-low)*0.2,max_value*0.95,szDesc);
      sprintf(szDesc,"FIT_sigma = %.6f",par[2]);
//      lat.DrawLatex( low+(up-low)*0.2,max_value*0.89,szDesc);


      double mean = sum/count;
      double mean2 = (sum2/count);
      double rms = sqrt( mean2 - mean*mean );

      FILE* out = fopen("sigma.txt","a+");
      // filename SIGMA MEAN NORMALIZATION AVG RMS
      if( bPrintHeader > 0 ){
         fprintf(out,"# FILE FIT_SIGMA FIT_MEAN FIT_NORM MEAN RMS\n");
      }
      fprintf(out,"%s %.8f %.8f %.8f %.8f %.8f %d\n",flag,par[2],par[1],par[0],mean,histo_stddev,unix_time); // was mean,rms
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
   }
   if( szTitleY && strlen(szTitleY) ){
      histo->SetYTitle( szTitleY );   
   }
   
   TLatex lat;
   lat.SetTextAlign(23);
   lat.SetTextSize(0.05);
   char szText[128];
   sprintf(szText,"Min Value = %.3f, Max Value = %.3f\n",min_val,max_val);
   lat.DrawLatex( min_val+(max_val-min_val)/3,  histo->GetBinContent(histo->GetMaximumBin()) * 0.9 , szText);
   printf("lat at (%.2f,%.2f) = %s\n",min_val+(max_val-min_val)/3,  histo->GetBinContent(histo->GetMaximumBin()) * 0.9,szText);

   if( unix_time > 0 ){
      FILE* of = fopen( "max_vs_time.txt", "a" );
      fprintf(of,"%d %.2f %.2f\n",unix_time,min_val,max_val);
      fclose(of);
   }


   c1->Update();
   // gSystem->Sleep(5000);

   printf("DEBUG : bin width = %.4f vs. %.4f\n",histo->GetBinWidth(0),histo->GetBinWidth(bin_no));


   FILE* outf = fopen("rfitimes.ranges","w");
   for(int ch=0;ch<histo->GetNbinsX();ch++){
      double val_histo = histo->GetBinContent(ch);
      printf("BIN(%d) = %.4f\n",ch,val_histo);

      if( val_histo > rfi_threshold ){
//         fprintf(outf,"%.8f %.8f\n",histo->GetBinCenter(ch),TMath::Log10(val_histo));
//         printf("DEBUG : %.8f %.8f\n",histo->GetBinCenter(ch),TMath::Log10(val_histo));

//         x_values[graph_count] = histo->GetBinCenter(ch);
//         y_values[graph_count] = val_histo; // dbm2mW(val_histo);
//         graph_count++;

           double bin_start = histo->GetBinCenter(ch) - histo->GetBinWidth(ch)/2;
           double bin_end   = histo->GetBinCenter(ch) + histo->GetBinWidth(ch)/2;
           printf("RFI : %.2f - %.2f\n",bin_start,bin_end);
           fprintf(outf,"%.4f %.4f\n",bin_start,bin_end);
      }
   }
   fclose(outf);


/*   TString szEpsName=fname;
   szEpsName += szOutPostfix;
   szEpsName += ".eps";
   c1->Print(szEpsName.Data());*/

//   TString szPngName=fname;
//   szPngName += szOutPostfix;
//   szPngName += ".png";
//   c1->Print(szPngName.Data());

   TString szPngName = szOutDir;
        gSystem->mkdir(szOutDir);

        char szColumn[64];
        

   szPngName += fname;
   szPngName += szOutPostfix;
        if( column > 0 ){
           sprintf(szColumn,"_col%04d",column);
           szPngName += szColumn;
        }
   szPngName += ".png";
   c1->Print(szPngName.Data());
}

