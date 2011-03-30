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
 * $Id: gifft_greater_16_cplx.c,v 1.1 1999/05/06 14:09:56 decoster Exp $
 *
*/

# include <math.h>
# include <gfft.h>

void gifft_greater_16_cplx(complex *in, complex *out,int size, int log2size)
{
    gfft_copy_scale_cplx(in,out,size,1.0/size);
    gifft_greater_16_cplx_for_real(out,size,log2size);
    gfft_shuffle(out,size,log2size);
}

