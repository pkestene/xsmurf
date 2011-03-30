/*
 * cv2d_n.c --
 *
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv2d_n.c,v 1.2 1999/05/15 15:58:57 decoster Exp $
 */

#include "cv2d_int.h"
#include "cv2d_limits.h"

int cv2dFlag;

/*
 * Array to store all the basic numerical convolution fonctions. Stored by
 * method.
 */

void * (* cv2d_n_fct_ptr_array[2][3])() = {
  {
    cv2d_n_real_d,
    cv2d_n_real_mp,
    cv2d_n_real_ft
  }, {
    cv2d_n_cplx_d,
    cv2d_n_cplx_mp,
    cv2d_n_cplx_ft
  }
};

/*
 * cv2d_n_real --
 */
void *
cv2d_n_real (int  border_effect,
	     void *res_data,
	     int  *first_exact_ptr,
	     int  *last_exact_ptr)
{
  int method;
  real * ret_value = 0;
  void * (*the_cv2d_fct_ptr)();


  LogMessage("n r ");

  if (cv2d_method == CV2D_UNDEFINED) {
    method =
      cv2d_convolution_method_ (im_n, flt2.d_n, lim_array[NUMERICAL][border_effect]);
  } else {
    method = cv2d_method;
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
  the_cv2d_fct_ptr = cv2d_n_fct_ptr_array[REAL][method];

  SetLogTimeBegin();
  ret_value = 
    the_cv2d_fct_ptr (border_effect, res_data, first_exact_ptr, last_exact_ptr);
  LogTime();

  return ret_value;
}


#define _outside_(x, y, nx, ny) (x<0 || y<0 || x>nx || y>ny)

/*
 * cv2d_n_real_d --
 */
void *
cv2d_n_real_d (int  border_effect,
	       void *res_data,
	       int  *first_exact_ptr,
	       int  *last_exact_ptr)
{
  real *filter_data;
  real *signal_data;
  real *result_data;
  real sig_val;
  int  ix, jx1, jx2;
  int  iy, jy1, jy2;
  int  jx3, jy3;
  int ind;
  int ind1;
  //int ind2;
  int im_nx, im_ny;
  int  beginx, endx; /* Begin and end of the filter */
  int  beginy, endy; /* Begin and end of the filter */

  assert (flt2.def == NUMERICAL);
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

  beginx = (int) flt2.d_begin;
  endx   = (int) flt2.d_end;
  beginy = (int) flt2.d_begin;
  endy   = (int) flt2.d_end;

  filter_data = (real *) (flt2.d_data) - beginx - beginy*flt2.d_n;
  signal_data = (real *) im_d_data;
  result_data = (real *) res_data;

  im_nx = im_n;
  im_ny = im_n;

  ind = 0;
  for (iy = 0; iy < im_ny; iy++) {
    for (ix = 0; ix < im_nx; ix++, ind++) {
      result_data[ind] = 0;
      ind1 = beginx + beginy*flt2.d_n;

      for (jy1 = beginy, jy2 = (iy - beginy);
	   jy1 <= endy;
	   jy1++, jy2--) {
	     
	for (jx1 = beginx, jx2 = (ix - beginx);
	     jx1 <= endx;
	     jx1++, jx2--, ind1++) {
	  sig_val = -1;
	  jx3 = jx2;
	  jy3 = jy2;
	  if (jx2 < 0) {
	    switch (border_effect) {
	    case CV2D_PERIODIC:
	      jx3 = (abs(jx2/im_nx)+1)*im_nx+jx2;
	      break;
	    case CV2D_0_PADDING:
	      sig_val = 0;
	      break;
	    case CV2D_PADDING:
	      jx3 = 0;
	      break;
	    case CV2D_MIRROR:
	      jx3 = -jx2;
	      break;
	    }
	  }
	  if (jy2 < 0) {
	    switch (border_effect) {
	    case CV2D_PERIODIC:
	      jy3 = (abs(jy2/im_ny)+1)*im_ny+jy2;
	      break;
	    case CV2D_0_PADDING:
	      sig_val = 0;
	      break;
	    case CV2D_PADDING:
	      jy3 = 0;
	      break;
	    case CV2D_MIRROR:
	      jy3 = -jy2;
	      break;
	    }
	  }
	  if (jx2 >= im_nx) {
	    switch (border_effect) {
	    case CV2D_PERIODIC:
	      jx3 = jx2 % im_nx;
	      break;
	    case CV2D_0_PADDING:
	      sig_val = 0;
	      break;
	    case CV2D_PADDING:
	      jx3 = im_nx-1;
	      break;
	    case CV2D_MIRROR:
	      jx3 = 2*im_nx - 2 - jx2;
	      break;
	    }
	  }
	  if (jy2 >= im_ny) {
	    switch (border_effect) {
	    case CV2D_PERIODIC:
	      jy3 = jy2 % im_ny;
	      break;
	    case CV2D_0_PADDING:
	      sig_val = 0;
	      break;
	    case CV2D_PADDING:
	      jy3 = im_ny-1;
	      break;
	    case CV2D_MIRROR:
	      jy3 = 2*im_ny - 2 - jy2;
	      break;
	    }
	  }
	  if (sig_val != 0) {
	    sig_val = signal_data[jx3+jy3*im_nx];
	    result_data[ind] += filter_data[ind1] * sig_val;
	  }
	}
      }
    }
  }
  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;
}


int zeTime2[20];

int
cv2d_get_good_part_size (int size);
/*
 * cv2d_n_real_mp --
 */
void *
cv2d_n_real_mp (int  border_effect,
		void *res_data,
		int  *first_exact_ptr,
		int  *last_exact_ptr)
{
  real    *signal_data;
  real    *result_data;
  real    *filter_data;
  real    *signal_part;
  real    *new_filter;
  complex *signal_part_ft;
  complex *new_filter_ft;
  int     nb_of_partsx;
  int     nb_of_partsy;
  int     part_sizex;
  int     part_sizey;
  int     part_nbx;
  int     part_nby;
  int     size_of_exact_datax;
  int     filter_beginx;
  int     filter_endx;
  int     size_of_exact_datay;
  int     filter_beginy;
  int     filter_endy;

  int     im_nx = im_n;
  int     im_ny = im_n;

  //int    begin, end;
  //struct tms time;
  //int i;

  real fft_factor; /* Used for non normalised fft (like FFTW) */

  assert (flt2.def == NUMERICAL);
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

  signal_data = (real *) im_d_data;
  filter_data = (real *) flt2.d_data;
  result_data = (real *) res_data;

  filter_beginx = (int) flt2.d_begin;
  filter_endx   = (int) flt2.d_end;
  filter_beginy = (int) flt2.d_begin;
  filter_endy   = (int) flt2.d_end;

  if (cv2dFlag == 1) {
    part_sizex = cv2d_next_good_fft_size (cv2d_mp_mult*flt2.d_n);
    part_sizey = cv2d_next_good_fft_size (cv2d_mp_mult*flt2.d_n);
  } else {
    part_sizex = cv2d_get_good_part_size(flt2.d_n);
    part_sizey = cv2d_get_good_part_size(flt2.d_n);
  }
  size_of_exact_datax = part_sizex - flt2.d_n + 1;
  size_of_exact_datay = part_sizey - flt2.d_n + 1;

  new_filter = cv2d_pure_periodic_extend_ (filter_data,
					   filter_beginx,
					   filter_endx,
					   part_sizex,
					   filter_beginy,
					   filter_endy,
					   part_sizey);

  if (!new_filter) {
    EX_RAISE(mem_alloc_error);
  }

  new_filter_ft = (complex *) malloc (sizeof (complex)*(part_sizex/2+1)*part_sizey);
  if (!new_filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv2d_fft_r (new_filter, new_filter_ft, part_sizex, part_sizey);

  nb_of_partsx = ceil (((double) im_nx)/size_of_exact_datax);
  nb_of_partsy = ceil (((double) im_ny)/size_of_exact_datay);

  signal_part = (real *) malloc (sizeof (real)*part_sizex*part_sizey);
  if (!signal_part) {
    EX_RAISE(mem_alloc_error);
  }
  signal_part_ft = (complex *) malloc (sizeof (complex)*(part_sizex/2+1)*part_sizey);
  if (!signal_part_ft) {
    EX_RAISE(mem_alloc_error);
  }

  fft_factor = cv2d_get_fft_factor(part_sizex, part_sizey);

  for (part_nby = 0; part_nby < nb_of_partsy; part_nby++) {
    int part_begin_in_signaly;

    part_begin_in_signaly = part_nby*size_of_exact_datay - filter_endy;

    for (part_nbx = 0; part_nbx < nb_of_partsx; part_nbx++) {
      int part_begin_in_signalx;

      part_begin_in_signalx = part_nbx*size_of_exact_datax - filter_endx;
      cv2d_get_part_r_ (signal_part, part_sizex, part_sizey,
			signal_data, im_nx, im_ny,
			part_begin_in_signalx, part_begin_in_signaly,
			border_effect);

      cv2d_fft_r (signal_part, signal_part_ft, part_sizex, part_sizey);

      cv2d_cplx_mult_ (signal_part_ft, new_filter_ft, (part_sizex/2 + 1)*part_sizey, fft_factor);

      cv2d_fft_r_i (signal_part_ft, signal_part, part_sizex, part_sizey);

      cv2d_put_part_in_result_r_ (result_data, im_nx, im_ny,
				  signal_part, part_sizex, part_sizey,
				  part_begin_in_signalx, part_begin_in_signaly,
				  filter_endx, filter_endy,
				  size_of_exact_datax, size_of_exact_datay);
    }
  }
  
  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  free (signal_part);
  free (signal_part_ft);
  free (new_filter);
  free (new_filter_ft);
  
  return res_data;

mem_alloc_error:
  free (signal_part);
  free (signal_part_ft);
  free (new_filter);
  free (new_filter_ft);

  return 0;
}


/*
 * cv2d_n_real_ft --
 */
void *
cv2d_n_real_ft (int  border_effect,
		void *res_data,
		int  *first_exact_ptr,
		int  *last_exact_ptr)
{
  real    *signal_data;
  real    *filter_data;
  int     filter_beginx;
  int     filter_endx;
  int     filter_beginy;
  int     filter_endy;
  real    *result_data;
  real    *new_signal;
  real    *new_filter;
  complex *new_signal_ft;
  complex *new_filter_ft;
  int     new_sizex;
  int     new_sizey;
  
  int im_nx = im_n;
  int im_ny = im_n;

  real fft_factor; /* Used for non normalised fft (like FFTW) */

  assert (flt2.def == NUMERICAL);
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
  assert (cv2d_is_good_fft_size (im_nx));
  assert (cv2d_is_good_fft_size (im_ny));

  signal_data = (real *) im_d_data;
  filter_data = (real *) flt2.d_data;
  result_data = (real *) res_data;

  filter_beginx = (int) flt2.d_begin;
  filter_endx   = (int) flt2.d_end;
  filter_beginy = (int) flt2.d_begin;
  filter_endy   = (int) flt2.d_end;

  switch (border_effect) {
  case CV2D_0_PADDING:
  case CV2D_PADDING:
  case CV2D_MIRROR:
    new_sizex = cv2d_next_good_fft_size(im_nx - filter_beginx + filter_endx);
    new_sizey = cv2d_next_good_fft_size(im_ny - filter_beginy + filter_endy);
    new_signal = cv2d_extend_r_ (signal_data,
				 im_nx,
				 im_ny,
				 new_sizex,
				 new_sizey,
				 -filter_beginx,
				 filter_endx,
				 -filter_beginy,
				 filter_endy,
				 border_effect);
    break;
  case CV2D_PERIODIC:
    new_sizex = im_nx;
    new_sizey = im_ny;
    new_signal = signal_data;
    break;
  }

  if (!new_signal) {
    EX_RAISE(mem_alloc_error);
  }

  new_filter = cv2d_pure_periodic_extend_ (filter_data,
					   filter_beginx,
					   filter_endx,
					   new_sizex,
					   filter_beginy,
					   filter_endy,
					   new_sizey);
  if (!new_filter) {
    EX_RAISE(mem_alloc_error);
  }

  new_filter_ft = (complex *) malloc (sizeof (complex)*(new_sizex/2+1)*new_sizey);
  if (!new_filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv2d_fft_r (new_filter, new_filter_ft, new_sizex, new_sizey);

  new_signal_ft = (complex *) malloc (sizeof (complex)*(new_sizex/2+1)*new_sizey);
  if (!new_signal_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv2d_fft_r (new_signal, new_signal_ft, new_sizex, new_sizey);

  fft_factor = cv2d_get_fft_factor(new_sizex, new_sizey);

  cv2d_cplx_mult_ (new_signal_ft, new_filter_ft, (new_sizex/2+1)*new_sizey, fft_factor);

  if (border_effect == CV2D_PERIODIC) {
      cv2d_fft_r_i (new_signal_ft, result_data, new_sizex, new_sizey);
  } else {
      cv2d_fft_r_i (new_signal_ft, new_signal, new_sizex, new_sizey);
      cv2d_put_signal_in_result_r_(result_data,
				   new_signal,
				   im_nx,
				   im_ny,
				   new_sizex,
				   new_sizey,
				   -filter_beginx,
				   filter_endx,
				   -filter_beginy,
				   filter_endy);
      free (new_signal);
    }

  free (new_signal_ft);
  free (new_filter);
  free (new_filter_ft);
  
  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  if (border_effect != CV2D_PERIODIC) {
    free (new_signal);
  }
  free (new_signal_ft);
  free (new_filter);
  free (new_filter_ft);

  return 0;
}


/*
 * cv2d_n_cplx --
 */
void *
cv2d_n_cplx (int  border_effect,
	   void *res_data,
	   int  *first_exact_ptr,
	   int  *last_exact_ptr)
{
  int method;
  real * ret_value = 0;
  void * (*the_cv2d_fct_ptr)();

  LogMessage("n c ");
  if (cv2d_method == CV2D_UNDEFINED) {
    method =
      cv2d_convolution_method_ (im_n, flt2.d_n, lim_array[NUMERICAL][border_effect]);
  } else {
    method = cv2d_method;
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
  the_cv2d_fct_ptr = cv2d_n_fct_ptr_array[CPLX][method];
  SetLogTimeBegin();
  ret_value = 
    the_cv2d_fct_ptr (border_effect, res_data, first_exact_ptr, last_exact_ptr);
  LogTime();

  return ret_value;
}


#define cplx_mult_and_add(r,a,b) (r).real+=(a).real*(b).real-(a).imag*(b).imag;\
(r).imag+=(a).real*(b).imag+(a).imag*(b).real

/*
 * cv2d_n_cplx_d --
 */
void *
cv2d_n_cplx_d (int  border_effect,
	     void *res_data,
	     int  *first_exact_ptr,
	     int  *last_exact_ptr)
{
  complex *signal_data;
  complex *filter_data;
  complex *result_data;
  complex sig_val;
  int  ix, jx1, jx2;
  int  iy, jy1, jy2;
  int  jx3, jy3;
  int ind;
  int ind1;
  //int ind2;
  int im_nx, im_ny;
  int  beginx, endx; /* Begin and end of the filter */
  int  beginy, endy; /* Begin and end of the filter */

  assert (flt2.def == NUMERICAL);
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

  beginx = (int) flt2.d_begin;
  endx   = (int) flt2.d_end;
  beginy = (int) flt2.d_begin;
  endy   = (int) flt2.d_end;

  filter_data = (complex *) (flt2.d_data) - beginx - beginy*flt2.d_n;
  signal_data = (complex *) im_d_data;
  result_data = (complex *) res_data;

  im_nx = im_n;
  im_ny = im_n;

  ind = 0;
  for (iy = 0; iy < im_ny; iy++) {
    for (ix = 0; ix < im_nx; ix++, ind++) {
      result_data[ind].real = 0;
      result_data[ind].imag = 0;
      ind1 = beginx + beginy*flt2.d_n;

      for (jy1 = beginy, jy2 = (iy - beginy);
	   jy1 <= endy;
	   jy1++, jy2--) {
	     
	for (jx1 = beginx, jx2 = (ix - beginx);
	     jx1 <= endx;
	     jx1++, jx2--, ind1++) {
	  sig_val.real = -1;
	  jx3 = jx2;
	  jy3 = jy2;
	  if (jx2 < 0) {
	    switch (border_effect) {
	    case CV2D_PERIODIC:
	      jx3 = (abs(jx2/im_nx)+1)*im_nx+jx2;
	      break;
	    case CV2D_0_PADDING:
	      sig_val.real = 0;
	      sig_val.imag = 0;
	      break;
	    case CV2D_PADDING:
	      jx3 = 0;
	      break;
	    case CV2D_MIRROR:
	      jx3 = -jx2;
	      break;
	    }
	  }
	  if (jy2 < 0) {
	    switch (border_effect) {
	    case CV2D_PERIODIC:
	      jy3 = (abs(jy2/im_ny)+1)*im_ny+jy2;
	      break;
	    case CV2D_0_PADDING:
	      sig_val.real = 0;
	      sig_val.imag = 0;
	      break;
	    case CV2D_PADDING:
	      jy3 = 0;
	      break;
	    case CV2D_MIRROR:
	      jy3 = -jy2;
	      break;
	    }
	  }
	  if (jx2 >= im_nx) {
	    switch (border_effect) {
	    case CV2D_PERIODIC:
	      jx3 = jx2 % im_nx;
	      break;
	    case CV2D_0_PADDING:
	      sig_val.real = 0;
	      sig_val.imag = 0;
	      break;
	    case CV2D_PADDING:
	      jx3 = im_nx-1;
	      break;
	    case CV2D_MIRROR:
	      jx3 = 2*im_nx - 2 - jx2;
	      break;
	    }
	  }
	  if (jy2 >= im_ny) {
	    switch (border_effect) {
	    case CV2D_PERIODIC:
	      jy3 = jy2 % im_ny;
	      break;
	    case CV2D_0_PADDING:
	      sig_val.real = 0;
	      sig_val.imag = 0;
	      break;
	    case CV2D_PADDING:
	      jy3 = im_ny-1;
	      break;
	    case CV2D_MIRROR:
	      jy3 = 2*im_ny - 2 - jy2;
	      break;
	    }
	  }
	  if (sig_val.real != 0) {
	    sig_val = signal_data[jx3+jy3*im_nx];
	    cplx_mult_and_add (result_data[ind],
			       filter_data[ind1],
			       sig_val);
	  }
	}
      }
    }
  }

  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;
}


/*
 * cv2d_n_cplx_mp --
 */
void *
cv2d_n_cplx_mp (int  border_effect,
		void *res_data,
		int  *first_exact_ptr,
		int  *last_exact_ptr)
{
  complex *signal_data;
  complex *filter_data;
  complex *result_data;
  complex *signal_part;
  complex *new_filter;
  complex *signal_part_ft;
  complex *new_filter_ft;

  int     nb_of_partsx;
  int     nb_of_partsy;
  int     part_sizex;
  int     part_sizey;
  int     part_nbx;
  int     part_nby;
  int     size_of_exact_datax;
  int     filter_beginx;
  int     filter_endx;
  int     size_of_exact_datay;
  int     filter_beginy;
  int     filter_endy;

  int     im_nx = im_n;
  int     im_ny = im_n;

  real fft_factor;

  assert (flt2.def == NUMERICAL);
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
  filter_data = (complex *) flt2.d_data;
  result_data = (complex *) res_data;

  filter_beginx = (int) flt2.d_begin;
  filter_endx   = (int) flt2.d_end;
  filter_beginy = (int) flt2.d_begin;
  filter_endy   = (int) flt2.d_end;

  if (cv2dFlag == 1) {
    part_sizex = cv2d_next_good_fft_size (cv2d_mp_mult*flt2.d_n);
    part_sizey = cv2d_next_good_fft_size (cv2d_mp_mult*flt2.d_n);
  } else {
    part_sizex = cv2d_get_good_part_size(flt2.d_n);
    part_sizey = cv2d_get_good_part_size(flt2.d_n);
  }
  size_of_exact_datax = part_sizex - flt2.d_n + 1;
  size_of_exact_datay = part_sizey - flt2.d_n + 1;


  new_filter = cv2d_pure_cplx_periodic_extend_  (filter_data,
					   filter_beginx,
					   filter_endx,
					   filter_beginy,
					   filter_endy,
					   part_sizex,
					   part_sizey);
  if (!new_filter) {
    EX_RAISE(mem_alloc_error);
  }
  new_filter_ft = (complex *) malloc (sizeof (complex)*part_sizex*part_sizey);
  if (!new_filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv2d_fft_c (new_filter, new_filter_ft, part_sizex, part_sizey);

  nb_of_partsx = ceil (((double) im_nx)/size_of_exact_datax);
  nb_of_partsy = ceil (((double) im_ny)/size_of_exact_datay);

  signal_part = (complex *) malloc (sizeof (complex)*part_sizex*part_sizey);
  if (!signal_part) {
    EX_RAISE(mem_alloc_error);
  }

  signal_part_ft = (complex *) malloc (sizeof (complex)*part_sizex*part_sizey);
  if (!signal_part_ft) {
    EX_RAISE(mem_alloc_error);
  }

  fft_factor = cv2d_get_fft_factor(part_sizex, part_sizey);

  for (part_nby = 0; part_nby < nb_of_partsy; part_nby++) {
    int part_begin_in_signaly;

    part_begin_in_signaly = part_nby*size_of_exact_datay - filter_endy;

    for (part_nbx = 0; part_nbx < nb_of_partsx; part_nbx++) {
      int part_begin_in_signalx;

      part_begin_in_signalx = part_nbx*size_of_exact_datax - filter_endx;

      cv2d_get_part_c_ (signal_part, part_sizex, part_sizey,
			signal_data, im_nx, im_ny,
			part_begin_in_signalx, part_begin_in_signaly,
			border_effect);
      /*      cv2d_get_part_c_ (signal_part, part_size,
			signal_data, im_n,
			part_begin_in_signal,
			border_effect);*/

      cv2d_fft_c (signal_part, signal_part_ft, part_sizex, part_sizey);

      cv2d_cplx_mult_ (signal_part_ft, new_filter_ft, part_sizex*part_sizey, fft_factor);

      cv2d_fft_c_i (signal_part_ft, signal_part, part_sizex, part_sizey);

      cv2d_put_part_in_result_c_ (result_data, im_nx, im_ny,
				  signal_part, part_sizex, part_sizey,
				  part_begin_in_signalx, part_begin_in_signaly,
				  filter_endx, filter_endy,
				  size_of_exact_datax, size_of_exact_datay);
    }
  }

  free (signal_part);
  free (signal_part_ft);
  free (new_filter);
  free (new_filter_ft);
  
  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (signal_part);
  free (signal_part_ft);
  free (new_filter);
  free (new_filter_ft);

  return 0;
}


/*
 * cv2d_n_cplx_ft --
 */
void *
cv2d_n_cplx_ft (int  border_effect,
	      void *res_data,
	      int  *first_exact_ptr,
	      int  *last_exact_ptr)
{
  complex *signal_data;
  complex *filter_data;
  int     filter_beginx;
  int     filter_endx;
  int     filter_beginy;
  int     filter_endy;
  complex *result_data;
  complex *new_signal;
  complex *new_filter;
  complex *new_signal_ft;
  complex *new_filter_ft;
  int     new_sizex;
  int     new_sizey;

  int     im_nx = im_n;
  int     im_ny = im_n;
  
  real fft_factor; /* Used for non normalised fft (like FFTW) */

  assert (flt2.def == NUMERICAL);
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
  assert (cv2d_is_good_fft_size (im_nx));
  assert (cv2d_is_good_fft_size (im_ny));

  signal_data = (complex *) im_d_data;
  filter_data = (complex *) flt2.d_data;
  result_data = (complex *) res_data;

  filter_beginx = (int) flt2.d_begin;
  filter_endx   = (int) flt2.d_end;
  filter_beginy = (int) flt2.d_begin;
  filter_endy   = (int) flt2.d_end;

  switch (border_effect) {
  case CV2D_0_PADDING:
  case CV2D_PADDING:
  case CV2D_MIRROR:
    new_sizex = cv2d_next_good_fft_size(im_nx - filter_beginx + filter_endx);
    new_sizey = cv2d_next_good_fft_size(im_ny - filter_beginy + filter_endy);
    new_signal = cv2d_extend_c_ (signal_data,
				 im_nx,
				 im_ny,
				 new_sizex,
				 new_sizey,
				 -filter_beginx,
				 filter_endx,
				 -filter_beginy,
				 filter_endy,
				 border_effect);
    break;
  case CV2D_PERIODIC:
    new_sizex = im_nx;
    new_sizey = im_ny;
    new_signal = signal_data;
    break;
  }
  /*  switch (border_effect) {
  case CV2D_0_PADDING:
    new_sizex = im_nx*2;
    new_sizey = im_ny*2;
    new_signal = cv2d_cplx_0_padding_transform_ (signal_data,
						 im_n,
						 filter_end);
    break;
  case CV2D_PADDING:
    new_sizex = im_nx*2;
    new_sizey = im_ny*2;
    new_signal = cv2d_cplx_padding_transform_ (signal_data,
					       im_n,
					       filter_end);
    break;
  case CV2D_MIRROR:
    new_sizex = im_nx*2;
    new_sizey = im_ny*2;
    new_signal = cv2d_cplx_mirror_transform_ (signal_data,
					      im_n,
					      filter_end);
    break;
  case CV2D_PERIODIC:
    new_size = im_n;
    new_signal = signal_data;
    break;
    }*/
  if (!new_signal) {
    EX_RAISE(mem_alloc_error);
  }

  new_filter = cv2d_pure_cplx_periodic_extend_ (filter_data,
						filter_beginx,
						filter_endx,
						filter_beginy,
						filter_endy,
						new_sizex,
						new_sizey);
  if (!new_filter) {
    EX_RAISE(mem_alloc_error);
  }
  new_filter_ft = (complex *) malloc (sizeof (complex)*new_sizex*new_sizey);
  if (!new_filter_ft) {
    EX_RAISE(mem_alloc_error);
  }
  cv2d_fft_c (new_filter, new_filter_ft, new_sizex, new_sizey);

  new_signal_ft = (complex *) malloc (sizeof (complex)*new_sizex*new_sizey);
  if (!new_signal_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv2d_fft_c (new_signal, new_signal_ft, new_sizex, new_sizey);

  fft_factor = cv2d_get_fft_factor(new_sizex, new_sizey);

  cv2d_cplx_mult_ (new_signal_ft, new_filter_ft, new_sizex*new_sizey, fft_factor);
  
  if (border_effect == CV2D_PERIODIC) {
    cv2d_fft_c_i (new_signal_ft, result_data, new_sizex, new_sizey);
  } else {
    cv2d_fft_c_i (new_signal_ft, new_signal, new_sizex, new_sizey);
    cv2d_put_signal_in_result_c_(result_data,
				 new_signal,
				 im_nx,
				 im_ny,
				 new_sizex,
				 new_sizey,
				 -filter_beginx,
				 filter_endx,
				 -filter_beginy,
				 filter_endy);
    /*    memcpy (result_data, new_signal, im_nx*im_ny*sizeof (complex));*/
    free (new_signal);
  }

  free (new_signal_ft);
  free (new_filter);
  free (new_filter_ft);
  

  cv2d_set_f_l_exact_ (first_exact_ptr, last_exact_ptr);

  return res_data;
mem_alloc_error:
  free (new_signal);
  free (new_signal_ft);
  free (new_filter);
  free (new_filter_ft);

  return 0;
}

