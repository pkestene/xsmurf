/*
 * misc.c --
 *
 *
 */

#include "misc.h"

/* Compute the gradient modulus in 2-D.
 *
 * gradient_modulus = sqrt (
 * derivative_along_X*derivative_along_X +
 * derivative_along_Y*derivative_along_Y ).
 */

void GradientModulus2D( float *gradient_modulus,
			float *derivative_along_X,
			float *derivative_along_Y,
			int length )
{
  register int i;
  register float *norm = gradient_modulus;
  register float *gx = derivative_along_X;
  register float *gy = derivative_along_Y;
  
  for ( i=0; i<length; i++, norm++, gx++, gy++ )
    *norm = sqrt( (*gx)*(*gx) + (*gy)*(*gy) );
}

void GradientArgument2D( float *gradient_argument,
			 float *derivative_along_X,
			 float *derivative_along_Y,
			 int length )
{
  register int i;
  register float *argu = gradient_argument;
  register float *gx = derivative_along_X;
  register float *gy = derivative_along_Y;
  
  for ( i=0; i<length; i++, argu++, gx++, gy++ ) {
    if (((*gx)*(*gx) + (*gy)*(*gy))>0.000001*0.000001)
      *argu = (float) atan2( (*gy), (*gx) );
    else
      *argu = 10.0;
  }
}

void GradientModulus2D_tensor2D( float *gradient_modulus,
				 float *gradient_argument,
				 float *derivative_along_X1,
				 float *derivative_along_Y1,
				 float *derivative_along_X2,
				 float *derivative_along_Y2,
				 int length,
				 int type)
{
  register int i;
  register float *norme = gradient_modulus;
  register float *argu  = gradient_argument;
  register float *gx1 = derivative_along_X1;
  register float *gy1 = derivative_along_Y1;
  register float *gx2 = derivative_along_X2;
  register float *gy2 = derivative_along_Y2;

  register int index_max_vp;

  /* some variable for SVD computations */
  float **wt_tensor;
  float **v;
  float  *w;
  double *rv1;

  /* init full 2D : allocation */
  /*init_full_2D(wt_tensor, v, w, rv1);*/
  {
    int z;
    /* alloc wt_tensor */
    wt_tensor = (float **) malloc(2 * sizeof(float *));
    for (z=0; z<2; z++)
      wt_tensor[z] = (float *) malloc(2 * sizeof(float));
    
    v = (float **) malloc(2 * sizeof(float *));
    for (z=0; z<2; z++)
      v[z] = (float *) malloc(2 * sizeof(float));
    
    w = (float *) malloc(2 * sizeof(float));
    rv1 = (double *) malloc(2 * sizeof(double));
  }

  for ( i=0; i<length; i++, norme++, argu++, gx1++, gy1++, gx2++, gy2++ ) {
    /* aim is fill norm (specral radius of wt_tensor) */
    /* fill wt_tensor */
    wt_tensor[0][0] = *gx1;
    wt_tensor[0][1] = *gx2;
    wt_tensor[1][0] = *gy1;
    wt_tensor[1][1] = *gy2;
    dsvd(wt_tensor,2,2,w,v,rv1);
    
    /* now wt_tensor contains the matrix U of the singular value decomposition
       wt_tensor(old) = wt_tensor * W * V' */

    /* find max_vp = spectral radius ! */
    /*
      if (w[0]>w[1]) {
      *norme = w[0];
      index_max_vp = 0;
      } else {
      *norme = w[1];
      index_max_vp = 1;
      }
    */
    if (type == SVD_TYPE_MAX) {
      if (w[0]>w[1]) {
	*norme = w[0];
	index_max_vp = 0;
      } else {
	*norme = w[1];
	index_max_vp = 1;
      }
    } else if (type == SVD_TYPE_MIN) {
      if (w[0]<w[1]) {
	*norme = w[0];
	index_max_vp = 0;
      } else {
	*norme = w[1];
	index_max_vp = 1;
      }
    }

    /* THE Wavelet Transform !!! */
    *gx1 = (*norme)*wt_tensor[0][index_max_vp];
    *gy1 = (*norme)*wt_tensor[1][index_max_vp];
    
    if ((*norme)>0.000001)
      *argu = (float) atan2(*gy1,*gx1);
    else
      *argu = 0.0;
  }

  /* close full 2D : free memory */
  /*close_full_2D(wt_tensor, v, w, rv1);*/
  {
    int z;
    for (z=0; z<2; z++) {
      free((void*) wt_tensor[z]);
      free((void*) v[z]);
    }
    free(w);
    free(rv1);
  }
  
}

