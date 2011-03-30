/*
 * fft.c --
 *
 *   Copyright (c) 1999 Nicolas Decoster
 *   Copyright (c) 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *
 *   Copyright (c) 1999-2007 Pierre Kestener.
 *   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
 *   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
 *   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
 *
 */

#include "image_int.h"

/*#define DIRECT 1
#define REVERSE -1
#define max(a,b) (a>b?a:b)
*/
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

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

static void Shift_Buffer_3D(float *indata, int L, int M, int N)
{
  int i,j;
  
  for (i=L-1; i>=0; i--)
    for (j=M-1; j>=0; j--)
      memmove(indata+(j+i*M)*2*(N/2+1),indata+(j+i*M)*N,N*sizeof(float));
}

static void Shift_Inv_Buffer_3D(float *indata, int L, int M, int N)
{
  int i,j;
  
  for (i=0; i<L; i++)
    for (j=0; j<M; j++)
      memmove(indata+(j+i*M)*N,indata+(j+i*M)*2*(N/2+1),N*sizeof(float));
}


/*------------------------------------------------------------------------
  _Fourn_
  Transformee de Fourier en dimension n.
  Extrait de NUMERICAL RECIPES.
  ----------------------------------------------------------------------*/
void
_Fourn_ (real         *data,
	 unsigned long *nn,
	 int           ndim,
	 int           isign)
{
  int           idim;
  unsigned long i1, i2, i3, i2rev, i3rev, ip1, ip2, ip3, ifp1, ifp2;
  unsigned long ibit, k1, k2, n, nprev, nrem, ntot;
  real         tempi, tempr;
  double        theta, wi, wpi, wpr, wr, wtemp;

  ntot = 1;
  for (idim = 1; idim <= ndim; idim++)
    ntot *= nn[idim];
  nprev = 1;
  for (idim = ndim; idim >= 1; idim--) {
    n = nn[idim];
    nrem = ntot / (n * nprev);
    ip1 = nprev << 1;
    ip2 = ip1 * n;
    ip3 = ip2 * nrem;
    i2rev = 1;
    for (i2 = 1; i2 <= ip2; i2 += ip1) {
      if (i2 < i2rev)
	for (i1 = i2; i1 <= i2 + ip1 - 2; i1 += 2)
	  for (i3 = i1; i3 <= ip3; i3 += ip2) {
	    i3rev = i2rev + i3 - i2;
	    SWAP (data[i3], data[i3rev]);
	    SWAP (data[i3 + 1], data[i3rev + 1]);
	  }
      ibit = ip2 >> 1;
      while (ibit >= ip1 && i2rev > ibit) {
	i2rev -= ibit;
	ibit >>= 1;
      }
      i2rev += ibit;
    }
    ifp1 = ip1;
    while (ifp1 < ip2) {
      ifp2 = ifp1 << 1;
      theta = isign * 6.28318530717959 / (ifp2 / ip1);
      wtemp = sin (0.5 * theta);
      wpr = -2.0 * wtemp * wtemp;
      wpi = sin (theta);
      wr = 1.0;
      wi = 0.0;
      for (i3 = 1; i3 <= ifp1; i3 += ip1) {
	for (i1 = i3; i1 <= i3 + ip1 - 2; i1 += 2)
	  for (i2 = i1; i2 <= ip3; i2 += ifp2) {
	    k1 = i2;
	    k2 = k1 + ifp1;
	    tempr = (real) wr *data[k2] - (real) wi *data[k2 + 1];
	    tempi = (real) wr *data[k2 + 1] + (real) wi *data[k2];
	    data[k2] = data[k1] - tempr;
	    data[k2 + 1] = data[k1 + 1] - tempi;
	    data[k1] += tempr;
	    data[k1 + 1] += tempi;
	  }
	wr = (wtemp = wr) * wpr - wi * wpi + wr;
	wi = wi * wpr + wtemp * wpi + wi;
      }
      ifp1 = ifp2;
    }
    nprev *= n;
  }
}

/*------------------------------------------------------------------------
  next_power_of_2__

  Cette fonction renvoie la premiere puissance de 2 superieure a
  l'entier passe en parametre.
  ----------------------------------------------------------------------*/
int
next_power_of_2__(int i)
{
  int j;

  for (j=1;j<i;j*=2);
  return j;
}

/*------------------------------------------------------------------------
  ImaFftImageToFftCmd_
  
  En entree, un nom d'image source et un nom d'image destination.
  On transforme source dans un format particulier a l'algo fft
  de N.R.. On stocke le resultat dans l'image destination et on lance
  la fft la d'sus.
  ----------------------------------------------------------------------*/
