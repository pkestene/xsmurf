/*
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster and Stephane Roux.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *  or  decoster@info.enserb.u-bordeaux.fr
 *
 *  Declaration of the signal library.
 */

#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#ifndef _REAL_
#define _REAL_

typedef float real;

#endif

#ifndef _COMPLEX_
#define _COMPLEX_
typedef struct
{
  real real;
  real imag;
} complex;
#endif

/*
 * There are several types of signals.
 *  - REALXY is made of two arrays X and Y (X is supposed to be made of
 *    increasing numbers for now )
 *  - REALY is supposed to have uniformed samples. So X is not used. Instead,
 *    we use x0 (i.e., first value of x) and dx (i.e., the sample rate).
 *  - CPLX
 *  - FOUR_NR
 */
enum {
  REALXY,
  REALY,
  CPLX,
  FOUR_NR
};

enum {
  REAL_CPLX,
  CPLX_CPLX
};

enum {
  ALONGX,
  ALONGY
};

typedef struct
{
  unsigned int type;
  real         *dataX;
  real         *dataY;
  int          first;
  int          last;
  int          size;	/* Size of array dataY. */
  real         x0;	/* first value of x */
  real         dx;	/* sample rate */
  int          n;	/* Number of points in the signal. */
} Signal;

typedef struct
{
  double x,y;
} XY;


Signal * sig_new                 (int, int, int);
void     sig_free                (Signal *);
void     sig_get_extrema         (Signal *, void*, void*, void*, void*);
void     sig_get_extrema_between (Signal *, int, int, void*, void*, void*, void*);
/*void     sig_get_extrema         (Signal *, void*, void*); */
int      sig_get_xy_extrema         (Signal *, real*, real*, real*, real*, int *, int *);
/*void     sig_get_extrema_between (Signal *, int, int, void*, void*);*/
Signal * sig_duplicate           (Signal *);


#define REVERSE -1
#define DIRECT 1

Signal * sig_fft_nr (Signal *, int);
Signal * sig_gfft   (Signal *, int, int);

Signal * sig_add             (Signal *, Signal *);
Signal * sig_mult            (Signal *, Signal *);
Signal * sig_scalar_mult     (Signal *, real);
Signal * sig_fourier_to_cplx (Signal *);
Signal * sig_conv            (Signal *, Signal *);
Signal * sig_extend          (Signal *);
Signal * sig_extend_to       (Signal *, int);
Signal * sig_extend_to_x     (Signal *, int, int);
Signal * sig_cut             (Signal *, int, int);
Signal * sig_shift           (Signal *, int);

Signal * sig_real_to_fourier (Signal *);

int sig_get_index (Signal *signal, real   x);

void srfft(double *xr,double *xi,int logm);
void srifft(double *xr,double *xi,int logm);
void TransformeFourier (double *tableau, int    N, int    logN, double *re, double *im);
void sig_gfft_to_2real (Signal *source,	Signal *res_real, Signal *res_imag);
void sig_2real_to_gfft (Signal *result,
		   Signal *src_real,
		   Signal *src_imag);

Signal *sig_realy2realxy (Signal *s);
Signal *sig_put_y_in_x (Signal *s);


/* **********************************
 * function from interpolation.c 
 * ********************************** */

Signal *sig_to_pol_int_derivative (Signal *source, int order);
void   polint(float *xa, float *ya, int n, float x, float *y, float *dy);
double CubicInterpolate(double, double, double, double, double);

Signal *sig_to_spline_int_derivative (Signal *source, int order);
void spline(float *x, float *y, int n, float yp1, float ypn, float *y2);
void splint(float *xa, float *ya, float *y2a, int n, float x, float *y);

Signal *sig_to_spline2_int_derivative (Signal *sig_in, int order, int direction);
Signal *sig_to_spline3_int_derivative (Signal *sig_in, int order, int direction);
void SplinePoint(int *u, int n, int t, double v, XY *control,XY *output);
double SplineBlend(int k,int t,int *u,double v);
void SplineKnots(int *u,int n,int t);
void SplineCurve(XY *inp,int n,int *knots,int t,XY *outp,int res);




#define sig_index_to_real(signal,index) (signal->x0+(index)*signal->dx)

#endif
