#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>


#include <bg_globals.h>
#include <bg_fits.h>
#include <bg_array.h>
#include <bg_bedlam.h>
#include <cvalue_vector.h>

#include <myfile.h>
#include <mystring.h>


#include <vector>
using namespace std;


enum eNormalisationType { eNoNorm=0, eNormMinMax=1, eNormRMS=2 };
eNormalisationType gNormaliseProfile = eNoNorm;

string gOutputDir="./";
string gInputFile = "DM_vs_UX.txt";
string gOutFitsFile="series.fits";
double gPeakAvoidInterval = 0.02; // seconds - where to calculate MEDIAN and RMSIQR 
double gOffPulseInterval=0.02; // in seconds
long int gOffPulseIntervalInSamples=(long int)(0.02/((1.08*64*14)/1000.00)); // in ms 
int gLocalMedianSamples=40;
double gSamplingTime = (1.08*64*14)/1000000.00; // in seconds

bool gSaveFITS=false;

// testing DM
double gTestDM = 56.73;
double gStartDM = -1;
double gEndDM   = -1;
double gStepDM  = 0.001;

// metadata :
double gUNIXTIME=-1;

// debug level 
int gVerb=0;

void usage()
{
   printf("main_structure_function_1D.cpp TIMESERIES.txt\n");
   printf("-o OUTDIR : output directory [default %s]\n",gOutputDir.c_str());
/*   printf("-S START_DM [default %.6f]\n",gStartDM);
   printf("-E START_DM [default %.6f]\n",gEndDM);
   printf("-s STEP_DM [default %.6f]\n",gStepDM);
   printf("-D DM [default %.6f]\n",gTestDM);
   printf("-U UNIXTIME [default %.8f]\n",gUNIXTIME);*/

   exit(-1);
}


void parse_cmdline(int argc, char * argv[]) {
   char optstring[] = "a:ho:S:E:s:D:U:";
   int opt,opt_param,i;
        
   while ((opt = getopt(argc, argv, optstring)) != -1) {
      printf("DEBUG : opt = %c, optarg = %s\n",opt,optarg);
   
      switch (opt) {
         case 'o' :
            gOutputDir = optarg;
            printf("DEBUG : set output directory to %s\n",gOutputDir.c_str());
            break;
            
         case 'D' :
            gTestDM = atof( optarg );
            break;
            
         case 'S' :
            gStartDM = atof( optarg );
            break;
            
         case 'E' :
            gEndDM = atof( optarg );
            break;
            
         case 's' :
            gStepDM = atof( optarg );
            break;
            
         case 'U' :
            gUNIXTIME = atof( optarg );
            break;
            
         case 'h':
            usage();
            break;
         default:  
            fprintf(stderr,"Unknown option %c\n",opt);
            usage();
      }
   }   
}
 

void print_parameters()
{
   printf("#####################################\n");
   printf("PARAMETERS :\n");
   printf("#####################################\n");
   printf("Input fits file  = %s\n",gInputFile.c_str());
   printf("Output directory = %s\n",gOutputDir.c_str());  
/*   printf("DM range = %.6f - %.6f\n",gStartDM,gEndDM);
   printf("DM step = %.6f\n",gStepDM);
   printf("Test DM = %.6f\n",gTestDM);*/
   printf("#####################################\n");   
}

void dedisperse_fits( CBgFits& dynaspec, double DM )
{
   double delta_x = dynaspec.cdelt1*1000.00; // Multiplied by 100 to convert to ms
   double starting_frequency = 199.60214170; // Starting frequency used by PRESTO 
   double delta_y = dynaspec.cdelt2;

   std::vector<double> frequencies;
   int n_ch = dynaspec.GetYSize();
   for(int ch=0;ch<n_ch;ch++){
      frequencies.push_back(ch*delta_y);
   }


   double freq = starting_frequency + (n_ch-1)*delta_y;
   
   // de-dispersion :
   for(int ch=0;ch<n_ch;ch++){
      double fch1 = starting_frequency + (ch * delta_y);
      
      // tdelay = Decimal(Decimal(4.15)* 10**6 * DM * ((fch1**-2) - (freq**-2)))
      double tdelay = 4.15*1000000.00*DM*(1.00/(fch1*fch1) - 1.00/(freq*freq));
      
      double shift = (tdelay / delta_x);
      int shift_int = -int(round(shift));
      
      if( gVerb >= 1 ){
         printf("DEBUG : channel = %d -> shift = %.4f -> %d\n",ch,shift,shift_int);
      }
      dynaspec.roll( ch, shift_int );      
   }
}

#define REPLACE_ELEMS( tab, pos1, pos2 ) { tmp=tab[pos1]; tab[pos1]=tab[pos2]; tab[pos2]=tmp; } 

void my_sort_float( float* ftab, long cnt )
{
        float divider = ftab[0];

        int beg = 1;
        int end = cnt-1;
        float tmp;

        if(cnt){        
                while(beg<=end){
                        if(ftab[beg]>divider){
                                if(ftab[end]<=divider){
                                        REPLACE_ELEMS( ftab, beg, end )
                                        beg++;
                                        end--;
                                }else{
                                        end--;
                                }
                        }else{          
                                beg++;
                                if(ftab[end]>divider)
                                        end--;
                        }
                }
                if(end!=0){
                        REPLACE_ELEMS( ftab, end, 0)
                }

                my_sort_float( ftab, end );
                my_sort_float( &(ftab[beg]), cnt-beg );
        }

}


