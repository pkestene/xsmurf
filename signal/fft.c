/*
 * fft.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: fft.c,v 1.4 1998/08/19 14:57:01 decoster Exp $
 */

#include "signal_int.h"
#include "gfft.h"

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
/*#define M_PI 3.141592654*/

/*
 * Numerical recipes fft algorithm.
 */
static void 
FourNR (float *data,
	int   nn,
	int   isign)
{
  int n, mmax, m, j, istep, i;
  double wtemp, wr, wpr, wpi, wi, theta;
  float tempr, tempi;

  n = nn << 1;
  j = 1;
  for (i = 1; i < n; i += 2)
    {
      if (j > i)
	{
	  SWAP (data[j], data[i]);
	  SWAP (data[j + 1], data[i + 1]);
	}
      m = n >> 1;
      while (m >= 2 && j > m)
	{
	  j -= m;
	  m >>= 1;
	}
      j += m;
    }
  mmax = 2;
  while (n > mmax)
    {
      istep = 2 * mmax;
      theta = 6.28318530717959 / (isign * mmax);
      wtemp = sin (0.5 * theta);
      wpr = -2.0 * wtemp * wtemp;
      wpi = sin (theta);
      wr = 1.0;
      wi = 0.0;
      for (m = 1; m < mmax; m += 2)
	{
	  for (i = m; i <= n; i += istep)
	    {
	      j = i + mmax;
	      tempr = wr * data[j] - wi * data[j + 1];
	      tempi = wr * data[j + 1] + wi * data[j];
	      data[j] = data[i] - tempr;
	      data[j + 1] = data[i + 1] - tempi;
	      data[i] += tempr;
	      data[i + 1] += tempi;
	    }
	  wr = (wtemp = wr) * wpr - wi * wpi + wr;
	  wi = wi * wpr + wtemp * wpi + wi;
	}
      mmax = istep;
    }
}
#undef SWAP

/*
 *  Call the numerical recipes fft algoritm with the data of a signal.
 */
Signal *
sig_fft_nr (Signal *in,
	    int    way)
{
  Signal *out;
  int    i, size;
  complex *data, *in_data;

#ifdef SMURF_DEV
  assert (in != 0);
  assert (in->dataY != 0);
  assert (way == REVERSE || way == DIRECT);
#endif

  size = in->n;

#ifdef SMURF_DEV
  assert (is_power_of_2_ (size));
#endif

  if(way == REVERSE)
    {
      switch (in->type)
	{
	case FOUR_NR :
	  
	  in_data = (complex *) in->dataY;
	  out = sig_new (REALY, 0, size-1);
	  data = (complex *) malloc (sizeof (complex)*size);
	  
	  for(i = 0; i < size; i++)
	    data[i] = in_data[i];

	  FourNR ((real *)(data)-1, size, REVERSE);

	  for (i = 0; i < size; i++)
	    out->dataY[i] = data[i].real/size;
	  free (data);
	  break;
	}
    }
  else
    {
      switch (in->type)
	{
	case REALY :
	  
	  out = sig_new (FOUR_NR, 0, size-1);
	  data = (complex *) out->dataY;
	  
	  for(i = in->first; i < 0; i++)
	    {
	      data[i+size].real = in->dataY[i];
	      data[i+size].imag = 0.0;
	    }
	  for(i = 0; i <= in->last; i++)
	    {
	      data[i].real = in->dataY[i];
	      data[i].imag = 0.0;
	    }
	  
	  FourNR ((real *)(data)-1, size, DIRECT);
	  break;
	case CPLX:

	  out = sig_new (FOUR_NR, 0, size-1);
	  data = (complex *) out->dataY;
	  
	  for(i = in->first; i < 0; i++)
	    {
	      data[i+size].real = in->dataY[2*i];
	      data[i+size].imag = in->dataY[2*i+1];
	    }
	  for(i = 0; i <= in->last; i++)
	    {
	      data[i].real = in->dataY[2*i];
	      data[i].imag = in->dataY[2*i+1];
	    }
	  
	  FourNR ((real *)(data)-1, size, DIRECT);
	  break;
	}
    }
  
  return out;
}

/*
 *  Call the strandh fft algorithm with the data of a signal.
 */
/* Rajouter la precondition sur le type du signal sous forme d'assert */
Signal *
sig_gfft (Signal *in,
	  int way,
	  int flag)
{
  Signal *out;
  int    size;

  size = in->n;

  /* Virer cette condition au niveau superieur. Mettre un assert a la place. */
  if (!is_power_of_2_ (size))
    ERROR ("The size of the signal must be a power of 2.\n");

  if (way == REVERSE)
    {
      switch (in->type)
	{
	case CPLX:
	case FOUR_NR:
	case REALY:
	  if (flag == REAL_CPLX)
	    {
	      out = sig_new (REALY, 0 , size*2 - 1);
	      gifft_real ((complex *) in->dataY, out->dataY, 2*size);
	    }
	  else
	    {
	      out = sig_new (CPLX, 0 , size - 1);
	      gifft_cplx ((complex *) in->dataY,
		     (complex *) out->dataY, size);
	    }
	  break;
	}
    }
  else
    {
      switch (in->type)
	{
	case CPLX:
	case FOUR_NR:
	  out = sig_new (FOUR_NR, 0, size - 1);
	  gfft_cplx ((complex *)in->dataY, (complex *)(out->dataY), size);
	  break;

	case REALY:
	  out = sig_new (FOUR_NR, 0, size/2 - 1);
	  gfft_real (in->dataY, (complex *)(out->dataY), size);
	  out->dx = M_PI*2/(in->dx *in->size);
	  break;
	}
    }
  return out;
}

/*
 *  Call the split radix fft algorithm with the data of a signal.
 */
/* Rajouter la precondition sur le type du signal sous forme d'assert */
Signal *
sig_srfft (Signal *in,
	  int way)
{
  Signal *out;
  int    size;

  assert (in);
  assert (way == DIRECT || way == REVERSE);
  assert (is_power_of_2_ (in->n));

  size = in->n;

  if (way == REVERSE)
    {
      switch (in->type)
	{
	case FOUR_NR:
	  out = sig_new (REALY, 0 , size*2 - 1);
	  gifft_real ((complex *) in->dataY, out->dataY, 2*size);
	  break;
	}
    }
  else
    {
      switch (in->type)
	{
	case FOUR_NR:
	  out = sig_new (FOUR_NR, 0, size - 1);
	  gfft_cplx ((complex *)in->dataY, (complex *)(out->dataY), size);
	  break;

	case REALY:
	  out = sig_new (FOUR_NR, 0, size/2 - 1);
	  gfft_real (in->dataY, (complex *)(out->dataY), size);
	  out->dx = M_PI*2/(in->dx *in->size);
	  break;
	}
    }
  return out;
}


