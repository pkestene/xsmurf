/*
 * cv2d_misc.c --
 *
 *  Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster, April 1997.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *  or  decoster@info.enserb.u-bordeaux.fr
 *
 *   $Id: cv2d_misc.c,v 1.2 1999/05/15 15:58:29 decoster Exp $
 */

#include "cv2d_int.h"
#include <string.h>

/*
 * _my_log2_ --
 * 
 *  New log2 function on integers.
 *
 * Arguments :
 *   i - value to compute. Must be greater or equal than 1.
 *
 * Return Value :
 *   log2(i).
 */

static int
_my_log2_ (int i)
{
  register int tmp, index;

  assert (i >= 1);

  tmp = i;
  index = 0;
  while (tmp != 1) {
    tmp /= 2;
    index++;
  }

  return index;
}


/*
 */
/*static void
_real_copy_ (real *dest,
	     real *src,
	     int  begin,
	     int  end)
{
  assert (dest != 0);
  assert (src != 0);
  assert (end >= begin);

  memcpy (dest, src + begin, (end - begin + 1)*sizeof (real)); 
}
*/


/*
 */
/*static void
_cplx_copy_ (complex *dest,
	     complex *src,
	     int  begin,
	     int  end)
{
  assert (dest != 0);
  assert (src != 0);
  assert (end >= begin);

  memcpy (dest, src + begin, (end - begin + 1)*sizeof (complex)); 
}
*/


/*
 */
int
cv2d_next_power_of_2_ (int i)
{
  int j;

  for (j = 1; j < i; j *= 2);

  return j;
}


/*
 */
int
cv2d_is_power_of_2_ (int i)
{
  real j;

  for (j = i; j > 1; j /= 2);

  return (j == 1);
}


/*
 */
void
cv2d_cplx_mult_ (complex *a,
		 complex *b,
		 int     size,
		 real    factor)
{
  complex      tmp;
  register int i;

  assert (a != 0);
  assert (b != 0);

  if (factor == 1) {
    for (i = 0; i < size; i++) {
      tmp = a[i];
      a[i].real = a[i].real*b[i].real - a[i].imag*b[i].imag;
      a[i].imag = tmp.real*b[i].imag  + tmp.imag*b[i].real;
    }
  } else {
    for (i = 0; i < size; i++) {
      tmp = a[i];
      a[i].real = factor*(a[i].real*b[i].real - a[i].imag*b[i].imag);
      a[i].imag = factor*(tmp.real*b[i].imag  + tmp.imag*b[i].real);
    }
  }
}


/*
 */
void
cv2d_cplx_mult_hc_ (real *a,
		    real *b,
		    int  size)
{
  real tmpr;
  real tmpi;
  register int ir, ii;

  assert (a != 0);
  assert (b != 0);

  a[0] *= b[0];
  for (ir = 1, ii = size-1; ir < size/2; ir++, ii--) {
    tmpr = a[ir];
    tmpi = a[ii];
    a[ir] = a[ir]*b[ir] - a[ii]*b[ii];
    a[ii] = tmpr*b[ii]  + tmpi*b[ir];
  }
  a[size/2] *= b[size/2];
}


/*
 */

double cv2d_flt_squared_mod_sum_;

/*
 */
double
cv2d_get_flt_squared_mod_sum ()
{
  return cv2d_flt_squared_mod_sum_;
}


/* 
 * Compute from BEGIN to END the multiplication between an array of
 * complex data and a complex function. The complex function is given
 * by 2 C-function pointer, one for its real part and one for its
 * imaginary part. If the pointer for the imaginary part is set to
 * NULL, it is considered the complex function has no imaginary
 * part.
 */ 
void
cv2d_cplx_mult_num_ana_ (complex *a,
			 /*double  (*b_real) (),
			   double  (*b_imag) (),*/
			 void *b_real,
			 void *b_imag,
			 int     begin,
			 int     end,
			 real    b_step,
			 real    b_begin)
{
  register complex tmp;
  register real    tmp2;
  register double  b_r;
  register double  b_i;
  register int     i;

  assert (a != 0);
  assert (b_real != 0);

  if (b_imag) {
    for (i = begin; i <= end; i++) {
      tmp = a[i];
      tmp2 = b_begin + i*b_step;
      b_r = evaluator_evaluate_x(b_real,(double)(tmp2));
      b_i = evaluator_evaluate_x(b_imag,(double)(tmp2));
      a[i].real = a[i].real*b_r - a[i].imag*b_i;
      a[i].imag = tmp.real*b_i + tmp.imag*b_r;

      /* Update the sum of the squared mof of the filter. */
      cv2d_flt_squared_mod_sum_ += b_r*b_r + b_i*b_i;
    }
  } else {
    for (i = begin; i <= end; i++) {
      tmp2 = evaluator_evaluate_x(b_real,(double)(b_begin+i*b_step));
      a[i].real *= tmp2;
      a[i].imag *= tmp2;

      /* Update the sum of the squared mof of the filter. */
      cv2d_flt_squared_mod_sum_ += tmp2*tmp2;
    }
  }
}


/* 
 * This fct is only kept in case the fct cv2d_mult_with_scaled_ft_fct_ doesn't
 * work properly...
 */ 
void
cv2d_cplx_mult_num_ana_1p_ (complex *a,
			    /*double  (*b_real) (),
			      double  (*b_imag) (),*/
			    void *b_real,
			    void *b_imag,
			    double  param,
			    int     begin,
			    int     end,
			    real    b_step,
			    real    b_begin)
{
  register complex tmp;
  register real    tmp2;
  register int     i;

  assert (a != 0);
  assert (b_real != 0);

  if (b_imag) {
    for (i = begin; i <= end; i++) {
      tmp = a[i];
      tmp2 = b_begin + i*b_step;
      a[i].real = (a[i].real*evaluator_evaluate_x_y(b_real,(double)(tmp2), param)
		   - a[i].imag*evaluator_evaluate_x_y(b_imag,(double)(tmp2), param));
      a[i].imag = (tmp.real*evaluator_evaluate_x_y(b_imag,(double)(tmp2), param)
		   + tmp.imag*evaluator_evaluate_x_y(b_real,(double)(tmp2), param));
    }
  } else {
    for (i = begin; i <= end; i++) {
      tmp2 = evaluator_evaluate_x_y(b_real,(double)(b_begin+i*b_step), param);
      a[i].real *= tmp2;
      a[i].imag *= tmp2;
    }
  }
}


/* 
 */ 
void
cv2d_mult_with_scaled_ft_fct_ (complex *a,
			       /*double  (*b_real) (),
				 double  (*b_imag) (),*/
			       void *b_real,
			       void *b_imag,
			       double  scale,
			       int     begin,
			       int     end,
			       real    b_step,
			       real    b_begin)
{
  register complex tmp;
  register real    tmp2;
  register int     i;
  register double  b_r;
  register double  b_i;

  assert (a != 0);
  assert (b_real != 0);

  if (b_imag) {
    for (i = begin; i <= end; i++) {
      tmp = a[i];
      tmp2 = (double) (b_begin + i*b_step)*scale;
      b_r = scale*evaluator_evaluate_x(b_real,tmp2);
      b_i = scale*evaluator_evaluate_x(b_imag,tmp2);
      a[i].real = a[i].real*b_r - a[i].imag*b_i;
      a[i].imag = tmp.real *b_i + tmp.imag *b_r;

      /* Update the sum of the squared mof of the filter. */
      cv2d_flt_squared_mod_sum_ += b_r*b_r + b_i*b_i;
    }
  } else {
    for (i = begin; i <= end; i++) {
      tmp2 = scale * evaluator_evaluate_x(b_real,((double)(b_begin+i*b_step))*scale);
      a[i].real *= tmp2;
      a[i].imag *= tmp2;

      /* Update the sum of the squared mof of the filter. */
      cv2d_flt_squared_mod_sum_ += tmp2*tmp2;
    }
  }

  return;
}


/*
 * Return the convolution method to use with a signal of SIGNAL_SIZE
 * data, and a filter of FILTER_SIZE data.
 */
int
cv2d_convolution_method_ (int signal_size,
			  int filter_size,
			  int limits_tab[LIMITS_TAB_SIZE][2])
{
  int index;
  
  assert (signal_size >= filter_size);

  index = _my_log2_ (signal_size) - 1;

  if (index >= LIMITS_TAB_SIZE) {
    index = LIMITS_TAB_SIZE - 1;
  }

  if (filter_size <= limits_tab[index][DIRECT_LIM]) {
    return DIRECT_CONVOLUTION;
  }
  else if (filter_size <= limits_tab[index][MULTI_PART_LIM]) {
    return MULTI_PART_CONVOLUTION;
  }
  else {
    return FOURIER_TRANSFORM_CONVOLUTION;
  }
}


/*
 * RESULT is a periodic signal which period is NEW_SIZE. Between BEGIN
 * and END, RESULT equals to SOURCE. Outside this, it equals to
 * 0.0. RESULT contains data from 0 to NEW_SIZE-1 (one period).
 *
 *                     |
 *                   /"|"""\
 *   transform      /  |    """\  
 *               __/   |        \__
 *               B     0          E    B = BEGIN, E = END
 *
 *              |                                |
 *              |"""\                          /"|
 *   into       |    """\                     /  |
 *              |        \___________________/   |
 *              +                                +
 *              0                               N-1   N = NEW_SIZE
 *
 *
 * Real data.
 */
