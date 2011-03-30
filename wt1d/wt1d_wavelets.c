/*
 * wt1d_wavelets.c
 *
 * Definition of all the functions that handle wavelets.
 *
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *   $Id: wt1d_wavelets.c,v 1.13 1998/11/24 16:13:05 decoster Exp $
 */

#include "wt1d_int.h"

/*
 * Definition of the tab structure, used to store numerical versions
 * of the wavelet.
 */

typedef struct _tab_
{
  real *data;  /* Numerical data, computed from the associated function ptr. */
  int  size;   /* Size of the array DATA.                                    */
  real scale;  /* Conrresponding scale of the array. */
  Tab  *next;  /* Link to the next numerical storage. */
} _tab_;

/*
 */
static Tab *
_create_tab_ (int  size,
	      real scale,
	      real *data)
{
  Tab *tab_ptr;

  tab_ptr = (Tab *) malloc (sizeof (Tab));
  if (!tab_ptr) {
    return 0;
  }

  tab_ptr->data = data;

  tab_ptr->size  = size;
  tab_ptr->scale = scale;
  tab_ptr->next  = 0;

  return (tab_ptr);
}


/*
 */
static Tab *
_init_tab_ (int  size,
	    real scale)
{
  Tab  *tab_ptr;
  int  i;
  real *data;

  data = (real *) malloc (size*sizeof (real));
  if (!data) {
      return 0;
  }

  for (i = 0; i < size; i++) {
    data[i] = 0.0;
  }

  tab_ptr = _create_tab_ (size, scale, data);
  if (!tab_ptr) {
    free (data);
    return 0;
  }

  return (tab_ptr);
}

/*
 */
static void
_destroy_tab_ (Tab *tab_ptr)
{
  free (tab_ptr->data);
  free (tab_ptr);

  return;
}


/*
 */
static Tab *
_get_tab_ (Tab  *tab_ptr,
	   real scale)
{
  Tab *current_tab_ptr = tab_ptr;

  while (current_tab_ptr) {
    if (current_tab_ptr->scale == scale) {
      return (current_tab_ptr);
    }
    current_tab_ptr = current_tab_ptr->next;
  }

  return (current_tab_ptr);
}


/*
 */
static void
_add_tab_ (Tab *first_tab_ptr,
	   Tab *tab_to_add_ptr)
{
  Tab *current_tab_ptr;

  assert (first_tab_ptr);

  current_tab_ptr = first_tab_ptr;
  while (current_tab_ptr->next) {
    current_tab_ptr = current_tab_ptr->next;
  }
  current_tab_ptr->next = tab_to_add_ptr;

  return;
}


/*
 */
real *
get_tab_data (Tab  *tab_ptr,
	      int  *tab_size_ptr,
	      real scale)
{
  Tab *current_tab_ptr = tab_ptr;

  while (current_tab_ptr) {
    if (current_tab_ptr->scale == scale) {
      *tab_size_ptr = current_tab_ptr->size;
      return (current_tab_ptr->data);
    }
    current_tab_ptr = current_tab_ptr->next;
  }

  return (0);
}


enum {
  EVEN,
  ODD,
  NONE
};

/* Create (if it doesn't exist) a tab associated to WAVELET at scale
 * SCALE.  The size of the tab is return in TAB_SIZE_PTR. FLAG
 * indicates the form of the wavelet to tab (WAVE_DIRECT or
 * WAVE_FOURIER). The return value is a pointer to the data.  */
