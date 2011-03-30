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
 * $Id: gifft_2pts.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/
# include <gfft.h>

void 
gifft_2pts_cplx(complex *in, complex *out)
{
    register real scale = (real)0.5;
    
    out[0].real = (in[0].real + in[1].real) * scale;
    out[0].imag = (in[0].imag + in[1].imag) * scale;
    out[1].real = (in[0].real - in[1].real) * scale; 
    out[1].imag = (in[0].imag - in[1].imag) * scale;
}

void 
gifft_2pts_real(complex *in, real *out)
{
    register real scale = (real)0.5;
    
    out[0] = (in[0].real + in[0].imag) * scale;
    out[1] = (in[0].real - in[0].imag) * scale; 
}