real *
cv2d_pure_periodic_extend_ (real *source_data,
			    int  beginx,
			    int  endx,
			    int  new_sizex,
			    int  beginy,
			    int  endy,
			    int  new_sizey)
{
  real *result, *source;
  int  i;
  int  j;
  int  nx = endx - beginx + 1;
  int  ny = endy - beginy + 1;
  int gah;

  assert (beginx <= 0);
  assert (beginy <= 0);
  assert (endx >= 0);
  assert (endy >= 0);
  assert (source_data != 0);
  assert (new_sizex >= nx);
  assert (new_sizey >= ny);

  source = source_data - beginx - beginy*nx;
  result = (real *) malloc (sizeof (real) * new_sizex*new_sizey);
  if (!result) {
    return 0;
  }

  for (i = 0; i < new_sizex*new_sizey; i++) {
    result[i] = 0.0;
  }

  for (j = 0; j <= endy; j++) {
    for (i = 0; i <= endx; i++) {
      result[i+j*new_sizex] = source[i+j*nx];
      if (result[i+j*new_sizex] > 2) {
	gah = 1;
      }
    }
    for (i = new_sizex + beginx ; i < new_sizex; i++) {
      result[i+j*new_sizex] = source[(i-new_sizex)+j*nx];
      if (result[i+j*new_sizex] > 2) {
	gah = 1;
      }
    }
  }
  for (j = new_sizey + beginy; j < new_sizey; j++) {
    for (i = 0; i <= endx; i++) {
      result[i+j*new_sizex] = source[i+(j-new_sizey)*nx];
      if (result[i+j*new_sizex] > 2) {
	gah = 1;
      }
    }
    for (i = new_sizex + beginx; i < new_sizex; i++) {
      result[i+j*new_sizex] = source[(i-new_sizex)+(j-new_sizey)*nx];
      if (result[i+j*new_sizex] > 2) {
	gah = 1;
      }
    }
  }

  return result;
}


/*
 * Complex data.
 */
complex *
cv2d_pure_cplx_periodic_extend_ (complex *source_data,
				 int  beginx,
				 int  endx,
				 int  beginy,
				 int  endy,
				 int  new_sizex,
				 int  new_sizey)
     /*				 int     begin,
				 int     end,
				 int     new_size)*/
{
  complex *result, *source;
  int  i;
  int  j;
  int  nx = endx - beginx + 1;
  int  ny = endy - beginy + 1;
  //int gah;

  assert (beginx <= 0);
  assert (beginy <= 0);
  assert (endx >= 0);
  assert (endy >= 0);
  assert (source_data != 0);
  assert (new_sizex >= nx);
  assert (new_sizey >= ny);

  source = source_data - beginx - beginy*nx;
  result = (complex *) malloc (sizeof (complex) * new_sizex*new_sizey);
  if (!result) {
    return 0;
  }

  for (i = 0; i < new_sizex*new_sizey; i++) {
    result[i].real = 0.0;
    result[i].imag = 0.0;
  }

  for (j = 0; j <= endy; j++) {
    for (i = 0; i <= endx; i++) {
      result[i+j*new_sizex] = source[i+j*nx];
    }
    for (i = new_sizex + beginx ; i < new_sizex; i++) {
      result[i+j*new_sizex] = source[(i-new_sizex)+j*nx];
    }
  }
  for (j = new_sizey + beginy; j < new_sizey; j++) {
    for (i = 0; i <= endx; i++) {
      result[i+j*new_sizex] = source[i+(j-new_sizey)*nx];
    }
    for (i = new_sizex + beginx; i < new_sizex; i++) {
      result[i+j*new_sizex] = source[(i-new_sizex)+(j-new_sizey)*nx];
    }
  }

  return result;
  /*  complex *result, *source;
  int  i;

  assert (source_data != 0);
  assert (new_size >= (end - begin +1));

  source = source_data - begin;
  result = (complex *) malloc (sizeof (complex) * new_size);
  if (!result) {
    return 0;
  }

  for (i = 0; i <= end; i++) {
    result[i] = source[i];
  }
  for (; i < (new_size + begin); i++) {
    result[i].real = 0.0;
    result[i].imag = 0.0;
  }
  for (; i < new_size; i++) {
    result[i] = source[i - new_size];
  }

  return result;*/
}


real *
cv2d_extend_r_ (real *source_data,
		int  sizex,
		int  sizey,
		int  new_sizex,
		int  new_sizey,
		int  x_border_1,
		int  x_border_2,
		int  y_border_1,
		int  y_border_2,
		int  border_effect)
{
  real *result;

  real the_value;

  int sx;
  int sy;
  int sx2;
  int sy2;

  int rx;
  int ry;
  int ri;

  assert (source_data != 0);

  result = (real *) malloc (sizeof (real)*new_sizex*new_sizey);
  if (!result) {
    return 0;
  }

  sy = -y_border_1;
  ry = 0;
  ri = 0;
  while (ry < new_sizey) {

    sx = -x_border_1;
    rx = 0;
    while (rx < new_sizex) {

      the_value = -1;
      sx2 = sx;
      sy2 = sy;
      if (sx < 0) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  sx2 = (abs(sx/sizex)+1)*sizex+sx;
	  break;
	case CV2D_0_PADDING:
	  the_value = 0;
	  break;
	case CV2D_PADDING:
	  sx2 = 0;
	  break;
	case CV2D_MIRROR:
	  sx2 = -sx;
	  break;
	}
      }
      if (sy < 0) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  sy2 = (abs(sy/sizey)+1)*sizey+sy;
	  break;
	case CV2D_0_PADDING:
	  the_value = 0;
	  break;
	case CV2D_PADDING:
	  sy2 = 0;
	  break;
	case CV2D_MIRROR:
	  sy2 = -sy;
	  break;
	}
      }
      if (sx >= sizex) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  sx2 = sx % sizex;
	  break;
	case CV2D_0_PADDING:
	  the_value = 0;
	  break;
	case CV2D_PADDING:
	  sx2 = sizex-1;
	  break;
	case CV2D_MIRROR:
	  sx2 = 2*sizex - 2 - sx;
	  break;
	}
      }
      if (sy >= sizey) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  sy2 = sy % sizey;
	  break;
	case CV2D_0_PADDING:
	  the_value = 0;
	  break;
	case CV2D_PADDING:
	  sy2 = sizey-1;
	  break;
	case CV2D_MIRROR:
	  sy2 = 2*sizey - 2 - sy;
	  break;
	}
      }

      if (the_value == 0) {
	result[ri] = 0.0;
      } else {
	result[ri] = source_data[sx2+sizex*sy2];
      }

      rx++;
      ri++;
      sx++;
    }

    ry++;
    sy++;
  }

  return result;
}


complex *
cv2d_extend_c_ (complex *source_data,
		int  sizex,
		int  sizey,
		int  new_sizex,
		int  new_sizey,
		int  x_border_1,
		int  x_border_2,
		int  y_border_1,
		int  y_border_2,
		int  border_effect)
{
  complex *result;

  complex the_value;

  int sx;
  int sy;
  int sx2;
  int sy2;

  int rx;
  int ry;
  int ri;

  assert (source_data != 0);

  result = (complex *) malloc (sizeof (complex)*new_sizex*new_sizey);
  if (!result) {
    return 0;
  }

  sy = -y_border_1;
  ry = 0;
  ri = 0;
  while (ry < new_sizey) {

    sx = -x_border_1;
    rx = 0;
    while (rx < new_sizex) {

      the_value.real = -1;
      sx2 = sx;
      sy2 = sy;
      if (sx < 0) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  sx2 = (abs(sx/sizex)+1)*sizex+sx;
	  break;
	case CV2D_0_PADDING:
	  the_value.real = 0;
	  break;
	case CV2D_PADDING:
	  sx2 = 0;
	  break;
	case CV2D_MIRROR:
	  sx2 = -sx;
	  break;
	}
      }
      if (sy < 0) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  sy2 = (abs(sy/sizey)+1)*sizey+sy;
	  break;
	case CV2D_0_PADDING:
	  the_value.real = 0;
	  break;
	case CV2D_PADDING:
	  sy2 = 0;
	  break;
	case CV2D_MIRROR:
	  sy2 = -sy;
	  break;
	}
      }
      if (sx >= sizex) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  sx2 = sx % sizex;
	  break;
	case CV2D_0_PADDING:
	  the_value.real = 0;
	  break;
	case CV2D_PADDING:
	  sx2 = sizex-1;
	  break;
	case CV2D_MIRROR:
	  sx2 = 2*sizex - 2 - sx;
	  break;
	}
      }
      if (sy >= sizey) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  sy2 = sy % sizey;
	  break;
	case CV2D_0_PADDING:
	  the_value.real = 0;
	  break;
	case CV2D_PADDING:
	  sy2 = sizey-1;
	  break;
	case CV2D_MIRROR:
	  sy2 = 2*sizey - 2 - sy;
	  break;
	}
      }

      if (the_value.real == 0) {
	result[ri].real = 0.0;
	result[ri].imag = 0.0;
      } else {
	result[ri] = source_data[sx2+sizex*sy2];
      }

      rx++;
      ri++;
      sx++;
    }

    ry++;
    sy++;
  }

  return result;
}


