/*
 * op.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: op.c,v 1.10 1999/07/09 15:28:49 decoster Exp $
 */

#include "signal_int.h"
#include "assert.h"

/*
 *  Macro that returns TRUE if s1 and s2 begin and last at the same index.
 */
#define same_x_domain(s1,s2) (s1->n==s2->n)

/*
 * Add the data of two signals.
 */
Signal *
sig_add (Signal *signal1,
	 Signal *signal2)
{
  Signal *result;
  int    i;

  /* Remonter au niveau superieur les pre-conditions. Mettre un assert a la place */

#ifdef SMURF_DEV
  assert (signal1->type == signal2->type);
  assert (same_x_domain(signal1, signal2));
#endif

  result = sig_new (signal1->type, 0, signal1->n - 1);
  result->dx = signal1->dx;
  result->x0 = signal1->x0;
   if (!result)
    return 0;

  switch (signal1->type)
    {
    case REALXY:
      for (i = 0; i < result->n; i++){
	result->dataY[i] = signal1->dataY[i] + signal2->dataY[i];
	result->dataX[i] = signal1->dataX[i];
      }
      break;
    case REALY:
      for (i = 0; i < result->n; i++)
	result->dataY[i] = signal1->dataY[i] + signal2->dataY[i];
      break;
    case CPLX:
    case FOUR_NR:
      for (i = 0; i < result->n; i++)
	{
	  /* real part */
	  result->dataY[2*i] = signal1->dataY[2*i] + signal2->dataY[2*i];
	  /* imaginary part */
	  result->dataY[2*i+1] = signal1->dataY[2*i+1] + signal2->dataY[2*i+1];
	}
      break;
    }

  return result;
}

/*
 * Multiplicate the data of two signals.
 */
Signal *
sig_mult (Signal *signal1,
	      Signal *signal2)
{
  Signal *result;
  int    i;

  /* Remonter au niveau superieur les pre-conditions. Mettre un assert a la place */
  if(signal1->type != signal2->type)
    ERROR ("Incompatible types.\n");

  if(!same_x_domain(signal1, signal2))
    ERROR ("Incompatible x domains.\n");

  result = sig_new(signal1->type, 0, signal1->n - 1);
  result->dx = signal1->dx;
  result->x0 = signal1->x0;

  switch(result->type)
    {
    case REALY:
      for (i = 0; i < result->n; i++)
	result->dataY[i] = signal1->dataY[i] * signal2->dataY[i];
      break;

    case REALXY:
      for (i = 0; i < result->n; i++)
	result->dataY[i] = signal1->dataY[i] * signal2->dataY[i];
	result->dataX[i] = signal1->dataX[i];
      break;

    case CPLX:
    case FOUR_NR:
      {
	complex *r, *s1, *s2;

	r  = (complex *) result->dataY;
	s1 = (complex *) signal1->dataY;
	s2 = (complex *) signal2->dataY;
	for (i = 0; i < result->n; i++)
	  {
	    r[i].real = s1[i].real*s2[i].real - s1[i].imag*s2[i].imag;
	    r[i].imag = s1[i].real*s2[i].imag + s1[i].imag*s2[i].real;
	  }
      }
      break;
    }
  return result;
}

/*
 * Multiplicate the data of a signal by a scalar.
 */
Signal *
sig_scalar_mult (Signal *signal,
		  real   scalar)
{
  Signal *result;
  int    i;
  int    n;
  real   *data, *rdata;

  data  = signal->dataY;
  n = signal->n;

  result = sig_new (signal->type, 0, signal->n - 1);
  result->dx = signal->dx;
  result->x0 = signal->x0;
  rdata = result->dataY;


  switch(result->type)
    {
    case REALY:
      for(i = 0; i < n; i++)
	rdata[i] = scalar*data[i];
      break;

    case REALXY:
      for(i = 0; i < n; i++)
	{
	  rdata[i] = scalar*data[i];
	  result->dataX[i] = signal->dataX[i];
	}
      break;

    case CPLX:
    case FOUR_NR:
      for(i = 0; i < n; i++)
	{
	  /* real part */
	  rdata[2*i] = scalar*data[2*i];
	  /* imaginary part */
	  rdata[2*i+1] = scalar*data[2*i+1];
	}
      break;
    }

  return result;
}


/*
 * Create a new signal which x domain is cut.
 */

