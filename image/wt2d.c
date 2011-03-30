/*
 * wt2d.c --
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

/*
 * Global variables for convolution computations.
 */
static real * _freq1_     = 0, *_freq2_     = 0;
static int    _allocSizeLx_=0    , _allocSizeLy_=0;

/*------------------------------------------------------------------------
  _ImaCvlInit_
  
  Initialisation avant convolution.
  ----------------------------------------------------------------------*/
static void _ImaCvlInit_ (int sizeX,int sizeY)
{
  int i, size2;
  
  if (sizeX > _allocSizeLx_)
    {
      if (_allocSizeLx_ != 0)
	free(_freq1_);
      _freq1_       = (real*) malloc (sizeX * sizeof (real));
      _allocSizeLx_ = sizeX;
    }
  if (sizeY > _allocSizeLy_)
    {
      if (_allocSizeLy_ != 0)
	free(_freq2_);
      _freq2_       = (real*) malloc (sizeY * sizeof (real));
      _allocSizeLy_ = sizeY;
    }
  
  size2 = sizeX / 2;

  for (i = 0; i < size2; i++)
    {
      _freq1_[i]         = ((real) i) / ((real) sizeX);
      _freq1_[i + size2] = ((real) i - (real) size2) / ((real) sizeX);
    }
  size2 = sizeY / 2;
  for (i = 0; i < size2; i++)
    {
      _freq2_[i]         = ((real) i) / ((real) sizeY);
      _freq2_[i + size2] = ((real) i - (real) size2) / ((real) sizeY);
    }
}

 /*------------------------------------------------------------------------
   a refaire ...
  ----------------------------------------------------------------------*/
