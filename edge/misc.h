/*
 * misc.h --
 *
 *
 */

#ifndef _extrema_misc_h_
#define _extrema_misc_h_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*#include <extrema_core.h>
  #include <extrema.h>*/
/*#include <rfftw.h>*/
#include "../image/image.h"
#include "../image/image3D.h"

#define SIGN(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#define MAX(x,y) ((x)>(y)?(x):(y))

/**
 * This enum is used to trigger different behavior when computing 
 * the tensor wavelet transform.
 *
 * SVD_TYPE_MAX means we keep the modulus,direction associated to 
 *              the largest singular value
 * SVD_TYPE_MIN means we keep the modulus,direction associated to 
 *              the smallest singular value
 * SVD_TYPE_MAX_L means we keep the direction associated to 
 *                the largest singular value and modulus is max of the symetric
 *                part of the wavelet tensor
 * SVD_TYPE_MAX_T means we keep the direction associated to 
 *                the largest singular value and modulus is max of the antisymetric
 *                part of the wavelet tensor
 */
enum {
  SVD_TYPE_MAX = 0,
  SVD_TYPE_MIN = 1,
  SVD_TYPE_MAX_L = 2,
  SVD_TYPE_MAX_T = 3
};

/* Compute the gradient modulus in 2-D. 
 * 
 * gradient_modulus = sqrt ( 
 * derivative_along_X*derivative_along_X + 
 * derivative_along_Y*derivative_along_Y ).
 */ 
void GradientModulus2D( float *gradient_modulus, /* result buffer */ 
			float *derivative_along_X, /* first component */
			float *derivative_along_Y, /* second component */
			int length /* buffers' length */
			);

/* Compute the gradient argument in 2-D.
 * 
 * gradient_argument = atan2 ( derivative_along_Y, derivative_along_X ). 
 */ 
void GradientArgument2D(float *gradient_argument, /* result buffer */ 
			float *derivative_along_X,/* first component */ 
			float *derivative_along_Y, /* second component */ 
			int length /* buffers' length */ 
			); 

/* Compute the gradient modulus in 3-D. 
 * 
 * gradient_modulus = sqrt ( 
 * derivative_along_X*derivative_along_X + 
 * derivative_along_Y*derivative_along_Y + 
 * derivative_along_Z*derivative_along_Z ). 
 */ 
void GradientModulus3D( float *gradient_modulus, /* result buffer */
			float *derivative_along_X, /* first component */
			float *derivative_along_Y, /* second component */
			float *derivative_along_Z, /* third component */
			int length /* buffers' length */
			);

/* void GradientModulus3D_L( fftw_real *gradient_modulus, */
/* 			  fftw_real *derivative_along_X, */
/* 			  fftw_real *derivative_along_Y, */
/* 			  fftw_real *derivative_along_Z, */
/* 			  int length, */
/* 			  int type); */

/* void GradientModulus3D_T( fftw_real *gradient_modulus, */
/* 			  fftw_real *derivative_along_X, */
/* 			  fftw_real *derivative_along_Y, */
/* 			  fftw_real *derivative_along_Z, */
/* 			  int length, */
/* 			  int type); */

/**
 * Compute the tensor wavelet transform (modulus, argument)
 * from the components of the WT tensor
 * 
 * \param[out] gradient_modulus
 * \param[out] gradient_argument
 * \param[in] derivative_along_X1
 * \param[in] derivative_along_Y1
 * \param[in] derivative_along_X2
 * \param[in] derivative_along_Y1
 * \param[in] type (a valid value is taken from enum SVD_TYPE)
 */
void GradientModulus2D_tensor2D( float *gradient_modulus,
				 float *gradient_argument,
				 float *derivative_along_X1,
				 float *derivative_along_Y1,
				 float *derivative_along_X2,
				 float *derivative_along_Y2,
				 int length,
				 int type);

/* void GradientModulus2D_tensor2D_vector( fftw_real *derivative_along_X1, */
/* 					fftw_real *derivative_along_Y1, */
/* 					fftw_real *derivative_along_X2, */
/* 					fftw_real *derivative_along_Y2, */
/* 					int length, */
/* 					int type, */
/* 					int isSvbulk); */

/* void Gradient_tensor2D_det( fftw_real *gradient_det, */
/* 			    fftw_real *derivative_along_X1, */
/* 			    fftw_real *derivative_along_Y1, */
/* 			    fftw_real *derivative_along_X2, */
/* 			    fftw_real *derivative_along_Y2, */
/* 			    int length); */

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
				 int is_trace);

/* void GradientModulus3D_tensor3D_vector( fftw_real *derivative_along_X1,  */
/* 					fftw_real *derivative_along_Y1,  */
/* 					fftw_real *derivative_along_Z1,  */
/* 					fftw_real *derivative_along_X2,  */
/* 					fftw_real *derivative_along_Y2,  */
/* 					fftw_real *derivative_along_Z2,  */
/* 					fftw_real *derivative_along_X3,  */
/* 					fftw_real *derivative_along_Y3,  */
/* 					fftw_real *derivative_along_Z3,  */
/* 					int length, */
/* 					int type, */
/* 					int isSvbulk);  */

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
/* 			     int length); */

/* void CurlModulus3D_tensor3D_vector( fftw_real *derivative_along_X1, */
/* 				    fftw_real *derivative_along_Y1, */
/* 				    fftw_real *derivative_along_Z1, */
/* 				    fftw_real *derivative_along_X2, */
/* 				    fftw_real *derivative_along_Y2, */
/* 				    fftw_real *derivative_along_Z2, */
/* 				    fftw_real *derivative_along_X3, */
/* 				    fftw_real *derivative_along_Y3, */
/* 				    fftw_real *derivative_along_Z3, */
/* 				    int length); */

/* void TakeParalleleCompo3( fftw_real *zecompoL, /\* result buffer *\/ */
/* 			  fftw_real *zecompoT, */
/* 			  fftw_real *gradX, */
/* 			  fftw_real *gradY, */
/* 			  fftw_real *gradZ,	 */
/* 			  int length, /\* buffers' length *\/ */
/* 			  int axis_proj */
/* 			  ); */


void init_full_2D(float **wt_tensor,
		  float **v,
		  float  *w,
		  double *rv1);

void close_full_2D(float **wt_tensor,
		   float **v,
		   float  *w,
		   double *rv1);

void init_full_3D(float **wt_tensor,
		  float **v,
		  float  *w,
		  double *rv1);

void close_full_3D(float **wt_tensor,
		  float **v,
		  float  *w,
		  double *rv1);

double PYTHAG(double a, double b);

int dsvd(float **a, 
	 int m, int n, 
	 float *w, 
	 float **v, 
	 double *rv1);


#endif /* _extrema_misc_h_ */
