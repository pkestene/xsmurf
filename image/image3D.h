/*
 * image3D.h --
 *
 * Header of the image3d library.
 *
 *   Copyright (c) 1999-2007 Pierre Kestener.
 *   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
 *   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
 *   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
 *
 *  Created on december 10th 2002
 */


#include <stdio.h>

#ifndef __IMAGE3D_H__
#define __IMAGE3D_H__

/* 
 * New type. 'real' is the type of the data. Switch to 'double' if you want to
 * change the precision.
 */
#ifndef _REAL_
#define _REAL_
typedef float real;
#endif

#ifndef _COMPLEX_
#define _COMPLEX_
typedef struct {
  real real;
  real imag;
} complex;
#endif

typedef struct {
  real *       data;	/* Array that contains image data lines after lines */
  int          lx;	/* Width of the image */
  int          ly;	/* Heigth of the image */
  int          lz;	/* Thrird dimension of the image */
  int          size;	/* size of data array */
  unsigned int type;    /* PHYSICAL or FFTW_R2C */
  real         min;	/* Min value in the image. */
  real         max;	/* Max value in the image. */
			/* max < min means that they are not set. */
} Image3D;

/*
 *
 */
Image3D * im3D_new		(int, int, int, int, unsigned int);
void      im3D_set_0		(Image3D *);
Image3D * im3D_duplicate        (Image3D *);
void      im3D_free		(Image3D *);
void      im3D_get_extrema	(Image3D *, real *, real *);
Image3D * im_cell_3D            (int, int, int, int, real, real, real, char *);

void      im_fftw3d_real        (float *, float *, int, int, int, int, int, int);
void      im_fftw3d_filter	(Image3D *, real, int, int);
void      im_fftw3d_filter_fct(Image3D *, real, void *, void *);
void      im3D_fftw_powerspectrum (Image3D  *, real *, int, real, int    *);


#endif /* __IMAGE3D_H__ */