/*
 * Create a new signal from signal : 
 *  - which size is SIZE multiply by 2,
 *  - its period is equal to this new_size,
 *  - the data are organized from 0 to new_size-1,
 *  - additionnal data are :
 *
 *              |
 *              |__/"""""""\
 *   transform  |           \__/"""  
 *              +                 +
 *              0                 S-1       S = SIZE
 *
 *              |
 *   into       |__/"""""""\             /""..""""\__
 *              |           \__/"""""\__/  
 *              +                 +                 +
 *              0                 S-1               2*S-1
 *                                            <----->
 *                                              CUT
 * Real data.
 */
/*real *
cv2d_mirror_transform_ (real *source_data,
			int  size,
			int  cut)
{
  real *result;
  int  i;
  int  new_size = size*2;

  assert (source_data != 0);

  result = (real *) malloc (sizeof (real)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(real));
  for (i = size; i <= (new_size - cut); i++) {
    result[i] = source_data[new_size - i - 2];
  }
  for (; i < new_size; i++) {
    result[i] = source_data[new_size - i];
  }

  return result;
}

*/
/*
 * Complex data.
 */
/*complex *
cv2d_cplx_mirror_transform_ (complex *source_data,
			     int     size,
			     int     cut)
{
  complex *result;
  int     i;
  int     new_size = size*2;

  assert (source_data != 0);

  result = (complex *) malloc (sizeof (complex)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(complex));
  for (i = size; i <= (new_size - cut); i++) {
    result[i] = source_data[new_size - i - 2];
  }
  for (; i < new_size; i++) {
    result[i] = source_data[new_size - i];
  }

  return result;
}

*/
/*
 * Create a new signal from signal : 
 *  - which size is SIZE multiply by 2,
 *  - its period is equal to this new_size,
 *  - the data are organized from 0 to new_size-1,
 *  - additionnal data are :
 *
 *              |
 *              |__/"""""""\
 *   transform  |           \__/"""  
 *              +                 +
 *              0                 S-1       S = SIZE
 *
 *              |
 *   into       |__/"""""""\                .._______
 *              |           \__/""""""""""""  
 *              +                 +                 +
 *              0                 S-1               2*S-1
 *                                            <----->
 *                                              CUT
 * Real data. "
 */
/*real *
cv2d_padding_transform_ (real *source_data,
			 int  size,
			 int  cut)
{
  real *result;
  int  i;
  int  new_size = size*2;

  assert (source_data != 0);

  result = (real *) malloc (sizeof (real)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(real));
  for (i = size; i <= (new_size - cut); i++) {
    result[i] = source_data[size - 1];
  }
  for (; i < new_size; i++) {
    result[i] = source_data[0];
  }

  return result;
}

*/
/*real *
cv2d_0_padding_transform_ (real *source_data,
			   int  size,
			   int  cut)
{
  real *result;
  int  i;
  int  new_size = size*2;

  assert (source_data != 0);

  result = (real *) malloc (sizeof (real)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(real));
  for (i = size; i <= (new_size - cut); i++) {
    result[i] = 0.0;
  }
  for (; i < new_size; i++) {
    result[i] = 0.0;
  }

  return result;
}

*/
/*
 * Complex data.
 */
/*complex *
cv2d_cplx_padding_transform_ (complex *source_data,
			      int     size,
			      int     cut)
{
  complex *result;
  int     i;
  int     new_size = size*2;

  assert (source_data != 0);

  result = (complex *) malloc (sizeof (complex)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(complex));
  for (i = size; i <= (new_size - cut); i++) {
    result[i] = source_data[size - 1];
  }
  for (; i < new_size; i++) {
    result[i] = source_data[0];
  }

  return result;
}

*/
/*
 * Complex data.
 */
/*complex *
cv2d_cplx_0_padding_transform_ (complex *source_data,
				int     size,
				int     cut)
{
  complex *result;
  int     i;
  int     new_size = size*2;

  assert (source_data != 0);

  result = (complex *) malloc (sizeof (complex)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(complex));
  for (i = size; i <= (new_size - cut); i++) {
    result[i].real = 0.0;
    result[i].imag = 0.0;
  }
  for (; i < new_size; i++) {
    result[i].real = 0.0;
    result[i].imag = 0.0;
  }

  return result;
}

*/
/*static void
_get_part_r_pe_ (real *signal_part,
		 int  part_sizex,
		 int  part_sizey,
		 real *signal_data,
		 int  signal_sizex,
		 int  signal_sizey,
		 int  part_begin_in_signalx,
		 int  part_begin_in_signaly)
{
  int ix;
  int iy;
  int iind;
  int jx;
  int jy;
  int kx;
  int ky;
  int tmp_n;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_sizex > 0);
  assert (part_sizey > 0);
  assert (signal_sizex > 0);
  assert (signal_sizey > 0);

  jy = part_begin_in_signaly;
  iind = 0;
  for (iy = 0; iy < part_sizey; iy++, jy++) {
    jx = part_begin_in_signalx;
    for (ix = 0; ix < part_sizex; ix++, jx++, iind++) {
      kx = jx;
      ky = jy;
      if (jx < 0) {
	kx = (abs(jx/signal_sizex)+1)*signal_sizex+jx;
      }
      if (jy < 0) {
	ky = (abs(jy/signal_sizey)+1)*signal_sizey+jy;
      }
      if (jx >= signal_sizex) {
	kx = jx % signal_sizex;
      }
      if (jy >= signal_sizey) {
	ky = jy % signal_sizey;
      }
      signal_part[iind] = signal_data[kx+ky*signal_sizex];
    }
  }

  return;
}

*/

void
cv2d_get_part_r_ (real *signal_part,
		  int  part_sizex,
		  int  part_sizey,
		  real *signal_data,
		  int  signal_sizex,
		  int  signal_sizey,
		  int  part_begin_in_signalx,
		  int  part_begin_in_signaly,
		  int  border_effect)
{
  int ix;
  int iy;
  int iind;
  int jx;
  int jy;
  int kx;
  int ky;
  //int tmp_n;
  real sig_val;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_sizex > 0);
  assert (part_sizey > 0);
  assert (signal_sizex > 0);
  assert (signal_sizey > 0);

  jy = part_begin_in_signaly;
  iind = 0;
  for (iy = 0; iy < part_sizey; iy++, jy++) {
    jx = part_begin_in_signalx;
    for (ix = 0; ix < part_sizex; ix++, jx++, iind++) {
      kx = jx;
      ky = jy;
      sig_val = -1;
      if (jx < 0) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  kx = (abs(jx/signal_sizex)+1)*signal_sizex+jx;
	  break;
	case CV2D_0_PADDING:
	  sig_val = 0;
	  break;
	case CV2D_PADDING:
	  kx = 0;
	  break;
	case CV2D_MIRROR:
	  kx = -jx;
	  break;
	}
      }
      if (jy < 0) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  ky = (abs(jy/signal_sizey)+1)*signal_sizey+jy;
	  break;
	case CV2D_0_PADDING:
	  sig_val = 0;
	  break;
	case CV2D_PADDING:
	  ky = 0;
	  break;
	case CV2D_MIRROR:
	  ky = -jy;
	  break;
	}
      }
      if (jx >= signal_sizex) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  kx = jx % signal_sizex;
	  break;
	case CV2D_0_PADDING:
	  sig_val = 0;
	  break;
	case CV2D_PADDING:
	  kx = signal_sizex-1;
	  break;
	case CV2D_MIRROR:
	  kx = 2*signal_sizex - 2 - jx;
	  break;
	}
      }
      if (jy >= signal_sizey) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  ky = jy % signal_sizey;
	  break;
	case CV2D_0_PADDING:
	  sig_val = 0;
	  break;
	case CV2D_PADDING:
	  ky = signal_sizey-1;
	  break;
	case CV2D_MIRROR:
	  ky = 2*signal_sizey - 2 - jy;
	  break;
	}
      }
      /*      if (jx < 0) {
	kx = (abs(jx/signal_sizex)+1)*signal_sizex+jx;
      }
      if (jy < 0) {
	ky = (abs(jy/signal_sizey)+1)*signal_sizey+jy;
      }
      if (jx >= signal_sizex) {
	kx = jx % signal_sizex;
      }
      if (jy >= signal_sizey) {
	ky = jy % signal_sizey;
      }*/
      if (sig_val == -1) {
	signal_part[iind] = signal_data[kx+ky*signal_sizex];
      } else if (sig_val == 0) {
	signal_part[iind] = 0;
      }
    }
  }

  return;
  /*  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_sizex > 0);
  assert (part_sizey > 0);
  assert (signal_sizex > 0);
  assert (signal_sizey > 0);

  switch (border_effect) {
  case CV2D_PERIODIC:
    _get_part_r_pe_ (signal_part, part_sizex, part_sizey,
		     signal_data, signal_sizex, signal_sizey,
		     part_begin_in_signalx, part_begin_in_signaly);
    break;
      case CV2D_MIRROR:
    _get_part_r_mi_ (signal_part, part_sizex, part_sizey,
		     signal_data, signal_sizex, signal_sizey,
		     part_begin_in_signalx, part_begin_in_signaly);
    break;
  case CV2D_PADDING:
    _get_part_r_pa_ (signal_part, part_sizex, part_sizey,
		     signal_data, signal_sizex, signal_sizey,
		     part_begin_in_signalx, part_begin_in_signaly);
    break;
  case CV2D_0_PADDING:
    _get_part_r_0p_ (signal_part, part_sizex, part_sizey,
		     signal_data, signal_sizex, signal_sizey,
		     part_begin_in_signalx, part_begin_in_signaly);
		     break;
  }*/
}