/********************** encore des fft...... *******************/

void mul_hermitian(float *a, float *b, int n);
void scramble_real(float *x, int n);
void fft_real_to_hermitian(float *z, int n);
void fftinv_hermitian_to_real(float *z, int n);
/*#define TWOPI (float)(2*3.14159265358979323846264338327950)*/

/*
 */
/* Rajouter la precondition sur le type du signal sous forme d'assert */
Signal *
sig_gfft2 (Signal *in,
	  int way,
	  int flag)
{
  Signal *out;
  int    size;
  int i;

  size = in->n;

  /* Virer cette condition au niveau superieur. Mettre un assert a la place. */
  if (!is_power_of_2_ (size))
    ERROR ("The size of the signal must be a power of 2.\n");

  if (way == REVERSE)
    {
      /* ("not implemented yet...\n");*/
      switch (in->type)
	{
	case CPLX:
	case FOUR_NR:
	  /*	  if (flag == REAL_CPLX)
	    {
	      out = sig_new (REALY, 0 , size*2 - 1);
	      gifft_real ((complex *) in->dataY, out->dataY, 2*size);
	    }
	  else
	    {
	      out = sig_new (CPLX, 0 , size - 1);
	      gifft_cplx ((complex *) in->dataY,
		     (complex *) out->dataY, size);
	    }*/
	  break;
	}
    }
  else
    {
      switch (in->type)
	{
	case CPLX:
	case FOUR_NR:
	  /*	  out = sig_new (FOUR_NR, 0, size - 1);
	  gfft_cplx ((complex *)in->dataY, (complex *)(out->dataY), size);*/
	  break;

	case REALY:
	  /*	  out = sig_new (FOUR_NR, 0, size/2 - 1);
	  gfft_real (in->dataY, (complex *)(out->dataY), size);
	  out->dx = M_PI*2/(in->dx *in->size);*/
	  out = sig_new (FOUR_NR, 0, size/2 - 1);
	  for (i = 0; i < size; i++)
	      out->dataY[i] = in->dataY[i];
	  fft_real_to_hermitian ((real *)out->dataY, size);
	  /*	  out->dx = M_PI*2/(in->dx *in->size);*/
	  break;
	}
    }
  return out;
}


/********************************************************************/
/* Begin FILE     : paqfftr.c                                       */
/* AUTHOR         : HENRIQUE MALVAR                                 */
/* CREATION DATE  : August 1991                                     */
/********************************************************************/

/*
DESCRIPTION
   1d and 2d complex fourier transforms
*/

/* Each function will be listed as follows */
/*****************************/
/*
B_COMMENT
  Name                            :
  Summary                         :
  Parameters in Entry             :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  |      |             |                                           |
  ------------------------------------------------------------------
  Parameters in Exit              :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  |      |             |                                           |
  ------------------------------------------------------------------
  Return                          :
  -----------------------------------------------------------
  | type        | Meaning                                   |
  |-------------|-------------------------------------------|
  |             |                                           |
  -----------------------------------------------------------
E_COMMENT
*/
/*****************************/


/*****************************************************/
/* Include Files                                     */
/*****************************************************/

/********************************************************************/
/*****************************************************/
#include <stdio.h>
#include <math.h>
/*#include "paqinclude.h"*/
#define MAXLOGM  25
#define TWOPI  6.28318530717958647692
#define SQHALF   0.707106781186547524401

#define REAL float
/*#include "paqbasics.h"*/
/*****************************************************/
/********************************************************************/

/*****************************************************/
/* Basic Functions                                   */
/*****************************************************/

/********************************************************************/
/*****************************************************/

/**************************************/
/* Function error_exit                */
/**************************************/

static void error_exit()
{
  exit(1);
}
/*****************************************************/

/**************************************/
/* Function BR_permute                */
/**************************************/

/**********************************************************
* Data unshuffling according to bit-reversed indexing.
*
* Bit reversal is done using Evans' algorithm. (Ref: DMW Evans,
* "An Improved Digit-Reversal Permutation Algorithm",
*  IEEE Trans. ASSP, Aug 1987, pp. 1120-25.
*
***********************************************************/

static int brseed[256]; /* Evans' seed table */
static int brsflg;   /* flag for table building */

#ifdef NOPROTO
void BR_permute(x, logm)
     double *x;
     int logm;
#else
     void BR_permute(double *x,int logm)
#endif
{
  register int lg2, n;
  int i,j,imax;
  int off,fj, gno, *brp;
  double tmp, *xp, *xq;

  lg2 = logm >> 1;
  n = 1 << lg2;

  if (logm & 1) lg2++;

  /* create seed table if not yet built */
  if (brsflg != logm) {
    brsflg = logm;
    brseed[0] = 0;
    brseed[1] = 1;
    for(j=2; j <= lg2; j++){
      imax =  1 << (j - 1);
      for (i=0; i < imax; i++){
	brseed[i] <<= 1;
	brseed[i + imax] = brseed[i] + 1;
      }
    }
  }

  /* unshuffling loop */
  for(off = 1; off < n; off++){
    fj = n*brseed[off]; i = off; j = fj;
    tmp = x[i]; x[i] = x[j]; x[j] = tmp;
    xp = &x[i];
    brp = &brseed[1];
    for(gno = 1; gno < brseed[off]; gno++){
      xp += n;
      j = fj + *brp++;
      xq = x + j ;
      tmp = *xp; *xp = *xq; *xq = tmp;
    }
  }


}
/*****************************************************/

/**************************************/
/* Function srrec                     */
/**************************************/

/**************************************************************
*                                                             *
* Recursive part of split radix FFT algorithm
*
***************************************************************/

#ifdef NOPROTO
void srrec(xr, xi, logm)
     double *xr, *xi;
     int logm;
#else
     void srrec(double *xr,double *xi,int logm)
