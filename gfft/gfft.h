/*
 *
 * This file is part of the FFTlib library.  This library is free
 * software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation; version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation; 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
 *
 * $Id: gfft.h,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# ifndef _GFFT_H_
# define _GFFT_H_

# ifndef _REAL_
# define _REAL_
typedef float real;
#endif

# ifndef _COMPLEX_
# define _COMPLEX_

struct complex 
{
    real real;
    real imag;
};

typedef struct complex complex;

#endif

#ifndef M_PI
#define M_PI 3.141592654
#endif

#define COS_PI_4 (real)0.70710678118654752440
#define COS_PI_8 (real)0.92387953251128673848
#define SIN_PI_8 (real)0.38268343236508978178

void
gfft_2pts_real(real *in, complex *out);

void
gifft_2pts_real(complex *in, real *out);


void
gfft_4pts_real(real *in, complex *out);

void
gifft_4pts_real(complex *in, real *out);

void
gfft_8pts_real(real *in, complex *out);
void
gifft_8pts_real(complex *in, real *out);

void
gfft_16pts_real(real *in, complex *out);

void
gifft_16pts_real(complex *in, real *out);

void
gfft_greater_32_real(real *in, complex *out, int size, int log2n);

void
gifft_greater_32_real(complex *in, real *out, int size, int log2n);

void
gfft_greater_16_cplx(complex *in, complex *out,int size, int log2size);

void
gifft_greater_16_cplx(complex *in, complex *out,int size, int log2size);

void
gfft_greater_16_cplx_for_real(complex *in, int size, int log2size);

void
gifft_greater_16_cplx_for_real(complex *in, int size, int log2size);

/* complex */
void
gfft_2pts_cplx(complex *in, complex *out);

void
gifft_2pts_cplx(complex *in, complex *out);

void
gfft_4pts_cplx(complex *in, complex *out);

void
gifft_4pts_cplx(complex *in, complex *out);

void
gfft_8pts_cplx(complex *in, complex *out);

void
gifft_8pts_cplx(complex *in, complex *out);

void
gfft_16pts_cplx(complex *in, complex *out);

void
gifft_16pts_cplx(complex *in, complex *out);

void
gfft_greater_16pts_cplx(complex *in, complex *out);

void
gifft_greater_16pts_cplx(complex *in, complex *out);

void
gfft_shuffle_16_cplx(complex *in);

void
gfft_shuffle_32_cplx(complex *in);

void
gfft_shuffle_64_cplx(complex *in);

void
gfft_shuffle_128_cplx(complex *in);

void
gfft_shuffle_256_cplx(complex *in);

void
gfft_shuffle_512_cplx(complex *in);

void
gfft_shuffle_1024_cplx(complex *in);

void
gfft_shuffle(complex *in, int size, int log2size );

void 
gfft_real(real *in, complex *out, int size);

void
gifft_real(complex *in, real *out, int size);

void
gfft_cplx(complex *in, complex *out, int size);

void
gifft_cplx(complex *in, complex *out, int size);

void
gfft_copy_cplx(complex *in, complex *out, int size);

void
gfft_copy_scale_cplx(complex *in, complex *out, int size, real scale);

#endif /* _GFFT_ */
