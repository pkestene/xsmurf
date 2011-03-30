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
 * $Id: gfft_shuffle_128_cplx.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

void
gfft_shuffle_128_cplx(complex *in)
{
	register real tmp_real;
	register real tmp_imag;

	tmp_real = in[1].real;
	tmp_imag = in[1].imag;
	in[1].real = in[64].real;
	in[1].imag = in[64].imag;
	in[64].real = tmp_real;
	in[64].imag = tmp_imag;

	tmp_real = in[2].real;
	tmp_imag = in[2].imag;
	in[2].real = in[32].real;
	in[2].imag = in[32].imag;
	in[32].real = tmp_real;
	in[32].imag = tmp_imag;

	tmp_real = in[3].real;
	tmp_imag = in[3].imag;
	in[3].real = in[96].real;
	in[3].imag = in[96].imag;
	in[96].real = tmp_real;
	in[96].imag = tmp_imag;

	tmp_real = in[4].real;
	tmp_imag = in[4].imag;
	in[4].real = in[16].real;
	in[4].imag = in[16].imag;
	in[16].real = tmp_real;
	in[16].imag = tmp_imag;

	tmp_real = in[5].real;
	tmp_imag = in[5].imag;
	in[5].real = in[80].real;
	in[5].imag = in[80].imag;
	in[80].real = tmp_real;
	in[80].imag = tmp_imag;

	tmp_real = in[6].real;
	tmp_imag = in[6].imag;
	in[6].real = in[48].real;
	in[6].imag = in[48].imag;
	in[48].real = tmp_real;
	in[48].imag = tmp_imag;

	tmp_real = in[7].real;
	tmp_imag = in[7].imag;
	in[7].real = in[112].real;
	in[7].imag = in[112].imag;
	in[112].real = tmp_real;
	in[112].imag = tmp_imag;

	tmp_real = in[9].real;
	tmp_imag = in[9].imag;
	in[9].real = in[72].real;
	in[9].imag = in[72].imag;
	in[72].real = tmp_real;
	in[72].imag = tmp_imag;

	tmp_real = in[10].real;
	tmp_imag = in[10].imag;
	in[10].real = in[40].real;
	in[10].imag = in[40].imag;
	in[40].real = tmp_real;
	in[40].imag = tmp_imag;

	tmp_real = in[11].real;
	tmp_imag = in[11].imag;
	in[11].real = in[104].real;
	in[11].imag = in[104].imag;
	in[104].real = tmp_real;
	in[104].imag = tmp_imag;

	tmp_real = in[12].real;
	tmp_imag = in[12].imag;
	in[12].real = in[24].real;
	in[12].imag = in[24].imag;
	in[24].real = tmp_real;
	in[24].imag = tmp_imag;

	tmp_real = in[13].real;
	tmp_imag = in[13].imag;
	in[13].real = in[88].real;
	in[13].imag = in[88].imag;
	in[88].real = tmp_real;
	in[88].imag = tmp_imag;

	tmp_real = in[14].real;
	tmp_imag = in[14].imag;
	in[14].real = in[56].real;
	in[14].imag = in[56].imag;
	in[56].real = tmp_real;
	in[56].imag = tmp_imag;

	tmp_real = in[15].real;
	tmp_imag = in[15].imag;
	in[15].real = in[120].real;
	in[15].imag = in[120].imag;
	in[120].real = tmp_real;
	in[120].imag = tmp_imag;

	tmp_real = in[17].real;
	tmp_imag = in[17].imag;
	in[17].real = in[68].real;
	in[17].imag = in[68].imag;
	in[68].real = tmp_real;
	in[68].imag = tmp_imag;

	tmp_real = in[18].real;
	tmp_imag = in[18].imag;
	in[18].real = in[36].real;
	in[18].imag = in[36].imag;
	in[36].real = tmp_real;
	in[36].imag = tmp_imag;

	tmp_real = in[19].real;
	tmp_imag = in[19].imag;
	in[19].real = in[100].real;
	in[19].imag = in[100].imag;
	in[100].real = tmp_real;
	in[100].imag = tmp_imag;

	tmp_real = in[21].real;
	tmp_imag = in[21].imag;
	in[21].real = in[84].real;
	in[21].imag = in[84].imag;
	in[84].real = tmp_real;
	in[84].imag = tmp_imag;

	tmp_real = in[22].real;
	tmp_imag = in[22].imag;
	in[22].real = in[52].real;
	in[22].imag = in[52].imag;
	in[52].real = tmp_real;
	in[52].imag = tmp_imag;

	tmp_real = in[23].real;
	tmp_imag = in[23].imag;
	in[23].real = in[116].real;
	in[23].imag = in[116].imag;
	in[116].real = tmp_real;
	in[116].imag = tmp_imag;

	tmp_real = in[25].real;
	tmp_imag = in[25].imag;
	in[25].real = in[76].real;
	in[25].imag = in[76].imag;
	in[76].real = tmp_real;
	in[76].imag = tmp_imag;

	tmp_real = in[26].real;
	tmp_imag = in[26].imag;
	in[26].real = in[44].real;
	in[26].imag = in[44].imag;
	in[44].real = tmp_real;
	in[44].imag = tmp_imag;

	tmp_real = in[27].real;
	tmp_imag = in[27].imag;
	in[27].real = in[108].real;
	in[27].imag = in[108].imag;
	in[108].real = tmp_real;
	in[108].imag = tmp_imag;

	tmp_real = in[29].real;
	tmp_imag = in[29].imag;
	in[29].real = in[92].real;
	in[29].imag = in[92].imag;
	in[92].real = tmp_real;
	in[92].imag = tmp_imag;

	tmp_real = in[30].real;
	tmp_imag = in[30].imag;
	in[30].real = in[60].real;
	in[30].imag = in[60].imag;
	in[60].real = tmp_real;
	in[60].imag = tmp_imag;

	tmp_real = in[31].real;
	tmp_imag = in[31].imag;
	in[31].real = in[124].real;
	in[31].imag = in[124].imag;
	in[124].real = tmp_real;
	in[124].imag = tmp_imag;

	tmp_real = in[33].real;
	tmp_imag = in[33].imag;
	in[33].real = in[66].real;
	in[33].imag = in[66].imag;
	in[66].real = tmp_real;
	in[66].imag = tmp_imag;

	tmp_real = in[35].real;
	tmp_imag = in[35].imag;
	in[35].real = in[98].real;
	in[35].imag = in[98].imag;
	in[98].real = tmp_real;
	in[98].imag = tmp_imag;

	tmp_real = in[37].real;
	tmp_imag = in[37].imag;
	in[37].real = in[82].real;
	in[37].imag = in[82].imag;
	in[82].real = tmp_real;
	in[82].imag = tmp_imag;

	tmp_real = in[38].real;
	tmp_imag = in[38].imag;
	in[38].real = in[50].real;
	in[38].imag = in[50].imag;
	in[50].real = tmp_real;
	in[50].imag = tmp_imag;

	tmp_real = in[39].real;
	tmp_imag = in[39].imag;
	in[39].real = in[114].real;
	in[39].imag = in[114].imag;
	in[114].real = tmp_real;
	in[114].imag = tmp_imag;

	tmp_real = in[41].real;
	tmp_imag = in[41].imag;
	in[41].real = in[74].real;
	in[41].imag = in[74].imag;
	in[74].real = tmp_real;
	in[74].imag = tmp_imag;

	tmp_real = in[43].real;
	tmp_imag = in[43].imag;
	in[43].real = in[106].real;
	in[43].imag = in[106].imag;
	in[106].real = tmp_real;
	in[106].imag = tmp_imag;

	tmp_real = in[45].real;
	tmp_imag = in[45].imag;
	in[45].real = in[90].real;
	in[45].imag = in[90].imag;
	in[90].real = tmp_real;
	in[90].imag = tmp_imag;

	tmp_real = in[46].real;
	tmp_imag = in[46].imag;
	in[46].real = in[58].real;
	in[46].imag = in[58].imag;
	in[58].real = tmp_real;
	in[58].imag = tmp_imag;

	tmp_real = in[47].real;
	tmp_imag = in[47].imag;
	in[47].real = in[122].real;
	in[47].imag = in[122].imag;
	in[122].real = tmp_real;
	in[122].imag = tmp_imag;

	tmp_real = in[49].real;
	tmp_imag = in[49].imag;
	in[49].real = in[70].real;
	in[49].imag = in[70].imag;
	in[70].real = tmp_real;
	in[70].imag = tmp_imag;

	tmp_real = in[51].real;
	tmp_imag = in[51].imag;
	in[51].real = in[102].real;
	in[51].imag = in[102].imag;
	in[102].real = tmp_real;
	in[102].imag = tmp_imag;

	tmp_real = in[53].real;
	tmp_imag = in[53].imag;
	in[53].real = in[86].real;
	in[53].imag = in[86].imag;
	in[86].real = tmp_real;
	in[86].imag = tmp_imag;

	tmp_real = in[55].real;
	tmp_imag = in[55].imag;
	in[55].real = in[118].real;
	in[55].imag = in[118].imag;
	in[118].real = tmp_real;
	in[118].imag = tmp_imag;

	tmp_real = in[57].real;
	tmp_imag = in[57].imag;
	in[57].real = in[78].real;
	in[57].imag = in[78].imag;
	in[78].real = tmp_real;
	in[78].imag = tmp_imag;

	tmp_real = in[59].real;
	tmp_imag = in[59].imag;
	in[59].real = in[110].real;
	in[59].imag = in[110].imag;
	in[110].real = tmp_real;
	in[110].imag = tmp_imag;

	tmp_real = in[61].real;
	tmp_imag = in[61].imag;
	in[61].real = in[94].real;
	in[61].imag = in[94].imag;
	in[94].real = tmp_real;
	in[94].imag = tmp_imag;

	tmp_real = in[63].real;
	tmp_imag = in[63].imag;
	in[63].real = in[126].real;
	in[63].imag = in[126].imag;
	in[126].real = tmp_real;
	in[126].imag = tmp_imag;

	tmp_real = in[67].real;
	tmp_imag = in[67].imag;
	in[67].real = in[97].real;
	in[67].imag = in[97].imag;
	in[97].real = tmp_real;
	in[97].imag = tmp_imag;

	tmp_real = in[69].real;
	tmp_imag = in[69].imag;
	in[69].real = in[81].real;
	in[69].imag = in[81].imag;
	in[81].real = tmp_real;
	in[81].imag = tmp_imag;

	tmp_real = in[71].real;
	tmp_imag = in[71].imag;
	in[71].real = in[113].real;
	in[71].imag = in[113].imag;
	in[113].real = tmp_real;
	in[113].imag = tmp_imag;

	tmp_real = in[75].real;
	tmp_imag = in[75].imag;
	in[75].real = in[105].real;
	in[75].imag = in[105].imag;
	in[105].real = tmp_real;
	in[105].imag = tmp_imag;

	tmp_real = in[77].real;
	tmp_imag = in[77].imag;
	in[77].real = in[89].real;
	in[77].imag = in[89].imag;
	in[89].real = tmp_real;
	in[89].imag = tmp_imag;

	tmp_real = in[79].real;
	tmp_imag = in[79].imag;
	in[79].real = in[121].real;
	in[79].imag = in[121].imag;
	in[121].real = tmp_real;
	in[121].imag = tmp_imag;

	tmp_real = in[83].real;
	tmp_imag = in[83].imag;
	in[83].real = in[101].real;
	in[83].imag = in[101].imag;
	in[101].real = tmp_real;
	in[101].imag = tmp_imag;

	tmp_real = in[87].real;
	tmp_imag = in[87].imag;
	in[87].real = in[117].real;
	in[87].imag = in[117].imag;
	in[117].real = tmp_real;
	in[117].imag = tmp_imag;

	tmp_real = in[91].real;
	tmp_imag = in[91].imag;
	in[91].real = in[109].real;
	in[91].imag = in[109].imag;
	in[109].real = tmp_real;
	in[109].imag = tmp_imag;

	tmp_real = in[95].real;
	tmp_imag = in[95].imag;
	in[95].real = in[125].real;
	in[95].imag = in[125].imag;
	in[125].real = tmp_real;
	in[125].imag = tmp_imag;

	tmp_real = in[103].real;
	tmp_imag = in[103].imag;
	in[103].real = in[115].real;
	in[103].imag = in[115].imag;
	in[115].real = tmp_real;
	in[115].imag = tmp_imag;

	tmp_real = in[111].real;
	tmp_imag = in[111].imag;
	in[111].real = in[123].real;
	in[111].imag = in[123].imag;
	in[123].real = tmp_real;
	in[123].imag = tmp_imag;

}