double get_trim_median( double n_sigma_iqr, double* tab, int& cnt, double& sigma_iqr )
{   
   double* newtab = new double[cnt];

   int q75= (int)(cnt*0.75);
   int q25= (int)(cnt*0.25);
   double iqr = tab[q75]-tab[q25];
   sigma_iqr = iqr/1.35;
   double range = sigma_iqr*n_sigma_iqr;
   double median = tab[(int)cnt/2];

   int newcnt=0;
   for(int i=0;i<cnt;i++){
      if( fabs(tab[i]-median) <= range ){
         newtab[newcnt] = tab[i];
         newcnt++;
      }
   }

   double ret=newtab[newcnt/2];

   // returning smaller array :
   cnt = newcnt;
   for(int i=0;i<newcnt;i++){
      tab[i] = newtab[i];
   }   

   delete [] newtab;
   return ret;
}

// INPUT  : 
// intab  : sorted table                                                 
// cnt    : number of elements in a table
// n_iter : number of iterations 
void GetAvgEstimator( float* intab, int cnt, int n_iter, double& sigma_iqr, double& median ) // , int x )
{
   double* tab = new double[cnt]; // working copy 
   for(int i=0;i<cnt;i++){
      tab[i] = intab[i];
   }

   int newcnt=cnt;
   median = tab[cnt/2];
   
   for(int i=0;i<n_iter;i++){
      median = get_trim_median( 3.00, tab, newcnt, sigma_iqr );
   }   

   delete [] tab;
}





void calc_median_rmsiqr( float* data_buffer, long int n_floats, long int start, long int end, double& median, double& rmsiqr )
{
   long int tmp_buffer_size = (end-start+1);
   float* tmp_buffer = new float[tmp_buffer_size];
   int tmp_count=0;
   
   for(long int i=start;i<end;i++){
      tmp_buffer[tmp_count] = data_buffer[i];
      tmp_count++;
   }
   
   my_sort_float( tmp_buffer, tmp_count );
   GetAvgEstimator( tmp_buffer, tmp_count, 5, rmsiqr, median );
   
   
   delete [] tmp_buffer;
}

void calc_median_rmsiqr( std::vector<double>& data_buffer, long int start, long int end, double& median, double& rmsiqr )
{
   int n_floats = data_buffer.size();
   long int tmp_buffer_size = (end-start+1);
   float* tmp_buffer = new float[tmp_buffer_size];
   int tmp_count=0;
   
   for(long int i=start;i<end;i++){
      tmp_buffer[tmp_count] = data_buffer[i];
      tmp_count++;
   }
   
   my_sort_float( tmp_buffer, tmp_count );
   GetAvgEstimator( tmp_buffer, tmp_count, 5, rmsiqr, median );
   
   
   delete [] tmp_buffer;
}

int find_peak( std::vector<double>& timeseries, int& min_t )
{
   double maxval=-1;
   int max_t=-1;
   
   min_t=-1;
   double minval = 1e20;
      
   for(int t=0;t<timeseries.size();t++){
      if( timeseries[t] > maxval ){
         maxval = timeseries[t];
         max_t = t;
      }
      if( timeseries[t] < minval ){
         minval = timeseries[t];
         min_t = t;
      }
   }
   
   return max_t;
}

void structure_function( CValueVector& timeseries, double tolerance=0.2 )
{
   int size = timeseries.size();
   double max_delta_time = timeseries[size-1].x - timeseries[0].x;
   double half_delta_time = max_delta_time/2.00;
   int half_delta_time_int = int(round(half_delta_time));
   
   printf("DEBUG : structure_function : max_delta_time = %.6f days -> half_delta_time = %.6f days\n",max_delta_time,half_delta_time);
   
   CValueVector out;
   out.init(half_delta_time_int);
   
   for(int delta_time=1;delta_time<half_delta_time_int;delta_time++){
      
      // 
      double sum = 0.00;
      bool bContinue = true;
      for(int i=0;(i<size && bContinue);i++){
         for(int j=(i+1);(j<size && bContinue);j++){
            double delta_time_data = timeseries[j].x - timeseries[i].x;
            
            double error = (delta_time_data - delta_time);
            printf("DEBUG : delta_time = %d vs. error = %.8f days ( %.6f and %.6f )\n",delta_time,error,timeseries[j].x,timeseries[i].x);
            if (fabs(error)<tolerance){
               double delta_dm = timeseries[j].y - timeseries[i].y;
               sum += delta_dm*delta_dm;
               printf("DEBUG : delta_time = %d -> added DM_%d - DM_%d (%.6f days)\n",delta_time,j,i,delta_time_data);
            }else{
               if( error > (2*delta_time) ){
                  // Data are sorted by time so if we are already at error (i.e. difference) of 2*delta_time means we can move on to next i:
                  break;
               }
            }
         }
      }
      out[delta_time].x = delta_time;
      out[delta_time].y = sum;                  
   }

   FILE* outf = fopen("DM_structure_function.txt","w");   
   for(int i=0;i<out.size();i++){
      printf("%.8f %.8f\n",out[i].x,out[i].y);      
      fprintf(outf,"%.8f %.8f\n",out[i].x,out[i].y);
   }
   fclose(outf);
}

int main(int argc,char* argv[])
{
  time_t start_time = get_dttm();
  if(( argc>=2 && (strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--h")==0)) ){
     usage();
  }

  if( argc>=2 ){
     gInputFile = argv[1];  
  }
  parse_cmdline(argc,argv); // -1,+1
  print_parameters();
  
  // create output directory :
  MyFile::CreateDir( gOutputDir.c_str() );
  printf("INFO : created directory %s\n",gOutputDir.c_str());

  CValueVector timeseries;
  int n = timeseries.read_file( gInputFile.c_str(), 0, 0, 1 );
  printf("Read %d points from input file %s (first point = %.8f / %.8f)\n",n,gInputFile.c_str(),timeseries[0].x,timeseries[0].y );

  // calculate structure function:  
  structure_function( timeseries );  
   
}
