/**************************************************
 * cannyEdge2D_cuda.c - program of 2D edges detection
 *                                        run on host and device (GPU)
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
#include "cmd_cannyEdge2D_cuda.h"

/*
 * calling command line
 *
 * ./cannyEdge2D_cuda -o edge -i image.xsm
 *  ../../bin/linux/release/cannyEdge2D_cuda -i image.xsm -o edge
 *
 * or
 *
 * ./cannyEdge2D_cuda --output edge --input image.xsm
 *
 */

/* 
 * CUDA header
 */ 
// includes, project
#include <cufft.h>
#include <cutil.h>

// Complex data type
typedef float2 Complex; 


/* custom header */
#include "fft_utils.h"
/*#include "edge.h"
#include "misc.h"*/

#define FORWARD 0
#define BACKWARD 1


// Thread block size
//#define BLOCK_SIZE 8


/* kernel functions */
#include "cannyEdge2D_cuda_kernel.cu"
#include <reduction.cu>



int main( int argc, char* argv[] )
{
  /* data buffers */
  FILE *fileIn;

  /* buffer for gradient components */
  /*float *gradx, *grady;
    float *mod, *arg, *max;*/

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

  /* cuda related variables */ 
  cudaError_t res;
  int BLOCK_SIZE=8;

  /* let's call our cmdline parser */
  if (cmdline_parser (argc, argv, &args_info) != 0)
    exit(EXIT_FAILURE);
  
  /* parse output parameter */
  if (strlen(args_info.output_arg) >70) {
    fprintf(stderr, "Output filename prefix is too large.\n");
    exit(EXIT_FAILURE);
  }
  strcpy(outputName, args_info.output_arg);

  if (args_info.blocksize_given)
    BLOCK_SIZE = args_info.blocksize_arg;
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
  
  /********************
   * CUDA
   ********************/
  CUT_DEVICE_INIT();

  // display CUDA device info
  int deviceCount;
  CUDA_SAFE_CALL(cudaGetDeviceCount(&deviceCount));
  for (int dev = 0; dev < deviceCount; ++dev) {
    cudaDeviceProp deviceProp;
    CUDA_SAFE_CALL(cudaGetDeviceProperties(&deviceProp, dev));
    printf("\nDevice %d: \"%s\"\n", dev, deviceProp.name);
    printf("  Major revision number:                         %d\n", deviceProp.major);
    printf("  Minor revision number:                         %d\n", deviceProp.minor);
    printf("  Total amount of global memory:                 %d bytes\n", deviceProp.totalGlobalMem);
    printf("  Clock rate:                                    %d kilohertz\n", deviceProp.clockRate);
  }


  /*
   * PINNED memory allocation on host
   */
  Complex *dataIn;
  res=cudaMallocHost((void **) &dataIn, Lx*Ly* sizeof(Complex));
  if (res != 0) {
    fprintf(stderr,"failed to alloc host mem for dataIn\n");
    exit(EXIT_FAILURE);
  }
  
  /* reading input data */
  float datain;
  for (i=0;i<Lx*Ly;i++) {
    fread(&datain, sizeof(float), 1, fileIn);
    dataIn[i].x = datain;
    dataIn[i].y = 0.0f;
  }
  
  /* close input */
  fclose( fileIn );
  
  t0 = clock( );

  /* allocated memory on device */
  Complex *deviceFourier, *deviceGradx, *deviceGrady;
  CUDA_SAFE_CALL(cudaMalloc((void**)&deviceFourier, Lx*Ly*sizeof(Complex) ));
  CUDA_SAFE_CALL(cudaMalloc((void**)&deviceGradx, Lx*Ly*sizeof(Complex) ));
  CUDA_SAFE_CALL(cudaMalloc((void**)&deviceGrady, Lx*Ly*sizeof(Complex) ));

  /* copy data to device */
  CUDA_SAFE_CALL(cudaMemcpy(deviceFourier, dataIn, Lx*Ly*sizeof(Complex), cudaMemcpyHostToDevice));

  /* create cuFFT plan */
  cufftHandle plan;
  CUFFT_SAFE_CALL(cufftPlan2d(&plan, Lx, Ly, CUFFT_C2C));
        
  /* perform cuFFT */
  CUFFT_SAFE_CALL(cufftExecC2C(plan, (cufftComplex *)deviceFourier, (cufftComplex *)deviceFourier, CUFFT_FORWARD));

  /* modulus, argument buffers on device */
  float *deviceMod, *deviceArg;
  CUDA_SAFE_CALL(cudaMalloc((void**)&deviceMod, Lx*Ly*sizeof(float) ));
  CUDA_SAFE_CALL(cudaMalloc((void**)&deviceArg, Lx*Ly*sizeof(float) ));
  float *deviceMaxima;
  CUDA_SAFE_CALL(cudaMalloc((void**)&deviceMaxima, lx*ly*sizeof(float) ));
  
  /* some host buffer for output */
  float *maximaMask;
  float *arg;
  cudaMallocHost((void **) &maximaMask, Lx*Ly* sizeof(float));
  cudaMallocHost((void **) &arg, Lx*Ly* sizeof(float));

  /*
   * LOOP overs scales
   */
  int num=0;
  for (int oct = 0; oct < octN; oct++) {
    for (int vox = 0; vox < voxN; vox++, num++) {
      scale = 6.0/0.86*amin*powf(2,oct+(vox/float(voxN)));
      sprintf(scaleId,"%.3d",num);
      strcpy(outputName, args_info.output_arg);
      strcat(outputName,scaleId);
      printf("%s\n",outputName);

      /* copy Fourier image into gradx and grady before performing filtering */
      CUDA_SAFE_CALL(cudaMemcpy(deviceGradx, deviceFourier, Lx*Ly*sizeof(Complex), cudaMemcpyDeviceToDevice));
      CUDA_SAFE_CALL(cudaMemcpy(deviceGrady, deviceFourier, Lx*Ly*sizeof(Complex), cudaMemcpyDeviceToDevice));
      
      /* execution parameter for Fourier filtering  */
      dim3 gridSize(Lx/BLOCK_SIZE, Ly/BLOCK_SIZE);
      dim3 blockSize(BLOCK_SIZE, BLOCK_SIZE);
      fft_filter_gradx_kernel<<< gridSize, blockSize >>>(deviceGradx,Lx,Ly,scale);
      fft_filter_grady_kernel<<< gridSize, blockSize >>>(deviceGrady,Lx,Ly,scale);      
      
      /*
       * perform inplace backward Fourier transform
       */
      CUFFT_SAFE_CALL(cufftExecC2C(plan, (cufftComplex *)deviceGradx, (cufftComplex *)deviceGradx, CUFFT_INVERSE));
      CUFFT_SAFE_CALL(cufftExecC2C(plan, (cufftComplex *)deviceGrady, (cufftComplex *)deviceGrady, CUFFT_INVERSE));

      /*
       * Compute gradient in polar representation (modulus, argument)
       */
      polar_gradient_kernel<<< gridSize, blockSize >>>(deviceMod,deviceArg, deviceGradx, deviceGrady,Lx,Ly);
      
      /*
       * Compute 2D edges; remove point non-maxima
       */
      remove_nonmaxima_gradient2D_kernel<<< gridSize, blockSize >>>(deviceMaxima, deviceGradx, deviceGrady,deviceMod,lx,ly); 
      
      /*
       * save bufferOut in an xsmurf format
       */      
      CUDA_SAFE_CALL(cudaMemcpy(maximaMask,deviceMaxima, Lx*Ly*sizeof(float), cudaMemcpyDeviceToHost));
      CUDA_SAFE_CALL(cudaMemcpy(arg,deviceArg, Lx*Ly*sizeof(float), cudaMemcpyDeviceToHost));
      SaveBuffer4xsmurf2D_extimage(maximaMask, arg, lx, ly, outputName, scale); 
    }
  }
  
  /* cufft plan destroy */
  cufftDestroy(plan);
  
  CUDA_SAFE_CALL(cudaFree(deviceFourier));
  CUDA_SAFE_CALL(cudaFree(deviceGradx));
  CUDA_SAFE_CALL(cudaFree(deviceGrady));
  CUDA_SAFE_CALL(cudaFree(deviceMod));
  CUDA_SAFE_CALL(cudaFree(deviceArg));
  CUDA_SAFE_CALL(cudaFree(deviceMaxima));
  CUDA_SAFE_CALL(cudaFreeHost(maximaMask));
  CUDA_SAFE_CALL(cudaFreeHost(arg));
  CUDA_SAFE_CALL(cudaFreeHost(dataIn)); 
  
  t1 = clock( );
  fprintf( stdout, "       processing time was %d microseconds.\n", t1-t0 );
  
  
  //CUT_EXIT(argc, argv);
  
}

