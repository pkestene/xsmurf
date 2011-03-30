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
 *              The first one (REALXY) is made of two arrays X and Y 
 *                              (X is supposed to be made of increasing numbers for now )
 *              The second one (REALY) is supposed to have uniformed samples. So X is
 *                      not used. Instead, we use x0 (i.e., first x value) and 
 *                       dx (i.e., the sample rate).
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


typedef struct
{
  unsigned int type;     /* either REALXY or REALY or CPLX or FOUR_NR */
  real         *dataX;  /* the X array */
  real         *dataY;  /* the Y array */
  int          first;
  int          last;
  int          size;     /* size of the signal */
  real         dx;       /* sample rate (for type REALY) */
  real         x0;       /* first x value (for type REALY ) */
} Signal;

Signal * sig_new                 (int, int, int);
void     sig_free                (Signal *);
void     sig_get_extrema         (Signal *, void*, void*);
void     sig_get_extrema_between (Signal *, int, int, void*, void*);
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


#endif
