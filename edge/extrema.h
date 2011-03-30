/*
 * extrema.h --
 *
 * tools for non-maxima gradient point suppression
 *
 * adapted from extrema.h written by G. Malandain, see this link
 * http://www-sop.inria.fr/epidaure/personnel/malandain/segment/edges.html
 *
 */

#ifndef _extrema_h_
#define _extrema_h_

#include "misc.h"

int Extract_Gradient_Maxima_2D( Image *GradX,
				Image *GradY,
				float *mod,
				float *arg,
				float *maxima,
				float scale);

int Extract_Gradient_Maxima_2D_vectorfield( Image *GradX1,
					    Image *GradY1,
					    Image *GradX2,
					    Image *GradY2,
					    float *mod,
					    float *arg,
					    float *maxima,
					    float scale, 
					    int type_singular_value);

/*   int Extract_Gradient_2D_full_2D_fftw_vector( void *bufferIn1, */
/* 					       void *bufferIn2, */
/* 					       void *bufferOut1, */
/* 					       void *bufferOut2, */
/* 					       int  *bufferDims, */
/* 					       float scale, */
/* 					       int   isMexican, */
/* 					       int   type_singular_value, */
/* 					       int   isSvbulk); */

/*   int Extract_Gradient_Argument_2D_full_2D_fftw_det( void *bufferIn1, */
/* 						     void *bufferIn2, */
/* 						     void *bufferOut, */
/* 						     int  *bufferDims, */
/* 						     float scale); */


/*   int Extract_Gradient_Argument_2D_Laplacian_filtering_fftw( void *bufferIn, /\* input buffer *\/ */
/* 							     void *bufferOut, /\* output buffer *\/ */
/* 							     void *bufferOut2, /\* output buffer2 *\/ */
/* 							     int *bufferDims, /\* buffers' dimensions *\/ */
/* 							     float scale); */
  
  
/*   int Extract_Gradient_Argument_2D_Laplacian_full_2D_fftw( void *bufferIn1, */
/* 							   void *bufferIn2, */
/* 							   void *bufferOut1, */
/* 							   void *bufferOut2, */
/* 							   int *bufferDims, */
/* 							   float scale,  */
/* 							   int type_singular_value); */
  
int Extract_Gradient_Maxima_3D(Image3D *GradX,
			       Image3D *GradY,
			       Image3D *GradZ,
			       float *mod,
			       float *maxima,
			       float scale);
  
int Extract_Gradient_Maxima_3D_vectorfield( Image3D *GradX1,
					    Image3D *GradY1,
					    Image3D *GradZ1,
					    Image3D *GradX2,
					    Image3D *GradY2,
					    Image3D *GradZ2,
					    Image3D *GradX3,
					    Image3D *GradY3,
					    Image3D *GradZ3,
					    float *mod,
					    float *maxima,
					    float scale, 
					    int type_singular_value);

/*   int Extract_Gradient_Maxima_3D_fftw_with_other_coordinate( */
/* 							    void *bufferIn, */
/* 							    void *bufferOut, */
/* 							    void *bufferOut2, */
/* 							    int  *bufferDims, */
/* 							    float scale, */
/* 							    int coord_type); */
  
/*   int Extract_Gradient_Maxima_3D_full_3D_fftw( void *bufferIn1, */
/* 					       void *bufferIn2, */
/* 					       void *bufferIn3, */
/* 					       void *bufferOut, */
/* 					       int *bufferDims, */
/* 					       float scale,  */
/* 					       int type_singular_value, */
/* 					       int is_trace); */
  
/*   int Extract_Curl_Maxima_3D_full_3D_fftw( void *bufferIn1, */
/* 					   void *bufferIn2, */
/* 					   void *bufferIn3, */
/* 					   void *bufferOut, */
/* 					   int *bufferDims, */
/* 					   float scale); */
  
/*   int Extract_Gradient_Maxima_3D_full_3D_fftw_vector( void *bufferIn1, */
/* 						      void *bufferIn2, */
/* 						      void *bufferIn3, */
/* 						      void *bufferOut1, */
/* 						      void *bufferOut2, */
/* 						      void *bufferOut3, */
/* 						      int *bufferDims, */
/* 						      float scale, */
/* 						      int type_filter, */
/* 						      int type_singular_value, */
/* 						      int   isSvbulk); */
  
/*   int Extract_Curl_Maxima_3D_full_3D_fftw_vector( void *bufferIn1, */
/* 						  void *bufferIn2, */
/* 						  void *bufferIn3, */
/* 						  void *bufferOut1, */
/* 						  void *bufferOut2, */
/* 						  void *bufferOut3, */
/* 						  int *bufferDims, */
/* 						  float scale, */
/* 						  int type_filter); */

/*   int Extract_Gradient_Maxima_3D_full_3D_fftw_vector2( void *bufferIn1, */
/* 						       void *bufferIn2, */
/* 						       void *bufferIn3, */
/* 						       void *bufferOut1, */
/* 						       void *bufferOut2, */
/* 						       void *bufferOut3, */
/* 						       int *bufferDims, */
/* 						       float scale, */
/* 						       int type_filter, */
/* 						       int type_singular_value, */
/* 						       int is_trace); */
  
/*   int Extract_Gradient_Maxima_3D_Laplacian_full_3D_fftw( void *bufferIn1, */
/* 							 void *bufferIn2, */
/* 							 void *bufferIn3, */
/* 							 void *bufferOut, */
/* 							 int *bufferDims, */
/* 							 float scale, */
/* 							 int type_singular_value, */
/* 							 int is_trace); */
  
/*   int Extract_Gradient_Maxima_3D_Laplacian_filtering_fftw( void *bufferIn, */
/* 							   void *bufferOut, */
/* 							   int  *bufferDims, */
/* 							   float scale); */
  
/*   int Extract_Gradient_Maxima_3D_Laplacian_filtering_fftw_parallele3( void *bufferIn, */
/* 								      void *bufferOut, */
/* 								      int  *bufferDims, */
/* 								      float scale, */
/* 								      void *bufferLongi, */
/* 								      void *bufferTrans, */
/* 								      int axis_proj); */
  
/*   int Extract_Gradient_Maxima_3D_Laplacian_filtering_fftw_parallele3_LT_pur( void *bufferIn, */
/* 									     void *bufferOut, */
/* 									     int  *bufferDims, */
/* 									     float scale, */
/* 									     void *bufferLongi, */
/* 									     void *bufferTrans, */
/* 									     int axis_proj); */

#endif /* _extrema_h_ */
