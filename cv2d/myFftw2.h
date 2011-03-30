/*
 * myFftw2.h --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: myFftw2.h,v 1.2 1999/05/06 11:59:01 decoster Exp $
 */

#ifndef _myFftw2_H_
#define _myFftw2_H_

#include <stdio.h>

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

void myFftw2_real(real *in, complex *out, int size, int);

void myFftw2I_real(complex *in, real *out, int size, int);

void myFftw2_cplx(complex *in, complex *out, int sizex, int sizey);

void myFftw2I_cplx(complex *in, complex *out, int sizex, int sizey);

int my_is_good_fftw_size (int size);

int my_is_good_fftw2_size (int size);

int my_next_good_fftw2_size (int size);

real my_fftw2_get_time (int size);

void my_fftw2_init (int, int, FILE *);

real cv2d_get_fftw_factor (int nx, int ny);

#endif /* _myFftw2_H_ */
