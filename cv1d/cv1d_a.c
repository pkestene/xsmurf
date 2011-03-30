/*
 * cv1d_a.c --
 *
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv1d_a.c,v 1.15 1999/05/05 20:42:24 decoster Exp $
 */

#include "cv1d_int.h"
#include "cv1d_limits.h"
#include <string.h> 

/*
 * Array to store all the basic analytical convolution fonctions. Stored by
 * method.
 */

void * (* cv1d_a_fct_ptr_array[2][3])() = {
  {
    cv1d_a_real_d,
    cv1d_a_real_mp,
    cv1d_a_real_ft
  }, {
    cv1d_a_cplx_d,
    cv1d_a_cplx_mp,
    cv1d_a_cplx_ft
  }
};

/*
 * cv1d_a_real --
 */

void *
cv1d_a_real (int  border_effect,
	     void *res_data,
	     int  *first_exact_ptr,
	     int  *last_exact_ptr)
{
  int method;
  real * ret_value = 0;
  void * (*the_cv1d_fct_ptr)();

  LogMessage("a r ");
  if (cv1d_method == CV1D_UNDEFINED) {
    method =
      cv1d_convolution_method_ (sig_n, flt.d_n, lim_array[ANALYTICAL][border_effect]);
  } else {
    method = cv1d_method;
  }
  if ((method == DIRECT_CONVOLUTION) && (!flt.d_real_ptr && !flt.d_imag_ptr)) {
    method = MULTI_PART_CONVOLUTION;
  }

  if ((method == FOURIER_TRANSFORM_CONVOLUTION)
      && !cv1d_is_good_fft_size (sig_n)) {
    method = MULTI_PART_CONVOLUTION;
  }

#ifdef LOG_MESSAGES
  LogMessage2("%s ", method_str[method]);
  LogMessage2("%d ", sig_n);
  LogMessage2("%d ", flt.d_n);
#endif
  the_cv1d_fct_ptr = cv1d_a_fct_ptr_array[REAL][method];
  SetLogTimeBegin();
  ret_value = 
    the_cv1d_fct_ptr (border_effect, res_data, first_exact_ptr, last_exact_ptr);
  LogTime();

  return ret_value;
}


/*
 * cv1d_a_real_d --
 */

