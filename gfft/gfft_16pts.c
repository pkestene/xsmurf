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
 * $Id: gfft_16pts.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/
# include <gfft.h>

void
gfft_16pts_cplx(complex *in,complex *out)
{

    register real c0_real,c0_imag;
    register real c1_real,c1_imag;
    register real c2_real,c2_imag;
    register real c3_real,c3_imag;
    register real c4_real,c4_imag;
    register real c5_real,c5_imag;
    register real c6_real,c6_imag;
    register real c7_real,c7_imag;
    register real c8_real,c8_imag;
    register real c9_real,c9_imag;
    register real c10_real,c10_imag;
    register real c11_real,c11_imag;
    register real c12_real,c12_imag;
    register real c13_real,c13_imag;
    register real c14_real,c14_imag;
    register real c15_real,c15_imag;

    register real cc0_real,cc0_imag;
    register real cc1_real,cc1_imag;
    register real cc2_real,cc2_imag;
    register real cc3_real,cc3_imag;
    register real cc4_real,cc4_imag;
    register real cc5_real,cc5_imag;
    register real cc6_real,cc6_imag;
    register real cc7_real,cc7_imag;
    register real cc8_real,cc8_imag;
    register real cc9_real,cc9_imag;
    register real cc10_real,cc10_imag;
    register real cc11_real,cc11_imag;
    register real cc12_real,cc12_imag;
    register real cc13_real,cc13_imag;
    register real cc14_real,cc14_imag;
    register real cc15_real,cc15_imag;

    register real ccc0_real,ccc0_imag;
    register real ccc1_real,ccc1_imag;
    register real ccc2_real,ccc2_imag;
    register real ccc3_real,ccc3_imag;
    register real ccc4_real,ccc4_imag;
    register real ccc5_real,ccc5_imag;
    register real ccc6_real,ccc6_imag;
    register real ccc7_real,ccc7_imag;
    register real ccc8_real,ccc8_imag;
    register real ccc9_real,ccc9_imag;
    register real ccc10_real,ccc10_imag;
    register real ccc11_real,ccc11_imag;
    register real ccc12_real,ccc12_imag;
    register real ccc13_real,ccc13_imag;
    register real ccc14_real,ccc14_imag;
    register real ccc15_real,ccc15_imag;
    
    register real m0_real,m0_imag;
    register real m1_real,m1_imag;
    register real m2_real,m2_imag;
    register real m3_real,m3_imag;
    register real m4_real,m4_imag;
    register real m5_real,m5_imag;
    register real m6_real,m6_imag;
    register real m7_real,m7_imag;
    register real m8_real,m8_imag;
    register real m9_real,m9_imag;


    c0_real = in[0].real + in[8].real;
    c0_imag = in[0].imag + in[8].imag ;
    
    c2_real = in[4].real + in[12].real;
    c2_imag = in[4].imag + in[12].imag ;

    cc0_real = c0_real + c2_real;
    cc0_imag = c0_imag + c2_imag ;
    
    cc2_real = c0_real - c2_real;
    cc2_imag = c0_imag - c2_imag ;
    
    c4_real = in[2].real + in[10].real;
    c4_imag = in[2].imag + in[10].imag ;

    c6_real = in[6].real + in[14].real;
    c6_imag = in[6].imag + in[14].imag ;

    cc4_real = c4_real + c6_real;
    cc4_imag = c4_imag + c6_imag ;

    cc6_real = c4_real - c6_real;
    cc6_imag = c4_imag - c6_imag ;

    ccc0_real = cc0_real + cc4_real;
    ccc0_imag = cc0_imag + cc4_imag ;

    ccc4_real = cc0_real - cc4_real;
    ccc4_imag = cc0_imag - cc4_imag ;

    ccc2_real = cc2_real + cc6_imag ;
    ccc2_imag = cc2_imag - cc6_real ;
    
    ccc6_real = cc2_real - cc6_imag ;
    ccc6_imag = cc2_imag + cc6_real ;

    c8_real = in[1].real + in[9].real;
    c8_imag = in[1].imag + in[9].imag ;
    
    c10_real = in[5].real + in[13].real;
    c10_imag = in[5].imag + in[13].imag ;

    c12_real = in[3].real + in[11].real;
    c12_imag = in[3].imag + in[11].imag ;
    
    c14_real = in[7].real + in[15].real;
    c14_imag = in[7].imag + in[15].imag ;

    cc8_real = c8_real + c10_real;
    cc8_imag = c8_imag + c10_imag ;

    cc10_real = c8_real - c10_real;
    cc10_imag = c8_imag - c10_imag ;

    cc12_real = c12_real + c14_real;
    cc12_imag = c12_imag + c14_imag ;

    cc14_real = c12_real - c14_real;
    cc14_imag = c12_imag - c14_imag ;
    
    ccc8_real = cc8_real + cc12_real;
    ccc8_imag = cc8_imag + cc12_imag ;

    ccc12_real = cc8_real - cc12_real;
    ccc12_imag = cc8_imag - cc12_imag ;

    ccc10_real = cc10_real + cc14_imag ;
    ccc10_imag = cc10_imag - cc14_real ;

    ccc14_real = cc10_real - cc14_imag ;
    ccc14_imag = cc10_imag + cc14_real ;

    out[0].real = ccc0_real + ccc8_real;
    out[0].imag = ccc0_imag + ccc8_imag ;
    
    out[8].real = ccc0_real - ccc8_real;
    out[8].imag = ccc0_imag - ccc8_imag ;

    m4_real =  COS_PI_4 * ( ccc10_real + ccc10_imag ) ;
    m4_imag =  COS_PI_4 * ( ccc10_imag - ccc10_real ) ;

    out[2].real = ccc2_real + m4_real;
    out[2].imag = ccc2_imag + m4_imag ;

    out[10].real = ccc2_real - m4_real;
    out[10].imag = ccc2_imag - m4_imag ;

    out[4].real = ccc4_real + ccc12_imag ;
    out[4].imag = ccc4_imag - ccc12_real ;

    out[12].real = ccc4_real - ccc12_imag ;
    out[12].imag = ccc4_imag + ccc12_real ;

    m5_real =  -COS_PI_4 * ( ccc14_real - ccc14_imag ) ;
    m5_imag =  -COS_PI_4 * ( ccc14_real + ccc14_imag ) ;

    out[6].real = ccc6_real + m5_real;
    out[6].imag = ccc6_imag + m5_imag ;
    
    out[14].real = ccc6_real - m5_real;
    out[14].imag = ccc6_imag - m5_imag ;

    c1_real = in[0].real - in[8].real;
    c1_imag = in[0].imag - in[8].imag ;
    
    c3_real = in[4].real - in[12].real;
    c3_imag = in[4].imag - in[12].imag ;

    cc1_real = c1_real + c3_imag ;
    cc1_imag = c1_imag - c3_real ;
    
    cc3_real = c1_real - c3_imag ;
    cc3_imag = c1_imag + c3_real ;
    
    c5_real = in[2].real - in[10].real;
    c5_imag = in[2].imag - in[10].imag ;

    c7_real = in[6].real - in[14].real;
    c7_imag = in[6].imag - in[14].imag ;

    cc5_real = c5_real + c7_imag ;
    cc5_imag = c5_imag - c7_real ;

    cc7_real = c5_real - c7_imag ;
    cc7_imag = c5_imag + c7_real ;

    m0_real =  COS_PI_4 * ( cc5_real + cc5_imag ) ;
    m0_imag =  COS_PI_4 * ( cc5_imag - cc5_real ) ;
    ccc1_real = cc1_real + m0_real;
    ccc1_imag = cc1_imag + m0_imag ;

    ccc5_real = cc1_real - m0_real;
    ccc5_imag = cc1_imag - m0_imag ;

    m1_real =  -COS_PI_4 * ( cc7_real - cc7_imag ) ;
    m1_imag =  -COS_PI_4 * ( cc7_real + cc7_imag ) ;
    ccc3_real = cc3_real + m1_real;
    ccc3_imag = cc3_imag + m1_imag ;
    
    ccc7_real = cc3_real - m1_real;
    ccc7_imag = cc3_imag - m1_imag ;
    
    c9_real = in[1].real - in[9].real;
    c9_imag = in[1].imag - in[9].imag ;
    
    c11_real = in[5].real - in[13].real;
    c11_imag = in[5].imag - in[13].imag ;

    c13_real = in[3].real - in[11].real;
    c13_imag = in[3].imag - in[11].imag ;

    c15_real = in[7].real - in[15].real;
    c15_imag = in[7].imag - in[15].imag ;

    cc9_real = c9_real + c11_imag ;
    cc9_imag = c9_imag - c11_real ;

    cc11_real = c9_real - c11_imag ;
    cc11_imag = c9_imag + c11_real ;
    
    cc13_real = c13_real + c15_imag ;
    cc13_imag = c13_imag - c15_real ;
    
    cc15_real = c13_real - c15_imag ;
    cc15_imag = c13_imag + c15_real ;

    m2_real =  COS_PI_4 * ( cc13_real + cc13_imag ) ;
    m2_imag =  COS_PI_4 * ( cc13_imag - cc13_real ) ;

    ccc9_real = cc9_real + m2_real;
    ccc9_imag = cc9_imag + m2_imag ;

    ccc13_real = cc9_real - m2_real;
    ccc13_imag = cc9_imag - m2_imag ;

    m3_real =  -COS_PI_4 * ( cc15_real - cc15_imag ) ;
    m3_imag =  -COS_PI_4 * ( cc15_real + cc15_imag ) ;

    ccc11_real = cc11_real + m3_real;
    ccc11_imag = cc11_imag + m3_imag ;

    ccc15_real = cc11_real - m3_real;
    ccc15_imag = cc11_imag - m3_imag ;
    
    m6_real = ccc9_real *  COS_PI_8 - ccc9_imag * -SIN_PI_8;
    m6_imag = ccc9_real * -SIN_PI_8 + ccc9_imag *  COS_PI_8;

    out[1].real = ccc1_real + m6_real;
    out[1].imag = ccc1_imag + m6_imag ;
    
    out[9].real = ccc1_real - m6_real;
    out[9].imag = ccc1_imag - m6_imag ;

    m7_real = ccc11_real *  SIN_PI_8 - ccc11_imag * -COS_PI_8;
    m7_imag = ccc11_real * -COS_PI_8 + ccc11_imag *  SIN_PI_8;

    out[3].real = ccc3_real + m7_real;
    out[3].imag = ccc3_imag + m7_imag ;

    out[11].real = ccc3_real - m7_real;
    out[11].imag = ccc3_imag - m7_imag ;

    m8_real = ccc13_real * -SIN_PI_8 - ccc13_imag * -COS_PI_8;
    m8_imag = ccc13_real * -COS_PI_8 + ccc13_imag * -SIN_PI_8;

    out[5].real = ccc5_real + m8_real;
    out[5].imag = ccc5_imag + m8_imag ;

    out[13].real = ccc5_real - m8_real;
    out[13].imag = ccc5_imag - m8_imag ;

    m9_real = ccc15_real * -COS_PI_8 - ccc15_imag * -SIN_PI_8;
    m9_imag = ccc15_real * -SIN_PI_8 + ccc15_imag * -COS_PI_8;

    out[7].real = ccc7_real + m9_real;
    out[7].imag = ccc7_imag + m9_imag ;

    out[15].real = ccc7_real - m9_real;
    out[15].imag = ccc7_imag - m9_imag ;
}

