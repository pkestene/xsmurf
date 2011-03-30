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
 * $Id: gfft_shuffle.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

static void
gfft_shuffle_gen_cplx(complex *in,int size);

void
gfft_shuffle(complex *in, int size, int log2size)
{
    switch(log2size)
    {
      case 4:
	gfft_shuffle_16_cplx(in);
	break;
      case 5:
	gfft_shuffle_32_cplx(in);
	break;
      case 6:
	gfft_shuffle_64_cplx(in);
	break;
      case 7:
	gfft_shuffle_128_cplx(in);
	break;
      case 8:
	gfft_shuffle_256_cplx(in);
	break;
      case 9:
	gfft_shuffle_512_cplx(in);
	break;
      case 10:
	gfft_shuffle_1024_cplx(in);
	break;
      default:
	gfft_shuffle_gen_cplx(in,size);
	break;
    }
}

static void
gfft_shuffle_gen_cplx(complex *in,int size)
{
    register real tmp_real;
    register real tmp_imag;
    
    register int i;
    register int m;
    register int j = 0;

    for (i = 0; i < size ; i++)
    {
	if (j > i)
	{
	    tmp_real = in[j].real;
	    tmp_imag = in[j].imag;
	    in[j].real = in[i].real;
	    in[j].imag = in[i].imag;
	    in[i].real = tmp_real;
	    in[i].imag = tmp_imag;
	}
	m = size >> 1;
	while ( (m >= 2) && ( j >= m))
	{
	    j -= m;
	    m = m >> 1;
	} 
	j  += m; 
    }  
}