void *
cv1d_a_real_d (int  border_effect,
	       void *res_data,
	       int  *first_exact_ptr,
	       int  *last_exact_ptr)
{
  real   *signal_data;
  real   *result_data;

  int  i;
  real *filter_data = 0;
  int  filter_begin_index;
  int  filter_end_index;

  _filter_ old_flt;

  assert (flt.def == ANALYTICAL);
  assert (flt.form != CV1D_UNDEFINED);
  assert (sig_form != CV1D_UNDEFINED);
  assert (sig_n >= flt.d_n);

  assert ((border_effect == CV1D_PERIODIC)
	  || (border_effect == CV1D_MIRROR)
	  || (border_effect == CV1D_PADDING)
	  || (border_effect == CV1D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (real *) sig_d_data;
  result_data = (real *) res_data;

  if (flt.scale == CV1D_NO_SCALE) {
    filter_begin_index = (int) floor (flt.d_begin) ;
    filter_end_index = (int) ceil (flt.d_end) ;
  } else {
    filter_begin_index = (int) floor (flt.d_begin*flt.scale) ;
    filter_end_index = (int) ceil (flt.d_end*flt.scale) ;
  }

  filter_data = (real *) malloc (sizeof(real)*flt.d_n);
  if (!filter_data) {
    EX_RAISE(mem_alloc_error);
  }

  if (flt.scale == CV1D_NO_SCALE) {
    for (i = filter_begin_index; i <= filter_end_index; i++) {
      filter_data[i - filter_begin_index] =
	evaluator_evaluate_x(flt.d_real_ptr,(double)(i));
    }
  } else {
    for (i = filter_begin_index; i <= filter_end_index; i++) {
      filter_data[i - filter_begin_index] =
	evaluator_evaluate_x(flt.d_real_ptr,(double)(i)*flt.scale);
    }
  }

  /*
   * Compute the sum of the squared modulus of all the filter values.
   */

  flt_squared_mod_sum_ = 0.0;
  for (i = filter_begin_index; i <= filter_end_index; i++) {
    flt_squared_mod_sum_ +=
      filter_data[i-filter_begin_index]*filter_data[i-filter_begin_index];
  }

  /*  old_flt.def = flt.def;
  old_flt.d_begin = flt.d_begin;
  old_flt.d_end = flt.d_end;
  old_flt.d_data = flt.d_data;
  flt.def = NUMERICAL;
  flt.d_begin = (real) filter_begin_index;
  flt.d_end = (real) filter_end_index;
  flt.d_data = (void *) filter_data;*/
  cv1d_flt_copy_ (&flt, &old_flt);
  cv1d_flt_init_n (flt.form,
		   filter_end_index - filter_begin_index + 1,
		   -filter_begin_index,
		   0, 0,
		   filter_data,
		   0);

  result_data = cv1d_n_real_d (border_effect, result_data,
			       first_exact_ptr, last_exact_ptr);

  cv1d_flt_copy_ (&old_flt, &flt);

  free (filter_data);

  /*  flt.def = old_flt.def;
  flt.d_begin = old_flt.d_begin;
  flt.d_end = old_flt.d_end;
  flt.d_data = old_flt.d_data;*/

  cv1d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return result_data;

mem_alloc_error:
  free (filter_data);

  return 0;
}


/*
 * cv1d_a_real_mp --
 */

void *
cv1d_a_real_mp (int  border_effect,
		void *res_data,
		int  *first_exact_ptr,
		int  *last_exact_ptr)
{
  real   *signal_data;
  real   *result_data;

  real    *signal_part = 0;
  complex *signal_part_ft = 0;
  complex *filter_ft = 0;

  int     nb_of_parts, part_nb, part_n;
  int     size_of_exact_data;
  int     i;
  int     flt_f_begin_index;
  int     flt_f_end_index;
  int     flt_d_end_index;
  real    f_step;

  assert (flt.def == ANALYTICAL);
  assert (flt.form != CV1D_UNDEFINED);
  assert (sig_form != CV1D_UNDEFINED);
  assert (sig_n >= flt.d_n);

  assert ((border_effect == CV1D_PERIODIC)
	  || (border_effect == CV1D_MIRROR)
	  || (border_effect == CV1D_PADDING)
	  || (border_effect == CV1D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (real *) sig_d_data;
  result_data = (real *) res_data;

  part_n = cv1d_next_good_fft_size (2*flt.d_n);
  size_of_exact_data = part_n - flt.d_n + 1;

  f_step = 2*M_PI/(part_n);

  nb_of_parts = ceil (((double) sig_n)/size_of_exact_data);

  assert (nb_of_parts >= 1);

  if (flt.scale == CV1D_NO_SCALE) {
    flt_f_begin_index = (int) floor (flt.d_begin/f_step) ;
    flt_f_end_index = (int) ceil (flt.d_end/f_step) ;
    flt_d_end_index = (int) ceil (flt.d_end) ;
  } else {
    flt_f_begin_index = (int) floor (flt.f_begin/flt.scale/f_step);
    flt_f_end_index   = (int) ceil  (flt.f_end/flt.scale/f_step);
    flt_d_end_index = (int) ceil (flt.d_end*flt.scale) ;
  }

  signal_part = (real *) malloc (sizeof (real)*part_n);
  if (!signal_part) {
    EX_RAISE(mem_alloc_error);
  }
  signal_part_ft = (complex *) malloc (sizeof (complex)*part_n/2);
  if (!signal_part_ft) {
    EX_RAISE(mem_alloc_error);
  }
  filter_ft = (complex *) malloc (sizeof (complex)*part_n/2);
  if (!filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  if (flt.scale == CV1D_NO_SCALE) {
    filter_ft[0].real = evaluator_evaluate_x(flt.f_real_ptr, 0.0);
    filter_ft[0].imag =
      evaluator_evaluate_x(flt.f_real_ptr,(double)(part_n*f_step/2));
    if (flt.f_imag_ptr) {
      for (i = 1; i < part_n/2; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)(i*f_step));
	filter_ft[i].imag =
	  evaluator_evaluate_x(flt.f_imag_ptr,(double)(i*f_step));
      }
    } else {
      for (i = 1; i < part_n/2; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)(i*f_step));
	filter_ft[i].imag = 0.0;
      }    
    }
  } else { /* flt.scale != CV1D_NO_SCALE */
    filter_ft[0].real = evaluator_evaluate_x(flt.f_real_ptr,0.0*flt.scale);
    filter_ft[0].imag =
      evaluator_evaluate_x(flt.f_real_ptr,(double)(part_n*f_step/2)*flt.scale);
    if (flt.f_imag_ptr) {
      for (i = 1; i < part_n/2; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)(i*f_step)*flt.scale);
	filter_ft[i].imag =
	  evaluator_evaluate_x(flt.f_imag_ptr,(double)(i*f_step)*flt.scale);
      }
    } else {
      for (i = 1; i < part_n/2; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)(i*f_step)*flt.scale);
	filter_ft[i].imag = 0.0;
      }    
    }
  }

  /*
   * Compute the sum of the squared modulus of all the filter values.
   */

  flt_squared_mod_sum_ = 0.0;
  for (i = 1; i < part_n/2; i++) {
    flt_squared_mod_sum_ +=
      filter_ft[i].real*filter_ft[i].real + filter_ft[i].imag*filter_ft[i].imag;
  }
  flt_squared_mod_sum_ *= 2.0;
  flt_squared_mod_sum_ +=
    filter_ft[0].real*filter_ft[0].real + filter_ft[0].imag*filter_ft[0].imag;
  flt_squared_mod_sum_ = flt_squared_mod_sum_/part_n;

  for (part_nb = 0; part_nb < nb_of_parts; part_nb++) {
    int part_begin_in_signal;

    part_begin_in_signal = part_nb*size_of_exact_data - flt_d_end_index;
    cv1d_get_part_r_ (signal_part, part_n,
		      signal_data, sig_n,
		      part_begin_in_signal,
		      border_effect);


    cv1d_fft_r (signal_part, signal_part_ft, part_n);

    signal_part_ft[0].real *= filter_ft[0].real;
    signal_part_ft[0].imag *= filter_ft[0].imag;
    cv1d_cplx_mult_ (signal_part_ft, filter_ft,
		     1, part_n/2 - 1);

    cv1d_fft_r_i (signal_part_ft, signal_part, part_n);

    if (part_nb < (nb_of_parts - 1)) {
      memcpy (result_data + part_nb*size_of_exact_data,
	      signal_part + flt_d_end_index,
	      size_of_exact_data*sizeof (real));
    } else {
      memcpy (result_data + part_nb*size_of_exact_data,
	      signal_part + flt_d_end_index,
	      (sig_n - part_nb*(size_of_exact_data))*sizeof (real));
    }
  }

  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);
  
  cv1d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);

  return 0;
}


