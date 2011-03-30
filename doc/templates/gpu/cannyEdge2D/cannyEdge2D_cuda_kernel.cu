

/****************************************************/
__device__ float gradx_filter(float x, float y) 
{
  return y*expf(-x*x-y*y);
}

/****************************************************/
__device__ float grady_filter(float x, float y) 
{
  return x*expf(-x*x-y*y);
}

/*************************************************************************/
/*
 * Compute the complex multiplication between an image of type FFTW_R2C 
 * and a function. The image data are organized as specified in the
 * fftw lib documentation (real multi-dimensional transform)
 */
__global__ void fft_filter_gradx_kernel (Complex *cData,
					 int Lx,
					 int Ly,
					 float   scale)
{
  float     tmp, kx, ky;  
  float     i_val;
  
  // read indexes
  int i = (int) (blockIdx.x * blockDim.x + threadIdx.x);
  int j = (int) (blockIdx.y * blockDim.y + threadIdx.y);
  
  
  int ij = j + i*Ly;
  
  if (i < Lx/2) {
    kx = scale/Lx*i;
  } else {
    kx = scale/Lx*(i-Lx);
  }
  if (j < Ly/2) {
    ky = scale/Ly*j;
  } else {
    ky = scale/Ly*(j-Ly);
  }
  
  i_val = gradx_filter(kx,ky);
  
  tmp = cData[ij].x;
  
  cData[ij].x = - cData[ij].y*i_val;
  cData[ij].y = tmp              *i_val;

}

__global__ void fft_filter_grady_kernel (Complex *cData,
					 int Lx,
					 int Ly,
					 float   scale)
{
  float     tmp, kx, ky;  
  float     i_val;
  
  // read indexes
  int i = (int) (blockIdx.x * blockDim.x + threadIdx.x);
  int j = (int) (blockIdx.y * blockDim.y + threadIdx.y);
  
  
  int ij = j + i*Ly;
  
  if (i < Lx/2) {
    kx = scale/Lx*i;
  } else {
    kx = scale/Lx*(i-Lx);
  }
  if (j < Ly/2) {
    ky = scale/Ly*j;
  } else {
    ky = scale/Ly*(j-Ly);
  }
  
  i_val = grady_filter(kx,ky);
  
  tmp = cData[ij].x;
  
  cData[ij].x = - cData[ij].y*i_val;
  cData[ij].y = tmp          *i_val;

}


__global__ void polar_gradient_kernel (float *mod, float *arg,
				       Complex *gradx,
				       Complex *grady,
				       int Lx,
				       int Ly)
{
  
  // read indexes
  int i = (int) (blockIdx.x * blockDim.x + threadIdx.x);
  int j = (int) (blockIdx.y * blockDim.y + threadIdx.y);
  int ij = j + i*Ly;
  
  // modulus
  mod[ij] = sqrtf( gradx[ij].x*gradx[ij].x +  grady[ij].x*grady[ij].x );

  // argument
  arg[ij] = atan2f(grady[ij].x, gradx[ij].x);

}

/*
 *
 */