void
cv2d_get_part_c_ (complex *signal_part,
		  int  part_sizex,
		  int  part_sizey,
		  complex *signal_data,
		  int  signal_sizex,
		  int  signal_sizey,
		  int  part_begin_in_signalx,
		  int  part_begin_in_signaly,
		  int  border_effect)
{
  int ix;
  int iy;
  int iind;
  int jx;
  int jy;
  int kx;
  int ky;
  //int tmp_n;
  complex sig_val;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_sizex > 0);
  assert (part_sizey > 0);
  assert (signal_sizex > 0);
  assert (signal_sizey > 0);

  jy = part_begin_in_signaly;
  iind = 0;
  for (iy = 0; iy < part_sizey; iy++, jy++) {
    jx = part_begin_in_signalx;
    for (ix = 0; ix < part_sizex; ix++, jx++, iind++) {
      kx = jx;
      ky = jy;
      sig_val.real = -1;
      if (jx < 0) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  kx = (abs(jx/signal_sizex)+1)*signal_sizex+jx;
	  break;
	case CV2D_0_PADDING:
	  sig_val.real = 0;
	  break;
	case CV2D_PADDING:
	  kx = 0;
	  break;
	case CV2D_MIRROR:
	  kx = -jx;
	  break;
	}
      }
      if (jy < 0) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  ky = (abs(jy/signal_sizey)+1)*signal_sizey+jy;
	  break;
	case CV2D_0_PADDING:
	  sig_val.real = 0;
	  break;
	case CV2D_PADDING:
	  ky = 0;
	  break;
	case CV2D_MIRROR:
	  ky = -jy;
	  break;
	}
      }
      if (jx >= signal_sizex) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  kx = jx % signal_sizex;
	  break;
	case CV2D_0_PADDING:
	  sig_val.real = 0;
	  break;
	case CV2D_PADDING:
	  kx = signal_sizex-1;
	  break;
	case CV2D_MIRROR:
	  kx = 2*signal_sizex - 2 - jx;
	  break;
	}
      }
      if (jy >= signal_sizey) {
	switch (border_effect) {
	case CV2D_PERIODIC:
	  ky = jy % signal_sizey;
	  break;
	case CV2D_0_PADDING:
	  sig_val.real = 0;
	  break;
	case CV2D_PADDING:
	  ky = signal_sizey-1;
	  break;
	case CV2D_MIRROR:
	  ky = 2*signal_sizey - 2 - jy;
	  break;
	}
      }
      /*      if (jx < 0) {
	kx = (abs(jx/signal_sizex)+1)*signal_sizex+jx;
      }
      if (jy < 0) {
	ky = (abs(jy/signal_sizey)+1)*signal_sizey+jy;
      }
      if (jx >= signal_sizex) {
	kx = jx % signal_sizex;
      }
      if (jy >= signal_sizey) {
	ky = jy % signal_sizey;
      }*/
      if (sig_val.real == -1) {
	signal_part[iind] = signal_data[kx+ky*signal_sizex];
      } else if (sig_val.real == 0) {
	signal_part[iind].real = 0;
	signal_part[iind].imag = 0;
      }
    }
  }

  return;
/* (complex *signal_part,
		  int     part_size,
		  complex *signal_data,
		  int     signal_size,
		  int     part_begin_in_signal,
		  int     border_effect)
{
    assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  switch (border_effect) {
  case CV2D_PERIODIC:
    _get_part_c_pe_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  case CV2D_MIRROR:
    _get_part_c_mi_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  case CV2D_PADDING:
    _get_part_c_pa_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  case CV2D_0_PADDING:
    _get_part_c_0p_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  }*/
}


/*
 */
void
cv2d_set_f_l_exact_ (int *first_exact_ptr,
	       int *last_exact_ptr)
{
  int i_begin;
  int i_end;

  switch (flt2.def) {
  case ANALYTICAL:
    if (flt2.scale == CV2D_NO_SCALE) {
      i_begin = (int) floor (flt2.d_begin);
      i_end   = (int) ceil  (flt2.d_end);
    } else {
      i_begin = (int) floor (flt2.d_begin*flt2.scale);
      i_end   = (int) ceil  (flt2.d_end*flt2.scale);
    }
    *first_exact_ptr = (int) i_end;
    *last_exact_ptr  = im_n + (int) i_begin - 1;
    break;
  case NUMERICAL:
    *first_exact_ptr = (int) flt2.d_end;
    *last_exact_ptr  = im_n + (int) flt2.d_begin - 1;
    break;
  }
}


/*
 * cv2d_flt_copy_ --
 *
 *   Copy all fields of a _filter_ into an other.
 *
 * Arguments :
 *   flt1 - _filter_ to copy.
 *   flt2 - Target _filter_.
 *
 * Return Value :
 *   None.
 */

void
cv2d_flt_copy_ (_filter_ *flt1,
		_filter_ *flt2)
{
  assert (flt1);
  assert (flt2);

  flt2->form = flt1->form;
  flt2->def = flt1->def;
  flt2->d_data = flt1->d_data;
  flt2->f_data = flt1->f_data;
  flt2->d_n = flt1->d_n;
  flt2->d_begin = flt1->d_begin;
  flt2->d_end = flt1->d_end;
  flt2->f_n = flt1->f_n;
  flt2->f_begin = flt1->f_begin;
  flt2->f_end = flt1->f_end;
  flt2->d_real_ptr = flt1->d_real_ptr;
  flt2->d_imag_ptr = flt1->d_imag_ptr;
  flt2->f_real_ptr = flt1->f_real_ptr;
  flt2->f_imag_ptr = flt1->f_imag_ptr;
  flt2->scale = flt1->scale;

  return;
}


/*
 * Array to store time for fftw2.
 */

typedef struct _time_
{
  int  size;	/* Size of the array */
  real t;	/* Sum of all the time records */
  real n;	/* Number of time records */
} _time_;

