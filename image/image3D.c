/*
 * image3D.c --
 *
 *   Copyright 2002 Laboratoire de Physique, ENS Lyon, France
 *  Written by Pierre Kestener.
 *
 *  The author may be reached (Email) at the address
 *      pierre.kestener@ens-lyon.fr
 *
 *  Created on december 10th 2002
 */

#include "image_int.h"

/*
 * im3D_new --
 *
 * Create and initialize a new Image3D.
 */

Image3D *
im3D_new(int lx,
	 int ly,
	 int lz,
	 int size,
	 unsigned int type)
{
  Image3D * new_image3D;

  new_image3D = (Image3D *) malloc(sizeof (Image3D));
  
  if (!new_image3D) {
    return 0;
  }

  new_image3D->lx   = lx;
  new_image3D->ly   = ly;
  new_image3D->lz   = lz;
  new_image3D->min  = 1.0;
  new_image3D->max  = -1.0;
  new_image3D->size = size;
  new_image3D->type = type;
  

  new_image3D->data = (real *) malloc(sizeof (real)*size);

  if (!new_image3D->data) {
    free(new_image3D);
    return 0;
  } 

  return new_image3D;
}


/*
 * im3D_set_0 --
 */

void
im3D_set_0 (Image3D * im)
{
  register int i;
  register real * data;

  data = im->data;
  for (i = 0; i < im->size; i++) {
    data[i]= 0.0;
  }

  return;
}


/*
 * im3D_duplicate --
 */

Image3D *
im3D_duplicate(Image3D * in)
{
  Image3D * out;

  out = im3D_new(in->lx, in->ly, in->lz, in->size, in->type);
  if (!out) {
    return 0;
  }
  out->min = in->min;
  out->max = in->max;
  memcpy(out->data, in->data, out->size*sizeof(real));
  
  return out;
}


/*
 * im3D_free --
 */

void
im3D_free (Image3D * im)
{
  if (im) {
    if (im->data) {
      free(im->data);
    }
    free(im);
  }
  return;
}


/*
 * im3D_get_extrema --
 */

void
im3D_get_extrema (Image3D *im,
		  real  *min_ptr,
		  real  *max_ptr)
{
  register real *data;
  register int  i;
  
  data = im->data;
  *min_ptr = *max_ptr = data[0];
  for (i = 0; i < im->size; i++) {
    if (data[i] > *max_ptr) {
      *max_ptr = data[i];
    } else {
      if (data[i] < *min_ptr) {
	*min_ptr = data[i];
      }
    }
  }
  im->min = *min_ptr;
  im->max = *max_ptr;

  return;
}



