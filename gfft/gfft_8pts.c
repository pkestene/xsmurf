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
 * $Id: gfft_8pts.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/
# include <gfft.h>

void
gfft_8pts_cplx(complex *in,complex *out)
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
    
    register real m0_real,m0_imag;
    register real m1_real,m1_imag;

    c0_real =  in[0].real +  in[4].real;  
    c0_imag =  in[0].imag +  in[4].imag;

    c2_real =  in[2].real +  in[6].real;
    c2_imag =  in[2].imag +  in[6].imag;

    cc0_real =  c0_real +  c2_real;  
    cc0_imag =  c0_imag +  c2_imag;

    cc2_real =  c0_real -  c2_real;  
    cc2_imag =  c0_imag -  c2_imag;

    c4_real =  in[1].real +  in[5].real;  
    c4_imag =  in[1].imag +  in[5].imag;

    c6_real =  in[3].real +  in[7].real;  
    c6_imag =  in[3].imag +  in[7].imag;

    cc4_real =  c4_real +  c6_real;  
    cc4_imag =  c4_imag +  c6_imag;
    
    cc6_real =  c4_real -  c6_real;  
    cc6_imag =  c4_imag -  c6_imag;

    out[0].real =  cc0_real +  cc4_real;  
    out[0].imag =  cc0_imag +  cc4_imag;

    out[2].real =  cc2_real +  cc6_imag;  
    out[2].imag =  cc2_imag -  cc6_real;

    out[4].real =  cc0_real -  cc4_real;  
    out[4].imag =  cc0_imag -  cc4_imag;
    
    out[6].real =  cc2_real -  cc6_imag;  
    out[6].imag =  cc2_imag +  cc6_real;

    c1_real =  in[0].real -  in[4].real;  
    c1_imag =  in[0].imag -  in[4].imag;
    c3_real =  in[2].real -  in[6].real;  
    c3_imag =  in[2].imag -  in[6].imag;

    cc1_real =  c1_real +  c3_imag;  
    cc1_imag =  c1_imag -  c3_real;
    
    cc3_real =  c1_real -  c3_imag;  
    cc3_imag =  c1_imag +  c3_real;

    c5_real =  in[1].real -  in[5].real;  
    c5_imag =  in[1].imag -  in[5].imag;

    c7_real =  in[3].real -  in[7].real;  
    c7_imag =  in[3].imag -  in[7].imag;

    cc5_real =  c5_real +  c7_imag;  
    cc5_imag =  c5_imag -  c7_real;
    
    cc7_real =  c5_real -  c7_imag;  
    cc7_imag =  c5_imag +  c7_real;
    
    m0_real =   COS_PI_4 *  (cc5_real +  cc5_imag);  
    m0_imag =   COS_PI_4 *  (cc5_imag -  cc5_real);
    
    m1_real =   COS_PI_4 *  (-cc7_real +  cc7_imag);  
    m1_imag =   COS_PI_4 *  (-cc7_real -  cc7_imag);
    
    out[1].real =  cc1_real +  m0_real;  
    out[1].imag =  cc1_imag +  m0_imag;
    
    out[3].real =  cc3_real +  m1_real;  
    out[3].imag =  cc3_imag +  m1_imag;
    
    out[5].real =  cc1_real -  m0_real;  
    out[5].imag =  cc1_imag -  m0_imag;
    
    out[7].real =  cc3_real -  m1_real;  
    out[7].imag =  cc3_imag -  m1_imag;
}

void
gfft_8pts_real(real *in,complex *out)
{
    
    register real c0;
    register real c1;
    register real c2;
    register real c3;
    register real c4;
    register real c5;
    register real c6;
    register real c7;

    register real cc0;
    register real cc2;
    register real cc4;
    register real cc6;
    register real cc1_real,cc1_imag;
    register real cc3_real,cc3_imag;
    register real cc5_real,cc5_imag;
    register real cc7_real,cc7_imag;

    register real m0_real,m0_imag;
    register real m1_real,m1_imag;

    
    c0 =  in[0] +  in[4];  
    c2 =  in[2] +  in[6];
    c4 =  in[1] +  in[5];  
    c6 =  in[3] +  in[7];  

    cc0 =  c0 +  c2;  
    cc2 =  c0 -  c2;  
    cc4 =  c4 +  c6;  
    cc6 =  c4 -  c6;  

    out[0].real =  cc0 +  cc4;  
    out[0].imag =  cc0 -  cc4;  
    out[2].real =  cc2;  
    out[2].imag = -cc6;

    c5 =  in[1] -  in[5];  
    c7 =  in[3] -  in[7];  
    cc5_real =   c5;  
    cc5_imag =  -c7;
    cc7_real =  c5;  
    cc7_imag =  c7;

    m0_real =   COS_PI_4 *  (cc5_real +  cc5_imag);  
    m0_imag =   COS_PI_4 *  (cc5_imag -  cc5_real);
    
    m1_real =  -COS_PI_4 *  (cc7_real -  cc7_imag);  
    m1_imag =  -COS_PI_4 *  (cc7_real +  cc7_imag);

    c1 =  in[0] -  in[4];  
    c3 =  in[2] -  in[6];  

    cc1_real =   c1;  
    cc1_imag =  -c3;
    cc3_real =   c1;  
    cc3_imag =   c3;

    out[1].real =  cc1_real +  m0_real;  
    out[1].imag =  cc1_imag +  m0_imag;

    out[3].real =  cc3_real +  m1_real;  
    out[3].imag =  cc3_imag + m1_imag;

}




    
    
    
    