static _time_ fft_r_t[] = {
  { 2 , 0.0020628338679671288 , 1 },
  { 3 , 0.0021529914811253548 , 1 },
  { 4 , 0.0032721441239118576 , 1 },
  { 5 , 0.0025002500042319298 , 1 },
  { 6 , 0.004543802235275507 , 1 },
  { 7 , 0.0020348364487290382 , 1 },
  { 8 , 0.0051602250896394253 , 1 },
  { 9 , 0.0034782609436661005 , 1 },
  { 10 , 0.0057464661076664925 , 1 },
  { 12 , 0.0089573627337813377 , 1 },
  { 14 , 0.005288486834615469 , 1 },
  { 15 , 0.005480653140693903 , 1 },
  { 16 , 0.0065560871735215187 , 1 },
  { 18 , 0.010331645607948303 , 1 },
  { 20 , 0.010237510316073895 , 1 },
  { 21 , 0.0074272132478654385 , 1 },
  { 24 , 0.014615609310567379 , 1 },
  { 25 , 0.0058319238014519215 , 1 },
  { 27 , 0.0082932496443390846 , 1 },
  { 28 , 0.0095831332728266716 , 1 },
  { 30 , 0.014005602337419987 , 1 },
  { 32 , 0.0082719828933477402 , 1 },
  { 35 , 0.008185315877199173 , 1 },
  { 36 , 0.018308311700820923 , 1 },
  { 40 , 0.015151515603065491 , 1 },
  { 42 , 0.015246226452291012 , 1 },
  { 45 , 0.012810658663511276 , 1 },
  { 48 , 0.020876826718449593 , 1 },
  { 49 , 0.0077863428741693497 , 1 },
  { 50 , 0.013222265988588333 , 1 },
  { 54 , 0.018964536488056183 , 1 },
  { 56 , 0.016406890004873276 , 1 },
  { 60 , 0.026990553364157677 , 1 },
  { 63 , 0.015311591327190399 , 1 },
  { 64 , 0.015024038031697273 , 1 },
  { 70 , 0.018298260867595673 , 1 },
  { 72 , 0.030797658488154411 , 1 },
  { 75 , 0.016597511246800423 , 1 },
  { 80 , 0.024697456508874893 , 1 },
  { 81 , 0.014430014416575432 , 1 },
  { 84 , 0.026123302057385445 , 1 },
  { 90 , 0.027374761179089546 , 1 },
  { 96 , 0.031065547838807106 , 1 },
  { 98 , 0.014361625537276268 , 1 },
  { 100 , 0.022163121029734612 , 1 },
  { 105 , 0.021905805915594101 , 1 },
  { 108 , 0.030931023880839348 , 1 },
  { 112 , 0.026226067915558815 , 1 },
  { 120 , 0.037425149232149124 , 1 },
  { 125 , 0.018406037241220474 , 1 },
  { 126 , 0.031318508088588715 , 1 },
  { 128 , 0.029274005442857742 , 1 },
  { 135 , 0.025348542258143425 , 1 },
  { 140 , 0.033344447612762451 , 1 },
  { 144 , 0.042753312736749649 , 1 },
  { 147 , 0.02425418421626091 , 1 },
  { 150 , 0.03249918669462204 , 1 },
  { 160 , 0.038270186632871628 , 1 },
  { 162 , 0.03581661731004715 , 1 },
  { 168 , 0.047236654907464981 , 1 },
  { 175 , 0.027427317574620247 , 1 },
  { 180 , 0.05727376788854599 , 1 },
  { 189 , 0.033944331109523773 , 1 },
  { 192 , 0.045934773981571198 , 1 },
  { 196 , 0.031496062874794006 , 1 },
  { 200 , 0.043122034519910812 , 1 },
  { 210 , 0.051546391099691391 , 1 },
  { 216 , 0.054200541228055954 , 1 },
  { 224 , 0.042625747621059418 , 1 },
  { 225 , 0.036751195788383484 , 1 },
  { 240 , 0.063734866678714752 , 1 },
  { 243 , 0.031867433339357376 , 1 },
  { 245 , 0.034614052623510361 , 1 },
  { 250 , 0.036805298179388046 , 1 },
  { 252 , 0.06064281240105629 , 1 },
  { 256 , 0.042955327779054642 , 1 },
  { 270 , 0.05714285746216774 , 1 },
  { 280 , 0.057372346520423889 , 1 },
  { 288 , 0.066137567162513733 , 1 },
  { 294 , 0.048520136624574661 , 1 },
  { 300 , 0.065789476037025452 , 1 },
  { 315 , 0.057077623903751373 , 1 },
  { 320 , 0.059701491147279739 , 1 },
  { 324 , 0.061576355248689651 , 1 },
  { 336 , 0.073475383222103119 , 1 },
  { 343 , 0.041169203817844391 , 1 },
  { 350 , 0.057903878390789032 , 1 },
  { 360 , 0.088339224457740784 , 1 },
  { 375 , 0.048100046813488007 , 1 },
  { 378 , 0.066755674779415131 , 1 },
  { 384 , 0.070571631193161011 , 1 },
  { 392 , 0.058072008192539215 , 1 },
  { 400 , 0.065274149179458618 , 1 },
  { 405 , 0.054083287715911865 , 1 },
  { 420 , 0.091659031808376312 , 1 },
  { 432 , 0.085034012794494629 , 1 },
  { 441 , 0.057971015572547913 , 1 },
  { 448 , 0.070671379566192627 , 1 },
  { 450 , 0.079554498195648193 , 1 },
  { 480 , 0.10193680226802826 , 1 },
  { 486 , 0.071942448616027832 , 1 },
  { 490 , 0.067704804241657257 , 1 },
  { 500 , 0.070077084004878998 , 1 },
  { 504 , 0.10010010004043579 , 1 },
  { 512 , 0.062814071774482727 , 1 },
  { 525 , 0.073583520948886871 , 1 },
  { 540 , 0.10526315867900848 , 1 },
  { 560 , 0.096805423498153687 , 1 },
  { 567 , 0.076219514012336731 , 1 },
  { 576 , 0.10319917649030685 , 1 },
  { 588 , 0.090252704918384552 , 1 },
  { 600 , 0.10893246531486511 , 1 },
  { 625 , 0.073800735175609589 , 1 },
  { 630 , 0.11614401638507843 , 1 },
  { 640 , 0.097181729972362518 , 1 },
  { 648 , 0.10482180118560791 , 1 },
  { 672 , 0.12690354883670807 , 1 },
  { 675 , 0.088105723261833191 , 1 },
  { 686 , 0.078926600515842438 , 1 },
  { 700 , 0.10373444110155106 , 1 },
  { 720 , 0.1461988240480423 , 1 },
  { 729 , 0.085689805448055267 , 1 },
  { 735 , 0.094966761767864227 , 1 },
  { 750 , 0.10266940295696259 , 1 },
  { 756 , 0.12391573935747147 , 1 },
  { 768 , 0.11750881373882294 , 1 },
  { 784 , 0.1041666641831398 , 1 },
  { 800 , 0.11481056362390518 , 1 },
  { 810 , 0.12165450304746628 , 1 },
  { 840 , 0.1587301641702652 , 1 },
  { 864 , 0.1428571492433548 , 1 },
  { 875 , 0.10288065671920776 , 1 },
  { 882 , 0.125 , 1 },
  { 896 , 0.11848340928554535 , 1 },
  { 900 , 0.1472754031419754 , 1 },
  { 945 , 0.12722645699977875 , 1 },
  { 960 , 0.16181229054927826 , 1 },
  { 972 , 0.13280212879180908 , 1 },
  { 980 , 0.1388888955116272 , 1 },
  { 1000 , 0.13297872245311737 , 1 },
  { 1008 , 0.18115942180156708 , 1 },
  { 1024 , 0.11614401638507843 , 1 },
  { 1029 , 0.12919896841049194 , 1 },
  { 1050 , 0.15479876101016998 , 1 },
  { 1080 , 0.18484288454055786 , 1 },
  { 1120 , 0.16583748161792755 , 1 },
  { 1125 , 0.13404825329780579 , 1 },
  { 1134 , 0.16233766078948975 , 1 },
  { 1152 , 0.17667844891548157 , 1 },
  { 1176 , 0.17152658104896545 , 1 },
  { 1200 , 0.19267822802066803 , 1 },
  { 1215 , 0.14204545319080353 , 1 },
  { 1225 , 0.15197569131851196 , 1 },
  { 1250 , 0.14662756025791168 , 1 },
  { 1260 , 0.21598272025585175 , 1 },
  { 1280 , 0.16611295938491821 , 1 },
  { 1296 , 0.19083969295024872 , 1 },
  { 1323 , 0.16750419139862061 , 1 },
  { 1344 , 0.2024291455745697 , 1 },
  { 1350 , 0.18832391500473022 , 1 },
  { 1372 , 0.16286644339561462 , 1 },
  { 1400 , 0.19230769574642181 , 1 },
  { 1440 , 0.2358490526676178 , 1 },
  { 1458 , 0.18903592228889465 , 1 },
  { 1470 , 0.20080322027206421 , 1 },
  { 1500 , 0.1953125 , 1 },
  { 1512 , 0.2358490526676178 , 1 },
  { 1536 , 0.18975332379341125 , 1 },
  { 1568 , 0.18552875518798828 , 1 },
  { 1575 , 0.1953125 , 1 },
  { 1600 , 0.20325203239917755 , 1 },
  { 1620 , 0.23866347968578339 , 1 },
  { 1680 , 0.27700832486152649 , 1 },
  { 1701 , 0.21834060549736023 , 1 },
  { 1715 , 0.2083333283662796 , 1 },
  { 1728 , 0.24213075637817383 , 1 },
  { 1750 , 0.21881838142871857 , 1 },
  { 1764 , 0.24509803950786591 , 1 },
  { 1792 , 0.21929824352264404 , 1 },
  { 1800 , 0.28169015049934387 , 1 },
  { 1875 , 0.21413275599479675 , 1 },
  { 1890 , 0.28089886903762817 , 1 },
  { 1920 , 0.26246720552444458 , 1 },
  { 1944 , 0.25773194432258606 , 1 },
  { 1960 , 0.25839793682098389 , 1 },
  { 2000 , 0.24570024013519287 , 1 },
  { 2016 , 0.28985506296157837 , 1 },
  { 2025 , 0.24038460850715637 , 1 },
  { 2048 , 0.22172948718070984 , 1 },
  { 2058 , 0.25316455960273743 , 1 },
  { 2100 , 0.30581039190292358 , 1 },
  { 2160 , 0.3333333432674408 , 1 },
  { 2187 , 0.26455026865005493 , 1 },
  { 2205 , 0.27932959794998169 , 1 },
  { 2240 , 0.2985074520111084 , 1 },
  { 2250 , 0.27548208832740784 , 1 },
  { 2268 , 0.31645569205284119 , 1 },
  { 2304 , 0.29673591256141663 , 1 },
  { 2352 , 0.31055900454521179 , 1 },
  { 2400 , 0.35335689783096313 , 1 },
  { 2401 , 0.28328612446784973 , 1 },
  { 2430 , 0.3174603283405304 , 1 },
  { 2450 , 0.29673591256141663 , 1 },
  { 2500 , 0.2958579957485199 , 1 },
  { 2520 , 0.3968254029750824 , 1 },
  { 2560 , 0.30674847960472107 , 1 },
  { 2592 , 0.33557048439979553 , 1 },
  { 2625 , 0.31347963213920593 , 1 },
  { 2646 , 0.33003300428390503 , 1 },
  { 2688 , 0.34129694104194641 , 1 },
  { 2700 , 0.38022813200950623 , 1 },
  { 2744 , 0.32679739594459534 , 1 },
  { 2800 , 0.35971224308013916 , 1 },
  { 2835 , 0.34843206405639648 , 1 },
  { 2880 , 0.42553192377090454 , 1 },
  { 2916 , 0.36496350169181824 , 1 },
  { 2940 , 0.3968254029750824 , 1 },
  { 3000 , 0.4065040647983551 , 1 },
  { 3024 , 0.43478259444236755 , 1 },
  { 3072 , 0.36900368332862854 , 1 },
  { 3087 , 0.38314175605773926 , 1 },
  { 3125 , 0.36900368332862854 , 1 },
  { 3136 , 0.38314175605773926 , 1 },
  { 3150 , 0.42553192377090454 , 1 },
  { 3200 , 0.3731343150138855 , 1 },
  { 3240 , 0.46082949638366699 , 1 },
  { 3360 , 0.49261084198951721 , 1 },
  { 3375 , 0.4048582911491394 , 1 },
  { 3402 , 0.4237288236618042 , 1 },
  { 3430 , 0.4098360538482666 , 1 },
  { 3456 , 0.44247788190841675 , 1 },
  { 3500 , 0.42735043168067932 , 1 },
  { 3528 , 0.4739336371421814 , 1 },
  { 3584 , 0.40816327929496765 , 1 },
  { 3600 , 0.50761419534683228 , 1 },
  { 3645 , 0.4184100329875946 , 1 },
  { 3675 , 0.43668121099472046 , 1 },
  { 3750 , 0.44247788190841675 , 1 },
  { 3780 , 0.53191488981246948 , 1 },
  { 3840 , 0.49504950642585754 , 1 },
  { 3888 , 0.50251257419586182 , 1 },
  { 3920 , 0.48780488967895508 , 1 },
  { 3969 , 0.50251257419586182 , 1 },
  { 4000 , 0.46511629223823547 , 1 },
  { 4032 , 0.54054051637649536 , 1 },
  { 4050 , 0.51546388864517212 , 1 },
  { 4096 , 0.4464285671710968 , 1 },
  { 4116 , 0.49504950642585754 , 1 },
  { 4200 , 0.58479529619216919 , 1 },
  { 4320 , 0.60240966081619263 , 1 },
  { 4374 , 0.54644811153411865 , 1 },
  { 4375 , 0.52083331346511841 , 1 },
  { 4410 , 0.5952380895614624 , 1 },
  { 4480 , 0.58139532804489136 , 1 },
  { 4500 , 0.57142859697341919 , 1 },
  { 4536 , 0.58479529619216919 , 1 },
  { 4608 , 0.5494505763053894 , 1 },
  { 4704 , 0.60975611209869385 , 1 },
  { 4725 , 0.56818181276321411 , 1 },
  { 4800 , 0.625 , 1 },
  { 4802 , 0.54644811153411865 , 1 },
  { 4860 , 0.62111800909042358 , 1 },
  { 4900 , 0.58139532804489136 , 1 },
  { 5000 , 0.58823531866073608 , 1 },
  { 5040 , 0.746268630027771 , 1 },
  { 5103 , 0.60975611209869385 , 1 },
  { 5120 , 0.55865919589996338 , 1 },
  { 5145 , 0.60975611209869385 , 1 },
  { 5184 , 0.65359479188919067 , 1 },
  { 5250 , 0.66225165128707886 , 1 },
  { 5292 , 0.66666668653488159 , 1 },
  { 5376 , 0.67567569017410278 , 1 },
  { 5400 , 0.71428573131561279 , 1 },
  { 5488 , 0.67114096879959106 , 1 },
  { 5600 , 0.70422536134719849 , 1 },
  { 5625 , 0.64935064315795898 , 1 },
  { 5670 , 0.71942448616027832 , 1 },
  { 5760 , 0.75757575035095215 , 1 },
  { 5832 , 0.68965518474578857 , 1 },
  { 5880 , 0.76923078298568726 , 1 },
  { 6000 , 0.69930070638656616 , 1 },
  { 6048 , 0.80000001192092896 , 1 },
  { 6075 , 0.71942448616027832 , 1 },
  { 6125 , 0.75187969207763672 , 1 },
  { 6144 , 0.68965518474578857 , 1 },
  { 6174 , 0.74074071645736694 , 1 },
  { 6250 , 0.72992700338363647 , 1 },
  { 6272 , 0.73529410362243652 , 1 },
  { 6300 , 0.8403361439704895 , 1 },
  { 6400 , 0.746268630027771 , 1 },
  { 6480 , 0.85470086336135864 , 1 },
  { 6561 , 0.76335877180099487 , 1 },
  { 6615 , 0.80000001192092896 , 1 },
  { 6720 , 0.8928571343421936 , 1 },
  { 6750 , 0.81300812959671021 , 1 },
  { 6804 , 0.83333331346511841 , 1 },
  { 6860 , 0.79365080595016479 , 1 },
  { 6912 , 0.86206895112991333 , 1 },
  { 7000 , 0.86206895112991333 , 1 },
  { 7056 , 0.87719297409057617 , 1 },
  { 7168 , 0.80645161867141724 , 1 },
  { 7200 , 0.9523809552192688 , 1 },
  { 7203 , 0.91743117570877075 , 1 },
  { 7290 , 0.87719297409057617 , 1 },
  { 7350 , 0.90090090036392212 , 1 },
  { 7500 , 0.90090090036392212 , 1 },
  { 7560 , 1.0101009607315063 , 1 },
  { 7680 , 0.96153843402862549 , 1 },
  { 7776 , 0.90090090036392212 , 1 },
  { 7840 , 1.0 , 1 },
  { 7875 , 0.9523809552192688 , 1 },
  { 7938 , 0.96153843402862549 , 1 },
  { 8000 , 0.94339621067047119 , 1 },
  { 8064 , 1.0204081535339355 , 1 },
  { 8100 , 1.0309277772903442 , 1 },
  { 8192 , 0.90090090036392212 , 1 },
  { 8232 , 1.0204081535339355 , 1 },
  { 8400 , 1.0869565010070801 , 1 },
  { 8505 , 1.0101009607315063 , 1 },
  { 8575 , 1.0309277772903442 , 1 },
  { 8640 , 1.1235954761505127 , 1 },
  { 8748 , 1.0416666269302368 , 1 },
  { 8750 , 1.0526316165924072 , 1 },
  { 8820 , 1.1764706373214722 , 1 },
  { 8960 , 1.0752688646316528 , 1 },
  { 9000 , 1.1111111640930176 , 1 },
  { 9072 , 1.1764706373214722 , 1 },
  { 9216 , 1.0752688646316528 , 1 },
  { 9261 , 1.1627906560897827 , 1 },
  { 9375 , 1.0752688646316528 , 1 },
  { 9408 , 1.1627906560897827 , 1 },
  { 9450 , 1.2195122241973877 , 1 },
  { 9600 , 1.2195122241973877 , 1 },
  { 9604 , 1.1494252681732178 , 1 },
  { 9720 , 1.2345678806304932 , 1 },
  { 9800 , 1.2168674468994141 , 1 },
  { 10000 , 1.1627906560897827 , 1 },
  { 10080 , 1.3888888359069824 , 1 },
  { 10125 , 1.1904761791229248 , 1 },
  { 10206 , 1.2048193216323853 , 1 },
  { 10240 , 1.1494252681732178 , 1 },
  { 10290 , 1.298701286315918 , 1 },
  { 10368 , 1.25 , 1 },
  { 10500 , 1.3333333730697632 , 1 },
  { 10584 , 1.3888888359069824 , 1 },
  { 10752 , 1.298701286315918 , 1 },
  { 10800 , 1.3698630332946777 , 1 },
  { 10935 , 1.2820513248443604 , 1 },
  { 10976 , 1.298701286315918 , 1 },
  { 11025 , 1.3513513803482056 , 1 },
  { 11200 , 1.3513513803482056 , 1 },
  { 11250 , 1.3513513803482056 , 1 },
  { 11340 , 1.507462739944458 , 1 },
  { 11520 , 1.4428571462631226 , 1 },
  { 11664 , 1.4637681245803833 , 1 },
  { 11760 , 1.578125 , 1 },
  { 11907 , 1.492537260055542 , 1 },
  { 12000 , 1.5151515007019043 , 1 },
  { 12005 , 1.5303030014038086 , 1 },
  { 12096 , 1.5384615659713745 , 1 },
  { 12150 , 1.492537260055542 , 1 },
  { 12250 , 1.5303030014038086 , 1 },
  { 12288 , 1.470588207244873 , 1 },
  { 12348 , 1.6666666269302368 , 1 },
  { 12500 , 1.5151515007019043 , 1 },
  { 12544 , 1.4637681245803833 , 1 },
  { 12600 , 1.5873016119003296 , 1 },
  { 12800 , 1.492537260055542 , 1 },
  { 12960 , 1.6557377576828003 , 1 },
  { 13122 , 1.6666666269302368 , 1 },
  { 13125 , 1.5538461208343506 , 1 },
  { 13230 , 1.7241379022598267 , 1 },
  { 13440 , 1.7241379022598267 , 1 },
  { 13500 , 1.6129032373428345 , 1 },
  { 13608 , 1.7413792610168457 , 1 },
  { 13720 , 1.7241379022598267 , 1 },
  { 13824 , 1.7118643522262573 , 1 },
  { 14000 , 1.8035714626312256 , 1 },
  { 14112 , 1.7543859481811523 , 1 },
  { 14175 , 1.7719298601150513 , 1 },
  { 14336 , 1.6666666269302368 , 1 },
  { 14400 , 1.8867924213409424 , 1 },
  { 14406 , 1.7857142686843872 , 1 },
  { 14580 , 1.9056603908538818 , 1 },
  { 14700 , 1.8867924213409424 , 1 },
  { 15000 , 1.836363673210144 , 1 },
  { 15120 , 2.0612244606018066 , 1 },
  { 15309 , 2.0 , 1 },
  { 15360 , 1.923076868057251 , 1 },
  { 15435 , 1.8867924213409424 , 1 },
  { 15552 , 1.942307710647583 , 1 },
  { 15625 , 2.0408163070678711 , 1 },
  { 15680 , 1.8867924213409424 , 1 },
  { 15750 , 2.0833332538604736 , 1 },
  { 15876 , 2.2173912525177002 , 1 },
  { 16000 , 1.8867924213409424 , 1 },
  { 16128 , 2.0612244606018066 , 1 },
  { 16200 , 2.1041667461395264 , 1 },
  { 16384 , 1.8867924213409424 , 1 },
  { 16464 , 2.0612244606018066 , 1 },
  { 16800 , 2.1702127456665039 , 1 },
  { 16807 , 2.1489362716674805 , 1 },
  { 16875 , 2.0408163070678711 , 1 },
  { 17010 , 2.2444443702697754 , 1 },
  { 17150 , 2.1956522464752197 , 1 },
  { 17280 , 2.1956522464752197 , 1 },
  { 17496 , 2.34883713722229 , 1 },
  { 17500 , 2.1041667461395264 , 1 },
  { 17640 , 2.34883713722229 , 1 },
  { 17920 , 2.1276595592498779 , 1 },
  { 18000 , 2.4634146690368652 , 1 },
  { 18144 , 2.404761791229248 , 1 },
  { 18225 , 2.3809523582458496 , 1 },
  { 18375 , 2.4390244483947754 , 1 },
  { 18432 , 2.34883713722229 , 1 },
  { 18522 , 2.4285714626312256 , 1 },
  { 18750 , 2.2444443702697754 , 1 },
  { 18816 , 2.3809523582458496 , 1 },
  { 18900 , 2.5897436141967773 , 1 },
  { 19200 , 2.404761791229248 , 1 },
  { 19208 , 2.4390244483947754 , 1 },
  { 19440 , 2.4390244483947754 , 1 },
  { 19600 , 2.4634146690368652 , 1 },
  { 19683 , 2.5499999523162842 , 1 },
  { 19845 , 2.5499999523162842 , 1 },
  { 20000 , 2.4285714626312256 , 1 },
  { 20160 , 2.6842105388641357 , 1 },
  { 20250 , 2.5897436141967773 , 1 },
  { 20412 , 2.6842105388641357 , 1 },
  { 20480 , 2.5897436141967773 , 1 },
  { 20580 , 2.7777776718139648 , 1 },
  { 20736 , 2.6315789222717285 , 1 },
  { 21000 , 2.7777776718139648 , 1 },
  { 21168 , 2.8333332538604736 , 1 },
  { 21504 , 2.5641026496887207 , 1 },
  { 21600 , 2.8857142925262451 , 1 },
  { 21609 , 3.0 , 1 },
  { 21870 , 2.8333332538604736 , 1 },
  { 21875 , 2.7777776718139648 , 1 },
  { 21952 , 2.9142856597900391 , 1 },
  { 22050 , 2.7777776718139648 , 1 },
  { 22400 , 2.6315789222717285 , 1 },
  { 22500 , 2.8857142925262451 , 1 },
  { 22680 , 3.125 , 1 },
  { 23040 , 3.0303030014038086 , 1 },
  { 23328 , 2.9142856597900391 , 1 },
  { 23520 , 3.0909090042114258 , 1 },
  { 23625 , 2.970588207244873 , 1 },
  { 23814 , 3.0303030014038086 , 1 },
  { 24000 , 3.0606060028076172 , 1 },
  { 24010 , 3.125 , 1 },
  { 24192 , 3.1875 , 1 },
  { 24300 , 3.2903225421905518 , 1 },
  { 24500 , 3.15625 , 1 },
  { 24576 , 3.0303030014038086 , 1 },
  { 24696 , 3.3333332538604736 , 1 },
  { 25000 , 3.15625 , 1 },
  { 25088 , 3.0909090042114258 , 1 },
  { 25200 , 3.5714285373687744 , 1 },
  { 25515 , 3.3333332538604736 , 1 },
  { 25600 , 3.125 , 1 },
  { 25725 , 3.4482758045196533 , 1 },
  { 25920 , 3.4000000953674316 , 1 },
  { 26244 , 3.4333333969116211 , 1 },
  { 26250 , 3.3333332538604736 , 1 },
  { 26460 , 3.6071429252624512 , 1 },
  { 26880 , 3.4482758045196533 , 1 },
  { 27000 , 3.5714285373687744 , 1 },
  { 27216 , 3.884615421295166 , 1 },
  { 27440 , 3.6071429252624512 , 1 },
  { 27648 , 3.4000000953674316 , 1 },
  { 27783 , 3.6071429252624512 , 1 },
  { 28000 , 3.4333333969116211 , 1 },
  { 28125 , 3.7407407760620117 , 1 },
  { 28224 , 3.6785714626312256 , 1 },
  { 28350 , 3.846153736114502 , 1 },
  { 28672 , 3.5172414779663086 , 1 },
  { 28800 , 3.7777776718139648 , 1 },
  { 28812 , 3.846153736114502 , 1 },
  { 29160 , 3.884615421295166 , 1 },
  { 29400 , 3.923076868057251 , 1 },
  { 30000 , 3.923076868057251 , 1 },
  { 30240 , 4.3333334922790527 , 1 },
  { 30375 , 3.923076868057251 , 1 },
  { 30618 , 4.0399999618530273 , 1 },
  { 30625 , 4.25 , 1 },
  { 30720 , 3.7777776718139648 , 1 },
  { 30870 , 4.0799999237060547 , 1 },
  { 31104 , 4.119999885559082 , 1 },
  { 31250 , 4.25 , 1 },
  { 31360 , 4.0 , 1 },
  { 31500 , 4.25 , 1 },
  { 31752 , 4.2916665077209473 , 1 },
  { 32000 , 3.8148148059844971 , 1 },
  { 32256 , 4.1666665077209473 , 1 },
  { 32400 , 4.4347825050354004 , 1 },
  { 32768 , 4.0799999237060547 , 1 },
  { 32805 , 4.2083334922790527 , 1 },
  { 32928 , 4.3913044929504395 , 1 },
  { 33075 , 4.3478260040283203 , 1 },
  { 33600 , 4.5909090042114258 , 1 },
  { 33614 , 4.4782609939575195 , 1 },
  { 33750 , 4.3478260040283203 , 1 },
  { 34020 , 4.6363635063171387 , 1 },
  { 34300 , 4.4347825050354004 , 1 },
  { 34560 , 4.4347825050354004 , 1 },
  { 34992 , 4.4782609939575195 , 1 },
  { 35000 , 4.5454545021057129 , 1 },
  { 35280 , 4.8095235824584961 , 1 },
  { 35721 , 5.0 , 1 },
  { 35840 , 4.8095235824584961 , 1 },
  { 36000 , 5.0999999046325684 , 1 },
  { 36015 , 5.0 , 1 },
  { 36288 , 4.7619047164916992 , 1 },
  { 36450 , 4.7272725105285645 , 1 },
  { 36750 , 4.904761791229248 , 1 },
  { 36864 , 4.8095235824584961 , 1 },
  { 37044 , 5.263157844543457 , 1 },
  { 37500 , 4.8571429252624512 , 1 },
  { 37632 , 4.4782609939575195 , 1 },
  { 37800 , 5.4210524559020996 , 1 },
  { 38400 , 5.0999999046325684 , 1 },
  { 38416 , 5.25 , 1 },
  { 38880 , 5.263157844543457 , 1 },
  { 39200 , 5.1999998092651367 , 1 },
  { 39366 , 5.6666665077209473 , 1 },
  { 39375 , 5.3157896995544434 , 1 },
  { 39690 , 5.4210524559020996 , 1 },
  { 40000 , 5.0999999046325684 , 1 },
  { 40320 , 5.7777776718139648 , 1 },
  { 40500 , 5.4210524559020996 , 1 },
  { 40824 , 5.5263156890869141 , 1 },
  { 40960 , 5.6111111640930176 , 1 },
  { 41160 , 5.7222223281860352 , 1 },
  { 41472 , 5.7222223281860352 , 1 },
  { 42000 , 5.4736843109130859 , 1 },
  { 42336 , 6.25 , 1 },
  { 42525 , 5.8333334922790527 , 1 },
  { 42875 , 5.7777776718139648 , 1 },
  { 43008 , 5.5555553436279297 , 1 },
  { 43200 , 6.0588235855102539 , 1 },
  { 43218 , 6.0588235855102539 , 1 },
  { 43740 , 6.25 , 1 },
  { 43750 , 6.0 , 1 },
  { 43904 , 5.7777776718139648 , 1 },
  { 44100 , 6.1176471710205078 , 1 },
  { 44800 , 5.9411764144897461 , 1 },
  { 45000 , 6.25 , 1 },
  { 45360 , 6.6666665077209473 , 1 },
  { 45927 , 6.5 , 1 },
  { 46080 , 6.2352943420410156 , 1 },
  { 46305 , 6.4375 , 1 },
  { 46656 , 6.5 , 1 },
  { 46875 , 6.625 , 1 },
  { 47040 , 6.4375 , 1 },
  { 47250 , 6.6666665077209473 , 1 },
  { 47628 , 6.6666665077209473 , 1 },
  { 48000 , 6.375 , 1 },
  { 48020 , 6.7333331108093262 , 1 },
  { 48384 , 6.8000001907348633 , 1 },
  { 48600 , 6.7333331108093262 , 1 },
  { 49000 , 6.5625 , 1 },
  { 49152 , 6.375 , 1 },
  { 49392 , 6.8666667938232422 , 1 },
  { 50000 , 6.7333331108093262 , 1 },
  { 50176 , 6.8000001907348633 , 1 },
  { 50400 , 7.2857141494750977 , 1 },
  { 50421 , 7.769230842590332 , 1 },
  { 50625 , 7.3571429252624512 , 1 },
  { 51030 , 7.1428570747375488 , 1 },
  { 51200 , 7.0666666030883789 , 1 },
  { 51450 , 7.0 , 1 },
  { 51840 , 7.9230771064758301 , 1 },
  { 52488 , 8.8333330154418945 , 1 },
  { 52500 , 8.1538457870483398 , 1 },
  { 52920 , 7.846153736114502 , 1 },
  { 53760 , 7.769230842590332 , 1 },
  { 54000 , 7.846153736114502 , 1 },
  { 54432 , 8.0 , 1 },
  { 54675 , 7.846153736114502 , 1 },
  { 54880 , 8.0 , 1 },
  { 55125 , 8.230769157409668 , 1 },
  { 55296 , 7.846153736114502 , 1 },
  { 55566 , 8.0 , 1 },
  { 56000 , 7.769230842590332 , 1 },
  { 56250 , 8.0769233703613281 , 1 },
  { 56448 , 8.0 , 1 },
  { 56700 , 7.769230842590332 , 1 },
  { 57344 , 7.846153736114502 , 1 },
  { 57600 , 8.1538457870483398 , 1 },
  { 57624 , 8.5 , 1 },
  { 58320 , 8.5833330154418945 , 1 },
  { 58800 , 8.3076925277709961 , 1 },
  { 59049 , 8.3333330154418945 , 1 },
  { 59535 , 8.4166669845581055 , 1 },
  { 60000 , 8.6666669845581055 , 1 },
  { 60025 , 8.75 , 1 },
  { 60480 , 9.0 , 1 },
  { 60750 , 8.8333330154418945 , 1 },
  { 61236 , 9.0 , 1 },
  { 61250 , 8.9166669845581055 , 1 },
  { 61440 , 8.8333330154418945 , 1 },
  { 61740 , 9.3636360168457031 , 1 },
  { 62208 , 9.1818180084228516 , 1 },
  { 62500 , 9.4545450210571289 , 1 },
  { 62720 , 8.75 , 1 },
  { 63000 , 9.6363639831542969 , 1 },
  { 63504 , 9.0909090042114258 , 1 },
  { 64000 , 9.1818180084228516 , 1 },
  { 64512 , 9.3636360168457031 , 1 },
  { 64800 , 9.0 , 1 },
  { 64827 , 10.0 , 1 },
  { 65536 , 9.5454549789428711 , 1 }
};