/* void GradientModulus2D_tensor2D_vector( fftw_real *derivative_along_X1, */
/* 					fftw_real *derivative_along_Y1, */
/* 					fftw_real *derivative_along_X2, */
/* 					fftw_real *derivative_along_Y2, */
/* 					int length, */
/* 					int type, */
/* 					int isSvbulk) */
/* { */
/*   register int i; */
/*   register fftw_real *gx1 = derivative_along_X1; */
/*   register fftw_real *gy1 = derivative_along_Y1; */
/*   register fftw_real *gx2 = derivative_along_X2; */
/*   register fftw_real *gy2 = derivative_along_Y2; */
/*   register fftw_real zenorme; */

/*   register int index_max_vp,index_min_vp; */
/*   register fftw_real min; */

/*   for ( i=0; i<length; i++, gx1++, gy1++, gx2++, gy2++ ) { */
/*     /\* aim is fill norm (specral radius of wt_tensor) *\/ */
/*     /\* fill wt_tensor *\/ */
/*     wt_tensor[0][0] = *gx1; */
/*     wt_tensor[0][1] = *gx2; */
/*     wt_tensor[1][0] = *gy1; */
/*     wt_tensor[1][1] = *gy2; */
/*     dsvd(wt_tensor,2,2,w,v,rv1); */
    
/*     /\* now wt_tensor contains the matrix U of the singular value decomposition */
/*        wt_tensor(old) = wt_tensor * W * V' *\/ */

/*     /\* find max_vp = spectral radius ! *\/ */
/*     if (w[0]>w[1]) { /\* max singular value *\/ */
/*       zenorme = w[0]; */
/*       index_max_vp = 0; */
/*       min = w[1]; */
/*       index_min_vp = 1; */
/*     } else { */
/*       zenorme = w[1]; */
/*       index_max_vp = 1; */
/*       min = w[0]; */
/*       index_min_vp = 0; */
/*     } */

/*     /\* THE Wavelet Transform !!! *\/ */
/*     if (isSvbulk) { /\* output ordered singular values *\/ */
/*       *gx1 = zenorme; */
/*       *gy1 = min; */
/*     } else { */
/*       if (type == 1) { /\* max singular value *\/ */
/* 	*gx1 = (zenorme)*wt_tensor[0][index_max_vp]; */
/* 	*gy1 = (zenorme)*wt_tensor[1][index_max_vp]; */
/* 	*gy2 = (zenorme)*wt_tensor[0][index_max_vp]; */
/*       } else { /\* min singular value *\/       */
/* 	*gx1 = (min)*wt_tensor[0][index_min_vp]; */
/* 	*gy1 = (min)*wt_tensor[1][index_min_vp]; */
/* 	*gy2 = (min)*wt_tensor[0][index_min_vp]; */
/*       } */
/*     } */
    
/*   } */
/* } */

/* void Gradient_tensor2D_det( fftw_real *gradient_det, */
/* 			    fftw_real *derivative_along_X1, */
/* 			    fftw_real *derivative_along_Y1, */
/* 			    fftw_real *derivative_along_X2, */
/* 			    fftw_real *derivative_along_Y2, */
/* 			    int length) */
/* { */
/*   register int i; */
/*   register fftw_real *det = gradient_det; */
/*   register fftw_real *gx1 = derivative_along_X1; */
/*   register fftw_real *gy1 = derivative_along_Y1; */
/*   register fftw_real *gx2 = derivative_along_X2; */
/*   register fftw_real *gy2 = derivative_along_Y2; */
  
  
/*   for ( i=0; i<length; i++, det++, gx1++, gy1++, gx2++, gy2++ ) { */
/*     /\* aim is fill det (determinant of wt_tensor) *\/ */
/*     *det = (*gx1) * (*gy2) - (*gy1) * (*gx2); */
/*     //fprintf(stdout, "det %f\n", *det); */
/*   } */

/* } */


void GradientModulus3D( float *gradient_modulus,
			float *derivative_along_X,
			float *derivative_along_Y,
			float *derivative_along_Z,
			int length )
{
  register int i;
  register float *norme = gradient_modulus;
  register float *gx = derivative_along_X;
  register float *gy = derivative_along_Y;
  register float *gz = derivative_along_Z;
  
  for ( i=0; i<length; i++, norme++, gx++, gy++, gz++ )
    *norme = (float) sqrt( (*gx)*(*gx) + (*gy)*(*gy) + (*gz)*(*gz) );
}

/* void GradientModulus3D_L( fftw_real *gradient_modulus, */
/* 			  fftw_real *derivative_along_X, */
/* 			  fftw_real *derivative_along_Y, */
/* 			  fftw_real *derivative_along_Z, */
/* 			  int length, */
/* 			  int type) */
/* { */
/*   register int i; */
/*   register fftw_real *norme = gradient_modulus; */
/*   register fftw_real *gx = derivative_along_X; */
/*   register fftw_real *gy = derivative_along_Y; */
/*   register fftw_real *gz = derivative_along_Z; */
  