#endif
{

  static int m, m2, m4, m8, nel, n;
  static double *xr1, *xr2, *xi1, *xi2;
  static double *cn, *spcn, *smcn, *c3n, *spc3n, *smc3n;
  static double tmp1, tmp2, ang, c, s;
  static double *tab[MAXLOGM];

  /* check range of logm */

  if ((logm < 0) || (logm > MAXLOGM)){
    fprintf( stdout, "Error: SRFFT logm = %d is out of bounds [%d %d]\n", logm, 0, MAXLOGM);
    error_exit();
  }

  /* compute trivial cases */
  if (logm < 3){
    if (logm == 2){
      xr2 = xr + 2;
      xi2= xi + 2;
      tmp1 = *xr + *xr2;
      *xr2 = *xr - *xr2;
      *xr = tmp1;
      tmp1 = *xi + *xi2;
      *xi2 = *xi - *xi2;
      *xi = tmp1;
      xr1 = xr + 1;
      xi1 = xi + 1;
      xr2++;
      xi2++;
      tmp1 = *xr1 + *xr2;
      *xr2 = *xr1 - *xr2;
      *xr1 = tmp1;
      tmp1 = *xi1 +  *xi2;
      *xi2 = *xi1 - *xi2;
      *xi1 = tmp1;
      xr2 = xr + 1;
      xi2 = xi + 1;
      tmp1 = *xr + *xr2;
      *xr2 = *xr - *xr2;
      *xr = tmp1;
      tmp1 = *xi + *xi2;
      *xi2 = *xi - *xi2;
      *xi = tmp1;
      xr1 = xr +2;
      xi1 = xi+2;
      xr2 = xr+3;
      xi2 = xi + 3;
      tmp1 = *xr1 + *xi2;
      tmp2 = *xi1 + *xr2;
      *xi1 = *xi1 - *xr2;
      *xr2 = *xr1 - *xi2;
      *xr1 = tmp1;
      *xi2 = tmp2;
      return;
    }
    else if (logm == 1){
      xr2 = xr + 1;
      xi2 = xi + 1;
      tmp1 = *xr + *xr2;
      *xr2 = *xr - *xr2;
      *xr = tmp1;
      tmp1 = *xi + *xi2;
      *xi2 = *xi - *xi2;
      *xi = tmp1;
      return;
    }
    else if (logm == 0) return;
  }

  /* compute a few constants */

  m = 1 << logm; m2 = m / 2; m4 = m2 / 2; m8 = m4 / 2;


  /* build tables of butterfly coefficients if necessary */
  if ((logm >=4) && (tab[logm-4] == NULL)){
    /* allocate memory for tables */
    nel = m4 - 2;
    if ((tab[logm-4] = (double *) calloc(6 * nel, sizeof(double)))
	== NULL){
      error_exit();
    }



    /* initialize pointers */
    cn = tab[logm-4]; spcn = cn + nel; smcn = spcn + nel;
    c3n = smcn + nel; spc3n = c3n + nel; smc3n = spc3n + nel;


    /* compute tables */
    for (n = 1; n < m4; n++){
      if (n == m8) continue;
      ang = n * TWOPI / m;
      c = cos(ang); s = sin(ang);
      *cn++ = c; *spcn++ = -(s + c); *smcn++ = s - c;
      ang = 3*n*TWOPI/m;
      c = cos(ang); s = sin(ang);
      *c3n++ = c; *spc3n++ = -(s + c); *smc3n++ = s - c;
    }
  }


  /* step 1 */
  xr1 = xr; xr2 = xr1 + m2;
  xi1 = xi; xi2 = xi1 + m2;

  for (n =0; n < m2; n++){
    tmp1 = *xr1 + *xr2;
    *xr2 = *xr1 - *xr2;
    *xr1 = tmp1;
    tmp2 = *xi1 + *xi2;
    *xi2 = *xi1 - *xi2;
    *xi1 = tmp2;
    xr1++; xr2++; xi1++; xi2++;
  } 

  /* Step 2 */
  xr1 = xr + m2;  xr2 = xr1 + m4;
  xi1 = xi + m2;  xi2 = xi1+ m4;
  for (n = 0; n < m4; n++){
    tmp1 = *xr1 + *xi2;
    tmp2 = *xi1 + *xr2;
    *xi1 = *xi1 - *xr2;
    *xr2 = *xr1 - *xi2;
    *xr1 = tmp1;
    *xi2 = tmp2;
    xr1++; xr2++; xi1++; xi2++;
  }

  /* Steps 3&4 */
  xr1 = xr + m2; xr2 = xr1 + m4;
  xi1 = xi + m2; xi2 = xi1 + m4;

  if (logm >= 4) {
    nel = m4 -2;
    cn = tab[logm-4]; spcn = cn + nel; smcn = spcn + nel;
    c3n = smcn + nel; spc3n = c3n + nel; smc3n = spc3n + nel;
  }

  xr1++; xr2++; xi1++; xi2++;

  for(n=1; n < m4; n++){
    if (n == m8){
      tmp1 = SQHALF*(*xr1 + *xi1);
      *xi1 = SQHALF*(*xi1 - *xr1);
      *xr1 = tmp1;
      tmp2 = SQHALF*(*xi2 - *xr2);
      *xi2 = -SQHALF*(*xr2 + *xi2);
      *xr2 = tmp2;
    }
    else {
      tmp2 = *cn++ *(*xr1 + *xi1);
      tmp1 = *spcn++ * *xr1 + tmp2;
      *xr1 = *smcn++ * *xi1 + tmp2;
      *xi1 = tmp1;
      tmp2 = *c3n++ * (*xr2 + *xi2);
      tmp1 = *spc3n++ * *xr2 + tmp2;
      *xr2 = *smc3n++ * *xi2 + tmp2;
      *xi2 = tmp1;
    }
    xr1++; xr2++; xi1++; xi2++;
  }


  /* call ssrec again with half DFT length */
  srrec(xr, xi, logm -1);
  /* call ssrec again twice with one quarter DFT length.
     Constants have to be recomputed because they are static! */

  m = 1 << logm; m2 = m/2;
  srrec(xr+m2, xi+m2, logm - 2);

  m = 1 << logm; m4 = 3*(m/4);
  srrec(xr+m4, xi+m4, logm - 2);
}
/*****************************************************/
/********************************************************************/

#ifdef COMMENT
/*****************************************************/
/* 1D Fourier Transforms                             */
/*****************************************************/
#endif /* COMMENT */

/********************************************************************/
/*****************************************************/

#ifdef COMMENT
/**************************************/
/* Function srfft                     */
/**************************************/
#endif /* COMMENT */

/*
#ifdef COMMENT
  Name                            : srfft
  Summary                         :
   Split Radix Fast Fourier Transform.

   This function computes the fast fourier transform
   of a complex vector whose real part is the array #1,
   whose imaginary part is #2 and whose size is 2^#3.
  Parameters in Entry             :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #1   | double *    | Pointer to the array of the real part     |
  | #2   | double *    | Pointer to the array of the imaginary part|
  | #3   | int         | Log_2(size)                               |
  ------------------------------------------------------------------
  Parameters in Exit              :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #1   | double *    | Pointer to the array of the real part     |
  | #2   | double *    | Pointer to the array of the imaginary part|
  ------------------------------------------------------------------
  Return                          :
  -----------------------------------------------------------
  | type        | Meaning                                   |
  |-------------|-------------------------------------------|
  | none        |                                           |
  -----------------------------------------------------------
#endif COMMENT
*/

