/* cv2d_fft.c --
 *
 *  This file defines the references to the fft functions.
 *
 *  Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv2d_fft.h,v 1.1.1.1 1999/05/06 09:38:04 decoster Exp $
 */

#ifndef _CV2D_FFT_H_
#define _CV2D_FFT_H_

/*
 * Header where the fft functions prototypes are defined.
 */

#include <myFftw2.h>

/*
 * Here are four function pointers that reference the fft functions. Replace
 * any pointers you want.
 */

/*
 * cv2d_fft_r --
 *
 *  This functions computes the fast fourier transform of a real signal.
 *
 * Arguments :
 *   in  - array of real data. Its size is n.
 *   out - allocated array to store the result. Its size is n/2. At the end of
 *         the computation, out[0].imag (wich is 0.0 in theory) must contains
 *         the value of out[n/2].real.
 *   n   - size of array in.
 *
 * Return Value :
 *   None.
 */

void (*cv2d_fft_r)  (real    *in, complex *out, int nx, int ny) = myFftw2_real;

/*
 * cv2d_fft_r_i --
 *
 *  This functions computes the inverse fast fourier transform of a real signal.
 *
 * Arguments :
 *   in  - array of complex data. Its size is n/2. out[0].imag (wich is 0.0 in
 *         theory) must contains the value of out[n/2].real.
 *   out - allocated array to store the result. Its size is n.
 *   n   - size of array out.
 *
 * Return Value :
 *   None.
 */

void (*cv2d_fft_r_i)(complex *in, real    *out, int nx, int ny) = myFftw2I_real;

/*
 * cv2d_fft_c --
 *
 *  This functions computes the fast fourier transform of a complex signal.
 *
 * Arguments :
 *   in  - array of complex data.
 *   out - allocated array to store the result.
 *   n   - size of both arrays.
 *
 * Return Value :
 *   None.
 */

void (*cv2d_fft_c)  (complex *in, complex *out, int nx, int ny) = myFftw2_cplx;

/*
 * cv2d_fft_c_i --
 *
 *  This functions computes the inverse fast fourier transform of a complex
 * signal.
 *
 * Arguments :
 *   in  - array of complex data.
 *   out - allocated array to store the result.
 *   n   - size of both arrays.
 *
 * Return Value :
 *   None.
 */

void (*cv2d_fft_c_i)(complex *in, complex *out, int nx, int ny) = myFftw2I_cplx;


/*
 *
 */

int (*cv2d_is_good_fft_size)(int size) = my_is_good_fftw2_size;


/*
 *
 */

int (*cv2d_next_good_fft_size)(int size) = my_next_good_fftw2_size;


/*
 *
 */

real (*cv2d_fft_get_time)(int size) = my_fftw2_get_time;

/*
 *
 */
real (*cv2d_get_fft_factor)(int nx, int ny) = cv2d_get_fftw_factor;

#endif /* _CV2D_FFT_H_ */
