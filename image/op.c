/*
 * op.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: op.c,v 1.6 1998/09/18 16:00:15 decoster Exp $
 */

#include "image_int.h"

/*
 */
Image *
im_real_to_complex (Image *source)
{
  Image *result;
  int   src_lx, src_ly;
  int   res_lx, res_ly, dst_size;
  int   dx, dy, x, y;
  
  /* On cree ou recupere du dictionnaire une image result aux dim. voulues */
  src_lx   = source -> lx;
  src_ly   = source -> ly;
  res_lx   = next_power_of_2__ (src_lx);
  res_ly   = next_power_of_2__ (src_ly);
  dst_size = 2* res_lx*res_ly + 2;

  result = im_new (src_lx, src_ly, dst_size, FOURIER);
  if (!result)
    return 0;
  
  /* on recopie source dans result avec un format adapte a _Fourn_ */      
  dx = (int)(res_lx - src_lx) / 2;
  dy = (int)(res_ly - src_ly) / 2;
  
  im_set_0 (result);
  for (x = 0; x < src_lx; x++)
    for (y = 0; y < src_ly; y++)
      result -> data[2*(res_lx*(y + dy) + x + dx) + 1]
	= source -> data[y*src_lx + x];
  
  return result;
}


/*
 */
void
_complex_multiplication_ (real *data1,
			  real *data2,
			  int  size)
{
  register int   i;
  real re1, im1;

  for (i = 1; i <= size; i += 2)
      {
	re1 = data1[i];
	im1 = data1[i+1];
	data1[i] = re1*data2[i] - im1*data2[i+1];
	data1[i+1] = re1*data2[i+1] + im1*data2[i];
      }
}

/*
 */
void
_real_multiplication_ (real *data1,
		       real *data2,
		       int  size)
{
  register int   i;

  for (i = 0; i < size; i++)
    data1[i] = data1[i] * data2[i];
}

/*
 * Multiplication between image1 and image2. Result in image1.
 */
Image *
im_mult (Image *image1,
	 Image *image2)
{
  switch (image1 -> type)
    {
    case FOURIER:
      _complex_multiplication_(image1 -> data, image2 -> data,
			       image1 -> size);
      break;
    case PHYSICAL:
      _real_multiplication_(image1 -> data, image2 -> data,
			    image1 -> size);
      break;
    }

  return image1;
}

/*
 * Scalar multiplication between scalar and image. result in image.
 */
/*Image * im_scalar_mult (Image *image, real scalar)*/
Image *
im_scalar_mult (Image *image,
		real  scalar)
{
  int  i;
  int  size;
  real *data; 

  size = image -> size;
  data = image -> data; 
  for (i = 0; i < size; i++)
    data[i] = scalar*data[i];

  return image;
}

/*
 * Scalar addition between scalar and image. result in image.
 */
Image *
im_scalar_add (Image *image,
	       real  scalar)
{
  int i;
  int size;
  real *data; 

  size = image -> size;
  data = image -> data; 
  for (i = 0; i < size; i++)
    data[i] = scalar + data[i];

  return image;
}

/*
 * Addition between image1 and image2. Result in image1. 
 */
Image *
im_add (Image *image1,
	Image *image2)
{
  int i;
  int size = image1 -> size;
  real *data1 = image1 -> data; 
  real *data2 = image2 -> data; 

  for (i = 0; i < size; i++)
    data1[i] = data1[i] + data2[i];

  return image1;
}

/*
 * Maximum of pixels values in image1 and image2. Result in image1. 
 */
Image *
im_max (Image *image1,
	Image *image2)
{
  int i;
  int size = image1 -> size;
  real *data1 = image1 -> data; 
  real *data2 = image2 -> data; 

  for (i = 0; i < size; i++)
    data1[i] = max(data1[i], data2[i]);

  return image1;
}


/*
 * Set to 0.0 every point wich absolute value is under a given value.
 */
Image *
im_filter (Image *image,
	   real  value)
{
  int  i;
  int  size;
  real *data; 

  if (!value)
    value = SEUIL;

  size = image -> size;
  data = image -> data; 
  for (i = 0; i < size; i++)
      if (data[i] < value)
	data[i] = 0.0;

  return image;
}

