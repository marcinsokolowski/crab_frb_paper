#include <time.h>
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
// #include <Math/Polynomial.h>
// #include <Math/Interpolator.h>

// always verify line :
// gROOT->Reset();

int gLog=0;
int gVerb=0;
int gPercent=0;
struct tm* gmtime_tm=NULL;
struct tm* localtime_tm=NULL;
Double_t gStartTime=0.00;

/*int gMinLocalHour=180000;
int gMaxLocalHour=70000;*/
int gMinLocalHour=-1e6;
int gMaxLocalHour=+1e6;

double gMinUxtime = -10000;
double gMaxUxtime = -10000;

#define MAX_ROWS 30000000

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

TGraphErrors* DrawGraph( Double_t* x_values, Double_t* y_values, Double_t* x_values_err, Double_t* y_values_err,
                   int numVal, 
		   long q, TPad* pPad, const char* fit_func_name=NULL, 
			double min_y=-10000, double max_y=-10000,
			const char* szStarName="", const char* fname="default",
			int bLog=0, const char* szDescX=NULL, const char* szDescY=NULL,
			double fit_min_x=-100000, double fit_max_x=-100000 )
{
    int MarkerType = 20;
    int ColorNum = kBlack;

    Double_t z,sigma_z,const_part;
    TF1 *line = NULL;
	 TF1 *line_draw = NULL;
	TF1 *part1 = NULL;
	TF1 *part2 = NULL;
    Double_t maxX=-100000,maxY=-100000;
    Double_t minX=100000,minY=100000;

    for(int i=0;i<numVal;i++){
        if(x_values[i]>maxX)
            maxX = x_values[i];
        if(y_values[i]>maxY)
            maxY = y_values[i];
		
        if(x_values[i]<minX)
            minX = x_values[i];
        if(y_values[i]<minY)
            minY = y_values[i];
    }


    TGraphErrors* pGraph = new TGraphErrors(q);
    for(int i=0;i<numVal;i++){
	if( gPercent ){
		y_values[i] = ( y_values[i] / maxY );
	}

	if( gVerb || 1 ){
           printf("q=%d %f %f +/- %f\n",q, x_values[i], y_values[i], y_values_err[i] );
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

//        pGraph->SetPointError( i, (0.5/24.00) , sqrt(y_values[i]) ); // 1/2 hour observation error.
        pGraph->SetPointError( i, 0.00 , y_values_err[i] );
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
    pGraph->SetMarkerSize(2);
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
//	 pGraph->SetMinimum( -90 );
//	 pGraph->SetMaximum( 90 );
	 pGraph->SetTitle( szStarName );
    pGraph->Draw("AP");

//    pGraph->GetXaxis()->SetTimeDisplay(1);
//    pGraph->GetXaxis()->SetTimeFormat("%d\/%m\/%y %H:%M:%S");

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
//	           par[0]=pol1->GetParameter(0);
//		   par[1]=pol1->GetParameter(1);	
                }

              if( strcmp(fit_func_name,"pol2")==0  ){
                   pGraph->Fit("pol2");
//                   par[0]=pol2->GetParameter(0);
//                   par[1]=pol2->GetParameter(1);
//                   par[2]=pol2->GetParameter(2);

                }

		if( strstr(fit_func_name,"line") || fit_func_name[0]=='l' || fit_func_name[0]=='L'
			 || fit_func_name[0]=='h' || fit_func_name[0]=='H' ){
			pGraph->Fit("fit_func","R");
		}

		if( local_func ){
		   line->GetParameters(par);
//   		line->Draw("same");
			line_draw->SetParameters(par);
			line_draw->Draw("same");
		}


		TLatex lat;
   	lat.SetTextAlign(23);
	   lat.SetTextSize(0.05);

   	TString szA,szB,szConst,szDesc1,szDesc2;

		printf("minX=%.2f maxX=%.2f minY=%.2f maxY=%.2f\n",minX,maxX,minY,maxY);
//		if( strcmp(fit_func_name,"line")==0 || fit_func_name[0]=='l' || fit_func_name[0]=='L'){



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

//		}
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

	time_t ut_start_time = (time_t)gStartTime;
//   if( !(gmtime_tm = gmtime( &ut_start_time )) ){
	if( !(gmtime_tm = localtime( &ut_start_time )) ){	
		printf("ERROR : could not convert unix time %d to ut time\n",ut_start_time);
		return NULL;
	}
	localtime_tm = localtime( &ut_start_time );
	printf("%.2u%.2u%.2u_%.2u%.2u%.2u\n",gmtime_tm->tm_year+1900,(gmtime_tm->tm_mon+1),gmtime_tm->tm_mday,gmtime_tm->tm_hour,gmtime_tm->tm_min,gmtime_tm->tm_sec);

	char szStartLocalTime[256];   
	localtime_tm = localtime( &ut_start_time );
	sprintf(szStartLocalTime,"%.2u%.2u%.2u_%.2u%.2u%.2u\n",localtime_tm->tm_year+1900,(localtime_tm->tm_mon+1),localtime_tm->tm_mday,localtime_tm->tm_hour,localtime_tm->tm_min,localtime_tm->tm_sec);
	

	pGraph->GetXaxis()->SetTimeDisplay(1);
	TDatime da(gmtime_tm->tm_year+1900,gmtime_tm->tm_mon+1,gmtime_tm->tm_mday,gmtime_tm->tm_hour,gmtime_tm->tm_min,gmtime_tm->tm_sec);
//	pGraph->GetXaxis()->SetTimeOffset((time_t)gStartTime+3600);
//	pGraph->GetXaxis()->SetTimeOffset(da.Convert(kFALSE)+3600);
//	pGraph->GetXaxis()->SetTimeOffset(da.Convert(kTRUE)+3600*8);
	pGraph->GetXaxis()->SetTimeOffset(da.Convert(kFALSE),"local");
        printf("DEBUG : %s from ux=%d\n",da.AsString(),da.Convert(kFALSE));		

	char szTmp[128];
	strftime(szTmp,128,"%y/%m/%d %H:%M",gmtime_tm);
	printf("strftime = %s\n",szTmp);
	
   pGraph->GetXaxis()->SetTimeFormat("%y%m%d");
//	pGraph->GetXaxis()->SetTimeFormat("%H:%M%F2012-05-17 04:00:00");


	TLatex lat;
   lat.SetTextAlign(23);
   lat.SetTextSize(0.05);
                                                                                
   char szDesc[255];
//   sprintf(szDesc,"sigma/mean = %.8f\n",r);
	sprintf(szDesc,"Start time = %s (LOCAL)",szStartLocalTime);
   printf("Ploting |%s| at (%.2f,%.2f)\n",szDesc,(minX+maxX)/2 , maxY-5);
   lat.DrawLatex( (minX+maxX)/2 , maxY-5 , szDesc);

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
//		ncols = sscanf(buff,"%f\t%f\n",&x_val,&y_val);

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

	   if( x_val <= 0 ){ 
        continue;
      }

	  
	  if(CondCol>=0){
          if(lval1!=CondValue)
		      continue;	      				 	  				 
	  }
     // printf("%s\n",buff);
	  

	  time_t ux = x_val;
	  struct tm* localtm = localtime(&ux);
	  int local_hour = localtm->tm_hour*10000 + localtm->tm_min*100 + localtm->tm_sec;
	  if( ((local_hour>=gMinLocalHour && local_hour<=gMaxLocalHour) && gMaxLocalHour>gMinLocalHour) || 
         ((local_hour>=gMinLocalHour || local_hour<=gMaxLocalHour) && gMaxLocalHour<gMinLocalHour)
       ){
		  if( gVerb>=2 ){
   	     printf("Local time %.2f OK -> %.06d accepted (limits = %06d - %06d )\n",x_val,local_hour,gMinLocalHour,gMaxLocalHour);
		  }
	  }else{
		  if( gVerb>=2 ){
   		  printf("OUTSIDE LIMITS : Local time %.2f -> %.06d skipped (limits = %06d - %06d )\n",x_val,local_hour,gMinLocalHour,gMaxLocalHour);
		  }
        continue;
	  }
	  if( gMinUxtime > 0 ){
        if( ux < gMinUxtime )
           continue;
	  }
	  if( gMaxUxtime > 0 ){
        if( ux > gMaxUxtime )
           continue;
	  }

     if( gStartTime <= 0.001 && all==0 ){
         gStartTime = x_val;
         printf("START TIME SET TO : %.8f\n",gStartTime);
     }

	  x_values[all] = (x_val - gStartTime);
	  y_values[all] = y_val;
	  if( gVerb ){
   	     printf("values : %f %f\n",x_val,y_val);
          }

	  all++;
   }
   fclose(fcd);

//	exit(0);   
   return all;
}  