Image *
im_fourier_conv_ (Image *image,
		  char  *filter,
		  int   Scale,
		  int   flag)
{
  int i, j, k;
  real x1, x2, wave, u, v;
  double scale,scale2,scale3;
  real *fftData;
  int imageLx,imageLy;
  int length;

  /* initialisations */
  fftData=image->data;
  scale =Scale;
  scale2=Scale*Scale;
  scale3=Scale*Scale*Scale;  
  length=strlen(filter);
  imageLx=next_power_of_2__(image->lx);
  imageLy=next_power_of_2__(image->ly);
  _ImaCvlInit_(imageLx,imageLy);
  
  if (!(strncmp(filter,"gauss",length))) /****  Gaussienne ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = ((real) exp ((double) (-x1)));
	    fftData[k] *= wave;
	    fftData[k + 1] *= wave;
	  }
    }  
  
  else if (!(strncmp(filter,"unknown",length)))    /**** unknown ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = ((real) exp ((double) (-x1)));
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k] = -scale3 * x2 * v * wave;
	    fftData[k + 1] = scale3 * scale * x2 * u * wave;
	  }
    }
  
  else if (!(strncmp(filter,"mexican",length))) /****  Chapeau Mexicain ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = ((real) exp ((double) (-x1)));
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k]*= x1 * wave;
	    fftData[k + 1]*= x1 * wave;
	  }
    } 
  
  else if (!(strncmp(filter,"dx",length)))  /****  d/dx Gaussienne ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = flag?1:((real) exp ((double) (-x1))); /* change le signe ?! */ 
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k] = scale * _freq1_[i] * v * wave;
	    fftData[k + 1] = -scale * _freq1_[i] * u * wave;
	  }
    }
  
  else if (!(strncmp(filter,"dy",length)))  /****  d/dy Gaussienne ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = flag?1:((real) exp ((double) (-x1)));
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k] = scale * _freq2_[j] * v * wave;
	    fftData[k + 1] = -scale * _freq2_[j] * u * wave;
	  }
    }

  else if (!(strncmp(filter,"dxx",length)))  /****  d2/dx2 Gaussienne ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = flag?1:((real) exp ((double) (-x1))); /* change le signe ?! */ 
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k] *= -scale2 * (_freq1_[i]*_freq1_[i]) * wave;
	    fftData[k + 1] *= -scale2 * (_freq1_[i]*_freq1_[i]) * wave;
	  }
    }
  else if (!(strncmp(filter,"dyy",length)))  /****  d2/dy2 Gaussienne ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = flag?1:((real) exp ((double) (-x1))); /* change le signe ?! */ 
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k] *= -scale2 * (_freq2_[j]*_freq2_[j]) * wave;
	    fftData[k + 1] *= -scale2 * (_freq2_[j]*_freq2_[j]) * wave;
	  }
    }
  else if (!(strncmp(filter,"dxy",length)))  /****  d2/dxdy Gaussienne ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = flag?1:((real) exp ((double) (-x1))); /* change le signe ?! */ 
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k] *= -scale2 * _freq1_[i]*_freq2_[j] * wave;
	    fftData[k + 1] *= -scale2 * _freq1_[i]*_freq2_[j] * wave;
	  }
    }
  else if (!(strncmp(filter,"dxxx",length)))  /****  d3/dx3 Gaussienne ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = flag?1:((real) exp ((double) (-x1))); /* change le signe ?! */ 
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k] = -v*scale3*_freq1_[i]*_freq1_[i]*_freq1_[i]* wave;
	    fftData[k + 1] = u*scale3*_freq1_[i]*_freq1_[i]*_freq1_[i]*wave;
	  }
    }
  else if (!(strncmp(filter,"dyyy",length)))  /****  d3/dy3 Gaussienne ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = flag?1:((real) exp ((double) (-x1))); /* change le signe ?! */ 
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k] = -v*scale3*_freq2_[j]*_freq2_[j]*_freq2_[j]* wave;
	    fftData[k + 1] = u*scale3*_freq2_[j]*_freq2_[j]*_freq2_[j]*wave;
	  }
    }
  else if (!(strncmp(filter,"dxxy",length)))  /****  d3/dx2dy Gaussienne ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = flag?1:((real) exp ((double) (-x1))); /* change le signe ?! */ 
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k] = -v*scale3*_freq1_[i]*_freq1_[i]*_freq2_[j]* wave;
	    fftData[k + 1] = u*scale3*_freq1_[i]*_freq1_[i]*_freq2_[j]*wave;
	  }
    }
  else if (!(strncmp(filter,"dxyy",length)))  /****  d3/dxdy2 Gaussienne ****/
    {
      for (i = 0; i < imageLx; i++)
	for (j = 0; j < imageLy; j++)
	  {
	    k = 2 * imageLx * j + 2 * i + 1;
	    x2 = _freq1_[i] * _freq1_[i] + _freq2_[j] * _freq2_[j];
	    x1 = scale2*x2 ;
	    wave = flag?1:((real) exp ((double) (-x1))); /* change le signe ?! */ 
	    u = fftData[k];
	    v = fftData[k + 1];
	    fftData[k] = -v*scale3*_freq1_[i]*_freq2_[j]*_freq2_[j]* wave;
	    fftData[k + 1] = u*scale3*_freq1_[i]*_freq2_[j]*_freq2_[j]*wave;
	  }
    }
  return image;
}

Image *
im_mult_analog2 (Image *image,
		 char  *filter,
		 real Scale,
		 int   flag)
{
  int i, j, k;
  real x1, x2, wave;
  double scale,scale2,scale3;
  real *fftData;
  int imageLx,imageLy;
  int length;
  complex *cdata;

  (void) flag;

  /* initialisations */
  fftData=image->data;
  scale =Scale;
  scale2=scale*scale;
  scale3=Scale*Scale*Scale;  
  length=strlen(filter);

  imageLx = image -> lx;
  imageLy = image -> ly;
  
  cdata = (complex *) image -> data;

  if (!(strncmp (filter, "gauss", length))) /****  Gaussienne ****/
    {
      cdata[0].real *= ((real) exp ((double) (0)));
      cdata[0].imag *= ((real) exp ((double) (-0.25*scale2)));

      k = imageLx*imageLy/4;
      cdata[k].real *= ((real) exp ((double) (-0.25*scale2)));
      cdata[k].imag *= ((real) exp ((double) (-0.5*scale2)));

      for (j = 1; j < imageLy/2; j++)
	{
	  k = imageLx*j/2;
	  x2 = j*j*1.0/imageLy/imageLy;
	  x1 = scale2*x2 ;
	  wave = ((real) exp ((double) (-x1)));
	  cdata[k].real *= wave;
	  cdata[k].imag *= wave;

	  k = imageLx*(j+imageLy/2)/2;
	  x2 = j*j*1.0/imageLy/imageLy + 0.25;
	  x1 = scale2*x2 ;
	  wave = ((real) exp ((double) (-x1)));
	  cdata[k].real *= wave;
	  cdata[k].imag *= wave;
	}
      for (i = 1; i < imageLx/2; i++)
	{
	  for (j = 0; j < imageLy/2; j++)
	    {
	      k = imageLx*j/2 + i;
	      x2 = i*i*1.0/imageLx/imageLx + j*j*1.0/imageLy/imageLy;
	      x1 = scale2*x2 ;
	      wave = ((real) exp ((double) (-x1)));
	      cdata[k].real *= wave;
	      cdata[k].imag *= wave;

	      k = imageLx*(imageLy-1-j)/2 + i;
	      x2 = i*i*1.0/imageLx/imageLx + (1+j)*(1+j)*1.0/imageLy/imageLy;
	      x1 = scale2*x2 ;
	      wave = ((real) exp ((double) (-x1)));
	      cdata[k].real *= wave;
	      cdata[k].imag *= wave;
	    }
	}
    }  

  return image;
}