real *
wt1d_tab_wavelet (Wavelet *wavelet,
		  real    d_scale,
		  real    dx,
		  int     *tab_size_ptr,
		  int     flag)
{
  Tab     *tab_ptr;
  Tab     *tab_list_ptr;
  int     i;
  real    x;
  real    *real_data;
  complex *cplx_data;
  /*double  (*r_fct_ptr)();
    double  (*i_fct_ptr)();*/
  void *r_fct_ptr, *i_fct_ptr;
  int     i_min;
  int     i_max;
  int     size;
  real    scale;

  real x_scale = 1;
  real y_scale = 1;
  real x_mult = 1;
  real y_mult = 1;

  int even_or_odd = NONE;

  /* Parameters initialisation. */
  if (flag == WAVE_DIRECT) {
    scale = d_scale;

    tab_list_ptr = wavelet->d_tab;
    r_fct_ptr = wavelet->d_r_fct_ptr;
    i_fct_ptr = wavelet->d_i_fct_ptr;
    i_min = (int) floor (wavelet->d_x_min*scale/dx);
    i_max = (int) ceil (wavelet->d_x_max*scale/dx);
    size = i_max - i_min + 1;
    *tab_size_ptr = size;
    y_scale = 1;
    x_scale = 1.0/scale;
  } else {
    scale = d_scale;

    tab_list_ptr = wavelet->f_tab;
    r_fct_ptr = wavelet->f_r_fct_ptr;
    i_fct_ptr = wavelet->f_i_fct_ptr;
    i_min = (int) floor (wavelet->f_x_min*dx/scale);
    i_max = (int) ceil (wavelet->f_x_max*dx/scale);
    size = i_max - i_min + 1;
    *tab_size_ptr = 2*size;
    y_scale = scale;
    x_scale = scale;
  }
      
  switch (wavelet->type) {
  case WAVE_REAL_REAL:
  case WAVE_IMAG_IMAG:
    even_or_odd = EVEN;
    break;
  case WAVE_REAL_IMAG:
  case WAVE_IMAG_REAL:
    even_or_odd = ODD;
    break;
  default:
    even_or_odd = NONE;
    break;
  }

  /* Check if this tab already exists. */
  tab_ptr = _get_tab_ (tab_list_ptr, scale);
  if (tab_ptr) {
    return (tab_ptr->data);
  }

  /* Create the structure. */
  if ((flag == WAVE_DIRECT && wavelet->type == WAVE_REAL_CPLX)
      || (flag == WAVE_FOURIER && wavelet->type == WAVE_CPLX_REAL)
      || wavelet->type == WAVE_REAL_REAL) {
    tab_ptr = _init_tab_ (size, scale);
    if (!tab_ptr) {
      return 0;
    }
  } else {
    tab_ptr = _init_tab_ (size*2, scale);
    if (!tab_ptr) {
      return 0;
    }
  }

  real_data = (real *) tab_ptr->data;
  cplx_data = (complex *) tab_ptr->data;

  for (i = i_min; i <= i_max; i++) {
    x = i;
    if (i < 0) {
      switch (even_or_odd) {
      case EVEN:
	x_mult = -x_scale;
	y_mult = y_scale;
	break;
      case ODD:
	x_mult = -x_scale;
	y_mult = -y_scale;
	break;
      default:
	x_mult = x_scale;
	y_mult = y_scale;
	break;
      }
    } else {
      x_mult = x_scale;
      y_mult = y_scale;
    }
    if ((flag == WAVE_DIRECT && wavelet->type == WAVE_REAL_CPLX)
	|| (flag == WAVE_DIRECT && wavelet->type == WAVE_REAL_IMAG)
	|| (flag == WAVE_FOURIER && wavelet->type == WAVE_CPLX_REAL)
	|| (flag == WAVE_FOURIER && wavelet->type == WAVE_IMAG_REAL)
	|| wavelet->type == WAVE_REAL_REAL) {
      real_data[i - i_min] = y_mult*(real) (evaluator_evaluate_x(r_fct_ptr, x*x_mult));
    } else {
      if (r_fct_ptr) {
	cplx_data[i - i_min].real = y_mult*(evaluator_evaluate_x(r_fct_ptr,x*x_mult));
      } else {
	cplx_data[i - i_min].real = 0.0;
      }
      if (i_fct_ptr) {
	cplx_data[i - i_min].imag = y_mult*(evaluator_evaluate_x(i_fct_ptr,x*x_mult));
      } else {
	cplx_data[i - i_min].imag = 0.0;
      }
    }
  }

  /* Store the tab in the wavelet structure */
  if (tab_list_ptr) {
    _add_tab_ (tab_list_ptr, tab_ptr);
  } else {
    if (flag == WAVE_DIRECT) {
      wavelet->d_tab = tab_ptr;
    } else {
      wavelet->f_tab = tab_ptr;
    }
  }

  return (tab_ptr->data);
}


/*
 * Create a new Wavelet. If allocation fails, returns 0.
 */
