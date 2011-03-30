/*
 * wt1d.c --
 *
 * Main file for 1D wavelet transform functions.
 *
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *   $Id: wt1d.c,v 1.10 1998/11/24 15:12:04 decoster Exp $
 */

#include "wt1d_int.h"

/*
 * wt1d_real_all --
 */
real **
wt1d_real_all (real    *d_data,
	       int     size_of_data,
	       Wavelet *wavelet,
	       real    first_scale,
	       int     nb_of_octave,
	       int     nb_of_vox,
	       real    exponent,
	       real    **d_result,
	       int     border_effect,
	       int     *first_exact_ptr,
	       int     *last_exact_ptr)
{
  int  octave;
  int  vox;
  real scale;
  int  i = 0;
  int  first_exact;
  int  last_exact;

  assert (d_data);
  assert (size_of_data > 0);
  assert (wavelet);
  assert (nb_of_octave > 0);
  assert (nb_of_vox > 0);
  assert (d_result);
  assert ((border_effect == CV1D_PERIODIC)
	  || (border_effect == CV1D_MIRROR)
	  || (border_effect == CV1D_PADDING)
	  || (border_effect == CV1D_0_PADDING));

  *first_exact_ptr = 0;
  *last_exact_ptr = size_of_data;

  for (octave = 0; octave < nb_of_octave; octave++) {
    for (vox = 0; vox < nb_of_vox; vox++) {
      scale = first_scale*pow (2, octave+(double)vox/nb_of_vox);
      LogMessage2("(%d) ", octave*nb_of_vox+vox);
      d_result[i] = wt1d_real_single (d_data,
				      size_of_data,
				      wavelet,
				      scale,
				      exponent,
				      d_result[i],
				      border_effect,
				      &first_exact,
				      &last_exact);
      if (!d_result[i]) {
	return 0;
      }
      if (first_exact > *first_exact_ptr) {
	*first_exact_ptr = first_exact;
      }
      if (last_exact < *last_exact_ptr) {
	*last_exact_ptr = last_exact;
      }
      i++;
    }
  }

  return d_result;
}


/*
 * wt1d_real_single --
 */