#ifdef NOPROTO
void srfft(xr, xi, logm)
     double *xr, *xi;
     int logm;
#else
     void srfft(double *xr,double *xi,int logm)
#endif
{
  int i,m;
  double fac, *xrp, *xip;
  /* call recursive routine */
  srrec(xr, xi, logm);

  /* output array unshuffling using bit-reversed indices */
  if (logm > 1){
    BR_permute(xr, logm);
    BR_permute(xi, logm);
  }

  /* Normalization */
  m = 1 << logm;
  fac = 1.0/sqrt((double)m);
  xrp = xr; xip = xi;

  for (i=0; i < m; i++){
    *xrp++ *= fac;
    *xip++ *= fac;
  }
}
/*****************************************************/

#ifdef COMMENT
/**************************************/
/* Function srifft                    */
/**************************************/
#endif /* COMMENT */

/**************************************************************
*
*Inverse transform. Uses Duhamel's trick (Ref: P. Duhamel 
* et al. "On Computing the Inverse DFT", IEEE Trans. ASSP
* Feb. 1988, pp. 285-286.
*
****************************************************************/

/*
#ifdef COMMENT
  Name                            : srifft
  Summary                         :
   Split Radix Inverse Fast Fourier Transform.

   This function computes the inverse fast fourier transform
   of a complex vector whose real part is the array #1,
   whose imaginary part is #2 and whose size is 2^#3.
  Parameters in Entry             :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #1   | double *    | Pointer to the array of the real part     |
  | #2   | double *    | Pointer to the array of the imaginary part|
  | #3   | int         | Log_2(size)                               |
  ------------------------------------------------------------------
  Parameters in Exit              :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #1   | double *    | Pointer to the array of the real part     |
  | #2   | double *    | Pointer to the array of the imaginary part|
  ------------------------------------------------------------------
  Return                          :
  -----------------------------------------------------------
  | type        | Meaning                                   |
  |-------------|-------------------------------------------|
  | none        |                                           |
  -----------------------------------------------------------
#endif COMMENT
*/

#ifdef NOPROTO
void srifft(xr, xi, logm)
     double *xr, *xi;
     int logm;
#else
     void srifft(double *xr,double *xi,int logm)
#endif
{
#ifdef OLD
  int i,m;
  double fac, *xrp, *xip;
#endif

  /* call direct FFT, swapping real and imaginery addresses */
  srfft(xi, xr, logm);
  
#ifdef OLD
  /* Normalization */
  m = 1 << logm;
  fac = 1.0/sqrt((double)m);
  xrp = xr; xip = xi;

  for (i=0; i < m; i++){
    *xrp++ *= fac;
    *xip++ *= fac;
  }
#endif
}
/*****************************************************/
/********************************************************************/

#ifdef COMMENT
/*****************************************************/
/* 2D Fourier Transforms                             */
/*****************************************************/
#endif /* COMMENT */

/********************************************************************/
/*****************************************************/

#ifdef COMMENT
/**************************************/
/* Function srfft2d                   */
/**************************************/
#endif /* COMMENT */

/*
#ifdef COMMENT
  Name                            : srfft2d
  Summary                         :
   2D Split Radix Fast Fourier Transform.

   This function computes the 2D fast fourier transform
   of a complex vector whose real part is the array #1,
   whose imaginary part is #2 and whose size is 2^#3 x 2^#4.
  Parameters in Entry             :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #1   | REAL *      | Pointer to the array of the real part     |
  | #2   | REAL *      | Pointer to the array of the imaginary part|
  | #3   | int         | Log_2(x_size)                             |
  | #4   | int         | Log_2(y_size)                             |
  ------------------------------------------------------------------
  Parameters in Exit              :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #1   | REAL *      | Pointer to the array of the real part     |
  | #2   | REAL *      | Pointer to the array of the imaginary part|
  ------------------------------------------------------------------
  Return                          :
  -----------------------------------------------------------
  | type        | Meaning                                   |
  |-------------|-------------------------------------------|
  | none        |                                           |
  -----------------------------------------------------------
#endif COMMENT
*/

#ifdef NOPROTO
void srfft2d(Fr,Fi,log2N1,log2N2)
     REAL *Fr,*Fi;
     int log2N1,log2N2;
#else
     void srfft2d(REAL *Fr,REAL *Fi,int log2N1,int log2N2)
#endif

     /*
//   Fr     array that contains the result of the transform
//   Fi     array that contains the result of the transform
//   N1     size of the array_x, it must be a power of 2
//   N2     size of the array_y, it must be a power of 2
*/
/********************************/
/*  Fast 2D-fourier transforms  */
/*  using frequential algorithm */
/********************************/
{
  int i,j;
  int N1,N2;
  REAL *tab1r,*tab1i;

  N1 = (1 << log2N1);
  N2 = (1 << log2N2);

  tab1r = (REAL *)malloc(N1*N2*sizeof(REAL));
  tab1i = (REAL *)malloc(N1*N2*sizeof(REAL));

  for(j=0;j<N2;j++)
    srfft((double *) &Fr[j*N1],(double *) &Fi[j*N1],log2N1);

  for(i=0;i<N1;i++)
    for(j=0;j<N2;j++)
      {
	tab1r[j+i*N2]=Fr[i+j*N1];
	tab1i[j+i*N2]=Fi[i+j*N1];
      }

  for(i=0;i<N1;i++)
    srfft((double *) &tab1r[i*N2],(double *) &tab1i[i*N2],log2N2);

  for(i=0;i<N1;i++)
    for(j=0;j<N2;j++)
      {
	Fr[i+j*N1]=tab1r[j+i*N2];
	Fi[i+j*N1]=tab1i[j+i*N2];
      }

  free(tab1r);
  free(tab1i);
}
/*****************************************************/

#ifdef COMMENT
/**************************************/
/* Function srifft2d                  */
/**************************************/
#endif /* COMMENT */