void
gfft_16pts_real(real *in, complex *out)
{       

    real t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
    real t11,t12,t13,t14,t15,t16,t17,t18,t19,t20;
    real t21,t22,t23,t24,t25,t26;
    real m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,m10;
    real m11,m12,m13,m14,m15,m16,m17;

    real s1,s2,s3,s4,s5,s6,s7,s8,s9,s10;
    real s11,s12,s13,s14,s15,s16,s17,s18,s19,s20;
    
    t1  = in[0] + in[8];
    t2  = in[4] + in[12];
    t3  = in[2] + in[10];
    t4  = in[2] - in[10];
    t5  = in[6] + in[14];
    t6  = in[6] - in[14];
    t7  = in[1] + in[9];
    t8  = in[1] - in[9];
    t9  = in[3] + in[11];
    t10 = in[3] - in[11];
    t11 = in[5] + in[13];
    t12 = in[5] - in[13];
    t13 = in[7] + in[15];
    t14 = in[7] - in[15];
    t15 = t1 + t2;
    t16 = t3 + t5;
    t17 = t15 + t16;
    t18 = t7 + t11;
    t19 = t7 - t11;
    t20 = t9 + t13;
    t21 = t9 - t13;
    t22 = t18 + t20;
    t23 = t8 + t14;
    t24 = t8 - t14;
    t25 = t10 + t12;
    t26 = t12 - t10;
    m0 = t17 + t22;
    m1 = t17 - t22;
    m2 = t15 - t16;
    m3 = t1 - t2;
    m4 = in[0] - in[8];
    m5 = (real)( 0.7071067811865476)  * (t19 - t21);
    m6 = (real)( 0.7071067811865476)  * (t4  - t6);
    m7 = (real)( 0.3826834323650898)  * (t24 + t26);
    m8 = (real)( 1.3065629648763766)  * t24;
    m9 = (real)(-0.5411961001461969)  * t26;
    s7 = m8 - m7;
    s8 = m9 - m7;
    m10 = t20 - t18;
    m11 = t5 - t3;
    m12 = in[12] - in[4];
    m13 = -(real)(0.7071067811865475) * (t19 + t21);
    m14 = -(real)(0.7071067811865475) * (t4  + t6);
    m15 = -(real)(0.9238795325112867) * (t23 + t25);
    m16 =  (real)(0.5411961001461969) * t23;
    m17 = -(real)(1.3065629648763766) * t25;
    s15 = m15 + m16;
    s16 = m15 - m17;
    s1  = m3 + m5;
    s2  = m3 - m5;
    s3  = m11 + m13;
    s4  = m13 - m11;
    s5  = m4 + m6;
    s6  = m4 - m6;
    s9  = s5 + s7;
    s10 = s5 - s7;
    s11 = s6 + s8;
    s12 = s6 - s8;
    s13 = m12 + m14;
    s14 = m12 - m14;
    s17 = s13 + s15;
    s18 = s13 - s15;
    s19 = s14 + s16;
    s20 = s14 - s16;

    out[0].real = m0;
    out[0].imag = m1;
    out[1].real = s9;
    out[1].imag = s17;
    out[2].real = s1;
    out[2].imag = s3;
    out[3].real = s12;
    out[3].imag = -s20;
    out[4].real = m2;
    out[4].imag = m10;
    out[5].real = s11;
    out[5].imag = s19;
    out[6].real = s2;
    out[6].imag = s4;
    out[7].real = s10;
    out[7].imag = -s18;
}       