real *
wt1d_real_single (real    *d_data,
		  int     size_of_data,
		  Wavelet *wavelet,
		  real    scale,
		  real    exponent,
		  real    *d_result,
		  int     border_effect,
		  int     *first_exact_ptr,
		  int     *last_exact_ptr)
{
  real *wv_d_data;
  int  tab_size;
  int  d_begin = 0;
  int  d_end = 0;
  int  f_begin = 0;
  int  f_end = 0;
  int  flt_form = -1;

  int i;
  double mult = 0.0;
  double sum = 0.0;

  assert (d_data);
  assert (size_of_data > 0);
  assert (wavelet);
  assert (scale > 0);
  assert (d_result);
  assert ((border_effect == CV1D_PERIODIC)
	  || (border_effect == CV1D_MIRROR)
	  || (border_effect == CV1D_PADDING)
	  || (border_effect == CV1D_0_PADDING));

  LogMessage2("%f ", scale);

  switch (wavelet->type) {
  case WAVE_REAL_REAL:
    flt_form = CV1D_RR_FORM;
    break;
  case WAVE_REAL_IMAG:
  case WAVE_REAL_CPLX:
    flt_form = CV1D_RC_FORM;
    break;
  case WAVE_IMAG_REAL:
  case WAVE_IMAG_IMAG:
  case WAVE_IMAG_CPLX:
  case WAVE_CPLX_REAL:
  case WAVE_CPLX_IMAG:
  case WAVE_CPLX_CPLX:
    flt_form = CV1D_CC_FORM;
    break;
  }

  wv_d_data = get_tab_data (wavelet->d_tab,
			    &tab_size,
			    scale);
  if (wv_d_data) {
    d_begin = (int) floor (wavelet->d_x_min*scale);
    d_end   = (int) ceil (wavelet->d_x_max*scale);
    f_begin = (int) floor (wavelet->f_x_min/scale);
    f_end   = (int) ceil (wavelet->f_x_max/scale);
    cv1d_flt_init_n (flt_form,
		     d_end - d_begin + 1, -d_begin,
		     f_end - f_begin + 1, -f_begin,
		     wv_d_data, 0);
  } else {
    cv1d_flt_init_a (wavelet->d_x_min, wavelet->d_x_max,
		     wavelet->f_x_min, wavelet->f_x_max,
		     wavelet->d_r_fct_ptr, wavelet->d_i_fct_ptr,
		     wavelet->f_r_fct_ptr, wavelet->f_i_fct_ptr,
		     scale);
  }
  cv1d_sig_init (CV1D_RC_FORM, d_data, 0, size_of_data);

  /*
   * To be sure that there is no method set.
   */

  cv1d_set_method (CV1D_UNDEFINED);

  d_result = (real *) cv1d_compute (border_effect,
				    d_result,
				    first_exact_ptr,
				    last_exact_ptr);

  /*
   * Norm convention :
   *                            1                     t
   *   psi_a (t) = --------------------------- * psi(---),
   *                 p            1                   a
   *                a   * sqrt(------- * sum)
   *                           sqrt(a)
   *
   *  with                |     t  |2
   *       sum = integral |psi(---)|  dt.
   *                      |     a  |
   *
   *   a   : the scale.
   *   psi : the wavelet.
   *   p   : the normalisation exponent.
   */

  if (wv_d_data) {
    int n = d_end - d_begin + 1;

    if (flt_form == CV1D_CC_FORM) {
      /*
       * Just to blablabla complex blablabla in the next loop...
       */

      n *= 2;
    }
    sum = 0;
    for (i = 0; i < n; i++) {
      sum += wv_d_data[i]*wv_d_data[i];
    }
  } else {
    sum = cv1d_get_flt_squared_mod_sum ();
  }
  mult = 1/(pow(scale, exponent - 0.5)*sqrt(sum));

  if (flt_form == CV1D_CC_FORM) {
    /*
     * The result is complex. So the array is 2 times bigger.
     */
    
    for (i = 0; i < 2*size_of_data; i++) {
      d_result[i] *= mult;
    }
  } else {
    /*
     * The result is real.
     */

    for (i = 0; i < size_of_data; i++) {
      d_result[i] *= mult;
    }
  }

  return d_result;
}


/*
 * wt1d_cplx_all --
 */
real **
wt1d_cplx_all (complex *d_data,
	       int     size_of_data,
	       Wavelet *wavelet,
	       real    first_scale,
	       int     nb_of_octave,
	       int     nb_of_vox,
	       real    exponent,
	       real    **d_result,
	       int     border_effect,
	       int     *first_exact_ptr,
	       int     *last_exact_ptr)
{
  int  octave;
  int  vox;
  real scale;
  int  i = 0;
  int  first_exact;
  int  last_exact;

  assert (d_data);
  assert (size_of_data > 0);
  assert (wavelet);
  assert (nb_of_octave > 0);
  assert (nb_of_vox > 0);
  assert (d_result);
  assert ((border_effect == CV1D_PERIODIC)
	  || (border_effect == CV1D_MIRROR)
	  || (border_effect == CV1D_PADDING)
	  || (border_effect == CV1D_0_PADDING));

  *first_exact_ptr = 0;
  *last_exact_ptr = size_of_data;

  for (octave = 0; octave < nb_of_octave; octave++) {
    for (vox = 0; vox < nb_of_vox; vox++) {
      scale = first_scale*pow (2, octave+(double)vox/nb_of_vox);
      LogMessage2("(%d) ", octave*nb_of_vox+vox);
      d_result[i] = wt1d_cplx_single (d_data,
				      size_of_data,
				      wavelet,
				      scale,
				      exponent,
				      d_result[i],
				      border_effect,
				      &first_exact,
				      &last_exact);
      if (!d_result[i]) {
	return 0;
      }
      if (first_exact > *first_exact_ptr) {
	*first_exact_ptr = first_exact;
      }
      if (last_exact < *last_exact_ptr) {
	*last_exact_ptr = last_exact;
      }
      i++;
    }
  }

  return d_result;
}


