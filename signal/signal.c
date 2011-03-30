/*
 * signal.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: signal.c,v 1.10 1998/08/19 14:57:01 decoster Exp $
 */

#include "signal_int.h"

/*
 * Signal constructor.
 */
Signal *
sig_new (int type,
	 int first,
	 int last)
{
  Signal *signal;

  signal = (Signal *) malloc (sizeof (Signal));
  if (!signal)
    return NULL;

  signal->type  = type;
  signal->first = first;
  signal->last  = last;
  signal->x0 = 0.0;
  signal->dx = 1.0;

  signal->n = last - first + 1;

  switch (type)
    {
    case REALY:
      signal->x0 = 0.0;
      signal->dx = 1.0;
      signal->size = signal->n;
      signal->dataY = (real *) malloc (sizeof (real)*signal->size);
      if (!signal->dataY)
	return NULL;
      /*      signal->dataY = &(signal->dataY[-first]);*/
      signal->dataX = NULL;
      break;

    case REALXY :
      signal->size = signal->n;
      signal->dataY = (real *) malloc (sizeof (real)*signal->size);
      if (!signal->dataY)
	return NULL;
      signal->dataX = (real *) malloc (sizeof (real)* signal->size);
      if (!signal->dataX)
	return NULL;
      /*      signal->dataX = &(signal->dataX[-first]);
      signal->dataY = &(signal->dataY[-first]);*/
      signal->dx = 0.0;
      break;

    case CPLX:
    case FOUR_NR:
      signal->size = 2*signal->n;
      signal->dataY = (real *) malloc (sizeof (real)*signal->size);
      if (!signal->dataY)
	return NULL;
      /*      signal->dataY = &(signal->dataY[-2*first]);*/
      signal->dataX = NULL;
      break;
    }

  return signal;
}

/*
 * Signal destructor
 */
void
sig_free (Signal *signal)
{
  if (signal->dataX)
    free (signal->dataX);
  free (signal->dataY);
  free (signal);
}

/*
 * Search the minimum value and the maximum value of an array of real values
 * from BEGIN index to END index.
 */
static void
_get_real_extrema_ (real *data,
                    int  begin,
                    int  end,
                    real *min_ptr,
                    real *max_ptr,
                    int  *imin_ptr,
                    int  *imax_ptr)
     /*_get_real_extrema_ (real *data,
		    int  begin,
		    int  end,
		    real *min_ptr,
		    real *max_ptr)*/
{
  register int i;

  (*min_ptr) = (*max_ptr) = data[begin];
  for (i = begin; i <= end; i++)
    if (data[i] > (*max_ptr)){
      (*max_ptr) = data[i];
      (*imax_ptr) = i;
    } else {
      if (data[i] < (*min_ptr)) {
	(*min_ptr) = data[i];
        (*imin_ptr) = i;
      }
    }
}

/*
 * Search the minimum real and imaginary values and the maximum real and
 * imaginary value of an array of complex values from BEGIN index to END
 * index.
 */
static void
_get_complex_extrema_ (complex *data,
		       int     begin,
		       int     end,
		       complex *min_ptr,
		       complex *max_ptr)
{
  register int i;

  (*min_ptr) = (*max_ptr) = data[begin];
  for (i = begin; i <= end; i++)
    {
      if (data[i].real > max_ptr->real)
	max_ptr->real = data[i].real;
      else
	if (data[i].real < min_ptr->real)
	  min_ptr->real = data[i].real; 
      
      if (data[i].imag > max_ptr->imag)
	max_ptr->imag = data[i].imag;
      else
	if (data[i].imag < min_ptr->imag)
	  min_ptr->imag = data[i].imag; 
    }
}

/*
 * Search the minimum value and the maximum value of a signal from BEGIN
 * index to END index. The way the search is done depends on the type of
 * the signal (see below).
 */
void
sig_get_extrema_between (Signal *signal,
			 int    begin,
			 int    end,
			 void   *min_ptr,
			 void   *max_ptr,
			 void   *imin_ptr,
			 void   *imax_ptr)
{
  begin = max (0, begin);
  end   = min (signal->n - 1, end);
  switch (signal->type)
    {
      /*
       * Min and max of "signal->dataY".
       */
    case REALY:
    case REALXY:
	_get_real_extrema_ (signal->dataY,
			    begin, end,
			    (real *) min_ptr,
			    (real *) max_ptr,
			    (int  *) imin_ptr,
			    (int  *) imax_ptr);
	/*    case REALXY:
	_get_real_extrema_ (signal->dataY,
			    begin, end,
			    (real *) min_ptr,
			    (real *) max_ptr);*/
	break;
      /*
       * Min and max of real part of "signal->data". Min and Max of
       * the imaginary part of "signal->dataY".
       */
    case CPLX:
    case FOUR_NR:
	_get_complex_extrema_ ((complex *)signal->dataY,
			       begin, end,
			       (complex *) min_ptr,
			       (complex *) max_ptr);
	break;
    }
}

/*
 * Search the minimum value and the maximum value of a signal.
 */
void
sig_get_extrema (Signal *signal,
		 void   *min_ptr,
		 void   *max_ptr,
		 void   *imin_ptr,
		 void   *imax_ptr)
{
  sig_get_extrema_between (signal,
			   0, signal->n - 1,
			   min_ptr, max_ptr, imin_ptr, imax_ptr);
}