/*   if (type == 0) { */
/*     for ( i=0; i<length; i++, norme++, gx++, gy++, gz++ ) */
/*       *norme = fabs((*gx)); */
/*   } else if (type == 1) { */
/*     for ( i=0; i<length; i++, norme++, gx++, gy++, gz++ ) */
/*       *norme = fabs((*gy)); */
/*   } else { */
/*     for ( i=0; i<length; i++, norme++, gx++, gy++, gz++ ) */
/*       *norme = fabs((*gz)); */
/*   } */
/* } */

/* void GradientModulus3D_T( fftw_real *gradient_modulus, */
/* 			  fftw_real *derivative_along_X, */
/* 			  fftw_real *derivative_along_Y, */
/* 			  fftw_real *derivative_along_Z, */
/* 			  int length, */
/* 			  int type) */
/* { */
/*   register int i; */
/*   register fftw_real *norme = gradient_modulus; */
/*   register fftw_real *gx = derivative_along_X; */
/*   register fftw_real *gy = derivative_along_Y; */
/*   register fftw_real *gz = derivative_along_Z; */
  
/*   if (type == 0) { */
/*     for ( i=0; i<length; i++, norme++, gx++, gy++, gz++ ) */
/*       *norme = sqrt( (*gy)*(*gy) + (*gz)*(*gz) ); */
/*   } else if (type == 1) { */
/*     for ( i=0; i<length; i++, norme++, gx++, gy++, gz++ ) */
/*       *norme = sqrt( (*gx)*(*gx) + (*gz)*(*gz) ); */
/*   } else { */
/*     for ( i=0; i<length; i++, norme++, gx++, gy++, gz++ ) */
/*       *norme = sqrt( (*gx)*(*gx) + (*gy)*(*gy) ); */
/*   } */
/* } */

/*
 * take care modification (july 18th 2003)
 * new input parameter : type -> to specify if you want min or max
 * of the singular values
 */
/* gradient modulus is computed  as the spectral radius of the 
   wavelet transform 3x3 tensor (matrix M)
   and gx1, gy1, gz1 are modified and will contain the components
   of the vector defined as followed :
   let M = U.W.V' be the singular value decomposition of M,
   W is a diagonal matrix, the maximun of its coefficients is the 
   spectral radius...
   let's consider the colums of the matrix M.V, the norms of these vectors
   are the eigenvalues, one choose the column that correspond to the spectral
   radius...
  */
void GradientModulus3D_tensor3D( float *gradient_modulus,
				 float *derivative_along_X1,
				 float *derivative_along_Y1,
				 float *derivative_along_Z1,
				 float *derivative_along_X2,
				 float *derivative_along_Y2,
				 float *derivative_along_Z2,
				 float *derivative_along_X3,
				 float *derivative_along_Y3,
				 float *derivative_along_Z3,
				 int length,
				 int type,
				 int is_trace)
{
  register int i;
  register float *norme = gradient_modulus;
  register float *gx1 = derivative_along_X1;
  register float *gy1 = derivative_along_Y1;
  register float *gz1 = derivative_along_Z1;
  register float *gx2 = derivative_along_X2;
  register float *gy2 = derivative_along_Y2;
  register float *gz2 = derivative_along_Z2;
  register float *gx3 = derivative_along_X3;
  register float *gy3 = derivative_along_Y3;
  register float *gz3 = derivative_along_Z3;

  register int index_max_vp,index_min_vp;
  register float min;

  register float trace;

  /* some variable for SVD computations */
  float **wt_tensor;
  float **v;
  float  *w;
  double *rv1;

  /* init full 3D : allocation */
  /* static void init_full_3D() */
  {
    int z;
    /* alloc wt_tensor */
    wt_tensor = (float **) malloc(3 * sizeof(float *));
    for (z=0; z<3; z++)
      wt_tensor[z] = (float *) malloc(3 * sizeof(float));
    
    v = (float **) malloc(3 * sizeof(float *));
    for (z=0; z<3; z++)
      v[z] = (float *) malloc(3 * sizeof(float));
    
    w = (float *) malloc(3 * sizeof(float));
    rv1 = (double *) malloc(3 * sizeof(double));  
  }

  
  for ( i=0; i<length; i++, norme++, gx1++, gy1++, gz1++, gx2++, gy2++, gz2++, gx3++, gy3++, gz3++ ) {
    /* aim is fill norm (specral radius of wt_tensor) */
    /* fill wt_tensor */
    wt_tensor[0][0] = *gx1;
    wt_tensor[0][1] = *gx2;
    wt_tensor[0][2] = *gx3;
    wt_tensor[1][0] = *gy1;
    wt_tensor[1][1] = *gy2;
    wt_tensor[1][2] = *gy3;
    wt_tensor[2][0] = *gz1;
    wt_tensor[2][1] = *gz2;
    wt_tensor[2][2] = *gz3;
    dsvd(wt_tensor,3,3,w,v,rv1);
    
    /* now wt_tensor contains the matrix U of the singular value decomposition
       wt_tensor(old) = wt_tensor * W * V' */

    /* find max_vp = spectral radius ! */
    if (type == 1) {
      if (w[0]>w[1]) {
	*norme = w[0];
	index_max_vp = 0;
	min = w[1];
	index_min_vp = 1;
      } else {
	*norme = w[1];
	index_max_vp = 1;
	min = w[0];
	index_min_vp = 0;
      }
      if (w[2]>(*norme)) {
	*norme = w[2];
	index_max_vp = 2;
      }
      if (w[2]<min) {
	min =w[2];
	index_min_vp = 2;
      }
    } else {
      if (w[0]<w[1]) {
	*norme = w[0];
	index_max_vp = 0;
      } else {
	*norme = w[1];
	index_max_vp = 1;
      }
      if (w[2]<(*norme)) {
	*norme = w[2];
	index_max_vp = 2;
      }
    }
    /*if (w[0]>w[1]) {
     *norme = w[0];
     index_max_vp = 0;
     } else {
     *norme = w[1];
     index_max_vp = 1;
     }
     if (w[2]>(*norme)) {
     *norme = w[2];
     index_max_vp = 2;
     }*/

    if (is_trace == 1) {
      trace = w[0] + w[1] + w[2];
      *norme = trace;
    }

    /* THE Wavelet Transform !!! */
    //*norme /= min;
    *gx1 = (*norme)*wt_tensor[0][index_max_vp];
    *gy1 = (*norme)*wt_tensor[1][index_max_vp];
    *gz1 = (*norme)*wt_tensor[2][index_max_vp];
    // *norme = sqrt( (*gx)*(*gx) + (*gy)*(*gy) + (*gz)*(*gz) );
  }

  /* close full 3D : free memory */
  /* static void close_full_3D() */
  {
    int z;
    for (z=0; z<3; z++) {
      free((void*) wt_tensor[z]);
      free((void*) v[z]);
    }
    free(w);
    free(rv1);
  }

}


