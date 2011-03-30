/*
 * cv2d.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv2d.c,v 1.1.1.1 1999/05/06 09:38:04 decoster Exp $
 */

#include "cv2d_int.h"
#include "cv2d_fft.h"

#ifdef LOG_MESSAGES
char* be2[4] = {
  "pe", "mi", "pa", "0p"
};
char* method_str2[3] = {
  "di", "mp", "ft"
};
#endif

real cv2d_mp_mult = 2;

/*
 * Global variables to handle filter parameters.
 */

_filter_ flt2;

/*
 * Global variables to handle signal parameters.
 */

int  im_form = CV2D_UNDEFINED; /* Direct and fourier property. CV2D_RC_FORM or */
                              /* CV2D_CC_FORM. */
void *im_d_data = 0;         /* Storage of direct numerical data */
void *im_f_data = 0;         /* Storage of fourier numerical data */
int  im_n = 0;               /* Size of the data arrays */

/*
 */
static double
_null_fct_ (double x,
	    double a)
{
  return 0.0;
}


/*
 * Global variable to store requested convolution method (CV2D_DI, CV2D_MP or CV2D_FT)
 */

int cv2d_method = CV2D_UNDEFINED;

/*
 * cv2d_flt_init_a --
 * This function initialize the parameters for an analytical filter.
 * No return value.
 */

void
cv2d_flt_init_a (real d_begin,
		 real d_end,
		 real f_begin,
		 real f_end,
		 /*double (* d_real_ptr)(),
		 double (* d_imag_ptr)(),
		 double (* f_real_ptr)(),
		 double (* f_imag_ptr)(),*/
		 void *d_real_ptr,
		 void *d_imag_ptr,
		 void *f_real_ptr,
		 void *f_imag_ptr,
		 real scale)
{
  int i_begin, i_end;

  assert ((d_begin <= 0) && (d_end >= 0));
  assert ((f_begin <= 0) && (f_end >= 0));
  assert ((d_real_ptr != 0)
	  || (d_imag_ptr != 0)
	  || (f_real_ptr != 0)
	  || (f_imag_ptr != 0));
  assert ((scale > 0) || (scale == CV2D_NO_SCALE));

  if (d_imag_ptr == 0) {
    if (f_imag_ptr == 0) {
      flt2.form = CV2D_RR_FORM;
    } else {
      flt2.form = CV2D_RC_FORM;
    }
  } else {
    flt2.form = CV2D_CC_FORM;
  }

  /*
   * For now, all the analytical cv2d algorithms work with non pure imaginary
   * filters. If the user gives a pure imaginary form, we correct it with a fct
   * that returns 0.0.
   */

  /*if (d_imag_ptr && !d_real_ptr) {
    d_real_ptr = &_null_fct_;
  }
  if (f_imag_ptr && !f_real_ptr) {
    f_real_ptr = &_null_fct_;
    }*/

  flt2.def = ANALYTICAL;
  flt2.d_data = 0;
  flt2.f_data = 0;

  if (scale == CV2D_NO_SCALE) {
    i_begin = (int) floor (d_begin);
    i_end   = (int) ceil  (d_end);
  } else {
    i_begin = (int) floor (d_begin*scale);
    i_end   = (int) ceil  (d_end*scale);
  }
  flt2.d_n = i_end - i_begin +1;

  if (scale == CV2D_NO_SCALE) {
    i_begin = (int) floor (f_begin);
    i_end   = (int) ceil  (f_end);
  } else {
    i_begin = (int) floor (f_begin/scale);
    i_end   = (int) ceil  (f_end/scale);
  }
  flt2.f_n = i_end - i_begin +1;

  flt2.d_begin = d_begin;
  flt2.d_end = d_end;
  flt2.f_begin = f_begin;
  flt2.f_end = f_end;
  flt2.d_real_ptr = d_real_ptr;
  flt2.d_imag_ptr = d_imag_ptr;
  flt2.f_real_ptr = f_real_ptr;
  flt2.f_imag_ptr = f_imag_ptr;
  flt2.scale = scale;

  return;
}


/*
 * cv2d_flt_init_n --
 * This function initialize the parameters for a numerical filter.
 * No return value.
 */