/*
 * Compute the complex multiplication between an image and a function. The
 * image data are "organized like after the gfft2d_real function" (if you see
 * what I mean).
 */
Image *
im_mult_analog (Image  *image,
		real   scale,
		real   normalisation,
		/*double r_fct (double, double),
		  double i_fct (double, double)*/
		void *r_fct,
		void *i_fct)
{
  int  i, j, k;
  real x, y;
  real r_val, i_val;
  real d_real;
  int  lx, ly;

  complex *cdata;

  (void) normalisation;

  lx = image -> lx;
  ly = image -> ly;
  
  cdata = (complex *) image -> data;

  /*cdata[0].real *= ((real) r_fct (0.0, 0.0));
    cdata[0].imag *= ((real) r_fct (0.0, -0.5*scale));*/
  cdata[0].real *= ((real) evaluator_evaluate_x_y(r_fct, 0.0, 0.0));
  cdata[0].imag *= ((real) evaluator_evaluate_x_y(r_fct, 0.0, -0.5*scale));

  k = lx*ly/4;
  /*cdata[k].real *= ((real) r_fct (0.5*scale, 0.0));
    cdata[k].imag *= ((real) r_fct (0.5*scale, -0.5*scale));*/
  cdata[k].real *= ((real) evaluator_evaluate_x_y(r_fct, 0.5*scale, 0.0));
  cdata[k].imag *= ((real) evaluator_evaluate_x_y(r_fct, 0.5*scale, -0.5*scale));

  for (j = 1; j < ly/2; j++)
    {
      k = lx*j/2;
      y = j*scale/ly;
      /*r_val = ((real) r_fct (0.0, y));
	i_val = ((real) i_fct (0.0, y));*/
      r_val = ((real) evaluator_evaluate_x_y(r_fct, 0.0, y));
      i_val = ((real) evaluator_evaluate_x_y(i_fct, 0.0, y));
      d_real = cdata[k].real;
      cdata[k].real = cdata[k].real*r_val - cdata[k].imag*i_val;
      cdata[k].imag = d_real*i_val + cdata[k].imag*r_val;

      k = lx*(j+ly/2)/2;
      y = j*scale/ly;
      /*r_val = ((real) r_fct (0.5*scale, y));
	i_val = ((real) i_fct (0.5*scale, y));*/
      r_val = ((real) evaluator_evaluate_x_y(r_fct, 0.5*scale, y));
      i_val = ((real) evaluator_evaluate_x_y(i_fct, 0.5*scale, y));
      d_real = cdata[k].real;
      cdata[k].real = cdata[k].real*r_val - cdata[k].imag*i_val;
      cdata[k].imag = d_real*i_val + cdata[k].imag*r_val;
    }
  for (j = 0; j < ly/2; j++) {
    for (i = 1; i < lx/2; i++) {
      k = lx*j/2 + i;
      x = i*scale/lx;
      y = j*scale/ly;
      /*r_val = ((real) r_fct (x, y));
	i_val = ((real) i_fct (x, y));*/
      r_val = ((real) evaluator_evaluate_x_y(r_fct, x, y));
      i_val = ((real) evaluator_evaluate_x_y(i_fct, x, y));
      d_real = cdata[k].real;
      cdata[k].real = cdata[k].real*r_val - cdata[k].imag*i_val;
      cdata[k].imag = d_real*i_val + cdata[k].imag*r_val;

      k = lx*(ly-1-j)/2 + i;
      x = i*scale/lx;
      y = -(1+j)*scale/ly;
      /*r_val = ((real) r_fct (x, y));
	i_val = ((real) i_fct (x, y));*/
      r_val = ((real) evaluator_evaluate_x_y(r_fct, x, y));
      i_val = ((real) evaluator_evaluate_x_y(i_fct, x, y));
      d_real = cdata[k].real;
      cdata[k].real = cdata[k].real*r_val - cdata[k].imag*i_val;
      cdata[k].imag = d_real*i_val + cdata[k].imag*r_val;
    }
  }

  return image;
}