/* void GradientModulus3D_tensor3D_vector( fftw_real *derivative_along_X1, */
/* 					fftw_real *derivative_along_Y1, */
/* 					fftw_real *derivative_along_Z1, */
/* 					fftw_real *derivative_along_X2, */
/* 					fftw_real *derivative_along_Y2, */
/* 					fftw_real *derivative_along_Z2, */
/* 					fftw_real *derivative_along_X3, */
/* 					fftw_real *derivative_along_Y3, */
/* 					fftw_real *derivative_along_Z3, */
/* 					int length, */
/* 					int type, */
/* 					int isSvbulk) */
/* { */
/*   register int i; */
/*   register fftw_real *gx1 = derivative_along_X1; */
/*   register fftw_real *gy1 = derivative_along_Y1; */
/*   register fftw_real *gz1 = derivative_along_Z1; */
/*   register fftw_real *gx2 = derivative_along_X2; */
/*   register fftw_real *gy2 = derivative_along_Y2; */
/*   register fftw_real *gz2 = derivative_along_Z2; */
/*   register fftw_real *gx3 = derivative_along_X3; */
/*   register fftw_real *gy3 = derivative_along_Y3; */
/*   register fftw_real *gz3 = derivative_along_Z3; */
/*   register fftw_real zenorme; */

/*   register int index_max_vp,index_min_vp, index_mid_vp; */
/*   register fftw_real min, mid; */

/*   for ( i=0; i<length; i++, gx1++, gy1++, gz1++, gx2++, gy2++, gz2++, gx3++, gy3++, gz3++ ) { */
/*     /\* aim is fill norm (specral radius of wt_tensor) *\/ */
/*     /\* fill wt_tensor *\/ */
/*     wt_tensor[0][0] = *gx1; */
/*     wt_tensor[0][1] = *gx2; */
/*     wt_tensor[0][2] = *gx3; */
/*     wt_tensor[1][0] = *gy1; */
/*     wt_tensor[1][1] = *gy2; */
/*     wt_tensor[1][2] = *gy3; */
/*     wt_tensor[2][0] = *gz1; */
/*     wt_tensor[2][1] = *gz2; */
/*     wt_tensor[2][2] = *gz3; */
/*     dsvd(wt_tensor,3,3,w,v,rv1); */
    
/*     /\* now wt_tensor contains the matrix U of the singular value decomposition */
/*        wt_tensor(old) = wt_tensor * W * V' *\/ */

/*     /\* find max_vp = spectral radius ! *\/ */
/*     /\* A REVOIR *\/ */
/*     if (w[0]>w[1] && w[0]>w[2]) { /\* max is at index 0 *\/ */

