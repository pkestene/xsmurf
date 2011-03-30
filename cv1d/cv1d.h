/*
 * cv1d.h --
 *
 *  This file is the header of the cv library.
 *
 *  Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv1d.h,v 1.8 1999/05/05 20:42:21 decoster Exp $
 */

#ifndef _CV1D_H_
#define _CV1D_H_

#include <matheval.h>

/* 
 * New type. 'real' is the type of the data. Switch to 'double' if you want to
 * change the precision.
 */

#ifndef _REAL_
#define _REAL_
typedef float real;
#endif

#ifndef _COMPLEX_
#define _COMPLEX_
typedef struct
{
  real real;
  real imag;
} complex;
#endif

/*
 * Constants for border_effect parameter of following functions.
 */

enum {
  CV1D_PERIODIC,
  CV1D_MIRROR,
  CV1D_PADDING,
  CV1D_0_PADDING
};

/*
 * Constants for the 'scale' parameter of following functions.
 */

#define CV1D_NO_SCALE 0

/*
 * cv1d_flt_init_a --
 *
 *   This function initialize the parameters for an analytical filter.
 *
 * Arguments :
 *   d_begin,   - Domain of the direct form :        
 *   d_end        [d_begin, d_end]
 *   f_begin,   - Domain of the fourier form :
 *   f_end        [f_begin, f_end]
 *   d_real_ptr - Function pointer for the real part of the direct form.
 *   d_imag_ptr - Function pointer for the imaginary part of the direct form.
 *   f_real_ptr - Function pointer for the real part of the fourier form.
 *   f_imag_ptr - Function pointer for the imaginary part of the fourier form.
 *       -> These four functions must have one of the following proptotypes :     
 *           double fct(double x, double scale)
 *           double fct(double x)
 *       -> You can use a 0 pointer. But at least on must be non 0.
 *       -> If one of the form (direct or Fourier) is not defined, this will
 *          force the use of the other form.
 *   scale      - Scale parameter for the four functions. If there is no scale
 *                parameter, (i.e. the second proptotype) scale must take
 *                CV1D_NO_SCALE value.
 *
 * No return value.
 */

extern void cv1d_flt_init_a (real d_begin,
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
			     real scale);

/*
 * Constants for the 'form' parameter of following functions.
 */

enum {
  CV1D_RR_FORM, /* CV1D_RC_FORM means that the direct form is real (R) and that */
  CV1D_RC_FORM, /* fourier form is complex (C). Etc. */
  CV1D_CC_FORM
};

/*
 * cv1d_flt_init_n --
 *
 * This function initialize the parameters for a numerical filter.
 *
 * Arguments :
 *   form      - Form of the filter (CV1D_RR_FORM, ...).
 *   d_n       - Nuber of point of both d_data.
 *   d_0_index - Index in d_data of point of abscissa 0.
 *   f_n       - Nuber of point of both f_data.
 *   f_0_index - Index in f_data of point of abscissa 0.
 *   d_data    - Array that contains the data of the direct form of the filter.
 *               d_data must be non 0.
 *   f_data    - Array that contains the data of the fourier form of the filter.
 *               f_data can be 0. (hum, in fact there is nothing done if it's 
 *               non 0, perhaps later...)
 *
 * No return value.
 */

extern void cv1d_flt_init_n (int form,
			     int d_n,
			     int d_0_index,
			     int f_n,
			     int f_0_index,
			     void * d_data,
			     void * f_data);

/*
 * cv1d_sig_init --
 *
 * This function initialize the parameters for a numerical signal.
 *
 * Arguments :
 *   form   - Form of the signal (CV1D_RC_FORM or CV1D_CC_FORM).
 *   d_data - Array that contains the data of the direct form of the signal.
 *             d_data must be non 0.
 *   f_data - Array that contains the data of the fourier form of the signal.
 *             f_data can be 0. (hum, in fact there is nothing done if it's non
 *             0, perhaps later...)
 *   n      - Size of both arrays.
 *
 * No return value.
 */

extern void cv1d_sig_init (int  form,
			   void * d_data,
			   void * f_data,
			   int  n);

/*
 * CV1D_UNDEFINED is used for undefined fields for various data.
 */

#define CV1D_UNDEFINED -1

/*
 * Constants for the 'method' parameter of function 'cv1d_set_method'.
 */

enum {
  CV1D_DI, /* Direct convolution */
  CV1D_MP, /* Multi part convolution */
  CV1D_FT  /* Fourier transform convolution */
};

/* cv1d_set_method --
 *
 * This function force the method to use for the convolution. Never use this
 * function. If you _have to_ use this function, you must have a special
 * authorisation from me and you must offer me a beer for every use.
 *
 * Arguments :
 *   method - one of the following constants CV1D_DI, CV1D_MP, CV1D_FT or
 *            CV1D_UNDEFINED (default). CV1D_UNDEFINED means that the method depends
 *            on the signal size and filter size.
 */

extern void cv1d_set_method (int method);

