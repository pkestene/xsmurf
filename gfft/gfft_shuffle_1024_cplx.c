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
 * $Id: gfft_shuffle_1024_cplx.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <gfft.h>

void
gfft_shuffle_1024_cplx(complex *in)
{
	register real tmp_real;
	register real tmp_imag;

	tmp_real = in[1].real;
	tmp_imag = in[1].imag;
	in[1].real = in[512].real;
	in[1].imag = in[512].imag;
	in[512].real = tmp_real;
	in[512].imag = tmp_imag;

	tmp_real = in[2].real;
	tmp_imag = in[2].imag;
	in[2].real = in[256].real;
	in[2].imag = in[256].imag;
	in[256].real = tmp_real;
	in[256].imag = tmp_imag;

	tmp_real = in[3].real;
	tmp_imag = in[3].imag;
	in[3].real = in[768].real;
	in[3].imag = in[768].imag;
	in[768].real = tmp_real;
	in[768].imag = tmp_imag;

	tmp_real = in[4].real;
	tmp_imag = in[4].imag;
	in[4].real = in[128].real;
	in[4].imag = in[128].imag;
	in[128].real = tmp_real;
	in[128].imag = tmp_imag;

	tmp_real = in[5].real;
	tmp_imag = in[5].imag;
	in[5].real = in[640].real;
	in[5].imag = in[640].imag;
	in[640].real = tmp_real;
	in[640].imag = tmp_imag;

	tmp_real = in[6].real;
	tmp_imag = in[6].imag;
	in[6].real = in[384].real;
	in[6].imag = in[384].imag;
	in[384].real = tmp_real;
	in[384].imag = tmp_imag;

	tmp_real = in[7].real;
	tmp_imag = in[7].imag;
	in[7].real = in[896].real;
	in[7].imag = in[896].imag;
	in[896].real = tmp_real;
	in[896].imag = tmp_imag;

	tmp_real = in[8].real;
	tmp_imag = in[8].imag;
	in[8].real = in[64].real;
	in[8].imag = in[64].imag;
	in[64].real = tmp_real;
	in[64].imag = tmp_imag;

	tmp_real = in[9].real;
	tmp_imag = in[9].imag;
	in[9].real = in[576].real;
	in[9].imag = in[576].imag;
	in[576].real = tmp_real;
	in[576].imag = tmp_imag;

	tmp_real = in[10].real;
	tmp_imag = in[10].imag;
	in[10].real = in[320].real;
	in[10].imag = in[320].imag;
	in[320].real = tmp_real;
	in[320].imag = tmp_imag;

	tmp_real = in[11].real;
	tmp_imag = in[11].imag;
	in[11].real = in[832].real;
	in[11].imag = in[832].imag;
	in[832].real = tmp_real;
	in[832].imag = tmp_imag;

	tmp_real = in[12].real;
	tmp_imag = in[12].imag;
	in[12].real = in[192].real;
	in[12].imag = in[192].imag;
	in[192].real = tmp_real;
	in[192].imag = tmp_imag;

	tmp_real = in[13].real;
	tmp_imag = in[13].imag;
	in[13].real = in[704].real;
	in[13].imag = in[704].imag;
	in[704].real = tmp_real;
	in[704].imag = tmp_imag;

	tmp_real = in[14].real;
	tmp_imag = in[14].imag;
	in[14].real = in[448].real;
	in[14].imag = in[448].imag;
	in[448].real = tmp_real;
	in[448].imag = tmp_imag;

	tmp_real = in[15].real;
	tmp_imag = in[15].imag;
	in[15].real = in[960].real;
	in[15].imag = in[960].imag;
	in[960].real = tmp_real;
	in[960].imag = tmp_imag;

	tmp_real = in[16].real;
	tmp_imag = in[16].imag;
	in[16].real = in[32].real;
	in[16].imag = in[32].imag;
	in[32].real = tmp_real;
	in[32].imag = tmp_imag;

	tmp_real = in[17].real;
	tmp_imag = in[17].imag;
	in[17].real = in[544].real;
	in[17].imag = in[544].imag;
	in[544].real = tmp_real;
	in[544].imag = tmp_imag;

	tmp_real = in[18].real;
	tmp_imag = in[18].imag;
	in[18].real = in[288].real;
	in[18].imag = in[288].imag;
	in[288].real = tmp_real;
	in[288].imag = tmp_imag;

	tmp_real = in[19].real;
	tmp_imag = in[19].imag;
	in[19].real = in[800].real;
	in[19].imag = in[800].imag;
	in[800].real = tmp_real;
	in[800].imag = tmp_imag;

	tmp_real = in[20].real;
	tmp_imag = in[20].imag;
	in[20].real = in[160].real;
	in[20].imag = in[160].imag;
	in[160].real = tmp_real;
	in[160].imag = tmp_imag;

	tmp_real = in[21].real;
	tmp_imag = in[21].imag;
	in[21].real = in[672].real;
	in[21].imag = in[672].imag;
	in[672].real = tmp_real;
	in[672].imag = tmp_imag;

	tmp_real = in[22].real;
	tmp_imag = in[22].imag;
	in[22].real = in[416].real;
	in[22].imag = in[416].imag;
	in[416].real = tmp_real;
	in[416].imag = tmp_imag;

	tmp_real = in[23].real;
	tmp_imag = in[23].imag;
	in[23].real = in[928].real;
	in[23].imag = in[928].imag;
	in[928].real = tmp_real;
	in[928].imag = tmp_imag;

	tmp_real = in[24].real;
	tmp_imag = in[24].imag;
	in[24].real = in[96].real;
	in[24].imag = in[96].imag;
	in[96].real = tmp_real;
	in[96].imag = tmp_imag;

	tmp_real = in[25].real;
	tmp_imag = in[25].imag;
	in[25].real = in[608].real;
	in[25].imag = in[608].imag;
	in[608].real = tmp_real;
	in[608].imag = tmp_imag;

	tmp_real = in[26].real;
	tmp_imag = in[26].imag;
	in[26].real = in[352].real;
	in[26].imag = in[352].imag;
	in[352].real = tmp_real;
	in[352].imag = tmp_imag;

	tmp_real = in[27].real;
	tmp_imag = in[27].imag;
	in[27].real = in[864].real;
	in[27].imag = in[864].imag;
	in[864].real = tmp_real;
	in[864].imag = tmp_imag;

	tmp_real = in[28].real;
	tmp_imag = in[28].imag;
	in[28].real = in[224].real;
	in[28].imag = in[224].imag;
	in[224].real = tmp_real;
	in[224].imag = tmp_imag;

	tmp_real = in[29].real;
	tmp_imag = in[29].imag;
	in[29].real = in[736].real;
	in[29].imag = in[736].imag;
	in[736].real = tmp_real;
	in[736].imag = tmp_imag;

	tmp_real = in[30].real;
	tmp_imag = in[30].imag;
	in[30].real = in[480].real;
	in[30].imag = in[480].imag;
	in[480].real = tmp_real;
	in[480].imag = tmp_imag;

	tmp_real = in[31].real;
	tmp_imag = in[31].imag;
	in[31].real = in[992].real;
	in[31].imag = in[992].imag;
	in[992].real = tmp_real;
	in[992].imag = tmp_imag;

	tmp_real = in[33].real;
	tmp_imag = in[33].imag;
	in[33].real = in[528].real;
	in[33].imag = in[528].imag;
	in[528].real = tmp_real;
	in[528].imag = tmp_imag;

	tmp_real = in[34].real;
	tmp_imag = in[34].imag;
	in[34].real = in[272].real;
	in[34].imag = in[272].imag;
	in[272].real = tmp_real;
	in[272].imag = tmp_imag;

	tmp_real = in[35].real;
	tmp_imag = in[35].imag;
	in[35].real = in[784].real;
	in[35].imag = in[784].imag;
	in[784].real = tmp_real;
	in[784].imag = tmp_imag;

	tmp_real = in[36].real;
	tmp_imag = in[36].imag;
	in[36].real = in[144].real;
	in[36].imag = in[144].imag;
	in[144].real = tmp_real;
	in[144].imag = tmp_imag;

	tmp_real = in[37].real;
	tmp_imag = in[37].imag;
	in[37].real = in[656].real;
	in[37].imag = in[656].imag;
	in[656].real = tmp_real;
	in[656].imag = tmp_imag;

	tmp_real = in[38].real;
	tmp_imag = in[38].imag;
	in[38].real = in[400].real;
	in[38].imag = in[400].imag;
	in[400].real = tmp_real;
	in[400].imag = tmp_imag;

	tmp_real = in[39].real;
	tmp_imag = in[39].imag;
	in[39].real = in[912].real;
	in[39].imag = in[912].imag;
	in[912].real = tmp_real;
	in[912].imag = tmp_imag;

	tmp_real = in[40].real;
	tmp_imag = in[40].imag;
	in[40].real = in[80].real;
	in[40].imag = in[80].imag;
	in[80].real = tmp_real;
	in[80].imag = tmp_imag;

	tmp_real = in[41].real;
	tmp_imag = in[41].imag;
	in[41].real = in[592].real;
	in[41].imag = in[592].imag;
	in[592].real = tmp_real;
	in[592].imag = tmp_imag;

	tmp_real = in[42].real;
	tmp_imag = in[42].imag;
	in[42].real = in[336].real;
	in[42].imag = in[336].imag;
	in[336].real = tmp_real;
	in[336].imag = tmp_imag;

	tmp_real = in[43].real;
	tmp_imag = in[43].imag;
	in[43].real = in[848].real;
	in[43].imag = in[848].imag;
	in[848].real = tmp_real;
	in[848].imag = tmp_imag;

	tmp_real = in[44].real;
	tmp_imag = in[44].imag;
	in[44].real = in[208].real;
	in[44].imag = in[208].imag;
	in[208].real = tmp_real;
	in[208].imag = tmp_imag;

	tmp_real = in[45].real;
	tmp_imag = in[45].imag;
	in[45].real = in[720].real;
	in[45].imag = in[720].imag;
	in[720].real = tmp_real;
	in[720].imag = tmp_imag;

	tmp_real = in[46].real;
	tmp_imag = in[46].imag;
	in[46].real = in[464].real;
	in[46].imag = in[464].imag;
	in[464].real = tmp_real;
	in[464].imag = tmp_imag;

	tmp_real = in[47].real;
	tmp_imag = in[47].imag;
	in[47].real = in[976].real;
	in[47].imag = in[976].imag;
	in[976].real = tmp_real;
	in[976].imag = tmp_imag;

	tmp_real = in[49].real;
	tmp_imag = in[49].imag;
	in[49].real = in[560].real;
	in[49].imag = in[560].imag;
	in[560].real = tmp_real;
	in[560].imag = tmp_imag;

	tmp_real = in[50].real;
	tmp_imag = in[50].imag;
	in[50].real = in[304].real;
	in[50].imag = in[304].imag;
	in[304].real = tmp_real;
	in[304].imag = tmp_imag;

	tmp_real = in[51].real;
	tmp_imag = in[51].imag;
	in[51].real = in[816].real;
	in[51].imag = in[816].imag;
	in[816].real = tmp_real;
	in[816].imag = tmp_imag;

	tmp_real = in[52].real;
	tmp_imag = in[52].imag;
	in[52].real = in[176].real;
	in[52].imag = in[176].imag;
	in[176].real = tmp_real;
	in[176].imag = tmp_imag;

	tmp_real = in[53].real;
	tmp_imag = in[53].imag;
	in[53].real = in[688].real;
	in[53].imag = in[688].imag;
	in[688].real = tmp_real;
	in[688].imag = tmp_imag;

	tmp_real = in[54].real;
	tmp_imag = in[54].imag;
	in[54].real = in[432].real;
	in[54].imag = in[432].imag;
	in[432].real = tmp_real;
	in[432].imag = tmp_imag;

	tmp_real = in[55].real;
	tmp_imag = in[55].imag;
	in[55].real = in[944].real;
	in[55].imag = in[944].imag;
	in[944].real = tmp_real;
	in[944].imag = tmp_imag;

	tmp_real = in[56].real;
	tmp_imag = in[56].imag;
	in[56].real = in[112].real;
	in[56].imag = in[112].imag;
	in[112].real = tmp_real;
	in[112].imag = tmp_imag;

	tmp_real = in[57].real;
	tmp_imag = in[57].imag;
	in[57].real = in[624].real;
	in[57].imag = in[624].imag;
	in[624].real = tmp_real;
	in[624].imag = tmp_imag;

	tmp_real = in[58].real;
	tmp_imag = in[58].imag;
	in[58].real = in[368].real;
	in[58].imag = in[368].imag;
	in[368].real = tmp_real;
	in[368].imag = tmp_imag;

	tmp_real = in[59].real;
	tmp_imag = in[59].imag;
	in[59].real = in[880].real;
	in[59].imag = in[880].imag;
	in[880].real = tmp_real;
	in[880].imag = tmp_imag;

	tmp_real = in[60].real;
	tmp_imag = in[60].imag;
	in[60].real = in[240].real;
	in[60].imag = in[240].imag;
	in[240].real = tmp_real;
	in[240].imag = tmp_imag;

	tmp_real = in[61].real;
	tmp_imag = in[61].imag;
	in[61].real = in[752].real;
	in[61].imag = in[752].imag;
	in[752].real = tmp_real;
	in[752].imag = tmp_imag;

	tmp_real = in[62].real;
	tmp_imag = in[62].imag;
	in[62].real = in[496].real;
	in[62].imag = in[496].imag;
	in[496].real = tmp_real;
	in[496].imag = tmp_imag;

	tmp_real = in[63].real;
	tmp_imag = in[63].imag;
	in[63].real = in[1008].real;
	in[63].imag = in[1008].imag;
	in[1008].real = tmp_real;
	in[1008].imag = tmp_imag;

	tmp_real = in[65].real;
	tmp_imag = in[65].imag;
	in[65].real = in[520].real;
	in[65].imag = in[520].imag;
	in[520].real = tmp_real;
	in[520].imag = tmp_imag;

	tmp_real = in[66].real;
	tmp_imag = in[66].imag;
	in[66].real = in[264].real;
	in[66].imag = in[264].imag;
	in[264].real = tmp_real;
	in[264].imag = tmp_imag;

	tmp_real = in[67].real;
	tmp_imag = in[67].imag;
	in[67].real = in[776].real;
	in[67].imag = in[776].imag;
	in[776].real = tmp_real;
	in[776].imag = tmp_imag;

	tmp_real = in[68].real;
	tmp_imag = in[68].imag;
	in[68].real = in[136].real;
	in[68].imag = in[136].imag;
	in[136].real = tmp_real;
	in[136].imag = tmp_imag;

	tmp_real = in[69].real;
	tmp_imag = in[69].imag;
	in[69].real = in[648].real;
	in[69].imag = in[648].imag;
	in[648].real = tmp_real;
	in[648].imag = tmp_imag;

	tmp_real = in[70].real;
	tmp_imag = in[70].imag;
	in[70].real = in[392].real;
	in[70].imag = in[392].imag;
	in[392].real = tmp_real;
	in[392].imag = tmp_imag;

	tmp_real = in[71].real;
	tmp_imag = in[71].imag;
	in[71].real = in[904].real;
	in[71].imag = in[904].imag;
	in[904].real = tmp_real;
	in[904].imag = tmp_imag;

	tmp_real = in[73].real;
	tmp_imag = in[73].imag;
	in[73].real = in[584].real;
	in[73].imag = in[584].imag;
	in[584].real = tmp_real;
	in[584].imag = tmp_imag;

	tmp_real = in[74].real;
	tmp_imag = in[74].imag;
	in[74].real = in[328].real;
	in[74].imag = in[328].imag;
	in[328].real = tmp_real;
	in[328].imag = tmp_imag;

	tmp_real = in[75].real;
	tmp_imag = in[75].imag;
	in[75].real = in[840].real;
	in[75].imag = in[840].imag;
	in[840].real = tmp_real;
	in[840].imag = tmp_imag;

	tmp_real = in[76].real;
	tmp_imag = in[76].imag;
	in[76].real = in[200].real;
	in[76].imag = in[200].imag;
	in[200].real = tmp_real;
	in[200].imag = tmp_imag;

	tmp_real = in[77].real;
	tmp_imag = in[77].imag;
	in[77].real = in[712].real;
	in[77].imag = in[712].imag;
	in[712].real = tmp_real;
	in[712].imag = tmp_imag;

	tmp_real = in[78].real;
	tmp_imag = in[78].imag;
	in[78].real = in[456].real;
	in[78].imag = in[456].imag;
	in[456].real = tmp_real;
	in[456].imag = tmp_imag;

	tmp_real = in[79].real;
	tmp_imag = in[79].imag;
	in[79].real = in[968].real;
	in[79].imag = in[968].imag;
	in[968].real = tmp_real;
	in[968].imag = tmp_imag;

	tmp_real = in[81].real;
	tmp_imag = in[81].imag;
	in[81].real = in[552].real;
	in[81].imag = in[552].imag;
	in[552].real = tmp_real;
	in[552].imag = tmp_imag;

	tmp_real = in[82].real;
	tmp_imag = in[82].imag;
	in[82].real = in[296].real;
	in[82].imag = in[296].imag;
	in[296].real = tmp_real;
	in[296].imag = tmp_imag;

	tmp_real = in[83].real;
	tmp_imag = in[83].imag;
	in[83].real = in[808].real;
	in[83].imag = in[808].imag;
	in[808].real = tmp_real;
	in[808].imag = tmp_imag;

	tmp_real = in[84].real;
	tmp_imag = in[84].imag;
	in[84].real = in[168].real;
	in[84].imag = in[168].imag;
	in[168].real = tmp_real;
	in[168].imag = tmp_imag;

	tmp_real = in[85].real;
	tmp_imag = in[85].imag;
	in[85].real = in[680].real;
	in[85].imag = in[680].imag;
	in[680].real = tmp_real;
	in[680].imag = tmp_imag;

	tmp_real = in[86].real;
	tmp_imag = in[86].imag;
	in[86].real = in[424].real;
	in[86].imag = in[424].imag;
	in[424].real = tmp_real;
	in[424].imag = tmp_imag;

	tmp_real = in[87].real;
	tmp_imag = in[87].imag;
	in[87].real = in[936].real;
	in[87].imag = in[936].imag;
	in[936].real = tmp_real;
	in[936].imag = tmp_imag;

	tmp_real = in[88].real;
	tmp_imag = in[88].imag;
	in[88].real = in[104].real;
	in[88].imag = in[104].imag;
	in[104].real = tmp_real;
	in[104].imag = tmp_imag;

	tmp_real = in[89].real;
	tmp_imag = in[89].imag;
	in[89].real = in[616].real;
	in[89].imag = in[616].imag;
	in[616].real = tmp_real;
	in[616].imag = tmp_imag;

	tmp_real = in[90].real;
	tmp_imag = in[90].imag;
	in[90].real = in[360].real;
	in[90].imag = in[360].imag;
	in[360].real = tmp_real;
	in[360].imag = tmp_imag;

	tmp_real = in[91].real;
	tmp_imag = in[91].imag;
	in[91].real = in[872].real;
	in[91].imag = in[872].imag;
	in[872].real = tmp_real;
	in[872].imag = tmp_imag;

	tmp_real = in[92].real;
	tmp_imag = in[92].imag;
	in[92].real = in[232].real;
	in[92].imag = in[232].imag;
	in[232].real = tmp_real;
	in[232].imag = tmp_imag;

	tmp_real = in[93].real;
	tmp_imag = in[93].imag;
	in[93].real = in[744].real;
	in[93].imag = in[744].imag;
	in[744].real = tmp_real;
	in[744].imag = tmp_imag;

	tmp_real = in[94].real;
	tmp_imag = in[94].imag;
	in[94].real = in[488].real;
	in[94].imag = in[488].imag;
	in[488].real = tmp_real;
	in[488].imag = tmp_imag;

	tmp_real = in[95].real;
	tmp_imag = in[95].imag;
	in[95].real = in[1000].real;
	in[95].imag = in[1000].imag;
	in[1000].real = tmp_real;
	in[1000].imag = tmp_imag;

	tmp_real = in[97].real;
	tmp_imag = in[97].imag;
	in[97].real = in[536].real;
	in[97].imag = in[536].imag;
	in[536].real = tmp_real;
	in[536].imag = tmp_imag;

	tmp_real = in[98].real;
	tmp_imag = in[98].imag;
	in[98].real = in[280].real;
	in[98].imag = in[280].imag;
	in[280].real = tmp_real;
	in[280].imag = tmp_imag;

	tmp_real = in[99].real;
	tmp_imag = in[99].imag;
	in[99].real = in[792].real;
	in[99].imag = in[792].imag;
	in[792].real = tmp_real;
	in[792].imag = tmp_imag;

	tmp_real = in[100].real;
	tmp_imag = in[100].imag;
	in[100].real = in[152].real;
	in[100].imag = in[152].imag;
	in[152].real = tmp_real;
	in[152].imag = tmp_imag;

	tmp_real = in[101].real;
	tmp_imag = in[101].imag;
	in[101].real = in[664].real;
	in[101].imag = in[664].imag;
	in[664].real = tmp_real;
	in[664].imag = tmp_imag;

	tmp_real = in[102].real;
	tmp_imag = in[102].imag;
	in[102].real = in[408].real;
	in[102].imag = in[408].imag;
	in[408].real = tmp_real;
	in[408].imag = tmp_imag;

	tmp_real = in[103].real;
	tmp_imag = in[103].imag;
	in[103].real = in[920].real;
	in[103].imag = in[920].imag;
	in[920].real = tmp_real;
	in[920].imag = tmp_imag;

	tmp_real = in[105].real;
	tmp_imag = in[105].imag;
	in[105].real = in[600].real;
	in[105].imag = in[600].imag;
	in[600].real = tmp_real;
	in[600].imag = tmp_imag;

	tmp_real = in[106].real;
	tmp_imag = in[106].imag;
	in[106].real = in[344].real;
	in[106].imag = in[344].imag;
	in[344].real = tmp_real;
	in[344].imag = tmp_imag;

	tmp_real = in[107].real;
	tmp_imag = in[107].imag;
	in[107].real = in[856].real;
	in[107].imag = in[856].imag;
	in[856].real = tmp_real;
	in[856].imag = tmp_imag;

	tmp_real = in[108].real;
	tmp_imag = in[108].imag;
	in[108].real = in[216].real;
	in[108].imag = in[216].imag;
	in[216].real = tmp_real;
	in[216].imag = tmp_imag;

	tmp_real = in[109].real;
	tmp_imag = in[109].imag;
	in[109].real = in[728].real;
	in[109].imag = in[728].imag;
	in[728].real = tmp_real;
	in[728].imag = tmp_imag;

	tmp_real = in[110].real;
	tmp_imag = in[110].imag;
	in[110].real = in[472].real;
	in[110].imag = in[472].imag;
	in[472].real = tmp_real;
	in[472].imag = tmp_imag;

	tmp_real = in[111].real;
	tmp_imag = in[111].imag;
	in[111].real = in[984].real;
	in[111].imag = in[984].imag;
	in[984].real = tmp_real;
	in[984].imag = tmp_imag;

	tmp_real = in[113].real;
	tmp_imag = in[113].imag;
	in[113].real = in[568].real;
	in[113].imag = in[568].imag;
	in[568].real = tmp_real;
	in[568].imag = tmp_imag;

	tmp_real = in[114].real;
	tmp_imag = in[114].imag;
	in[114].real = in[312].real;
	in[114].imag = in[312].imag;
	in[312].real = tmp_real;
	in[312].imag = tmp_imag;

	tmp_real = in[115].real;
	tmp_imag = in[115].imag;
	in[115].real = in[824].real;
	in[115].imag = in[824].imag;
	in[824].real = tmp_real;
	in[824].imag = tmp_imag;

	tmp_real = in[116].real;
	tmp_imag = in[116].imag;
	in[116].real = in[184].real;
	in[116].imag = in[184].imag;
	in[184].real = tmp_real;
	in[184].imag = tmp_imag;

	tmp_real = in[117].real;
	tmp_imag = in[117].imag;
	in[117].real = in[696].real;
	in[117].imag = in[696].imag;
	in[696].real = tmp_real;
	in[696].imag = tmp_imag;

	tmp_real = in[118].real;
	tmp_imag = in[118].imag;
	in[118].real = in[440].real;
	in[118].imag = in[440].imag;
	in[440].real = tmp_real;
	in[440].imag = tmp_imag;

	tmp_real = in[119].real;
	tmp_imag = in[119].imag;
	in[119].real = in[952].real;
	in[119].imag = in[952].imag;
	in[952].real = tmp_real;
	in[952].imag = tmp_imag;

	tmp_real = in[121].real;
	tmp_imag = in[121].imag;
	in[121].real = in[632].real;
	in[121].imag = in[632].imag;
	in[632].real = tmp_real;
	in[632].imag = tmp_imag;

	tmp_real = in[122].real;
	tmp_imag = in[122].imag;
	in[122].real = in[376].real;
	in[122].imag = in[376].imag;
	in[376].real = tmp_real;
	in[376].imag = tmp_imag;

	tmp_real = in[123].real;
	tmp_imag = in[123].imag;
	in[123].real = in[888].real;
	in[123].imag = in[888].imag;
	in[888].real = tmp_real;
	in[888].imag = tmp_imag;

	tmp_real = in[124].real;
	tmp_imag = in[124].imag;
	in[124].real = in[248].real;
	in[124].imag = in[248].imag;
	in[248].real = tmp_real;
	in[248].imag = tmp_imag;

	tmp_real = in[125].real;
	tmp_imag = in[125].imag;
	in[125].real = in[760].real;
	in[125].imag = in[760].imag;
	in[760].real = tmp_real;
	in[760].imag = tmp_imag;

	tmp_real = in[126].real;
	tmp_imag = in[126].imag;
	in[126].real = in[504].real;
	in[126].imag = in[504].imag;
	in[504].real = tmp_real;
	in[504].imag = tmp_imag;

	tmp_real = in[127].real;
	tmp_imag = in[127].imag;
	in[127].real = in[1016].real;
	in[127].imag = in[1016].imag;
	in[1016].real = tmp_real;
	in[1016].imag = tmp_imag;

	tmp_real = in[129].real;
	tmp_imag = in[129].imag;
	in[129].real = in[516].real;
	in[129].imag = in[516].imag;
	in[516].real = tmp_real;
	in[516].imag = tmp_imag;

	tmp_real = in[130].real;
	tmp_imag = in[130].imag;
	in[130].real = in[260].real;
	in[130].imag = in[260].imag;
	in[260].real = tmp_real;
	in[260].imag = tmp_imag;

	tmp_real = in[131].real;
	tmp_imag = in[131].imag;
	in[131].real = in[772].real;
	in[131].imag = in[772].imag;
	in[772].real = tmp_real;
	in[772].imag = tmp_imag;

	tmp_real = in[133].real;
	tmp_imag = in[133].imag;
	in[133].real = in[644].real;
	in[133].imag = in[644].imag;
	in[644].real = tmp_real;
	in[644].imag = tmp_imag;

	tmp_real = in[134].real;
	tmp_imag = in[134].imag;
	in[134].real = in[388].real;
	in[134].imag = in[388].imag;
	in[388].real = tmp_real;
	in[388].imag = tmp_imag;

	tmp_real = in[135].real;
	tmp_imag = in[135].imag;
	in[135].real = in[900].real;
	in[135].imag = in[900].imag;
	in[900].real = tmp_real;
	in[900].imag = tmp_imag;

	tmp_real = in[137].real;
	tmp_imag = in[137].imag;
	in[137].real = in[580].real;
	in[137].imag = in[580].imag;
	in[580].real = tmp_real;
	in[580].imag = tmp_imag;

	tmp_real = in[138].real;
	tmp_imag = in[138].imag;
	in[138].real = in[324].real;
	in[138].imag = in[324].imag;
	in[324].real = tmp_real;
	in[324].imag = tmp_imag;

	tmp_real = in[139].real;
	tmp_imag = in[139].imag;
	in[139].real = in[836].real;
	in[139].imag = in[836].imag;
	in[836].real = tmp_real;
	in[836].imag = tmp_imag;

	tmp_real = in[140].real;
	tmp_imag = in[140].imag;
	in[140].real = in[196].real;
	in[140].imag = in[196].imag;
	in[196].real = tmp_real;
	in[196].imag = tmp_imag;

	tmp_real = in[141].real;
	tmp_imag = in[141].imag;
	in[141].real = in[708].real;
	in[141].imag = in[708].imag;
	in[708].real = tmp_real;
	in[708].imag = tmp_imag;

	tmp_real = in[142].real;
	tmp_imag = in[142].imag;
	in[142].real = in[452].real;
	in[142].imag = in[452].imag;
	in[452].real = tmp_real;
	in[452].imag = tmp_imag;

	tmp_real = in[143].real;
	tmp_imag = in[143].imag;
	in[143].real = in[964].real;
	in[143].imag = in[964].imag;
	in[964].real = tmp_real;
	in[964].imag = tmp_imag;

	tmp_real = in[145].real;
	tmp_imag = in[145].imag;
	in[145].real = in[548].real;
	in[145].imag = in[548].imag;
	in[548].real = tmp_real;
	in[548].imag = tmp_imag;

	tmp_real = in[146].real;
	tmp_imag = in[146].imag;
	in[146].real = in[292].real;
	in[146].imag = in[292].imag;
	in[292].real = tmp_real;
	in[292].imag = tmp_imag;

	tmp_real = in[147].real;
	tmp_imag = in[147].imag;
	in[147].real = in[804].real;
	in[147].imag = in[804].imag;
	in[804].real = tmp_real;
	in[804].imag = tmp_imag;

	tmp_real = in[148].real;
	tmp_imag = in[148].imag;
	in[148].real = in[164].real;
	in[148].imag = in[164].imag;
	in[164].real = tmp_real;
	in[164].imag = tmp_imag;

	tmp_real = in[149].real;
	tmp_imag = in[149].imag;
	in[149].real = in[676].real;
	in[149].imag = in[676].imag;
	in[676].real = tmp_real;
	in[676].imag = tmp_imag;

	tmp_real = in[150].real;
	tmp_imag = in[150].imag;
	in[150].real = in[420].real;
	in[150].imag = in[420].imag;
	in[420].real = tmp_real;
	in[420].imag = tmp_imag;

	tmp_real = in[151].real;
	tmp_imag = in[151].imag;
	in[151].real = in[932].real;
	in[151].imag = in[932].imag;
	in[932].real = tmp_real;
	in[932].imag = tmp_imag;

	tmp_real = in[153].real;
	tmp_imag = in[153].imag;
	in[153].real = in[612].real;
	in[153].imag = in[612].imag;
	in[612].real = tmp_real;
	in[612].imag = tmp_imag;

	tmp_real = in[154].real;
	tmp_imag = in[154].imag;
	in[154].real = in[356].real;
	in[154].imag = in[356].imag;
	in[356].real = tmp_real;
	in[356].imag = tmp_imag;

	tmp_real = in[155].real;
	tmp_imag = in[155].imag;
	in[155].real = in[868].real;
	in[155].imag = in[868].imag;
	in[868].real = tmp_real;
	in[868].imag = tmp_imag;

	tmp_real = in[156].real;
	tmp_imag = in[156].imag;
	in[156].real = in[228].real;
	in[156].imag = in[228].imag;
	in[228].real = tmp_real;
	in[228].imag = tmp_imag;

	tmp_real = in[157].real;
	tmp_imag = in[157].imag;
	in[157].real = in[740].real;
	in[157].imag = in[740].imag;
	in[740].real = tmp_real;
	in[740].imag = tmp_imag;

	tmp_real = in[158].real;
	tmp_imag = in[158].imag;
	in[158].real = in[484].real;
	in[158].imag = in[484].imag;
	in[484].real = tmp_real;
	in[484].imag = tmp_imag;

	tmp_real = in[159].real;
	tmp_imag = in[159].imag;
	in[159].real = in[996].real;
	in[159].imag = in[996].imag;
	in[996].real = tmp_real;
	in[996].imag = tmp_imag;

	tmp_real = in[161].real;
	tmp_imag = in[161].imag;
	in[161].real = in[532].real;
	in[161].imag = in[532].imag;
	in[532].real = tmp_real;
	in[532].imag = tmp_imag;

	tmp_real = in[162].real;
	tmp_imag = in[162].imag;
	in[162].real = in[276].real;
	in[162].imag = in[276].imag;
	in[276].real = tmp_real;
	in[276].imag = tmp_imag;

	tmp_real = in[163].real;
	tmp_imag = in[163].imag;
	in[163].real = in[788].real;
	in[163].imag = in[788].imag;
	in[788].real = tmp_real;
	in[788].imag = tmp_imag;

	tmp_real = in[165].real;
	tmp_imag = in[165].imag;
	in[165].real = in[660].real;
	in[165].imag = in[660].imag;
	in[660].real = tmp_real;
	in[660].imag = tmp_imag;

	tmp_real = in[166].real;
	tmp_imag = in[166].imag;
	in[166].real = in[404].real;
	in[166].imag = in[404].imag;
	in[404].real = tmp_real;
	in[404].imag = tmp_imag;

	tmp_real = in[167].real;
	tmp_imag = in[167].imag;
	in[167].real = in[916].real;
	in[167].imag = in[916].imag;
	in[916].real = tmp_real;
	in[916].imag = tmp_imag;

	tmp_real = in[169].real;
	tmp_imag = in[169].imag;
	in[169].real = in[596].real;
	in[169].imag = in[596].imag;
	in[596].real = tmp_real;
	in[596].imag = tmp_imag;

	tmp_real = in[170].real;
	tmp_imag = in[170].imag;
	in[170].real = in[340].real;
	in[170].imag = in[340].imag;
	in[340].real = tmp_real;
	in[340].imag = tmp_imag;

	tmp_real = in[171].real;
	tmp_imag = in[171].imag;
	in[171].real = in[852].real;
	in[171].imag = in[852].imag;
	in[852].real = tmp_real;
	in[852].imag = tmp_imag;

	tmp_real = in[172].real;
	tmp_imag = in[172].imag;
	in[172].real = in[212].real;
	in[172].imag = in[212].imag;
	in[212].real = tmp_real;
	in[212].imag = tmp_imag;

	tmp_real = in[173].real;
	tmp_imag = in[173].imag;
	in[173].real = in[724].real;
	in[173].imag = in[724].imag;
	in[724].real = tmp_real;
	in[724].imag = tmp_imag;

	tmp_real = in[174].real;
	tmp_imag = in[174].imag;
	in[174].real = in[468].real;
	in[174].imag = in[468].imag;
	in[468].real = tmp_real;
	in[468].imag = tmp_imag;

	tmp_real = in[175].real;
	tmp_imag = in[175].imag;
	in[175].real = in[980].real;
	in[175].imag = in[980].imag;
	in[980].real = tmp_real;
	in[980].imag = tmp_imag;

	tmp_real = in[177].real;
	tmp_imag = in[177].imag;
	in[177].real = in[564].real;
	in[177].imag = in[564].imag;
	in[564].real = tmp_real;
	in[564].imag = tmp_imag;

	tmp_real = in[178].real;
	tmp_imag = in[178].imag;
	in[178].real = in[308].real;
	in[178].imag = in[308].imag;
	in[308].real = tmp_real;
	in[308].imag = tmp_imag;

	tmp_real = in[179].real;
	tmp_imag = in[179].imag;
	in[179].real = in[820].real;
	in[179].imag = in[820].imag;
	in[820].real = tmp_real;
	in[820].imag = tmp_imag;

	tmp_real = in[181].real;
	tmp_imag = in[181].imag;
	in[181].real = in[692].real;
	in[181].imag = in[692].imag;
	in[692].real = tmp_real;
	in[692].imag = tmp_imag;

	tmp_real = in[182].real;
	tmp_imag = in[182].imag;
	in[182].real = in[436].real;
	in[182].imag = in[436].imag;
	in[436].real = tmp_real;
	in[436].imag = tmp_imag;

	tmp_real = in[183].real;
	tmp_imag = in[183].imag;
	in[183].real = in[948].real;
	in[183].imag = in[948].imag;
	in[948].real = tmp_real;
	in[948].imag = tmp_imag;

	tmp_real = in[185].real;
	tmp_imag = in[185].imag;
	in[185].real = in[628].real;
	in[185].imag = in[628].imag;
	in[628].real = tmp_real;
	in[628].imag = tmp_imag;

	tmp_real = in[186].real;
	tmp_imag = in[186].imag;
	in[186].real = in[372].real;
	in[186].imag = in[372].imag;
	in[372].real = tmp_real;
	in[372].imag = tmp_imag;

	tmp_real = in[187].real;
	tmp_imag = in[187].imag;
	in[187].real = in[884].real;
	in[187].imag = in[884].imag;
	in[884].real = tmp_real;
	in[884].imag = tmp_imag;

	tmp_real = in[188].real;
	tmp_imag = in[188].imag;
	in[188].real = in[244].real;
	in[188].imag = in[244].imag;
	in[244].real = tmp_real;
	in[244].imag = tmp_imag;

	tmp_real = in[189].real;
	tmp_imag = in[189].imag;
	in[189].real = in[756].real;
	in[189].imag = in[756].imag;
	in[756].real = tmp_real;
	in[756].imag = tmp_imag;

	tmp_real = in[190].real;
	tmp_imag = in[190].imag;
	in[190].real = in[500].real;
	in[190].imag = in[500].imag;
	in[500].real = tmp_real;
	in[500].imag = tmp_imag;

	tmp_real = in[191].real;
	tmp_imag = in[191].imag;
	in[191].real = in[1012].real;
	in[191].imag = in[1012].imag;
	in[1012].real = tmp_real;
	in[1012].imag = tmp_imag;

	tmp_real = in[193].real;
	tmp_imag = in[193].imag;
	in[193].real = in[524].real;
	in[193].imag = in[524].imag;
	in[524].real = tmp_real;
	in[524].imag = tmp_imag;

	tmp_real = in[194].real;
	tmp_imag = in[194].imag;
	in[194].real = in[268].real;
	in[194].imag = in[268].imag;
	in[268].real = tmp_real;
	in[268].imag = tmp_imag;

	tmp_real = in[195].real;
	tmp_imag = in[195].imag;
	in[195].real = in[780].real;
	in[195].imag = in[780].imag;
	in[780].real = tmp_real;
	in[780].imag = tmp_imag;

	tmp_real = in[197].real;
	tmp_imag = in[197].imag;
	in[197].real = in[652].real;
	in[197].imag = in[652].imag;
	in[652].real = tmp_real;
	in[652].imag = tmp_imag;

	tmp_real = in[198].real;
	tmp_imag = in[198].imag;
	in[198].real = in[396].real;
	in[198].imag = in[396].imag;
	in[396].real = tmp_real;
	in[396].imag = tmp_imag;

	tmp_real = in[199].real;
	tmp_imag = in[199].imag;
	in[199].real = in[908].real;
	in[199].imag = in[908].imag;
	in[908].real = tmp_real;
	in[908].imag = tmp_imag;

	tmp_real = in[201].real;
	tmp_imag = in[201].imag;
	in[201].real = in[588].real;
	in[201].imag = in[588].imag;
	in[588].real = tmp_real;
	in[588].imag = tmp_imag;

	tmp_real = in[202].real;
	tmp_imag = in[202].imag;
	in[202].real = in[332].real;
	in[202].imag = in[332].imag;
	in[332].real = tmp_real;
	in[332].imag = tmp_imag;

	tmp_real = in[203].real;
	tmp_imag = in[203].imag;
	in[203].real = in[844].real;
	in[203].imag = in[844].imag;
	in[844].real = tmp_real;
	in[844].imag = tmp_imag;

	tmp_real = in[205].real;
	tmp_imag = in[205].imag;
	in[205].real = in[716].real;
	in[205].imag = in[716].imag;
	in[716].real = tmp_real;
	in[716].imag = tmp_imag;

	tmp_real = in[206].real;
	tmp_imag = in[206].imag;
	in[206].real = in[460].real;
	in[206].imag = in[460].imag;
	in[460].real = tmp_real;
	in[460].imag = tmp_imag;

	tmp_real = in[207].real;
	tmp_imag = in[207].imag;
	in[207].real = in[972].real;
	in[207].imag = in[972].imag;
	in[972].real = tmp_real;
	in[972].imag = tmp_imag;

	tmp_real = in[209].real;
	tmp_imag = in[209].imag;
	in[209].real = in[556].real;
	in[209].imag = in[556].imag;
	in[556].real = tmp_real;
	in[556].imag = tmp_imag;

	tmp_real = in[210].real;
	tmp_imag = in[210].imag;
	in[210].real = in[300].real;
	in[210].imag = in[300].imag;
	in[300].real = tmp_real;
	in[300].imag = tmp_imag;

	tmp_real = in[211].real;
	tmp_imag = in[211].imag;
	in[211].real = in[812].real;
	in[211].imag = in[812].imag;
	in[812].real = tmp_real;
	in[812].imag = tmp_imag;

	tmp_real = in[213].real;
	tmp_imag = in[213].imag;
	in[213].real = in[684].real;
	in[213].imag = in[684].imag;
	in[684].real = tmp_real;
	in[684].imag = tmp_imag;

	tmp_real = in[214].real;
	tmp_imag = in[214].imag;
	in[214].real = in[428].real;
	in[214].imag = in[428].imag;
	in[428].real = tmp_real;
	in[428].imag = tmp_imag;

	tmp_real = in[215].real;
	tmp_imag = in[215].imag;
	in[215].real = in[940].real;
	in[215].imag = in[940].imag;
	in[940].real = tmp_real;
	in[940].imag = tmp_imag;

	tmp_real = in[217].real;
	tmp_imag = in[217].imag;
	in[217].real = in[620].real;
	in[217].imag = in[620].imag;
	in[620].real = tmp_real;
	in[620].imag = tmp_imag;

	tmp_real = in[218].real;
	tmp_imag = in[218].imag;
	in[218].real = in[364].real;
	in[218].imag = in[364].imag;
	in[364].real = tmp_real;
	in[364].imag = tmp_imag;

	tmp_real = in[219].real;
	tmp_imag = in[219].imag;
	in[219].real = in[876].real;
	in[219].imag = in[876].imag;
	in[876].real = tmp_real;
	in[876].imag = tmp_imag;

	tmp_real = in[220].real;
	tmp_imag = in[220].imag;
	in[220].real = in[236].real;
	in[220].imag = in[236].imag;
	in[236].real = tmp_real;
	in[236].imag = tmp_imag;

	tmp_real = in[221].real;
	tmp_imag = in[221].imag;
	in[221].real = in[748].real;
	in[221].imag = in[748].imag;
	in[748].real = tmp_real;
	in[748].imag = tmp_imag;

	tmp_real = in[222].real;
	tmp_imag = in[222].imag;
	in[222].real = in[492].real;
	in[222].imag = in[492].imag;
	in[492].real = tmp_real;
	in[492].imag = tmp_imag;

	tmp_real = in[223].real;
	tmp_imag = in[223].imag;
	in[223].real = in[1004].real;
	in[223].imag = in[1004].imag;
	in[1004].real = tmp_real;
	in[1004].imag = tmp_imag;

	tmp_real = in[225].real;
	tmp_imag = in[225].imag;
	in[225].real = in[540].real;
	in[225].imag = in[540].imag;
	in[540].real = tmp_real;
	in[540].imag = tmp_imag;

	tmp_real = in[226].real;
	tmp_imag = in[226].imag;
	in[226].real = in[284].real;
	in[226].imag = in[284].imag;
	in[284].real = tmp_real;
	in[284].imag = tmp_imag;

	tmp_real = in[227].real;
	tmp_imag = in[227].imag;
	in[227].real = in[796].real;
	in[227].imag = in[796].imag;
	in[796].real = tmp_real;
	in[796].imag = tmp_imag;

	tmp_real = in[229].real;
	tmp_imag = in[229].imag;
	in[229].real = in[668].real;
	in[229].imag = in[668].imag;
	in[668].real = tmp_real;
	in[668].imag = tmp_imag;

	tmp_real = in[230].real;
	tmp_imag = in[230].imag;
	in[230].real = in[412].real;
	in[230].imag = in[412].imag;
	in[412].real = tmp_real;
	in[412].imag = tmp_imag;

	tmp_real = in[231].real;
	tmp_imag = in[231].imag;
	in[231].real = in[924].real;
	in[231].imag = in[924].imag;
	in[924].real = tmp_real;
	in[924].imag = tmp_imag;

	tmp_real = in[233].real;
	tmp_imag = in[233].imag;
	in[233].real = in[604].real;
	in[233].imag = in[604].imag;
	in[604].real = tmp_real;
	in[604].imag = tmp_imag;

	tmp_real = in[234].real;
	tmp_imag = in[234].imag;
	in[234].real = in[348].real;
	in[234].imag = in[348].imag;
	in[348].real = tmp_real;
	in[348].imag = tmp_imag;

	tmp_real = in[235].real;
	tmp_imag = in[235].imag;
	in[235].real = in[860].real;
	in[235].imag = in[860].imag;
	in[860].real = tmp_real;
	in[860].imag = tmp_imag;

	tmp_real = in[237].real;
	tmp_imag = in[237].imag;
	in[237].real = in[732].real;
	in[237].imag = in[732].imag;
	in[732].real = tmp_real;
	in[732].imag = tmp_imag;

	tmp_real = in[238].real;
	tmp_imag = in[238].imag;
	in[238].real = in[476].real;
	in[238].imag = in[476].imag;
	in[476].real = tmp_real;
	in[476].imag = tmp_imag;

	tmp_real = in[239].real;
	tmp_imag = in[239].imag;
	in[239].real = in[988].real;
	in[239].imag = in[988].imag;
	in[988].real = tmp_real;
	in[988].imag = tmp_imag;

	tmp_real = in[241].real;
	tmp_imag = in[241].imag;
	in[241].real = in[572].real;
	in[241].imag = in[572].imag;
	in[572].real = tmp_real;
	in[572].imag = tmp_imag;

	tmp_real = in[242].real;
	tmp_imag = in[242].imag;
	in[242].real = in[316].real;
	in[242].imag = in[316].imag;
	in[316].real = tmp_real;
	in[316].imag = tmp_imag;

	tmp_real = in[243].real;
	tmp_imag = in[243].imag;
	in[243].real = in[828].real;
	in[243].imag = in[828].imag;
	in[828].real = tmp_real;
	in[828].imag = tmp_imag;

	tmp_real = in[245].real;
	tmp_imag = in[245].imag;
	in[245].real = in[700].real;
	in[245].imag = in[700].imag;
	in[700].real = tmp_real;
	in[700].imag = tmp_imag;

	tmp_real = in[246].real;
	tmp_imag = in[246].imag;
	in[246].real = in[444].real;
	in[246].imag = in[444].imag;
	in[444].real = tmp_real;
	in[444].imag = tmp_imag;

	tmp_real = in[247].real;
	tmp_imag = in[247].imag;
	in[247].real = in[956].real;
	in[247].imag = in[956].imag;
	in[956].real = tmp_real;
	in[956].imag = tmp_imag;

	tmp_real = in[249].real;
	tmp_imag = in[249].imag;
	in[249].real = in[636].real;
	in[249].imag = in[636].imag;
	in[636].real = tmp_real;
	in[636].imag = tmp_imag;

	tmp_real = in[250].real;
	tmp_imag = in[250].imag;
	in[250].real = in[380].real;
	in[250].imag = in[380].imag;
	in[380].real = tmp_real;
	in[380].imag = tmp_imag;

	tmp_real = in[251].real;
	tmp_imag = in[251].imag;
	in[251].real = in[892].real;
	in[251].imag = in[892].imag;
	in[892].real = tmp_real;
	in[892].imag = tmp_imag;

	tmp_real = in[253].real;
	tmp_imag = in[253].imag;
	in[253].real = in[764].real;
	in[253].imag = in[764].imag;
	in[764].real = tmp_real;
	in[764].imag = tmp_imag;

	tmp_real = in[254].real;
	tmp_imag = in[254].imag;
	in[254].real = in[508].real;
	in[254].imag = in[508].imag;
	in[508].real = tmp_real;
	in[508].imag = tmp_imag;

	tmp_real = in[255].real;
	tmp_imag = in[255].imag;
	in[255].real = in[1020].real;
	in[255].imag = in[1020].imag;
	in[1020].real = tmp_real;
	in[1020].imag = tmp_imag;

	tmp_real = in[257].real;
	tmp_imag = in[257].imag;
	in[257].real = in[514].real;
	in[257].imag = in[514].imag;
	in[514].real = tmp_real;
	in[514].imag = tmp_imag;

	tmp_real = in[259].real;
	tmp_imag = in[259].imag;
	in[259].real = in[770].real;
	in[259].imag = in[770].imag;
	in[770].real = tmp_real;
	in[770].imag = tmp_imag;

	tmp_real = in[261].real;
	tmp_imag = in[261].imag;
	in[261].real = in[642].real;
	in[261].imag = in[642].imag;
	in[642].real = tmp_real;
	in[642].imag = tmp_imag;

	tmp_real = in[262].real;
	tmp_imag = in[262].imag;
	in[262].real = in[386].real;
	in[262].imag = in[386].imag;
	in[386].real = tmp_real;
	in[386].imag = tmp_imag;

	tmp_real = in[263].real;
	tmp_imag = in[263].imag;
	in[263].real = in[898].real;
	in[263].imag = in[898].imag;
	in[898].real = tmp_real;
	in[898].imag = tmp_imag;

	tmp_real = in[265].real;
	tmp_imag = in[265].imag;
	in[265].real = in[578].real;
	in[265].imag = in[578].imag;
	in[578].real = tmp_real;
	in[578].imag = tmp_imag;

	tmp_real = in[266].real;
	tmp_imag = in[266].imag;
	in[266].real = in[322].real;
	in[266].imag = in[322].imag;
	in[322].real = tmp_real;
	in[322].imag = tmp_imag;

	tmp_real = in[267].real;
	tmp_imag = in[267].imag;
	in[267].real = in[834].real;
	in[267].imag = in[834].imag;
	in[834].real = tmp_real;
	in[834].imag = tmp_imag;

	tmp_real = in[269].real;
	tmp_imag = in[269].imag;
	in[269].real = in[706].real;
	in[269].imag = in[706].imag;
	in[706].real = tmp_real;
	in[706].imag = tmp_imag;

	tmp_real = in[270].real;
	tmp_imag = in[270].imag;
	in[270].real = in[450].real;
	in[270].imag = in[450].imag;
	in[450].real = tmp_real;
	in[450].imag = tmp_imag;

	tmp_real = in[271].real;
	tmp_imag = in[271].imag;
	in[271].real = in[962].real;
	in[271].imag = in[962].imag;
	in[962].real = tmp_real;
	in[962].imag = tmp_imag;

	tmp_real = in[273].real;
	tmp_imag = in[273].imag;
	in[273].real = in[546].real;
	in[273].imag = in[546].imag;
	in[546].real = tmp_real;
	in[546].imag = tmp_imag;

	tmp_real = in[274].real;
	tmp_imag = in[274].imag;
	in[274].real = in[290].real;
	in[274].imag = in[290].imag;
	in[290].real = tmp_real;
	in[290].imag = tmp_imag;

	tmp_real = in[275].real;
	tmp_imag = in[275].imag;
	in[275].real = in[802].real;
	in[275].imag = in[802].imag;
	in[802].real = tmp_real;
	in[802].imag = tmp_imag;

	tmp_real = in[277].real;
	tmp_imag = in[277].imag;
	in[277].real = in[674].real;
	in[277].imag = in[674].imag;
	in[674].real = tmp_real;
	in[674].imag = tmp_imag;

	tmp_real = in[278].real;
	tmp_imag = in[278].imag;
	in[278].real = in[418].real;
	in[278].imag = in[418].imag;
	in[418].real = tmp_real;
	in[418].imag = tmp_imag;

	tmp_real = in[279].real;
	tmp_imag = in[279].imag;
	in[279].real = in[930].real;
	in[279].imag = in[930].imag;
	in[930].real = tmp_real;
	in[930].imag = tmp_imag;

	tmp_real = in[281].real;
	tmp_imag = in[281].imag;
	in[281].real = in[610].real;
	in[281].imag = in[610].imag;
	in[610].real = tmp_real;
	in[610].imag = tmp_imag;

	tmp_real = in[282].real;
	tmp_imag = in[282].imag;
	in[282].real = in[354].real;
	in[282].imag = in[354].imag;
	in[354].real = tmp_real;
	in[354].imag = tmp_imag;

	tmp_real = in[283].real;
	tmp_imag = in[283].imag;
	in[283].real = in[866].real;
	in[283].imag = in[866].imag;
	in[866].real = tmp_real;
	in[866].imag = tmp_imag;

	tmp_real = in[285].real;
	tmp_imag = in[285].imag;
	in[285].real = in[738].real;
	in[285].imag = in[738].imag;
	in[738].real = tmp_real;
	in[738].imag = tmp_imag;

	tmp_real = in[286].real;
	tmp_imag = in[286].imag;
	in[286].real = in[482].real;
	in[286].imag = in[482].imag;
	in[482].real = tmp_real;
	in[482].imag = tmp_imag;

	tmp_real = in[287].real;
	tmp_imag = in[287].imag;
	in[287].real = in[994].real;
	in[287].imag = in[994].imag;
	in[994].real = tmp_real;
	in[994].imag = tmp_imag;

	tmp_real = in[289].real;
	tmp_imag = in[289].imag;
	in[289].real = in[530].real;
	in[289].imag = in[530].imag;
	in[530].real = tmp_real;
	in[530].imag = tmp_imag;

	tmp_real = in[291].real;
	tmp_imag = in[291].imag;
	in[291].real = in[786].real;
	in[291].imag = in[786].imag;
	in[786].real = tmp_real;
	in[786].imag = tmp_imag;

	tmp_real = in[293].real;
	tmp_imag = in[293].imag;
	in[293].real = in[658].real;
	in[293].imag = in[658].imag;
	in[658].real = tmp_real;
	in[658].imag = tmp_imag;

	tmp_real = in[294].real;
	tmp_imag = in[294].imag;
	in[294].real = in[402].real;
	in[294].imag = in[402].imag;
	in[402].real = tmp_real;
	in[402].imag = tmp_imag;

	tmp_real = in[295].real;
	tmp_imag = in[295].imag;
	in[295].real = in[914].real;
	in[295].imag = in[914].imag;
	in[914].real = tmp_real;
	in[914].imag = tmp_imag;

	tmp_real = in[297].real;
	tmp_imag = in[297].imag;
	in[297].real = in[594].real;
	in[297].imag = in[594].imag;
	in[594].real = tmp_real;
	in[594].imag = tmp_imag;

	tmp_real = in[298].real;
	tmp_imag = in[298].imag;
	in[298].real = in[338].real;
	in[298].imag = in[338].imag;
	in[338].real = tmp_real;
	in[338].imag = tmp_imag;

	tmp_real = in[299].real;
	tmp_imag = in[299].imag;
	in[299].real = in[850].real;
	in[299].imag = in[850].imag;
	in[850].real = tmp_real;
	in[850].imag = tmp_imag;

	tmp_real = in[301].real;
	tmp_imag = in[301].imag;
	in[301].real = in[722].real;
	in[301].imag = in[722].imag;
	in[722].real = tmp_real;
	in[722].imag = tmp_imag;

	tmp_real = in[302].real;
	tmp_imag = in[302].imag;
	in[302].real = in[466].real;
	in[302].imag = in[466].imag;
	in[466].real = tmp_real;
	in[466].imag = tmp_imag;

	tmp_real = in[303].real;
	tmp_imag = in[303].imag;
	in[303].real = in[978].real;
	in[303].imag = in[978].imag;
	in[978].real = tmp_real;
	in[978].imag = tmp_imag;

	tmp_real = in[305].real;
	tmp_imag = in[305].imag;
	in[305].real = in[562].real;
	in[305].imag = in[562].imag;
	in[562].real = tmp_real;
	in[562].imag = tmp_imag;

	tmp_real = in[307].real;
	tmp_imag = in[307].imag;
	in[307].real = in[818].real;
	in[307].imag = in[818].imag;
	in[818].real = tmp_real;
	in[818].imag = tmp_imag;

	tmp_real = in[309].real;
	tmp_imag = in[309].imag;
	in[309].real = in[690].real;
	in[309].imag = in[690].imag;
	in[690].real = tmp_real;
	in[690].imag = tmp_imag;

	tmp_real = in[310].real;
	tmp_imag = in[310].imag;
	in[310].real = in[434].real;
	in[310].imag = in[434].imag;
	in[434].real = tmp_real;
	in[434].imag = tmp_imag;

	tmp_real = in[311].real;
	tmp_imag = in[311].imag;
	in[311].real = in[946].real;
	in[311].imag = in[946].imag;
	in[946].real = tmp_real;
	in[946].imag = tmp_imag;

	tmp_real = in[313].real;
	tmp_imag = in[313].imag;
	in[313].real = in[626].real;
	in[313].imag = in[626].imag;
	in[626].real = tmp_real;
	in[626].imag = tmp_imag;

	tmp_real = in[314].real;
	tmp_imag = in[314].imag;
	in[314].real = in[370].real;
	in[314].imag = in[370].imag;
	in[370].real = tmp_real;
	in[370].imag = tmp_imag;

	tmp_real = in[315].real;
	tmp_imag = in[315].imag;
	in[315].real = in[882].real;
	in[315].imag = in[882].imag;
	in[882].real = tmp_real;
	in[882].imag = tmp_imag;

	tmp_real = in[317].real;
	tmp_imag = in[317].imag;
	in[317].real = in[754].real;
	in[317].imag = in[754].imag;
	in[754].real = tmp_real;
	in[754].imag = tmp_imag;

	tmp_real = in[318].real;
	tmp_imag = in[318].imag;
	in[318].real = in[498].real;
	in[318].imag = in[498].imag;
	in[498].real = tmp_real;
	in[498].imag = tmp_imag;

	tmp_real = in[319].real;
	tmp_imag = in[319].imag;
	in[319].real = in[1010].real;
	in[319].imag = in[1010].imag;
	in[1010].real = tmp_real;
	in[1010].imag = tmp_imag;

	tmp_real = in[321].real;
	tmp_imag = in[321].imag;
	in[321].real = in[522].real;
	in[321].imag = in[522].imag;
	in[522].real = tmp_real;
	in[522].imag = tmp_imag;

	tmp_real = in[323].real;
	tmp_imag = in[323].imag;
	in[323].real = in[778].real;
	in[323].imag = in[778].imag;
	in[778].real = tmp_real;
	in[778].imag = tmp_imag;

	tmp_real = in[325].real;
	tmp_imag = in[325].imag;
	in[325].real = in[650].real;
	in[325].imag = in[650].imag;
	in[650].real = tmp_real;
	in[650].imag = tmp_imag;

	tmp_real = in[326].real;
	tmp_imag = in[326].imag;
	in[326].real = in[394].real;
	in[326].imag = in[394].imag;
	in[394].real = tmp_real;
	in[394].imag = tmp_imag;

	tmp_real = in[327].real;
	tmp_imag = in[327].imag;
	in[327].real = in[906].real;
	in[327].imag = in[906].imag;
	in[906].real = tmp_real;
	in[906].imag = tmp_imag;

	tmp_real = in[329].real;
	tmp_imag = in[329].imag;
	in[329].real = in[586].real;
	in[329].imag = in[586].imag;
	in[586].real = tmp_real;
	in[586].imag = tmp_imag;

	tmp_real = in[331].real;
	tmp_imag = in[331].imag;
	in[331].real = in[842].real;
	in[331].imag = in[842].imag;
	in[842].real = tmp_real;
	in[842].imag = tmp_imag;

	tmp_real = in[333].real;
	tmp_imag = in[333].imag;
	in[333].real = in[714].real;
	in[333].imag = in[714].imag;
	in[714].real = tmp_real;
	in[714].imag = tmp_imag;

	tmp_real = in[334].real;
	tmp_imag = in[334].imag;
	in[334].real = in[458].real;
	in[334].imag = in[458].imag;
	in[458].real = tmp_real;
	in[458].imag = tmp_imag;

	tmp_real = in[335].real;
	tmp_imag = in[335].imag;
	in[335].real = in[970].real;
	in[335].imag = in[970].imag;
	in[970].real = tmp_real;
	in[970].imag = tmp_imag;

	tmp_real = in[337].real;
	tmp_imag = in[337].imag;
	in[337].real = in[554].real;
	in[337].imag = in[554].imag;
	in[554].real = tmp_real;
	in[554].imag = tmp_imag;

	tmp_real = in[339].real;
	tmp_imag = in[339].imag;
	in[339].real = in[810].real;
	in[339].imag = in[810].imag;
	in[810].real = tmp_real;
	in[810].imag = tmp_imag;

	tmp_real = in[341].real;
	tmp_imag = in[341].imag;
	in[341].real = in[682].real;
	in[341].imag = in[682].imag;
	in[682].real = tmp_real;
	in[682].imag = tmp_imag;

	tmp_real = in[342].real;
	tmp_imag = in[342].imag;
	in[342].real = in[426].real;
	in[342].imag = in[426].imag;
	in[426].real = tmp_real;
	in[426].imag = tmp_imag;

	tmp_real = in[343].real;
	tmp_imag = in[343].imag;
	in[343].real = in[938].real;
	in[343].imag = in[938].imag;
	in[938].real = tmp_real;
	in[938].imag = tmp_imag;

	tmp_real = in[345].real;
	tmp_imag = in[345].imag;
	in[345].real = in[618].real;
	in[345].imag = in[618].imag;
	in[618].real = tmp_real;
	in[618].imag = tmp_imag;

	tmp_real = in[346].real;
	tmp_imag = in[346].imag;
	in[346].real = in[362].real;
	in[346].imag = in[362].imag;
	in[362].real = tmp_real;
	in[362].imag = tmp_imag;

	tmp_real = in[347].real;
	tmp_imag = in[347].imag;
	in[347].real = in[874].real;
	in[347].imag = in[874].imag;
	in[874].real = tmp_real;
	in[874].imag = tmp_imag;

	tmp_real = in[349].real;
	tmp_imag = in[349].imag;
	in[349].real = in[746].real;
	in[349].imag = in[746].imag;
	in[746].real = tmp_real;
	in[746].imag = tmp_imag;

	tmp_real = in[350].real;
	tmp_imag = in[350].imag;
	in[350].real = in[490].real;
	in[350].imag = in[490].imag;
	in[490].real = tmp_real;
	in[490].imag = tmp_imag;

	tmp_real = in[351].real;
	tmp_imag = in[351].imag;
	in[351].real = in[1002].real;
	in[351].imag = in[1002].imag;
	in[1002].real = tmp_real;
	in[1002].imag = tmp_imag;

	tmp_real = in[353].real;
	tmp_imag = in[353].imag;
	in[353].real = in[538].real;
	in[353].imag = in[538].imag;
	in[538].real = tmp_real;
	in[538].imag = tmp_imag;

	tmp_real = in[355].real;
	tmp_imag = in[355].imag;
	in[355].real = in[794].real;
	in[355].imag = in[794].imag;
	in[794].real = tmp_real;
	in[794].imag = tmp_imag;

	tmp_real = in[357].real;
	tmp_imag = in[357].imag;
	in[357].real = in[666].real;
	in[357].imag = in[666].imag;
	in[666].real = tmp_real;
	in[666].imag = tmp_imag;

	tmp_real = in[358].real;
	tmp_imag = in[358].imag;
	in[358].real = in[410].real;
	in[358].imag = in[410].imag;
	in[410].real = tmp_real;
	in[410].imag = tmp_imag;

	tmp_real = in[359].real;
	tmp_imag = in[359].imag;
	in[359].real = in[922].real;
	in[359].imag = in[922].imag;
	in[922].real = tmp_real;
	in[922].imag = tmp_imag;

	tmp_real = in[361].real;
	tmp_imag = in[361].imag;
	in[361].real = in[602].real;
	in[361].imag = in[602].imag;
	in[602].real = tmp_real;
	in[602].imag = tmp_imag;

	tmp_real = in[363].real;
	tmp_imag = in[363].imag;
	in[363].real = in[858].real;
	in[363].imag = in[858].imag;
	in[858].real = tmp_real;
	in[858].imag = tmp_imag;

	tmp_real = in[365].real;
	tmp_imag = in[365].imag;
	in[365].real = in[730].real;
	in[365].imag = in[730].imag;
	in[730].real = tmp_real;
	in[730].imag = tmp_imag;

	tmp_real = in[366].real;
	tmp_imag = in[366].imag;
	in[366].real = in[474].real;
	in[366].imag = in[474].imag;
	in[474].real = tmp_real;
	in[474].imag = tmp_imag;

	tmp_real = in[367].real;
	tmp_imag = in[367].imag;
	in[367].real = in[986].real;
	in[367].imag = in[986].imag;
	in[986].real = tmp_real;
	in[986].imag = tmp_imag;

	tmp_real = in[369].real;
	tmp_imag = in[369].imag;
	in[369].real = in[570].real;
	in[369].imag = in[570].imag;
	in[570].real = tmp_real;
	in[570].imag = tmp_imag;

	tmp_real = in[371].real;
	tmp_imag = in[371].imag;
	in[371].real = in[826].real;
	in[371].imag = in[826].imag;
	in[826].real = tmp_real;
	in[826].imag = tmp_imag;

	tmp_real = in[373].real;
	tmp_imag = in[373].imag;
	in[373].real = in[698].real;
	in[373].imag = in[698].imag;
	in[698].real = tmp_real;
	in[698].imag = tmp_imag;

	tmp_real = in[374].real;
	tmp_imag = in[374].imag;
	in[374].real = in[442].real;
	in[374].imag = in[442].imag;
	in[442].real = tmp_real;
	in[442].imag = tmp_imag;

	tmp_real = in[375].real;
	tmp_imag = in[375].imag;
	in[375].real = in[954].real;
	in[375].imag = in[954].imag;
	in[954].real = tmp_real;
	in[954].imag = tmp_imag;

	tmp_real = in[377].real;
	tmp_imag = in[377].imag;
	in[377].real = in[634].real;
	in[377].imag = in[634].imag;
	in[634].real = tmp_real;
	in[634].imag = tmp_imag;

	tmp_real = in[379].real;
	tmp_imag = in[379].imag;
	in[379].real = in[890].real;
	in[379].imag = in[890].imag;
	in[890].real = tmp_real;
	in[890].imag = tmp_imag;

	tmp_real = in[381].real;
	tmp_imag = in[381].imag;
	in[381].real = in[762].real;
	in[381].imag = in[762].imag;
	in[762].real = tmp_real;
	in[762].imag = tmp_imag;

	tmp_real = in[382].real;
	tmp_imag = in[382].imag;
	in[382].real = in[506].real;
	in[382].imag = in[506].imag;
	in[506].real = tmp_real;
	in[506].imag = tmp_imag;

	tmp_real = in[383].real;
	tmp_imag = in[383].imag;
	in[383].real = in[1018].real;
	in[383].imag = in[1018].imag;
	in[1018].real = tmp_real;
	in[1018].imag = tmp_imag;

	tmp_real = in[385].real;
	tmp_imag = in[385].imag;
	in[385].real = in[518].real;
	in[385].imag = in[518].imag;
	in[518].real = tmp_real;
	in[518].imag = tmp_imag;

	tmp_real = in[387].real;
	tmp_imag = in[387].imag;
	in[387].real = in[774].real;
	in[387].imag = in[774].imag;
	in[774].real = tmp_real;
	in[774].imag = tmp_imag;

	tmp_real = in[389].real;
	tmp_imag = in[389].imag;
	in[389].real = in[646].real;
	in[389].imag = in[646].imag;
	in[646].real = tmp_real;
	in[646].imag = tmp_imag;

	tmp_real = in[391].real;
	tmp_imag = in[391].imag;
	in[391].real = in[902].real;
	in[391].imag = in[902].imag;
	in[902].real = tmp_real;
	in[902].imag = tmp_imag;

	tmp_real = in[393].real;
	tmp_imag = in[393].imag;
	in[393].real = in[582].real;
	in[393].imag = in[582].imag;
	in[582].real = tmp_real;
	in[582].imag = tmp_imag;

	tmp_real = in[395].real;
	tmp_imag = in[395].imag;
	in[395].real = in[838].real;
	in[395].imag = in[838].imag;
	in[838].real = tmp_real;
	in[838].imag = tmp_imag;

	tmp_real = in[397].real;
	tmp_imag = in[397].imag;
	in[397].real = in[710].real;
	in[397].imag = in[710].imag;
	in[710].real = tmp_real;
	in[710].imag = tmp_imag;

	tmp_real = in[398].real;
	tmp_imag = in[398].imag;
	in[398].real = in[454].real;
	in[398].imag = in[454].imag;
	in[454].real = tmp_real;
	in[454].imag = tmp_imag;

	tmp_real = in[399].real;
	tmp_imag = in[399].imag;
	in[399].real = in[966].real;
	in[399].imag = in[966].imag;
	in[966].real = tmp_real;
	in[966].imag = tmp_imag;

	tmp_real = in[401].real;
	tmp_imag = in[401].imag;
	in[401].real = in[550].real;
	in[401].imag = in[550].imag;
	in[550].real = tmp_real;
	in[550].imag = tmp_imag;

	tmp_real = in[403].real;
	tmp_imag = in[403].imag;
	in[403].real = in[806].real;
	in[403].imag = in[806].imag;
	in[806].real = tmp_real;
	in[806].imag = tmp_imag;

	tmp_real = in[405].real;
	tmp_imag = in[405].imag;
	in[405].real = in[678].real;
	in[405].imag = in[678].imag;
	in[678].real = tmp_real;
	in[678].imag = tmp_imag;

	tmp_real = in[406].real;
	tmp_imag = in[406].imag;
	in[406].real = in[422].real;
	in[406].imag = in[422].imag;
	in[422].real = tmp_real;
	in[422].imag = tmp_imag;

	tmp_real = in[407].real;
	tmp_imag = in[407].imag;
	in[407].real = in[934].real;
	in[407].imag = in[934].imag;
	in[934].real = tmp_real;
	in[934].imag = tmp_imag;

	tmp_real = in[409].real;
	tmp_imag = in[409].imag;
	in[409].real = in[614].real;
	in[409].imag = in[614].imag;
	in[614].real = tmp_real;
	in[614].imag = tmp_imag;

	tmp_real = in[411].real;
	tmp_imag = in[411].imag;
	in[411].real = in[870].real;
	in[411].imag = in[870].imag;
	in[870].real = tmp_real;
	in[870].imag = tmp_imag;

	tmp_real = in[413].real;
	tmp_imag = in[413].imag;
	in[413].real = in[742].real;
	in[413].imag = in[742].imag;
	in[742].real = tmp_real;
	in[742].imag = tmp_imag;

	tmp_real = in[414].real;
	tmp_imag = in[414].imag;
	in[414].real = in[486].real;
	in[414].imag = in[486].imag;
	in[486].real = tmp_real;
	in[486].imag = tmp_imag;

	tmp_real = in[415].real;
	tmp_imag = in[415].imag;
	in[415].real = in[998].real;
	in[415].imag = in[998].imag;
	in[998].real = tmp_real;
	in[998].imag = tmp_imag;

	tmp_real = in[417].real;
	tmp_imag = in[417].imag;
	in[417].real = in[534].real;
	in[417].imag = in[534].imag;
	in[534].real = tmp_real;
	in[534].imag = tmp_imag;

	tmp_real = in[419].real;
	tmp_imag = in[419].imag;
	in[419].real = in[790].real;
	in[419].imag = in[790].imag;
	in[790].real = tmp_real;
	in[790].imag = tmp_imag;

	tmp_real = in[421].real;
	tmp_imag = in[421].imag;
	in[421].real = in[662].real;
	in[421].imag = in[662].imag;
	in[662].real = tmp_real;
	in[662].imag = tmp_imag;

	tmp_real = in[423].real;
	tmp_imag = in[423].imag;
	in[423].real = in[918].real;
	in[423].imag = in[918].imag;
	in[918].real = tmp_real;
	in[918].imag = tmp_imag;

	tmp_real = in[425].real;
	tmp_imag = in[425].imag;
	in[425].real = in[598].real;
	in[425].imag = in[598].imag;
	in[598].real = tmp_real;
	in[598].imag = tmp_imag;

	tmp_real = in[427].real;
	tmp_imag = in[427].imag;
	in[427].real = in[854].real;
	in[427].imag = in[854].imag;
	in[854].real = tmp_real;
	in[854].imag = tmp_imag;

	tmp_real = in[429].real;
	tmp_imag = in[429].imag;
	in[429].real = in[726].real;
	in[429].imag = in[726].imag;
	in[726].real = tmp_real;
	in[726].imag = tmp_imag;

	tmp_real = in[430].real;
	tmp_imag = in[430].imag;
	in[430].real = in[470].real;
	in[430].imag = in[470].imag;
	in[470].real = tmp_real;
	in[470].imag = tmp_imag;

	tmp_real = in[431].real;
	tmp_imag = in[431].imag;
	in[431].real = in[982].real;
	in[431].imag = in[982].imag;
	in[982].real = tmp_real;
	in[982].imag = tmp_imag;

	tmp_real = in[433].real;
	tmp_imag = in[433].imag;
	in[433].real = in[566].real;
	in[433].imag = in[566].imag;
	in[566].real = tmp_real;
	in[566].imag = tmp_imag;

	tmp_real = in[435].real;
	tmp_imag = in[435].imag;
	in[435].real = in[822].real;
	in[435].imag = in[822].imag;
	in[822].real = tmp_real;
	in[822].imag = tmp_imag;

	tmp_real = in[437].real;
	tmp_imag = in[437].imag;
	in[437].real = in[694].real;
	in[437].imag = in[694].imag;
	in[694].real = tmp_real;
	in[694].imag = tmp_imag;

	tmp_real = in[439].real;
	tmp_imag = in[439].imag;
	in[439].real = in[950].real;
	in[439].imag = in[950].imag;
	in[950].real = tmp_real;
	in[950].imag = tmp_imag;

	tmp_real = in[441].real;
	tmp_imag = in[441].imag;
	in[441].real = in[630].real;
	in[441].imag = in[630].imag;
	in[630].real = tmp_real;
	in[630].imag = tmp_imag;

	tmp_real = in[443].real;
	tmp_imag = in[443].imag;
	in[443].real = in[886].real;
	in[443].imag = in[886].imag;
	in[886].real = tmp_real;
	in[886].imag = tmp_imag;

	tmp_real = in[445].real;
	tmp_imag = in[445].imag;
	in[445].real = in[758].real;
	in[445].imag = in[758].imag;
	in[758].real = tmp_real;
	in[758].imag = tmp_imag;

	tmp_real = in[446].real;
	tmp_imag = in[446].imag;
	in[446].real = in[502].real;
	in[446].imag = in[502].imag;
	in[502].real = tmp_real;
	in[502].imag = tmp_imag;

	tmp_real = in[447].real;
	tmp_imag = in[447].imag;
	in[447].real = in[1014].real;
	in[447].imag = in[1014].imag;
	in[1014].real = tmp_real;
	in[1014].imag = tmp_imag;

	tmp_real = in[449].real;
	tmp_imag = in[449].imag;
	in[449].real = in[526].real;
	in[449].imag = in[526].imag;
	in[526].real = tmp_real;
	in[526].imag = tmp_imag;

	tmp_real = in[451].real;
	tmp_imag = in[451].imag;
	in[451].real = in[782].real;
	in[451].imag = in[782].imag;
	in[782].real = tmp_real;
	in[782].imag = tmp_imag;

	tmp_real = in[453].real;
	tmp_imag = in[453].imag;
	in[453].real = in[654].real;
	in[453].imag = in[654].imag;
	in[654].real = tmp_real;
	in[654].imag = tmp_imag;

	tmp_real = in[455].real;
	tmp_imag = in[455].imag;
	in[455].real = in[910].real;
	in[455].imag = in[910].imag;
	in[910].real = tmp_real;
	in[910].imag = tmp_imag;

	tmp_real = in[457].real;
	tmp_imag = in[457].imag;
	in[457].real = in[590].real;
	in[457].imag = in[590].imag;
	in[590].real = tmp_real;
	in[590].imag = tmp_imag;

	tmp_real = in[459].real;
	tmp_imag = in[459].imag;
	in[459].real = in[846].real;
	in[459].imag = in[846].imag;
	in[846].real = tmp_real;
	in[846].imag = tmp_imag;

	tmp_real = in[461].real;
	tmp_imag = in[461].imag;
	in[461].real = in[718].real;
	in[461].imag = in[718].imag;
	in[718].real = tmp_real;
	in[718].imag = tmp_imag;

	tmp_real = in[463].real;
	tmp_imag = in[463].imag;
	in[463].real = in[974].real;
	in[463].imag = in[974].imag;
	in[974].real = tmp_real;
	in[974].imag = tmp_imag;

	tmp_real = in[465].real;
	tmp_imag = in[465].imag;
	in[465].real = in[558].real;
	in[465].imag = in[558].imag;
	in[558].real = tmp_real;
	in[558].imag = tmp_imag;

	tmp_real = in[467].real;
	tmp_imag = in[467].imag;
	in[467].real = in[814].real;
	in[467].imag = in[814].imag;
	in[814].real = tmp_real;
	in[814].imag = tmp_imag;

	tmp_real = in[469].real;
	tmp_imag = in[469].imag;
	in[469].real = in[686].real;
	in[469].imag = in[686].imag;
	in[686].real = tmp_real;
	in[686].imag = tmp_imag;

	tmp_real = in[471].real;
	tmp_imag = in[471].imag;
	in[471].real = in[942].real;
	in[471].imag = in[942].imag;
	in[942].real = tmp_real;
	in[942].imag = tmp_imag;

	tmp_real = in[473].real;
	tmp_imag = in[473].imag;
	in[473].real = in[622].real;
	in[473].imag = in[622].imag;
	in[622].real = tmp_real;
	in[622].imag = tmp_imag;

	tmp_real = in[475].real;
	tmp_imag = in[475].imag;
	in[475].real = in[878].real;
	in[475].imag = in[878].imag;
	in[878].real = tmp_real;
	in[878].imag = tmp_imag;

	tmp_real = in[477].real;
	tmp_imag = in[477].imag;
	in[477].real = in[750].real;
	in[477].imag = in[750].imag;
	in[750].real = tmp_real;
	in[750].imag = tmp_imag;

	tmp_real = in[478].real;
	tmp_imag = in[478].imag;
	in[478].real = in[494].real;
	in[478].imag = in[494].imag;
	in[494].real = tmp_real;
	in[494].imag = tmp_imag;

	tmp_real = in[479].real;
	tmp_imag = in[479].imag;
	in[479].real = in[1006].real;
	in[479].imag = in[1006].imag;
	in[1006].real = tmp_real;
	in[1006].imag = tmp_imag;

	tmp_real = in[481].real;
	tmp_imag = in[481].imag;
	in[481].real = in[542].real;
	in[481].imag = in[542].imag;
	in[542].real = tmp_real;
	in[542].imag = tmp_imag;

	tmp_real = in[483].real;
	tmp_imag = in[483].imag;
	in[483].real = in[798].real;
	in[483].imag = in[798].imag;
	in[798].real = tmp_real;
	in[798].imag = tmp_imag;

	tmp_real = in[485].real;
	tmp_imag = in[485].imag;
	in[485].real = in[670].real;
	in[485].imag = in[670].imag;
	in[670].real = tmp_real;
	in[670].imag = tmp_imag;

	tmp_real = in[487].real;
	tmp_imag = in[487].imag;
	in[487].real = in[926].real;
	in[487].imag = in[926].imag;
	in[926].real = tmp_real;
	in[926].imag = tmp_imag;

	tmp_real = in[489].real;
	tmp_imag = in[489].imag;
	in[489].real = in[606].real;
	in[489].imag = in[606].imag;
	in[606].real = tmp_real;
	in[606].imag = tmp_imag;

	tmp_real = in[491].real;
	tmp_imag = in[491].imag;
	in[491].real = in[862].real;
	in[491].imag = in[862].imag;
	in[862].real = tmp_real;
	in[862].imag = tmp_imag;

	tmp_real = in[493].real;
	tmp_imag = in[493].imag;
	in[493].real = in[734].real;
	in[493].imag = in[734].imag;
	in[734].real = tmp_real;
	in[734].imag = tmp_imag;

	tmp_real = in[495].real;
	tmp_imag = in[495].imag;
	in[495].real = in[990].real;
	in[495].imag = in[990].imag;
	in[990].real = tmp_real;
	in[990].imag = tmp_imag;

	tmp_real = in[497].real;
	tmp_imag = in[497].imag;
	in[497].real = in[574].real;
	in[497].imag = in[574].imag;
	in[574].real = tmp_real;
	in[574].imag = tmp_imag;

	tmp_real = in[499].real;
	tmp_imag = in[499].imag;
	in[499].real = in[830].real;
	in[499].imag = in[830].imag;
	in[830].real = tmp_real;
	in[830].imag = tmp_imag;

	tmp_real = in[501].real;
	tmp_imag = in[501].imag;
	in[501].real = in[702].real;
	in[501].imag = in[702].imag;
	in[702].real = tmp_real;
	in[702].imag = tmp_imag;

	tmp_real = in[503].real;
	tmp_imag = in[503].imag;
	in[503].real = in[958].real;
	in[503].imag = in[958].imag;
	in[958].real = tmp_real;
	in[958].imag = tmp_imag;

	tmp_real = in[505].real;
	tmp_imag = in[505].imag;
	in[505].real = in[638].real;
	in[505].imag = in[638].imag;
	in[638].real = tmp_real;
	in[638].imag = tmp_imag;

	tmp_real = in[507].real;
	tmp_imag = in[507].imag;
	in[507].real = in[894].real;
	in[507].imag = in[894].imag;
	in[894].real = tmp_real;
	in[894].imag = tmp_imag;

	tmp_real = in[509].real;
	tmp_imag = in[509].imag;
	in[509].real = in[766].real;
	in[509].imag = in[766].imag;
	in[766].real = tmp_real;
	in[766].imag = tmp_imag;

	tmp_real = in[511].real;
	tmp_imag = in[511].imag;
	in[511].real = in[1022].real;
	in[511].imag = in[1022].imag;
	in[1022].real = tmp_real;
	in[1022].imag = tmp_imag;

	tmp_real = in[515].real;
	tmp_imag = in[515].imag;
	in[515].real = in[769].real;
	in[515].imag = in[769].imag;
	in[769].real = tmp_real;
	in[769].imag = tmp_imag;

	tmp_real = in[517].real;
	tmp_imag = in[517].imag;
	in[517].real = in[641].real;
	in[517].imag = in[641].imag;
	in[641].real = tmp_real;
	in[641].imag = tmp_imag;

	tmp_real = in[519].real;
	tmp_imag = in[519].imag;
	in[519].real = in[897].real;
	in[519].imag = in[897].imag;
	in[897].real = tmp_real;
	in[897].imag = tmp_imag;

	tmp_real = in[521].real;
	tmp_imag = in[521].imag;
	in[521].real = in[577].real;
	in[521].imag = in[577].imag;
	in[577].real = tmp_real;
	in[577].imag = tmp_imag;

	tmp_real = in[523].real;
	tmp_imag = in[523].imag;
	in[523].real = in[833].real;
	in[523].imag = in[833].imag;
	in[833].real = tmp_real;
	in[833].imag = tmp_imag;

	tmp_real = in[525].real;
	tmp_imag = in[525].imag;
	in[525].real = in[705].real;
	in[525].imag = in[705].imag;
	in[705].real = tmp_real;
	in[705].imag = tmp_imag;

	tmp_real = in[527].real;
	tmp_imag = in[527].imag;
	in[527].real = in[961].real;
	in[527].imag = in[961].imag;
	in[961].real = tmp_real;
	in[961].imag = tmp_imag;

	tmp_real = in[529].real;
	tmp_imag = in[529].imag;
	in[529].real = in[545].real;
	in[529].imag = in[545].imag;
	in[545].real = tmp_real;
	in[545].imag = tmp_imag;

	tmp_real = in[531].real;
	tmp_imag = in[531].imag;
	in[531].real = in[801].real;
	in[531].imag = in[801].imag;
	in[801].real = tmp_real;
	in[801].imag = tmp_imag;

	tmp_real = in[533].real;
	tmp_imag = in[533].imag;
	in[533].real = in[673].real;
	in[533].imag = in[673].imag;
	in[673].real = tmp_real;
	in[673].imag = tmp_imag;

	tmp_real = in[535].real;
	tmp_imag = in[535].imag;
	in[535].real = in[929].real;
	in[535].imag = in[929].imag;
	in[929].real = tmp_real;
	in[929].imag = tmp_imag;

	tmp_real = in[537].real;
	tmp_imag = in[537].imag;
	in[537].real = in[609].real;
	in[537].imag = in[609].imag;
	in[609].real = tmp_real;
	in[609].imag = tmp_imag;

	tmp_real = in[539].real;
	tmp_imag = in[539].imag;
	in[539].real = in[865].real;
	in[539].imag = in[865].imag;
	in[865].real = tmp_real;
	in[865].imag = tmp_imag;

	tmp_real = in[541].real;
	tmp_imag = in[541].imag;
	in[541].real = in[737].real;
	in[541].imag = in[737].imag;
	in[737].real = tmp_real;
	in[737].imag = tmp_imag;

	tmp_real = in[543].real;
	tmp_imag = in[543].imag;
	in[543].real = in[993].real;
	in[543].imag = in[993].imag;
	in[993].real = tmp_real;
	in[993].imag = tmp_imag;

	tmp_real = in[547].real;
	tmp_imag = in[547].imag;
	in[547].real = in[785].real;
	in[547].imag = in[785].imag;
	in[785].real = tmp_real;
	in[785].imag = tmp_imag;

	tmp_real = in[549].real;
	tmp_imag = in[549].imag;
	in[549].real = in[657].real;
	in[549].imag = in[657].imag;
	in[657].real = tmp_real;
	in[657].imag = tmp_imag;

	tmp_real = in[551].real;
	tmp_imag = in[551].imag;
	in[551].real = in[913].real;
	in[551].imag = in[913].imag;
	in[913].real = tmp_real;
	in[913].imag = tmp_imag;

	tmp_real = in[553].real;
	tmp_imag = in[553].imag;
	in[553].real = in[593].real;
	in[553].imag = in[593].imag;
	in[593].real = tmp_real;
	in[593].imag = tmp_imag;

	tmp_real = in[555].real;
	tmp_imag = in[555].imag;
	in[555].real = in[849].real;
	in[555].imag = in[849].imag;
	in[849].real = tmp_real;
	in[849].imag = tmp_imag;

	tmp_real = in[557].real;
	tmp_imag = in[557].imag;
	in[557].real = in[721].real;
	in[557].imag = in[721].imag;
	in[721].real = tmp_real;
	in[721].imag = tmp_imag;

	tmp_real = in[559].real;
	tmp_imag = in[559].imag;
	in[559].real = in[977].real;
	in[559].imag = in[977].imag;
	in[977].real = tmp_real;
	in[977].imag = tmp_imag;

	tmp_real = in[563].real;
	tmp_imag = in[563].imag;
	in[563].real = in[817].real;
	in[563].imag = in[817].imag;
	in[817].real = tmp_real;
	in[817].imag = tmp_imag;

	tmp_real = in[565].real;
	tmp_imag = in[565].imag;
	in[565].real = in[689].real;
	in[565].imag = in[689].imag;
	in[689].real = tmp_real;
	in[689].imag = tmp_imag;

	tmp_real = in[567].real;
	tmp_imag = in[567].imag;
	in[567].real = in[945].real;
	in[567].imag = in[945].imag;
	in[945].real = tmp_real;
	in[945].imag = tmp_imag;

	tmp_real = in[569].real;
	tmp_imag = in[569].imag;
	in[569].real = in[625].real;
	in[569].imag = in[625].imag;
	in[625].real = tmp_real;
	in[625].imag = tmp_imag;

	tmp_real = in[571].real;
	tmp_imag = in[571].imag;
	in[571].real = in[881].real;
	in[571].imag = in[881].imag;
	in[881].real = tmp_real;
	in[881].imag = tmp_imag;

	tmp_real = in[573].real;
	tmp_imag = in[573].imag;
	in[573].real = in[753].real;
	in[573].imag = in[753].imag;
	in[753].real = tmp_real;
	in[753].imag = tmp_imag;

	tmp_real = in[575].real;
	tmp_imag = in[575].imag;
	in[575].real = in[1009].real;
	in[575].imag = in[1009].imag;
	in[1009].real = tmp_real;
	in[1009].imag = tmp_imag;

	tmp_real = in[579].real;
	tmp_imag = in[579].imag;
	in[579].real = in[777].real;
	in[579].imag = in[777].imag;
	in[777].real = tmp_real;
	in[777].imag = tmp_imag;

	tmp_real = in[581].real;
	tmp_imag = in[581].imag;
	in[581].real = in[649].real;
	in[581].imag = in[649].imag;
	in[649].real = tmp_real;
	in[649].imag = tmp_imag;

	tmp_real = in[583].real;
	tmp_imag = in[583].imag;
	in[583].real = in[905].real;
	in[583].imag = in[905].imag;
	in[905].real = tmp_real;
	in[905].imag = tmp_imag;

	tmp_real = in[587].real;
	tmp_imag = in[587].imag;
	in[587].real = in[841].real;
	in[587].imag = in[841].imag;
	in[841].real = tmp_real;
	in[841].imag = tmp_imag;

	tmp_real = in[589].real;
	tmp_imag = in[589].imag;
	in[589].real = in[713].real;
	in[589].imag = in[713].imag;
	in[713].real = tmp_real;
	in[713].imag = tmp_imag;

	tmp_real = in[591].real;
	tmp_imag = in[591].imag;
	in[591].real = in[969].real;
	in[591].imag = in[969].imag;
	in[969].real = tmp_real;
	in[969].imag = tmp_imag;

	tmp_real = in[595].real;
	tmp_imag = in[595].imag;
	in[595].real = in[809].real;
	in[595].imag = in[809].imag;
	in[809].real = tmp_real;
	in[809].imag = tmp_imag;

	tmp_real = in[597].real;
	tmp_imag = in[597].imag;
	in[597].real = in[681].real;
	in[597].imag = in[681].imag;
	in[681].real = tmp_real;
	in[681].imag = tmp_imag;

	tmp_real = in[599].real;
	tmp_imag = in[599].imag;
	in[599].real = in[937].real;
	in[599].imag = in[937].imag;
	in[937].real = tmp_real;
	in[937].imag = tmp_imag;

	tmp_real = in[601].real;
	tmp_imag = in[601].imag;
	in[601].real = in[617].real;
	in[601].imag = in[617].imag;
	in[617].real = tmp_real;
	in[617].imag = tmp_imag;

	tmp_real = in[603].real;
	tmp_imag = in[603].imag;
	in[603].real = in[873].real;
	in[603].imag = in[873].imag;
	in[873].real = tmp_real;
	in[873].imag = tmp_imag;

	tmp_real = in[605].real;
	tmp_imag = in[605].imag;
	in[605].real = in[745].real;
	in[605].imag = in[745].imag;
	in[745].real = tmp_real;
	in[745].imag = tmp_imag;

	tmp_real = in[607].real;
	tmp_imag = in[607].imag;
	in[607].real = in[1001].real;
	in[607].imag = in[1001].imag;
	in[1001].real = tmp_real;
	in[1001].imag = tmp_imag;

	tmp_real = in[611].real;
	tmp_imag = in[611].imag;
	in[611].real = in[793].real;
	in[611].imag = in[793].imag;
	in[793].real = tmp_real;
	in[793].imag = tmp_imag;

	tmp_real = in[613].real;
	tmp_imag = in[613].imag;
	in[613].real = in[665].real;
	in[613].imag = in[665].imag;
	in[665].real = tmp_real;
	in[665].imag = tmp_imag;

	tmp_real = in[615].real;
	tmp_imag = in[615].imag;
	in[615].real = in[921].real;
	in[615].imag = in[921].imag;
	in[921].real = tmp_real;
	in[921].imag = tmp_imag;

	tmp_real = in[619].real;
	tmp_imag = in[619].imag;
	in[619].real = in[857].real;
	in[619].imag = in[857].imag;
	in[857].real = tmp_real;
	in[857].imag = tmp_imag;

	tmp_real = in[621].real;
	tmp_imag = in[621].imag;
	in[621].real = in[729].real;
	in[621].imag = in[729].imag;
	in[729].real = tmp_real;
	in[729].imag = tmp_imag;

	tmp_real = in[623].real;
	tmp_imag = in[623].imag;
	in[623].real = in[985].real;
	in[623].imag = in[985].imag;
	in[985].real = tmp_real;
	in[985].imag = tmp_imag;

	tmp_real = in[627].real;
	tmp_imag = in[627].imag;
	in[627].real = in[825].real;
	in[627].imag = in[825].imag;
	in[825].real = tmp_real;
	in[825].imag = tmp_imag;

	tmp_real = in[629].real;
	tmp_imag = in[629].imag;
	in[629].real = in[697].real;
	in[629].imag = in[697].imag;
	in[697].real = tmp_real;
	in[697].imag = tmp_imag;

	tmp_real = in[631].real;
	tmp_imag = in[631].imag;
	in[631].real = in[953].real;
	in[631].imag = in[953].imag;
	in[953].real = tmp_real;
	in[953].imag = tmp_imag;

	tmp_real = in[635].real;
	tmp_imag = in[635].imag;
	in[635].real = in[889].real;
	in[635].imag = in[889].imag;
	in[889].real = tmp_real;
	in[889].imag = tmp_imag;

	tmp_real = in[637].real;
	tmp_imag = in[637].imag;
	in[637].real = in[761].real;
	in[637].imag = in[761].imag;
	in[761].real = tmp_real;
	in[761].imag = tmp_imag;

	tmp_real = in[639].real;
	tmp_imag = in[639].imag;
	in[639].real = in[1017].real;
	in[639].imag = in[1017].imag;
	in[1017].real = tmp_real;
	in[1017].imag = tmp_imag;

	tmp_real = in[643].real;
	tmp_imag = in[643].imag;
	in[643].real = in[773].real;
	in[643].imag = in[773].imag;
	in[773].real = tmp_real;
	in[773].imag = tmp_imag;

	tmp_real = in[647].real;
	tmp_imag = in[647].imag;
	in[647].real = in[901].real;
	in[647].imag = in[901].imag;
	in[901].real = tmp_real;
	in[901].imag = tmp_imag;

	tmp_real = in[651].real;
	tmp_imag = in[651].imag;
	in[651].real = in[837].real;
	in[651].imag = in[837].imag;
	in[837].real = tmp_real;
	in[837].imag = tmp_imag;

	tmp_real = in[653].real;
	tmp_imag = in[653].imag;
	in[653].real = in[709].real;
	in[653].imag = in[709].imag;
	in[709].real = tmp_real;
	in[709].imag = tmp_imag;

	tmp_real = in[655].real;
	tmp_imag = in[655].imag;
	in[655].real = in[965].real;
	in[655].imag = in[965].imag;
	in[965].real = tmp_real;
	in[965].imag = tmp_imag;

	tmp_real = in[659].real;
	tmp_imag = in[659].imag;
	in[659].real = in[805].real;
	in[659].imag = in[805].imag;
	in[805].real = tmp_real;
	in[805].imag = tmp_imag;

	tmp_real = in[661].real;
	tmp_imag = in[661].imag;
	in[661].real = in[677].real;
	in[661].imag = in[677].imag;
	in[677].real = tmp_real;
	in[677].imag = tmp_imag;

	tmp_real = in[663].real;
	tmp_imag = in[663].imag;
	in[663].real = in[933].real;
	in[663].imag = in[933].imag;
	in[933].real = tmp_real;
	in[933].imag = tmp_imag;

	tmp_real = in[667].real;
	tmp_imag = in[667].imag;
	in[667].real = in[869].real;
	in[667].imag = in[869].imag;
	in[869].real = tmp_real;
	in[869].imag = tmp_imag;

	tmp_real = in[669].real;
	tmp_imag = in[669].imag;
	in[669].real = in[741].real;
	in[669].imag = in[741].imag;
	in[741].real = tmp_real;
	in[741].imag = tmp_imag;

	tmp_real = in[671].real;
	tmp_imag = in[671].imag;
	in[671].real = in[997].real;
	in[671].imag = in[997].imag;
	in[997].real = tmp_real;
	in[997].imag = tmp_imag;

	tmp_real = in[675].real;
	tmp_imag = in[675].imag;
	in[675].real = in[789].real;
	in[675].imag = in[789].imag;
	in[789].real = tmp_real;
	in[789].imag = tmp_imag;

	tmp_real = in[679].real;
	tmp_imag = in[679].imag;
	in[679].real = in[917].real;
	in[679].imag = in[917].imag;
	in[917].real = tmp_real;
	in[917].imag = tmp_imag;

	tmp_real = in[683].real;
	tmp_imag = in[683].imag;
	in[683].real = in[853].real;
	in[683].imag = in[853].imag;
	in[853].real = tmp_real;
	in[853].imag = tmp_imag;

	tmp_real = in[685].real;
	tmp_imag = in[685].imag;
	in[685].real = in[725].real;
	in[685].imag = in[725].imag;
	in[725].real = tmp_real;
	in[725].imag = tmp_imag;

	tmp_real = in[687].real;
	tmp_imag = in[687].imag;
	in[687].real = in[981].real;
	in[687].imag = in[981].imag;
	in[981].real = tmp_real;
	in[981].imag = tmp_imag;

	tmp_real = in[691].real;
	tmp_imag = in[691].imag;
	in[691].real = in[821].real;
	in[691].imag = in[821].imag;
	in[821].real = tmp_real;
	in[821].imag = tmp_imag;

	tmp_real = in[695].real;
	tmp_imag = in[695].imag;
	in[695].real = in[949].real;
	in[695].imag = in[949].imag;
	in[949].real = tmp_real;
	in[949].imag = tmp_imag;

	tmp_real = in[699].real;
	tmp_imag = in[699].imag;
	in[699].real = in[885].real;
	in[699].imag = in[885].imag;
	in[885].real = tmp_real;
	in[885].imag = tmp_imag;

	tmp_real = in[701].real;
	tmp_imag = in[701].imag;
	in[701].real = in[757].real;
	in[701].imag = in[757].imag;
	in[757].real = tmp_real;
	in[757].imag = tmp_imag;

	tmp_real = in[703].real;
	tmp_imag = in[703].imag;
	in[703].real = in[1013].real;
	in[703].imag = in[1013].imag;
	in[1013].real = tmp_real;
	in[1013].imag = tmp_imag;

	tmp_real = in[707].real;
	tmp_imag = in[707].imag;
	in[707].real = in[781].real;
	in[707].imag = in[781].imag;
	in[781].real = tmp_real;
	in[781].imag = tmp_imag;

	tmp_real = in[711].real;
	tmp_imag = in[711].imag;
	in[711].real = in[909].real;
	in[711].imag = in[909].imag;
	in[909].real = tmp_real;
	in[909].imag = tmp_imag;

	tmp_real = in[715].real;
	tmp_imag = in[715].imag;
	in[715].real = in[845].real;
	in[715].imag = in[845].imag;
	in[845].real = tmp_real;
	in[845].imag = tmp_imag;

	tmp_real = in[719].real;
	tmp_imag = in[719].imag;
	in[719].real = in[973].real;
	in[719].imag = in[973].imag;
	in[973].real = tmp_real;
	in[973].imag = tmp_imag;

	tmp_real = in[723].real;
	tmp_imag = in[723].imag;
	in[723].real = in[813].real;
	in[723].imag = in[813].imag;
	in[813].real = tmp_real;
	in[813].imag = tmp_imag;

	tmp_real = in[727].real;
	tmp_imag = in[727].imag;
	in[727].real = in[941].real;
	in[727].imag = in[941].imag;
	in[941].real = tmp_real;
	in[941].imag = tmp_imag;

	tmp_real = in[731].real;
	tmp_imag = in[731].imag;
	in[731].real = in[877].real;
	in[731].imag = in[877].imag;
	in[877].real = tmp_real;
	in[877].imag = tmp_imag;

	tmp_real = in[733].real;
	tmp_imag = in[733].imag;
	in[733].real = in[749].real;
	in[733].imag = in[749].imag;
	in[749].real = tmp_real;
	in[749].imag = tmp_imag;

	tmp_real = in[735].real;
	tmp_imag = in[735].imag;
	in[735].real = in[1005].real;
	in[735].imag = in[1005].imag;
	in[1005].real = tmp_real;
	in[1005].imag = tmp_imag;

	tmp_real = in[739].real;
	tmp_imag = in[739].imag;
	in[739].real = in[797].real;
	in[739].imag = in[797].imag;
	in[797].real = tmp_real;
	in[797].imag = tmp_imag;

	tmp_real = in[743].real;
	tmp_imag = in[743].imag;
	in[743].real = in[925].real;
	in[743].imag = in[925].imag;
	in[925].real = tmp_real;
	in[925].imag = tmp_imag;

	tmp_real = in[747].real;
	tmp_imag = in[747].imag;
	in[747].real = in[861].real;
	in[747].imag = in[861].imag;
	in[861].real = tmp_real;
	in[861].imag = tmp_imag;

	tmp_real = in[751].real;
	tmp_imag = in[751].imag;
	in[751].real = in[989].real;
	in[751].imag = in[989].imag;
	in[989].real = tmp_real;
	in[989].imag = tmp_imag;

	tmp_real = in[755].real;
	tmp_imag = in[755].imag;
	in[755].real = in[829].real;
	in[755].imag = in[829].imag;
	in[829].real = tmp_real;
	in[829].imag = tmp_imag;

	tmp_real = in[759].real;
	tmp_imag = in[759].imag;
	in[759].real = in[957].real;
	in[759].imag = in[957].imag;
	in[957].real = tmp_real;
	in[957].imag = tmp_imag;

	tmp_real = in[763].real;
	tmp_imag = in[763].imag;
	in[763].real = in[893].real;
	in[763].imag = in[893].imag;
	in[893].real = tmp_real;
	in[893].imag = tmp_imag;

	tmp_real = in[767].real;
	tmp_imag = in[767].imag;
	in[767].real = in[1021].real;
	in[767].imag = in[1021].imag;
	in[1021].real = tmp_real;
	in[1021].imag = tmp_imag;

	tmp_real = in[775].real;
	tmp_imag = in[775].imag;
	in[775].real = in[899].real;
	in[775].imag = in[899].imag;
	in[899].real = tmp_real;
	in[899].imag = tmp_imag;

	tmp_real = in[779].real;
	tmp_imag = in[779].imag;
	in[779].real = in[835].real;
	in[779].imag = in[835].imag;
	in[835].real = tmp_real;
	in[835].imag = tmp_imag;

	tmp_real = in[783].real;
	tmp_imag = in[783].imag;
	in[783].real = in[963].real;
	in[783].imag = in[963].imag;
	in[963].real = tmp_real;
	in[963].imag = tmp_imag;

	tmp_real = in[787].real;
	tmp_imag = in[787].imag;
	in[787].real = in[803].real;
	in[787].imag = in[803].imag;
	in[803].real = tmp_real;
	in[803].imag = tmp_imag;

	tmp_real = in[791].real;
	tmp_imag = in[791].imag;
	in[791].real = in[931].real;
	in[791].imag = in[931].imag;
	in[931].real = tmp_real;
	in[931].imag = tmp_imag;

	tmp_real = in[795].real;
	tmp_imag = in[795].imag;
	in[795].real = in[867].real;
	in[795].imag = in[867].imag;
	in[867].real = tmp_real;
	in[867].imag = tmp_imag;

	tmp_real = in[799].real;
	tmp_imag = in[799].imag;
	in[799].real = in[995].real;
	in[799].imag = in[995].imag;
	in[995].real = tmp_real;
	in[995].imag = tmp_imag;

	tmp_real = in[807].real;
	tmp_imag = in[807].imag;
	in[807].real = in[915].real;
	in[807].imag = in[915].imag;
	in[915].real = tmp_real;
	in[915].imag = tmp_imag;

	tmp_real = in[811].real;
	tmp_imag = in[811].imag;
	in[811].real = in[851].real;
	in[811].imag = in[851].imag;
	in[851].real = tmp_real;
	in[851].imag = tmp_imag;

	tmp_real = in[815].real;
	tmp_imag = in[815].imag;
	in[815].real = in[979].real;
	in[815].imag = in[979].imag;
	in[979].real = tmp_real;
	in[979].imag = tmp_imag;

	tmp_real = in[823].real;
	tmp_imag = in[823].imag;
	in[823].real = in[947].real;
	in[823].imag = in[947].imag;
	in[947].real = tmp_real;
	in[947].imag = tmp_imag;

	tmp_real = in[827].real;
	tmp_imag = in[827].imag;
	in[827].real = in[883].real;
	in[827].imag = in[883].imag;
	in[883].real = tmp_real;
	in[883].imag = tmp_imag;

	tmp_real = in[831].real;
	tmp_imag = in[831].imag;
	in[831].real = in[1011].real;
	in[831].imag = in[1011].imag;
	in[1011].real = tmp_real;
	in[1011].imag = tmp_imag;

	tmp_real = in[839].real;
	tmp_imag = in[839].imag;
	in[839].real = in[907].real;
	in[839].imag = in[907].imag;
	in[907].real = tmp_real;
	in[907].imag = tmp_imag;

	tmp_real = in[847].real;
	tmp_imag = in[847].imag;
	in[847].real = in[971].real;
	in[847].imag = in[971].imag;
	in[971].real = tmp_real;
	in[971].imag = tmp_imag;

	tmp_real = in[855].real;
	tmp_imag = in[855].imag;
	in[855].real = in[939].real;
	in[855].imag = in[939].imag;
	in[939].real = tmp_real;
	in[939].imag = tmp_imag;

	tmp_real = in[859].real;
	tmp_imag = in[859].imag;
	in[859].real = in[875].real;
	in[859].imag = in[875].imag;
	in[875].real = tmp_real;
	in[875].imag = tmp_imag;

	tmp_real = in[863].real;
	tmp_imag = in[863].imag;
	in[863].real = in[1003].real;
	in[863].imag = in[1003].imag;
	in[1003].real = tmp_real;
	in[1003].imag = tmp_imag;

	tmp_real = in[871].real;
	tmp_imag = in[871].imag;
	in[871].real = in[923].real;
	in[871].imag = in[923].imag;
	in[923].real = tmp_real;
	in[923].imag = tmp_imag;

	tmp_real = in[879].real;
	tmp_imag = in[879].imag;
	in[879].real = in[987].real;
	in[879].imag = in[987].imag;
	in[987].real = tmp_real;
	in[987].imag = tmp_imag;

	tmp_real = in[887].real;
	tmp_imag = in[887].imag;
	in[887].real = in[955].real;
	in[887].imag = in[955].imag;
	in[955].real = tmp_real;
	in[955].imag = tmp_imag;

	tmp_real = in[895].real;
	tmp_imag = in[895].imag;
	in[895].real = in[1019].real;
	in[895].imag = in[1019].imag;
	in[1019].real = tmp_real;
	in[1019].imag = tmp_imag;

	tmp_real = in[911].real;
	tmp_imag = in[911].imag;
	in[911].real = in[967].real;
	in[911].imag = in[967].imag;
	in[967].real = tmp_real;
	in[967].imag = tmp_imag;

	tmp_real = in[919].real;
	tmp_imag = in[919].imag;
	in[919].real = in[935].real;
	in[919].imag = in[935].imag;
	in[935].real = tmp_real;
	in[935].imag = tmp_imag;

	tmp_real = in[927].real;
	tmp_imag = in[927].imag;
	in[927].real = in[999].real;
	in[927].imag = in[999].imag;
	in[999].real = tmp_real;
	in[999].imag = tmp_imag;

	tmp_real = in[943].real;
	tmp_imag = in[943].imag;
	in[943].real = in[983].real;
	in[943].imag = in[983].imag;
	in[983].real = tmp_real;
	in[983].imag = tmp_imag;

	tmp_real = in[959].real;
	tmp_imag = in[959].imag;
	in[959].real = in[1015].real;
	in[959].imag = in[1015].imag;
	in[1015].real = tmp_real;
	in[1015].imag = tmp_imag;

	tmp_real = in[991].real;
	tmp_imag = in[991].imag;
	in[991].real = in[1007].real;
	in[991].imag = in[1007].imag;
	in[1007].real = tmp_real;
	in[1007].imag = tmp_imag;

}