#define ARR_SIZE 316

int fftw2_time_max_size = 65536;

real
my_fftw2_get_time (int size)
{
  int i = 0;

  while (fft_r_t[i].size < size && i < ARR_SIZE) {
    i++;
  }

  if (i == ARR_SIZE) {
    /*
     * If the requested size is not on the array, we compute a kind of
     * estimation on the basis: time = k * size * log2(size). k is
     * estimated with the last time record.
     */

    real k;
    int  s = fft_r_t[ARR_SIZE-1].size;
    real t = fft_r_t[ARR_SIZE-1].t;
    int  n = fft_r_t[ARR_SIZE-1].n;
    int good_size;

    k = t*log(2)/(n*s*log(s));
    good_size = cv2d_next_good_fft_size(size);

    return (k*good_size*log(good_size)/log(2));
  }

  return (fft_r_t[i].t/fft_r_t[i].n);
}


/*
 * cv2d_get_good_part_size --
 */

int
cv2d_get_good_part_size (int size)
{
  real t_min;
  int ze_size;
  real fft_t;
  real t;
  int p_size;

  int    begin, end;
  struct tms time;

  times (&time);
  begin = time.tms_utime;

  p_size = cv2d_next_good_fft_size(size + 1);
  fft_t = my_fftw2_get_time(p_size);
  t = fft_t / (p_size - size);
  t_min = t;
  ze_size = p_size;

  while (t < 2*t_min && size*100 > p_size) {
    p_size = cv2d_next_good_fft_size(p_size + 1);
    fft_t = my_fftw2_get_time(p_size);
    t = fft_t / (p_size - size);
    if (t < t_min) {
      t_min = t;
      ze_size = p_size;
    }
  }

  times (&time);
  end = time.tms_utime;
  /*  printf ("haha - %d\n", end - begin);*/

  return ze_size;
}

