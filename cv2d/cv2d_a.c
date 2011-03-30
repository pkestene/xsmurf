/*
 * cv2d_a.c --
 *
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv2d_a.c,v 1.1.1.1 1999/05/06 09:38:04 decoster Exp $
 */

#include "cv2d_int.h"
#include "cv2d_limits.h"
#include <string.h>

extern int cv2d_get_good_part_size (int);

/*
 * Array to store all the basic analytical convolution fonctions. Stored by
 * method.
 */

void * (* cv2d_a_fct_ptr_array[2][3])() = {
  {
    cv2d_a_real_d,
    cv2d_a_real_mp,
    cv2d_a_real_ft
  }, {
    cv2d_a_cplx_d,
    cv2d_a_cplx_mp,
    cv2d_a_cplx_ft
  }
};

/*
 * cv2d_a_real --
 */

void *
cv2d_a_real (int  border_effect,
	     void *res_data,
	     int  *first_exact_ptr,
	     int  *last_exact_ptr)
{
  int method;
  real * ret_value = 0;
  void * (*the_cv2d_fct_ptr)();

  LogMessage("a r ");
  if (cv2d_method == CV2D_UNDEFINED) {
    method =
      cv2d_convolution_method_ (im_n, flt2.d_n, lim_array[ANALYTICAL][border_effect]);
  } else {
    method = cv2d_method;
  }
  if ((method == DIRECT_CONVOLUTION) && (!flt2.d_real_ptr && !flt2.d_imag_ptr)) {
    method = MULTI_PART_CONVOLUTION;
  }

  if ((method == FOURIER_TRANSFORM_CONVOLUTION)
      && !cv2d_is_good_fft_size (im_n)) {
    method = MULTI_PART_CONVOLUTION;
  }

#ifdef LOG_MESSAGES
  LogMessage2("%s ", method_str2[method]);
  LogMessage2("%d ", im_n);
  LogMessage2("%d ", flt2.d_n);
#endif
  the_cv2d_fct_ptr = cv2d_a_fct_ptr_array[REAL][method];
  SetLogTimeBegin();
  ret_value = 
    the_cv2d_fct_ptr (border_effect, res_data, first_exact_ptr, last_exact_ptr);
  LogTime();

  return ret_value;
}


/*
 * cv2d_a_real_d --
 */

