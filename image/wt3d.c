/*
 * wt3d.c --
 *
 *   Copyright (c) 1999-2007 Pierre Kestener.
 *   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
 *   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
 *   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
 *
 */

#include "image_int.h"

/*
 * Wavelets derivatives expression definition for FFTW
 * x and z are swapped because fftw lib expects row-major ordered data
 *
 */
static double _gaussian_dx_ (double x, double y, double z) {

  return z*exp(-x*x-y*y-z*z);

}

static double _gaussian_dy_ (double x, double y, double z) {

  return y*exp(-x*x-y*y-z*z);

}

static double _gaussian_dz_ (double x, double y, double z) {

  return x*exp(-x*x-y*y-z*z);

}

/* 
 * mexican wavelet
 */
static double _mexican_dx_ (double x, double y, double z) {

  return z*(x*x+y*y+z*z)*exp(-x*x-y*y-z*z);

}

static double _mexican_dy_ (double x, double y, double z) {

  return y*(x*x+y*y+z*z)*exp(-x*x-y*y-z*z);

}

static double _mexican_dz_ (double x, double y, double z) {

  return x*(x*x+y*y+z*z)*exp(-x*x-y*y-z*z);

}


/*
 * Compute the complex multiplication between an image of type FFTW_R2C 
 * and a function. The image data are organized as specified in the
 * fftw lib documentation (real multi-dimensional transform)
 */
void
im_fftw3d_filter (Image3D  *image,
		  real   scale,
		  int waveType,
		  int direction)
{
  int           i, j, k;
  int           lx, ly, lz;
  int           Lx,Ly,Lz;
  FFTW_REAL     tmp, kx, ky, kz;
  FFTW_COMPLEX *cData;
  
  FFTW_REAL     r_val, i_val;

  /*double (*r_fct)();*/
  double (*i_fct)(double x, double y, double z);
  
  lx = image -> lx;
  ly = image -> ly;
  lz = image -> lz;
  Lx = lz;
  Ly = ly;
  Lz = lx;
  cData = (FFTW_COMPLEX *) &(image->data[0]);

  if (waveType == 0) { /* gaussian wavelet */
    
    if (direction == 0) {        /* grad X */
      i_fct = _gaussian_dx_;
    } else if (direction == 1) { /* grad Y */
      i_fct = _gaussian_dy_;
    } else {                    /* grad Z */
      i_fct = _gaussian_dz_;
    }

  } else { /* mexican */

    if (direction == 0) {        /* grad X */
      i_fct = _mexican_dx_;
    } else if (direction == 1) { /* grad Y */
      i_fct = _mexican_dy_;
    } else {                    /* grad Z */
      i_fct = _mexican_dz_;
    }

  }

  /*printf("direction %d waveType %d\n",direction,waveType);
    printf("toto %f\n",i_fct(1,2,3));*/

  for (i = 0; i < Lx; i++)
    for (j = 0; j < Ly; j++)
      for (k = 0; k < Lz/2+1; k++) {
      
      int ijk = k + (Lz/2+1)*(j + Ly*i);
    
      if (i < Lx/2) {
	kx = (FFTW_REAL) i*scale/Lx;
      } else {
	kx = (FFTW_REAL) (i-Lx)*scale/Lx;
      }

      if (j < Ly/2) {
	ky = (FFTW_REAL) j*scale/Ly;
      } else {
	ky = (FFTW_REAL) (j-Ly)*scale/Ly;
      }
      kz = (FFTW_REAL) k*scale/Lz;
      
      /*r_val = ((FFTW_REAL) r_fct (kx, ky ,kz));*/
      r_val = 0.0;
      i_val = ((FFTW_REAL) i_fct (kx, ky, kz));

      tmp = cData[ijk][0];
      /*printf("%f %f\n",kx,ky);*/
      
      /*cData[ijk][0] = cData[ijk][0]*r_val - cData[ijk][1]*i_val;
	cData[ijk][1] = tmp          *i_val + cData[ijk][1]*r_val;*/
      cData[ijk][0] = - cData[ijk][1]*i_val;
      cData[ijk][1] =   tmp          *i_val;
      
    }

}

