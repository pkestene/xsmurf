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
 * $Id: gifft_8pts.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

void
gifft_8pts_cplx(complex *in,complex *out)
{
    register real c0_real,c0_imag;
    register real c1_real,c1_imag;
    register real c2_real,c2_imag;
    register real c3_real,c3_imag;
    register real c4_real,c4_imag;
    register real c5_real,c5_imag;
    register real c6_real,c6_imag;
    register real c7_real,c7_imag;

    register real cc0_real,cc0_imag;
    register real cc1_real,cc1_imag;
    register real cc2_real,cc2_imag;
    register real cc3_real,cc3_imag;
    register real cc4_real,cc4_imag;
    register real cc5_real,cc5_imag;
    register real cc6_real,cc6_imag;
    register real cc7_real,cc7_imag;

    register real diff_0_real,diff_0_imag;
    register real diff_1_real,diff_1_imag;
    
    register real scale = (real)0.125;

    c0_real = in[0].real + in[4].real;
    c0_imag = in[0].imag + in[4].imag;
    c2_real = in[2].real + in[6].real;
    c2_imag = in[2].imag + in[6].imag;
    cc0_real = c0_real + c2_real;
    cc0_imag = c0_imag + c2_imag;
    cc2_real = c0_real - c2_real;
    cc2_imag = c0_imag - c2_imag;

    c4_real = in[0].real - in[4].real; 
    c4_imag = in[0].imag - in[4].imag; 
    c6_real =  -(in[2].imag - in[6].imag);
    c6_imag =  in[2].real - in[6].real;
    cc4_real = c4_real + c6_real;
    cc4_imag = c4_imag + c6_imag;
    cc6_real = c4_real - c6_real;
    cc6_imag = c4_imag - c6_imag;

    c1_real = in[1].real + in[5].real;
    c1_imag = in[1].imag + in[5].imag;
    diff_0_real = in[1].real - in[5].real;
    diff_0_imag = in[1].imag - in[5].imag;
    c5_real = COS_PI_4 * (diff_0_real - diff_0_imag);
    c5_imag = COS_PI_4 * (diff_0_imag + diff_0_real);

    c3_real = in[3].real + in[7].real;
    c3_imag = in[3].imag + in[7].imag;
    diff_1_real = in[3].real - in[7].real;
    diff_1_imag = in[3].imag - in[7].imag;
    c7_real =  -COS_PI_4 * (diff_1_real + diff_1_imag);
    c7_imag =  -COS_PI_4 * (diff_1_imag - diff_1_real);

    cc1_real = c1_real + c3_real;
    cc1_imag = c1_imag + c3_imag;
    cc3_real =  -(c1_imag - c3_imag);
    cc3_imag =  c1_real - c3_real;

    cc5_real = c5_real + c7_real;
    cc5_imag = c5_imag + c7_imag;
    cc7_real =  -(c5_imag - c7_imag);
    cc7_imag =  c5_real - c7_real;

    out[0].real = (cc0_real + cc1_real) * scale;
    out[0].imag = (cc0_imag + cc1_imag) * scale;
    out[4].real = (cc0_real - cc1_real) * scale;
    out[4].imag = (cc0_imag - cc1_imag) * scale;
    out[2].real = (cc2_real + cc3_real) * scale;
    out[2].imag = (cc2_imag + cc3_imag) * scale;
    out[6].real = (cc2_real - cc3_real) * scale;
    out[6].imag = (cc2_imag - cc3_imag) * scale;
    out[1].real = (cc4_real + cc5_real) * scale;
    out[1].imag = (cc4_imag + cc5_imag) * scale;
    out[5].real = (cc4_real - cc5_real) * scale;
    out[5].imag = (cc4_imag - cc5_imag) * scale;
    out[3].real = (cc6_real + cc7_real) * scale;
    out[3].imag = (cc6_imag + cc7_imag) * scale;
    out[7].real = (cc6_real - cc7_real) * scale;
    out[7].imag = (cc6_imag - cc7_imag) * scale;

}

void
gifft_8pts_real(complex *in,real *out)
{

    register real c0_real;
    register real c1_real,c1_imag;
    register real c2_real;
    register real c4_real;
    register real c5_real,c5_imag;
    register real c6_real;

    register real cc0_real;
    register real cc1_real;
    register real cc2_real;
    register real cc3_real;
    register real cc4_real;
    register real cc5_real;
    register real cc6_real;
    register real cc7_real;

    register real diff_0_real,diff_0_imag;
    
    register real scale = (real)0.125;

    c0_real = in[0].real + in[0].imag;
    c4_real = in[0].real - in[0].imag; 

    diff_0_real = in[1].real - in[3].real;
    diff_0_imag = in[1].imag + in[3].imag;
    c1_real = in[1].real + in[3].real;
    c1_imag = in[1].imag - in[3].imag;
    c5_real = COS_PI_4 * (diff_0_real - diff_0_imag);
    c5_imag = COS_PI_4 * (diff_0_imag + diff_0_real);

    c2_real = in[2].real + in[2].real;
    c6_real =  -(in[2].imag + in[2].imag);

    cc0_real = c0_real + c2_real;
    cc2_real = c0_real - c2_real;
    cc4_real = c4_real + c6_real;
    cc6_real = c4_real - c6_real;
    cc1_real = c1_real + c1_real;
    cc3_real =  -(c1_imag + c1_imag);
    cc5_real = c5_real + c5_real;
    cc7_real =  -(c5_imag + c5_imag);

    out[0] = (cc0_real + cc1_real) * scale;
    out[4] = (cc0_real - cc1_real) * scale;
    out[2] = (cc2_real + cc3_real) * scale;
    out[6] = (cc2_real - cc3_real) * scale;
    out[1] = (cc4_real + cc5_real) * scale;
    out[5] = (cc4_real - cc5_real) * scale;
    out[3] = (cc6_real + cc7_real) * scale;
    out[7] = (cc6_real - cc7_real) * scale;

}
