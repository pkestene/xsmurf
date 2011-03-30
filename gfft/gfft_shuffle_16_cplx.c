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
 * $Id: gfft_shuffle_16_cplx.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

void
gfft_shuffle_16_cplx(complex *in)
{
	register real tmp_real;
	register real tmp_imag;

	tmp_real = in[1].real;
	tmp_imag = in[1].imag;
	in[1].real = in[8].real;
	in[1].imag = in[8].imag;
	in[8].real = tmp_real;
	in[8].imag = tmp_imag;

	tmp_real = in[2].real;
	tmp_imag = in[2].imag;
	in[2].real = in[4].real;
	in[2].imag = in[4].imag;
	in[4].real = tmp_real;
	in[4].imag = tmp_imag;

	tmp_real = in[3].real;
	tmp_imag = in[3].imag;
	in[3].real = in[12].real;
	in[3].imag = in[12].imag;
	in[12].real = tmp_real;
	in[12].imag = tmp_imag;

	tmp_real = in[5].real;
	tmp_imag = in[5].imag;
	in[5].real = in[10].real;
	in[5].imag = in[10].imag;
	in[10].real = tmp_real;
	in[10].imag = tmp_imag;

	tmp_real = in[7].real;
	tmp_imag = in[7].imag;
	in[7].real = in[14].real;
	in[7].imag = in[14].imag;
	in[14].real = tmp_real;
	in[14].imag = tmp_imag;

	tmp_real = in[11].real;
	tmp_imag = in[11].imag;
	in[11].real = in[13].real;
	in[11].imag = in[13].imag;
	in[13].real = tmp_real;
	in[13].imag = tmp_imag;

}
