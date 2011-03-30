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
 * $Id: gifft_greater_32_real.c,v 1.1 1999/05/06 14:09:56 decoster Exp $
 *
*/
# include <math.h>
# include <gfft.h>

void
gifft_greater_32_real(complex *in, real *out, int size,int log2n)
{
    int i;
    register int half_size;
    int quater_size;
    register real even_real, even_imag;
    register real odd_real, odd_imag;
    register real angle_incr,angle;
    register real cosinus, sinus;
    register real next_cosinus,next_sinus;
    register real cosinus_incr,sinus_incr;
    register real scale;
    register real tmp;
    
    register complex *c_out = (complex *)out;

    angle_incr = angle = 2 * M_PI / size;
    
    cosinus_incr = cos(angle);
    sinus_incr   = sin(angle);

    cosinus = cosinus_incr;
    sinus   = sinus_incr;

    half_size = size >> 1;
    quater_size = size >> 2;
    scale = 1.0 / (real)size;
    
    for(i = 1; i <= quater_size ; i++) 
    {
	even_real = in[i].real + in[half_size - i].real;
	even_imag = in[i].imag - in[half_size - i].imag;
	
	odd_real = in[i].real - in[half_size - i].real;
	odd_imag = in[i].imag + in[half_size - i].imag;
	
	tmp = odd_imag * sinus - odd_real * cosinus;
	odd_real =  odd_real * sinus + odd_imag * cosinus;
	odd_imag = tmp;
	
	c_out[i].real = scale * (even_real - odd_real);
	c_out[i].imag = scale * (even_imag - odd_imag);
	c_out[half_size - i].real = scale * (even_real + odd_real);
	c_out[half_size - i].imag = scale * (-odd_imag - even_imag);

	next_cosinus = cosinus * cosinus_incr - sinus * sinus_incr;
	next_sinus = sinus   * cosinus_incr  + cosinus * sinus_incr;

	cosinus = next_cosinus;
	sinus   = next_sinus;
    }

    even_real = scale * (in[0].real + in[0].imag);
    c_out[0].imag = scale *(in[0].real - in[0].imag);
    c_out[0].real = even_real;

    gifft_greater_16_cplx_for_real(c_out,half_size,log2n-1);
    gfft_shuffle(c_out,half_size,log2n-1);
    
}