int
sig_get_xy_extrema (Signal *signal,
                    real   *x_min_ptr,
                    real   *x_max_ptr,
                    real   *y_min_ptr,
                    real   *y_max_ptr,
                    int    *i_min_ptr,
                    int    *i_max_ptr)
{
  int  i;
  real x, y;

  if (*x_max_ptr < *x_min_ptr && *y_max_ptr < *y_min_ptr) {
    *x_min_ptr = *x_max_ptr = signal->dataX[0];
    *y_min_ptr = *y_max_ptr = signal->dataY[0];
    for (i = 1; i < signal->size; i++) {
      x = signal->dataX[i];
      if (x < *x_min_ptr) *x_min_ptr = x;
      if (x > *x_max_ptr) *x_max_ptr = x;
      y = signal->dataY[i];
      if (y < *y_min_ptr) {
        *y_min_ptr = y;
        *i_min_ptr = i;
      }
      if (y > *y_max_ptr) {
        *y_max_ptr = y;
        *i_max_ptr = i;
      }
    }
    return 1;
  }

  if (*x_max_ptr < *x_min_ptr) {
    for (i = 0; *x_min_ptr != *x_max_ptr && i < signal->size; i++) {
      y = signal->dataY[i];
      if (y >= *y_min_ptr && y <= *y_max_ptr) {
        *x_min_ptr = *x_max_ptr = signal->dataX[i];
      }
    }
    for (; i < signal->size; i++) {
      y = signal->dataY[i];
      if (y >= *y_min_ptr && y <= *y_max_ptr) {
        x = signal->dataX[i];
        if (x < *x_min_ptr) *x_min_ptr = x;
        if (x > *x_max_ptr) *x_max_ptr = x;
      }
    }
    if (*x_max_ptr < *x_min_ptr) {
      return 0;
    } else {
      return 1;
    }
  }

  if (*y_max_ptr < *y_min_ptr) {
    for (i = 0; *y_min_ptr != *y_max_ptr && i < signal->size; i++) {
      x = signal->dataX[i];
      if (x >= *x_min_ptr && x <= *x_max_ptr) {
        *y_min_ptr = *y_max_ptr = signal->dataY[i];
      }
    }
    for (i--; i < signal->size; i++) {
      x = signal->dataX[i];
      if (x >= *x_min_ptr && x <= *x_max_ptr) {
        y = signal->dataY[i];
        if (y < *y_min_ptr) {
          *y_min_ptr = y;
          *i_min_ptr = i;
        }
        if (y > *y_max_ptr) {
          *y_max_ptr = y;
          *i_max_ptr = i;
        }
      }
    }
    if (*y_max_ptr < *y_min_ptr) {
      return 0;
    } else {
      return 1;
    }
  }
  return 0;
}


/*
 * to be continued...
 */
int
sig_get_xy_extrema2 (Signal *signal,
		    real   *x_min_ptr,
		    real   *x_max_ptr,
		    real   *y_min_ptr,
		    real   *y_max_ptr)
{
  int  i;
  real x, y;

  if (*x_max_ptr < *x_min_ptr && *y_max_ptr < *y_min_ptr) {
    *x_min_ptr = *x_max_ptr = signal->dataX[0];
    *y_min_ptr = *y_max_ptr = signal->dataY[0];
    for (i = 1; i < signal->size; i++) {
      x = signal->dataX[i];
      if (x < *x_min_ptr) *x_min_ptr = x;
      if (x > *x_max_ptr) *x_max_ptr = x;
      y = signal->dataY[i];
      if (y < *y_min_ptr) *y_min_ptr = y;
      if (y > *y_max_ptr) *y_max_ptr = y;
    }
    return 1;
  }

  if (*x_max_ptr < *x_min_ptr) {
    for (i = 0; *x_min_ptr != *x_max_ptr && i < signal->size; i++) {
      y = signal->dataY[i];
      if (y >= *y_min_ptr && y <= *y_max_ptr) {
	*x_min_ptr = *x_max_ptr = signal->dataX[i];
      }
    }
    for (; i < signal->size; i++) {
      y = signal->dataY[i];
      if (y >= *y_min_ptr && y <= *y_max_ptr) {
	x = signal->dataX[i];
	if (x < *x_min_ptr) *x_min_ptr = x;
	if (x > *x_max_ptr) *x_max_ptr = x;
      }
    }
    if (*x_max_ptr < *x_min_ptr) {
      return 0;
    } else {
      return 1;
    }
  }

  if (*y_max_ptr < *y_min_ptr) {
    for (i = 0; *y_min_ptr != *y_max_ptr && i < signal->size; i++) {
      x = signal->dataX[i];
      if (x >= *x_min_ptr && x <= *x_max_ptr) {
	*y_min_ptr = *y_max_ptr = signal->dataY[i];
      }
    }
    for (i--; i < signal->size; i++) {
      x = signal->dataX[i];
      if (x >= *x_min_ptr && x <= *x_max_ptr) {
	y = signal->dataY[i];
	if (y < *y_min_ptr) *y_min_ptr = y;
	if (y > *y_max_ptr) *y_max_ptr = y;
      }
    }
    if (*y_max_ptr < *y_min_ptr) {
      return 0;
    } else {
      return 1;
    }
  }
  return 0;
}

/*
 * Duplicate all the fields of a signal.
 */
Signal *
sig_duplicate(Signal *in)
{
  Signal *out;

  out = sig_new (in->type, 0, in->n-1);
  if (out) {
    switch (in->type)	{
    case REALY :
      memcpy (out->dataY, in->dataY, out->size*sizeof (real));
      break;
    case REALXY :
      memcpy (out->dataY, in->dataY, out->size*sizeof (real));
      memcpy (out->dataX, in->dataX, out->size*sizeof (real));
      break;
    case CPLX :
    case FOUR_NR :
      memcpy (out->dataY, in->dataY, out->size*sizeof (real));
      break;
    }
    out->x0 = in->x0;
    out->dx = in->dx;
  }

  return out;
}
