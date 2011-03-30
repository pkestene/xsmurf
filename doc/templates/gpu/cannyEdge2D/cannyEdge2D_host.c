/**************************************************
 * cannyEdge2D_host.c - program of 2D edges detection
 *                      run on host (nothing on GPU)
 *
 *
 * DESCRIPTION:
 * programme test pour port vers GPU (NVIDIA/CUDA)
 * et benchmark
 *
 * P. Kestener (CEA, IRFU/SEDI, March 2008)
 **************************************************/

#include <stdio.h>
#include <malloc.h>
#include <fcntl.h> /* open, close */
#include <sys/stat.h> /* open, close */
#include <sys/types.h> /* open, close */
#include <unistd.h> /* write */
#include <stdlib.h> /* to make 'atof' useable */
#include <string.h>

#include <time.h>

#include <math.h>

#include <getopt.h>
#include <matheval.h>

/* parse command line arguments */
#include "cmd_cannyEdge2D_host.h"
/*
 * calling command line
 *
 * ./cannyEdge2D_host -o edge -i image.xsm
 *
 * or
 *
 * ./cannyEdge2D_host --output edge --input image.xsm
 *
 */


/* custom header */
#include "fft.h"
#include "edge.h"
#include "misc.h"

#define FORWARD 0
#define BACKWARD 1

int main( int argc, char* argv[] )
{
  /* data buffers */
  FILE *fileIn;
  int bufferLength, bufferLength2;

  /* buffer for gradient components */
  float *datain, *gradx, *grady;
  float *mod, *arg, *max;

  /* input image parameters */
  int thesize,lx,ly,Lx,Ly;
  unsigned int type;
  char    tempBuffer[100],saveFormat[10];

  /* output */
  char outputName[80];

  /* execution time variables */
  int t0, t1;

  /* command line argument parser */
  struct gengetopt_args_info args_info;

  /* other variables */
  int i;
  float amin=1.0;
  int octN = 1;
  int voxN = 5;
  float scale;
  char scaleId[4];

  void *fct_x, *fct_y, *fct_null;
  
  /* let's call our cmdline parser */
  if (cmdline_parser (argc, argv, &args_info) != 0)
    exit(EXIT_FAILURE);
  
  /* parse output parameter */
  if (strlen(args_info.output_arg) >70) {
    fprintf(stderr, "Output filename prefix is too large.\n");
    exit(EXIT_FAILURE);
  }
  strcpy(outputName, args_info.output_arg);

  if (args_info.octave_given)
    octN = args_info.octave_arg;
  if (args_info.vox_given)
    voxN = args_info.vox_arg;
  
  /*
   * open input 2D image (float data, Xsmurf format)
   */
  if (!(fileIn = fopen( args_info.input_arg , "r"))) {
    fprintf( stderr, "Couldn't open '%s' for reading.", args_info.input_arg);
    exit(EXIT_FAILURE);
  } 
  
  /*
   * read data binary xsmurf format
   */
  //fscanf(fileIn, "Binary %d %dx%d %d(%d byte reals)\n", type, lx, ly, thesize,(int) sizeof(real));
  fgets(tempBuffer, 100, fileIn);
  sscanf(tempBuffer, "%s %d %dx%d %d",
         saveFormat, &type, &lx, &ly, &thesize);
  
  /*
   * take care that we have to swap lx and ly for fftw 
   * because fftw uses row-major whereas xsmurf uses col-major
   * 
   */
  Lx = ly;
  Ly = lx;
  
  bufferLength = Lx*Ly;
  bufferLength2 = Lx*2*(Ly/2+1);
 
  /*
   * memory allocation
   */
  datain = (float*)malloc( bufferLength2 * sizeof(float) );
  gradx = (float*)malloc( bufferLength2 * sizeof(float) );
  grady = (float*)malloc( bufferLength2 * sizeof(float) );
  if ( !datain || !gradx || !grady) {
    fprintf( stderr, " allocation of buffer in failed.\n" );
    exit( EXIT_FAILURE );
  }

  /* reading data */
  fread(datain, sizeof(float), bufferLength, fileIn);
  /* close input */
  fclose( fileIn );

  t0 = clock( );

  /*
   * perform inplace forward Fourier transform
   */
  im_fftw2d_real (datain, NULL, Lx, Ly, FORWARD, 1);
 
  fct_x = evaluator_create("y*exp(-x*x-y*y)");
  fct_y = evaluator_create("x*exp(-x*x-y*y)");
  fct_null = evaluator_create("0");
  
  mod = (float *) malloc(bufferLength * sizeof(float));
  arg = (float *) malloc(bufferLength * sizeof(float));
  max =  (float *) malloc(bufferLength * sizeof(float));
  
  /*
   * LOOP overs scales
   */
  {
    int num=0;
    int oct, vox;
    for (oct = 0; oct < octN; oct++) {
      for (vox = 0; vox < voxN; vox++, num++) {
	scale = 6.0/0.86*amin*powf(2,oct+(vox/(float)voxN));
	sprintf(scaleId,"%.3d",num);
	strcpy(outputName, args_info.output_arg);
	strcat(outputName,scaleId);
	printf("%s\n",outputName);
	
	//memcpy(grady, gradx, bufferLength2 * sizeof(float));
	for (i=0; i<bufferLength2; i++) {
	  gradx[i] = datain[i];
	  grady[i] = datain[i];
	}
	
	/* apply filter in Fourier space to obtain the gradient itself */
	im_fftw_filter (gradx, Lx, Ly, scale, fct_null, fct_x);
	im_fftw_filter (grady, Lx, Ly, scale, fct_null, fct_y);
	
	/*
	 * perform inplace backward Fourier transform
	 */
	im_fftw2d_real (gradx, NULL, Lx, Ly, BACKWARD, 1);
	im_fftw2d_real (grady, NULL, Lx, Ly, BACKWARD, 1);
	
	/*
	 * Compute gradient in polar representation (modulus, argument)
	 */
	for ( i=0; i<lx*ly; i++) {
	  mod[i] = sqrtf( gradx[i]*gradx[i] +  grady[i]*grady[i] );
	  arg[i] = atan2f( grady[i], gradx[i] );
	}
	
	/* 
	 * Compute 2D edges; remove point non-maxima
	 */
	Remove_Gradient_NonMaxima_Slice_2D( max, gradx ,grady, mod, lx, ly );
	
	/*
	 * save bufferOut in an xsmurf format
	 */
	SaveBuffer4xsmurf2D_extimage(max, arg, lx, ly, outputName, scale);
      }
    }
  }

  evaluator_destroy(fct_x);
  evaluator_destroy(fct_y);
  evaluator_destroy(fct_null);
  

  t1 = clock( );
  fprintf( stdout, "       processing time was %d microseconds.\n", t1-t0 );


  /*
   * memory free
   */
  free(datain);
  free( gradx );
  free( grady );
  free( mod );
  free( arg );
  free ( max );
 
  return 0;
}