Image *
im_direct_to_fourier (Image *srcImage,
		      int   dim)
{
  Image *dstImage;
  int   srcLx, srcLy;
  int   dstLx, dstLy, dstSize;
  int   dx, dy, x, y;
  
  srcLx    = srcImage->lx;
  srcLy    = srcImage->ly;
  dstLx    = next_power_of_2__(max(srcLx,dim));
  dstLy    = next_power_of_2__(max(srcLy,dim));
  dstSize  = 2 * dstLx * dstLy + 2;
  /* Pourquoi "+2" a la fin de dstSize ??
     Because zenight ??
     non... parce que les indices commencent a 1 dans
     le numerical recipes....
  */


  dstImage = im_new(srcLx, srcLy, dstSize, FOURIER);
  if (!dstImage)
/*    GenErrorMemoryAlloc(interp);*/
    return 0;

  /* on recopie srcImage dans dstImage avec un format adapte a _Fourn_ */      
  dx = (int)(dstLx - srcLx) / 2;
  dy = (int)(dstLy - srcLy) / 2;
  
  im_set_0 (dstImage);
  for (x = 0 ; x < srcLx ; x++)
    for (y = 0 ; y < srcLy ; y++)
      dstImage->data[2 * (dstLx * (y + dy) + x + dx) + 1]
	= srcImage->data[y * srcLx + x];
  
  return dstImage;
}

/*------------------------------------------------------------------------
  ImaFftImageToFft2Cmd_

  voir plus haut.
  ici, on prend deux images "PHYSICAL" pour la partie reelle et imaginaire
  et on fabrique une image de type FOURIER !!! (etonnant, non !?)
  ----------------------------------------------------------------------*/
Image *
im_direct_to_fourier2 (Image *srcImage, Image *src2Image,
		      int   dim)
{
  Image *dstImage;
  int   srcLx, srcLy;
  int   dstLx, dstLy, dstSize;
  int   dx, dy, x, y;
  
  srcLx    = srcImage->lx;
  srcLy    = srcImage->ly;
  dstLx    = next_power_of_2__(max(srcLx,dim));
  dstLy    = next_power_of_2__(max(srcLy,dim));
  dstSize  = 2 * dstLx * dstLy + 2;
  /* Pourquoi "+2" a la fin de dstSize ??
     Because zenight ??
     non... parce que les indices commencent a 1 dans
     le numerical recipes....
  */


  dstImage = im_new(srcLx, srcLy, dstSize, FOURIER);
  if (!dstImage)
/*    GenErrorMemoryAlloc(interp);*/
    return 0;

  /* on recopie srcImage dans dstImage avec un format adapte a _Fourn_ */      
  dx = (int)(dstLx - srcLx) / 2;
  dy = (int)(dstLy - srcLy) / 2;
  
  im_set_0 (dstImage);
  for (x = 0 ; x < srcLx ; x++)
    for (y = 0 ; y < srcLy ; y++) {
      dstImage->data[2 * (dstLx * (y + dy) + x + dx) + 1]
	= srcImage->data[y * srcLx + x];
      dstImage->data[2 * (dstLx * (y + dy) + x + dx) + 2]
	= src2Image->data[y * srcLx + x];
    }
  
  return dstImage;
}



/*------------------------------------------------------------------------
  ImaFftFftToImageCmd_

  Transforme l'image source au format utilisable par _Fourn_ en une image
  de type normal.
  (en fait, ca prend la partie reelle du resultat...
   pourquoi pas...)
  ----------------------------------------------------------------------*/
Image *
im_fourier_to_direct (Image *srcImage)
{
  Image *dstImage;
  int   srcLx, srcLy;
  int   dstLx, dstLy, dstSize;
  int   dx, dy, x, y;
  
  dstLx    = srcImage->lx;
  dstLy    = srcImage->ly;
  dstSize  = dstLx * dstLy;
  srcLx    = next_power_of_2__(dstLx);
  srcLy    = next_power_of_2__(dstLy);

  dstImage = im_new (dstLx, dstLy, dstSize,PHYSICAL);
  if (!dstImage)
/*    GenErrorMemoryAlloc(interp);  */
    return 0;
  
  /* on recopie srcImage dans dstImage */      
  dx = (int)(srcLx - dstLx) / 2;
  dy = (int)(srcLy - dstLy) / 2;

  for (x = 0 ; x < dstLx ; x++)
    for (y = 0 ; y < dstLy ; y++)
      dstImage->data[y * dstLx + x] 
	= srcImage->data[2 * (srcLx *(y + dy) + x + dx) + 1]
	/((real)(srcLx * srcLy));

  return dstImage;
}