/*
 * cv1d_a_real_ft --
 */

void *
cv1d_a_real_ft (int  border_effect,
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
  
  assert (flt.def == ANALYTICAL);
  assert (flt.form != CV1D_UNDEFINED);
  assert (sig_form != CV1D_UNDEFINED);
  assert (sig_n >= flt.d_n);

  assert ((border_effect == CV1D_PERIODIC)
	  || (border_effect == CV1D_MIRROR)
	  || (border_effect == CV1D_PADDING)
	  || (border_effect == CV1D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);
  assert (cv1d_is_good_fft_size (sig_n));

  signal_data = (real *) sig_d_data;
  result_data = (real *) res_data;

  switch (border_effect) {
  case CV1D_0_PADDING:
    new_size = sig_n*2;
    break;
  case CV1D_PADDING:
    new_size = sig_n*2;
    break;
  case CV1D_MIRROR:
    new_size = sig_n*2;
    break;
  case CV1D_PERIODIC:
    new_size = sig_n;
    break;
  }
  f_step = 2*M_PI/(new_size);

  if (flt.scale == CV1D_NO_SCALE) {
    flt_f_begin_index = (int) floor (flt.f_begin) ;
    flt_f_end_index = (int) ceil (flt.f_end) ;
    flt_d_begin_index = (int) floor (flt.d_begin) ;
    flt_d_end_index = (int) ceil (flt.d_end) ;
  } else {
    flt_f_begin_index = (int) floor (flt.f_begin/flt.scale/f_step);
    flt_f_end_index   = (int) ceil  (flt.f_end/flt.scale/f_step);
    flt_d_begin_index = (int) floor (flt.d_begin*flt.scale);
    flt_d_end_index   = (int) ceil  (flt.d_end*flt.scale);
  }

  assert (new_size >= (2*max(flt_f_end_index, - flt_f_begin_index) + 1));

  switch (border_effect) {
  case CV1D_0_PADDING:
    new_signal = cv1d_0_padding_transform_ (signal_data,
					    sig_n,
					    flt_d_end_index);
    break;
  case CV1D_PADDING:
    new_signal = cv1d_padding_transform_ (signal_data,
					  sig_n,
					  flt_d_end_index);
    break;
  case CV1D_MIRROR:
    new_signal = cv1d_mirror_transform_ (signal_data,
					 sig_n,
					 flt_d_end_index);
    break;
  case CV1D_PERIODIC:
    new_signal = signal_data;
    break;
  }
  if (!new_signal) {
    EX_RAISE(mem_alloc_error);
  }

  new_signal_ft = (complex *) malloc (sizeof (complex)*new_size/2);
  if (!new_signal_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv1d_fft_r (new_signal, new_signal_ft, new_size);

  flt_squared_mod_sum_ = 0.0;

  if (flt.scale == CV1D_NO_SCALE) {
    flt_f_real_0 = evaluator_evaluate_x(flt.f_real_ptr,0.0);
    flt_f_imag_0 = evaluator_evaluate_x(flt.f_real_ptr,(double)new_size/2*f_step);
    new_signal_ft[0].real *= flt_f_real_0;
    new_signal_ft[0].imag *= flt_f_imag_0;
    cv1d_cplx_mult_num_ana_ (new_signal_ft, flt.f_real_ptr, flt.f_imag_ptr,
			     1, new_size/2 - 1, f_step, 0.0);
    flt_squared_mod_sum_ *= 2;
    flt_squared_mod_sum_ +=
      flt_f_real_0*flt_f_real_0 + flt_f_imag_0*flt_f_imag_0;
  } else {
    flt_f_real_0 = flt.scale*evaluator_evaluate_x(flt.f_real_ptr,0.0*flt.scale);
    flt_f_imag_0 =  flt.scale
      * evaluator_evaluate_x(flt.f_real_ptr, (double)new_size/2*f_step*flt.scale);
    new_signal_ft[0].real *= flt_f_real_0;
    new_signal_ft[0].imag *= flt_f_imag_0;
    cv1d_mult_with_scaled_ft_fct_ (new_signal_ft, flt.f_real_ptr, flt.f_imag_ptr,
				   flt.scale, 1, new_size/2 - 1, f_step, 0.0);
    flt_squared_mod_sum_ *= 2;
    flt_squared_mod_sum_ +=
      flt_f_real_0*flt_f_real_0 + flt_f_imag_0*flt_f_imag_0;
  }

  flt_squared_mod_sum_ = flt_squared_mod_sum_/new_size;
  
  if (border_effect == CV1D_PERIODIC) {
    cv1d_fft_r_i (new_signal_ft, result_data, new_size);
  } else {
    cv1d_fft_r_i (new_signal_ft, new_signal, new_size);
    memcpy (result_data, new_signal, sig_n*sizeof (real));
    free (new_signal);
  }

  free (new_signal_ft);
  
  cv1d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);
  return res_data;

mem_alloc_error:
  free (new_signal_ft);
  free (new_signal);


  return 0;
}


/*
 * cv1d_a_cplx --
 */

void *
cv1d_a_cplx (int  border_effect,
	     void *res_data,
	     int  *first_exact_ptr,
	     int  *last_exact_ptr)
{
  int method;
  real * ret_value = 0;
  void * (*the_cv1d_fct_ptr)();

  LogMessage("a c ");
  if (cv1d_method == CV1D_UNDEFINED) {
    method =
      cv1d_convolution_method_ (sig_n, flt.d_n, lim_array[ANALYTICAL][border_effect]);
  } else {
    method = cv1d_method;
  }
  if ((method == DIRECT_CONVOLUTION) && (!flt.d_real_ptr && !flt.d_imag_ptr)) {
    method = MULTI_PART_CONVOLUTION;
  }
  if ((method == FOURIER_TRANSFORM_CONVOLUTION)
      && !cv1d_is_good_fft_size (sig_n)) {
    method = MULTI_PART_CONVOLUTION;
  }

#ifdef LOG_MESSAGES
  LogMessage2("%s ", method_str[method]);
  LogMessage2("%d ", sig_n);
  LogMessage2("%d ", flt.d_n);
#endif
  the_cv1d_fct_ptr = cv1d_a_fct_ptr_array[CPLX][method];
  SetLogTimeBegin();
  ret_value = 
    the_cv1d_fct_ptr (border_effect, res_data, first_exact_ptr, last_exact_ptr);
  LogTime();
  return ret_value;
}


/*
 * cv1d_a_cplx_d --
 */
void *
cv1d_a_cplx_d (int  border_effect,
	       void *res_data,
	       int  *first_exact_ptr,
	       int  *last_exact_ptr)
{
  complex *signal_data;
  complex *result_data;

  int i;
  int j;

  complex *filter_data = 0;

  int  filter_begin_index;
  int  filter_end_index;

  _filter_ old_flt;

  assert (flt.def == ANALYTICAL);
  assert (flt.form != CV1D_UNDEFINED);
  assert (sig_form != CV1D_UNDEFINED);
  assert (sig_n >= flt.d_n);

  assert ((border_effect == CV1D_PERIODIC)
	  || (border_effect == CV1D_MIRROR)
	  || (border_effect == CV1D_PADDING)
	  || (border_effect == CV1D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (complex *) sig_d_data;
  result_data = (complex *) res_data;

  if (flt.scale == CV1D_NO_SCALE) {
    filter_begin_index = (int) ceil (flt.d_begin) ;
    filter_end_index = (int) ceil (flt.d_end) ;
  } else {
    filter_begin_index = (int) ceil (flt.d_begin*flt.scale) ;
    filter_end_index = (int) ceil (flt.d_end*flt.scale) ;
  }


  filter_data = (complex *) malloc (sizeof(complex)*flt.d_n);
  if (!filter_data) {
    EX_RAISE(mem_alloc_error);
  }

  if (flt.scale == CV1D_NO_SCALE) {
    for (i = filter_begin_index; i <= filter_end_index; i++) {
      filter_data[i - filter_begin_index].real =
	evaluator_evaluate_x(flt.d_real_ptr,(double)(i));
      filter_data[i - filter_begin_index].imag =
	evaluator_evaluate_x(flt.d_imag_ptr,(double)(i));
    }
  } else {
    for (i = filter_begin_index; i <= filter_end_index; i++) {
      filter_data[i - filter_begin_index].real =
	evaluator_evaluate_x(flt.d_real_ptr,(double)(i)*flt.scale);
      filter_data[i - filter_begin_index].imag =
	evaluator_evaluate_x(flt.d_imag_ptr,(double)(i)*flt.scale);
    }
  }

  /*
   * Compute the sum of the squared modulus of all the filter values.
   */

  flt_squared_mod_sum_ = 0.0;
  for (i = filter_begin_index; i <= filter_end_index; i++) {
    j = i - filter_begin_index;
    flt_squared_mod_sum_ +=
      filter_data[j].real*filter_data[j].real
      + filter_data[j].imag*filter_data[j].imag;
  }

  /*  old_flt.def = flt.def;
  old_flt.d_begin = flt.d_begin;
  old_flt.d_end = flt.d_end;
  old_flt.d_data = flt.d_data;
  flt.def = NUMERICAL;
  flt.d_begin = (real) filter_begin_index;
  flt.d_end = (real) filter_end_index;
  flt.d_data = (void *) filter_data;
  */
  cv1d_flt_copy_ (&flt, &old_flt);
  cv1d_flt_init_n (flt.form,
		   filter_end_index - filter_begin_index + 1,
		   -filter_begin_index,
		   0, 0,
		   filter_data,
		   0);

  result_data = cv1d_n_cplx_d (border_effect, result_data,
			       first_exact_ptr, last_exact_ptr);

  cv1d_flt_copy_ (&old_flt, &flt);

  free (filter_data);

  /*  flt.def = old_flt.def;
  flt.d_begin = old_flt.d_begin;
  flt.d_end = old_flt.d_end;
  flt.d_data = old_flt.d_data;*/

  cv1d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (filter_data);

  return 0;
}


/*
 * cv1d_a_cplx_mp --
 */

void *
cv1d_a_cplx_mp (int  border_effect,
		void *res_data,
		int  *first_exact_ptr,
		int  *last_exact_ptr)
{
  complex *signal_data;
  complex *result_data;

  complex *signal_part = 0;
  complex *signal_part_ft = 0 ;
  complex *filter_ft = 0;

  int     nb_of_parts, part_nb, part_n;
  int     size_of_exact_data;
  int     i;
  int     flt_f_begin_index;
  int     flt_f_end_index;
  int     flt_d_end_index;
  real    f_step;

  assert (flt.def == ANALYTICAL);
  assert (flt.form != CV1D_UNDEFINED);
  assert (sig_form != CV1D_UNDEFINED);
  assert (sig_n >= flt.d_n);

  assert ((border_effect == CV1D_PERIODIC)
	  || (border_effect == CV1D_MIRROR)
	  || (border_effect == CV1D_PADDING)
	  || (border_effect == CV1D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (complex *) sig_d_data;
  result_data = (complex *) res_data;

  part_n = cv1d_next_good_fft_size (2*flt.d_n);
  size_of_exact_data = part_n - flt.d_n + 1;

  nb_of_parts = ceil (((double) sig_n)/size_of_exact_data);
  assert (nb_of_parts >= 1);

  f_step = 2*M_PI/(part_n);

  if (flt.scale == CV1D_NO_SCALE) {
    flt_f_begin_index = (int) floor (flt.f_begin/f_step) ;
    flt_f_end_index = (int) ceil (flt.f_end/f_step) ;
    flt_d_end_index = (int) ceil (flt.d_end) ;
  } else {
    flt_f_begin_index = (int) floor (flt.f_begin/flt.scale/f_step);
    flt_f_end_index   = (int) ceil  (flt.f_end/flt.scale/f_step);
    flt_d_end_index   = (int) ceil  (flt.d_end*flt.scale);
  }

  signal_part = (complex *) malloc (sizeof (complex)*part_n);
  if (!signal_part) {
    EX_RAISE(mem_alloc_error);
  }
  signal_part_ft = (complex *) malloc (sizeof (complex)*part_n);
  if (!signal_part_ft) {
    EX_RAISE(mem_alloc_error);
  }
  filter_ft = (complex *) malloc (sizeof (complex)*part_n);
  if (!filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  /* Fill filter fourier form array. */

  if (flt.scale == CV1D_NO_SCALE) {
    if (flt.f_imag_ptr) {
      for (i = 0; i <= flt_f_end_index; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)(i*f_step));
	filter_ft[i].imag =
	  evaluator_evaluate_x(flt.f_imag_ptr,(double)(i*f_step));
      }
      for (; i < part_n+flt_f_begin_index; i++) {
	filter_ft[i].real = 0.0;
	filter_ft[i].imag = 0.0;
      }
      for (; i < part_n; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)((i-flt_f_end_index)*f_step));
	filter_ft[i].imag =
	  evaluator_evaluate_x(flt.f_imag_ptr,(double)((i-flt_f_end_index)*f_step));
      }
    } else { /* No function pointer for the imaginary part. */
      for (i = 0; i <= flt_f_end_index; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)(i*f_step));
	filter_ft[i].imag = 0.0;
      }    
      for (; i < part_n+flt_f_begin_index; i++) {
	filter_ft[i].real = 0.0;
	filter_ft[i].imag = 0.0;
      }
      for (; i < part_n; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)((i-flt_f_end_index)*f_step));
	filter_ft[i].imag = 0.0;
      }
    }
  } else { /* flt.scale != CV1D_NO_SCALE */
    if (flt.f_imag_ptr) {
      for (i = 0; i <= flt_f_end_index; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)(i*f_step)*flt.scale);
	filter_ft[i].imag =
	  evaluator_evaluate_x(flt.f_imag_ptr,(double)(i*f_step)*flt.scale);
      }
      for (; i < part_n+flt_f_begin_index; i++) {
	filter_ft[i].real = 0.0;
	filter_ft[i].imag = 0.0;
      }
      for (; i < part_n; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)((i-flt_f_end_index)*f_step)*flt.scale);
	filter_ft[i].imag =
	  evaluator_evaluate_x(flt.f_imag_ptr,(double)((i-flt_f_end_index)*f_step)*flt.scale);
      }
    } else { /* No function pointer for the imaginary part. */
      for (i = 0; i <= flt_f_end_index; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)(i*f_step)*flt.scale);
	filter_ft[i].imag = 0.0;
      }    
      for (; i < part_n+flt_f_begin_index; i++) {
	filter_ft[i].real = 0.0;
	filter_ft[i].imag = 0.0;
      }
      for (; i < part_n; i++) {
	filter_ft[i].real =
	  evaluator_evaluate_x(flt.f_real_ptr,(double)((i-flt_f_end_index)*f_step)*flt.scale);
	filter_ft[i].imag = 0.0;
      }
    }
  }

  /*
   * Compute the sum of the squared modulus of all the filter values.
   */

  flt_squared_mod_sum_ = 0.0;
  for (i = 0; i < part_n; i++) {
    flt_squared_mod_sum_ +=
      filter_ft[i].real*filter_ft[i].real
      + filter_ft[i].imag*filter_ft[i].imag;
  }
  flt_squared_mod_sum_ = flt_squared_mod_sum_/part_n;

  for (part_nb = 0; part_nb < nb_of_parts; part_nb++) {
    int part_begin_in_signal;

    part_begin_in_signal = part_nb*size_of_exact_data - flt_d_end_index;
    cv1d_get_part_c_ (signal_part, part_n,
		      signal_data, sig_n,
		      part_begin_in_signal,
		      border_effect);

    cv1d_fft_c (signal_part, signal_part_ft, part_n);

    cv1d_cplx_mult_ (signal_part_ft, filter_ft,
		     0, part_n - 1);

    cv1d_fft_c_i (signal_part_ft, signal_part, part_n);

    if (part_nb < (nb_of_parts - 1)) {
      memcpy (result_data + part_nb*size_of_exact_data,
	      signal_part + flt_d_end_index,
	      size_of_exact_data*sizeof (complex));
    } else {
      memcpy (result_data + part_nb*size_of_exact_data,
	      signal_part + flt_d_end_index,
	      (sig_n - part_nb*(size_of_exact_data))*sizeof (complex));
    }
  }

  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);
  
  cv1d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);

  return 0;
}