Signal *
sig_cut (Signal *signal,
	 int    left_cut,
	 int    rigth_cut)
{
  Signal *result;
  int    i;
  int    j;
  int    n;

  switch (signal->type) {
  case REALY:
  case REALXY:
    n = signal->size;
    break;
  case FOUR_NR:
  case CPLX:
    n = signal->size/2;
    break;
  }

  if (left_cut < 0) {
    left_cut = 0;
  }
  if (rigth_cut < 0) {
    rigth_cut = 0;
  }

  if ((rigth_cut + left_cut) >= n) {
    result = sig_new (signal->type, 0, 0);
    return result;
  }

  result = sig_new (signal->type, 0, n - rigth_cut - left_cut - 1);

  switch (signal->type) {
  case REALY:
    result->dx = signal->dx;
    result->x0 = signal->x0 + signal->dx*left_cut;
    for(i = left_cut, j = 0; i < n-rigth_cut; i++, j++) {
      result->dataY[j] = signal->dataY[i];
    }
    break;
  case REALXY:
    for(i = left_cut, j = 0; i < n-rigth_cut; i++, j++) {
      result->dataY[j] = signal->dataY[i];
      result->dataX[j] = signal->dataX[i];
    }
    break;
  case FOUR_NR:
  case CPLX:
    result->dx = signal->dx;
    result->x0 = signal->x0 + signal->dx*left_cut;
    for(i = left_cut, j = 0; i < n-rigth_cut; i++, j++) {
      result->dataY[2*j] = signal->dataY[2*i];
      result->dataY[2*j+1] = signal->dataY[2*i+1];
    }
    break;
  }

  return result;
}

#define same_x_domain(s1,s2) (s1->n==s2->n)

/*
 *  Create a FOUR_NR signal from a REALY signal.
 */
Signal *
sig_real_to_fourier (Signal *signal)
{
  Signal  *result;
  int     i;
  real    *data;
  complex *rdata;
  int     size;

  assert(is_power_of_2_(signal->n));

  data  = signal->dataY;

  result = sig_new (FOUR_NR, 0, size-1);

  rdata  = (complex *)result->dataY;

  for(i = 0; i < size; i++)
    {
      rdata[i].real = data[i];
      rdata[i].imag = 0.0;
    }

  return result;
}

/*
 *  This function transform an signal in the gfft format to 2 signals, one for the
 * real part and one for the imaginary part. RES_REAL and RES_IMAG must have
 * been allocated with the same parameters than SOURCE.
 */
void
sig_gfft_to_2real (Signal *source,
		   Signal *res_real,
		   Signal *res_imag)
{
  complex *sdata;
  real *rdata;
  real *idata;
  int size;
  int i;

  sdata = (complex *) source->dataY;
  rdata = res_real->dataY;
  idata = res_imag->dataY;
  size = res_real->size;

  rdata[0] = sdata[0].imag;
  idata[0] = 0;
  for (i = 1; i < size/2; i++) {
    rdata[size/2+i] = sdata[i].real;
    idata[size/2+i] = sdata[i].imag;
    rdata[size/2-i] = sdata[i].real;
    idata[size/2-i] = -sdata[i].imag;
  }
  rdata[size/2] =  sdata[0].real;
  idata[size/2] =  0;
}

/*
 */
void
sig_2real_to_gfft (Signal *result,
		   Signal *src_real,
		   Signal *src_imag)
{
  complex *data;
  real *rdata;
  real *idata;
  int size;
  int i;

  data = (complex *) result->dataY;
  rdata = src_real->dataY;
  idata = src_imag->dataY;
  size = src_real->size;

  data[0].imag = rdata[0];
  for (i = 1; i < size/2; i++) {
    data[i].real = rdata[size/2+i];
    data[i].imag = idata[size/2+i];
  }
  data[0].real = rdata[size/2];
}

/*
 * Transform a REALY signal S into a REALXY signal. Returns 0 in case of malloc
 * error, S if succesfull.
 */

Signal *
sig_realy2realxy (Signal *s)
{
  real *data;
  int  i;

  assert(s->type == REALY);

  data = (real *) malloc(sizeof(real)*s->n);
  if (!data) {
    return 0;
  }

  s->dataX = data;

  for (i = 0; i < s->n; i++) {
    s->dataX[i] = s->x0 + i*s->dx;
  }

  s->type = REALXY;

  assert(s->type == REALXY);

  return s;
}


/*
 * S is a REALXY signal. Put the values of dataY in dataX. dataY is unchanged.
 * Returns S.
 */

Signal *
sig_put_y_in_x (Signal *s)
{
  int  i;

  assert(s->type == REALXY);

  for (i = 0; i < s->n; i++) {
    s->dataX[i] = s->dataY[i];
  }

  return s;
}


#undef same_x_domain
