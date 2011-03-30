/*
 * cv2d_int.h --
 *
 *  Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv2d_int.h,v 1.3 1999/05/15 15:45:30 decoster Exp $
 */

#ifndef _CV2D_INT_H_
#define _CV2D_INT_H_

#include <math.h>
#include <stdlib.h>
#include <sys/times.h>
#include <time.h>

#include "cv2d.h"

#ifdef MEM_DEBUG
#include "mem_debug.h"
#endif

/*
 * Include for optionnal messages log. This is used to keep trace of computation
 * times of the different convolution algorithm.
 */

#include "../log/logMsg.h"
#ifdef LOG_MESSAGES
extern char* method_str2[3];
#endif

#include "assert.h"

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
 * The filter structure is private to the cv2d library.
 */

typedef struct _filter_
{
  int    form;			/* Direct and fourier property. */
				/* CV2D_RR_FORM...*/
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
  /*double (* d_real_ptr)();*/	/* Function pointers to filter defs for */
  /*double (* d_imag_ptr)();*/	/* its direct and its fourier forms. Used */
  /*double (* f_real_ptr)();*/	/* for analytical computation. */
  /*double (* f_imag_ptr)();*/
  void * d_real_ptr;
  void * d_imag_ptr;
  void * f_real_ptr;
  void * f_imag_ptr;
  real   scale;			/* Scale to apply to the filter. Used for */
				/* analytical computation.*/
} _filter_;

extern _filter_ flt2;

/*
 * Global variables to handle signal parameters.
 */

extern int  im_form;    /* Direct and fourier property. CV2D_RC_FORM or */
                         /* CV2D_CC_FORM. */
extern void *im_d_data; /* Storage of direct numerical data */
extern void *im_f_data; /* Storage of fourier numerical data */
extern int  im_n;       /* Size of the data arrays */

/*
 * Global variable to store requested convolution method (CV2D_D, CV2D_MP or CV2D_FT)
 */

extern int cv2d_method;

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
 * Internal functions defined in cv2d_misc.c.
 */

extern int       cv2d_next_power_of_2_ (int);
extern int       cv2d_is_power_of_2_   (int);

extern void      cv2d_cplx_mult_            (complex *, complex *, int, real);
extern void      cv2d_cplx_mult_num_ana_    (complex *, void *, void *, int, int, real, real);
extern void      cv2d_cplx_mult_num_ana_1p_ (complex *, void *, void *, double, int, int, real, real);

extern void      cv2d_mult_with_scaled_ft_fct_ (complex *, void *, void *, double, int, int, real, real);

extern int       cv2d_convolution_method_ (int, int, int[LIMITS_TAB_SIZE][2]);

extern real *    cv2d_pure_periodic_extend_      (real *, int, int, int, int, int, int);

extern real *    cv2d_mirror_transform_          (real *, int, int);
extern real *    cv2d_padding_transform_         (real *, int, int);
extern real *    cv2d_0_padding_transform_       (real *, int, int);

extern complex * cv2d_pure_cplx_periodic_extend_ (complex *, int, int, int, int, int, int);

extern complex * cv2d_cplx_mirror_transform_     (complex *, int, int);
extern complex * cv2d_cplx_padding_transform_    (complex *, int, int);
extern complex * cv2d_cplx_0_padding_transform_  (complex *, int, int);

extern void	 cv2d_get_part_r_ (real *, int, int, real *, int, int, int, int, int);

extern void	 cv2d_get_part_c_ (complex *, int, int, complex *, int, int, int, int, int);

extern void	 cv2d_set_f_l_exact_ (int *, int *);

extern void	 cv2d_flt_copy_ (_filter_ *, _filter_ *);

extern double	cv2d_flt_squared_mod_sum_;

void
cv2d_put_part_in_result_r_ (real *result_data,
			    int res_nx, int res_ny,
			    real *part_data,
			    int part_nx, int part_ny,
			    int x, int y,
			    int x_offset, int y_offset,
			    int width, int height);
void
cv2d_put_part_in_result_c_ (complex *result_data,
			    int res_nx, int res_ny,
			    complex *part_data,
			    int part_nx, int part_ny,
			    int x, int y,
			    int x_offset, int y_offset,
			    int width, int height);

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
		int  border_effect);

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
			      int  y_border_2);

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
			      int  y_border_2);

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
		int  border_effect);


/*extern int	cv2d_get_good_part_size (int);*/

#define EX_RAISE(exception) goto exception

extern void *myMalloc(size_t size);
extern void myFree(void *ptr);

/*#define malloc myMalloc
#define free myFree*/

#endif /* _CV2D_INT_H_ */