/*       index_max_vp = 0;  */
/*       zenorme = w[index_max_vp]; */
/*       if (w[1]>w[2]) { */
/* 	index_min_vp = 2; */
/*       } else { */
/* 	index_min_vp = 1; */
/*       } */
/*       min = w[index_min_vp]; */

/*     } else if (w[1]>w[2] && w[1]>w[0]) {/\* max is at index 1 *\/ */

/*       index_max_vp = 1;  */
/*       zenorme = w[index_max_vp]; */
/*       if (w[2]>w[0]) { */
/* 	index_min_vp = 0; */
/*       } else { */
/* 	index_min_vp = 2; */
/*       } */
/*       min = w[index_min_vp]; */
      
/*     } else if (w[2]>w[0] && w[2]>w[1]) {/\* max is at index 2 *\/ */

/*       index_max_vp = 2;  */
/*       zenorme = w[index_max_vp]; */
/*       if (w[0]>w[1]) { */
/* 	index_min_vp = 1; */
/*       } else { */
/* 	index_min_vp = 0; */
/*       } */
/*       min = w[index_min_vp]; */
      
/*     } */

/*     /\* use the fact that sum of index is 0+1+2=3 *\/ */
/*     index_mid_vp = 3 - (index_max_vp + index_min_vp); */
/*     mid = w[index_mid_vp]; */

/*     /\* THE Wavelet Transform !!! *\/ */
/*     //\*norme /= min; */
/*     if (isSvbulk) { /\* output ordered singular values *\/ */
/*       *gx1 = zenorme; */
/*       *gy1 = mid; */
/*       *gz1 = min; */
/*     } else { */
/*       if (type == 1) { */
/* 	*gx1 = (zenorme)*wt_tensor[0][index_max_vp]; */
/* 	*gy1 = (zenorme)*wt_tensor[1][index_max_vp]; */
/* 	*gz1 = (zenorme)*wt_tensor[2][index_max_vp]; */
	
/* 	*gz2 = (zenorme)*wt_tensor[0][index_max_vp]; */
/* 	*gz3 = (zenorme)*wt_tensor[1][index_max_vp]; */
/*       } else { */
/* 	*gx1 = (min)*wt_tensor[0][index_min_vp]; */
/* 	*gy1 = (min)*wt_tensor[1][index_min_vp]; */
/* 	*gz1 = (min)*wt_tensor[2][index_min_vp]; */
	
/* 	*gz2 = (min)*wt_tensor[0][index_min_vp]; */
/* 	*gz3 = (min)*wt_tensor[1][index_min_vp]; */
/*       } */
/*       // *norme = sqrt( (*gx)*(*gx) + (*gy)*(*gy) + (*gz)*(*gz) ); */
/*     } */
/*   } */
/* } */

/* /\*  */
/*  * (august 3rd 2004) */
/*  * compute "a kind of" curl */
/*  *\/ */
/* void CurlModulus3D_tensor3D( fftw_real *curl_modulus, */
/* 			     fftw_real *derivative_along_X1, */
/* 			     fftw_real *derivative_along_Y1, */
/* 			     fftw_real *derivative_along_Z1, */
/* 			     fftw_real *derivative_along_X2, */
/* 			     fftw_real *derivative_along_Y2, */
/* 			     fftw_real *derivative_along_Z2, */
/* 			     fftw_real *derivative_along_X3, */
/* 			     fftw_real *derivative_along_Y3, */
/* 			     fftw_real *derivative_along_Z3, */
/* 			     int length) */
/* { */
/*   register int i; */
/*   register fftw_real *norme = curl_modulus; */
/*   register fftw_real *gx1 = derivative_along_X1; */
/*   register fftw_real *gy1 = derivative_along_Y1; */
/*   register fftw_real *gz1 = derivative_along_Z1; */
/*   register fftw_real *gx2 = derivative_along_X2; */
/*   register fftw_real *gy2 = derivative_along_Y2; */
/*   register fftw_real *gz2 = derivative_along_Z2; */
/*   register fftw_real *gx3 = derivative_along_X3; */
/*   register fftw_real *gy3 = derivative_along_Y3; */
/*   register fftw_real *gz3 = derivative_along_Z3; */

/*   register fftw_real min; */

/*   for ( i=0; i<length; i++, norme++, gx1++, gy1++, gz1++, gx2++, gy2++, gz2++, gx3++, gy3++, gz3++ ) { */
/*     /\* The "Curl" Wavelet Transform *\/ */
/*     *gx1 = *gy1 - *gx2; */
/*     *gy1 = *gz2 - *gy3; */
/*     *gz1 = *gx3 - *gz1; */
/*     *norme = sqrt( (*gx1)*(*gx1) + (*gy1)*(*gy1) + (*gz1)*(*gz1) ); */
/*   } */
/* } */

