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
 * $Id: gifft_greater_16_cplx_for_real.c,v 1.1 1999/05/06 14:09:56 decoster Exp $
 *
*/

# include <math.h>
# include <gfft.h>

static void
gifft_16pts_cplx_spec(complex *in);


void 
gifft_greater_16_cplx_for_real(complex *in, int n, int log2n)
{   
    register complex *part = in;
    
    register int size;
    register int current_size;
    register int size_over_8;

    register int step_number;
    register int step;

    register int j,i,k;
    register int part_number;

    register int butterfly_size;
    register int top_butterfly,bottom_butterfly;
    register int top_butterfly_1,bottom_butterfly_1;
    register int top_butterfly_2,bottom_butterfly_2;
    register int top_butterfly_3,bottom_butterfly_3;
    register int top_butterfly_4,bottom_butterfly_4;


    register real rtmp, itmp;
    register real cosinus_incr,sinus_incr;
    register real cos_tmp,sin_tmp;
    register real cosinus,sinus;

    size = n;
    current_size = n;

    cosinus_incr = cos(-2*M_PI/size);
    sinus_incr = sin(-2*M_PI/size);

    step_number = log2n - 4;
    part_number = n >> 4;

    for ( step = 0 ; step < step_number ; step++ )
    {
	
	butterfly_size = current_size >> 1;
	size_over_8 = current_size >> 3;
	    
	cosinus =  (real)1.0;
	sinus   =  (real)0.0;

	for( k = 0 ; k < size ; k += current_size) 
	{
	    

	    /* */
	    top_butterfly    = k;
	    bottom_butterfly = top_butterfly + butterfly_size;
	    
	    rtmp = in[top_butterfly].real - in[bottom_butterfly].real;
	    itmp = in[top_butterfly].imag - in[bottom_butterfly].imag;
	    in[top_butterfly].real += in[bottom_butterfly].real;
	    in[top_butterfly].imag += in[bottom_butterfly].imag;
	    in[bottom_butterfly].real = rtmp;
	    in[bottom_butterfly].imag = itmp;
	    

	    /* */
	    top_butterfly    += size_over_8;
	    bottom_butterfly += size_over_8;

	    rtmp = in[top_butterfly].real - in[bottom_butterfly].real;
	    itmp = in[top_butterfly].imag - in[bottom_butterfly].imag;
	    in[top_butterfly].real += in[bottom_butterfly].real;
	    in[top_butterfly].imag += in[bottom_butterfly].imag;
	    in[bottom_butterfly].real = COS_PI_4 * (rtmp + -itmp);	
	    in[bottom_butterfly].imag = COS_PI_4 * (itmp - -rtmp);


	    /* */
	    top_butterfly    += size_over_8;
	    bottom_butterfly += size_over_8;

	    rtmp = in[top_butterfly].real - in[bottom_butterfly].real;
	    itmp = in[top_butterfly].imag - in[bottom_butterfly].imag;
	    in[top_butterfly].real += in[bottom_butterfly].real;
	    in[top_butterfly].imag += in[bottom_butterfly].imag;
	    in[bottom_butterfly].real =  -itmp;	
	    in[bottom_butterfly].imag = -(-rtmp);


	    /* */
	    top_butterfly    += size_over_8;
	    bottom_butterfly += size_over_8;

	    rtmp = in[top_butterfly].real - in[bottom_butterfly].real;
	    itmp = in[top_butterfly].imag - in[bottom_butterfly].imag;
	    in[top_butterfly].real += in[bottom_butterfly].real;
	    in[top_butterfly].imag += in[bottom_butterfly].imag;
	    in[bottom_butterfly].real = COS_PI_4 * (- rtmp + -itmp);	
	    in[bottom_butterfly].imag = COS_PI_4 * (- itmp - -rtmp);


	}	
	
	cos_tmp   = cosinus;
	sin_tmp = sinus; 
	cosinus = cos_tmp * cosinus_incr - sin_tmp  * sinus_incr;
	sinus   = cos_tmp * sinus_incr + cosinus_incr * sin_tmp;
	
	for(j = 1 ; j < (current_size >> 3) ; j++) 
	{
	    
	    top_butterfly_1 = j;
	    bottom_butterfly_1 = top_butterfly_1 + butterfly_size;
	    
	    top_butterfly_2 = butterfly_size - j;
	    bottom_butterfly_2 = top_butterfly_2 + butterfly_size;

	    top_butterfly_3 = (butterfly_size >> 1) - j;
	    bottom_butterfly_3 = top_butterfly_3 + butterfly_size;

	    top_butterfly_4 = (butterfly_size >> 1) + j;
	    bottom_butterfly_4 = top_butterfly_4 + butterfly_size;
	    
	    for( k = j ; k < size ; k += current_size) 
	    {
		
		/* */
		rtmp = in[top_butterfly_1].real - in[bottom_butterfly_1].real;
		itmp = in[top_butterfly_1].imag - in[bottom_butterfly_1].imag;
		in[top_butterfly_1].real += in[bottom_butterfly_1].real;
		in[top_butterfly_1].imag += in[bottom_butterfly_1].imag;
		in[bottom_butterfly_1].real = (cosinus * rtmp + sinus * itmp);	
		in[bottom_butterfly_1].imag = (cosinus * itmp - sinus * rtmp);
		

		top_butterfly_1 += current_size;
		bottom_butterfly_1 += current_size;

		/* */
		rtmp = in[top_butterfly_2].real - in[bottom_butterfly_2].real;
		itmp = in[top_butterfly_2].imag - in[bottom_butterfly_2].imag;
		in[top_butterfly_2].real += in[bottom_butterfly_2].real;
		in[top_butterfly_2].imag += in[bottom_butterfly_2].imag;
		in[bottom_butterfly_2].real = (-cosinus * rtmp + sinus * itmp);	
		in[bottom_butterfly_2].imag = (-cosinus * itmp - sinus * rtmp);


		top_butterfly_2 += current_size;
		bottom_butterfly_2 += current_size;

		/* */
		rtmp = in[top_butterfly_3].real - in[bottom_butterfly_3].real;
		itmp = in[top_butterfly_3].imag - in[bottom_butterfly_3].imag;
		in[top_butterfly_3].real += in[bottom_butterfly_3].real;
		in[top_butterfly_3].imag += in[bottom_butterfly_3].imag;
		in[bottom_butterfly_3].real = (-sinus * rtmp + -cosinus * itmp);	
		in[bottom_butterfly_3].imag = (-sinus * itmp - -cosinus * rtmp);
		

		top_butterfly_3 += current_size;
		bottom_butterfly_3 += current_size;

		/* */
		rtmp = in[top_butterfly_4].real - in[bottom_butterfly_4].real;
		itmp = in[top_butterfly_4].imag - in[bottom_butterfly_4].imag;
		in[top_butterfly_4].real += in[bottom_butterfly_4].real;
		in[top_butterfly_4].imag += in[bottom_butterfly_4].imag;
		in[bottom_butterfly_4].real = (sinus * rtmp + -cosinus * itmp);	
		in[bottom_butterfly_4].imag = (sinus * itmp - -cosinus * rtmp);
		

		top_butterfly_4 += current_size;
		bottom_butterfly_4 += current_size;
		
	    }

	    cos_tmp   = cosinus;
	    sin_tmp = sinus; 
	    cosinus = cos_tmp * cosinus_incr - sin_tmp  * sinus_incr;
	    sinus   = cos_tmp * sinus_incr + cosinus_incr * sin_tmp;
	}

	cos_tmp = cosinus_incr;
	sin_tmp = sinus_incr; 
	
	cosinus_incr = cos_tmp * cos_tmp - sin_tmp * sin_tmp;
	sinus_incr   = cos_tmp * sin_tmp + cos_tmp * sin_tmp;

	current_size >>= 1;
    }
    
    for ( i = 0 ; i < part_number ; i++)
    {
	gifft_16pts_cplx_spec(part);
	part += 16;
    }
}


