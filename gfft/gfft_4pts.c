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
 * $Id: gfft_4pts.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

void
gfft_4pts_cplx(complex *in, complex *out)
{
    register real c0_real;
    register real c0_imag;
    register real c1_real;
    register real c1_imag;
    register real c2_real;
    register real c2_imag;
    register real c3_real;
    register real c3_imag;
    
    c0_real = in[0].real + in[2].real;
    c0_imag = in[0].imag + in[2].imag;
    c1_real = in[0].real - in[2].real;
    c1_imag = in[0].imag - in[2].imag;
    c2_real = in[1].real + in[3].real;
    c2_imag = in[1].imag + in[3].imag;
    c3_real = in[1].real - in[3].real;
    c3_imag = in[1].imag - in[3].imag;

    out[0].real = c0_real + c2_real;
    out[0].imag = c0_imag + c2_imag;
    out[2].real = c0_real - c2_real;
    out[2].imag = c0_imag - c2_imag;
    out[1].real = c1_real + c3_imag;
    out[1].imag = c1_imag - c3_real;
    out[3].real = c1_real - c3_imag;
    out[3].imag = c1_imag + c3_real;
}

void
gfft_4pts_real(real *in, complex *out)
{
    register real c0_real;
    register real c1_real;
    register real c2_real;
    register real c3_real;
    
    c0_real = in[0] + in[2];
    c1_real = in[0] - in[2];
    c2_real = in[1] + in[3];
    c3_real = in[1] - in[3];

    out[0].real = c0_real + c2_real;
    out[0].imag = c0_real - c2_real;
    out[1].real = c1_real;
    out[1].imag = -c3_real;
}