/*
 * Convolution between image1 and image2.
 */
Image *
im_conv (Image *image1,
	 Image *image2)
{
  Image *result;
  int x1, y1, x2, y2;
  int i, size;
  real *data1;
  real *data2;
  real *res_data;

  if (image1 -> type != image2 -> type)
/*    return GenErrorAppend(interp,"Images have not the same type.",NULL);*/
    return 0;

 /* if (image1 -> size < image2 -> size)
    swap(image1, image2);*/
  data1 = image1 -> data;
  data2 = image2 -> data;
  size = image1 -> size;

  result = im_new (image1 -> lx, image1 -> ly, size, PHYSICAL);

  if (!result)
/*    GenErrorMemoryAlloc(interp);*/
    return 0;

  res_data = result -> data;

  for(i = 0; i < size; i++)
    res_data[i] = 0.0;

  for (x1 = 0; x1 < image1 -> lx; i++)
    for (y1 = 0; y1 < image1 -> ly; i++)
      for (x2 = 0; x2 < inf(image2 -> lx, image1 -> lx-x1); i++)
	for (y2 = 0; y2 < inf(image2 -> ly, image1 -> ly-y1); i++)
	  res_data[x1+y1*image1 -> lx] += (data1[x1+x2+(y1+y2)*image1 -> lx]
					 * data2[x2+y2*image2 -> lx]);

  return result;
}