/*
#ifdef COMMENT
  Name                            : srifft2d
  Summary                         :
   2D Split Radix Inverse Fast Fourier Transform.

   This function computes the 2D inverse fast fourier transform
   of a complex vector whose real part is the array #1,
   whose imaginary part is #2 and whose size is 2^#3 x 2^#4.
  Parameters in Entry             :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #1   | REAL *      | Pointer to the array of the real part     |
  | #2   | REAL *      | Pointer to the array of the imaginary part|
  | #3   | int         | Log_2(x_size)                             |
  | #4   | int         | Log_2(y_size)                             |
  ------------------------------------------------------------------
  Parameters in Exit              :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #1   | REAL *      | Pointer to the array of the real part     |
  | #2   | REAL *      | Pointer to the array of the imaginary part|
  ------------------------------------------------------------------
  Return                          :
  -----------------------------------------------------------
  | type        | Meaning                                   |
  |-------------|-------------------------------------------|
  | none        |                                           |
  -----------------------------------------------------------
#endif COMMENT
*/

#ifdef NOPROTO
void srifft2d(Fr,Fi,log2N1,log2N2)
     REAL *Fr,*Fi;
     int log2N1,log2N2;
#else
     void srifft2d(REAL *Fr,REAL *Fi,int log2N1,int log2N2)
#endif

     /*
//   Fr     array that contains the result of the transform
//   Fi     array that contains the result of the transform
//   N1     size of the array_x, it must be a power of 2
//   N2     size of the array_y, it must be a power of 2
*/
/********************************/
/*  Fast 2D-fourier transforms  */
/*  using frequential algorithm */
/********************************/
{
  int i,j,N1,N2;
  REAL *tab1r,*tab1i;

  N1 = (1 << log2N1);
  N2 = (1 << log2N2);

  tab1r = (REAL *)malloc(N1*N2*sizeof(REAL));
  tab1i = (REAL *)malloc(N1*N2*sizeof(REAL));

  for(j=0;j<N2;j++)
    srifft((double *) &Fr[j*N1],(double *) &Fi[j*N1],log2N1);

  for(i=0;i<N1;i++)
    for(j=0;j<N2;j++)
      {
	tab1r[j+i*N2]=Fr[i+j*N1];
	tab1i[j+i*N2]=Fi[i+j*N1];
      }

  for(i=0;i<N1;i++)
    srifft((double *) &tab1r[i*N2],(double *) &tab1i[i*N2],log2N2);

  for(i=0;i<N1;i++)
    for(j=0;j<N2;j++)
      {
	Fr[i+j*N1]=tab1r[j+i*N2];
	Fi[i+j*N1]=tab1i[j+i*N2];
      }

  free(tab1r);
  free(tab1i);
}
/*****************************************************/
/********************************************************************/

#ifdef COMMENT
/*****************************************************/
/* Power Spectrum functions                          */
/*****************************************************/
#endif /* COMMENT */

/********************************************************************/
/*****************************************************/

#ifdef COMMENT
/**************************************/
/* Function crown                     */
/**************************************/
#endif /* COMMENT */

/*
#ifdef COMMENT
  Name                            : crown 
  Summary                         :
   This function computes the crown number of 
   an element whose x- and y- indexes are #1 and #2,
   in a 2D field of size #3x#4.
  Parameters in Entry             :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #1   | int         | x-index                                   |
  | #2   | int         | y-index                                   |
  | #3   | int         | Number of x elements                      |
  | #4   | int         | Number of y elements                      |
  ------------------------------------------------------------------
  Parameters in Exit              :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | none |             |                                           |
  ------------------------------------------------------------------
  Return                          :
  -----------------------------------------------------------
  | type        | Meaning                                   |
  |-------------|-------------------------------------------|
  | int         | Crown number                              |
  -----------------------------------------------------------
#endif COMMENT
*/

#ifdef NOPROTO
int crown(ind1,ind2,N1,N2)
     int ind1,ind2,N1,N2;
#else
     int crown(int ind1,int ind2,int N1,int N2)
#endif
{
  int N12,N22;
  int rind1,rind2;
  int valcrown;
  REAL val;

  N12 = N1/2; 
  N22 = N2/2; 

  if (ind1 < N12)
    rind1 = ind1;
  else
    rind1 = ind1-N1;

  if (ind2 < N22)
    rind2 = ind2;
  else
    rind2 = ind2-N2;

  val = sqrt((REAL)(rind1*rind1+rind2*rind2));

  if ((val-floor(val)) < 0.5)
    valcrown = (int) floor(val);
  else
    valcrown = (int) ceil(val);

  return valcrown;
}
/*****************************************************/

#ifdef COMMENT
/**************************************/
/* Function powspec2d                 */
/**************************************/
#endif /* COMMENT */

/*
#ifdef COMMENT
  Name                            : powspec2d
  Summary                         :
   This function computes the power spectrum of a 2D field
   of size #3x#4.
   This field is represented by 2 arrays of REAL, #1 for 
   its real part and #2 for its imaginary part.  
  Parameters in Entry             :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #1   | REAL *      | Pointer to the real part                  |
  | #2   | REAL *      | Pointer to the imaginary part             |
  | #4   | int         | x-Size                                    |
  | #5   | int         | y-Size                                    |
  ------------------------------------------------------------------
  Parameters in Exit              :
  ------------------------------------------------------------------
  | Num  | type        | Meaning                                   |
  |------|-------------|-------------------------------------------|
  | #3   | REAL *      | Pointer to the power spectrum array       |
  ------------------------------------------------------------------
  Return                          :
  -----------------------------------------------------------
  | type        | Meaning                                   |
  |-------------|-------------------------------------------|
  | none        |                                           |
  -----------------------------------------------------------
#endif COMMENT
*/

#ifdef NOPROTO
void powspec2d(Fr,Fi,PS2d,N1,N2)
     REAL *Fr,*Fi;
     REAL *PS2d; /* Must have been preallocated by min(N1,N2)/2+1 points */
     int N1,N2;
#else
     void powspec2d(REAL *Fr,REAL *Fi,REAL *PS2d,int N1,int N2)