/*
 * Compute the complex multiplication between an image of type FFTW_R2C 
 * and a function. The image data are organized as specified in the
 * fftw lib documentation (real multi-dimensional transform)
 */
void
im_fftw3d_filter_fct (Image3D  *image,
		      real   scale,
		      void *r_fct,
		      void *i_fct)
{
  int           i, j, k;
  int           lx, ly, lz;
  int           Lx,Ly,Lz;
  FFTW_REAL     tmp, kx, ky, kz;
  FFTW_COMPLEX *cData;
  
  FFTW_REAL     r_val, i_val;

  lx = image -> lx;
  ly = image -> ly;
  lz = image -> lz;
  Lx = lz;
  Ly = ly;
  Lz = lx;
  cData = (FFTW_COMPLEX *) &(image->data[0]);

  for (i = 0; i < Lx; i++)
    for (j = 0; j < Ly; j++)
      for (k = 0; k < Lz/2+1; k++) {
      
      int ijk = k + (Lz/2+1)*(j + Ly*i);
    
      if (i < Lx/2) {
	kx = (FFTW_REAL) i*scale/Lx;
      } else {
	kx = (FFTW_REAL) (i-Lx)*scale/Lx;
      }

      if (j < Ly/2) {
	ky = (FFTW_REAL) j*scale/Ly;
      } else {
	ky = (FFTW_REAL) (j-Ly)*scale/Ly;
      }
      kz = (FFTW_REAL) k*scale/Lz;
      
      r_val = (FFTW_REAL) evaluator_evaluate_x_y_z(r_fct, kx, ky ,kz);
      /*r_val = 0.0;*/
      i_val = (FFTW_REAL) evaluator_evaluate_x_y_z(i_fct, kx, ky, kz);

      tmp = cData[ijk][0];
      /*printf("%f %f\n",kx,ky);*/
      
      cData[ijk][0] = cData[ijk][0]*r_val - cData[ijk][1]*i_val;
      cData[ijk][1] = tmp          *i_val + cData[ijk][1]*r_val;
      /*cData[ijk][0] = - cData[ijk][1]*i_val;
	cData[ijk][1] =   tmp          *i_val;*/
      
    }

}

/*
 * Compute the Fourier power spectrum.
 */
void
im3D_fftw_powerspectrum (Image3D *image,
			 real  *result,
			 int    resSize,
			 real   dx,
			 int   *nb)
{
  int          i, j, k, index;
  int          lx, ly, lz;
  int          Lx, Ly, Lz;
  FFTW_REAL    kx, ky, kz;
  FFTW_COMPLEX *cData;
  
  real   logk;
  FFTW_REAL    power;
  
  lx = image -> lx;
  ly = image -> ly;
  lz = image -> lz;
  Lx = lz;
  Ly = ly;
  Lz = lx;
  cData = (FFTW_COMPLEX *) &(image->data[0]);

  for (i = 0; i < Lx; i++)
    for (j = 0; j < Ly; j++)
      for (k = 0; k < Lz/2+1; k++) {
	
	int ijk = k + (Lz/2+1)*(j + Ly*i);
	
	
	if (i < Lx/2) {
	  kx = (FFTW_REAL) i;
	} else {
	  kx = (FFTW_REAL) (i-Lx);
	}
	
	if (j < Ly/2) {
	  ky = (FFTW_REAL) j;
	} else {
	  ky = (FFTW_REAL) (j-Ly);
	}
	kz = (FFTW_REAL) k;
     
	logk = (real) sqrt(kx*kx + ky*ky + kz*kz);
	power = cData[ijk][0] * cData[ijk][0] 
	  +     cData[ijk][1] * cData[ijk][1];

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
