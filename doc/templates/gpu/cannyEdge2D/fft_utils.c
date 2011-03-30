#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "fft_utils.h"

/*
 * These routines is usefull to fit data into the 
 * format used by rfftwnd_one_real_to_complex 
 */ 
/* recall memmove prototype :
 *   void *memmove(void *dest, const void *src, size_t n);
 */
void Shift_Buffer_2D(float *indata, int M, int N) 
{ 
  int i; 
  
  for (i=M-1; i>=0; i--)
    memmove(indata+i*2*(N/2+1),indata+i*N,N*sizeof(float));
}

void Shift_Inv_Buffer_2D(float *indata, int M, int N)
{
  int i;
  
  for (i=0; i<M; i++)
    memmove(indata+i*N,indata+i*2*(N/2+1),N*sizeof(float));
  //for (i=M-1; i<=0; i--)
  // memmove(indata+i*N,indata+i*2*(N/2+1),N*sizeof(fftw_real));
}

int SaveBuffer4xsmurf2D_extimage(void *bufferOut, void *bufferOut2,
                                 int lx, int ly, char *extImageFilename,
                                 float coef)
{
  FILE     *fileOut;
  char     *proc = "SaveBuffer4xsmurf2D_extimage";
  int      sizeBuf = lx*ly;
  int      i, NbExtr;
  
  if (!(fileOut = fopen(extImageFilename, "w")))
    fprintf( stderr, "%s: Couldn't open `%s' for writing.\n", proc, extImageFilename);
  
  /*
   * How many extrema ??
   */
  NbExtr = 0;
  {
    float *tmp = (float *) bufferOut;
    for (i = 0; i < sizeBuf; i++) {
      if (*tmp>0) 
        NbExtr++;
      tmp++;
    }
  }

  fprintf(fileOut,
          "Binary ExtImage %dx%d %d %f"
          "(%d byte reals, %d byte ints)\n",
	  lx, ly,NbExtr, coef, (int) sizeof(float),
          (int) sizeof(int));
  {
    float *tmp = (float *) bufferOut;
    float *arg = (float *) bufferOut2;
    for (i = 0; i < sizeBuf; i++) {
      if (*tmp>0) {
        fwrite(&i,  sizeof(int),   1, fileOut);
        fwrite(tmp, sizeof(float), 1, fileOut);
        fwrite(arg, sizeof(float), 1, fileOut);
      }
      tmp++; arg++;
    }
  }
  
  fclose(fileOut);
  return( EXIT_SUCCESS );
}
