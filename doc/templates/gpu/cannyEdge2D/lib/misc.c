#include <stdlib.h>
#include <stdio.h>

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