void *
cv2d_a_real_d (int  border_effect,
	       void *res_data,
	       int  *first_exact_ptr,
	       int  *last_exact_ptr)
{
  real   *signal_data;
  real   *result_data;

  int  x;
  int  y;
  int  ind;

  real *filter_data = 0;
  int  filter_beginx_index;
  int  filter_endx_index;
  int  filter_beginy_index;
  int  filter_endy_index;

  int im_nx = im_n;
  int im_ny = im_n;

  real the_scale;

  _filter_ old_flt;

  assert (flt2.def == ANALYTICAL);
  assert (flt2.form != CV2D_UNDEFINED);
  assert (im_form != CV2D_UNDEFINED);
  assert (im_nx >= flt2.d_n);
  assert (im_ny >= flt2.d_n);

  assert ((border_effect == CV2D_PERIODIC)
	  || (border_effect == CV2D_MIRROR)
	  || (border_effect == CV2D_PADDING)
	  || (border_effect == CV2D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (real *) im_d_data;
  result_data = (real *) res_data;

  if (flt2.scale == CV2D_NO_SCALE) {
    the_scale = 1;
  } else {
    the_scale = flt2.scale;
  }

  filter_beginx_index = (int) floor (flt2.d_begin*the_scale) ;
  filter_endx_index = (int) ceil (flt2.d_end*the_scale) ;
  filter_beginy_index = (int) floor (flt2.d_begin*the_scale) ;
  filter_endy_index = (int) ceil (flt2.d_end*the_scale) ;

  filter_data = (real *) malloc (sizeof(real)*flt2.d_n*flt2.d_n);
  if (!filter_data) {
    EX_RAISE(mem_alloc_error);
  }

  /*
   * Init the filter array and compute the sum of the squared modulus of all
   * the filter values.
   */

  ind = 0;
  cv2d_flt_squared_mod_sum_ = 0.0;
  for (y = filter_beginy_index; y <= filter_endy_index; y++) {
    for (x = filter_beginx_index; x <= filter_endx_index; x++, ind++) {
      filter_data[ind] =
	evaluator_evaluate_x_y(flt2.d_real_ptr,(double)(x)*the_scale, (double)(y)*the_scale);

      cv2d_flt_squared_mod_sum_ += filter_data[ind]*filter_data[ind];
    }
  }
  cv2d_flt_squared_mod_sum_ = cv2d_flt_squared_mod_sum_/(flt2.d_n*flt2.d_n);

  cv2d_flt_copy_ (&flt2, &old_flt);
  cv2d_flt_init_n (flt2.form,
		   filter_endx_index - filter_beginx_index + 1,
		   -filter_beginx_index,
		   0, 0,
		   filter_data,
		   0);

  result_data = cv2d_n_real_d (border_effect, result_data,
			       first_exact_ptr, last_exact_ptr);

  cv2d_flt_copy_ (&old_flt, &flt2);

  free (filter_data);

  /*  flt2.def = old_flt.def;
  flt2.d_begin = old_flt.d_begin;
  flt2.d_end = old_flt.d_end;
  flt2.d_data = old_flt.d_data;*/

  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return result_data;

mem_alloc_error:
  free (filter_data);

  return 0;
}


/*
 * cv2d_a_real_mp --
 */

void *
cv2d_a_real_mp (int  border_effect,
		void *res_data,
		int  *first_exact_ptr,
		int  *last_exact_ptr)
{
  real   *signal_data;
  real   *result_data;

  real    *signal_part = 0;
  complex *signal_part_ft = 0;
  complex *filter_ft = 0;

  int     nb_of_partsx;
  int     nb_of_partsy;
  int     part_nx;
  int     part_ny;
  int     part_nbx;
  int     part_nby;
  int     size_of_exact_datax;
  int     size_of_exact_datay;

  int     im_nx = im_n;
  int     im_ny = im_n;

  int ind;
  int i;
  int j;

  real x;
  real y;

  int     flt_f_beginx_index;
  int     flt_f_endx_index;
  int     flt_d_endx_index;

  int     flt_f_beginy_index;
  int     flt_f_endy_index;
  int     flt_d_endy_index;

  real    f_stepx;
  real    f_stepy;

  real fft_factor; /* Used for non normalised fft (like FFTW) */

  real the_scale;

  assert (flt2.def == ANALYTICAL);
  assert (flt2.form != CV2D_UNDEFINED);
  assert (im_form != CV2D_UNDEFINED);
  assert (im_nx >= flt2.d_n);
  assert (im_ny >= flt2.d_n);

  assert ((border_effect == CV2D_PERIODIC)
	  || (border_effect == CV2D_MIRROR)
	  || (border_effect == CV2D_PADDING)
	  || (border_effect == CV2D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (real *) im_d_data;
  result_data = (real *) res_data;

  part_nx = cv2d_get_good_part_size(flt2.d_n);
  part_ny = cv2d_get_good_part_size(flt2.d_n);

  size_of_exact_datax = part_nx - flt2.d_n + 1;
  size_of_exact_datay = part_ny - flt2.d_n + 1;

  f_stepx = 2*M_PI/(part_nx);
  f_stepy = 2*M_PI/(part_ny);

  nb_of_partsx = ceil (((double) im_nx)/size_of_exact_datax);
  nb_of_partsy = ceil (((double) im_ny)/size_of_exact_datay);

  assert (nb_of_partsx >= 1);
  assert (nb_of_partsy >= 1);

  if (flt2.scale == CV2D_NO_SCALE) {
    the_scale = 1.0;
  } else {
    the_scale = flt2.scale;
  }

  flt_f_beginx_index = (int) floor (flt2.f_begin/the_scale/f_stepx);
  flt_f_endx_index   = (int) ceil  (flt2.f_end/the_scale/f_stepx);
  flt_d_endx_index = (int) ceil (flt2.d_end*the_scale) ;

  flt_f_beginy_index = (int) floor (flt2.f_begin/the_scale/f_stepy);
  flt_f_endy_index   = (int) ceil  (flt2.f_end/the_scale/f_stepy);
  flt_d_endy_index = (int) ceil (flt2.d_end*the_scale) ;

  signal_part = (real *) malloc (sizeof (real)*part_nx*part_ny);
  if (!signal_part) {
    EX_RAISE(mem_alloc_error);
  }
  signal_part_ft = (complex *) malloc (sizeof (complex)*part_nx*(part_ny/2+1));
  if (!signal_part_ft) {
    EX_RAISE(mem_alloc_error);
  }
  filter_ft = (complex *) malloc (sizeof (complex)*part_nx*(part_ny/2+1));
  if (!filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  /*
   * Init the filter array and compute the sum of the squared modulus of all
   * the filter values.
   */

  ind = 0;
  y = 0;
  cv2d_flt_squared_mod_sum_ = 0.0;
  for (j = 0; j < flt_f_endy_index; j++) {
    x = 0;
    for (i = 0; i < (part_nx/2+1); i++) {
      filter_ft[ind].real =
	evaluator_evaluate_x_y(flt2.f_real_ptr,(double)(x), (double)(y));
      if (flt2.f_imag_ptr) {
	filter_ft[ind].imag =
	  evaluator_evaluate_x_y(flt2.f_imag_ptr,(double)(x), (double)(y));
      } else {
	filter_ft[ind].imag = 0.0;
      }

      if (j == 0 || j == part_ny/2) {
	cv2d_flt_squared_mod_sum_ +=
	filter_ft[ind].real*filter_ft[ind].real + filter_ft[ind].imag*filter_ft[ind].imag;
      } else {
	cv2d_flt_squared_mod_sum_ +=
	2*(filter_ft[ind].real*filter_ft[ind].real + filter_ft[ind].imag*filter_ft[ind].imag);
      }

      x += f_stepx*the_scale;
      ind++;
    }
    y += f_stepy*the_scale;
  }
  for (;j < part_nx+flt_f_beginy_index; j++) {
    x = 0;
    for (i = 0; i < (part_nx/2+1); i++) {
      filter_ft[ind].real = 0.0;
      filter_ft[ind].imag = 0.0;
      x += f_stepx*the_scale;
      ind++;
    }
  }
  y = flt_f_beginy_index*f_stepy*the_scale;
  for (; j < part_ny; j++) {
    x = 0;
    for (i = 0; i < (part_nx/2+1); i++) {
      filter_ft[ind].real =
      evaluator_evaluate_x_y(flt2.f_real_ptr,(double)(x), (double)(y));
      if (flt2.f_imag_ptr) {
	filter_ft[ind].imag =
	  evaluator_evaluate_x_y(flt2.f_imag_ptr,(double)(x), (double)(y));
      } else {
	filter_ft[ind].imag = 0.0;
      }

      if (j == 0 || j == part_ny/2) {
	cv2d_flt_squared_mod_sum_ +=
	filter_ft[ind].real*filter_ft[ind].real + filter_ft[ind].imag*filter_ft[ind].imag;
      } else {
	cv2d_flt_squared_mod_sum_ +=
	2*(filter_ft[ind].real*filter_ft[ind].real + filter_ft[ind].imag*filter_ft[ind].imag);
      }

      x += f_stepx*the_scale;
      ind++;
    }
    y += f_stepy*the_scale;
  }
  cv2d_flt_squared_mod_sum_ = cv2d_flt_squared_mod_sum_/(part_nx*part_ny);

  fft_factor = cv2d_get_fft_factor(part_nx, part_ny);

  for (part_nby = 0; part_nby < nb_of_partsy; part_nby++) {
    int part_begin_in_signaly;

    part_begin_in_signaly = part_nby*size_of_exact_datay - flt_d_endy_index;

    for (part_nbx = 0; part_nbx < nb_of_partsx; part_nbx++) {
      int part_begin_in_signalx;

      part_begin_in_signalx = part_nbx*size_of_exact_datax - flt_d_endx_index;
      cv2d_get_part_r_ (signal_part, part_nx, part_ny,
			signal_data, im_nx, im_ny,
			part_begin_in_signalx, part_begin_in_signaly,
			border_effect);

      cv2d_fft_r (signal_part, signal_part_ft, part_nx, part_ny);

      cv2d_cplx_mult_ (signal_part_ft, filter_ft,  part_nx*(part_ny/2+1), fft_factor);
      
      cv2d_fft_r_i (signal_part_ft, signal_part, part_nx, part_ny);

      cv2d_put_part_in_result_r_ (result_data, im_nx, im_ny,
				  signal_part, part_nx, part_ny,
				  part_begin_in_signalx, part_begin_in_signaly,
				  flt_d_endx_index, flt_d_endy_index,
				  size_of_exact_datax, size_of_exact_datay);
    }
  }

  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);
  
  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);

  return 0;
}


/*
 * cv2d_a_real_ft --
 */

void *
cv2d_a_real_ft (int  border_effect,
		void *res_data,
		int  *first_exact_ptr,
		int  *last_exact_ptr)
{
  real   *signal_data;
  real   *result_data;

  real    *new_signal = 0;
  complex *new_signal_ft = 0;

  int     new_size;
  int     flt_f_begin_index;
  int     flt_f_end_index;
  int     flt_d_begin_index;
  int     flt_d_end_index;
  real    f_step;

  real flt_f_real_0;
  real flt_f_imag_0;
  
  assert (flt2.def == ANALYTICAL);
  assert (flt2.form != CV2D_UNDEFINED);
  assert (im_form != CV2D_UNDEFINED);
  assert (im_n >= flt2.d_n);

  assert ((border_effect == CV2D_PERIODIC)
	  || (border_effect == CV2D_MIRROR)
	  || (border_effect == CV2D_PADDING)
	  || (border_effect == CV2D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);
  assert (cv2d_is_good_fft_size (im_n));

  signal_data = (real *) im_d_data;
  result_data = (real *) res_data;

  switch (border_effect) {
  case CV2D_0_PADDING:
    new_size = im_n*2;
    break;
  case CV2D_PADDING:
    new_size = im_n*2;
    break;
  case CV2D_MIRROR:
    new_size = im_n*2;
    break;
  case CV2D_PERIODIC:
    new_size = im_n;
    break;
  }
  f_step = 2*M_PI/(new_size);

  if (flt2.scale == CV2D_NO_SCALE) {
    flt_f_begin_index = (int) floor (flt2.f_begin) ;
    flt_f_end_index = (int) ceil (flt2.f_end) ;
    flt_d_begin_index = (int) floor (flt2.d_begin) ;
    flt_d_end_index = (int) ceil (flt2.d_end) ;
  } else {
    flt_f_begin_index = (int) floor (flt2.f_begin/flt2.scale/f_step);
    flt_f_end_index   = (int) ceil  (flt2.f_end/flt2.scale/f_step);
    flt_d_begin_index = (int) floor (flt2.d_begin*flt2.scale);
    flt_d_end_index   = (int) ceil  (flt2.d_end*flt2.scale);
  }

  assert (new_size >= (2*max(flt_f_end_index, - flt_f_begin_index) + 1));

  /*  switch (border_effect) {
  case CV2D_0_PADDING:
    new_signal = cv2d_0_padding_transform_ (signal_data,
					    im_n,
					    flt_d_end_index);
    break;
  case CV2D_PADDING:
    new_signal = cv2d_padding_transform_ (signal_data,
					  im_n,
					  flt_d_end_index);
    break;
  case CV2D_MIRROR:
    new_signal = cv2d_mirror_transform_ (signal_data,
					 im_n,
					 flt_d_end_index);
    break;
  case CV2D_PERIODIC:
    new_signal = signal_data;
    break;
    }*/
  if (!new_signal) {
    EX_RAISE(mem_alloc_error);
  }

  new_signal_ft = (complex *) malloc (sizeof (complex)*new_size/2);
  if (!new_signal_ft) {
    EX_RAISE(mem_alloc_error);
  }

  /*  cv2d_fft_r (new_signal, new_signal_ft, new_size);*/

  cv2d_flt_squared_mod_sum_ = 0.0;

  if (flt2.scale == CV2D_NO_SCALE) {
    flt_f_real_0 = evaluator_evaluate_x(flt2.f_real_ptr,0.0);
    flt_f_imag_0 = evaluator_evaluate_x(flt2.f_real_ptr,(double)new_size/2*f_step);
    new_signal_ft[0].real *= flt_f_real_0;
    new_signal_ft[0].imag *= flt_f_imag_0;
    cv2d_cplx_mult_num_ana_ (new_signal_ft, flt2.f_real_ptr, flt2.f_imag_ptr,
			     1, new_size/2 - 1, f_step, 0.0);
    cv2d_flt_squared_mod_sum_ *= 2;
    cv2d_flt_squared_mod_sum_ +=
      flt_f_real_0*flt_f_real_0 + flt_f_imag_0*flt_f_imag_0;
  } else {
    flt_f_real_0 = flt2.scale*evaluator_evaluate_x(flt2.f_real_ptr,0.0*flt2.scale);
    flt_f_imag_0 =  flt2.scale
      * evaluator_evaluate_x(flt2.f_real_ptr,(double)new_size/2*f_step*flt2.scale);
    new_signal_ft[0].real *= flt_f_real_0;
    new_signal_ft[0].imag *= flt_f_imag_0;
    cv2d_mult_with_scaled_ft_fct_ (new_signal_ft, flt2.f_real_ptr, flt2.f_imag_ptr,
				   flt2.scale, 1, new_size/2 - 1, f_step, 0.0);
    cv2d_flt_squared_mod_sum_ *= 2;
    cv2d_flt_squared_mod_sum_ +=
      flt_f_real_0*flt_f_real_0 + flt_f_imag_0*flt_f_imag_0;
  }

  cv2d_flt_squared_mod_sum_ = cv2d_flt_squared_mod_sum_/new_size;
  
  if (border_effect == CV2D_PERIODIC) {
    /*    cv2d_fft_r_i (new_signal_ft, result_data, new_size);*/
  } else {
    /*    cv2d_fft_r_i (new_signal_ft, new_signal, new_size);*/
    memcpy (result_data, new_signal, im_n*sizeof (real));
    free (new_signal);
  }

  free (new_signal_ft);
  
  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);
  return res_data;

mem_alloc_error:
  free (new_signal_ft);
  free (new_signal);


  return 0;
}


/*
 * cv2d_a_cplx --
 */

void *
cv2d_a_cplx (int  border_effect,
	     void *res_data,
	     int  *first_exact_ptr,
	     int  *last_exact_ptr)
{
  int method;
  real * ret_value = 0;
  void * (*the_cv2d_fct_ptr)();

  LogMessage("a c ");
  if (cv2d_method == CV2D_UNDEFINED) {
    method =
      cv2d_convolution_method_ (im_n, flt2.d_n, lim_array[ANALYTICAL][border_effect]);
  } else {
    method = cv2d_method;
  }
  if ((method == DIRECT_CONVOLUTION) && (!flt2.d_real_ptr && !flt2.d_imag_ptr)) {
    method = MULTI_PART_CONVOLUTION;
  }
  if ((method == FOURIER_TRANSFORM_CONVOLUTION)
      && !cv2d_is_good_fft_size (im_n)) {
    method = MULTI_PART_CONVOLUTION;
  }

#ifdef LOG_MESSAGES
  LogMessage2("%s ", method_str2[method]);
  LogMessage2("%d ", im_n);
  LogMessage2("%d ", flt2.d_n);
#endif
  the_cv2d_fct_ptr = cv2d_a_fct_ptr_array[CPLX][method];
  SetLogTimeBegin();
  ret_value = 
    the_cv2d_fct_ptr (border_effect, res_data, first_exact_ptr, last_exact_ptr);
  LogTime();
  return ret_value;
}


/*
 * cv2d_a_cplx_d --
 */
void *
cv2d_a_cplx_d (int  border_effect,
	       void *res_data,
	       int  *first_exact_ptr,
	       int  *last_exact_ptr)
{
  complex *signal_data;
  complex *result_data;

  int  x;
  int  y;
  int  ind;

  /*int i;
    int j;*/

  complex *filter_data = 0;

  int  filter_beginx_index;
  int  filter_endx_index;
  int  filter_beginy_index;
  int  filter_endy_index;

  int im_nx = im_n;
  int im_ny = im_n;

  real the_scale;

  _filter_ old_flt;

  assert (flt2.def == ANALYTICAL);
  assert (flt2.form != CV2D_UNDEFINED);
  assert (im_form != CV2D_UNDEFINED);
  assert (im_nx >= flt2.d_n);
  assert (im_ny >= flt2.d_n);

  assert ((border_effect == CV2D_PERIODIC)
	  || (border_effect == CV2D_MIRROR)
	  || (border_effect == CV2D_PADDING)
	  || (border_effect == CV2D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (complex *) im_d_data;
  result_data = (complex *) res_data;

  if (flt2.scale == CV2D_NO_SCALE) {
    the_scale = 1;
  } else {
    the_scale = flt2.scale;
  }

  filter_beginx_index = (int) floor (flt2.d_begin*the_scale) ;
  filter_endx_index = (int) ceil (flt2.d_end*the_scale) ;
  filter_beginy_index = (int) floor (flt2.d_begin*the_scale) ;
  filter_endy_index = (int) ceil (flt2.d_end*the_scale) ;


  filter_data = (complex *) malloc (sizeof(complex)*flt2.d_n*flt2.d_n);
  if (!filter_data) {
    EX_RAISE(mem_alloc_error);
  }

  /*
   * Init the filter array and compute the sum of the squared modulus of all
   * the filter values.
   */

  ind = 0;
  cv2d_flt_squared_mod_sum_ = 0.0;
  for (y = filter_beginy_index; y <= filter_endy_index; y++) {
    for (x = filter_beginx_index; x <= filter_endx_index; x++, ind++) {
      filter_data[ind].real =
	evaluator_evaluate_x_y(flt2.d_real_ptr,(double)(x)*the_scale, (double)(y)*the_scale);
      filter_data[ind].imag =
	evaluator_evaluate_x_y(flt2.d_imag_ptr,(double)(x)*the_scale, (double)(y)*the_scale);

    cv2d_flt_squared_mod_sum_ +=
      filter_data[ind].real*filter_data[ind].real
      + filter_data[ind].imag*filter_data[ind].imag;
    }
  }
  cv2d_flt_squared_mod_sum_ = cv2d_flt_squared_mod_sum_/(flt2.d_n*flt2.d_n);

  cv2d_flt_copy_ (&flt2, &old_flt);
  cv2d_flt_init_n (flt2.form,
		   filter_endx_index - filter_beginx_index + 1,
		   -filter_beginx_index,
		   0, 0,
		   filter_data,
		   0);

  result_data = cv2d_n_cplx_d (border_effect, result_data,
			       first_exact_ptr, last_exact_ptr);

  cv2d_flt_copy_ (&old_flt, &flt2);

  free (filter_data);

  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (filter_data);

  return 0;
}


/*
 * cv2d_a_cplx_mp --
 */

void *
cv2d_a_cplx_mp (int  border_effect,
		void *res_data,
		int  *first_exact_ptr,
		int  *last_exact_ptr)
{
  complex *signal_data;
  complex *result_data;

  complex *signal_part = 0;
  complex *signal_part_ft = 0 ;
  complex *filter_ft = 0;

  int     nb_of_partsx;
  int     nb_of_partsy;
  int     part_nx;
  int     part_ny;
  int     part_nbx;
  int     part_nby;
  int     size_of_exact_datax;
  int     size_of_exact_datay;

  int     im_nx = im_n;
  int     im_ny = im_n;

  int ind;
  int i;
  int j;

  real x;
  real y;

  int     flt_f_beginx_index;
  int     flt_f_endx_index;
  int     flt_d_endx_index;

  int     flt_f_beginy_index;
  int     flt_f_endy_index;
  int     flt_d_endy_index;

  real    f_stepx;
  real    f_stepy;

  real fft_factor; /* Used for non normalised fft (like FFTW) */

  real the_scale;

  assert (flt2.def == ANALYTICAL);
  assert (flt2.form != CV2D_UNDEFINED);
  assert (im_form != CV2D_UNDEFINED);
  assert (im_nx >= flt2.d_n);
  assert (im_ny >= flt2.d_n);

  assert ((border_effect == CV2D_PERIODIC)
	  || (border_effect == CV2D_MIRROR)
	  || (border_effect == CV2D_PADDING)
	  || (border_effect == CV2D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (complex *) im_d_data;
  result_data = (complex *) res_data;

  part_nx = cv2d_get_good_part_size(flt2.d_n);
  part_ny = cv2d_get_good_part_size(flt2.d_n);

  size_of_exact_datax = part_nx - flt2.d_n + 1;
  size_of_exact_datay = part_ny - flt2.d_n + 1;

  f_stepx = 2*M_PI/(part_nx);
  f_stepy = 2*M_PI/(part_ny);

  nb_of_partsx = ceil (((double) im_nx)/size_of_exact_datax);
  nb_of_partsy = ceil (((double) im_ny)/size_of_exact_datay);

  assert (nb_of_partsx >= 1);
  assert (nb_of_partsy >= 1);

  if (flt2.scale == CV2D_NO_SCALE) {
    the_scale = 1.0;
  } else {
    the_scale = flt2.scale;
  }

  flt_f_beginx_index = (int) floor (flt2.f_begin/the_scale/f_stepx);
  flt_f_endx_index   = (int) ceil  (flt2.f_end/the_scale/f_stepx);
  flt_d_endx_index = (int) ceil (flt2.d_end*the_scale) ;

  flt_f_beginy_index = (int) floor (flt2.f_begin/the_scale/f_stepy);
  flt_f_endy_index   = (int) ceil  (flt2.f_end/the_scale/f_stepy);
  flt_d_endy_index = (int) ceil (flt2.d_end*the_scale) ;

  signal_part = (complex *) malloc (sizeof (complex)*part_nx*part_ny);
  if (!signal_part) {
    EX_RAISE(mem_alloc_error);
  }
  signal_part_ft = (complex *) malloc (sizeof (complex)*part_nx*part_ny);
  if (!signal_part_ft) {
    EX_RAISE(mem_alloc_error);
  }
  filter_ft = (complex *) malloc (sizeof (complex)*part_nx*part_ny);
  if (!filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  /*
   * Init the filter array and compute the sum of the squared modulus of all
   * the filter values.
   */

  ind = 0;
  y = 0;
  cv2d_flt_squared_mod_sum_ = 0.0;
  for (j = 0; j < part_ny; j++) {
    x = 0;
    for (i = 0; i < part_nx; i++) {
      if ((i > flt_f_endx_index && i < part_nx+ flt_f_beginx_index)
	  || (j > flt_f_endy_index && j < part_ny+ flt_f_beginy_index)) {
	filter_ft[ind].real = 0.0;
	filter_ft[ind].imag = 0.0;
      } else {
	filter_ft[ind].real =
	  evaluator_evaluate_x_y(flt2.f_real_ptr,(double)(x), (double)(y));
	if (flt2.f_imag_ptr) {
	  filter_ft[ind].imag =
	    evaluator_evaluate_x_y(flt2.f_imag_ptr,(double)(x), (double)(y));
	} else {
	  filter_ft[ind].imag = 0.0;
	}

	if (j == 0 || j == part_ny/2) {
	  cv2d_flt_squared_mod_sum_ +=
	    filter_ft[ind].real*filter_ft[ind].real + filter_ft[ind].imag*filter_ft[ind].imag;
	} else {
	  cv2d_flt_squared_mod_sum_ +=
	    2*(filter_ft[ind].real*filter_ft[ind].real + filter_ft[ind].imag*filter_ft[ind].imag);
	}
      }

      x += f_stepx*the_scale;
      ind++;
    }
    y += f_stepy*the_scale;
  }

  cv2d_flt_squared_mod_sum_ = cv2d_flt_squared_mod_sum_/(part_nx*part_ny);

  fft_factor = cv2d_get_fft_factor(part_nx, part_ny);

  for (part_nby = 0; part_nby < nb_of_partsy; part_nby++) {
    int part_begin_in_signaly;

    part_begin_in_signaly = part_nby*size_of_exact_datay - flt_d_endy_index;

    for (part_nbx = 0; part_nbx < nb_of_partsx; part_nbx++) {
      int part_begin_in_signalx;

      part_begin_in_signalx = part_nbx*size_of_exact_datax - flt_d_endx_index;
      cv2d_get_part_c_ (signal_part, part_nx, part_ny,
			signal_data, im_nx, im_ny,
			part_begin_in_signalx, part_begin_in_signaly,
			border_effect);

      cv2d_fft_c (signal_part, signal_part_ft, part_nx, part_ny);

      cv2d_cplx_mult_ (signal_part_ft, filter_ft,  part_nx*part_ny, fft_factor);
      
      cv2d_fft_c_i (signal_part_ft, signal_part, part_nx, part_ny);

      cv2d_put_part_in_result_c_ (result_data, im_nx, im_ny,
				  signal_part, part_nx, part_ny,
				  part_begin_in_signalx, part_begin_in_signaly,
				  flt_d_endx_index, flt_d_endy_index,
				  size_of_exact_datax, size_of_exact_datay);
      /*    cv2d_get_part_c_ (signal_part, part_n,
		      signal_data, im_n,
		      part_begin_in_signal,
		      border_effect);

        cv2d_fft_c (signal_part, signal_part_ft, part_n);

    cv2d_cplx_mult_ (signal_part_ft, filter_ft,
	  0, part_n - 1);

    cv2d_fft_c_i (signal_part_ft, signal_part, part_n);

      if (part_nb < (nb_of_parts - 1)) {
	memcpy (result_data + part_nb*size_of_exact_data,
		signal_part + flt_d_end_index,
		size_of_exact_data*sizeof (complex));
      } else {
	memcpy (result_data + part_nb*size_of_exact_data,
		signal_part + flt_d_end_index,
		(im_n - part_nb*(size_of_exact_data))*sizeof (complex));
		}*/
    }
  }
  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);
  
  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);

  return 0;
}


/*
 * cv2d_a_cplx_ft --
 */

void *
cv2d_a_cplx_ft (int  border_effect,
		void *res_data,
		int  *first_exact_ptr,
		int  *last_exact_ptr)
{
  complex *signal_data;
  complex *result_data;

  complex *new_signal = 0;
  complex *new_signal_ft = 0;

  int     new_size;
  int     flt_f_begin_index;
  int     flt_f_end_index;
  int     filter_ft_size;
  real    f_step;
  int     tmp_index;
  
  assert (flt2.def == ANALYTICAL);
  assert (flt2.form != CV2D_UNDEFINED);
  assert (im_form != CV2D_UNDEFINED);
  assert (im_n >= flt2.d_n);

  assert ((border_effect == CV2D_PERIODIC)
	  || (border_effect == CV2D_MIRROR)
	  || (border_effect == CV2D_PADDING)
	  || (border_effect == CV2D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (complex *) im_d_data;
  result_data = (complex *) res_data;

  switch (border_effect) {
  case CV2D_0_PADDING:
    new_size = im_n*2;
    break;
  case CV2D_PADDING:
    new_size = im_n*2;
    break;
  case CV2D_MIRROR:
    new_size = im_n*2;
    break;
  case CV2D_PERIODIC:
    new_size = im_n;
    break;
  }
  f_step = 2*M_PI/(new_size);

  if (flt2.scale == CV2D_NO_SCALE) {
    flt_f_begin_index = (int) floor (flt2.f_begin) ;
    flt_f_end_index = (int) ceil (flt2.f_end) ;
    /*    flt_d_begin_index = (int) floor (flt2.d_begin) ;
	  flt_d_end_index = (int) ceil (flt2.d_end) ;*/
  } else {
    flt_f_begin_index = (int) floor (flt2.f_begin/flt2.scale/f_step);
    flt_f_end_index   = (int) ceil  (flt2.f_end/flt2.scale/f_step);
    /*    flt_d_begin_index = (int) floor (flt2.d_begin*flt2.scale);
	  flt_d_end_index   = (int) ceil  (flt2.d_end*flt2.scale);*/
  }

  assert (new_size >= (2*max(flt_f_end_index, - flt_f_begin_index) + 1));
  /*  assert (new_size >= (flt_f_end_index - flt_f_begin_index +1));*/

  filter_ft_size = 2*max(flt_f_end_index, - flt_f_begin_index) + 1;
  /*  if (flt2.scale == CV2D_NO_SCALE) {
      flt_f_begin_index = (int) ceil (flt2.d_begin/f_step) ;
      flt_f_end_index = (int) ceil (flt2.d_end/f_step) ;
      } else {
      flt_f_begin_index = (int) floor (flt2.f_begin/flt2.scale/f_step);
      flt_f_end_index   = (int) ceil  (flt2.f_end/flt2.scale/f_step);
      }

      filter_ft_size = flt_f_end_index - flt_f_begin_index + 1;
      assert (im_n >= filter_ft_size);*/
  /*
  switch (border_effect)
    {
    case CV2D_0_PADDING:
      new_signal = cv2d_cplx_0_padding_transform_ (signal_data,
						   im_n,
						   flt_f_end_index);
      break;
    case CV2D_PADDING:
      new_signal = cv2d_cplx_padding_transform_ (signal_data,
						 im_n,
						 flt_f_end_index);
      break;
    case CV2D_MIRROR:
      new_signal = cv2d_cplx_mirror_transform_ (signal_data,
						im_n,
						flt_f_end_index);
      break;
    case CV2D_PERIODIC:
      new_signal = signal_data;
      break;
      }*/
  if (!new_signal) {
    EX_RAISE(mem_alloc_error);
  }

  new_signal_ft = (complex *) malloc (sizeof (complex)*new_size);
  if (!new_signal_ft) {
    EX_RAISE(mem_alloc_error);
  }

  /*  cv2d_fft_c (new_signal, new_signal_ft, new_size);*/

  cv2d_flt_squared_mod_sum_ = 0.0;

  if (flt2.scale == CV2D_NO_SCALE) {
    tmp_index = flt_f_end_index + (new_size - filter_ft_size)/2;
    cv2d_cplx_mult_num_ana_ (new_signal_ft, flt2.f_real_ptr, flt2.f_imag_ptr,
			     0, tmp_index - 1, f_step,
			     0.0);
    cv2d_cplx_mult_num_ana_ (new_signal_ft, flt2.f_real_ptr, flt2.f_imag_ptr,
			     tmp_index, new_size - 1, f_step,
			     (real)((- new_size)*f_step));
  } else {
    tmp_index = flt_f_end_index + (new_size - filter_ft_size)/2;
    cv2d_mult_with_scaled_ft_fct_ (new_signal_ft, flt2.f_real_ptr, flt2.f_imag_ptr,
				   flt2.scale, 0, tmp_index - 1, f_step,
				   0.0);
    cv2d_mult_with_scaled_ft_fct_ (new_signal_ft, flt2.f_real_ptr, flt2.f_imag_ptr,
				   flt2.scale, tmp_index, new_size - 1, f_step,
				   (real)((- new_size)*f_step));
  }

  cv2d_flt_squared_mod_sum_ = cv2d_flt_squared_mod_sum_/new_size;

  if (border_effect == CV2D_PERIODIC) {
    /*    cv2d_fft_c_i (new_signal_ft, result_data, new_size);*/
  } else {
    /*    cv2d_fft_c_i (new_signal_ft, new_signal, new_size);*/
    memcpy (result_data, new_signal, im_n*sizeof (complex));
    free (new_signal);
  }

  free (new_signal_ft);
  
  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;

 mem_alloc_error:
  free (new_signal);
  free (new_signal_ft);

  return 0;
}

