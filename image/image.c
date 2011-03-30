/*
 * image.c --
 *
 *   Copyright (c) 1999 Nicolas Decoster
 *   Copyright (c) 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *
 *   Copyright (c) 1999-2007 Pierre Kestener.
 *   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
 *   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
 *   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
 */

#include "image_int.h"

/*
 * im_new --
 *
 * Create and initialize a new Image.
 */

Image *
im_new(int lx,
       int ly,
       int size,
       int type)
{
  Image * new_image;

  new_image = (Image *) malloc(sizeof (Image));
  
  if (!new_image) {
    return 0;
  }

  new_image->lx   = lx;
  new_image->ly   = ly;
  new_image->min  = 1.0;
  new_image->max  = -1.0;
  new_image->avg  = 0.0;
  new_image->size = size;
  new_image->type = type;
  new_image->border_hor = 0;
  new_image->border_ver = 0;

  new_image->data = (real *) malloc(sizeof (real)*size);

  if (!new_image->data) {
    free(new_image);
    return 0;
  } 

  return new_image;
}


/*
 * im_set_0 --
 */

void
im_set_0 (Image * im)
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
 * im_duplicate --
 */

Image *
im_duplicate(Image * in)
{
  Image * out;

  out = im_new(in->lx, in->ly, in->size, in->type);
  if (!out) {
    return 0;
  }
  out->min = in->min;
  out->max = in->max;
  out->avg = in->avg;
  out->border_hor = in->border_hor;
  out->border_ver = in->border_ver;
  memcpy(out->data, in->data, out->size*sizeof(real));
  
  return out;
}


/*
 * im_free --
 */

void
im_free (Image * im)
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
 * im_get_extrema --
 */

void
im_get_extrema (Image *im,
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

/*
 * im_get_average --
   added by A. Khalil 2006 06 14
 */

void
im_get_average (Image *im,
		real  *avg_ptr)
{
  register real *data;
  register int  i;
  
  data = im->data;
  *avg_ptr = 0;
  for (i = 0; i < im->size; i++) {
      *avg_ptr = *avg_ptr + data[i]/(im->size);
  }
  im->avg = *avg_ptr;
  return;
}
/*
 * im_get_extrema_increment --
 */
void
im_get_extrema_increment (Image *im,
			  real  *min_ptr,
			  real  *max_ptr,
			  int    direction,
			  int    d,
			  Image *mask)
{
  register real *data, *maskData;
  register int  i,j, index1, index2;
  int lx,ly;
  
  data = im->data;
  lx   = im->lx;
  ly   = im->ly;
  maskData = mask->data;

  *min_ptr = *max_ptr = data[0]-data[d];
  
  if (!mask) {
    if (direction) {
      
      for (i = 0; i < lx; i++)
	for (j = 0; j < ly-d; j++) {
	  index1 = i + j*lx;
	  index2 = i + (j+d)*lx;
	  
	  if (data[index1]-data[index2] > *max_ptr) {
	    
	    *max_ptr = data[index1]-data[index2];
	    
	  } else if (data[index1]-data[index2] < *min_ptr) {
	    
	    *min_ptr = data[index1]-data[index2];
	    
	  }
	
	}
    
    } else { /* direction = 0 */
      
      for (i = 0; i < lx-d; i++)
	for (j = 0; j < ly; j++) {
	  index1 = i + j*lx;
	  index2 = i+d + j*lx;
	  
	  if (data[index1]-data[index2] > *max_ptr) {
	    
	    *max_ptr = data[index1]-data[index2];
	    
	  } else if (data[index1]-data[index2] < *min_ptr) {
	    
	    *min_ptr = data[index1]-data[index2];
	    
	  }
	  
	}
      
    }
  } else { /* mask != NULL */

    if (direction) {
      
      for (i = 0; i < lx; i++)
	for (j = 0; j < ly-d; j++) {
	  index1 = i + j*lx;
	  index2 = i + (j+d)*lx;
	  
	  if (maskData[index1] <= 0.0 || maskData[index2] <= 0.0)
	    continue;

	  if (data[index1]-data[index2] > *max_ptr) {
	    
	    *max_ptr = data[index1]-data[index2];
	    
	  } else if (data[index1]-data[index2] < *min_ptr) {
	    
	    *min_ptr = data[index1]-data[index2];
	    
	  }
	
	}
    
    } else { /* direction = 0 */
      
      for (i = 0; i < lx-d; i++)
	for (j = 0; j < ly; j++) {
	  index1 = i + j*lx;
	  index2 = i+d + j*lx;
	  
	  if (maskData[index1] <= 0.0 || maskData[index2] <= 0.0)
	    continue;

	  if (data[index1]-data[index2] > *max_ptr) {
	    
	    *max_ptr = data[index1]-data[index2];
	    
	  } else if (data[index1]-data[index2] < *min_ptr) {
	    
	    *min_ptr = data[index1]-data[index2];
	    
	  }
	  
	}
      
    }

  }
    
  return;
}

/* The following Routines are taken from Lastwave
   Copyright (C) 2000 Emmanuel Bacry.
   email : lastwave@cmap.polytechnique.fr
*/
/* ***********************/
/* Random Uniform number */ 
/* ***********************/

# define MBIG 1000000000
# define MSEED 161803398
# define MZ 0
# define FAC (1.0/MBIG)

float Urand(void)
{
  static int inext,inextp;
  static long ma[56];
  static int iff=0;
  static long int idum = -1;
  register long mj,mk;
  register int i,ii,k;

  if ( idum <0 || iff == 0){
     iff = 1;
     if (idum < 0) idum = -time(NULL);
     mj = MSEED - ( idum < 0 ? -idum : idum); 
     mj %= MBIG;                         
     ma[55] = mj;
     mk = 1;
     for ( i=1; i<=54; i++){
       ii = ( 21*i) % 55;
       ma[ii] = mk;
       mk = mj - mk;
       if ( mk < MZ ) mk += MBIG;
       mj = ma[ii];
     }
     for ( k=1; k<=4; k++)
       for (i=1; i<=55; i++){
	 ma[i] -= ma[1 + (i+30) % 55];
	 if ( ma[i] < MZ ) ma[i] += MBIG;
       }
     inext = 0;
     inextp = 31;
     idum = 1;
   }
   if( ++inext == 56 )inext=1;
   if( ++inextp == 56 )inextp=1;
   mj = ma[inext] - ma[inextp];
   if( mj < MZ) mj += MBIG;
   ma[inext] = mj;
   return mj*FAC;
}


/* ************************/
/* Random Gaussian number */ 
/* ************************/

float Grand(float sigma)
{
   static int iset=0;
   static float gset;
   float fac,rsq,v1,v2;

   if (iset == 0) {
    do {
         v1 = 2.0*Urand()-1.0;
         v2 = 2.0*Urand()-1.0;
         rsq=v1*v1+v2*v2;
        }
    while (rsq >= 1.0 || rsq == 0.0);
  fac=sqrt(-2.0*log(rsq)/rsq);
  gset = v1*fac;
  iset=1;
  return(v2*fac*sigma);
 }
 else {
  iset = 0;
  return(sigma*gset);
 }
}
