/*
 * wt1d.h --
 *
 *  Header of the wt1d library.
 *
 *  Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: wt1d.h,v 1.14 1998/11/24 16:13:05 decoster Exp $
 */

#ifndef _WT1D_H_
#define _WT1D_H_

#include <cv1d.h>

#include <matheval.h>

/*
 * Declaration of the wavelet structure. This is a private structure .
 */

typedef struct _wavelet_ Wavelet;

/*
 *  Different types of wavelets. Example : WAVE_REAL_CPLX defined a wavelet
 * whose direct form is real and whose fourier form is complex.
 */

enum
{			/* direct form + fourier form */
  WAVE_REAL_REAL,	/*  real even  |  real even   */
  WAVE_REAL_IMAG,	/*  real odd   |  imag odd    */
  WAVE_REAL_CPLX,	/*  real       |              */
  WAVE_IMAG_REAL,	/*  imag odd   |  real odd    */
  WAVE_IMAG_IMAG,       /*  imag even  |  imag even   */
  WAVE_IMAG_CPLX,	/*  imag       |              */
  WAVE_CPLX_REAL,	/*             |  real        */
  WAVE_CPLX_IMAG,	/*             |  imag        */
  WAVE_CPLX_CPLX	/*             |              */
};

/*
 * Different wavelet form.
 */

enum {
  WAVE_DIRECT,
  WAVE_FOURIER
};

/*
 * wt1d_new_wavelet --
 *
 * Creation of a new wavelet.
 *
 * Arguments :
 *   type            - Wavelet type. Must be WAVE_REAL_REAL, WAVE_REAL_IMAG,
 *                     WAVE_REAL_CPLX, WAVE_IMAG_REAL, etc.
 *   d_r_fct_ptr     - 4 function pointers are needed. Real and imaginary part
 *   d_i_fct_ptr       of the direct and the fourier form of the wavelet.
 *   f_r_fct_ptr       d_r_fct_ptr and f_r_fct_ptr must be non-NULL. All the
 *   f_i_fct_ptr       pointers must be coherent with the type of the wavelet.
 *   d_x_min         - Domain of the direct form : [d_x_min, d_x_min].
 *   d_x_max          
 *   f_x_min         - Domain of the fourier form : [f_x_min, f_x_min].
 *   f_x_max          
 *   time_scale_mult - Unused (to be removed ?!?!)
 *   freq_scale_mult - Unused (to be removed ?!?!)
 *
 * Return value :
 *   A pointer to a brand new wavelet !
 */

extern Wavelet *
wt1d_new_wavelet (int    type,
		  /*double (*d_r_fct_ptr)(double, double),
		  double (*d_i_fct_ptr)(double, double),
		  double (*f_r_fct_ptr)(double, double),
		  double (*f_i_fct_ptr)(double, double),*/
		  void *d_r_fct_ptr,
		  void *d_i_fct_ptr,
		  void *f_r_fct_ptr,
		  void *f_i_fct_ptr,
		  real   d_x_min,
		  real   d_x_max,
		  real   f_x_min,
		  real   f_x_max,
		  real   time_scale_mult,
		  real   freq_scale_mult);

/*
 * wt1d_get_wavelet_attributes --
 *
 * Get the attributes of a wavelet wavelet.
 *
 * Arguments :
 *   wavelet         - The wavelet.
 *   type            - Wavelet type. will be WAVE_REAL_REAL, WAVE_REAL_CPLX,
 *                     WAVE_CPLX_REAL or WAVE_CPLX_CPLX.
 *   d_r_fct_ptr     - 4 function pointers. Real and imaginary part
 *   d_i_fct_ptr       of the direct and the fourier form of the wavelet.
 *   f_r_fct_ptr       d_r_fct_ptr and f_r_fct_ptr are non-NULL.
 *   f_i_fct_ptr
 *   d_x_min         - Domain of the direct form : [d_x_min, d_x_min].
 *   d_x_max          
 *   f_x_min         - Domain of the fourier form : [f_x_min, f_x_min].
 *   f_x_max          
 *   time_scale_mult - Unused (to be removed ?!?!)
 *   freq_scale_mult - Unused (to be removed ?!?!)
 *
 * Return value :
 *   None.
 */

extern void
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
			     real   *freq_scale_mult);

/*
 * wt1d_free_wavelet --
 *
 * Destruction of a wavelet.
 *
 * Arguments :
 *   wavelet - hmmm, the wavelet to exterminate.
 *
 * Return value :
 *  None.
 */

extern void
wt1d_free_wavelet (Wavelet *wavelet);

/*
 * wt1d_wavelet_type --
 *
 * Gets the type of a wavelet.
 *
 * Arguments :
 *   wavelet - The wavelet.
 *
 * Return value :
 *   The wavelet type.
 */

