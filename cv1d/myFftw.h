/*
 * myFftw.h --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: myFftw.h,v 1.2 1999/05/05 20:42:26 decoster Exp $
 */

#ifndef _MYFFTW_H_
#define _MYFFTW_H_

#include <stdio.h> // for FILE

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

void myFftw_real(real *in, complex *out, int size);

void myFftwI_real(complex *in, real *out, int size);

void myFftw_cplx(complex *in, complex *out, int size);

void myFftwI_cplx(complex *in, complex *out, int size);

int my_is_good_fftw_size (int size);

int my_next_good_fftw_size (int size);

real my_fftw_get_time (int size);

void my_fftw_init (int, int, FILE *);

#endif /* _MYFFTW_H_ */