void
cv2d_flt_init_n (int form,
	       int d_n,
	       int d_0_index,
	       int f_n,
	       int f_0_index,
	       void * d_data,
	       void * f_data)
{
  assert ((form == CV2D_RR_FORM) || (form == CV2D_RC_FORM) || (form == CV2D_CC_FORM));
  assert (d_n > 0);
  assert (d_0_index < d_n);
  assert (d_data != 0);
  if (f_data != 0) {
    assert (f_n > 0);
    assert (f_0_index < f_n);
    assert (d_n > f_n);
  }

  flt2.form = form;
  flt2.def = NUMERICAL;
  flt2.d_data = d_data;
  flt2.f_data = f_data;
  flt2.d_n = d_n;
  flt2.d_begin = (real) -d_0_index;
  flt2.d_end = (real) d_n - d_0_index - 1;
  if (f_data != 0) {
    flt2.f_begin = (real) -f_0_index;
    flt2.f_end = (real) f_n - f_0_index - 1;
    flt2.f_n = f_n;
  } else {
    flt2.f_begin = 0;
    flt2.f_end = 0;
  }

  /* For now, we store the minimum acceptable numerical filter size in */
  /* flt2.d_n. */

  flt2.d_n = max(flt2.d_n, flt2.f_n);

  flt2.d_real_ptr = 0;
  flt2.d_imag_ptr = 0;
  flt2.f_real_ptr = 0;
  flt2.f_imag_ptr = 0;
  flt2.scale = CV2D_NO_SCALE;

  return;
}


/*
 * cv2d_sig_init --
 * This function initialize the parameters for a numerical signal.
 * No return value.
 */

void
cv2d_sig_init (int  form,
	     void * d_data,
	     void * f_data,
	     int  n)
{
  assert ((form == CV2D_RR_FORM) || (form == CV2D_RC_FORM) || (form == CV2D_CC_FORM));
  assert (d_data != 0);
  assert (n > 0);

  im_form = form;
  im_d_data = d_data;
  im_f_data = f_data;
  im_n = n;

  return;
}


/*
 * cv2d_set_method --
 */

void
cv2d_set_method (int method)
{
  assert ((method == CV2D_UNDEFINED)
	  || (method == CV2D_DI)
	  || (method == CV2D_MP)
	  || (method == CV2D_FT));

  cv2d_method = method;
}

/*
 * cv2d_compute --
 */

