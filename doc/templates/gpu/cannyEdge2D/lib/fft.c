#include <string.h>
#include <srfftw.h>
#include <matheval.h>

/*
 * These routines is usefull to fit data into the 
 * format used by rfftwnd_one_real_to_complex 
 */ 
/* recall memmove prototype :
 *   void *memmove(void *dest, const void *src, size_t n);
 */
static void Shift_Buffer_2D(float *indata, int M, int N) 
{ 
  int i; 
  
  for (i=M-1; i>=0; i--)
    memmove(indata+i*2*(N/2+1),indata+i*N,N*sizeof(float));
}

static void Shift_Inv_Buffer_2D(float *indata, int M, int N)
{
  int i;
  
  for (i=0; i<M; i++)
    memmove(indata+i*N,indata+i*2*(N/2+1),N*sizeof(float));
  //for (i=M-1; i<=0; i--)
  // memmove(indata+i*N,indata+i*2*(N/2+1),N*sizeof(fftw_real));
}

/*************************************************************************/
void im_fftw2d_real (float *in, float *out, int lx, int ly, int direction, int inplace)
{
  /*
   * Fourier transform using FFTW2 lib compiled with --enable-float
   */
  int i;

  /* declare fftw plan structure */
  rfftwnd_plan plan;

  /* when in place direct transform, we have to reorder data */
  if ((inplace == 1) && (direction == 0))
    Shift_Buffer_2D(in,lx,ly);

  /* initialize fftw plan */
  if ((direction == 0) && (inplace == 0))
    plan = rfftw2d_create_plan(lx, ly, FFTW_REAL_TO_COMPLEX,  FFTW_ESTIMATE);
  else if ((direction == 0) && (inplace == 1))
    plan = rfftw2d_create_plan(lx, ly, FFTW_REAL_TO_COMPLEX,  FFTW_ESTIMATE | FFTW_IN_PLACE);
  else if ((direction == 1) && (inplace == 0))
    plan = rfftw2d_create_plan(lx, ly, FFTW_COMPLEX_TO_REAL,  FFTW_ESTIMATE);
  else if ((direction == 1) && (inplace == 1))
    plan = rfftw2d_create_plan(lx, ly, FFTW_COMPLEX_TO_REAL,  FFTW_ESTIMATE | FFTW_IN_PLACE);

  /* compute fft */
  if (direction == 0) { /* direct */
    if (inplace)
      rfftwnd_one_real_to_complex(plan, (fftw_real *) in, NULL);
    else
      rfftwnd_one_real_to_complex(plan, (fftw_real *) in, (fftw_complex *) out);
  } else { /* reverse */
    if (inplace)
      rfftwnd_one_complex_to_real(plan, (fftw_complex *) in, NULL);
    else
      rfftwnd_one_complex_to_real(plan, (fftw_complex *) in, (fftw_real*) out);  
  }

  /* when in place reverse transform, we have to reorder data */
  if ((inplace == 1) && (direction == 1))
    Shift_Inv_Buffer_2D(in,lx,ly);

  /* normalisation */
  if (direction == 1) {
    if (inplace == 1)
      for (i=0;i<lx*2*(ly/2+1);i++)
	in[i] /= (lx*ly);
    else
      for (i=0;i<lx*ly;i++)
	out[i] /= (lx*ly);
  }

  /* delete fftw plan */
  rfftwnd_destroy_plan(plan);

}


/*************************************************************************/
/*
 * Compute the complex multiplication between an image of type FFTW_R2C 
 * and a function. The image data are organized as specified in the
 * fftw lib documentation (real multi-dimensional transform)
 */
void
im_fftw_filter (float  *data,
		int Lx,
		int Ly,
		float   scale,
		void *r_fct,
		void *i_fct)
{
  int           i, j;
  fftw_real     tmp, kx, ky;
  fftw_complex *cData;
  
  fftw_real     r_val, i_val;
  
  cData = (fftw_complex *) &(data[0]);

  for (i = 0; i < Lx; i++)
    for (j = 0; j < Ly/2+1; j++) {
      
      int ij = j + i*(Ly/2+1);
    
      if (i < Lx/2) {
	kx = (fftw_real) i*scale/Lx;
      } else {
	kx = (fftw_real) (i-Lx)*scale/Lx;
      }
      ky = (fftw_real) j*scale/Ly;
      
      r_val = ((fftw_real) evaluator_evaluate_x_y(r_fct, kx, ky));
      i_val = ((fftw_real) evaluator_evaluate_x_y(i_fct, kx, ky));

      tmp = cData[ij].re;
      
      cData[ij].re = cData[ij].re*r_val - cData[ij].im*i_val;
      cData[ij].im = tmp         *i_val + cData[ij].im*r_val;
      
    }

}