__global__ void remove_nonmaxima_gradient2D_kernel (float *max,
						    Complex *gradx,
						    Complex *grady,
						    float *mod,
						    int lx,
						    int ly)
{
  
  // read indexes
  int i = (int) (blockIdx.x * blockDim.x + threadIdx.x);
  int j = (int) (blockIdx.y * blockDim.y + threadIdx.y);
  //int ij = j + i*Ly;
  int ij = i + j*lx;

  /*
   * dimensions
   */
  int dimx = lx;
  int dimy = ly;
  int dimxMinusOne = dimx - 1;
  int dimxPlusOne = dimx + 1;
  int dimyMinusOne = dimy - 1;
  
  float _EPSILON_NORM_ = 0.0000005f;
  /*
   * epsilon value to decide of the interpolation type.
   * If one derivative's absolute value is larger than this
   * epsilon (close to one), then we use the nearest value
   * else we perform a [bi,tri]linear interpolation.
   */
  float _EPSILON_DERIVATIVE_ = 0.9995f;
  
  /* 
   * pointers
   */
  /*float *fl_pt1;
  float *fl_pt2;
  float *fl_max;
  float *fl_nor;*/
  int fl_upper_left;
  float gx,gy,norme;

  /*
   * coordinates and vector's components
   */
  float normalized_gx;
  float normalized_gy;
  float x_point_to_be_interpolated;
  float y_point_to_be_interpolated;
  int x_upper_left_corner;
  int y_upper_left_corner;
  /*
   * coefficients
   */ 
  float dx, dy, dxdy;
  float c00, c01, c10, c11;
  /*
   * modulus
   */
  float interpolated_norme;
  
  if (i == 0 || i== lx-1 || j==0 || j==ly-1) {
   /*
   * we set the image border to zero.
   * First the borders along X direction,
   * second, the borders along the Y direction.
   */
    max[ij] = 0.0f;
    return;
  }
   
  /*
   * We investigate the middle of the image.
   */
  gx = gradx[ij].x;
  gy = grady[ij].x;
  norme = mod[ij];
  
  if (norme < _EPSILON_NORM_) {
    max[ij] = 0.0f;
      return;
  }
  
  normalized_gx = gx/norme;
  normalized_gy = gy/norme;
  
  /*
   * May we use the nearest value?
   */
  if ( (-normalized_gx > _EPSILON_DERIVATIVE_) ||
       (normalized_gx > _EPSILON_DERIVATIVE_) ||
       (-normalized_gy > _EPSILON_DERIVATIVE_) ||
       (normalized_gy > _EPSILON_DERIVATIVE_) ) {
    /*
     * First point to be interpolated.
     */
    x_upper_left_corner = (int)( (float)i + normalized_gx + 0.5 );
    y_upper_left_corner = (int)( (float)j + normalized_gy + 0.5 );
    interpolated_norme = mod[x_upper_left_corner + y_upper_left_corner * dimx];
    if ( norme <= interpolated_norme ) {
      max[ij] = 0.0f;
      return;
    }
    /*
     * Second point to be interpolated.
     */
    x_upper_left_corner = (int)( (float)i - normalized_gx + 0.5 );
    y_upper_left_corner = (int)( (float)j - normalized_gy + 0.5 );
    interpolated_norme = mod[x_upper_left_corner + y_upper_left_corner * dimx];
    if ( norme < interpolated_norme ) {
      max[ij] = 0.0f;
      return;
    }
    /*
     * We found a gradient extrema.
     */
    max[ij] = norme;
    return;
  }
  /*
   * From here we perform a bilinear interpolation
   */
  
  /*
   * First point to be interpolated.
   * It is the current point + an unitary vector
   * in the direction of the gradient.
   * It must be inside the image.
   */
  x_point_to_be_interpolated = (float)i + normalized_gx;
  y_point_to_be_interpolated = (float)j + normalized_gy;
  if ( (x_point_to_be_interpolated < 0.0f) ||
       (x_point_to_be_interpolated >= dimxMinusOne) ||
       (y_point_to_be_interpolated < 0.0f) ||
       (y_point_to_be_interpolated >= dimyMinusOne) ) {
    max[ij] = 0.0f;
    return;
  }
  /* 
   * Upper left corner,
   * coordinates of the point to be interpolated
   * with respect to this corner.
   */
  x_upper_left_corner = (int)x_point_to_be_interpolated;
  y_upper_left_corner = (int)y_point_to_be_interpolated;
  dx = x_point_to_be_interpolated - (float)x_upper_left_corner;
  dy = y_point_to_be_interpolated - (float)y_upper_left_corner;
  dxdy = dx * dy;
  /* 
   * bilinear interpolation of the gradient modulus 
   * norme[x_point_to_be_interpolated, y_point_to_be_interpolated] =
   *   norme[0,0] * ( 1 - dx) * ( 1 - dy ) +
   *   norme[1,0] * ( dx ) * ( 1 - dy ) +
   *   norme[0,1] * ( 1 - dx ) * ( dy ) +
   *   norme[1,1] * ( dx ) * ( dy )
   */
  c00 = 1.0f - dx - dy + dxdy;
  c10 = dx - dxdy;
  c01 = dy - dxdy;
  c11 = dxdy;
  fl_upper_left = x_upper_left_corner + y_upper_left_corner * dimx;
  interpolated_norme = mod[fl_upper_left] * c00 +
    mod[fl_upper_left + 1] * c10 +
    mod[fl_upper_left + dimx] * c01 +
    mod[fl_upper_left + dimxPlusOne] * c11;
  /*
   * We compare the modulus of the point with the
   * interpolated modulus. It must be larger to be
   * still considered as a potential gradient extrema.
   *
   * Here, we consider that it is strictly superior.
   * The next comparison will be superior or equal.
   * This way, the extrema is in the light part of the
   * image. 
   * By inverting both tests, we can put it in the
   * dark side of the image.
   */
  if ( norme <= interpolated_norme ) {
    max[ij] = 0.0f;
    return;
  }
  /*
   * Second point to be interpolated.
   * It is the current point - an unitary vector
   * in the direction of the gradient.
   * It must be inside the image.
   */
  x_point_to_be_interpolated = (float)i - normalized_gx;
  y_point_to_be_interpolated = (float)j - normalized_gy;
  if ( (x_point_to_be_interpolated < 0.0) ||
       (x_point_to_be_interpolated >= dimxMinusOne) ||
       (y_point_to_be_interpolated < 0.0) ||
       (y_point_to_be_interpolated >= dimyMinusOne) ) {
    max[ij] = 0.0f;
    return;
  } 
  /* 
   * Upper left corner.
   */
  x_upper_left_corner = (int)x_point_to_be_interpolated;
  y_upper_left_corner = (int)y_point_to_be_interpolated;
  /* we do not recompute the coefficients
     dx = x_point_to_be_interpolated - (double)x_upper_left_corner;
     dy = y_point_to_be_interpolated - (double)y_upper_left_corner;
     dxdy = dx * dy;
  */
  /*
   * We may use the previous coefficients.
   * norme[x_point_to_be_interpolated, y_point_to_be_interpolated] =
   *   norme[0,0] * c11 +
   *   norme[1,0] * c01 +
   *   norme[0,1] * c10 +
   *   norme[1,1] * c00
   *
   * WARNING: it works only if the cases where one derivative is close
   *          to -/+ 1 are already be independently processed, else
   *          it may lead to errors.
   */
  /* we do not recompute the coefficients
     c00 = 1.0 - dx - dy + dxdy;
     c10 = dx - dxdy;
     c01 = dy - dxdy;
     c11 = dxdy;
     fl_upper_left = norme + (x_upper_left_corner + y_upper_left_corner * dimx);
     interpolated_norme = *(fl_upper_left) * c00 +
     *(fl_upper_left + 1) * c10 +
     *(fl_upper_left + dimx) * c01 +
     *(fl_upper_left + dimxPlusOne) * c11;
     */
  fl_upper_left = x_upper_left_corner + y_upper_left_corner * dimx;
  interpolated_norme = mod[fl_upper_left] * c11 +
    mod[fl_upper_left + 1] * c01 +
    mod[fl_upper_left + dimx] * c10 +
    mod[fl_upper_left + dimxPlusOne] * c00;
  /*
   * Last test to decide whether or not we 
   * have an extrema
   */
  if ( norme < interpolated_norme ) {
    max[ij] = 0.0f;
    return;
  }
  /*
   * We found a gradient extrema.
   */
  max[ij] = norme;

}

