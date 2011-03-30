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
 * $Id: gfft_shuffle_64_cplx.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

void
gfft_shuffle_64_cplx(complex *in)
{
	register real tmp_real;
	register real tmp_imag;

	tmp_real = in[1].real;
	tmp_imag = in[1].imag;
	in[1].real = in[32].real;
	in[1].imag = in[32].imag;
	in[32].real = tmp_real;
	in[32].imag = tmp_imag;

	tmp_real = in[2].real;
	tmp_imag = in[2].imag;
	in[2].real = in[16].real;
	in[2].imag = in[16].imag;
	in[16].real = tmp_real;
	in[16].imag = tmp_imag;

	tmp_real = in[3].real;
	tmp_imag = in[3].imag;
	in[3].real = in[48].real;
	in[3].imag = in[48].imag;
	in[48].real = tmp_real;
	in[48].imag = tmp_imag;

	tmp_real = in[4].real;
	tmp_imag = in[4].imag;
	in[4].real = in[8].real;
	in[4].imag = in[8].imag;
	in[8].real = tmp_real;
	in[8].imag = tmp_imag;

	tmp_real = in[5].real;
	tmp_imag = in[5].imag;
	in[5].real = in[40].real;
	in[5].imag = in[40].imag;
	in[40].real = tmp_real;
	in[40].imag = tmp_imag;

	tmp_real = in[6].real;
	tmp_imag = in[6].imag;
	in[6].real = in[24].real;
	in[6].imag = in[24].imag;
	in[24].real = tmp_real;
	in[24].imag = tmp_imag;

	tmp_real = in[7].real;
	tmp_imag = in[7].imag;
	in[7].real = in[56].real;
	in[7].imag = in[56].imag;
	in[56].real = tmp_real;
	in[56].imag = tmp_imag;

	tmp_real = in[9].real;
	tmp_imag = in[9].imag;
	in[9].real = in[36].real;
	in[9].imag = in[36].imag;
	in[36].real = tmp_real;
	in[36].imag = tmp_imag;

	tmp_real = in[10].real;
	tmp_imag = in[10].imag;
	in[10].real = in[20].real;
	in[10].imag = in[20].imag;
	in[20].real = tmp_real;
	in[20].imag = tmp_imag;

	tmp_real = in[11].real;
	tmp_imag = in[11].imag;
	in[11].real = in[52].real;
	in[11].imag = in[52].imag;
	in[52].real = tmp_real;
	in[52].imag = tmp_imag;

	tmp_real = in[13].real;
	tmp_imag = in[13].imag;
	in[13].real = in[44].real;
	in[13].imag = in[44].imag;
	in[44].real = tmp_real;
	in[44].imag = tmp_imag;

	tmp_real = in[14].real;
	tmp_imag = in[14].imag;
	in[14].real = in[28].real;
	in[14].imag = in[28].imag;
	in[28].real = tmp_real;
	in[28].imag = tmp_imag;

	tmp_real = in[15].real;
	tmp_imag = in[15].imag;
	in[15].real = in[60].real;
	in[15].imag = in[60].imag;
	in[60].real = tmp_real;
	in[60].imag = tmp_imag;

	tmp_real = in[17].real;
	tmp_imag = in[17].imag;
	in[17].real = in[34].real;
	in[17].imag = in[34].imag;
	in[34].real = tmp_real;
	in[34].imag = tmp_imag;

	tmp_real = in[19].real;
	tmp_imag = in[19].imag;
	in[19].real = in[50].real;
	in[19].imag = in[50].imag;
	in[50].real = tmp_real;
	in[50].imag = tmp_imag;

	tmp_real = in[21].real;
	tmp_imag = in[21].imag;
	in[21].real = in[42].real;
	in[21].imag = in[42].imag;
	in[42].real = tmp_real;
	in[42].imag = tmp_imag;

	tmp_real = in[22].real;
	tmp_imag = in[22].imag;
	in[22].real = in[26].real;
	in[22].imag = in[26].imag;
	in[26].real = tmp_real;
	in[26].imag = tmp_imag;

	tmp_real = in[23].real;
	tmp_imag = in[23].imag;
	in[23].real = in[58].real;
	in[23].imag = in[58].imag;
	in[58].real = tmp_real;
	in[58].imag = tmp_imag;

	tmp_real = in[25].real;
	tmp_imag = in[25].imag;
	in[25].real = in[38].real;
	in[25].imag = in[38].imag;
	in[38].real = tmp_real;
	in[38].imag = tmp_imag;

	tmp_real = in[27].real;
	tmp_imag = in[27].imag;
	in[27].real = in[54].real;
	in[27].imag = in[54].imag;
	in[54].real = tmp_real;
	in[54].imag = tmp_imag;

	tmp_real = in[29].real;
	tmp_imag = in[29].imag;
	in[29].real = in[46].real;
	in[29].imag = in[46].imag;
	in[46].real = tmp_real;
	in[46].imag = tmp_imag;

	tmp_real = in[31].real;
	tmp_imag = in[31].imag;
	in[31].real = in[62].real;
	in[31].imag = in[62].imag;
	in[62].real = tmp_real;
	in[62].imag = tmp_imag;

	tmp_real = in[35].real;
	tmp_imag = in[35].imag;
	in[35].real = in[49].real;
	in[35].imag = in[49].imag;
	in[49].real = tmp_real;
	in[49].imag = tmp_imag;

	tmp_real = in[37].real;
	tmp_imag = in[37].imag;
	in[37].real = in[41].real;
	in[37].imag = in[41].imag;
	in[41].real = tmp_real;
	in[41].imag = tmp_imag;

	tmp_real = in[39].real;
	tmp_imag = in[39].imag;
	in[39].real = in[57].real;
	in[39].imag = in[57].imag;
	in[57].real = tmp_real;
	in[57].imag = tmp_imag;

	tmp_real = in[43].real;
	tmp_imag = in[43].imag;
	in[43].real = in[53].real;
	in[43].imag = in[53].imag;
	in[53].real = tmp_real;
	in[53].imag = tmp_imag;

	tmp_real = in[47].real;
	tmp_imag = in[47].imag;
	in[47].real = in[61].real;
	in[47].imag = in[61].imag;
	in[61].real = tmp_real;
	in[61].imag = tmp_imag;

	tmp_real = in[55].real;
	tmp_imag = in[55].imag;
	in[55].real = in[59].real;
	in[55].imag = in[59].imag;
	in[59].real = tmp_real;
	in[59].imag = tmp_imag;

}
