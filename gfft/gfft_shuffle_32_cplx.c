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
 * $Id: gfft_shuffle_32_cplx.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

void
gfft_shuffle_32_cplx(complex *in)
{
	register real tmp_real;
	register real tmp_imag;

	tmp_real = in[1].real;
	tmp_imag = in[1].imag;
	in[1].real = in[16].real;
	in[1].imag = in[16].imag;
	in[16].real = tmp_real;
	in[16].imag = tmp_imag;

	tmp_real = in[2].real;
	tmp_imag = in[2].imag;
	in[2].real = in[8].real;
	in[2].imag = in[8].imag;
	in[8].real = tmp_real;
	in[8].imag = tmp_imag;

	tmp_real = in[3].real;
	tmp_imag = in[3].imag;
	in[3].real = in[24].real;
	in[3].imag = in[24].imag;
	in[24].real = tmp_real;
	in[24].imag = tmp_imag;

	tmp_real = in[5].real;
	tmp_imag = in[5].imag;
	in[5].real = in[20].real;
	in[5].imag = in[20].imag;
	in[20].real = tmp_real;
	in[20].imag = tmp_imag;

	tmp_real = in[6].real;
	tmp_imag = in[6].imag;
	in[6].real = in[12].real;
	in[6].imag = in[12].imag;
	in[12].real = tmp_real;
	in[12].imag = tmp_imag;

	tmp_real = in[7].real;
	tmp_imag = in[7].imag;
	in[7].real = in[28].real;
	in[7].imag = in[28].imag;
	in[28].real = tmp_real;
	in[28].imag = tmp_imag;

	tmp_real = in[9].real;
	tmp_imag = in[9].imag;
	in[9].real = in[18].real;
	in[9].imag = in[18].imag;
	in[18].real = tmp_real;
	in[18].imag = tmp_imag;

	tmp_real = in[11].real;
	tmp_imag = in[11].imag;
	in[11].real = in[26].real;
	in[11].imag = in[26].imag;
	in[26].real = tmp_real;
	in[26].imag = tmp_imag;

	tmp_real = in[13].real;
	tmp_imag = in[13].imag;
	in[13].real = in[22].real;
	in[13].imag = in[22].imag;
	in[22].real = tmp_real;
	in[22].imag = tmp_imag;

	tmp_real = in[15].real;
	tmp_imag = in[15].imag;
	in[15].real = in[30].real;
	in[15].imag = in[30].imag;
	in[30].real = tmp_real;
	in[30].imag = tmp_imag;

	tmp_real = in[19].real;
	tmp_imag = in[19].imag;
	in[19].real = in[25].real;
	in[19].imag = in[25].imag;
	in[25].real = tmp_real;
	in[25].imag = tmp_imag;

	tmp_real = in[23].real;
	tmp_imag = in[23].imag;
	in[23].real = in[29].real;
	in[23].imag = in[29].imag;
	in[29].real = tmp_real;
	in[29].imag = tmp_imag;

}