/*
 * cv1d_compute --
 *
 * General function that compute the convolution between a signal (defined with
 * cv1d_sig_init) and a filter (defined with cv1d_flt_init_n or cv1d_flt_init_a). This
 * function choose the correct function(s) to call to compute the convolution.
 * This choice depends on the form (CV1D_RR_FORM, CV1D_RC_FORM or CV1D_CC_FORM) of
 * both the signal and the filter; and it depends on the kind of definition of
 * filter (numerical or analytical). The method (direct, multi-part or fourier
 * transform) is defined by the cv1d_set_method function. If no method is set the
 * choice depends on the signal size and the filter size.
 *
 * Arguments :
 *   border_effect   - indicates the way to handle border effect. Must be
 *                     CV1D_PERIODIC, CV1D_MIRROR, CV1D_PADDING or CV1D_0_PADDING.
 *   res_data        - an allocated array to store the result. The type of this
 *                     array (real or complex) depends on the forms of both the
 *                     signal and the filter.
 *   first_exact_ptr - index of the first point of exact data (i.e. with no
 *                     border effect "pollution").
 *   last_exact_ptr  - index of the last point of exact data (i.e. with no
 *                     border effect "pollution").
 *
 * Return value :
 *   If any memory allocation fails during the computation, the return value
 *   if 0. Otherwise, value of res_data is return.
 */

extern void * cv1d_compute (int  border_effect,
			    void *res_data,
			    int  *first_exact_ptr,
			    int  *last_exact_ptr);

/*
 * All the following functions have exactly the same proptotype than the
 * cv1d_compute function. 
 */

/*
 * Codes for the functions names :
 *   a         : analytical filter definition.
 *   n         : numerical filter definition.
 *   real      : real signal (and so real result).
 *   cplx      : cplx signal (and so cplx result).
 *   d         : direct method.
 *   mp        : multi-part method.
 *   ft        : fourier transform method.
 *   'nothing' : the choice of the method depends on the last call to
 *               cv1d_set_method function.
 */

extern void * cv1d_a_real    (int, void *, int  *, int  *);
extern void * cv1d_a_cplx    (int, void *, int  *, int  *);
extern void * cv1d_n_real    (int, void *, int  *, int  *);
extern void * cv1d_n_cplx    (int, void *, int  *, int  *);

extern void * cv1d_a_real_d  (int, void *, int  *, int  *);
extern void * cv1d_a_real_mp (int, void *, int  *, int  *);
extern void * cv1d_a_real_ft (int, void *, int  *, int  *);

extern void * cv1d_a_cplx_d  (int, void *, int  *, int  *);
extern void * cv1d_a_cplx_mp (int, void *, int  *, int  *);
extern void * cv1d_a_cplx_ft (int, void *, int  *, int  *);

extern void * cv1d_n_real_d  (int, void *, int  *, int  *);
extern void * cv1d_n_real_mp (int, void *, int  *, int  *);
extern void * cv1d_n_real_ft (int, void *, int  *, int  *);

extern void * cv1d_n_cplx_d  (int, void *, int  *, int  *);
extern void * cv1d_n_cplx_mp (int, void *, int  *, int  *);
extern void * cv1d_n_cplx_ft (int, void *, int  *, int  *);

/*
 * cv1d_fft_r --
 *
 *  This functions computes the fast fourier transform of a real signal.
 *
 * Arguments :
 *   in  - array of real data. Its size is n.
 *   out - allocated array to store the result. Its size is n/2. At the end of
 *         the computation, out[0].imag (wich is 0.0 in theory) must contains
 *         the value of out[n/2].real.
 *   n   - size of array in.
 *
 * Return Value :
 *   None.
 */

extern void (*cv1d_fft_r)  (real    *in, complex *out, int n);

/*
 * cv1d_fft_r_i --
 *
 *  This functions computes the inverse fast fourier transform of a real signal.
 *
 * Arguments :
 *   in  - array of complex data. Its size is n/2. out[0].imag (wich is 0.0 in
 *         theory) must contains the value of out[n/2].real.
 *   out - allocated array to store the result. Its size is n.
 *   n   - size of array out.
 *
 * Return Value :
 *   None.
 */

extern void (*cv1d_fft_r_i)(complex *in, real    *out, int n);

/*
 * cv1d_fft_c --
 *
 *  This functions computes the fast fourier transform of a complex signal.
 *
 * Arguments :
 *   in  - array of complex data.
 *   out - allocated array to store the result.
 *   n   - size of both arrays.
 *
 * Return Value :
 *   None.
 */

extern void (*cv1d_fft_c)  (complex *in, complex *out, int n);

/*
 * cv1d_fft_c_i --
 *
 *  This functions computes the inverse fast fourier transform of a complex
 * signal.
 *
 * Arguments :
 *   in  - array of complex data.
 *   out - allocated array to store the result.
 *   n   - size of both arrays.
 *
 * Return Value :
 *   None.
 */

extern void (*cv1d_fft_c_i)(complex *in, complex *out, int n);

/*
 *
 */

extern int (*cv1d_is_good_fft_size)(int size);


/*
 *
 */

extern int (*cv1d_next_good_fft_size)(int size);


/*
 *
 */

extern real (*cv1d_fft_get_time)(int size);


/*
 * cv1d_get_flt_squared_mod_sum --
 *
 *   This function returns the sum of the squared modulus of all the values of
 * the last used analytical filter.
 *
 * Arguments :
 *   None.
 *
 * Return value :
 *   The sum...
 */

double cv1d_get_flt_squared_mod_sum ();

extern real cv1d_mp_mult;

extern int zeTime[20];

extern int cv1dFlag;

#endif /* _CV1D_H_ */
