/*
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster and Stephane Roux.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *  or  decoster@info.enserb.u-bordeaux.fr
 *
 *  Declaration of the stats library.
 */

#ifndef __STATS_H__
#define __STATS_H__

#include <tcl.h>
#include "../signal/signal.h"
#include "../image/image.h"
#include "../image/image3D.h"
#include "../wt2d/wt2d.h"
#include "../wt3d/wt3d.h"
#include "../interpreter/hash_tables.h"

#include <matheval.h>

enum
{
  MOD_STAT,
  ARG_STAT,
  MX_STAT,
  MY_STAT
};

enum
{
  LINE_MAX_STAT,
  VERT_CHAINED_MAX_STAT
};

Signal * stat_signal_histogram (Signal *, int, real, real, Signal * , int);
Signal * stat_imimage_histogram (Image *, int, real, real, Image * , int, Image *);
Signal * stat_imimage3D_histogram (Image3D *, int, real, real);
Signal * stat_extimage_histogram2 (ExtImage *, int, real, real, int);
Signal * stat_imimage_increment_histogram (Image *, int, real, real, int, int, Image *);
Signal * stat_imimage_masklines_histogram (Image *, ExtImage *, int, real, real);

Signal * stat_extimage_histogram (ExtImage *, int, real, real, int, int, Image *);
Signal * stat_extimage_histogram3 (ExtImage *, int, real, real, int, int);

Signal * stat_extimage3D_histogram (Tcl_Interp *, char *, int, int, int);
Signal * stat_extimage3Dsmall_histogram (ExtImage3Dsmall *, int, real, real);

Signal * stat_line_histogram (Line *, int, real, real, int, int, Signal *);

Image * stat_imimage_gradient_histogram(Image *, Image *, int, real, real);
Image * stat_imimage_histogram2D(Image *, real, real, int,
				 Image *, real, real, int);
Image * stat_imimage3D_histogram2D(Image3D *, Image3D *, int, int, real, real, real, real);
Image * stat_imimage3D_gradient_histogram_mask(Image3D *, Image3D *, ExtImage3Dsmall *, int, real, real);
Image * stat_extimage_gradient_histogram (ExtImage *, int, real *, real *, int);
Image * stat_extimage_gradient_histogram3 (ExtImage *, int, real *, real *, int);
Image * stat_line_gradient_histogram (Line *, int, real, real, Image *, int);

Signal * stat_x_line_size_histo (ExtImage *, real, real, int);
Signal * stat_n_line_histo      (ExtImage *, real, real, int);

real *
stat_array_histo(real *data,
		 int  size,
		 real min,
		 real max,
		 real *histo,
		 int  h_size,
		 real *x0,
		 real *dx,
		 /*double (* fct_ptr)(),*/
		 void *fct_ptr,
		 real *weight);

real *
stat_array_histo_mask(real *data,
		      int  size,
		      real min,
		      real max,
		      real *histo,
		      int  h_size,
		      real *x0,
		      real *dx,
		      /*double (* fct_ptr)(),*/
		      void *fct_ptr,
		      real *weight,
		      int image_lx,
		      int posX,
		      int posY,
		      int width,
		      int height);


#endif /* __STATS_H__ */