void
im_gfft2d_real (real    *in,
		complex *out,
		int     lx,
		int     ly)
{
  int     x, y;
  real    *in_1d;
  complex *out_1d;
  complex *in_tmp;
  complex *out_tmp;
  real    *first, *last;

  out_tmp = (complex *) malloc (ly*sizeof (complex));
  in_tmp  = (complex *) malloc (ly*sizeof (complex));
  first  = (real *) malloc (ly*sizeof (real));
  last   = (real *) malloc (ly*sizeof (real));

  for (y = 0; y < ly; y++)
    {
      in_1d  = &in[y*lx];
      out_1d = &out[y*lx/2];
      gfft_real (in_1d, out_1d, lx);
    }

  for (y = 0; y < ly; y++)
    {
      first[y] = out[y*lx/2].real;
      last[y]  = out[y*lx/2].imag;
    }
  gfft_real (first, out_tmp, ly);
  for (y = 0; y < ly/2; y++)
    out[y*lx/2] = out_tmp[y];
  gfft_real (last, out_tmp, ly);
  for (y = ly/2; y < ly; y++)
    out[y*lx/2] = out_tmp[y-ly/2];

  for (x = 1; x < lx/2; x++)
    {
      for (y = 0; y < ly; y++)
	in_tmp[y] = out[x + y*lx/2];
      gfft_cplx (in_tmp, out_tmp, ly);
      for (y = 0; y < ly; y++)
	out[x + y*lx/2] = out_tmp[y];
    }
  free (in_tmp);
  free (out_tmp);
  free (first);
  free (last);
}

void
im_gifft2d_real (complex *in,
		 real    *out,
		 int     lx,
		 int     ly)
{
  int     x, y;
  real    *out_1d;
  complex *tmp1;
  complex *tmp2;
  complex *out_cplx;
  real    *out_tmp;

  out_cplx = (complex *) out;

  tmp2 = (complex *) malloc (ly*sizeof (complex));
  tmp1  = (complex *) malloc (lx*sizeof (complex));
  out_tmp  = (real *) malloc (ly*sizeof (real));

  for (y = 0; y < ly/2; y++)
      tmp1[y] = in[y*lx/2];
  for (y = ly/2; y < ly; y++)
      tmp2[y-ly/2]  = in[y*lx/2];
  gifft_real (tmp1, out_tmp, ly);
  for (y = 0; y < ly; y++)
    out_cplx[y*lx/2].real = out_tmp[y];
  gifft_real (tmp2, out_tmp, ly);
  for (y = 0; y < ly; y++)
    out_cplx[y*lx/2].imag = out_tmp[y];

  for (x = 1; x < lx/2; x++)
    {
      for (y = 0; y < ly; y++)
	tmp1[y] = in[x + y*lx/2];
      gifft_cplx (tmp1, tmp2, ly);
      for (y = 0; y < ly; y++)
	out_cplx[x + y*lx/2] = tmp2[y];
    }
  for (y = 0; y < ly; y++)
    {
      for (x = 0; x < lx/2; x++)
	tmp1[x] = out_cplx[x + y*lx/2];
      out_1d = &out[y*lx];
      gifft_real (tmp1, out_1d, lx);
    }

  free (tmp1);
  free (tmp2);
  free (out_tmp);
}

/*
void
im_gfft2d_cplx (complex *in,
		complex *out,
		int     lx,
		int     ly)
{
  int     x, y;
  complex *in_1d;
  complex *out_1d;
  complex *in_tmp;
  complex *out_tmp;
  real    *first, *last;

  out_tmp = (complex *) malloc (ly*sizeof (complex));
  in_tmp  = (complex *) malloc (ly*sizeof (complex));

  for (y = 0; y < ly; y++)
    {
      in_1d  = &in[y*lx];
      out_1d = &out[y*lx];
      gfft_cplx (in_1d, out_1d, lx);
    }

  for (x = 0; x < lx; x++)
    {
      for (y = 0; y < ly; y++)
	in_tmp[y] = out[x + y*lx];
      gfft_cplx (in_tmp, out_tmp, ly);
      for (y = 0; y < ly; y++)
	out[x + y*lx] = out_tmp[y];
    }

  free (in_tmp);
  free (out_tmp);
}

void
im_gifft2d_cplx (complex *in,
		 complex *out,
		 int     lx,
		 int     ly)
{
  int     x, y;
  complex *in_1d;
  complex *out_1d;
  complex *in_tmp;
  complex *out_tmp;
  real    *first, *last;

  out_tmp = (complex *) malloc (ly*sizeof (complex));
  in_tmp  = (complex *) malloc (ly*sizeof (complex));

  for (y = 0; y < ly; y++)
    {
      in_1d  = &in[y*lx];
      out_1d = &out[y*lx];
      gfft_cplx (in_1d, out_1d, lx);
    }

  for (x = 0; x < lx; x++)
    {
      for (y = 0; y < ly; y++)
	in_tmp[y] = out[x + y*lx];
      gfft_cplx (in_tmp, out_tmp, ly);
      for (y = 0; y < ly; y++)
	out[x + y*lx] = out_tmp[y];
    }

  free (in_tmp);
  free (out_tmp);
}
*/