/*
 */

void
cv2d_put_part_in_result_r_ (real *result_data,
			    int res_nx, int res_ny,
			    real *part_data,
			    int part_nx, int part_ny,
			    int x, int y,
			    int x_offset, int y_offset,
			    int width, int height)
{
  int rx;
  int ry;
  int ri;
  int px;
  int py;
  int pi;

  int gah;

  ry = y + y_offset;
  py = y_offset;

  while (ry < res_ny && py < part_ny) {

    rx = x + x_offset;
    ri = ry*res_nx +rx;
    px = x_offset;
    pi = py*part_nx + px;

    while (rx < res_nx && px < part_nx) {
      if (rx < 0 || rx >= res_nx || ry < 0 || ry >= res_ny
	  || px < 0 || px >= part_nx || py < 0 || py >= part_ny ) {
	gah = 0;
      }
      result_data[ri] = part_data[pi];


      rx++;
      ri++;
      px++;
      pi++;
    }

    ry++;
    py++;
  }

  return;
}


/*
 */

void
cv2d_put_part_in_result_c_ (complex *result_data,
			    int res_nx, int res_ny,
			    complex *part_data,
			    int part_nx, int part_ny,
			    int x, int y,
			    int x_offset, int y_offset,
			    int width, int height)
{
  int rx;
  int ry;
  int ri;
  int px;
  int py;
  int pi;

  int gah;

  ry = y + y_offset;
  py = y_offset;

  while (ry < res_ny && py < part_ny) {

    rx = x + x_offset;
    ri = ry*res_nx +rx;
    px = x_offset;
    pi = py*part_nx + px;

    while (rx < res_nx && px < part_nx) {
      if (rx < 0 || rx >= res_nx || ry < 0 || ry >= res_ny
	  || px < 0 || px >= part_nx || py < 0 || py >= part_ny ) {
	gah = 0;
      }
      result_data[ri] = part_data[pi];


      rx++;
      ri++;
      px++;
      pi++;
    }

    ry++;
    py++;
  }

  return;
}


