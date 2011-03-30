/*
 *
 * This file is part of the Gnu FFT library.  This library is free
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
 * $Id: gfft_greater_32_real.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <math.h>
# include <gfft.h>

void
gfft_greater_32_real(real *in,complex *out,int size, int log2size)
{
    unsigned int i;
    register real even_real, even_imag;
    register real odd_real, odd_imag;
    register real cosinus, sinus;
    register real angle_incr,angle;
    register real next_cosinus,next_sinus;
    register real cosinus_incr,sinus_incr;
    
    int half_size = size >> 1;
    int quater_size = size >> 2;
    
    for (i = 0 ; i < half_size ; i++)
    {
	out[i].real = in[(i << 1)];
	out[i].imag = in[(i << 1) + 1];
    }
    
    gfft_greater_16_cplx_for_real(out, half_size, log2size - 1);
    gfft_shuffle(out,half_size,log2size-1);
    
    angle_incr = angle = 2 * M_PI / size;
    
    cosinus_incr = cos(angle);
    sinus_incr = sin(angle);

    cosinus = cosinus_incr;
    sinus   = -sinus_incr;

    for(i = 1; i <= quater_size ; i++) 
    {

	even_real = out[i].real + out[half_size - i].real;
	even_imag = out[i].imag - out[half_size - i].imag;
	
	odd_real = out[i].real - out[half_size - i].real;
	odd_imag = out[i].imag + out[half_size - i].imag;
	
	out[i].imag = odd_imag * sinus - odd_real * cosinus;
	odd_real =  odd_real * sinus + odd_imag * cosinus;
	odd_imag = out[i].imag;
	
	out[i].real = .5 * (even_real + odd_real);
	out[i].imag = .5 * (even_imag + odd_imag);
	out[half_size - i].real = .5 * (even_real - odd_real);
	out[half_size - i].imag = .5 * (odd_imag - even_imag);

	next_cosinus = cosinus * cosinus_incr + sinus * sinus_incr;
	next_sinus = sinus   * cosinus_incr  - cosinus * sinus_incr;

	cosinus = next_cosinus;
	sinus   = next_sinus;
    }

    even_real = out[0].real + out[0].imag;

    out[0].imag = out[0].real - out[0].imag;
    out[0].real = even_real;
}