/*
 *
 */
void im_fftw2d_real (float *in, float *out, int lx, int ly, int direction, int inplace, int nThreads)
{
  /*
   * Fourier transform using FFTW3 lib compiled with --enable-float
   */
  int i;

  /* declare fftw3 plan structure */
  FFTW_PLAN plan;

  /* when in place direct transform, we have to reorder data */
  if ((inplace == 1) && (direction == 0))
    Shift_Buffer_2D(in,lx,ly);

  /* initialize fftw plan */
#ifdef FFTW_THREADS
  my_fftw_plan_with_nthreads(nThreads);
#endif 
  if ((direction == 0) && (inplace == 0))
    plan = my_fftw_plan_dft_r2c_2d(lx, ly, 
				   (FFTW_REAL    *) in, 
				   (FFTW_COMPLEX *) out,
				   FFTW_ESTIMATE);
  else if ((direction == 0) && (inplace == 1))
    plan = my_fftw_plan_dft_r2c_2d(lx, ly, 
				   (FFTW_REAL    *) in,
				   (FFTW_COMPLEX *) in,
				   FFTW_ESTIMATE);
  else if ((direction == 1) && (inplace == 0))
    plan = my_fftw_plan_dft_c2r_2d(lx, ly, 
				   (FFTW_COMPLEX *) in,
				   (FFTW_REAL    *) out,
				   FFTW_ESTIMATE);
  else if ((direction == 1) && (inplace == 1))
    plan = my_fftw_plan_dft_c2r_2d(lx, ly, 
				   (FFTW_COMPLEX *) in,
				   (FFTW_REAL    *) in,
				   FFTW_ESTIMATE);

  /* compute fft */
  my_fftw_execute(plan);

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
  my_fftw_destroy_plan(plan);

}

/*
 *
 */
void im_fftw3d_real (float *in, float *out, int lx, int ly, int lz, int direction, int inplace, int nThreads)
{
  /*
   * Fourier transform using FFTW2 lib compiled with --enable-float
   */
  int i;

  /* declare fftw plan structure */
  FFTW_PLAN plan;

  /* when in place direct transform, we have to reorder data */
  if ((inplace == 1) && (direction == 0))
    Shift_Buffer_3D(in,lx,ly,lz);

  /* initialize fftw plan */
#ifdef FFTW_THREADS
  my_fftw_plan_with_nthreads(nThreads);
#endif 
  if ((direction == 0) && (inplace == 0))
    plan = my_fftw_plan_dft_r2c_3d(lx, ly, lz, 
				   (FFTW_REAL    *) in, 
				   (FFTW_COMPLEX *) out,  
				   FFTW_ESTIMATE);
  else if ((direction == 0) && (inplace == 1))
    plan = my_fftw_plan_dft_r2c_3d(lx, ly, lz, 
				   (FFTW_REAL    *) in, 
				   (FFTW_COMPLEX *) in,  
				   FFTW_ESTIMATE);
  else if ((direction == 1) && (inplace == 0))
    plan = my_fftw_plan_dft_c2r_3d(lx, ly, lz,
				   (FFTW_COMPLEX *) in,
				   (FFTW_REAL    *) out,
				   FFTW_ESTIMATE);
  else if ((direction == 1) && (inplace == 1))
    plan = my_fftw_plan_dft_c2r_3d(lx, ly, lz, 
				   (FFTW_COMPLEX *) in,
				   (FFTW_REAL    *) in,  
				   FFTW_ESTIMATE);

  /* compute fft */
  my_fftw_execute(plan);

  /* when in place reverse transform, we have to reorder data */
  if ((inplace == 1) && (direction == 1))
    Shift_Inv_Buffer_3D(in,lx,ly,lz);

  /* normalisation */
  if (direction == 1) {
    if (inplace == 1)
      for (i=0;i<lx*ly*2*(lz/2+1);i++)
	in[i] /= (lx*ly*lz);
    else
      for (i=0;i<lx*ly*lz;i++)
	out[i] /= (lx*ly*lz);
  }

  /* delete fftw plan */
  my_fftw_destroy_plan(plan);

}
