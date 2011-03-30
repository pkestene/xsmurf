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
 * $Id: gfft_shuffle_256_cplx.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

void
gfft_shuffle_256_cplx(complex *in)
{
	register real tmp_real;
	register real tmp_imag;

	tmp_real = in[1].real;
	tmp_imag = in[1].imag;
	in[1].real = in[128].real;
	in[1].imag = in[128].imag;
	in[128].real = tmp_real;
	in[128].imag = tmp_imag;

	tmp_real = in[2].real;
	tmp_imag = in[2].imag;
	in[2].real = in[64].real;
	in[2].imag = in[64].imag;
	in[64].real = tmp_real;
	in[64].imag = tmp_imag;

	tmp_real = in[3].real;
	tmp_imag = in[3].imag;
	in[3].real = in[192].real;
	in[3].imag = in[192].imag;
	in[192].real = tmp_real;
	in[192].imag = tmp_imag;

	tmp_real = in[4].real;
	tmp_imag = in[4].imag;
	in[4].real = in[32].real;
	in[4].imag = in[32].imag;
	in[32].real = tmp_real;
	in[32].imag = tmp_imag;

	tmp_real = in[5].real;
	tmp_imag = in[5].imag;
	in[5].real = in[160].real;
	in[5].imag = in[160].imag;
	in[160].real = tmp_real;
	in[160].imag = tmp_imag;

	tmp_real = in[6].real;
	tmp_imag = in[6].imag;
	in[6].real = in[96].real;
	in[6].imag = in[96].imag;
	in[96].real = tmp_real;
	in[96].imag = tmp_imag;

	tmp_real = in[7].real;
	tmp_imag = in[7].imag;
	in[7].real = in[224].real;
	in[7].imag = in[224].imag;
	in[224].real = tmp_real;
	in[224].imag = tmp_imag;

	tmp_real = in[8].real;
	tmp_imag = in[8].imag;
	in[8].real = in[16].real;
	in[8].imag = in[16].imag;
	in[16].real = tmp_real;
	in[16].imag = tmp_imag;

	tmp_real = in[9].real;
	tmp_imag = in[9].imag;
	in[9].real = in[144].real;
	in[9].imag = in[144].imag;
	in[144].real = tmp_real;
	in[144].imag = tmp_imag;

	tmp_real = in[10].real;
	tmp_imag = in[10].imag;
	in[10].real = in[80].real;
	in[10].imag = in[80].imag;
	in[80].real = tmp_real;
	in[80].imag = tmp_imag;

	tmp_real = in[11].real;
	tmp_imag = in[11].imag;
	in[11].real = in[208].real;
	in[11].imag = in[208].imag;
	in[208].real = tmp_real;
	in[208].imag = tmp_imag;

	tmp_real = in[12].real;
	tmp_imag = in[12].imag;
	in[12].real = in[48].real;
	in[12].imag = in[48].imag;
	in[48].real = tmp_real;
	in[48].imag = tmp_imag;

	tmp_real = in[13].real;
	tmp_imag = in[13].imag;
	in[13].real = in[176].real;
	in[13].imag = in[176].imag;
	in[176].real = tmp_real;
	in[176].imag = tmp_imag;

	tmp_real = in[14].real;
	tmp_imag = in[14].imag;
	in[14].real = in[112].real;
	in[14].imag = in[112].imag;
	in[112].real = tmp_real;
	in[112].imag = tmp_imag;

	tmp_real = in[15].real;
	tmp_imag = in[15].imag;
	in[15].real = in[240].real;
	in[15].imag = in[240].imag;
	in[240].real = tmp_real;
	in[240].imag = tmp_imag;

	tmp_real = in[17].real;
	tmp_imag = in[17].imag;
	in[17].real = in[136].real;
	in[17].imag = in[136].imag;
	in[136].real = tmp_real;
	in[136].imag = tmp_imag;

	tmp_real = in[18].real;
	tmp_imag = in[18].imag;
	in[18].real = in[72].real;
	in[18].imag = in[72].imag;
	in[72].real = tmp_real;
	in[72].imag = tmp_imag;

	tmp_real = in[19].real;
	tmp_imag = in[19].imag;
	in[19].real = in[200].real;
	in[19].imag = in[200].imag;
	in[200].real = tmp_real;
	in[200].imag = tmp_imag;

	tmp_real = in[20].real;
	tmp_imag = in[20].imag;
	in[20].real = in[40].real;
	in[20].imag = in[40].imag;
	in[40].real = tmp_real;
	in[40].imag = tmp_imag;

	tmp_real = in[21].real;
	tmp_imag = in[21].imag;
	in[21].real = in[168].real;
	in[21].imag = in[168].imag;
	in[168].real = tmp_real;
	in[168].imag = tmp_imag;

	tmp_real = in[22].real;
	tmp_imag = in[22].imag;
	in[22].real = in[104].real;
	in[22].imag = in[104].imag;
	in[104].real = tmp_real;
	in[104].imag = tmp_imag;

	tmp_real = in[23].real;
	tmp_imag = in[23].imag;
	in[23].real = in[232].real;
	in[23].imag = in[232].imag;
	in[232].real = tmp_real;
	in[232].imag = tmp_imag;

	tmp_real = in[25].real;
	tmp_imag = in[25].imag;
	in[25].real = in[152].real;
	in[25].imag = in[152].imag;
	in[152].real = tmp_real;
	in[152].imag = tmp_imag;

	tmp_real = in[26].real;
	tmp_imag = in[26].imag;
	in[26].real = in[88].real;
	in[26].imag = in[88].imag;
	in[88].real = tmp_real;
	in[88].imag = tmp_imag;

	tmp_real = in[27].real;
	tmp_imag = in[27].imag;
	in[27].real = in[216].real;
	in[27].imag = in[216].imag;
	in[216].real = tmp_real;
	in[216].imag = tmp_imag;

	tmp_real = in[28].real;
	tmp_imag = in[28].imag;
	in[28].real = in[56].real;
	in[28].imag = in[56].imag;
	in[56].real = tmp_real;
	in[56].imag = tmp_imag;

	tmp_real = in[29].real;
	tmp_imag = in[29].imag;
	in[29].real = in[184].real;
	in[29].imag = in[184].imag;
	in[184].real = tmp_real;
	in[184].imag = tmp_imag;

	tmp_real = in[30].real;
	tmp_imag = in[30].imag;
	in[30].real = in[120].real;
	in[30].imag = in[120].imag;
	in[120].real = tmp_real;
	in[120].imag = tmp_imag;

	tmp_real = in[31].real;
	tmp_imag = in[31].imag;
	in[31].real = in[248].real;
	in[31].imag = in[248].imag;
	in[248].real = tmp_real;
	in[248].imag = tmp_imag;

	tmp_real = in[33].real;
	tmp_imag = in[33].imag;
	in[33].real = in[132].real;
	in[33].imag = in[132].imag;
	in[132].real = tmp_real;
	in[132].imag = tmp_imag;

	tmp_real = in[34].real;
	tmp_imag = in[34].imag;
	in[34].real = in[68].real;
	in[34].imag = in[68].imag;
	in[68].real = tmp_real;
	in[68].imag = tmp_imag;

	tmp_real = in[35].real;
	tmp_imag = in[35].imag;
	in[35].real = in[196].real;
	in[35].imag = in[196].imag;
	in[196].real = tmp_real;
	in[196].imag = tmp_imag;

	tmp_real = in[37].real;
	tmp_imag = in[37].imag;
	in[37].real = in[164].real;
	in[37].imag = in[164].imag;
	in[164].real = tmp_real;
	in[164].imag = tmp_imag;

	tmp_real = in[38].real;
	tmp_imag = in[38].imag;
	in[38].real = in[100].real;
	in[38].imag = in[100].imag;
	in[100].real = tmp_real;
	in[100].imag = tmp_imag;

	tmp_real = in[39].real;
	tmp_imag = in[39].imag;
	in[39].real = in[228].real;
	in[39].imag = in[228].imag;
	in[228].real = tmp_real;
	in[228].imag = tmp_imag;

	tmp_real = in[41].real;
	tmp_imag = in[41].imag;
	in[41].real = in[148].real;
	in[41].imag = in[148].imag;
	in[148].real = tmp_real;
	in[148].imag = tmp_imag;

	tmp_real = in[42].real;
	tmp_imag = in[42].imag;
	in[42].real = in[84].real;
	in[42].imag = in[84].imag;
	in[84].real = tmp_real;
	in[84].imag = tmp_imag;

	tmp_real = in[43].real;
	tmp_imag = in[43].imag;
	in[43].real = in[212].real;
	in[43].imag = in[212].imag;
	in[212].real = tmp_real;
	in[212].imag = tmp_imag;

	tmp_real = in[44].real;
	tmp_imag = in[44].imag;
	in[44].real = in[52].real;
	in[44].imag = in[52].imag;
	in[52].real = tmp_real;
	in[52].imag = tmp_imag;

	tmp_real = in[45].real;
	tmp_imag = in[45].imag;
	in[45].real = in[180].real;
	in[45].imag = in[180].imag;
	in[180].real = tmp_real;
	in[180].imag = tmp_imag;

	tmp_real = in[46].real;
	tmp_imag = in[46].imag;
	in[46].real = in[116].real;
	in[46].imag = in[116].imag;
	in[116].real = tmp_real;
	in[116].imag = tmp_imag;

	tmp_real = in[47].real;
	tmp_imag = in[47].imag;
	in[47].real = in[244].real;
	in[47].imag = in[244].imag;
	in[244].real = tmp_real;
	in[244].imag = tmp_imag;

	tmp_real = in[49].real;
	tmp_imag = in[49].imag;
	in[49].real = in[140].real;
	in[49].imag = in[140].imag;
	in[140].real = tmp_real;
	in[140].imag = tmp_imag;

	tmp_real = in[50].real;
	tmp_imag = in[50].imag;
	in[50].real = in[76].real;
	in[50].imag = in[76].imag;
	in[76].real = tmp_real;
	in[76].imag = tmp_imag;

	tmp_real = in[51].real;
	tmp_imag = in[51].imag;
	in[51].real = in[204].real;
	in[51].imag = in[204].imag;
	in[204].real = tmp_real;
	in[204].imag = tmp_imag;

	tmp_real = in[53].real;
	tmp_imag = in[53].imag;
	in[53].real = in[172].real;
	in[53].imag = in[172].imag;
	in[172].real = tmp_real;
	in[172].imag = tmp_imag;

	tmp_real = in[54].real;
	tmp_imag = in[54].imag;
	in[54].real = in[108].real;
	in[54].imag = in[108].imag;
	in[108].real = tmp_real;
	in[108].imag = tmp_imag;

	tmp_real = in[55].real;
	tmp_imag = in[55].imag;
	in[55].real = in[236].real;
	in[55].imag = in[236].imag;
	in[236].real = tmp_real;
	in[236].imag = tmp_imag;

	tmp_real = in[57].real;
	tmp_imag = in[57].imag;
	in[57].real = in[156].real;
	in[57].imag = in[156].imag;
	in[156].real = tmp_real;
	in[156].imag = tmp_imag;

	tmp_real = in[58].real;
	tmp_imag = in[58].imag;
	in[58].real = in[92].real;
	in[58].imag = in[92].imag;
	in[92].real = tmp_real;
	in[92].imag = tmp_imag;

	tmp_real = in[59].real;
	tmp_imag = in[59].imag;
	in[59].real = in[220].real;
	in[59].imag = in[220].imag;
	in[220].real = tmp_real;
	in[220].imag = tmp_imag;

	tmp_real = in[61].real;
	tmp_imag = in[61].imag;
	in[61].real = in[188].real;
	in[61].imag = in[188].imag;
	in[188].real = tmp_real;
	in[188].imag = tmp_imag;

	tmp_real = in[62].real;
	tmp_imag = in[62].imag;
	in[62].real = in[124].real;
	in[62].imag = in[124].imag;
	in[124].real = tmp_real;
	in[124].imag = tmp_imag;

	tmp_real = in[63].real;
	tmp_imag = in[63].imag;
	in[63].real = in[252].real;
	in[63].imag = in[252].imag;
	in[252].real = tmp_real;
	in[252].imag = tmp_imag;

	tmp_real = in[65].real;
	tmp_imag = in[65].imag;
	in[65].real = in[130].real;
	in[65].imag = in[130].imag;
	in[130].real = tmp_real;
	in[130].imag = tmp_imag;

	tmp_real = in[67].real;
	tmp_imag = in[67].imag;
	in[67].real = in[194].real;
	in[67].imag = in[194].imag;
	in[194].real = tmp_real;
	in[194].imag = tmp_imag;

	tmp_real = in[69].real;
	tmp_imag = in[69].imag;
	in[69].real = in[162].real;
	in[69].imag = in[162].imag;
	in[162].real = tmp_real;
	in[162].imag = tmp_imag;

	tmp_real = in[70].real;
	tmp_imag = in[70].imag;
	in[70].real = in[98].real;
	in[70].imag = in[98].imag;
	in[98].real = tmp_real;
	in[98].imag = tmp_imag;

	tmp_real = in[71].real;
	tmp_imag = in[71].imag;
	in[71].real = in[226].real;
	in[71].imag = in[226].imag;
	in[226].real = tmp_real;
	in[226].imag = tmp_imag;

	tmp_real = in[73].real;
	tmp_imag = in[73].imag;
	in[73].real = in[146].real;
	in[73].imag = in[146].imag;
	in[146].real = tmp_real;
	in[146].imag = tmp_imag;

	tmp_real = in[74].real;
	tmp_imag = in[74].imag;
	in[74].real = in[82].real;
	in[74].imag = in[82].imag;
	in[82].real = tmp_real;
	in[82].imag = tmp_imag;

	tmp_real = in[75].real;
	tmp_imag = in[75].imag;
	in[75].real = in[210].real;
	in[75].imag = in[210].imag;
	in[210].real = tmp_real;
	in[210].imag = tmp_imag;

	tmp_real = in[77].real;
	tmp_imag = in[77].imag;
	in[77].real = in[178].real;
	in[77].imag = in[178].imag;
	in[178].real = tmp_real;
	in[178].imag = tmp_imag;

	tmp_real = in[78].real;
	tmp_imag = in[78].imag;
	in[78].real = in[114].real;
	in[78].imag = in[114].imag;
	in[114].real = tmp_real;
	in[114].imag = tmp_imag;

	tmp_real = in[79].real;
	tmp_imag = in[79].imag;
	in[79].real = in[242].real;
	in[79].imag = in[242].imag;
	in[242].real = tmp_real;
	in[242].imag = tmp_imag;

	tmp_real = in[81].real;
	tmp_imag = in[81].imag;
	in[81].real = in[138].real;
	in[81].imag = in[138].imag;
	in[138].real = tmp_real;
	in[138].imag = tmp_imag;

	tmp_real = in[83].real;
	tmp_imag = in[83].imag;
	in[83].real = in[202].real;
	in[83].imag = in[202].imag;
	in[202].real = tmp_real;
	in[202].imag = tmp_imag;

	tmp_real = in[85].real;
	tmp_imag = in[85].imag;
	in[85].real = in[170].real;
	in[85].imag = in[170].imag;
	in[170].real = tmp_real;
	in[170].imag = tmp_imag;

	tmp_real = in[86].real;
	tmp_imag = in[86].imag;
	in[86].real = in[106].real;
	in[86].imag = in[106].imag;
	in[106].real = tmp_real;
	in[106].imag = tmp_imag;

	tmp_real = in[87].real;
	tmp_imag = in[87].imag;
	in[87].real = in[234].real;
	in[87].imag = in[234].imag;
	in[234].real = tmp_real;
	in[234].imag = tmp_imag;

	tmp_real = in[89].real;
	tmp_imag = in[89].imag;
	in[89].real = in[154].real;
	in[89].imag = in[154].imag;
	in[154].real = tmp_real;
	in[154].imag = tmp_imag;

	tmp_real = in[91].real;
	tmp_imag = in[91].imag;
	in[91].real = in[218].real;
	in[91].imag = in[218].imag;
	in[218].real = tmp_real;
	in[218].imag = tmp_imag;

	tmp_real = in[93].real;
	tmp_imag = in[93].imag;
	in[93].real = in[186].real;
	in[93].imag = in[186].imag;
	in[186].real = tmp_real;
	in[186].imag = tmp_imag;

	tmp_real = in[94].real;
	tmp_imag = in[94].imag;
	in[94].real = in[122].real;
	in[94].imag = in[122].imag;
	in[122].real = tmp_real;
	in[122].imag = tmp_imag;

	tmp_real = in[95].real;
	tmp_imag = in[95].imag;
	in[95].real = in[250].real;
	in[95].imag = in[250].imag;
	in[250].real = tmp_real;
	in[250].imag = tmp_imag;

	tmp_real = in[97].real;
	tmp_imag = in[97].imag;
	in[97].real = in[134].real;
	in[97].imag = in[134].imag;
	in[134].real = tmp_real;
	in[134].imag = tmp_imag;

	tmp_real = in[99].real;
	tmp_imag = in[99].imag;
	in[99].real = in[198].real;
	in[99].imag = in[198].imag;
	in[198].real = tmp_real;
	in[198].imag = tmp_imag;

	tmp_real = in[101].real;
	tmp_imag = in[101].imag;
	in[101].real = in[166].real;
	in[101].imag = in[166].imag;
	in[166].real = tmp_real;
	in[166].imag = tmp_imag;

	tmp_real = in[103].real;
	tmp_imag = in[103].imag;
	in[103].real = in[230].real;
	in[103].imag = in[230].imag;
	in[230].real = tmp_real;
	in[230].imag = tmp_imag;

	tmp_real = in[105].real;
	tmp_imag = in[105].imag;
	in[105].real = in[150].real;
	in[105].imag = in[150].imag;
	in[150].real = tmp_real;
	in[150].imag = tmp_imag;

	tmp_real = in[107].real;
	tmp_imag = in[107].imag;
	in[107].real = in[214].real;
	in[107].imag = in[214].imag;
	in[214].real = tmp_real;
	in[214].imag = tmp_imag;

	tmp_real = in[109].real;
	tmp_imag = in[109].imag;
	in[109].real = in[182].real;
	in[109].imag = in[182].imag;
	in[182].real = tmp_real;
	in[182].imag = tmp_imag;

	tmp_real = in[110].real;
	tmp_imag = in[110].imag;
	in[110].real = in[118].real;
	in[110].imag = in[118].imag;
	in[118].real = tmp_real;
	in[118].imag = tmp_imag;

	tmp_real = in[111].real;
	tmp_imag = in[111].imag;
	in[111].real = in[246].real;
	in[111].imag = in[246].imag;
	in[246].real = tmp_real;
	in[246].imag = tmp_imag;

	tmp_real = in[113].real;
	tmp_imag = in[113].imag;
	in[113].real = in[142].real;
	in[113].imag = in[142].imag;
	in[142].real = tmp_real;
	in[142].imag = tmp_imag;

	tmp_real = in[115].real;
	tmp_imag = in[115].imag;
	in[115].real = in[206].real;
	in[115].imag = in[206].imag;
	in[206].real = tmp_real;
	in[206].imag = tmp_imag;

	tmp_real = in[117].real;
	tmp_imag = in[117].imag;
	in[117].real = in[174].real;
	in[117].imag = in[174].imag;
	in[174].real = tmp_real;
	in[174].imag = tmp_imag;

	tmp_real = in[119].real;
	tmp_imag = in[119].imag;
	in[119].real = in[238].real;
	in[119].imag = in[238].imag;
	in[238].real = tmp_real;
	in[238].imag = tmp_imag;

	tmp_real = in[121].real;
	tmp_imag = in[121].imag;
	in[121].real = in[158].real;
	in[121].imag = in[158].imag;
	in[158].real = tmp_real;
	in[158].imag = tmp_imag;

	tmp_real = in[123].real;
	tmp_imag = in[123].imag;
	in[123].real = in[222].real;
	in[123].imag = in[222].imag;
	in[222].real = tmp_real;
	in[222].imag = tmp_imag;

	tmp_real = in[125].real;
	tmp_imag = in[125].imag;
	in[125].real = in[190].real;
	in[125].imag = in[190].imag;
	in[190].real = tmp_real;
	in[190].imag = tmp_imag;

	tmp_real = in[127].real;
	tmp_imag = in[127].imag;
	in[127].real = in[254].real;
	in[127].imag = in[254].imag;
	in[254].real = tmp_real;
	in[254].imag = tmp_imag;

	tmp_real = in[131].real;
	tmp_imag = in[131].imag;
	in[131].real = in[193].real;
	in[131].imag = in[193].imag;
	in[193].real = tmp_real;
	in[193].imag = tmp_imag;

	tmp_real = in[133].real;
	tmp_imag = in[133].imag;
	in[133].real = in[161].real;
	in[133].imag = in[161].imag;
	in[161].real = tmp_real;
	in[161].imag = tmp_imag;

	tmp_real = in[135].real;
	tmp_imag = in[135].imag;
	in[135].real = in[225].real;
	in[135].imag = in[225].imag;
	in[225].real = tmp_real;
	in[225].imag = tmp_imag;

	tmp_real = in[137].real;
	tmp_imag = in[137].imag;
	in[137].real = in[145].real;
	in[137].imag = in[145].imag;
	in[145].real = tmp_real;
	in[145].imag = tmp_imag;

	tmp_real = in[139].real;
	tmp_imag = in[139].imag;
	in[139].real = in[209].real;
	in[139].imag = in[209].imag;
	in[209].real = tmp_real;
	in[209].imag = tmp_imag;

	tmp_real = in[141].real;
	tmp_imag = in[141].imag;
	in[141].real = in[177].real;
	in[141].imag = in[177].imag;
	in[177].real = tmp_real;
	in[177].imag = tmp_imag;

	tmp_real = in[143].real;
	tmp_imag = in[143].imag;
	in[143].real = in[241].real;
	in[143].imag = in[241].imag;
	in[241].real = tmp_real;
	in[241].imag = tmp_imag;

	tmp_real = in[147].real;
	tmp_imag = in[147].imag;
	in[147].real = in[201].real;
	in[147].imag = in[201].imag;
	in[201].real = tmp_real;
	in[201].imag = tmp_imag;

	tmp_real = in[149].real;
	tmp_imag = in[149].imag;
	in[149].real = in[169].real;
	in[149].imag = in[169].imag;
	in[169].real = tmp_real;
	in[169].imag = tmp_imag;

	tmp_real = in[151].real;
	tmp_imag = in[151].imag;
	in[151].real = in[233].real;
	in[151].imag = in[233].imag;
	in[233].real = tmp_real;
	in[233].imag = tmp_imag;

	tmp_real = in[155].real;
	tmp_imag = in[155].imag;
	in[155].real = in[217].real;
	in[155].imag = in[217].imag;
	in[217].real = tmp_real;
	in[217].imag = tmp_imag;

	tmp_real = in[157].real;
	tmp_imag = in[157].imag;
	in[157].real = in[185].real;
	in[157].imag = in[185].imag;
	in[185].real = tmp_real;
	in[185].imag = tmp_imag;

	tmp_real = in[159].real;
	tmp_imag = in[159].imag;
	in[159].real = in[249].real;
	in[159].imag = in[249].imag;
	in[249].real = tmp_real;
	in[249].imag = tmp_imag;

	tmp_real = in[163].real;
	tmp_imag = in[163].imag;
	in[163].real = in[197].real;
	in[163].imag = in[197].imag;
	in[197].real = tmp_real;
	in[197].imag = tmp_imag;

	tmp_real = in[167].real;
	tmp_imag = in[167].imag;
	in[167].real = in[229].real;
	in[167].imag = in[229].imag;
	in[229].real = tmp_real;
	in[229].imag = tmp_imag;

	tmp_real = in[171].real;
	tmp_imag = in[171].imag;
	in[171].real = in[213].real;
	in[171].imag = in[213].imag;
	in[213].real = tmp_real;
	in[213].imag = tmp_imag;

	tmp_real = in[173].real;
	tmp_imag = in[173].imag;
	in[173].real = in[181].real;
	in[173].imag = in[181].imag;
	in[181].real = tmp_real;
	in[181].imag = tmp_imag;

	tmp_real = in[175].real;
	tmp_imag = in[175].imag;
	in[175].real = in[245].real;
	in[175].imag = in[245].imag;
	in[245].real = tmp_real;
	in[245].imag = tmp_imag;

	tmp_real = in[179].real;
	tmp_imag = in[179].imag;
	in[179].real = in[205].real;
	in[179].imag = in[205].imag;
	in[205].real = tmp_real;
	in[205].imag = tmp_imag;

	tmp_real = in[183].real;
	tmp_imag = in[183].imag;
	in[183].real = in[237].real;
	in[183].imag = in[237].imag;
	in[237].real = tmp_real;
	in[237].imag = tmp_imag;

	tmp_real = in[187].real;
	tmp_imag = in[187].imag;
	in[187].real = in[221].real;
	in[187].imag = in[221].imag;
	in[221].real = tmp_real;
	in[221].imag = tmp_imag;

	tmp_real = in[191].real;
	tmp_imag = in[191].imag;
	in[191].real = in[253].real;
	in[191].imag = in[253].imag;
	in[253].real = tmp_real;
	in[253].imag = tmp_imag;

	tmp_real = in[199].real;
	tmp_imag = in[199].imag;
	in[199].real = in[227].real;
	in[199].imag = in[227].imag;
	in[227].real = tmp_real;
	in[227].imag = tmp_imag;

	tmp_real = in[203].real;
	tmp_imag = in[203].imag;
	in[203].real = in[211].real;
	in[203].imag = in[211].imag;
	in[211].real = tmp_real;
	in[211].imag = tmp_imag;

	tmp_real = in[207].real;
	tmp_imag = in[207].imag;
	in[207].real = in[243].real;
	in[207].imag = in[243].imag;
	in[243].real = tmp_real;
	in[243].imag = tmp_imag;

	tmp_real = in[215].real;
	tmp_imag = in[215].imag;
	in[215].real = in[235].real;
	in[215].imag = in[235].imag;
	in[235].real = tmp_real;
	in[235].imag = tmp_imag;

	tmp_real = in[223].real;
	tmp_imag = in[223].imag;
	in[223].real = in[251].real;
	in[223].imag = in[251].imag;
	in[251].real = tmp_real;
	in[251].imag = tmp_imag;

	tmp_real = in[239].real;
	tmp_imag = in[239].imag;
	in[239].real = in[247].real;
	in[239].imag = in[247].imag;
	in[247].real = tmp_real;
	in[247].imag = tmp_imag;

}
