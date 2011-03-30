/*
 * wt1d_collection.c
 *
 * Definition of usual wavelets.
 *
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *   $Id: wt1d_collection.c,v 1.11 1998/11/24 16:13:05 decoster Exp $
 */

#include "wt1d_int.h"

#define SQRT_PI  1.77245385090551602729
#define SQRT_2PI 2.50662827463100050241

/*
 *  For all the predefined wavelets the domains are defined as [x_min x_max],
 * with :
 *   x_min : the first point that is greater than 0.000001*(max value)
 *   x_max : the last point that is greater than 0.000001*(max value)
 */

double
null_fct (double x,
	  double a)
{
  return 0.0;
}


/*
 * Definition of the gaussian wavelet.
 */

double
d_r_gauss (double x)
{
  return exp(-x*x/2);
}

double
f_r_gauss (double k)
{
  return SQRT_2PI*exp(-k*k/2);
}

Wavelet _gaussian_ = {
  WAVE_REAL_REAL,
  &d_r_gauss,
  NULL,
  &f_r_gauss,
  NULL,
  -5.29296875,
  5.29296875,
  -5.44921875,
  5.44921875,
  1.0,
  1.0
};

Wavelet *wt1d_gaussian_ptr = &_gaussian_;

/*
 * Definition of the first derivative of the gaussian wavelet.
 */

double
d_r_d1_gauss (double x)
{
  return -x*exp(-x*x/2);
}

double
f_i_d1_gauss (double k)
{
  return SQRT_2PI*k*exp(-k*k/2);
}

Wavelet _d1_gaussian_ = {
  WAVE_REAL_IMAG,
  &d_r_d1_gauss,
  NULL,
  &null_fct,
  &f_i_d1_gauss,
  -5.60546875,
  5.60546875,
  -5.76171875,
  5.76171875,
  1.0,
  1.0
};

Wavelet *wt1d_d1_gaussian_ptr = &_d1_gaussian_;

/*
 * Definition of the second derivative of the gaussian wavelet.
 */

double
d_r_d2_gauss (double x)
{
  register double x2 = x*x;
  return (x2-1)*exp(-x2/2);
}

double
f_r_d2_gauss (double k)
{
  register double k2 = k*k;
  return -SQRT_2PI*k2*exp(-k2/2);
}

Wavelet _d2_gaussian_ = {
  WAVE_REAL_REAL,
  &d_r_d2_gauss,
  NULL,
  &f_r_d2_gauss,
  NULL,
  -5.91796875,
  5.91796875,
  -6.09375,
  6.09375,
  1.0,
  1.0
};

Wavelet *wt1d_d2_gaussian_ptr = &_d2_gaussian_;

/*
 * Definition of the third derivative of the gaussian wavelet.
*/

double
d_r_d3_gauss (double x)
{
  register double x2 = x*x;
  double gah;
  gah = x*(3-x2)*exp(-x2/2);
  return gah;
}

double
f_i_d3_gauss (double k)
{
  register double k2 = k*k;
  return -SQRT_2PI*k*k2*exp(-k2/2);
}

Wavelet _d3_gaussian_ = {
  WAVE_REAL_IMAG,
  &d_r_d3_gauss,
  NULL,
  &null_fct,
  &f_i_d3_gauss,
  -6.23046875,
  6.23046875,
  -6.40625,
  6.40625,
  1.0,
  1.0
};

Wavelet *wt1d_d3_gaussian_ptr = &_d3_gaussian_;

/*
 * Definition of the fourth derivative of the gaussian wavelet.
 */

double
d_r_d4_gauss (double x)
{
  register double x2 = x*x;
  return (3-6*x2+x2*x2)*exp(-x2/2);
}

double
f_r_d4_gauss (double k)
{
  register double k2 = k*k;
  return SQRT_2PI*k2*k2*exp(-k2/2);
}

Wavelet _d4_gaussian_ = {
  WAVE_REAL_REAL,
  &d_r_d4_gauss,
  NULL,
  &f_r_d4_gauss,
  NULL,
  -6.54296875,
  6.54296875,
  -6.71875,
  6.71875,
  1.0,
  1.0
};

Wavelet *wt1d_d4_gaussian_ptr = &_d4_gaussian_;

/*
 * Definition of the Morlet wavelet.
 */

#define OMEGA 5.336446

double
d_r_morlet (double x)
{
  return cos(OMEGA*x)*exp(-x*x/2);
}

double
d_i_morlet (double x)
{
  return sin(OMEGA*x)*exp(-x*x/2);
}

double
f_r_morlet (double k)
{
  register double ko = (k-OMEGA);
  return SQRT_2PI*exp(-ko*ko/2);
}

Wavelet _morlet_ = {
  WAVE_CPLX_REAL,
  &d_r_morlet,
  &d_i_morlet,
  &f_r_morlet,
  NULL,
  -5.26,
  5.26,
  -0.1,
  10.9,
  1.0,
  1.0
};

Wavelet *wt1d_morlet_ptr = &_morlet_;

