/*
 * image.h --
 *
 * Header of the image library.
 *
 *   Copyright (c) 1999 Nicolas Decoster
 *   Copyright (c) 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *
 *   Copyright (c) 1999-2007 Pierre Kestener.
 *   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
 *   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
 *   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
 */

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdio.h>

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

/*
 * Constants for numerical recipes FFT algorithm.
 */
#define REVERSE -1
#define DIRECT 1

typedef struct {
  real *       data;	/* Array that contains image data lines after lines */
  int          lx;	/* Width of the image */
  int          ly;	/* Heigth of the image */
  int          size;	/* Allocated memory, size of array data. */
  unsigned int type;	/* Type of the image (see below). */
  real         min;	/* Min value in the image. */
  real         max;	/* Max value in the image. */
			/* max < min means that they are not set. */
  real         avg;	/* Avg value in the image. */
  int          border_hor; /* Horizontal size of "border effects". */
  int          border_ver; /* Vertical size of "border effects". */
} Image;

/*
 * Different kind of image (field type of the Image structure).
 */
#define   PHYSICAL  1	/* size = lx*ly */
#define   FOURIER   2	/* size = lx*ly*2 */
#define   FFTW_R2C  3   /* size = lx*2*(ly/2+1) */

/*
 *
 */
Image * im_new			(int, int, int, int);
void    im_set_0		(Image *);
Image * im_duplicate		(Image *);
void    im_free			(Image *);
void    im_get_extrema		(Image *, real *, real *);
void    im_get_extrema_increment(Image *, real *, real *, int, int, Image *);
void    im_get_average		(Image *, real *);
float   Urand                   (void);
float   Grand                   (float sigma);


Image * im_real_to_complex	(Image *);
Image * im_mult			(Image *, Image *);
Image * im_scalar_mult		(Image *, real);
Image * im_scalar_add		(Image *, real);
Image * im_add			(Image *, Image *);
Image * im_max			(Image *, Image *);
Image * im_filter		(Image *, real);
Image * im_conv			(Image *, Image *);
Image * im_xy_to_mod		(Image *, Image *);
Image * im_xy_to_arg		(Image *, Image *, real);
Image * im_cut_edge		(Image *, int);
Image * im_insert		(Image *, Image *, int, int);

Image * im_set_border		(Image *, int, int, int, int, real);

Image * im_diamond		(int);
Image * im_dirac		(int, int, int, int, int);
Image * im_gauss		(int, int, int, real, int, real);
Image * im_cell                 (int, int, int, int, real, real, real);
Image * im_cell_elli                 (int, int, int, int, int, int, real, real, real);
Image * im_cell3Dproj           (int, int, int, int, real, real, real);
Image * im_gauss_ellipse     	(int, int, int, real, real, real);
Image * im_step			(int);
Image * im_circle               (int, int, int, int, int);

Image * im_ellipse		(int, real);
Image * my_im_ellipse		(int, int, int, real, real, real);
Image * my_im_ellipse_3Dproj	(int, int, int, int, real, real, real, real);
Image * im_cell_ellipsoid_proj	(int, int, int, int, int, int, int, int, real, real, real, int);
Image * im_cell_ellipsoid_slice	(int, int, int, int, int, int, int, int, int, int, int, int, int, int, real, real, real, real, real, real, real, int);
void    im_construit_brownien_fft (real *, int, real, int, int, void *);
void    im_construit_brownien_fft2 (real *, real *, int, real, int, int, void *);
void    im_construit_brownien_fft_anis (real *, int, real, int, int, void *, float);
void    im_construit_brownien_fft_anis2 (real *, int, real, real, int, int);
void    im_construit_brownien_fft_bsheet (real *, int, real, real, int, int);
void    im_construit_brownien_fft_anis_montseny (real *, int, real, real, real, int, int);
void    im_construit_brownien_fft_anis_yafmont (real *, int, real, real, real, real, real, int, int);
void    im_construit_brownien_fft_anis_yaf (real *, int, real, real, real, real, int, int);

void    im_construit_brownien_2D_field_fft(real *, real *, int, real, real, int, int, int, int, void *);
void    im_construit_brownien_2D_field_fft_pedagogic(real *, real *, real *, real *, real *, real *, int, real, real, real, int, int, int, int, void *);
void    im_construit_brownien_2D_field_fft_div_free_pedagogic(real *, real *, real *, real *, int, real, real, int, int, int, int, void *);
void    im_construit_brownien3D_fft (float *, int, real, int, int, void *);
void    im_Lit_dla_		(real *,char *, int, int, real);
void    im_bruit_blanc_		(real *, int, real, int, int);
void	im_construit_brownien_mid_point (real *, int, real, int, real);
void	im_construit_brownien3D_mid_point (FILE *, int, real, int, real);
void	im_construit_brownien_mid_point_special	(real *, int, real, int, real);
void    _SFlake_		(real *, int, real, real, real, real, real,
				 real, real, real, real, real, real, real,
				 real, real);
void    fl2			(real *, int, real, real, real, real, real, int);

void    _ImaNuaLoadFunction_	(Image *, char *, int, int, int, int, int, int,
				 int, int);
void    _ImaSimNuaLoadFunction_	(Image *, char *, int);

Image * im_fourier_conv_	(Image *, char  *, int, int);

void    _Fourn_			(real *, unsigned long*, int, int);
Image * im_direct_to_fourier	(Image *srcImage, int   dim);
Image * im_direct_to_fourier2   (Image *srcImage, Image *src2Image, int dim);
Image * im_fourier_to_direct	(Image *srcImage);

extern void  im_gfft2d_real	(real *, complex *, int, int);
extern void  im_gifft2d_real	(complex *, real *, int, int);
extern void  im_fftw2d_real	(float *, float *, int, int, int, int, int);
void    im_gfft_to_2real	(Image *, Image *, Image *);


extern Image * im_mult_analog2	(Image *, char *, real, int);
extern Image * im_mult_analog	(Image *, real, real,
				 void *, void *);
extern void    im_fftw_filter	(Image *, real,
				 void *, void *);
extern void im_fftw_powerspectrum (Image  *, real *, int, real, int    *);

Image * im_thresh		(Image *, real, real, real);

/* must not be here ! */

#define NO_ARG 10


#endif /* __IMAGE_H__ */