/* /\*  */
/*  * (august 3rd 2004) */
/*  * compute "a kind of" curl */
/*  *\/ */
/* void CurlModulus3D_tensor3D_vector( fftw_real *derivative_along_X1, */
/* 				    fftw_real *derivative_along_Y1, */
/* 				    fftw_real *derivative_along_Z1, */
/* 				    fftw_real *derivative_along_X2, */
/* 				    fftw_real *derivative_along_Y2, */
/* 				    fftw_real *derivative_along_Z2, */
/* 				    fftw_real *derivative_along_X3, */
/* 				    fftw_real *derivative_along_Y3, */
/* 				    fftw_real *derivative_along_Z3, */
/* 				    int length) */
/* { */
/*   register int i; */
/*   register fftw_real *gx1 = derivative_along_X1; */
/*   register fftw_real *gy1 = derivative_along_Y1; */
/*   register fftw_real *gz1 = derivative_along_Z1; */
/*   register fftw_real *gx2 = derivative_along_X2; */
/*   register fftw_real *gy2 = derivative_along_Y2; */
/*   register fftw_real *gz2 = derivative_along_Z2; */
/*   register fftw_real *gx3 = derivative_along_X3; */
/*   register fftw_real *gy3 = derivative_along_Y3; */
/*   register fftw_real *gz3 = derivative_along_Z3; */

/*   register fftw_real min; */

/*   for ( i=0; i<length; i++, gx1++, gy1++, gz1++, gx2++, gy2++, gz2++, gx3++, gy3++, gz3++ ) { */
/*     /\* The "Curl" Wavelet Transform *\/ */
/*     *gx1 = *gy1 - *gx2; */
/*     *gy1 = *gz2 - *gy3; */
/*     *gz1 = *gx3 - *gz1; */
/*   } */
/* } */

/* /\* */
/*  * */
/*  *\/ */
/* void TakeParalleleCompo3( fftw_real *zecompoL, /\* result buffer *\/ */
/* 			  fftw_real *zecompoT, /\* result buffer *\/ */
/* 			  fftw_real *gradX, /\* gradX buffer *\/ */
/* 			  fftw_real *gradY, */
/* 			  fftw_real *gradZ,				  */
/* 			  int length, /\* buffers' length *\/ */
/* 			  int axis_proj */
/* 			  ) */
/* { */
/*   register int i; */
/*   register fftw_real *gradx = gradX; */
/*   register fftw_real *grady = gradY; */
/*   register fftw_real *gradz = gradZ; */
/*   register fftw_real *ZeL = zecompoL; */
/*   register fftw_real *ZeT = zecompoT; */

/*   if (axis_proj == 0) { */
/*     for ( i=0; i<length; i++, ZeL++, ZeT++, gradx++, grady++, gradz++ ) { */
/*       *ZeL = fabsf(*gradx); */
/*       *ZeT = (fftw_real) sqrt(*grady*(*grady)+*gradz*(*gradz)); */
/*     } */
/*   } else if (axis_proj == 1) { */
/*     for ( i=0; i<length; i++, ZeL++, ZeT++, gradx++, grady++, gradz++ ) { */
/*       *ZeL = fabsf(*grady); */
/*       *ZeT = (fftw_real) sqrt((*gradx)*(*gradx)+(*gradz)*(*gradz)); */
/*     } */
/*   } else if (axis_proj == 2) { */
/*     for ( i=0; i<length; i++, ZeL++, ZeT++, gradx++, grady++, gradz++ ) { */
/*       *ZeL = fabsf(*gradz); */
/*       *ZeT = (fftw_real) sqrt((*gradx)*(*gradx)+(*grady)*(*grady)); */
/*     } */
/*   } */
/* } */

void init_full_2D(float **wt_tensor,
		  float **v,
		  float  *w,
		  double *rv1)
{
  int z;
  /* alloc wt_tensor */
  wt_tensor = (float **) malloc(2 * sizeof(float *));
  for (z=0; z<2; z++)
    wt_tensor[z] = (float *) malloc(2 * sizeof(float));
  
  v = (float **) malloc(2 * sizeof(float *));
  for (z=0; z<2; z++)
    v[z] = (float *) malloc(2 * sizeof(float));
  
  w = (float *) malloc(2 * sizeof(float));
  rv1 = (double *) malloc(2 * sizeof(double));

}

void close_full_2D(float **wt_tensor,
		   float **v,
		   float  *w,
		   double *rv1)
{
  int z;
  for (z=0; z<2; z++) {
    free((void*) wt_tensor[z]);
    free((void*) v[z]);
  }
  free(w);
  free(rv1);
}