Wavelet *
wt1d_new_wavelet (int  type,
		  /*double (*d_r_fct_ptr)(double, double),
		  double (*d_i_fct_ptr)(double, double),
		  double (*f_r_fct_ptr)(double, double),
		  double (*f_i_fct_ptr)(double, double),*/
		  void *d_r_fct_ptr,
		  void *d_i_fct_ptr,
		  void *f_r_fct_ptr,
		  void *f_i_fct_ptr,
		  real d_x_min,
		  real d_x_max,
		  real f_x_min,
		  real f_x_max,
		  real time_scale_mult,
		  real freq_scale_mult)
{
  Wavelet *wavelet;

  assert (d_r_fct_ptr);
  assert (f_r_fct_ptr);
  assert (d_x_min <= 0.0);
  assert (d_x_max >= 0.0);
  assert (type != WAVE_REAL_CPLX || d_x_min == -d_x_max);
  assert (type != WAVE_IMAG_CPLX || d_x_min == -d_x_max);
  assert (type != WAVE_CPLX_REAL || f_x_min == -f_x_max);
  assert (type != WAVE_CPLX_IMAG || f_x_min == -f_x_max);
  assert (type != WAVE_REAL_REAL || d_x_min == -d_x_max);
  assert (type != WAVE_REAL_REAL || f_x_min == -f_x_max);
  assert (type != WAVE_IMAG_REAL || d_x_min == -d_x_max);
  assert (type != WAVE_IMAG_REAL || f_x_min == -f_x_max);
  assert (type != WAVE_REAL_IMAG || d_x_min == -d_x_max);
  assert (type != WAVE_REAL_IMAG || f_x_min == -f_x_max);
  assert (type != WAVE_IMAG_IMAG || d_x_min == -d_x_max);
  assert (type != WAVE_IMAG_IMAG || f_x_min == -f_x_max);
  assert (f_x_min <= 0.0);
  assert (f_x_max >= 0.0);
  assert (time_scale_mult >= 0.0);
  assert (freq_scale_mult >= 0.0);

  wavelet = (Wavelet *) malloc (sizeof (Wavelet));
  if (!wavelet) {
    return 0;
  }

  wavelet->type = type;
  wavelet->d_r_fct_ptr = d_r_fct_ptr;
  wavelet->d_i_fct_ptr = d_i_fct_ptr;
  wavelet->f_r_fct_ptr = f_r_fct_ptr;
  wavelet->f_i_fct_ptr = f_i_fct_ptr;
  wavelet->d_x_min = d_x_min;
  wavelet->d_x_max = d_x_max;
  wavelet->f_x_min = f_x_min;
  wavelet->f_x_max = f_x_max;
  wavelet->time_scale_mult = time_scale_mult;
  wavelet->freq_scale_mult = freq_scale_mult;

  wavelet->d_tab = 0;
  wavelet->f_tab = 0;

  return wavelet;
}


/*
 */
void
wt1d_get_wavelet_attributes (Wavelet *wavelet,
			     int    *type,
			     /*double (**d_r_fct_ptr)(double),
			     double (**d_i_fct_ptr)(double),
			     double (**f_r_fct_ptr)(double),
			     double (**f_i_fct_ptr)(double),*/
			     void **d_r_fct_ptr,
			     void **d_i_fct_ptr,
			     void **f_r_fct_ptr,
			     void **f_i_fct_ptr,
			     real   *d_x_min,
			     real   *d_x_max,
			     real   *f_x_min,
			     real   *f_x_max,
			     real   *time_scale_mult,
			     real   *freq_scale_mult)
{
  assert (wavelet);

  if (type) {
    *type = wavelet->type;
  }
  if (d_r_fct_ptr) {
    *d_r_fct_ptr = wavelet->d_r_fct_ptr;
  }
  if (d_i_fct_ptr) {
    *d_i_fct_ptr = wavelet->d_i_fct_ptr;
  }
  if (f_r_fct_ptr) {
    *f_r_fct_ptr = wavelet->f_r_fct_ptr;
  }
  if (f_i_fct_ptr) {
    *f_i_fct_ptr = wavelet->f_i_fct_ptr;
  }
  if (d_x_min) {
    *d_x_min = wavelet->d_x_min;
  }
  if (d_x_max) {
    *d_x_max = wavelet->d_x_max;
  }
  if (f_x_min) {
    *f_x_min = wavelet->f_x_min;
  }
  if (f_x_max) {
    *f_x_max = wavelet->f_x_max;
  }
  if (time_scale_mult) {
    *time_scale_mult = wavelet->time_scale_mult;
  }
  if (freq_scale_mult) {
    *freq_scale_mult = wavelet->freq_scale_mult;
  }
}


/*
 * Free the memory allocated to a wavelet structure.
 */
void
wt1d_free_wavelet (Wavelet *wavelet)
{
  Tab *current_tab_ptr;
  Tab *next_tab_ptr;

  if (!wavelet) {
    return;
  }

  current_tab_ptr = wavelet->d_tab;
  while (current_tab_ptr) {
    next_tab_ptr = current_tab_ptr->next;
    _destroy_tab_ (current_tab_ptr);
    current_tab_ptr = next_tab_ptr;
  }

  current_tab_ptr = wavelet->f_tab;
  while (current_tab_ptr) {
    next_tab_ptr = current_tab_ptr->next;
    _destroy_tab_ (current_tab_ptr);
    current_tab_ptr = next_tab_ptr;
  }

  free (wavelet);
}


/*
 */
int
wt1d_wavelet_type (Wavelet *wavelet)
{
  return (wavelet->type);
}


/*
 */
