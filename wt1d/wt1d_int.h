/*
 * wt1d_int.h
 *
 * Internal header of the wt1d library.
 *
 *  Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: wt1d_int.h,v 1.4 1998/07/09 13:46:13 decoster Exp $
 */

#ifndef _WT1D_INT_H_
#define _WT1D_INT_H_

#include <math.h>
#include <stdlib.h>
#include "wt1d.h"

#ifdef MEM_DEBUG
#include "mem_debug.h"
#endif

#ifndef _COMPLEX_
#define _COMPLEX_
typedef struct
{
  real real;
  real imag;
} complex;
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <assert.h>

#include "../log/logMsg.h"

#ifndef swap
#define swap(a,b) tmp=(a);(a)=(b);(b)=tmp
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

typedef struct _tab_ Tab;

/*
 * The wavelet structure is private to the wt1d library.
 */

typedef struct _wavelet_
{
  int  type;                  /* Must be one of the types enumerate in        */
                              /* wt1d.h.                                      */
/*   double (* d_r_fct_ptr)();   /\* One of these two function pointers must      *\/ */
/*   double (* d_i_fct_ptr)();   /\* be non-NULL. d_fct_ptr is for the direct     *\/ */
/*   double (* f_r_fct_ptr)();   /\* form of the wavelet, f_fct_ptr is for        *\/ */
/*   double (* f_i_fct_ptr)();   /\* its fourier form. r & i is for the real      *\/ */
  void *d_r_fct_ptr;
  void *d_i_fct_ptr;
  void *f_r_fct_ptr;
  void *f_i_fct_ptr;
                              /* and the imaginary parts of the functions.    */
  real d_x_min, d_x_max;      /* Define the domain of the direct form of the  */
                              /* wavelet.                                     */
  real f_x_min, f_x_max;      /* Define the domain of the fourier form of the */
                              /* wavelet.                                     */
  real time_scale_mult;
  real freq_scale_mult;

  Tab  *d_tab;                /* To store numerical versions of the direct  */
                              /* form of the wavelet.                       */
  Tab  *f_tab;                /* To store numerical versions of the fourier */
                              /* form of the wavelet.                       */
} _wavelet_;

extern real *
get_tab_data (Tab  *tab_ptr,
	      int  *tab_size_ptr,
	      real scale);

#endif /* _WT1D_INT_H_ */