static void
gifft_16pts_cplx_spec(complex *in)
{
    register real diff_0_real,diff_0_imag;
    register real diff_1_real,diff_1_imag;
    register real diff_2_real,diff_2_imag;
    register real diff_3_real,diff_3_imag;
    register real diff_4_real,diff_4_imag;
    register real diff_5_real,diff_5_imag;
    register real diff_6_real,diff_6_imag;
    register real diff_7_real,diff_7_imag;
    register real diff_8_real,diff_8_imag;
    register real diff_9_real,diff_9_imag;

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

    
    /* etape 1 */

    c0_real = in[0].real + in[8].real;
    c0_imag = in[0].imag + in[8].imag;
    c8_real = in[0].real - in[8].real;
    c8_imag = in[0].imag - in[8].imag;
    
    diff_0_real = in[1].real - in[9].real;
    diff_0_imag = in[1].imag - in[9].imag;
    c1_real = in[1].real + in[9].real;
    c1_imag = in[1].imag + in[9].imag;
    c9_real = (COS_PI_8 * diff_0_real - SIN_PI_8 * diff_0_imag);
    c9_imag = (COS_PI_8 * diff_0_imag + SIN_PI_8 * diff_0_real);
    
    diff_1_real = in[2].real - in[10].real;
    diff_1_imag = in[2].imag - in[10].imag;
    c2_real = in[2].real + in[10].real;
    c2_imag = in[2].imag + in[10].imag;
    c10_real = COS_PI_4 * (diff_1_real - diff_1_imag);
    c10_imag = COS_PI_4 * (diff_1_imag + diff_1_real);
    
    diff_2_real = in[3].real - in[11].real;
    diff_2_imag = in[3].imag - in[11].imag;
    c3_real  = in[3].real + in[11].real;
    c3_imag  = in[3].imag + in[11].imag;
    c11_real = (SIN_PI_8 * diff_2_real - COS_PI_8 * diff_2_imag);
    c11_imag = (SIN_PI_8 * diff_2_imag + COS_PI_8 * diff_2_real);
    
    c4_real = in[4].real + in[12].real;
    c4_imag = in[4].imag + in[12].imag;
    c12_real = -in[4].imag + in[12].imag;
    c12_imag =  in[4].real - in[12].real;
    
    diff_3_real = in[5].real - in[13].real;
    diff_3_imag = in[5].imag - in[13].imag;
    c5_real = in[5].real + in[13].real;
    c5_imag = in[5].imag + in[13].imag;
    c13_real = ( -SIN_PI_8 * diff_3_real - COS_PI_8 * diff_3_imag);
    c13_imag = ( -SIN_PI_8 * diff_3_imag + COS_PI_8 * diff_3_real);
    
    diff_4_real = in[6].real - in[14].real;
    diff_4_imag = in[6].imag - in[14].imag;
    c6_real = in[6].real + in[14].real;
    c6_imag = in[6].imag + in[14].imag;
    c14_real =  -COS_PI_4 * (diff_4_real + diff_4_imag);
    c14_imag =  -COS_PI_4 * (diff_4_imag  - diff_4_real);
    
    diff_5_real = in[7].real - in[15].real;
    diff_5_imag = in[7].imag - in[15].imag;
    c7_real = in[7].real + in[15].real;
    c7_imag = in[7].imag + in[15].imag;
    c15_real = ( -COS_PI_8 * diff_5_real - SIN_PI_8 * diff_5_imag);
    c15_imag = ( -COS_PI_8 * diff_5_imag + SIN_PI_8 * diff_5_real);
    
    
    /* etape 2 */
    cc0_real = c0_real + c4_real;
    cc0_imag = c0_imag + c4_imag;
    cc4_real = c0_real - c4_real;
    cc4_imag = c0_imag - c4_imag;
    
    cc8_real = c8_real + c12_real;
    cc8_imag = c8_imag + c12_imag;
    cc12_real = c8_real - c12_real;
    cc12_imag = c8_imag - c12_imag;
    
    diff_6_real = c1_real - c5_real;
    diff_6_imag = c1_imag - c5_imag;
    cc1_real = c1_real + c5_real;
    cc1_imag = c1_imag + c5_imag;
    cc5_real =  COS_PI_4 * (diff_6_real - diff_6_imag);
    cc5_imag =  COS_PI_4 * (diff_6_imag + diff_6_real);
    
    diff_7_real = c9_real - c13_real;
    diff_7_imag = c9_imag - c13_imag;
    cc9_real = c9_real + c13_real;
    cc9_imag = c9_imag + c13_imag;
    cc13_real =  COS_PI_4 * (diff_7_real - diff_7_imag);
    cc13_imag =  COS_PI_4 * (diff_7_imag + diff_7_real);
    
    
    cc2_real = c2_real + c6_real;
    cc2_imag = c2_imag + c6_imag;
    cc6_real = -(c2_imag - c6_imag);
    cc6_imag = c2_real - c6_real;
    
    cc10_real = c10_real + c14_real;
    cc10_imag = c10_imag + c14_imag;
    cc14_real = -(c10_imag - c14_imag);
    cc14_imag = c10_real - c14_real;
    
    diff_8_real = c3_real - c7_real;
    diff_8_imag = c3_imag - c7_imag;
    cc3_real = c3_real + c7_real;
    cc3_imag = c3_imag + c7_imag;
    cc7_real =  -COS_PI_4 * (diff_8_real + diff_8_imag);
    cc7_imag =  -COS_PI_4 * (diff_8_imag - diff_8_real);
    
    diff_9_real = c11_real - c15_real;
    diff_9_imag = c11_imag - c15_imag;
    cc11_real = c11_real + c15_real;
    cc11_imag = c11_imag + c15_imag;
    cc15_real =  -COS_PI_4 * (diff_9_real + diff_9_imag);
    cc15_imag =  -COS_PI_4 * (diff_9_imag - diff_9_real);
    
    
    /* etape 3 */
    
    ccc0_real = cc0_real + cc2_real;
    ccc0_imag = cc0_imag + cc2_imag;
    ccc2_real = cc0_real - cc2_real;
    ccc2_imag = cc0_imag - cc2_imag;
    
    ccc4_real = cc4_real + cc6_real;
    ccc4_imag = cc4_imag + cc6_imag;
    ccc6_real = cc4_real - cc6_real;
    ccc6_imag = cc4_imag - cc6_imag;
    
    ccc8_real = cc8_real + cc10_real;
    ccc8_imag = cc8_imag + cc10_imag;
    ccc10_real = cc8_real - cc10_real;
    ccc10_imag = cc8_imag - cc10_imag;
    
    ccc12_real = cc12_real + cc14_real;
    ccc12_imag = cc12_imag + cc14_imag;
    ccc14_real = cc12_real - cc14_real;
    ccc14_imag = cc12_imag - cc14_imag;
    
    
    ccc1_real = cc1_real + cc3_real;
    ccc1_imag = cc1_imag + cc3_imag;
    ccc3_real = -(cc1_imag - cc3_imag);
    ccc3_imag = cc1_real - cc3_real;
    
    ccc5_real = cc5_real + cc7_real;
    ccc5_imag = cc5_imag + cc7_imag;
    ccc7_real = -(cc5_imag - cc7_imag);
    ccc7_imag = cc5_real - cc7_real;
    
    ccc9_real = cc9_real + cc11_real;
    ccc9_imag = cc9_imag + cc11_imag;
    ccc11_real = -(cc9_imag - cc11_imag);
    ccc11_imag = cc9_real - cc11_real;
    
    ccc13_real = cc13_real + cc15_real;
    ccc13_imag = cc13_imag + cc15_imag;
    ccc15_real = -(cc13_imag - cc15_imag);
    ccc15_imag = cc13_real - cc15_real;
    
    
    /* etape 4 */
    
    in[0].real = ccc0_real + ccc1_real;
    in[0].imag = ccc0_imag + ccc1_imag;
    in[1].real = ccc0_real - ccc1_real;
    in[1].imag = ccc0_imag - ccc1_imag;
    
    in[2].real = ccc2_real + ccc3_real;
    in[2].imag = ccc2_imag + ccc3_imag;
    in[3].real = ccc2_real - ccc3_real;
    in[3].imag = ccc2_imag - ccc3_imag;

    in[4].real = ccc4_real + ccc5_real;
    in[4].imag = ccc4_imag + ccc5_imag;
    in[5].real = ccc4_real - ccc5_real;
    in[5].imag = ccc4_imag - ccc5_imag;

    in[6].real = ccc6_real + ccc7_real;
    in[6].imag = ccc6_imag + ccc7_imag;
    in[7].real = ccc6_real - ccc7_real;
    in[7].imag = ccc6_imag - ccc7_imag;

    in[8].real = ccc8_real + ccc9_real;
    in[8].imag = ccc8_imag + ccc9_imag;
    in[9].real = ccc8_real - ccc9_real;
    in[9].imag = ccc8_imag - ccc9_imag;

    in[10].real = ccc10_real + ccc11_real;
    in[10].imag = ccc10_imag + ccc11_imag;
    in[11].real = ccc10_real - ccc11_real;
    in[11].imag = ccc10_imag - ccc11_imag;

    in[12].real = ccc12_real + ccc13_real;
    in[12].imag = ccc12_imag + ccc13_imag;
    in[13].real = ccc12_real - ccc13_real;
    in[13].imag = ccc12_imag - ccc13_imag;

    in[14].real = ccc14_real + ccc15_real;
    in[14].imag = ccc14_imag + ccc15_imag;
    in[15].real = ccc14_real - ccc15_real;
    in[15].imag = ccc14_imag - ccc15_imag;

}