#endif
{
  int i,j,n,a,b;
  int ind,ind2,*number;
  REAL val;

  n=min(N1,N2)/2;

#ifdef BEFORE
  n=(int)sqrt((REAL)(N1*N1)/4.0+(REAL)(N2*N2)/4.0)+1;
#endif
 
  number = (int *) malloc((n+1)*sizeof(int));

  for(i=0;i<=n;i++)
    {
      PS2d[i]=0;
      number[i]=0;
    }

  for(i=0;i<N1/2;i++)
    for(j=0;j<N2/2;j++)
      {
	a=i;
	b=j;
	ind2=i+j*N1;
#ifdef CARRE
	ind = max(abs(a),abs(b));
#else
#ifdef OLD
	ind = (int) sqrt((REAL)(a*a+b*b)); 
#else
	val = sqrt((REAL)(a*a+b*b));

	if ((val-floor(val)) < 0.5)
	  ind = (int) floor(val);
	else
	  ind = (int) ceil(val);
#endif
#endif
	if (ind > n)
	  ind = n;
	PS2d[ind]+=Fr[ind2]*Fr[ind2]+Fi[ind2]*Fi[ind2];
	number[ind]++;
      }

  for(i=0;i<N1/2;i++)
    for(j=N2/2;j<N2;j++)
      {
	a=i;
	b=j-N2;
	ind2=i+j*N1;
#ifdef CARRE
	ind = max(abs(a),abs(b));
#else
#ifdef OLD
	ind = (int) sqrt((REAL)(a*a+b*b)); 
#else
	val = sqrt((REAL)(a*a+b*b));

	if ((val-floor(val)) < 0.5)
	  ind = (int) floor(val);
	else
	  ind = (int) ceil(val);
#endif
#endif
	if (ind > n)
	  ind = n;
	PS2d[ind]+=Fr[ind2]*Fr[ind2]+Fi[ind2]*Fi[ind2];
	number[ind]++;
      }

  for(i=N1/2;i<N1;i++)
    for(j=N2/2;j<N2;j++)
      {
	a=i-N1;
	b=j-N2;
	ind2=i+j*N1;
#ifdef CARRE
	ind = max(abs(a),abs(b));
#else
#ifdef OLD
	ind = (int) sqrt((REAL)(a*a+b*b)); 
#else
	val = sqrt((REAL)(a*a+b*b));

	if ((val-floor(val)) < 0.5)
	  ind = (int) floor(val);
	else
	  ind = (int) ceil(val);
#endif
#endif
	if (ind > n)
	  ind = n;
	PS2d[ind]+=Fr[ind2]*Fr[ind2]+Fi[ind2]*Fi[ind2];
	number[ind]++;
      }

  for(i=N1/2;i<N1;i++)
    for(j=0;j<N2/2;j++)
      {
	a=i-N1;
	b=j;
	ind2=i+j*N1;
#ifdef CARRE
	ind = max(abs(a),abs(b));
#else
#ifdef OLD
	ind = (int) sqrt((REAL)(a*a+b*b)); 
#else
	val = sqrt((REAL)(a*a+b*b));

	if ((val-floor(val)) < 0.5)
	  ind = (int) floor(val);
	else
	  ind = (int) ceil(val);
#endif
#endif
	if (ind > n)
	  ind = n;
	PS2d[ind]+=Fr[ind2]*Fr[ind2]+Fi[ind2]*Fi[ind2];
	number[ind]++;
      }

  for(i=0;i<=n;i++)
    {
      if (number[i] != 0)
	PS2d[i] = (REAL)(i)*PS2d[i]/(REAL)number[i];
    }

  free(number);
}
/*****************************************************/
/********************************************************************/

/********************************************************************/
/* End FILE       : paqfftr.c                                       */
/********************************************************************/


/*****************************************
 *
 * Generalites sur la transformee de Fourier d'un signal echantillonne 
 *
 *****************************************/
 
/*
 * La transformation de fourier Ts(f) d'un signal s(t) reel 
 * peut etre definie de la facon suivante:
 *
 *    Ts(f) = integrale s(t) exp(-2*PI*f*t) dt
 *
 * Le signal sur ordinateur a ete echantillonne a la frequence F et
correspond donc 
 * aux echantillons s(n/F). 
 * Le signal ayant ete echantillonne a la frequence F, on peut montrer
 * qu'il ne contient aucune frequence superieure a F/2. Sa transformee
 * de Fourier Ts(f) est donc nulle pour |f| > F/2. 
 * De plus le signal etant reel, sa transformee de fourier verifie la
relation :
 *
 *          Ts(f) = Ts*(-f)
 *
 * 
 * 
 * La fonction C fournie ici permet de calculer un echantillonnage de
 * Ts(f) pour f appartenant a [0,F/2] a partir des echantillons s(n/F).
 * Si le signal numerise original contient N points : 
 *
 *             s(0) , s(1/F) , s(2/F) ,... , s((N-1)/F))
 *
 * le resultat de la fonction "TransformeFourier" donne deux tableaux de
 * taille (N/2)+1 contenant respectivement les valeurs
 *
 *          Re(Ts(0)), Re(Ts(F/N)), Re(Ts(2F/N), .... Re(Ts(F/2)) et
 *          Im(Ts(0)), Im(Ts(F/N)), Im(Ts(2F/N), .... Im(Ts(F/2)).
 *             
 * Cela correspond a un echantillonnage de Ts sur [0,F/2] avec un pas de
F/N.
 */     

    

/***************************************************************************
*********
 *
 *   void TransformeFourier (double *tableau, int N, int logN,double
*re,double *im)
 *
 *
 *   C'est la fonction C a appeler pour effectuer une transformee de
Fourier
 *   d'un signal echantillonne de taille N (ou N est FORCEMENT une
puissance de 2).
 *   C'est la seule fonction de ce fichier que vous aurez a appeler.
 *
 *
 *   Les parametres sont
 *
 *       - 'tableau' : tableau de flottant de taille 'N' representant
 *                     le signal echantillonne tableau[n] = s(n/F) (pour
0<= n <= N-1).
 *       - 'N'       : Taille du signal echantillonne represente par
'tableau'.
 *                     N est FORCEMENT une puissance de 2.
 *       - 'logN'    : nombre entier tel que N = 2^logN
 *       - 're'      : un tableau de taille (N/2)+1 qui aura ete alloue
avant
 *                     l'appel de la fonction et qui permettra de renvoyer
 *                     les valeurs :  re[i] = Re(Ts(iF/N)) (pour 0<= i <=
N/2).
 *       - 'im'      : un tableau de taille (N/2)+1 qui aura ete alloue
avant
 *                     l'appel de la fonction et qui permettra de renvoyer
 *                     les valeurs :  im[i] = Im(Ts(iF/N)) (pour 0<= i 
<=
N/2).
 *
 *   ATTENTION
 *   ATTENTION
 *   ATTENTION : le contenu de la variable 'tableau' est change a la sortie
 *   ATTENTION   de la routine
 *   ATTENTION
 *   ATTENTION
 *

************************************************************************************/
 
static void rsrec(double *x, int logN);
/*static void BR_permute(double *x, int logN);*/


void
TransformeFourier (double *tableau,
		   int    N,
		   int    logN,
		   double *re,
		   double *im)
{
  int i;
  
  rsrec(tableau,logN);
  if (logN > 1) BR_permute(tableau,logN);

  for (i=0;i<N/2+1;i++) re[i] = tableau[i];
  im[0] = 0;
  im[N/2] = 0;
  for (i=1;i<N/2;i++) im[i] = -tableau[N-i];

}




