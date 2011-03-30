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
 * $Id: gfft_shuffle_512_cplx.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

void
gfft_shuffle_512_cplx(complex *in)
{
	register real tmp_real;
	register real tmp_imag;

	tmp_real = in[1].real;
	tmp_imag = in[1].imag;
	in[1].real = in[256].real;
	in[1].imag = in[256].imag;
	in[256].real = tmp_real;
	in[256].imag = tmp_imag;

	tmp_real = in[2].real;
	tmp_imag = in[2].imag;
	in[2].real = in[128].real;
	in[2].imag = in[128].imag;
	in[128].real = tmp_real;
	in[128].imag = tmp_imag;

	tmp_real = in[3].real;
	tmp_imag = in[3].imag;
	in[3].real = in[384].real;
	in[3].imag = in[384].imag;
	in[384].real = tmp_real;
	in[384].imag = tmp_imag;

	tmp_real = in[4].real;
	tmp_imag = in[4].imag;
	in[4].real = in[64].real;
	in[4].imag = in[64].imag;
	in[64].real = tmp_real;
	in[64].imag = tmp_imag;

	tmp_real = in[5].real;
	tmp_imag = in[5].imag;
	in[5].real = in[320].real;
	in[5].imag = in[320].imag;
	in[320].real = tmp_real;
	in[320].imag = tmp_imag;

	tmp_real = in[6].real;
	tmp_imag = in[6].imag;
	in[6].real = in[192].real;
	in[6].imag = in[192].imag;
	in[192].real = tmp_real;
	in[192].imag = tmp_imag;

	tmp_real = in[7].real;
	tmp_imag = in[7].imag;
	in[7].real = in[448].real;
	in[7].imag = in[448].imag;
	in[448].real = tmp_real;
	in[448].imag = tmp_imag;

	tmp_real = in[8].real;
	tmp_imag = in[8].imag;
	in[8].real = in[32].real;
	in[8].imag = in[32].imag;
	in[32].real = tmp_real;
	in[32].imag = tmp_imag;

	tmp_real = in[9].real;
	tmp_imag = in[9].imag;
	in[9].real = in[288].real;
	in[9].imag = in[288].imag;
	in[288].real = tmp_real;
	in[288].imag = tmp_imag;

	tmp_real = in[10].real;
	tmp_imag = in[10].imag;
	in[10].real = in[160].real;
	in[10].imag = in[160].imag;
	in[160].real = tmp_real;
	in[160].imag = tmp_imag;

	tmp_real = in[11].real;
	tmp_imag = in[11].imag;
	in[11].real = in[416].real;
	in[11].imag = in[416].imag;
	in[416].real = tmp_real;
	in[416].imag = tmp_imag;

	tmp_real = in[12].real;
	tmp_imag = in[12].imag;
	in[12].real = in[96].real;
	in[12].imag = in[96].imag;
	in[96].real = tmp_real;
	in[96].imag = tmp_imag;

	tmp_real = in[13].real;
	tmp_imag = in[13].imag;
	in[13].real = in[352].real;
	in[13].imag = in[352].imag;
	in[352].real = tmp_real;
	in[352].imag = tmp_imag;

	tmp_real = in[14].real;
	tmp_imag = in[14].imag;
	in[14].real = in[224].real;
	in[14].imag = in[224].imag;
	in[224].real = tmp_real;
	in[224].imag = tmp_imag;

	tmp_real = in[15].real;
	tmp_imag = in[15].imag;
	in[15].real = in[480].real;
	in[15].imag = in[480].imag;
	in[480].real = tmp_real;
	in[480].imag = tmp_imag;

	tmp_real = in[17].real;
	tmp_imag = in[17].imag;
	in[17].real = in[272].real;
	in[17].imag = in[272].imag;
	in[272].real = tmp_real;
	in[272].imag = tmp_imag;

	tmp_real = in[18].real;
	tmp_imag = in[18].imag;
	in[18].real = in[144].real;
	in[18].imag = in[144].imag;
	in[144].real = tmp_real;
	in[144].imag = tmp_imag;

	tmp_real = in[19].real;
	tmp_imag = in[19].imag;
	in[19].real = in[400].real;
	in[19].imag = in[400].imag;
	in[400].real = tmp_real;
	in[400].imag = tmp_imag;

	tmp_real = in[20].real;
	tmp_imag = in[20].imag;
	in[20].real = in[80].real;
	in[20].imag = in[80].imag;
	in[80].real = tmp_real;
	in[80].imag = tmp_imag;

	tmp_real = in[21].real;
	tmp_imag = in[21].imag;
	in[21].real = in[336].real;
	in[21].imag = in[336].imag;
	in[336].real = tmp_real;
	in[336].imag = tmp_imag;

	tmp_real = in[22].real;
	tmp_imag = in[22].imag;
	in[22].real = in[208].real;
	in[22].imag = in[208].imag;
	in[208].real = tmp_real;
	in[208].imag = tmp_imag;

	tmp_real = in[23].real;
	tmp_imag = in[23].imag;
	in[23].real = in[464].real;
	in[23].imag = in[464].imag;
	in[464].real = tmp_real;
	in[464].imag = tmp_imag;

	tmp_real = in[24].real;
	tmp_imag = in[24].imag;
	in[24].real = in[48].real;
	in[24].imag = in[48].imag;
	in[48].real = tmp_real;
	in[48].imag = tmp_imag;

	tmp_real = in[25].real;
	tmp_imag = in[25].imag;
	in[25].real = in[304].real;
	in[25].imag = in[304].imag;
	in[304].real = tmp_real;
	in[304].imag = tmp_imag;

	tmp_real = in[26].real;
	tmp_imag = in[26].imag;
	in[26].real = in[176].real;
	in[26].imag = in[176].imag;
	in[176].real = tmp_real;
	in[176].imag = tmp_imag;

	tmp_real = in[27].real;
	tmp_imag = in[27].imag;
	in[27].real = in[432].real;
	in[27].imag = in[432].imag;
	in[432].real = tmp_real;
	in[432].imag = tmp_imag;

	tmp_real = in[28].real;
	tmp_imag = in[28].imag;
	in[28].real = in[112].real;
	in[28].imag = in[112].imag;
	in[112].real = tmp_real;
	in[112].imag = tmp_imag;

	tmp_real = in[29].real;
	tmp_imag = in[29].imag;
	in[29].real = in[368].real;
	in[29].imag = in[368].imag;
	in[368].real = tmp_real;
	in[368].imag = tmp_imag;

	tmp_real = in[30].real;
	tmp_imag = in[30].imag;
	in[30].real = in[240].real;
	in[30].imag = in[240].imag;
	in[240].real = tmp_real;
	in[240].imag = tmp_imag;

	tmp_real = in[31].real;
	tmp_imag = in[31].imag;
	in[31].real = in[496].real;
	in[31].imag = in[496].imag;
	in[496].real = tmp_real;
	in[496].imag = tmp_imag;

	tmp_real = in[33].real;
	tmp_imag = in[33].imag;
	in[33].real = in[264].real;
	in[33].imag = in[264].imag;
	in[264].real = tmp_real;
	in[264].imag = tmp_imag;

	tmp_real = in[34].real;
	tmp_imag = in[34].imag;
	in[34].real = in[136].real;
	in[34].imag = in[136].imag;
	in[136].real = tmp_real;
	in[136].imag = tmp_imag;

	tmp_real = in[35].real;
	tmp_imag = in[35].imag;
	in[35].real = in[392].real;
	in[35].imag = in[392].imag;
	in[392].real = tmp_real;
	in[392].imag = tmp_imag;

	tmp_real = in[36].real;
	tmp_imag = in[36].imag;
	in[36].real = in[72].real;
	in[36].imag = in[72].imag;
	in[72].real = tmp_real;
	in[72].imag = tmp_imag;

	tmp_real = in[37].real;
	tmp_imag = in[37].imag;
	in[37].real = in[328].real;
	in[37].imag = in[328].imag;
	in[328].real = tmp_real;
	in[328].imag = tmp_imag;

	tmp_real = in[38].real;
	tmp_imag = in[38].imag;
	in[38].real = in[200].real;
	in[38].imag = in[200].imag;
	in[200].real = tmp_real;
	in[200].imag = tmp_imag;

	tmp_real = in[39].real;
	tmp_imag = in[39].imag;
	in[39].real = in[456].real;
	in[39].imag = in[456].imag;
	in[456].real = tmp_real;
	in[456].imag = tmp_imag;

	tmp_real = in[41].real;
	tmp_imag = in[41].imag;
	in[41].real = in[296].real;
	in[41].imag = in[296].imag;
	in[296].real = tmp_real;
	in[296].imag = tmp_imag;

	tmp_real = in[42].real;
	tmp_imag = in[42].imag;
	in[42].real = in[168].real;
	in[42].imag = in[168].imag;
	in[168].real = tmp_real;
	in[168].imag = tmp_imag;

	tmp_real = in[43].real;
	tmp_imag = in[43].imag;
	in[43].real = in[424].real;
	in[43].imag = in[424].imag;
	in[424].real = tmp_real;
	in[424].imag = tmp_imag;

	tmp_real = in[44].real;
	tmp_imag = in[44].imag;
	in[44].real = in[104].real;
	in[44].imag = in[104].imag;
	in[104].real = tmp_real;
	in[104].imag = tmp_imag;

	tmp_real = in[45].real;
	tmp_imag = in[45].imag;
	in[45].real = in[360].real;
	in[45].imag = in[360].imag;
	in[360].real = tmp_real;
	in[360].imag = tmp_imag;

	tmp_real = in[46].real;
	tmp_imag = in[46].imag;
	in[46].real = in[232].real;
	in[46].imag = in[232].imag;
	in[232].real = tmp_real;
	in[232].imag = tmp_imag;

	tmp_real = in[47].real;
	tmp_imag = in[47].imag;
	in[47].real = in[488].real;
	in[47].imag = in[488].imag;
	in[488].real = tmp_real;
	in[488].imag = tmp_imag;

	tmp_real = in[49].real;
	tmp_imag = in[49].imag;
	in[49].real = in[280].real;
	in[49].imag = in[280].imag;
	in[280].real = tmp_real;
	in[280].imag = tmp_imag;

	tmp_real = in[50].real;
	tmp_imag = in[50].imag;
	in[50].real = in[152].real;
	in[50].imag = in[152].imag;
	in[152].real = tmp_real;
	in[152].imag = tmp_imag;

	tmp_real = in[51].real;
	tmp_imag = in[51].imag;
	in[51].real = in[408].real;
	in[51].imag = in[408].imag;
	in[408].real = tmp_real;
	in[408].imag = tmp_imag;

	tmp_real = in[52].real;
	tmp_imag = in[52].imag;
	in[52].real = in[88].real;
	in[52].imag = in[88].imag;
	in[88].real = tmp_real;
	in[88].imag = tmp_imag;

	tmp_real = in[53].real;
	tmp_imag = in[53].imag;
	in[53].real = in[344].real;
	in[53].imag = in[344].imag;
	in[344].real = tmp_real;
	in[344].imag = tmp_imag;

	tmp_real = in[54].real;
	tmp_imag = in[54].imag;
	in[54].real = in[216].real;
	in[54].imag = in[216].imag;
	in[216].real = tmp_real;
	in[216].imag = tmp_imag;

	tmp_real = in[55].real;
	tmp_imag = in[55].imag;
	in[55].real = in[472].real;
	in[55].imag = in[472].imag;
	in[472].real = tmp_real;
	in[472].imag = tmp_imag;

	tmp_real = in[57].real;
	tmp_imag = in[57].imag;
	in[57].real = in[312].real;
	in[57].imag = in[312].imag;
	in[312].real = tmp_real;
	in[312].imag = tmp_imag;

	tmp_real = in[58].real;
	tmp_imag = in[58].imag;
	in[58].real = in[184].real;
	in[58].imag = in[184].imag;
	in[184].real = tmp_real;
	in[184].imag = tmp_imag;

	tmp_real = in[59].real;
	tmp_imag = in[59].imag;
	in[59].real = in[440].real;
	in[59].imag = in[440].imag;
	in[440].real = tmp_real;
	in[440].imag = tmp_imag;

	tmp_real = in[60].real;
	tmp_imag = in[60].imag;
	in[60].real = in[120].real;
	in[60].imag = in[120].imag;
	in[120].real = tmp_real;
	in[120].imag = tmp_imag;

	tmp_real = in[61].real;
	tmp_imag = in[61].imag;
	in[61].real = in[376].real;
	in[61].imag = in[376].imag;
	in[376].real = tmp_real;
	in[376].imag = tmp_imag;

	tmp_real = in[62].real;
	tmp_imag = in[62].imag;
	in[62].real = in[248].real;
	in[62].imag = in[248].imag;
	in[248].real = tmp_real;
	in[248].imag = tmp_imag;

	tmp_real = in[63].real;
	tmp_imag = in[63].imag;
	in[63].real = in[504].real;
	in[63].imag = in[504].imag;
	in[504].real = tmp_real;
	in[504].imag = tmp_imag;

	tmp_real = in[65].real;
	tmp_imag = in[65].imag;
	in[65].real = in[260].real;
	in[65].imag = in[260].imag;
	in[260].real = tmp_real;
	in[260].imag = tmp_imag;

	tmp_real = in[66].real;
	tmp_imag = in[66].imag;
	in[66].real = in[132].real;
	in[66].imag = in[132].imag;
	in[132].real = tmp_real;
	in[132].imag = tmp_imag;

	tmp_real = in[67].real;
	tmp_imag = in[67].imag;
	in[67].real = in[388].real;
	in[67].imag = in[388].imag;
	in[388].real = tmp_real;
	in[388].imag = tmp_imag;

	tmp_real = in[69].real;
	tmp_imag = in[69].imag;
	in[69].real = in[324].real;
	in[69].imag = in[324].imag;
	in[324].real = tmp_real;
	in[324].imag = tmp_imag;

	tmp_real = in[70].real;
	tmp_imag = in[70].imag;
	in[70].real = in[196].real;
	in[70].imag = in[196].imag;
	in[196].real = tmp_real;
	in[196].imag = tmp_imag;

	tmp_real = in[71].real;
	tmp_imag = in[71].imag;
	in[71].real = in[452].real;
	in[71].imag = in[452].imag;
	in[452].real = tmp_real;
	in[452].imag = tmp_imag;

	tmp_real = in[73].real;
	tmp_imag = in[73].imag;
	in[73].real = in[292].real;
	in[73].imag = in[292].imag;
	in[292].real = tmp_real;
	in[292].imag = tmp_imag;

	tmp_real = in[74].real;
	tmp_imag = in[74].imag;
	in[74].real = in[164].real;
	in[74].imag = in[164].imag;
	in[164].real = tmp_real;
	in[164].imag = tmp_imag;

	tmp_real = in[75].real;
	tmp_imag = in[75].imag;
	in[75].real = in[420].real;
	in[75].imag = in[420].imag;
	in[420].real = tmp_real;
	in[420].imag = tmp_imag;

	tmp_real = in[76].real;
	tmp_imag = in[76].imag;
	in[76].real = in[100].real;
	in[76].imag = in[100].imag;
	in[100].real = tmp_real;
	in[100].imag = tmp_imag;

	tmp_real = in[77].real;
	tmp_imag = in[77].imag;
	in[77].real = in[356].real;
	in[77].imag = in[356].imag;
	in[356].real = tmp_real;
	in[356].imag = tmp_imag;

	tmp_real = in[78].real;
	tmp_imag = in[78].imag;
	in[78].real = in[228].real;
	in[78].imag = in[228].imag;
	in[228].real = tmp_real;
	in[228].imag = tmp_imag;

	tmp_real = in[79].real;
	tmp_imag = in[79].imag;
	in[79].real = in[484].real;
	in[79].imag = in[484].imag;
	in[484].real = tmp_real;
	in[484].imag = tmp_imag;

	tmp_real = in[81].real;
	tmp_imag = in[81].imag;
	in[81].real = in[276].real;
	in[81].imag = in[276].imag;
	in[276].real = tmp_real;
	in[276].imag = tmp_imag;

	tmp_real = in[82].real;
	tmp_imag = in[82].imag;
	in[82].real = in[148].real;
	in[82].imag = in[148].imag;
	in[148].real = tmp_real;
	in[148].imag = tmp_imag;

	tmp_real = in[83].real;
	tmp_imag = in[83].imag;
	in[83].real = in[404].real;
	in[83].imag = in[404].imag;
	in[404].real = tmp_real;
	in[404].imag = tmp_imag;

	tmp_real = in[85].real;
	tmp_imag = in[85].imag;
	in[85].real = in[340].real;
	in[85].imag = in[340].imag;
	in[340].real = tmp_real;
	in[340].imag = tmp_imag;

	tmp_real = in[86].real;
	tmp_imag = in[86].imag;
	in[86].real = in[212].real;
	in[86].imag = in[212].imag;
	in[212].real = tmp_real;
	in[212].imag = tmp_imag;

	tmp_real = in[87].real;
	tmp_imag = in[87].imag;
	in[87].real = in[468].real;
	in[87].imag = in[468].imag;
	in[468].real = tmp_real;
	in[468].imag = tmp_imag;

	tmp_real = in[89].real;
	tmp_imag = in[89].imag;
	in[89].real = in[308].real;
	in[89].imag = in[308].imag;
	in[308].real = tmp_real;
	in[308].imag = tmp_imag;

	tmp_real = in[90].real;
	tmp_imag = in[90].imag;
	in[90].real = in[180].real;
	in[90].imag = in[180].imag;
	in[180].real = tmp_real;
	in[180].imag = tmp_imag;

	tmp_real = in[91].real;
	tmp_imag = in[91].imag;
	in[91].real = in[436].real;
	in[91].imag = in[436].imag;
	in[436].real = tmp_real;
	in[436].imag = tmp_imag;

	tmp_real = in[92].real;
	tmp_imag = in[92].imag;
	in[92].real = in[116].real;
	in[92].imag = in[116].imag;
	in[116].real = tmp_real;
	in[116].imag = tmp_imag;

	tmp_real = in[93].real;
	tmp_imag = in[93].imag;
	in[93].real = in[372].real;
	in[93].imag = in[372].imag;
	in[372].real = tmp_real;
	in[372].imag = tmp_imag;

	tmp_real = in[94].real;
	tmp_imag = in[94].imag;
	in[94].real = in[244].real;
	in[94].imag = in[244].imag;
	in[244].real = tmp_real;
	in[244].imag = tmp_imag;

	tmp_real = in[95].real;
	tmp_imag = in[95].imag;
	in[95].real = in[500].real;
	in[95].imag = in[500].imag;
	in[500].real = tmp_real;
	in[500].imag = tmp_imag;

	tmp_real = in[97].real;
	tmp_imag = in[97].imag;
	in[97].real = in[268].real;
	in[97].imag = in[268].imag;
	in[268].real = tmp_real;
	in[268].imag = tmp_imag;

	tmp_real = in[98].real;
	tmp_imag = in[98].imag;
	in[98].real = in[140].real;
	in[98].imag = in[140].imag;
	in[140].real = tmp_real;
	in[140].imag = tmp_imag;

	tmp_real = in[99].real;
	tmp_imag = in[99].imag;
	in[99].real = in[396].real;
	in[99].imag = in[396].imag;
	in[396].real = tmp_real;
	in[396].imag = tmp_imag;

	tmp_real = in[101].real;
	tmp_imag = in[101].imag;
	in[101].real = in[332].real;
	in[101].imag = in[332].imag;
	in[332].real = tmp_real;
	in[332].imag = tmp_imag;

	tmp_real = in[102].real;
	tmp_imag = in[102].imag;
	in[102].real = in[204].real;
	in[102].imag = in[204].imag;
	in[204].real = tmp_real;
	in[204].imag = tmp_imag;

	tmp_real = in[103].real;
	tmp_imag = in[103].imag;
	in[103].real = in[460].real;
	in[103].imag = in[460].imag;
	in[460].real = tmp_real;
	in[460].imag = tmp_imag;

	tmp_real = in[105].real;
	tmp_imag = in[105].imag;
	in[105].real = in[300].real;
	in[105].imag = in[300].imag;
	in[300].real = tmp_real;
	in[300].imag = tmp_imag;

	tmp_real = in[106].real;
	tmp_imag = in[106].imag;
	in[106].real = in[172].real;
	in[106].imag = in[172].imag;
	in[172].real = tmp_real;
	in[172].imag = tmp_imag;

	tmp_real = in[107].real;
	tmp_imag = in[107].imag;
	in[107].real = in[428].real;
	in[107].imag = in[428].imag;
	in[428].real = tmp_real;
	in[428].imag = tmp_imag;

	tmp_real = in[109].real;
	tmp_imag = in[109].imag;
	in[109].real = in[364].real;
	in[109].imag = in[364].imag;
	in[364].real = tmp_real;
	in[364].imag = tmp_imag;

	tmp_real = in[110].real;
	tmp_imag = in[110].imag;
	in[110].real = in[236].real;
	in[110].imag = in[236].imag;
	in[236].real = tmp_real;
	in[236].imag = tmp_imag;

	tmp_real = in[111].real;
	tmp_imag = in[111].imag;
	in[111].real = in[492].real;
	in[111].imag = in[492].imag;
	in[492].real = tmp_real;
	in[492].imag = tmp_imag;

	tmp_real = in[113].real;
	tmp_imag = in[113].imag;
	in[113].real = in[284].real;
	in[113].imag = in[284].imag;
	in[284].real = tmp_real;
	in[284].imag = tmp_imag;

	tmp_real = in[114].real;
	tmp_imag = in[114].imag;
	in[114].real = in[156].real;
	in[114].imag = in[156].imag;
	in[156].real = tmp_real;
	in[156].imag = tmp_imag;

	tmp_real = in[115].real;
	tmp_imag = in[115].imag;
	in[115].real = in[412].real;
	in[115].imag = in[412].imag;
	in[412].real = tmp_real;
	in[412].imag = tmp_imag;

	tmp_real = in[117].real;
	tmp_imag = in[117].imag;
	in[117].real = in[348].real;
	in[117].imag = in[348].imag;
	in[348].real = tmp_real;
	in[348].imag = tmp_imag;

	tmp_real = in[118].real;
	tmp_imag = in[118].imag;
	in[118].real = in[220].real;
	in[118].imag = in[220].imag;
	in[220].real = tmp_real;
	in[220].imag = tmp_imag;

	tmp_real = in[119].real;
	tmp_imag = in[119].imag;
	in[119].real = in[476].real;
	in[119].imag = in[476].imag;
	in[476].real = tmp_real;
	in[476].imag = tmp_imag;

	tmp_real = in[121].real;
	tmp_imag = in[121].imag;
	in[121].real = in[316].real;
	in[121].imag = in[316].imag;
	in[316].real = tmp_real;
	in[316].imag = tmp_imag;

	tmp_real = in[122].real;
	tmp_imag = in[122].imag;
	in[122].real = in[188].real;
	in[122].imag = in[188].imag;
	in[188].real = tmp_real;
	in[188].imag = tmp_imag;

	tmp_real = in[123].real;
	tmp_imag = in[123].imag;
	in[123].real = in[444].real;
	in[123].imag = in[444].imag;
	in[444].real = tmp_real;
	in[444].imag = tmp_imag;

	tmp_real = in[125].real;
	tmp_imag = in[125].imag;
	in[125].real = in[380].real;
	in[125].imag = in[380].imag;
	in[380].real = tmp_real;
	in[380].imag = tmp_imag;

	tmp_real = in[126].real;
	tmp_imag = in[126].imag;
	in[126].real = in[252].real;
	in[126].imag = in[252].imag;
	in[252].real = tmp_real;
	in[252].imag = tmp_imag;

	tmp_real = in[127].real;
	tmp_imag = in[127].imag;
	in[127].real = in[508].real;
	in[127].imag = in[508].imag;
	in[508].real = tmp_real;
	in[508].imag = tmp_imag;

	tmp_real = in[129].real;
	tmp_imag = in[129].imag;
	in[129].real = in[258].real;
	in[129].imag = in[258].imag;
	in[258].real = tmp_real;
	in[258].imag = tmp_imag;

	tmp_real = in[131].real;
	tmp_imag = in[131].imag;
	in[131].real = in[386].real;
	in[131].imag = in[386].imag;
	in[386].real = tmp_real;
	in[386].imag = tmp_imag;

	tmp_real = in[133].real;
	tmp_imag = in[133].imag;
	in[133].real = in[322].real;
	in[133].imag = in[322].imag;
	in[322].real = tmp_real;
	in[322].imag = tmp_imag;

	tmp_real = in[134].real;
	tmp_imag = in[134].imag;
	in[134].real = in[194].real;
	in[134].imag = in[194].imag;
	in[194].real = tmp_real;
	in[194].imag = tmp_imag;

	tmp_real = in[135].real;
	tmp_imag = in[135].imag;
	in[135].real = in[450].real;
	in[135].imag = in[450].imag;
	in[450].real = tmp_real;
	in[450].imag = tmp_imag;

	tmp_real = in[137].real;
	tmp_imag = in[137].imag;
	in[137].real = in[290].real;
	in[137].imag = in[290].imag;
	in[290].real = tmp_real;
	in[290].imag = tmp_imag;

	tmp_real = in[138].real;
	tmp_imag = in[138].imag;
	in[138].real = in[162].real;
	in[138].imag = in[162].imag;
	in[162].real = tmp_real;
	in[162].imag = tmp_imag;

	tmp_real = in[139].real;
	tmp_imag = in[139].imag;
	in[139].real = in[418].real;
	in[139].imag = in[418].imag;
	in[418].real = tmp_real;
	in[418].imag = tmp_imag;

	tmp_real = in[141].real;
	tmp_imag = in[141].imag;
	in[141].real = in[354].real;
	in[141].imag = in[354].imag;
	in[354].real = tmp_real;
	in[354].imag = tmp_imag;

	tmp_real = in[142].real;
	tmp_imag = in[142].imag;
	in[142].real = in[226].real;
	in[142].imag = in[226].imag;
	in[226].real = tmp_real;
	in[226].imag = tmp_imag;

	tmp_real = in[143].real;
	tmp_imag = in[143].imag;
	in[143].real = in[482].real;
	in[143].imag = in[482].imag;
	in[482].real = tmp_real;
	in[482].imag = tmp_imag;

	tmp_real = in[145].real;
	tmp_imag = in[145].imag;
	in[145].real = in[274].real;
	in[145].imag = in[274].imag;
	in[274].real = tmp_real;
	in[274].imag = tmp_imag;

	tmp_real = in[147].real;
	tmp_imag = in[147].imag;
	in[147].real = in[402].real;
	in[147].imag = in[402].imag;
	in[402].real = tmp_real;
	in[402].imag = tmp_imag;

	tmp_real = in[149].real;
	tmp_imag = in[149].imag;
	in[149].real = in[338].real;
	in[149].imag = in[338].imag;
	in[338].real = tmp_real;
	in[338].imag = tmp_imag;

	tmp_real = in[150].real;
	tmp_imag = in[150].imag;
	in[150].real = in[210].real;
	in[150].imag = in[210].imag;
	in[210].real = tmp_real;
	in[210].imag = tmp_imag;

	tmp_real = in[151].real;
	tmp_imag = in[151].imag;
	in[151].real = in[466].real;
	in[151].imag = in[466].imag;
	in[466].real = tmp_real;
	in[466].imag = tmp_imag;

	tmp_real = in[153].real;
	tmp_imag = in[153].imag;
	in[153].real = in[306].real;
	in[153].imag = in[306].imag;
	in[306].real = tmp_real;
	in[306].imag = tmp_imag;

	tmp_real = in[154].real;
	tmp_imag = in[154].imag;
	in[154].real = in[178].real;
	in[154].imag = in[178].imag;
	in[178].real = tmp_real;
	in[178].imag = tmp_imag;

	tmp_real = in[155].real;
	tmp_imag = in[155].imag;
	in[155].real = in[434].real;
	in[155].imag = in[434].imag;
	in[434].real = tmp_real;
	in[434].imag = tmp_imag;

	tmp_real = in[157].real;
	tmp_imag = in[157].imag;
	in[157].real = in[370].real;
	in[157].imag = in[370].imag;
	in[370].real = tmp_real;
	in[370].imag = tmp_imag;

	tmp_real = in[158].real;
	tmp_imag = in[158].imag;
	in[158].real = in[242].real;
	in[158].imag = in[242].imag;
	in[242].real = tmp_real;
	in[242].imag = tmp_imag;

	tmp_real = in[159].real;
	tmp_imag = in[159].imag;
	in[159].real = in[498].real;
	in[159].imag = in[498].imag;
	in[498].real = tmp_real;
	in[498].imag = tmp_imag;

	tmp_real = in[161].real;
	tmp_imag = in[161].imag;
	in[161].real = in[266].real;
	in[161].imag = in[266].imag;
	in[266].real = tmp_real;
	in[266].imag = tmp_imag;

	tmp_real = in[163].real;
	tmp_imag = in[163].imag;
	in[163].real = in[394].real;
	in[163].imag = in[394].imag;
	in[394].real = tmp_real;
	in[394].imag = tmp_imag;

	tmp_real = in[165].real;
	tmp_imag = in[165].imag;
	in[165].real = in[330].real;
	in[165].imag = in[330].imag;
	in[330].real = tmp_real;
	in[330].imag = tmp_imag;

	tmp_real = in[166].real;
	tmp_imag = in[166].imag;
	in[166].real = in[202].real;
	in[166].imag = in[202].imag;
	in[202].real = tmp_real;
	in[202].imag = tmp_imag;

	tmp_real = in[167].real;
	tmp_imag = in[167].imag;
	in[167].real = in[458].real;
	in[167].imag = in[458].imag;
	in[458].real = tmp_real;
	in[458].imag = tmp_imag;

	tmp_real = in[169].real;
	tmp_imag = in[169].imag;
	in[169].real = in[298].real;
	in[169].imag = in[298].imag;
	in[298].real = tmp_real;
	in[298].imag = tmp_imag;

	tmp_real = in[171].real;
	tmp_imag = in[171].imag;
	in[171].real = in[426].real;
	in[171].imag = in[426].imag;
	in[426].real = tmp_real;
	in[426].imag = tmp_imag;

	tmp_real = in[173].real;
	tmp_imag = in[173].imag;
	in[173].real = in[362].real;
	in[173].imag = in[362].imag;
	in[362].real = tmp_real;
	in[362].imag = tmp_imag;

	tmp_real = in[174].real;
	tmp_imag = in[174].imag;
	in[174].real = in[234].real;
	in[174].imag = in[234].imag;
	in[234].real = tmp_real;
	in[234].imag = tmp_imag;

	tmp_real = in[175].real;
	tmp_imag = in[175].imag;
	in[175].real = in[490].real;
	in[175].imag = in[490].imag;
	in[490].real = tmp_real;
	in[490].imag = tmp_imag;

	tmp_real = in[177].real;
	tmp_imag = in[177].imag;
	in[177].real = in[282].real;
	in[177].imag = in[282].imag;
	in[282].real = tmp_real;
	in[282].imag = tmp_imag;

	tmp_real = in[179].real;
	tmp_imag = in[179].imag;
	in[179].real = in[410].real;
	in[179].imag = in[410].imag;
	in[410].real = tmp_real;
	in[410].imag = tmp_imag;

	tmp_real = in[181].real;
	tmp_imag = in[181].imag;
	in[181].real = in[346].real;
	in[181].imag = in[346].imag;
	in[346].real = tmp_real;
	in[346].imag = tmp_imag;

	tmp_real = in[182].real;
	tmp_imag = in[182].imag;
	in[182].real = in[218].real;
	in[182].imag = in[218].imag;
	in[218].real = tmp_real;
	in[218].imag = tmp_imag;

	tmp_real = in[183].real;
	tmp_imag = in[183].imag;
	in[183].real = in[474].real;
	in[183].imag = in[474].imag;
	in[474].real = tmp_real;
	in[474].imag = tmp_imag;

	tmp_real = in[185].real;
	tmp_imag = in[185].imag;
	in[185].real = in[314].real;
	in[185].imag = in[314].imag;
	in[314].real = tmp_real;
	in[314].imag = tmp_imag;

	tmp_real = in[187].real;
	tmp_imag = in[187].imag;
	in[187].real = in[442].real;
	in[187].imag = in[442].imag;
	in[442].real = tmp_real;
	in[442].imag = tmp_imag;

	tmp_real = in[189].real;
	tmp_imag = in[189].imag;
	in[189].real = in[378].real;
	in[189].imag = in[378].imag;
	in[378].real = tmp_real;
	in[378].imag = tmp_imag;

	tmp_real = in[190].real;
	tmp_imag = in[190].imag;
	in[190].real = in[250].real;
	in[190].imag = in[250].imag;
	in[250].real = tmp_real;
	in[250].imag = tmp_imag;

	tmp_real = in[191].real;
	tmp_imag = in[191].imag;
	in[191].real = in[506].real;
	in[191].imag = in[506].imag;
	in[506].real = tmp_real;
	in[506].imag = tmp_imag;

	tmp_real = in[193].real;
	tmp_imag = in[193].imag;
	in[193].real = in[262].real;
	in[193].imag = in[262].imag;
	in[262].real = tmp_real;
	in[262].imag = tmp_imag;

	tmp_real = in[195].real;
	tmp_imag = in[195].imag;
	in[195].real = in[390].real;
	in[195].imag = in[390].imag;
	in[390].real = tmp_real;
	in[390].imag = tmp_imag;

	tmp_real = in[197].real;
	tmp_imag = in[197].imag;
	in[197].real = in[326].real;
	in[197].imag = in[326].imag;
	in[326].real = tmp_real;
	in[326].imag = tmp_imag;

	tmp_real = in[199].real;
	tmp_imag = in[199].imag;
	in[199].real = in[454].real;
	in[199].imag = in[454].imag;
	in[454].real = tmp_real;
	in[454].imag = tmp_imag;

	tmp_real = in[201].real;
	tmp_imag = in[201].imag;
	in[201].real = in[294].real;
	in[201].imag = in[294].imag;
	in[294].real = tmp_real;
	in[294].imag = tmp_imag;

	tmp_real = in[203].real;
	tmp_imag = in[203].imag;
	in[203].real = in[422].real;
	in[203].imag = in[422].imag;
	in[422].real = tmp_real;
	in[422].imag = tmp_imag;

	tmp_real = in[205].real;
	tmp_imag = in[205].imag;
	in[205].real = in[358].real;
	in[205].imag = in[358].imag;
	in[358].real = tmp_real;
	in[358].imag = tmp_imag;

	tmp_real = in[206].real;
	tmp_imag = in[206].imag;
	in[206].real = in[230].real;
	in[206].imag = in[230].imag;
	in[230].real = tmp_real;
	in[230].imag = tmp_imag;

	tmp_real = in[207].real;
	tmp_imag = in[207].imag;
	in[207].real = in[486].real;
	in[207].imag = in[486].imag;
	in[486].real = tmp_real;
	in[486].imag = tmp_imag;

	tmp_real = in[209].real;
	tmp_imag = in[209].imag;
	in[209].real = in[278].real;
	in[209].imag = in[278].imag;
	in[278].real = tmp_real;
	in[278].imag = tmp_imag;

	tmp_real = in[211].real;
	tmp_imag = in[211].imag;
	in[211].real = in[406].real;
	in[211].imag = in[406].imag;
	in[406].real = tmp_real;
	in[406].imag = tmp_imag;

	tmp_real = in[213].real;
	tmp_imag = in[213].imag;
	in[213].real = in[342].real;
	in[213].imag = in[342].imag;
	in[342].real = tmp_real;
	in[342].imag = tmp_imag;

	tmp_real = in[215].real;
	tmp_imag = in[215].imag;
	in[215].real = in[470].real;
	in[215].imag = in[470].imag;
	in[470].real = tmp_real;
	in[470].imag = tmp_imag;

	tmp_real = in[217].real;
	tmp_imag = in[217].imag;
	in[217].real = in[310].real;
	in[217].imag = in[310].imag;
	in[310].real = tmp_real;
	in[310].imag = tmp_imag;

	tmp_real = in[219].real;
	tmp_imag = in[219].imag;
	in[219].real = in[438].real;
	in[219].imag = in[438].imag;
	in[438].real = tmp_real;
	in[438].imag = tmp_imag;

	tmp_real = in[221].real;
	tmp_imag = in[221].imag;
	in[221].real = in[374].real;
	in[221].imag = in[374].imag;
	in[374].real = tmp_real;
	in[374].imag = tmp_imag;

	tmp_real = in[222].real;
	tmp_imag = in[222].imag;
	in[222].real = in[246].real;
	in[222].imag = in[246].imag;
	in[246].real = tmp_real;
	in[246].imag = tmp_imag;

	tmp_real = in[223].real;
	tmp_imag = in[223].imag;
	in[223].real = in[502].real;
	in[223].imag = in[502].imag;
	in[502].real = tmp_real;
	in[502].imag = tmp_imag;

	tmp_real = in[225].real;
	tmp_imag = in[225].imag;
	in[225].real = in[270].real;
	in[225].imag = in[270].imag;
	in[270].real = tmp_real;
	in[270].imag = tmp_imag;

	tmp_real = in[227].real;
	tmp_imag = in[227].imag;
	in[227].real = in[398].real;
	in[227].imag = in[398].imag;
	in[398].real = tmp_real;
	in[398].imag = tmp_imag;

	tmp_real = in[229].real;
	tmp_imag = in[229].imag;
	in[229].real = in[334].real;
	in[229].imag = in[334].imag;
	in[334].real = tmp_real;
	in[334].imag = tmp_imag;

	tmp_real = in[231].real;
	tmp_imag = in[231].imag;
	in[231].real = in[462].real;
	in[231].imag = in[462].imag;
	in[462].real = tmp_real;
	in[462].imag = tmp_imag;

	tmp_real = in[233].real;
	tmp_imag = in[233].imag;
	in[233].real = in[302].real;
	in[233].imag = in[302].imag;
	in[302].real = tmp_real;
	in[302].imag = tmp_imag;

	tmp_real = in[235].real;
	tmp_imag = in[235].imag;
	in[235].real = in[430].real;
	in[235].imag = in[430].imag;
	in[430].real = tmp_real;
	in[430].imag = tmp_imag;

	tmp_real = in[237].real;
	tmp_imag = in[237].imag;
	in[237].real = in[366].real;
	in[237].imag = in[366].imag;
	in[366].real = tmp_real;
	in[366].imag = tmp_imag;

	tmp_real = in[239].real;
	tmp_imag = in[239].imag;
	in[239].real = in[494].real;
	in[239].imag = in[494].imag;
	in[494].real = tmp_real;
	in[494].imag = tmp_imag;

	tmp_real = in[241].real;
	tmp_imag = in[241].imag;
	in[241].real = in[286].real;
	in[241].imag = in[286].imag;
	in[286].real = tmp_real;
	in[286].imag = tmp_imag;

	tmp_real = in[243].real;
	tmp_imag = in[243].imag;
	in[243].real = in[414].real;
	in[243].imag = in[414].imag;
	in[414].real = tmp_real;
	in[414].imag = tmp_imag;

	tmp_real = in[245].real;
	tmp_imag = in[245].imag;
	in[245].real = in[350].real;
	in[245].imag = in[350].imag;
	in[350].real = tmp_real;
	in[350].imag = tmp_imag;

	tmp_real = in[247].real;
	tmp_imag = in[247].imag;
	in[247].real = in[478].real;
	in[247].imag = in[478].imag;
	in[478].real = tmp_real;
	in[478].imag = tmp_imag;

	tmp_real = in[249].real;
	tmp_imag = in[249].imag;
	in[249].real = in[318].real;
	in[249].imag = in[318].imag;
	in[318].real = tmp_real;
	in[318].imag = tmp_imag;

	tmp_real = in[251].real;
	tmp_imag = in[251].imag;
	in[251].real = in[446].real;
	in[251].imag = in[446].imag;
	in[446].real = tmp_real;
	in[446].imag = tmp_imag;

	tmp_real = in[253].real;
	tmp_imag = in[253].imag;
	in[253].real = in[382].real;
	in[253].imag = in[382].imag;
	in[382].real = tmp_real;
	in[382].imag = tmp_imag;

	tmp_real = in[255].real;
	tmp_imag = in[255].imag;
	in[255].real = in[510].real;
	in[255].imag = in[510].imag;
	in[510].real = tmp_real;
	in[510].imag = tmp_imag;

	tmp_real = in[259].real;
	tmp_imag = in[259].imag;
	in[259].real = in[385].real;
	in[259].imag = in[385].imag;
	in[385].real = tmp_real;
	in[385].imag = tmp_imag;

	tmp_real = in[261].real;
	tmp_imag = in[261].imag;
	in[261].real = in[321].real;
	in[261].imag = in[321].imag;
	in[321].real = tmp_real;
	in[321].imag = tmp_imag;

	tmp_real = in[263].real;
	tmp_imag = in[263].imag;
	in[263].real = in[449].real;
	in[263].imag = in[449].imag;
	in[449].real = tmp_real;
	in[449].imag = tmp_imag;

	tmp_real = in[265].real;
	tmp_imag = in[265].imag;
	in[265].real = in[289].real;
	in[265].imag = in[289].imag;
	in[289].real = tmp_real;
	in[289].imag = tmp_imag;

	tmp_real = in[267].real;
	tmp_imag = in[267].imag;
	in[267].real = in[417].real;
	in[267].imag = in[417].imag;
	in[417].real = tmp_real;
	in[417].imag = tmp_imag;

	tmp_real = in[269].real;
	tmp_imag = in[269].imag;
	in[269].real = in[353].real;
	in[269].imag = in[353].imag;
	in[353].real = tmp_real;
	in[353].imag = tmp_imag;

	tmp_real = in[271].real;
	tmp_imag = in[271].imag;
	in[271].real = in[481].real;
	in[271].imag = in[481].imag;
	in[481].real = tmp_real;
	in[481].imag = tmp_imag;

	tmp_real = in[275].real;
	tmp_imag = in[275].imag;
	in[275].real = in[401].real;
	in[275].imag = in[401].imag;
	in[401].real = tmp_real;
	in[401].imag = tmp_imag;

	tmp_real = in[277].real;
	tmp_imag = in[277].imag;
	in[277].real = in[337].real;
	in[277].imag = in[337].imag;
	in[337].real = tmp_real;
	in[337].imag = tmp_imag;

	tmp_real = in[279].real;
	tmp_imag = in[279].imag;
	in[279].real = in[465].real;
	in[279].imag = in[465].imag;
	in[465].real = tmp_real;
	in[465].imag = tmp_imag;

	tmp_real = in[281].real;
	tmp_imag = in[281].imag;
	in[281].real = in[305].real;
	in[281].imag = in[305].imag;
	in[305].real = tmp_real;
	in[305].imag = tmp_imag;

	tmp_real = in[283].real;
	tmp_imag = in[283].imag;
	in[283].real = in[433].real;
	in[283].imag = in[433].imag;
	in[433].real = tmp_real;
	in[433].imag = tmp_imag;

	tmp_real = in[285].real;
	tmp_imag = in[285].imag;
	in[285].real = in[369].real;
	in[285].imag = in[369].imag;
	in[369].real = tmp_real;
	in[369].imag = tmp_imag;

	tmp_real = in[287].real;
	tmp_imag = in[287].imag;
	in[287].real = in[497].real;
	in[287].imag = in[497].imag;
	in[497].real = tmp_real;
	in[497].imag = tmp_imag;

	tmp_real = in[291].real;
	tmp_imag = in[291].imag;
	in[291].real = in[393].real;
	in[291].imag = in[393].imag;
	in[393].real = tmp_real;
	in[393].imag = tmp_imag;

	tmp_real = in[293].real;
	tmp_imag = in[293].imag;
	in[293].real = in[329].real;
	in[293].imag = in[329].imag;
	in[329].real = tmp_real;
	in[329].imag = tmp_imag;

	tmp_real = in[295].real;
	tmp_imag = in[295].imag;
	in[295].real = in[457].real;
	in[295].imag = in[457].imag;
	in[457].real = tmp_real;
	in[457].imag = tmp_imag;

	tmp_real = in[299].real;
	tmp_imag = in[299].imag;
	in[299].real = in[425].real;
	in[299].imag = in[425].imag;
	in[425].real = tmp_real;
	in[425].imag = tmp_imag;

	tmp_real = in[301].real;
	tmp_imag = in[301].imag;
	in[301].real = in[361].real;
	in[301].imag = in[361].imag;
	in[361].real = tmp_real;
	in[361].imag = tmp_imag;

	tmp_real = in[303].real;
	tmp_imag = in[303].imag;
	in[303].real = in[489].real;
	in[303].imag = in[489].imag;
	in[489].real = tmp_real;
	in[489].imag = tmp_imag;

	tmp_real = in[307].real;
	tmp_imag = in[307].imag;
	in[307].real = in[409].real;
	in[307].imag = in[409].imag;
	in[409].real = tmp_real;
	in[409].imag = tmp_imag;

	tmp_real = in[309].real;
	tmp_imag = in[309].imag;
	in[309].real = in[345].real;
	in[309].imag = in[345].imag;
	in[345].real = tmp_real;
	in[345].imag = tmp_imag;

	tmp_real = in[311].real;
	tmp_imag = in[311].imag;
	in[311].real = in[473].real;
	in[311].imag = in[473].imag;
	in[473].real = tmp_real;
	in[473].imag = tmp_imag;

	tmp_real = in[315].real;
	tmp_imag = in[315].imag;
	in[315].real = in[441].real;
	in[315].imag = in[441].imag;
	in[441].real = tmp_real;
	in[441].imag = tmp_imag;

	tmp_real = in[317].real;
	tmp_imag = in[317].imag;
	in[317].real = in[377].real;
	in[317].imag = in[377].imag;
	in[377].real = tmp_real;
	in[377].imag = tmp_imag;

	tmp_real = in[319].real;
	tmp_imag = in[319].imag;
	in[319].real = in[505].real;
	in[319].imag = in[505].imag;
	in[505].real = tmp_real;
	in[505].imag = tmp_imag;

	tmp_real = in[323].real;
	tmp_imag = in[323].imag;
	in[323].real = in[389].real;
	in[323].imag = in[389].imag;
	in[389].real = tmp_real;
	in[389].imag = tmp_imag;

	tmp_real = in[327].real;
	tmp_imag = in[327].imag;
	in[327].real = in[453].real;
	in[327].imag = in[453].imag;
	in[453].real = tmp_real;
	in[453].imag = tmp_imag;

	tmp_real = in[331].real;
	tmp_imag = in[331].imag;
	in[331].real = in[421].real;
	in[331].imag = in[421].imag;
	in[421].real = tmp_real;
	in[421].imag = tmp_imag;

	tmp_real = in[333].real;
	tmp_imag = in[333].imag;
	in[333].real = in[357].real;
	in[333].imag = in[357].imag;
	in[357].real = tmp_real;
	in[357].imag = tmp_imag;

	tmp_real = in[335].real;
	tmp_imag = in[335].imag;
	in[335].real = in[485].real;
	in[335].imag = in[485].imag;
	in[485].real = tmp_real;
	in[485].imag = tmp_imag;

	tmp_real = in[339].real;
	tmp_imag = in[339].imag;
	in[339].real = in[405].real;
	in[339].imag = in[405].imag;
	in[405].real = tmp_real;
	in[405].imag = tmp_imag;

	tmp_real = in[343].real;
	tmp_imag = in[343].imag;
	in[343].real = in[469].real;
	in[343].imag = in[469].imag;
	in[469].real = tmp_real;
	in[469].imag = tmp_imag;

	tmp_real = in[347].real;
	tmp_imag = in[347].imag;
	in[347].real = in[437].real;
	in[347].imag = in[437].imag;
	in[437].real = tmp_real;
	in[437].imag = tmp_imag;

	tmp_real = in[349].real;
	tmp_imag = in[349].imag;
	in[349].real = in[373].real;
	in[349].imag = in[373].imag;
	in[373].real = tmp_real;
	in[373].imag = tmp_imag;

	tmp_real = in[351].real;
	tmp_imag = in[351].imag;
	in[351].real = in[501].real;
	in[351].imag = in[501].imag;
	in[501].real = tmp_real;
	in[501].imag = tmp_imag;

	tmp_real = in[355].real;
	tmp_imag = in[355].imag;
	in[355].real = in[397].real;
	in[355].imag = in[397].imag;
	in[397].real = tmp_real;
	in[397].imag = tmp_imag;

	tmp_real = in[359].real;
	tmp_imag = in[359].imag;
	in[359].real = in[461].real;
	in[359].imag = in[461].imag;
	in[461].real = tmp_real;
	in[461].imag = tmp_imag;

	tmp_real = in[363].real;
	tmp_imag = in[363].imag;
	in[363].real = in[429].real;
	in[363].imag = in[429].imag;
	in[429].real = tmp_real;
	in[429].imag = tmp_imag;

	tmp_real = in[367].real;
	tmp_imag = in[367].imag;
	in[367].real = in[493].real;
	in[367].imag = in[493].imag;
	in[493].real = tmp_real;
	in[493].imag = tmp_imag;

	tmp_real = in[371].real;
	tmp_imag = in[371].imag;
	in[371].real = in[413].real;
	in[371].imag = in[413].imag;
	in[413].real = tmp_real;
	in[413].imag = tmp_imag;

	tmp_real = in[375].real;
	tmp_imag = in[375].imag;
	in[375].real = in[477].real;
	in[375].imag = in[477].imag;
	in[477].real = tmp_real;
	in[477].imag = tmp_imag;

	tmp_real = in[379].real;
	tmp_imag = in[379].imag;
	in[379].real = in[445].real;
	in[379].imag = in[445].imag;
	in[445].real = tmp_real;
	in[445].imag = tmp_imag;

	tmp_real = in[383].real;
	tmp_imag = in[383].imag;
	in[383].real = in[509].real;
	in[383].imag = in[509].imag;
	in[509].real = tmp_real;
	in[509].imag = tmp_imag;

	tmp_real = in[391].real;
	tmp_imag = in[391].imag;
	in[391].real = in[451].real;
	in[391].imag = in[451].imag;
	in[451].real = tmp_real;
	in[451].imag = tmp_imag;

	tmp_real = in[395].real;
	tmp_imag = in[395].imag;
	in[395].real = in[419].real;
	in[395].imag = in[419].imag;
	in[419].real = tmp_real;
	in[419].imag = tmp_imag;

	tmp_real = in[399].real;
	tmp_imag = in[399].imag;
	in[399].real = in[483].real;
	in[399].imag = in[483].imag;
	in[483].real = tmp_real;
	in[483].imag = tmp_imag;

	tmp_real = in[407].real;
	tmp_imag = in[407].imag;
	in[407].real = in[467].real;
	in[407].imag = in[467].imag;
	in[467].real = tmp_real;
	in[467].imag = tmp_imag;

	tmp_real = in[411].real;
	tmp_imag = in[411].imag;
	in[411].real = in[435].real;
	in[411].imag = in[435].imag;
	in[435].real = tmp_real;
	in[435].imag = tmp_imag;

	tmp_real = in[415].real;
	tmp_imag = in[415].imag;
	in[415].real = in[499].real;
	in[415].imag = in[499].imag;
	in[499].real = tmp_real;
	in[499].imag = tmp_imag;

	tmp_real = in[423].real;
	tmp_imag = in[423].imag;
	in[423].real = in[459].real;
	in[423].imag = in[459].imag;
	in[459].real = tmp_real;
	in[459].imag = tmp_imag;

	tmp_real = in[431].real;
	tmp_imag = in[431].imag;
	in[431].real = in[491].real;
	in[431].imag = in[491].imag;
	in[491].real = tmp_real;
	in[491].imag = tmp_imag;

	tmp_real = in[439].real;
	tmp_imag = in[439].imag;
	in[439].real = in[475].real;
	in[439].imag = in[475].imag;
	in[475].real = tmp_real;
	in[475].imag = tmp_imag;

	tmp_real = in[447].real;
	tmp_imag = in[447].imag;
	in[447].real = in[507].real;
	in[447].imag = in[507].imag;
	in[507].real = tmp_real;
	in[507].imag = tmp_imag;

	tmp_real = in[463].real;
	tmp_imag = in[463].imag;
	in[463].real = in[487].real;
	in[463].imag = in[487].imag;
	in[487].real = tmp_real;
	in[487].imag = tmp_imag;

	tmp_real = in[479].real;
	tmp_imag = in[479].imag;
	in[479].real = in[503].real;
	in[479].imag = in[503].imag;
	in[503].real = tmp_real;
	in[503].imag = tmp_imag;

}