void
wt1d_wavelet_scale_limits (Wavelet *wavelet,
			   real    d_size,
			   real    *min_scale,
			   real    *max_scale)
{
  /*
   * The min_scale formula comes from this :
   *   For each scale, the size of the fourier transform of the signal (i.e.
   *   d_size for this function) must be greater than the size of the fourier
   *   transform of the wavelet. These 2 sizes are integers (i.e. number of
   *   points). Here are the formulas that compute the size for the wavelet :
   *     f_step = 2*M_PI/(d_size)
   *     f_x_min_index = (int) floor (f_x_min/scale/f_step)
   *     f_x_max_index = (int) ceil  (f_x_max/scale/f_step)
   *     wv_f_size = 2*max(f_x_max_index, - f_x_min_index) + 1
   *    Scale takes the value min_scale when wv_f_size = d_size. We approximate
   *    floor(x) by x-1 and ceil(x) by x+1.
   */

  *min_scale =
    2*max(wavelet->f_x_max, wavelet->f_x_min)/(2*M_PI*(1-3.0/d_size));

  /*
   * The max_scale formula comes from this :
   *   It's mainly the same consideration but in the direct space in place of
   *   the fourier space. The formulas to use are :
   *     d_x_max_index = (int) floor (d_x_max*scale)
   *     d_x_min_index = (int) ceil  (d_x_min*scale)
   *     wv_d_size = d_x_max_index - d_x_min_index + 1
   *   Scale takes the value max_scale when wv_d_size = d_size.
   */

  *max_scale =
    (real)(d_size-3)/(wavelet->d_x_max - wavelet->d_x_min);
}


/*
 * Fill DATA (an array which size is SIZE_OF_DATA) with the wavelet
 * compute between its min and its max. SCALE gives the scale of the
 * wavelet. FLAG indicates the form to compute (direct or fourier).
 * FIRST_X and LAST_X define the domain of WAVELET at scale SCALE.
 */
void
wt1d_wavelet_to_data (Wavelet *wavelet,
		      real    scale,
		      int     size_of_data,
		      void    *data,
		      real    *first_x,
		      real    *last_x,
		      int     flag)
{
  int     i;
  real    x;
  real    step;
  real    *real_data;
  complex *cplx_data;
  /*double  (*r_fct_ptr)();
    double  (*i_fct_ptr)();*/
  void *r_fct_ptr, *i_fct_ptr;

  double mult = 1;

  assert (wavelet);
  assert (scale > 0.0);
  assert (data);
  assert (size_of_data > 0);

  if (flag == WAVE_DIRECT) {
    r_fct_ptr = wavelet->d_r_fct_ptr;
    i_fct_ptr = wavelet->d_i_fct_ptr;
    step = (wavelet->d_x_max - wavelet->d_x_min)
      * scale / (float) size_of_data;
    *first_x = wavelet->d_x_min * scale;
    *last_x  = wavelet->d_x_max * scale;
    x = wavelet->d_x_min * scale;
  }
  else { /* flag == WAVE_FOURIER */
    r_fct_ptr = wavelet->f_r_fct_ptr;
    i_fct_ptr = wavelet->f_i_fct_ptr;

    step = (wavelet->f_x_max - wavelet->f_x_min)
      / scale / (float) size_of_data;
    *first_x = wavelet->f_x_min / scale;
    *last_x  = wavelet->f_x_max / scale;
    x = wavelet->f_x_min / scale;
    mult = scale;
  }

  real_data = (real *) data;
  cplx_data = (complex *) data;
  for (i = 0; i < size_of_data; i++) {
    x += step;
    if ((flag == WAVE_DIRECT && wavelet->type == WAVE_REAL_CPLX)
	|| (flag == WAVE_FOURIER && wavelet->type == WAVE_CPLX_REAL)
	|| wavelet->type == WAVE_REAL_REAL) {
      real_data[i] = mult*(real) (evaluator_evaluate_x(r_fct_ptr, (double)x*(double)scale));
    } else {
      if (r_fct_ptr) {
	cplx_data[i].real = mult*(real) (evaluator_evaluate_x(r_fct_ptr, (double)x*(double)scale));
      } else {
	cplx_data[i].real = 0.0;
      }
      if (i_fct_ptr) {
	cplx_data[i].imag = mult*(real) (evaluator_evaluate_x(i_fct_ptr, (double)x*(double)scale));
      } else {
	cplx_data[i].imag = 0.0;
      }
    }
  }
}


/*
 */
extern void
wt1d_cv1d_flt_init_with_wavelet (Wavelet *wavelet, real scale)
{
  assert(wavelet);
  assert(scale > 0);

  cv1d_flt_init_a (wavelet->d_x_min,
		   wavelet->d_x_max,
		   wavelet->f_x_min,
		   wavelet->f_x_max,
		   wavelet->d_r_fct_ptr,
		   wavelet->d_i_fct_ptr,
		   wavelet->f_r_fct_ptr,
		   wavelet->f_i_fct_ptr,
		   scale);
}