extern int
wt1d_wavelet_type (Wavelet *wavelet);

/*
 * wt1d_wavelet_to_data --
 *
 *  Fill an array with a wavelet.
 *
 * Arguments :
 *   wavelet      - The wavelet.
 *   scale        - Scale to use.
 *   size_of_data - Size of the array to fill.
 *   data         - Allocated array to fill.
 *   first_x      - (out) Defines the domain of the wavelet at this scale :
 *   last_x         [first_x, last_x]
 *   flag         - This flag indicates the request form of the wavelet
 *                  (WAVE_FOURIER or WAVE_DIRECT).
 *
 * Return value :
 *   None.
 */

extern void
wt1d_wavelet_to_data (Wavelet *wavelet,
		      real    scale,
		      int     size_of_data,
		      void    *data,
		      real    *first_x,
		      real    *last_x,
		      int     flag);

/*
 * wt1d_tab_wavelet --
 *
 * Create (if it doesn't exist) a tabulation associated to a wavelet.
 *
 * Arguments :
 *   wavelet      - The wavelet.
 *   scale        - The scale to tabulate.
 *   dx           - Step between to points of the created tabulation (usually
 *                  dx = 1).
 *   tab_size_ptr - (out) Number of points of the created tabulation.
 *   flag         - This flag indicates the request form of the wavelet
 *                  (WAVE_FOURIER or WAVE_DIRECT).
 *
 * Return value ;
 *   A pointer to the array of data that contains the data of the tabulation.
 * The return value is 0 if a memory allocation error occurs.
 */

extern real *
wt1d_tab_wavelet (Wavelet *wavelet,
		  real    scale,
		  real    dx,
		  int     *tab_size_ptr,
		  int     flag);

/*
 * wt1d_wavelet_scale_limits --
 *
 * Cets the range of allowed scales associated to a wavelet.
 *
 * Arguments :
 *   wavelet   - The wavelet.
 *   d_size    - Size of the signal that will be transformed with the wavelet.
 *   min_scale - (out) First allowed scale.
 *   max_scale - (out) Last allowed scale.
 *
 * Return value ;
 *   None.
 */

extern void
wt1d_wavelet_scale_limits (Wavelet *wavelet,
			   real    d_size,
			   real    *min_scale,
			   real    *max_scale);

/*
 * wt1d_real_single --
 *
 * Wavelet transform at one scale of a real signal.
 *
 * Arguments :
 *   d_data        - Array that contains the data of the direct form of the
 *                   signal. d_data must be non 0.
 *   size_of_data  - Number of points of d_data.
 *   wavelet       - A pointer to the wavelet to use.
 *   scale         - Scale of the transform.
 *   exponent      - Exponent of the normalisation (i.e. 1/a^exponent).
 *   d_result      - An allocated array to store the result. The type of the
 *                   result depends on the form of the filter. If the result is
 *                   complex, real part are in even index of the array and
 *                   imaginary part are in odd index of the array.
 *   border_effect - Indicates the way to handle border effect. Must be
 *                   CV1D_PERIODIC, CV1D_MIRROR or CV1D_PADDING.
 *   first_exact_ptr - (out) First index in array d_result that has no border
 *                     effect.
 *   last_exact_ptr  - (out) Last index in array d_result that has no border
 *                     effect.
 *
 * Return value :
 *   If any memory allocation fails during the computation, the return value
 *   if 0. Otherwise, value of d_result is return.
 */

extern real *
wt1d_real_single (real    *d_data,
		  int     size_of_data,
		  Wavelet *wavelet,
		  real    scale,
		  real    exponent,
		  real    *d_result,
		  int     border_effect,
		  int     *first_exact_ptr,
		  int     *last_exact_ptr);

/*
 * wt1d_cplx_single --
 *
 * Wavelet transform at one scale of a complex signal.
 *
 * Arguments :
 *   d_data        - Array that contains the data of the direct form of the
 *                   signal. d_data must be non 0.
 *   size_of_data  - Number of points of d_data.
 *   wavelet       - A pointer to the wavelet to use.
 *   scale         - Scale of the transform.
 *   exponent      - Exponent of the normalisation (i.e. 1/a^exponent).
 *   d_result      - An allocated array to store the result. Real part are in
 *                   even index of the array and imaginary part are in odd index
 *                   of the array.
 *   border_effect - Indicates the way to handle border effect. Must be
 *                   CV1D_PERIODIC, CV1D_MIRROR or CV1D_PADDING.
 *   first_exact_ptr - (out) First index in array d_result that has no border
 *                     effect.
 *   last_exact_ptr  - (out) Last index in array d_result that has no border
 *                     effect.
 *
 * Return value :
 *   If any memory allocation fails during the computation, the return value
 *   if 0. Otherwise, value of d_result is return.
 */