/********************************************
 * new version to compute number of maxima
 ********************************************/
__global__ void remove_nonmaxima_gradient2D_kernel2 (int *max,
						     Complex *gradx,
						     Complex *grady,
						     float *mod,
						     int lx,
						     int ly)
{
  // read indexes
  int i = (int) (blockIdx.x * blockDim.x + threadIdx.x);
  int j = (int) (blockIdx.y * blockDim.y + threadIdx.y);
  //int ij = j + i*Ly;
  int ij = i + j*lx;

  /*
   * dimensions
   */
  int dimx = lx;
  int dimy = ly;
  int dimxMinusOne = dimx - 1;
  int dimxPlusOne = dimx + 1;
  int dimyMinusOne = dimy - 1;
  
  float _EPSILON_NORM_ = 0.0000005f;
  /*
   * epsilon value to decide of the interpolation type.
   * If one derivative's absolute value is larger than this
   * epsilon (close to one), then we use the nearest value
   * else we perform a [bi,tri]linear interpolation.
   */
  float _EPSILON_DERIVATIVE_ = 0.9995f;
  
  /* 
   * pointers
   */
  /*float *fl_pt1;
  float *fl_pt2;
  float *fl_max;
  float *fl_nor;*/
  int fl_upper_left;
  float gx,gy,norme;

  /*
   * coordinates and vector's components
   */
  float normalized_gx;
  float normalized_gy;
  float x_point_to_be_interpolated;
  float y_point_to_be_interpolated;
  int x_upper_left_corner;
  int y_upper_left_corner;
  /*
   * coefficients
   */ 
  float dx, dy, dxdy;
  float c00, c01, c10, c11;
  /*
   * modulus
   */
  float interpolated_norme;
  
  if (i == 0 || i== lx-1 || j==0 || j==ly-1) {
   /*
   * we set the image border to zero.
   * First the borders along X direction,
   * second, the borders along the Y direction.
   */
    max[ij] = 0.0f;
    return;
  }
   
  /*
   * We investigate the middle of the image.
   */
  gx = gradx[ij].x;
  gy = grady[ij].x;
  norme = mod[ij];
  
  if (norme < _EPSILON_NORM_) {
    max[ij] = 0.0f;
      return;
  }
  
  normalized_gx = gx/norme;
  normalized_gy = gy/norme;
  
  /*
   * May we use the nearest value?
   */
  if ( (-normalized_gx > _EPSILON_DERIVATIVE_) ||
       (normalized_gx > _EPSILON_DERIVATIVE_) ||
       (-normalized_gy > _EPSILON_DERIVATIVE_) ||
       (normalized_gy > _EPSILON_DERIVATIVE_) ) {
    /*
     * First point to be interpolated.
     */
    x_upper_left_corner = (int)( (float)i + normalized_gx + 0.5 );
    y_upper_left_corner = (int)( (float)j + normalized_gy + 0.5 );
    interpolated_norme = mod[x_upper_left_corner + y_upper_left_corner * dimx];
    if ( norme <= interpolated_norme ) {
      max[ij] = 0.0f;
      return;
    }
    /*
     * Second point to be interpolated.
     */
    x_upper_left_corner = (int)( (float)i - normalized_gx + 0.5 );
    y_upper_left_corner = (int)( (float)j - normalized_gy + 0.5 );
    interpolated_norme = mod[x_upper_left_corner + y_upper_left_corner * dimx];
    if ( norme < interpolated_norme ) {
      max[ij] = 0.0f;
      return;
    }
    /*
     * We found a gradient extrema.
     */
    max[ij] = 1;
    return;
  }
  /*
   * From here we perform a bilinear interpolation
   */
  
  /*
   * First point to be interpolated.
   * It is the current point + an unitary vector
   * in the direction of the gradient.
   * It must be inside the image.
   */
  x_point_to_be_interpolated = (float)i + normalized_gx;
  y_point_to_be_interpolated = (float)j + normalized_gy;
  if ( (x_point_to_be_interpolated < 0.0f) ||
       (x_point_to_be_interpolated >= dimxMinusOne) ||
       (y_point_to_be_interpolated < 0.0f) ||
       (y_point_to_be_interpolated >= dimyMinusOne) ) {
    max[ij] = 0.0f;
    return;
  }
  /* 
   * Upper left corner,
   * coordinates of the point to be interpolated
   * with respect to this corner.
   */
  x_upper_left_corner = (int)x_point_to_be_interpolated;
  y_upper_left_corner = (int)y_point_to_be_interpolated;
  dx = x_point_to_be_interpolated - (float)x_upper_left_corner;
  dy = y_point_to_be_interpolated - (float)y_upper_left_corner;
  dxdy = dx * dy;
  /* 
   * bilinear interpolation of the gradient modulus 
   * norme[x_point_to_be_interpolated, y_point_to_be_interpolated] =
   *   norme[0,0] * ( 1 - dx) * ( 1 - dy ) +
   *   norme[1,0] * ( dx ) * ( 1 - dy ) +
   *   norme[0,1] * ( 1 - dx ) * ( dy ) +
   *   norme[1,1] * ( dx ) * ( dy )
   */
  c00 = 1.0f - dx - dy + dxdy;
  c10 = dx - dxdy;
  c01 = dy - dxdy;
  c11 = dxdy;
  fl_upper_left = x_upper_left_corner + y_upper_left_corner * dimx;
  interpolated_norme = mod[fl_upper_left] * c00 +
    mod[fl_upper_left + 1] * c10 +
    mod[fl_upper_left + dimx] * c01 +
    mod[fl_upper_left + dimxPlusOne] * c11;
  /*
   * We compare the modulus of the point with the
   * interpolated modulus. It must be larger to be
   * still considered as a potential gradient extrema.
   *
   * Here, we consider that it is strictly superior.
   * The next comparison will be superior or equal.
   * This way, the extrema is in the light part of the
   * image. 
   * By inverting both tests, we can put it in the
   * dark side of the image.
   */
  if ( norme <= interpolated_norme ) {
    max[ij] = 0.0f;
    return;
  }
  /*
   * Second point to be interpolated.
   * It is the current point - an unitary vector
   * in the direction of the gradient.
   * It must be inside the image.
   */
  x_point_to_be_interpolated = (float)i - normalized_gx;
  y_point_to_be_interpolated = (float)j - normalized_gy;
  if ( (x_point_to_be_interpolated < 0.0) ||
       (x_point_to_be_interpolated >= dimxMinusOne) ||
       (y_point_to_be_interpolated < 0.0) ||
       (y_point_to_be_interpolated >= dimyMinusOne) ) {
    max[ij] = 0.0f;
    return;
  } 
  /* 
   * Upper left corner.
   */
  x_upper_left_corner = (int)x_point_to_be_interpolated;
  y_upper_left_corner = (int)y_point_to_be_interpolated;
  /* we do not recompute the coefficients
     dx = x_point_to_be_interpolated - (double)x_upper_left_corner;
     dy = y_point_to_be_interpolated - (double)y_upper_left_corner;
     dxdy = dx * dy;
  */
  /*
   * We may use the previous coefficients.
   * norme[x_point_to_be_interpolated, y_point_to_be_interpolated] =
   *   norme[0,0] * c11 +
   *   norme[1,0] * c01 +
   *   norme[0,1] * c10 +
   *   norme[1,1] * c00
   *
   * WARNING: it works only if the cases where one derivative is close
   *          to -/+ 1 are already be independently processed, else
   *          it may lead to errors.
   */
  /* we do not recompute the coefficients
     c00 = 1.0 - dx - dy + dxdy;
     c10 = dx - dxdy;
     c01 = dy - dxdy;
     c11 = dxdy;
     fl_upper_left = norme + (x_upper_left_corner + y_upper_left_corner * dimx);
     interpolated_norme = *(fl_upper_left) * c00 +
     *(fl_upper_left + 1) * c10 +
     *(fl_upper_left + dimx) * c01 +
     *(fl_upper_left + dimxPlusOne) * c11;
     */
  fl_upper_left = x_upper_left_corner + y_upper_left_corner * dimx;
  interpolated_norme = mod[fl_upper_left] * c11 +
    mod[fl_upper_left + 1] * c01 +
    mod[fl_upper_left + dimx] * c10 +
    mod[fl_upper_left + dimxPlusOne] * c00;
  /*
   * Last test to decide whether or not we 
   * have an extrema
   */
  if ( norme < interpolated_norme ) {
    max[ij] = 0.0f;
    return;
  }
  /*
   * We found a gradient extrema.
   */
  max[ij] = 1;

}