/**************************************************************************************

**************************************************************************************

**************************************************************************************

**************************************************************************************
 *
 *
 *     VOUS N'AVEZ AUCUN INTERET A COMPRENDRE LE CODE SE SITUANT SOUS CE
COMMENTAIRE
 *
 *
 *

**************************************************************************************

**************************************************************************************

**************************************************************************************

**************************************************************************************/



/* Des variables globales */
#define  TWOPI       6.28318530717958647692
#define  SQHALF      0.707106781186547524401

/*
 * Valeur maximale que peut prendre la variable 'logN'
 * (peut etre changee...mais 32 devrait etre suffisant!)
 */


static   double    *tab[MAXLOGM];             /* tables of butterfly angles
*/
/*static   double    *tab1[MAXLOGM];            */



/*
 *  Fonction permettant la tabulation de la table de cosinus 'tab'
 */

static void BuildTable(int logm)
{
  static   int      m, m2, m4, m8, nel, n;
  static   double    *cn, *spcn, *smcn;
  static   double    ang, c, s;
  
  /* Compute a few constants */
  m = 1 << logm; m2 = m / 2; m4 = m2 / 2; m8 = m4 /2;
  
  /* Allocate memory for tables */
  nel = m4 - 2;
  if ((tab[logm-4] = (double *) calloc(3 * nel, sizeof(double))) == NULL) {
    /*("Error :  pas assez de memoire pour les tables de cosinus!\n");*/
    exit(0);
  }
    
  /* Initialize pointers */
  cn  = tab[logm-4]; spcn  = cn + nel;  smcn  = spcn + nel;
  
  /* Compute tables */
  for (n = 1; n < m4; n++) {
    if (n == m8) continue;
    ang = n * TWOPI / m;
    c = cos(ang); s = sin(ang);
    *cn++ = c; *spcn++ = - (s + c); *smcn++ = s - c;
  }
}

/* 
 * La routine principale permettant de calculer une transformee de Fourier
 */
 
static void rsrec(double *x,int logm)
{
  static   int      m, m2, m4, m8, nel, n;
  static   double    *xr1, *xr2, *xi1;
  static   double    *cn, *spcn, *smcn;
  static   double    tmp1, tmp2;
  
  /* Check range of logm */
  if ((logm < 0) || (logm > MAXLOGM)) {
    /* ("Error : rsrec : le log de la taille du signal (%d) doit etre compris dans [%d, %d]\n",logm, 0, MAXLOGM);*/
    exit(0);
  }
  
  /* Compute trivial cases */
  if (logm < 2) {
    if (logm == 1) {   /* length m = 2 */
      xr2  = x + 1;
      tmp1 = *x + *xr2;
      *xr2 = *x - *xr2;
      *x   = tmp1;
      return;
    }
    else if (logm == 0) return;   /* length m = 1 */
  }
  
  /* Compute a few constants */
  m = 1 << logm; m2 = m / 2; m4 = m2 / 2; m8 = m4 /2;
  
  /* Build tables of butterfly coefficients, if necessary */
  if ((logm >= 4) && (tab[logm-4] == NULL))
    BuildTable(logm);
  
  /* Step 1 */
  xr1 = x; xr2 = xr1 + m2;
  for (n = 0; n < m2; n++) {
    tmp1 = *xr1 + *xr2;
    *xr2 = *xr1 - *xr2;
    *xr1 = tmp1;
    xr1++; xr2++;
  }
  
  /* Step 2 */
  xr1 = x + m2 + m4;
  for (n = 0; n < m4; n++) {
    *xr1 = - *xr1;
    xr1++;
  }
  
  /* Step 3: multiplication by W_M^n */
  xr1 = x + m2; xi1 = xr1 + m4;
  if (logm >= 4) {
    nel = m4 - 2;
    cn  = tab[logm-4]; spcn  = cn + nel;  smcn  = spcn + nel;
  }
  xr1++; xi1++;
  for (n = 1; n < m4; n++) {
    if (n == m8) {
      tmp1 =  SQHALF * (*xr1 + *xi1);
      *xi1 =  SQHALF * (*xi1 - *xr1);
      *xr1 =  tmp1;
    } else {
      tmp2 = *cn++ * (*xr1 + *xi1);
      tmp1 = *spcn++ * *xr1 + tmp2;
      *xr1 = *smcn++ * *xi1 + tmp2;
      *xi1 = tmp1;
    }
    xr1++; xi1++;
  }
  
  /* Call rsrec again with half DFT length */
  rsrec(x, logm-1);
  
  /* Step 4: Call complex DFT routine, with quarter DFT length.
     Constants have to be recomputed, because they are static! */
  m = 1 << logm; m2 = m / 2; m4 = 3 * (m / 4);
  srrec(x + m2, x + m4, logm-2);
  
  /* Step 5: sign change & data reordering */
  m = 1 << logm; m2 = m / 2; m4 = m2 / 2; m8 = m4 / 2;
  xr1 = x + m2 + m4;
  xr2 = x + m - 1;
  for (n = 0; n < m8; n++) {
    tmp1   =    *xr1;
    *xr1++ =  - *xr2;
    *xr2-- =  - tmp1;
  }
  xr1 = x + m2 + 1;
  xr2 = x + m - 2;
  for (n = 0; n < m8; n++) {
    tmp1   =    *xr1;
    *xr1++ =  - *xr2;
    *xr2-- =    tmp1;
    xr1++;
    xr2--;
  }
  if (logm == 2) x[3] = -x[3];
}

/************************** encore des fft... **********************/
void mul_hermitian(float *a, float *b, int n)
{
	int k, half = n>>1;
	register float c, d, e, f;
	
	b[0] *= a[0];
	b[half] *= a[half];
	for(k=1;k<half;k++) {
		c = a[k]; 
		d = b[k]; 
		e = a[n-k]; 
		f = b[n-k];
		b[n-k] = c*f + d*e;
		b[k] = c*d - e*f;
	}
}

void scramble_real(float *x, int n)
{	
	register int i,j,k;
	float tmp;
	
	for(i=0,j=0;i<n-1;i++) {
		if(i<j) {
			tmp = x[j];
			x[j]=x[i];
			x[i]=tmp;
	  	}
		k = n/2;
		while(k<=j) {
			j -= k;
			k >>= 1;
		}
  		j += k;
 	}
}