extern real *
wt1d_cplx_single (complex *d_data,
		  int     size_of_data,
		  Wavelet *wavelet,
		  real    scale,
		  real    exponent,
		  real    *d_result,
		  int     border_effect,
		  int     *first_exact_ptr,
		  int     *last_exact_ptr);

/*
 * wt1d_real_all --
 *
 * Wavelet transform at several scales of a real signal.
 *
 * Arguments :
 *   d_data        - Array that contains the data of the direct form of the
 *                   signal. d_data must be non 0.
 *   size_of_data  - Number of points of d_data.
 *   wavelet       - A pointer to the wavelet to use.
 *   first_scale   - 3 parameters to define all the scales. ... 
 *   nb_of_octave  
 *   nb_of_vox     
 *   exponent      - Exponent of the normalisation (i.e. 1/a^exponent).
 *   d_result      - An allocated array of arrays to store the result. The type
 *                   of the results depends on the form of the filter. If the
 *                   result is complex, real part are in even index of the array
 *                   and imaginary part are in odd index of the array.
 *   border_effect - Indicates the way to handle border effect. Must be
 *                   CV1D_PERIODIC, CV1D_MIRROR or CV1D_PADDING.
 *   first_exact_ptr - (out) First index with no border effect in all d_result
 *                     arrays.
 *   last_exact_ptr  - (out) Last index with no border effect in all d_result
 *                     arrays.
 *
 * Return value :
 *   If any memory allocation fails during the computation, the return value
 *   if 0. Otherwise, value of d_result is return.
 */

extern real **
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
	       int     *last_exact_ptr);

/*
 * wt1d_cplx_all --
 *
 * Wavelet transform at several scales of a complex signal.
 *
 * Arguments :
 *   d_data        - Array that contains the data of the direct form of the
 *                   signal. d_data must be non 0.
 *   size_of_data  - Number of points of d_data.
 *   wavelet       - A pointer to the wavelet to use.
 *   first_scale   - 3 parameters to define all the scales. ... 
 *   nb_of_octave  
 *   nb_of_vox     
 *   exponent      - Exponent of the normalisation (i.e. 1/a^exponent).
 *   d_result      - An allocated array of arrays to store the result. Real part
 *                   are in even index of the array and imaginary part are in
 *                   odd index of the array.
 *   border_effect - Indicates the way to handle border effect. Must be
 *                   CV1D_PERIODIC, CV1D_MIRROR or CV1D_PADDING.
 *   first_exact_ptr - (out) First index with no border effect in all d_result
 *                     arrays.
 *   last_exact_ptr  - (out) Last index with no border effect in all d_result
 *                     arrays.
 *
 * Return value :
 *   If any memory allocation fails during the computation, the return value
 *   if 0. Otherwise, value of d_result is return.
 */

extern real **
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
	       int     *last_exact_ptr);

/*
 * wt1d_cv1d_flt_init_with_wavelet --
 *
 * Call the 'cv1d_flt_init_a' function from the cv1d library with the parameters of
 * a wavelet.
 *
 * Arguments :
 *   Wavelet - The wavelet.
 *   real    - The scale to use.
 *
 * Return value :
 *   None.
 */

extern void
wt1d_cv1d_flt_init_with_wavelet (Wavelet *wavelet,
				 real scale);

/*
 * Pointers to predefined wavelets.
 */

/*
 *  For all the predefined wavelets the domains are defined as [x_min x_max],
 * with :
 *   x_min : the first point that is greater than 0.000001*(max value)
 *   x_max : the last point that is greater than 0.000001*(max value)
 */

/*
 * Gaussian : exp(-x*x/2)
 */

extern Wavelet *wt1d_gaussian_ptr;

/*
 * First derivative of the gaussian : -x*exp(-x*x/2)
 */

extern Wavelet *wt1d_d1_gaussian_ptr;

/*
 * Second derivative of the gaussian : (x*x-1)*exp(-x*x/2)
 */

extern Wavelet *wt1d_d2_gaussian_ptr;

/*
 * Third derivative of the gaussian : x*(3-x*x)*exp(-x*x/2)
 */

extern Wavelet *wt1d_d3_gaussian_ptr;

/*
 * Fourth derivative of the gaussian : (3-6*x*x+x^4)*exp(-x*x/2)
 */

extern Wavelet *wt1d_d4_gaussian_ptr;

/*
 * Morlet wavelet :
 *    cos(omega*x)*exp(-x*x/2) + i * sin(omega*x)*exp(-x*x/2)
 *  with omega = 5.336446
 */

extern Wavelet *wt1d_morlet_ptr;

#endif /* _WT1D_H_ */
