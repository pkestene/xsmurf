/*
 * extrema_core.h --
 *
 *
 */

#ifndef _extrema_core_h_
#define _extrema_core_h_

void Remove_Gradient_NonMaxima_Slice_2D( float *maxima,
					 float *gx,
					 float *gy,
					 float *norme,
					 int *bufferDims );

void Remove_Gradient_NonMaxima_Slice_3D( float *maxima,
					 float *gx,
					 float *gy,
					 float *gz,
					 float **norme,
					 int *bufferDims );

#endif /* _extrema_core_h_ */
