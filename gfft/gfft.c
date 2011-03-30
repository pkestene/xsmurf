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
 * $Id: gfft.c,v 1.1 1999/05/06 14:09:55 decoster Exp $
 *
*/

# include <math.h>
# include <gfft.h>

#ifdef HAVE_LOG2
#define LOG2(x) (int)log2((double)(x))
#else
#define LOG2(x) (int)(log((double)(x))/log((double)2.0))
#endif

#undef LOG2
#define LOG2(i) (_my_log2_(i))

static int
_my_log2_ (int i)
{
  register int tmp, index;

  tmp = i;
  index = 0;
  while (tmp != 1)
    {
      tmp /= 2;
      index++;
    }

  return index;
}

void
gfft_real(real *in, complex *out, int size)
{

    int log2size = LOG2(size);

    switch (log2size)
    {
      case 1:
	gfft_2pts_real(in,out);
	break;
      case 2:
	gfft_4pts_real(in,out);
	break;
      case 3:
	gfft_8pts_real(in,out);
	break;
      case 4:
	gfft_16pts_real(in,out);
	break;
      default:
	gfft_greater_32_real(in,out,size,log2size);
    }
}

void
gifft_real(complex *in, real *out, int size)
{
    
    int log2size = LOG2(size);
    switch (log2size)
    {
      case 1:
	gifft_2pts_real(in,out);
	break;
      case 2:
	gifft_4pts_real(in,out);
	break;
      case 3:
	gifft_8pts_real(in,out);
	break;
      case 4:
	gifft_16pts_real(in,out);
	break;
      default:
	gifft_greater_32_real(in,out,size,log2size);
    }
}

void
gfft_cplx(complex *in, complex *out,int size)
{
    
    int log2size = LOG2(size);
    switch (log2size)
    {
      case 1:
	gfft_2pts_cplx(in,out);
	break;
      case 2:
	gfft_4pts_cplx(in,out);
	break;
      case 3:
	gfft_8pts_cplx(in,out);
	break;
      case 4:
	gfft_16pts_cplx(in,out);
	break;
      default:
	gfft_greater_16_cplx(in,out,size,log2size);
	break;
    }
}

void
gifft_cplx(complex *in, complex *out,int size)
{
    
    int log2size = LOG2(size);
    switch (log2size)
    {
      case 1:
	gifft_2pts_cplx(in,out);
	break;
      case 2:
	gifft_4pts_cplx(in,out);
	break;
      case 3:
	gifft_8pts_cplx(in,out);
	break;
      case 4:
	gifft_16pts_cplx(in,out);
	break;
      default:
	gifft_greater_16_cplx(in,out,size,log2size);
	break;
    }
}