/*
 * cv1d_a_cplx_ft --
 */

void *
cv1d_a_cplx_ft (int  border_effect,
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
  
  assert (flt.def == ANALYTICAL);
  assert (flt.form != CV1D_UNDEFINED);
  assert (sig_form != CV1D_UNDEFINED);
  assert (sig_n >= flt.d_n);

  assert ((border_effect == CV1D_PERIODIC)
	  || (border_effect == CV1D_MIRROR)
	  || (border_effect == CV1D_PADDING)
	  || (border_effect == CV1D_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (complex *) sig_d_data;
  result_data = (complex *) res_data;

  switch (border_effect) {
  case CV1D_0_PADDING:
    new_size = sig_n*2;
    break;
  case CV1D_PADDING:
    new_size = sig_n*2;
    break;
  case CV1D_MIRROR:
    new_size = sig_n*2;
    break;
  case CV1D_PERIODIC:
    new_size = sig_n;
    break;
  }
  f_step = 2*M_PI/(new_size);

  if (flt.scale == CV1D_NO_SCALE) {
    flt_f_begin_index = (int) floor (flt.f_begin) ;
    flt_f_end_index = (int) ceil (flt.f_end) ;
    /*    flt_d_begin_index = (int) floor (flt.d_begin) ;
	  flt_d_end_index = (int) ceil (flt.d_end) ;*/
  } else {
    flt_f_begin_index = (int) floor (flt.f_begin/flt.scale/f_step);
    flt_f_end_index   = (int) ceil  (flt.f_end/flt.scale/f_step);
    /*    flt_d_begin_index = (int) floor (flt.d_begin*flt.scale);
	  flt_d_end_index   = (int) ceil  (flt.d_end*flt.scale);*/
  }

  assert (new_size >= (2*max(flt_f_end_index, - flt_f_begin_index) + 1));
  /*  assert (new_size >= (flt_f_end_index - flt_f_begin_index +1));*/

  filter_ft_size = 2*max(flt_f_end_index, - flt_f_begin_index) + 1;
  /*  if (flt.scale == CV1D_NO_SCALE) {
      flt_f_begin_index = (int) ceil (flt.d_begin/f_step) ;
      flt_f_end_index = (int) ceil (flt.d_end/f_step) ;
      } else {
      flt_f_begin_index = (int) floor (flt.f_begin/flt.scale/f_step);
      flt_f_end_index   = (int) ceil  (flt.f_end/flt.scale/f_step);
      }

      filter_ft_size = flt_f_end_index - flt_f_begin_index + 1;
      assert (sig_n >= filter_ft_size);*/

  switch (border_effect)
    {
    case CV1D_0_PADDING:
      new_signal = cv1d_cplx_0_padding_transform_ (signal_data,
						   sig_n,
						   flt_f_end_index);
      break;
    case CV1D_PADDING:
      new_signal = cv1d_cplx_padding_transform_ (signal_data,
						 sig_n,
						 flt_f_end_index);
      break;
    case CV1D_MIRROR:
      new_signal = cv1d_cplx_mirror_transform_ (signal_data,
						sig_n,
						flt_f_end_index);
      break;
    case CV1D_PERIODIC:
      new_signal = signal_data;
      break;
    }
  if (!new_signal) {
    EX_RAISE(mem_alloc_error);
  }

  new_signal_ft = (complex *) malloc (sizeof (complex)*new_size);
  if (!new_signal_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv1d_fft_c (new_signal, new_signal_ft, new_size);

  flt_squared_mod_sum_ = 0.0;

  if (flt.scale == CV1D_NO_SCALE) {
    tmp_index = flt_f_end_index + (new_size - filter_ft_size)/2;
    cv1d_cplx_mult_num_ana_ (new_signal_ft, flt.f_real_ptr, flt.f_imag_ptr,
			     0, tmp_index - 1, f_step,
			     0.0);
    cv1d_cplx_mult_num_ana_ (new_signal_ft, flt.f_real_ptr, flt.f_imag_ptr,
			     tmp_index, new_size - 1, f_step,
			     (real)((- new_size)*f_step));
  } else {
    tmp_index = flt_f_end_index + (new_size - filter_ft_size)/2;
    cv1d_mult_with_scaled_ft_fct_ (new_signal_ft, flt.f_real_ptr, flt.f_imag_ptr,
				   flt.scale, 0, tmp_index - 1, f_step,
				   0.0);
    cv1d_mult_with_scaled_ft_fct_ (new_signal_ft, flt.f_real_ptr, flt.f_imag_ptr,
				   flt.scale, tmp_index, new_size - 1, f_step,
				   (real)((- new_size)*f_step));
  }

  flt_squared_mod_sum_ = flt_squared_mod_sum_/new_size;

  if (border_effect == CV1D_PERIODIC) {
    cv1d_fft_c_i (new_signal_ft, result_data, new_size);
  } else {
    cv1d_fft_c_i (new_signal_ft, new_signal, new_size);
    memcpy (result_data, new_signal, sig_n*sizeof (complex));
    free (new_signal);
  }

  free (new_signal_ft);
  
  cv1d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (new_signal);
  free (new_signal_ft);

  return 0;
}

