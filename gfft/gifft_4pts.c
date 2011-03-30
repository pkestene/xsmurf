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
 * $Id: gifft_4pts.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

void
gifft_4pts_cplx(complex *in,complex *out)
{

    register real c0_real,c0_imag;
    register real c1_real,c1_imag;
    register real c2_real,c2_imag;
    register real c3_real,c3_imag;
    register real scale = (real).25;
    
    c0_real = in[0].real + in[2].real;
    c0_imag = in[0].imag + in[2].imag;
    c2_real = in[0].real - in[2].real;
    c2_imag = in[0].imag - in[2].imag;
    
    c1_real = in[1].real + in[3].real;
    c1_imag = in[1].imag + in[3].imag;
    c3_real = -(in[1].imag - in[3].imag);
    c3_imag = in[1].real - in[3].real;
        
    out[0].real = (c0_real + c1_real) * scale;
    out[0].imag = (c0_imag + c1_imag) * scale;
    out[2].real = (c0_real - c1_real) * scale;
    out[2].imag = (c0_imag - c1_imag) * scale; 
    
    out[1].real = (c2_real + c3_real) * scale;
    out[1].imag = (c2_imag + c3_imag) * scale;
    out[3].real = (c2_real - c3_real) * scale;
    out[3].imag = (c2_imag - c3_imag) * scale;
}

void
gifft_4pts_real(complex *in,real *out)
{
    register real c0_real;
    register real c1_real;
    register real c2_real;
    register real c3_real;
    register real scale = (real).25;
    
    c0_real = in[0].real + in[0].imag;
    c2_real = in[0].real - in[0].imag;
    c1_real = in[1].real + in[1].real;
    c3_real = -(in[1].imag + in[1].imag);
        
    out[0] = (c0_real + c1_real) * scale;
    out[2] = (c0_real - c1_real) * scale;
    out[1] = (c2_real + c3_real) * scale;
    out[3] = (c2_real - c3_real) * scale;
}