void init_full_3D(float **wt_tensor,
		  float **v,
		  float  *w,
		  double *rv1)
{
  int z;
  /* alloc wt_tensor */
  wt_tensor = (float **) malloc(3 * sizeof(float *));
  for (z=0; z<3; z++)
    wt_tensor[z] = (float *) malloc(3 * sizeof(float));
  
  v = (float **) malloc(3 * sizeof(float *));
  for (z=0; z<3; z++)
    v[z] = (float *) malloc(3 * sizeof(float));
  
  w = (float *) malloc(3 * sizeof(float));
  rv1 = (double *) malloc(3 * sizeof(double));


}

void close_full_3D(float **wt_tensor,
		   float **v,
		   float  *w,
		   double *rv1)
{
  int z;
  for (z=0; z<3; z++) {
    free((void*) wt_tensor[z]);
    free((void*) v[z]);
  }
  free(w);
  free(rv1);
}


/*
 * singular value decomposition (from Numerical Recipes)
 * TO BE REMOVED and/or IMPROVED
 */
int dsvd(float **a, int m, int n, float *w, float **v, double *rrv1)
{
  int flag, i, its, j, jj, k, l, nm;
  double c, f, h, s, x, y, z;
  double anorm = 0.0, g = 0.0, scale = 0.0;
  //double *rv1;
  
  if (m < n) 
    {
      fprintf(stderr, "#rows must be > #cols \n");
      return(0);
    }
  
  //rv1 = (double *)malloc((unsigned int) n*sizeof(double));
  
  /* Householder reduction to bidiagonal form */
  for (i = 0; i < n; i++) 
    {
      /* left-hand reduction */
      l = i + 1;
      rrv1[i] = scale * g;
      g = s = scale = 0.0;
      if (i < m) 
        {
	  for (k = i; k < m; k++) 
	    scale += fabs((double)a[k][i]);
	  if (scale) 
            {
	      for (k = i; k < m; k++) 
                {
		  a[k][i] = (float)((double)a[k][i]/scale);
		  s += ((double)a[k][i] * (double)a[k][i]);
                }
	      f = (double)a[i][i];
	      g = -SIGN(sqrt(s), f);
	      h = f * g - s;
	      a[i][i] = (float)(f - g);
	      if (i != n - 1) 
                {
		  for (j = l; j < n; j++) 
                    {
		      for (s = 0.0, k = i; k < m; k++) 
			s += ((double)a[k][i] * (double)a[k][j]);
		      f = s / h;
		      for (k = i; k < m; k++) 
			a[k][j] += (float)(f * (double)a[k][i]);
                    }
                }
	      for (k = i; k < m; k++) 
		a[k][i] = (float)((double)a[k][i]*scale);
            }
        }
      w[i] = (float)(scale * g);
      
      /* right-hand reduction */
      g = s = scale = 0.0;
      if (i < m && i != n - 1) 
        {
	  for (k = l; k < n; k++) 
	    scale += fabs((double)a[i][k]);
	  if (scale) 
            {
	      for (k = l; k < n; k++) 
                {
		  a[i][k] = (float)((double)a[i][k]/scale);
		  s += ((double)a[i][k] * (double)a[i][k]);
                }
	      f = (double)a[i][l];
	      g = -SIGN(sqrt(s), f);
	      h = f * g - s;
	      a[i][l] = (float)(f - g);
	      for (k = l; k < n; k++) 
		rrv1[k] = (double)a[i][k] / h;
	      if (i != m - 1) 
                {
		  for (j = l; j < m; j++) 
                    {
		      for (s = 0.0, k = l; k < n; k++) 
			s += ((double)a[j][k] * (double)a[i][k]);
		      for (k = l; k < n; k++) 
			a[j][k] += (float)(s * rrv1[k]);
                    }
                }
	      for (k = l; k < n; k++) 
		a[i][k] = (float)((double)a[i][k]*scale);
            }
        }
      anorm = MAX(anorm, (fabs((double)w[i]) + fabs(rrv1[i])));
    }
  
  /* accumulate the right-hand transformation */
  for (i = n - 1; i >= 0; i--) 
    {
      if (i < n - 1) 
        {
	  if (g) 
            {
	      for (j = l; j < n; j++)
		v[j][i] = (float)(((double)a[i][j] / (double)a[i][l]) / g);
	      /* double division to avoid underflow */
	      for (j = l; j < n; j++) 
                {
		  for (s = 0.0, k = l; k < n; k++) 
		    s += ((double)a[i][k] * (double)v[k][j]);
		  for (k = l; k < n; k++) 
		    v[k][j] += (float)(s * (double)v[k][i]);
                }
            }
	  for (j = l; j < n; j++) 
	    v[i][j] = v[j][i] = 0.0;
        }
      v[i][i] = 1.0;
      g = rrv1[i];
      l = i;
    }
  
  /* accumulate the left-hand transformation */
  for (i = n - 1; i >= 0; i--) 
    {
      l = i + 1;
      g = (double)w[i];
      if (i < n - 1) 
	for (j = l; j < n; j++) 
	  a[i][j] = 0.0;
      if (g) 
        {
	  g = 1.0 / g;
	  if (i != n - 1) 
            {
	      for (j = l; j < n; j++) 
                {
		  for (s = 0.0, k = l; k < m; k++) 
		    s += ((double)a[k][i] * (double)a[k][j]);
		  f = (s / (double)a[i][i]) * g;
		  for (k = i; k < m; k++) 
		    a[k][j] += (float)(f * (double)a[k][i]);
                }
            }
	  for (j = i; j < m; j++) 
	    a[j][i] = (float)((double)a[j][i]*g);
        }
      else 
        {
	  for (j = i; j < m; j++) 
	    a[j][i] = 0.0;
        }
      ++a[i][i];
    }
  
  /* diagonalize the bidiagonal form */
  for (k = n - 1; k >= 0; k--) 
    {                             /* loop over singular values */
      for (its = 0; its < 30; its++) 
        {                         /* loop over allowed iterations */
	  flag = 1;
	  for (l = k; l >= 0; l--) 
            {                     /* test for splitting */
	      nm = l - 1;
	      if (fabs(rrv1[l]) + anorm == anorm) 
                {
		  flag = 0;
		  break;
                }
	      if (fabs((double)w[nm]) + anorm == anorm) 
		break;
            }
	  if (flag) 
            {
	      c = 0.0;
	      s = 1.0;
	      for (i = l; i <= k; i++) 
                {
		  f = s * rrv1[i];
		  if (fabs(f) + anorm != anorm) 
                    {
		      g = (double)w[i];
		      h = PYTHAG(f, g);
		      w[i] = (float)h; 
		      h = 1.0 / h;
		      c = g * h;
		      s = (- f * h);
		      for (j = 0; j < m; j++) 
                        {
			  y = (double)a[j][nm];
			  z = (double)a[j][i];
			  a[j][nm] = (float)(y * c + z * s);
			  a[j][i] = (float)(z * c - y * s);
                        }
                    }
                }
            }
	  z = (double)w[k];
	  if (l == k) 
            {                  /* convergence */
	      if (z < 0.0) 
                {              /* make singular value nonnegative */
		  w[k] = (float)(-z);
		  for (j = 0; j < n; j++) 
		    v[j][k] = (-v[j][k]);
                }
	      break;
            }
	  if (its >= 30) {
	    //free((void*) rv1);
	    fprintf(stderr, "No convergence after 30,000! iterations \n");
	    return(0);
	  }
	  
	  /* shift from bottom 2 x 2 minor */
	  x = (double)w[l];
	  nm = k - 1;
	  y = (double)w[nm];
	  g = rrv1[nm];
	  h = rrv1[k];
	  f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
	  g = PYTHAG(f, 1.0);
	  f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
          
	  /* next QR transformation */
	  c = s = 1.0;
	  for (j = l; j <= nm; j++) 
            {
	      i = j + 1;
	      g = rrv1[i];
	      y = (double)w[i];
	      h = s * g;
	      g = c * g;
	      z = PYTHAG(f, h);
	      rrv1[j] = z;
	      c = f / z;
	      s = h / z;
	      f = x * c + g * s;
	      g = g * c - x * s;
	      h = y * s;
	      y = y * c;
	      for (jj = 0; jj < n; jj++) 
                {
		  x = (double)v[jj][j];
		  z = (double)v[jj][i];
		  v[jj][j] = (float)(x * c + z * s);
		  v[jj][i] = (float)(z * c - x * s);
                }
	      z = PYTHAG(f, h);
	      w[j] = (float)z;
	      if (z) 
                {
		  z = 1.0 / z;
		  c = f * z;
		  s = h * z;
                }
	      f = (c * g) + (s * y);
	      x = (c * y) - (s * g);
	      for (jj = 0; jj < m; jj++) 
                {
		  y = (double)a[jj][j];
		  z = (double)a[jj][i];
		  a[jj][j] = (float)(y * c + z * s);
		  a[jj][i] = (float)(z * c - y * s);
                }
            }
	  rrv1[l] = 0.0;
	  rrv1[k] = f;
	  w[k] = (float)x;
        }
    }
  //free((void*) rv1);
  return 1;
}

double PYTHAG(double a, double b)
{
  double at = fabs(a), bt = fabs(b), ct, result;

  if (at > bt)       { ct = bt / at; result = at * sqrt(1.0 + ct * ct); }
  else if (bt > 0.0) { ct = at / bt; result = bt * sqrt(1.0 + ct * ct); }
  else result = 0.0;
  return(result);
}