/*
 * Compute the complex multiplication between an image of type FFTW_R2C 
 * and a function. The image data are organized as specified in the
 * fftw lib documentation (real multi-dimensional transform)
 */
void
im_fftw_filter (Image  *image,
		real   scale,
		/*double r_fct (double, double),
		  double i_fct (double, double)*/
		void *r_fct,
		void *i_fct)
{
  int           i, j;
  int          lx, ly;
  int          Lx,Ly;
  real         tmp, kx, ky;
  FFTW_COMPLEX *cData;
  
  FFTW_REAL     r_val, i_val;
  
  lx = image -> lx;
  ly = image -> ly;
  Lx = ly;
  Ly = lx;
  cData = (FFTW_COMPLEX *) &(image->data[0]);

  for (i = 0; i < Lx; i++)
    for (j = 0; j < Ly/2+1; j++) {
      
      int ij = j + i*(Ly/2+1);
    
      if (i < Lx/2) {
	kx = (FFTW_REAL) i*scale/Lx;
      } else {
	kx = (FFTW_REAL) (i-Lx)*scale/Lx;
      }
      ky = (FFTW_REAL) j*scale/Ly;
      
      /*r_val = ((FFTW_REAL) r_fct (kx, ky));
	i_val = ((FFTW_REAL) i_fct (kx, ky));*/
      r_val = ((FFTW_REAL) evaluator_evaluate_x_y(r_fct, kx, ky));
      i_val = ((FFTW_REAL) evaluator_evaluate_x_y(i_fct, kx, ky));

      tmp = cData[ij][0];
      /*printf("%f %f\n",kx,ky);*/
      
      cData[ij][0] = cData[ij][0]*r_val - cData[ij][1]*i_val;
      cData[ij][1] = tmp         *i_val + cData[ij][1]*r_val;
      
    }

}

/*
 * Compute the Fourier power spectrum.
 */
void
im_fftw_powerspectrum (Image *image,
		       real  *result,
		       int    resSize,
		       real   dx,
		       int   *nb)
{
  int           i, j, index;
  int           lx, ly;
  int           Lx,Ly;
  FFTW_REAL     kx, ky;
  FFTW_COMPLEX *cData;
  
  real          logk;
  FFTW_REAL     power;
  
  lx = image -> lx;
  ly = image -> ly;
  Lx = ly;
  Ly = lx;
  cData = (FFTW_COMPLEX *) &(image->data[0]);

  for (i = 0; i < Lx; i++)
    for (j = 0; j < Ly/2+1; j++) {
      
      int ij = j + i*(Ly/2+1);
      
      if (i < Lx/2) {
	kx = (FFTW_REAL) i;
      } else {
	kx = (FFTW_REAL) (i-Lx);
      }
      ky = (FFTW_REAL) j;
      
      logk = (real) sqrt(kx*kx + ky*ky);
      power = cData[ij][0]*cData[ij][0]+cData[ij][1]*cData[ij][1];
      
      index = (int) (logk/dx+0.5);
      result[index] += power;
      nb[index]++;
    }
  
  for (index = 0; index < resSize; index++) {
    if (nb[index] != 0) {
      result[index] /= nb[index];
    }
  }
  
}