void plot_plidx_vs_time_error( const char* basename="sigmaG1_vs_lapSigmaG1_for_root", double min_y=-6, double max_y=0,
                              const char* szDescY="SNR : fitted power law index vs. time",
                              int x_col=0, int y_col=2, int y_col_err=3,
                                        int min_local_time=-1e6, int max_local_time=1e6,
					const char* fit_func_name=NULL,
					int bLog=0,
		const char* szDescX="Local Time", const char* szTitle=NULL,
		int b_percent=0,
		double fit_min_x=-100000, double fit_max_x=-100000,
      double min_uxtime=-10000, double max_uxtime=-10000, const char* outdir="images/" )
{
	gPercent = b_percent;
	gMinLocalHour = min_local_time;
	gMaxLocalHour = max_local_time;
	gMinUxtime = min_uxtime;
	gMaxUxtime = max_uxtime;

	if( !szTitle){
		szTitle = basename;
	}
	gLog = bLog;
	
   // gROOT->Reset();
   // const char* basename = "s_vs_sigma_g_sqr";

   TCanvas* c1 = new TCanvas("c1","xxxx",10,10,1800,1000);
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



	// p1 = new TPad("p1","p1",0,0,1,1,-1);
	// p1->Draw();


   Double_t* x_value1 = new Double_t[MAX_ROWS];
   Double_t* y_value1 = new Double_t[MAX_ROWS];
   Double_t* x_value_err1 = new Double_t[MAX_ROWS];
   Double_t* y_value_err1 = new Double_t[MAX_ROWS];
   Double_t maxX=-100000,maxY=-100000;	
   Double_t minX=100000,minY=100000;
	Double_t first_value;

   Int_t ncols;
   Int_t lq1=0,lq2=0,lq3=0,lq5=0,lq9=0,lq25=0;

   lq1 = ReadResultsFile( basename, x_value1, y_value1, -1, -1, x_col, y_col ); 
   Int_t lq1_err = ReadResultsFile( basename, x_value_err1, y_value_err1, -1, -1, x_col+1, y_col_err ); 

	time_t ut_start_time = (time_t)gStartTime;
   if( !(gmtime_tm = gmtime( &ut_start_time )) ){
//	if( !(gmtime_tm = localtime( &ut_start_time )) ){	
		printf("ERROR : could not convert unix time %d to ut time\n",ut_start_time);
		return;
	}
	printf("%.2u%.2u%.2u_%.2u%.2u%.2u\n",gmtime_tm->tm_year+1900,(gmtime_tm->tm_mon+1),gmtime_tm->tm_mday,
                           gmtime_tm->tm_hour,gmtime_tm->tm_min,gmtime_tm->tm_sec);
//	TDatime da(gmtime_tm->tm_year+1900,gmtime_tm->tm_mon+1,gmtime_tm->tm_mday,gmtime_tm->tm_hour,gmtime_tm->tm_min,gmtime_tm->tm_sec);
//	gStyle->SetTimeOffset(da.Convert());
//	printf("DEBUG : %s\n",da.AsString());


   
   // drawing background graphs here :
   TGraph* pGraph1 = DrawGraph( x_value1, y_value1, NULL, y_value_err1, lq1, 1, NULL, 
				  fit_func_name, min_y, max_y, szTitle,
				basename, bLog, szDescX, szDescY, fit_min_x,
				fit_max_x );
	
   c1->Update();

   TString szEpsName1=outdir;
//  szEpsName1 += basename;
//   szEpsName1 += ".eps";
//   c1->Print(szEpsName1.Data());

   gSystem->mkdir(outdir);

   TString szPngName1=outdir;
   szPngName1 += basename;
   szPngName1 += ".png";
   c1->Print(szPngName1.Data());
}