void fft_real_to_hermitian(float *z, int n)
{	float *x, e, a, a3, sqrthalf = 1/(float)sqrt(2.0);
	float cc1, ss1, cc3, ss3;
	int nn = n>>1, is, id, i0, i1, i2, i3, i4, i5, i6, i7, i8;
	float t1, t2, t3, t4, t5, t6;
	int n2, n4, n8, i, j;
	
	scramble_real(z, n);
	x = z-1;  
	is = 1;
	id = 4;
	do{
		for(i0=is;i0<=n;i0+=id) {
			i1 = i0+1;
			e = x[i0];
			x[i0] = e + x[i1];
			x[i1] = e - x[i1];
		}
		is = (id<<1)-1;
		id <<= 2;
	} while(is<n);
	n2 = 2;
	while(nn>>=1) {
		n2 <<= 1;
		n4 = n2>>2;
		n8 = n2>>3;
		e = TWOPI/n2;
		is = 0;
		id = n2<<1;
		do {
			for(i=is;i<n;i+=id) {
				i1 = i+1;
				i2 = i1 + n4;
				i3 = i2 + n4;
				i4 = i3 + n4;
				t1 = x[i4]+x[i3];
				x[i4] -= x[i3];
				x[i3] = x[i1] - t1;
				x[i1] += t1;
				if(n4==1) continue;
				i1 += n8;
				i2 += n8;
				i3 += n8;
				i4 += n8;
				t1 = (x[i3]+x[i4])*sqrthalf;
				t2 = (x[i3]-x[i4])*sqrthalf;
				x[i4] = x[i2] - t1;
				x[i3] = -x[i2] - t1;
				x[i2] = x[i1] - t2;
				x[i1] += t2;
			}
			is = (id<<1) - n2;
			id <<= 2;
		} while(is<n);
		a = e;
		for(j=2;j<=n8;j++) {
			a3 = 3*a;
			cc1 = (float)cos(a);
			ss1 = (float)sin(a);
			cc3 = (float)cos(a3);
			ss3 = (float)sin(a3);
			a = e*j;
			is = 0;
			id = n2<<1;
			do {
				for(i=is;i<n;i+=id) {
					i1 = i+j;
					i2 = i1 + n4;
					i3 = i2 + n4;
					i4 = i3 + n4;
					i5 = i + n4 - j + 2;
					i6 = i5 + n4;
					i7 = i6 + n4;
					i8 = i7 + n4;
					t1 = x[i3]*cc1 + x[i7]*ss1;
					t2 = x[i7]*cc1 - x[i3]*ss1;
					t3 = x[i4]*cc3 + x[i8]*ss3;
					t4 = x[i8]*cc3 - x[i4]*ss3;
					t5 = t1 + t3;
					t6 = t2 + t4;
					t3 = t1 - t3;
					t4 = t2 - t4;
					t2 = x[i6] + t6;
					x[i3] = t6 - x[i6];
					x[i8] = t2;
					t2 = x[i2] - t3;
					x[i7] = -x[i2] - t3;
					x[i4] = t2;
					t1 = x[i1] + t5;
					x[i6] = x[i1] - t5;
					x[i1] = t1;
					t1 = x[i5] + t4;
					x[i5] -= t4;
					x[i2] = t1;
				}
				is = (id<<1) - n2;
				id <<= 2;
			} while(is<n);
		}
	}
}

void fftinv_hermitian_to_real(float *z, int n)
{	float *x, e, a, a3, sqrthalf = 1/(float)sqrt(2.0);
	float cc1, ss1, cc3, ss3;
	int nn = n>>1, is, id, i0, i1, i2, i3, i4, i5, i6, i7, i8;
	float t1, t2, t3, t4, t5;
	int n2, n4, n8, i, j;
	
	x = z-1; 
	n2 = n<<1;
	while(nn >>= 1) {
		is = 0;
		id = n2;
		n2 >>= 1;
		n4 = n2>>2;
		n8 = n4>>1;
		e = TWOPI/n2;
		do {
			for(i=is;i<n;i+=id) {
				i1 = i+1;
				i2 = i1 + n4;
				i3 = i2 + n4;
				i4 = i3 + n4;
				t1 = x[i1] - x[i3];
				x[i1] += x[i3];
				x[i2] += x[i2];
				x[i3] = t1 - 2.0*x[i4];
				x[i4] = t1 + 2.0*x[i4];
				if(n4==1) continue;
				i1 += n8;
				i2 += n8;
				i3 += n8;
				i4 += n8;
				t1 = (x[i2]-x[i1])*sqrthalf;
				t2 = (x[i4]+x[i3])*sqrthalf;
				x[i1] += x[i2];
				x[i2] = x[i4]-x[i3];
				x[i3] = -2.0*(t2+t1);
				x[i4] = 2.0*(t1-t2);
			}
			is = (id<<1) - n2;
			id <<= 2;
		} while(is<n-1);
		a = e;
		for(j=2;j<=n8;j++) {
			a3 = 3*a;
			cc1 = (float)cos(a);
			ss1 = (float)sin(a);
			cc3 = (float)cos(a3);
			ss3 = (float)sin(a3);
			a = e*j;
			is = 0;
			id = n2<<1;
			do {
				for(i=is;i<n;i+=id) {
					i1 = i+j;
					i2 = i1+n4;
					i3 = i2+n4;
					i4 = i3+n4;
					i5 = i+n4-j+2;
					i6 = i5+n4;
					i7 = i6+n4;
					i8 = i7+n4;
					t1 = x[i1] - x[i6];
					x[i1] += x[i6];
					t2 = x[i5] - x[i2];
					x[i5] += x[i2];
					t3 = x[i8] + x[i3];
					x[i6] = x[i8] - x[i3];
					t4 = x[i4] + x[i7];
					x[i2] = x[i4] - x[i7];
					t5 = t1 - t4;
					t1 += t4;
					t4 = t2 - t3;
					t2 += t3;
					x[i3] = t5*cc1 + t4*ss1;
					x[i7] = -t4*cc1 + t5*ss1;
					x[i4] = t1*cc3 - t2*ss3;
					x[i8] = t2*cc3 + t1*ss3;
				}
				is = (id<<1) - n2;
				id <<= 2;
			} while(is<n-1);
		}
	}
	is = 1;
	id = 4;
	do {
		for(i0=is;i0<=n;i0+=id){
			i1 = i0+1;
			e = x[i0];
			x[i0] = e + x[i1];
			x[i1] = e - x[i1];
		}
		is = (id<<1) - 1;
		id <<= 2;
	} while(is<n);
	scramble_real(z, n);
	e = 1/(float)n;
	for(i=0;i<n;i++) z[i] *= e;				
}