/*
 * wt1d_cplx_single --
 */
real *
wt1d_cplx_single (complex *d_data,
		  int     size_of_data,
		  Wavelet *wavelet,
		  real    scale,
		  real    exponent,
		  real    *d_result,
		  int     border_effect,
		  int     *first_exact_ptr,
		  int     *last_exact_ptr)
{
  real *wv_d_data;
  int  tab_size;
  int  d_begin = 0;
  int  d_end = 0;
  int  f_begin = 0;
  int  f_end = 0;
  int  flt_form = -1;

  int i;
  real mult = 0.0;
  real sum = 0.0;

  assert (d_data);
  assert (size_of_data > 0);
  assert (wavelet);
  assert (scale > 0);
  assert (d_result);
  assert ((border_effect == CV1D_PERIODIC)
	  || (border_effect == CV1D_MIRROR)
	  || (border_effect == CV1D_PADDING)
	  || (border_effect == CV1D_0_PADDING));

  LogMessage2("%f ", scale);

  switch (wavelet->type) {
  case WAVE_REAL_REAL:
    flt_form = CV1D_RR_FORM;
    break;
  case WAVE_REAL_IMAG:
  case WAVE_REAL_CPLX:
    flt_form = CV1D_RC_FORM;
    break;
  case WAVE_IMAG_REAL:
  case WAVE_IMAG_IMAG:
  case WAVE_IMAG_CPLX:
  case WAVE_CPLX_REAL:
  case WAVE_CPLX_IMAG:
  case WAVE_CPLX_CPLX:
    flt_form = CV1D_CC_FORM;
    break;
  }

  wv_d_data = get_tab_data (wavelet->d_tab,
			    &tab_size,
			    scale);
  if (wv_d_data) {
    d_begin = (int) floor (wavelet->d_x_min*scale);
    d_end   = (int) ceil (wavelet->d_x_max*scale);
    f_begin = (int) floor (wavelet->f_x_min/scale);
    f_end   = (int) ceil (wavelet->f_x_max/scale);
    cv1d_flt_init_n (flt_form,
		     d_end - d_begin + 1, -d_begin,
		     f_end - f_begin + 1, -f_begin,
		     wv_d_data, 0);
  } else {
    cv1d_flt_init_a (wavelet->d_x_min, wavelet->d_x_max,
		     wavelet->f_x_min, wavelet->f_x_max,
		     wavelet->d_r_fct_ptr, wavelet->d_i_fct_ptr,
		     wavelet->f_r_fct_ptr, wavelet->f_i_fct_ptr,
		     scale);
  }
  cv1d_sig_init (CV1D_CC_FORM, d_data, 0, size_of_data);

  /*
   * To be sure that there is no method set.
   */

  cv1d_set_method (CV1D_UNDEFINED);

  d_result = (real *) cv1d_compute (border_effect,
				    d_result,
				    first_exact_ptr,
				    last_exact_ptr);

  /*
   * Norm convention :
   *                            1                     t
   *   psi_a (t) = --------------------------- * psi(---),
   *                 p            1                   a
   *                a   * sqrt(------- * sum)
   *                           sqrt(a)
   *
   *  with                |     t  |2
   *       sum = integral |psi(---)|  dt.
   *                      |     a  |
   *
   *   a   : the scale.
   *   psi : the wavelet.
   *   p   : the normalisation exponent.
   */

  if (wv_d_data) {
    int n = d_end - d_begin + 1;

    if (flt_form == CV1D_CC_FORM) {
      /*
       * Just to blablabla complex blablabla in the next loop...
       */
      n *= 2;
    }
    sum = 0;
    for (i = 0; i < n; i++) {
      sum += wv_d_data[i]*wv_d_data[i];
    }
  } else {
    sum = cv1d_get_flt_squared_mod_sum ();
  }
  mult = 1/(pow(scale, exponent - 0.5)*sqrt(sum));

  /*
   * The following loops on 2 times the number of points, it is to avoid a cast
   * to the complex type... you see ?
   */

  for (i = 0; i < size_of_data*2; i++) {
    d_result[i] *= mult;
  }

  return d_result;
}