void *
cv2d_compute (int  border_effect,
	    void *res_data,
	    int  *first_exact_ptr,
	    int  *last_exact_ptr)
{
  void * ret_value = 0;
  real * old_im_d_data_r;
  complex * old_im_d_data_c;
  complex * old_flt_d_data_c;
  complex * old_res_data_c;

  real    * im_d_data_r = 0;
  complex * im_d_data_c = 0;
  real    * flt_d_data_r = 0;
  real    * res_data_r   = 0;

  real *the_d_data = 0;

  int  i;

  int im_nx = im_n;
  int im_ny = im_n;

  _filter_ old_flt;

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

  LogMessage2("%s ", be2[border_effect]);

  /* We save the old state of the filter, in case of state changes. */

  cv2d_flt_copy_(&flt2, &old_flt);

  if ((flt2.def == ANALYTICAL) && (!flt2.f_real_ptr && !flt2.f_imag_ptr)) {
    /* There is no analytical Fourier definition. */
    /* Then we use the numerical algorithm */

    int the_form;
    int the_d_n;

    int the_begin_index;
    int the_end_index;

    if (flt2.scale == CV2D_NO_SCALE) {
      the_begin_index = (int) floor (flt2.d_begin) ;
      the_end_index = (int) ceil (flt2.d_end) ;
    } else {
      the_begin_index = (int) floor (flt2.d_begin*flt2.scale) ;
      the_end_index = (int) ceil (flt2.d_end*flt2.scale) ;
    }

    the_d_n = the_end_index - the_begin_index + 1;

    if (!flt2.d_imag_ptr) {
      the_form = CV2D_RC_FORM;
      the_d_data = (real *) malloc (sizeof(real)*the_d_n);
    } else {
      the_form = CV2D_CC_FORM;
      the_d_data = (real *) malloc (sizeof(complex)*the_d_n);
    }

    if (!the_d_data) {
      EX_RAISE(mem_alloc_error);
    }

    if (flt2.scale == CV2D_NO_SCALE) {
      for (i = the_begin_index; i <= the_end_index; i++) {
	the_d_data[i - the_begin_index] =
	  evaluator_evaluate_x(flt2.d_real_ptr,(double)(i));
      }
    } else {
      for (i = the_begin_index; i <= the_end_index; i++) {
	the_d_data[i - the_begin_index] =
	  evaluator_evaluate_x_y(flt2.d_real_ptr,(double)(i), flt2.scale);
      }
    }

    cv2d_flt_init_n (the_form,
		     the_d_n,
		     -the_begin_index,
		     0,
		     0,
		     the_d_data,
		     0);
  }

  switch (flt2.def) {
  case ANALYTICAL:
    switch (im_form) {
    case CV2D_RC_FORM:
      switch (flt2.form) {

      case CV2D_RR_FORM: /* signal RC - filter RR */
      case CV2D_RC_FORM: /* signal RC - filter RC */
	ret_value = (void *)
	  cv2d_a_real (border_effect, res_data, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}

	ret_value = res_data;
	break;

      case CV2D_CC_FORM: /* signal RC - filter CC */
	old_im_d_data_r = (real *) im_d_data;

	/* We transform the signal into a complex signal. Create a new array. */

	im_d_data_c = (complex *) malloc (im_n*sizeof (complex));
	if (!im_d_data_c) {
	  EX_RAISE(mem_alloc_error);
	}

	for (i = 0; i < im_n; i++) {
	  im_d_data_c[i].real = old_im_d_data_r[i];
	  im_d_data_c[i].imag = 0.0;
	}

	/* Point the global var to the good array, then compute. */

	im_d_data = (void *) im_d_data_c;
	ret_value = (void *)
	  cv2d_a_cplx (border_effect, res_data, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Restore the old state. */

	free (im_d_data_c);

	im_d_data = (void *) old_im_d_data_r;

	ret_value = res_data;
	break;
      }
      break;
    case CV2D_CC_FORM:
      switch (flt2.form) {

      case CV2D_RR_FORM: /* signal CC - filter RR */
      case CV2D_RC_FORM: /* signal CC - filter RC */
	old_im_d_data_c = (complex *) im_d_data;
	old_res_data_c = (complex *) res_data;

	/* We transform the signal into a real signal that contains the real */
	/* part. Create a new array. */

	im_d_data_r = (real *) malloc (im_n*sizeof (real));
	if (!im_d_data_r) {
	  EX_RAISE(mem_alloc_error);
	}
	for (i = 0; i < im_n; i++) {
	  im_d_data_r[i] = old_im_d_data_c[i].real;
	}

	res_data_r = (real *) malloc (im_n*sizeof (real));
	if (!res_data) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Point the global var to the good array, then compute. */

	im_d_data = (void *) im_d_data_r;
	ret_value = (void *)
	  cv2d_a_real (border_effect, res_data_r, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Fill the result array. */

	for (i = 0; i < im_n; i++) {
	  old_res_data_c[i].real = res_data_r[i];
	}

	/* We transform the signal into a real signal that contains the */
	/* imaginary part. */

	for (i = 0; i < im_n; i++) {
	  im_d_data_r[i] = old_im_d_data_c[i].imag;
	}

	/* Point the global var to the good array, then compute. */

	im_d_data = (void *) im_d_data_r;
	ret_value = (void *)
	  cv2d_a_real (border_effect, res_data, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Fill the result array. */

	for (i = 0; i < im_n; i++) {
	  old_res_data_c[i].imag = res_data_r[i];
	}

	/* Restore the old state. */

	free (res_data_r);
	free (im_d_data_r);

	im_d_data = (void *) old_im_d_data_c;
	res_data = (void *) old_res_data_c;

	ret_value = res_data;
	break;

      case CV2D_CC_FORM: /* signal CC - filter CC */
	ret_value = (void *)
	  cv2d_a_cplx (border_effect, res_data, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}

	ret_value = res_data;
	break;
      }
      break;
    }
    break;
  case NUMERICAL:
    switch (im_form) {

    case CV2D_RC_FORM:
      switch (flt2.form) {

      case CV2D_RR_FORM: /* signal RC - filter RR */
      case CV2D_RC_FORM: /* signal RC - filter RC */
	ret_value = (void *)
	  cv2d_n_real (border_effect, res_data, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}

	ret_value = res_data;
	break;

      case CV2D_CC_FORM: /* signal RC - filter CC */
	old_flt_d_data_c = (complex *) flt2.d_data;
	old_res_data_c = (complex *) res_data;
	
	/* We transform the filter into a real signal that contains the real */
	/* part. Create a new array. */

	flt_d_data_r = (real *) malloc (flt2.d_n*sizeof (real));
	if (!flt_d_data_r) {
	  EX_RAISE(mem_alloc_error);
	}
	for (i = 0; i < flt2.d_n; i++) {
	  flt_d_data_r[i] = old_flt_d_data_c[i].real;
	}

	res_data_r = (real *) malloc (im_n*sizeof (real));
	if (!res_data_r) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Point the global var to the good array, then compute. */

	flt2.d_data = (void *) flt_d_data_r;
	ret_value = (void *)
	  cv2d_n_real (border_effect, res_data_r, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}
	for (i = 0; i < im_n; i++) {
	  old_res_data_c[i].real = res_data_r[i];
	}

	/* We transform the filter into a real signal that contains the */
	/* imaginary part. */

	for (i = 0; i < flt2.d_n; i++) {
	  flt_d_data_r[i] = old_flt_d_data_c[i].imag;
	}

	/* Point the global var to the good array, then compute. */

	flt2.d_data = (void *) flt_d_data_r;
	ret_value = (void *)
	  cv2d_n_real (border_effect, res_data_r, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Fill the result array. */

	for (i = 0; i < im_n; i++) {
	  old_res_data_c[i].imag = res_data_r[i];
	}

	/* Restore the old state. */

	free (res_data_r);
	free (flt_d_data_r);

	flt2.d_data = (void *) old_flt_d_data_c;
	res_data = (void *) old_res_data_c;

	ret_value = res_data;
	break;
      }
      break;

    case CV2D_CC_FORM:
      switch (flt2.form) {

      case CV2D_RR_FORM: /* signal CC - filter RR */
      case CV2D_RC_FORM: /* signal CC - filter RC */
	old_im_d_data_c = (complex *) im_d_data;
	old_res_data_c = (complex *) res_data;

	/* We transform the signal into a real signal that contains the real */
	/* part. Create a new array. */

	im_d_data_r = (real *) malloc (im_nx*im_ny*sizeof (real));
	if (!im_d_data_r) {
	  EX_RAISE(mem_alloc_error);
	}
	for (i = 0; i < im_ny*im_ny; i++) {
	  im_d_data_r[i] = old_im_d_data_c[i].real;
	}

	res_data_r = (real *) malloc (im_nx*im_ny*sizeof (real));
	if (!res_data_r) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Point the global var to the good array, then compute. */

	im_d_data = (void *) im_d_data_r;
	ret_value = (void *)
	  cv2d_n_real (border_effect, res_data_r, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Fill the result array. */

	for (i = 0; i < im_nx*im_ny; i++) {
	  old_res_data_c[i].real = res_data_r[i];
	}

	/* We transform the signal into a real signal that contains the real */
	/* part. */

	for (i = 0; i < im_nx*im_ny; i++) {
	  im_d_data_r[i] = old_im_d_data_c[i].imag;
	}

	/* Point the global var to the good array, then compute. */

	im_d_data = (void *) im_d_data_r;
	ret_value = (void *)
	  cv2d_n_real (border_effect, res_data_r, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Fill the result array. */

	for (i = 0; i < im_nx*im_ny; i++) {
	  old_res_data_c[i].imag = res_data_r[i];
	}

	/* Restore the old state. */

	free (res_data_r);
	free (im_d_data_r);

	im_d_data = (void *) old_im_d_data_c;
	res_data = (void *) old_res_data_c;

	ret_value = res_data;
	break;

      case CV2D_CC_FORM: /* signal CC - filter CC */
	ret_value = (void *)
	  cv2d_n_cplx (border_effect, res_data, first_exact_ptr, last_exact_ptr);
	if (!ret_value) {
	  EX_RAISE(mem_alloc_error);
	}

	ret_value = res_data;
	break;
      }
      break;
    }
    break;
  }

  /* Restore the old filter state. */
  cv2d_flt_copy_(&old_flt, &flt2);

  /* Free this eventually allocated memory... hmm. */
  free (the_d_data);

  assert (ret_value != 0);

  LogMessage("\n");

  return ret_value;

mem_alloc_error:
  free (im_d_data_r);
  free (im_d_data_c);
  free (res_data_r);
  free (flt_d_data_r);
  free (the_d_data);

  return 0;
}