/*
 */

void
cv2d_put_signal_in_result_r_ (real *result_data,
			      real *signal_data,
			      int  sizex,
			      int  sizey,
			      int  new_sizex,
			      int  new_sizey,
			      int  x_border_1,
			      int  x_border_2,
			      int  y_border_1,
			      int  y_border_2)
{
  int ri;
  int ry;
  int si;

  ri = 0;
  si = x_border_1 + y_border_1*new_sizex;
  ry = 0;

  while (ry < sizey) {
    memcpy (&result_data[ri], &signal_data[si], sizex*sizeof (real));

    ri += sizex;
    si += new_sizex;
    ry++;
  }

  return;
}


/*
 */

void
cv2d_put_signal_in_result_c_ (complex *result_data,
			      complex *signal_data,
			      int  sizex,
			      int  sizey,
			      int  new_sizex,
			      int  new_sizey,
			      int  x_border_1,
			      int  x_border_2,
			      int  y_border_1,
			      int  y_border_2)
{
  int ri;
  int ry;
  int si;

  ri = 0;
  si = x_border_1 + y_border_1*new_sizex;
  ry = 0;

  while (ry < sizey) {
    memcpy (&result_data[ri], &signal_data[si], sizex*sizeof (complex));

    ri += sizex;
    si += new_sizex;
    ry++;
  }

  return;
}

