/*
 * cv1d_int.h --
 *
 *  Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv1d_int.h,v 1.14 1999/05/06 09:38:45 decoster Exp $
 */

#ifndef _CV1D_INT_H_
#define _CV1D_INT_H_

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <time.h>

#include "cv1d.h"

#ifdef MEM_DEBUG
#include "mem_debug.h"
#endif

/*
 * Include for optionnal messages log. This is used to keep trace of computation
 * times of the different convolution algorithm.
 */

#include "../log/logMsg.h"
#ifdef LOG_MESSAGES
extern char* method_str[3];
#endif

#include <assert.h>

#ifndef swap
#define swap(a,b) tmp=(a);(a)=(b);(b)=tmp
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

/*
 * Global variables that handle the filter parameters.
 */

enum {
  NUMERICAL,
  ANALYTICAL
};

enum {
  REAL,
  CPLX
};

/*
 * The filter structure is private to the cv1d library.
 */

typedef struct _filter_
{
  int    form;			/* Direct and fourier property. */
				/* CV1D_RR_FORM...*/
  int    def;			/* Kind of definition : NUMERICAL or */
				/* ANALYTICAL.*/
  void * d_data;		/* Storage of direct numerical data */
  void * f_data;		/* Storage of fourier numerical data */
  int    d_n;			/* Size of the direct data arrays */
  real   d_begin;		/* Domain of the direct form. Used for */
  real   d_end;			/* analytical computation.*/
  int    f_n;			/* Size of the fourier data arrays */
  real   f_begin;		/* Domain of the fourier form. Used for */
  real   f_end;			/* analytical computation.*/
/*   double (* d_real_ptr)();	/\* Function pointers to filter defs for *\/ */
/*   double (* d_imag_ptr)();	/\* its direct and its fourier forms. Used *\/ */
/*   double (* f_real_ptr)();	/\* for analytical computation. *\/ */
/*   double (* f_imag_ptr)(); */
  void *d_real_ptr;
  void *d_imag_ptr;
  void *f_real_ptr;
  void *f_imag_ptr;

  real   scale;			/* Scale to apply to the filter. Used for */
				/* analytical computation.*/
} _filter_;

extern _filter_ flt;

/*
 * Global variables to handle signal parameters.
 */

extern int  sig_form;    /* Direct and fourier property. CV1D_RC_FORM or */
                         /* CV1D_CC_FORM. */
extern void *sig_d_data; /* Storage of direct numerical data */
extern void *sig_f_data; /* Storage of fourier numerical data */
extern int  sig_n;       /* Size of the data arrays */

/*
 * Global variable to store requested convolution method (CV1D_D, CV1D_MP or CV1D_FT)
 */

extern int cv1d_method;

enum {
  DIRECT_CONVOLUTION,
  MULTI_PART_CONVOLUTION,
  FOURIER_TRANSFORM_CONVOLUTION
};

enum {
  DIRECT_LIM,
  MULTI_PART_LIM
};

#define LIMITS_TAB_SIZE 18

/*
 * Internal functions defined in cv1d_misc.c.
 */

extern int       cv1d_next_power_of_2_ (int);
extern int       cv1d_is_power_of_2_   (int);

extern void      cv1d_cplx_mult_            (complex *, complex *, int, int);
/*extern void      cv1d_cplx_mult_num_ana_    (complex *, double (*)(), double (*)(), int, int, real, real);*/
extern void      cv1d_cplx_mult_num_ana_    (complex *, void*, void*, int, int, real, real);
/*extern void      cv1d_cplx_mult_num_ana_1p_ (complex *, double (*)(), double (*)(), double, int, int, real, real);*/
extern void      cv1d_cplx_mult_num_ana_1p_ (complex *, void*, void*, double, int, int, real, real);

/*extern void      cv1d_mult_with_scaled_ft_fct_ (complex *, double (*)(), double (*)(), double, int, int, real, real);*/
extern void      cv1d_mult_with_scaled_ft_fct_ (complex *, void*, void*, double, int, int, real, real);

extern int       cv1d_convolution_method_ (int, int, int[LIMITS_TAB_SIZE][2]);

extern real *    cv1d_pure_periodic_extend_      (real *, int, int, int);
extern real *    cv1d_mirror_transform_          (real *, int, int);
extern real *    cv1d_padding_transform_         (real *, int, int);
extern real *    cv1d_0_padding_transform_       (real *, int, int);
extern complex * cv1d_pure_cplx_periodic_extend_ (complex *, int, int, int);
extern complex * cv1d_cplx_mirror_transform_     (complex *, int, int);
extern complex * cv1d_cplx_padding_transform_    (complex *, int, int);
extern complex * cv1d_cplx_0_padding_transform_  (complex *, int, int);

extern void	 cv1d_get_part_r_ (real *, int, real *, int, int, int);
extern void	 cv1d_get_part_c_ (complex *, int, complex *, int, int, int);

extern void	 cv1d_set_f_l_exact_ (int *, int *);

extern void	 cv1d_flt_copy_ (_filter_ *, _filter_ *);

extern double	flt_squared_mod_sum_;

/*extern int	cv1d_get_good_part_size (int);*/

#define EX_RAISE(exception) goto exception

#endif /* _CV1D_INT_H_ */