/* a mettre ailleur (?) */
#define EPSILON 0.000001
#define NO_ARG 10

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
Image *
im_xy_to_mod (Image *x_image,
	      Image *y_image)
{
  int lx,ly,i;
  Image * image;

  lx = x_image -> lx;
  ly = x_image  -> ly;
  
  image = im_new (lx, ly, lx*ly, PHYSICAL);

  if (!image)
    return 0;

  for (i = 0; i < lx*ly; i++)
    image -> data[i] = sqrt (x_image -> data[i]*x_image -> data[i]
			     + y_image -> data[i]*y_image -> data[i]);

  return image;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
Image *
im_xy_to_arg (Image *x_image,
	      Image *y_image,
	      real  thresh)
{ 
  int lx,ly,i;
  real dx,dy;
  real minx, maxx, miny, maxy;
  real epsilon;
  Image * image;
  
  (void) thresh;

  lx = x_image -> lx;
  ly = x_image  -> ly;
  
  image = im_new (lx, ly, lx*ly, PHYSICAL);
  if (!image)
/*    return GenErrorMemoryAlloc(interp);*/
    return 0;
  
  im_get_extrema (x_image, &minx, &maxx);
  im_get_extrema (y_image, &miny, &maxy);
  epsilon = EPSILON;
/*((maxx-minx)*(maxx-minx)+(maxy-miny)*(maxy-miny))*thresh;*/
  for (i=0;i<lx*ly;i++)
    {
      dx = x_image->data[i];
      dy = y_image->data[i];
      if ((dx*dx+dy*dy)>epsilon*epsilon) 
	image->data[i]=atan2(dy,dx);
      else
	image->data[i] = NO_ARG;
    }

  return image;
}

/*
 */
Image *
im_cut_edge (Image *src_image,
	     int   cut_size)
{ 
  Image *dst_image;
  int   x, y, dim;

  dim = src_image->lx-2*cut_size;
  dst_image=im_new(dim,dim,dim*dim,PHYSICAL);
  if (!dst_image)
/*    return GenErrorMemoryAlloc(interp);*/
    return 0;

  im_set_0 (dst_image);

  for(x = cut_size; x < (src_image->lx-cut_size); x++)
    for(y = cut_size; y < (src_image->lx-cut_size); y++) {
      dst_image->data[x-cut_size+dim*(y-cut_size)] =
	                                   src_image->data[x+y*src_image->lx];
    }

  return dst_image;
}

/*
 */
Image *
im_insert (Image *image1,
	   Image *image2,
	   int   x_pos,
	   int   y_pos)
{
  int  x, y;
  int  lx1, ly1, lx2, ly2;
  real *data1, *data2;

  lx1 = image1 ->lx;
  ly1 = image1 ->ly;
  data1 = image1 -> data;
  lx2 = image2 ->lx;
  ly2 = image2 ->ly;
  data2 = image2 -> data;

  for (x = 0; x < lx2; x++)
    for (y = 0; y < ly2; y++)
      if (((x + x_pos) < lx1) && ((y + y_pos) < ly1))
	data1[(x+x_pos) + (y+y_pos)*lx1] = data2[x + y*lx2];

  return image1;
}

/*
 * Set all the points outside a box to a specific VALUE.
 */
Image *
im_set_border (Image *image,
	       int   x_min,
	       int   y_min,
	       int   x_max,
	       int   y_max,
	       real  value)
{ 
  int   x, y;

  for(x = 0; x < image -> lx; x++)
    for(y = 0; y < image -> ly; y++)
      if ((x < x_min)
	  || (x > x_max)
	  || (y < y_min)
	  || (y > y_max))
	image->data[x + y*image -> lx] = value;

  return image;
}


/*
 *  This function transform an image in the gfft format to 2 images, one for the
 * real part and one for the imaginary part. RES_REAL and RES_IMAG must have
 * been allocated with the same parameters than SOURCE.
 */
void
im_gfft_to_2real (Image *source,
		  Image *res_real,
		  Image *res_imag)
{
  complex *sdata;
  real *rdata;
  real *idata;
  int lx;
  int ly;
  int x;
  int y;
  int i;
  int j;

  /*  Remenber, the data of a gfft image must be treat as complex data. Even
   * if its type is still PHYSICAL. */
  sdata = (complex *) source->data;
  rdata = res_real->data;
  idata = res_imag->data;
  lx = res_real->lx;
  ly = res_real->ly;

  /*  The first half of the first column of SOURCE gives the first column of
   * RES_REAL and the first column of RES_IMAG. */
  rdata[0] = sdata[0].real;
  idata[0] = 0.0;
  for (y = 1; y < ly/2; y++) {
      i = y*lx/2;
      j = y*lx;
      rdata[j] = sdata[i].real;
      idata[j] = sdata[i].imag;
      j = (ly-y)*lx;
      rdata[j] = sdata[i].real;
      idata[j] = -sdata[i].imag;
  }
  rdata[(ly/2)*lx] = sdata[0].imag;
  idata[(ly/2)*lx] = 0.0;

  /*  The second half of the first column of SOURCE gives the middle column of
   * RES_REAL and the middle column of RES_IMAG. */
  rdata[lx/2] = sdata[(ly/2)*(lx/2)].real;
  idata[lx/2] = 0.0;
  for (y = 1; y < ly/2; y++) {
      i = (y+ly/2)*lx/2;
      j = lx/2+y*lx;
      rdata[j] = sdata[i].real;
      idata[j] = sdata[i].imag;
      j = lx/2+(ly-y)*lx;
      rdata[j] = sdata[i].real;
      idata[j] = -sdata[i].imag;
  }
  rdata[lx/2+(ly/2)*lx] = sdata[(ly/2)*(lx/2)].imag;
  idata[lx/2+(ly/2)*lx] = 0.0;

  /*  Copy the left data in place and with the "parity shift". */
  for (y = 0; y < ly; y++) {
    for (x = 1; x < lx/2; x++) {
      i = x + y*lx/2;
      j = x + y*lx;
      rdata[j] = sdata[i].real;
      idata[j] = sdata[i].imag;
      if (y == 0) {
	j = lx - x + y*lx;
	rdata[j] = sdata[i].real;
	idata[j] = -sdata[i].imag;
      } else {
	j = lx - x + (ly - y)*lx;
	rdata[j] = sdata[i].real;
	idata[j] = -sdata[i].imag;
      }
    }
  }
}


Image *
im_thresh (Image *image,
	   real  xmin,
	   real  xmax,
	   real  value)
{
  int  i;
  int  size;
  real *data;

  size = image->size;
  data = image->data; 
  for (i = 0; i < size; i++) {
    if (data[i] < xmin || data[i] > xmax) {
      data[i] = value;
    }
  }

  return image;
}

