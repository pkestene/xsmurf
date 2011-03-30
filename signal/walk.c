/*
 * walk.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: walk.c,v 1.2 1998/08/19 14:57:01 decoster Exp $
 */

#include "signal_int.h"
#include "assert.h"

/*
 * Give the index of the data which absciss is the nearest of the
 * value of X.
 */
int
sig_get_index (Signal *signal,
	       real   x)
{
  int    i;
  int    result;
  real   distance;
  real   *data;
  real   x0, dx;

  assert (signal != NULL);

  data = signal->dataX;
  x0   = signal->x0;
  dx   = signal->dx;

  switch (signal->type)
    {
    case REALY:
    case CPLX:
    case FOUR_NR:
      result = (int) floor ((x - x0)/dx + 0.5);
      if (result < 0)
	result = 0;
      else if (result > (signal->n - 1))
	result = signal->n - 1;
      break;
    case REALXY:
      /* For now, this fct assumes that REALXY signals have dataX sorted
       * increasly. Must be modified... */
      distance = x - data [0];
      if (distance <= 0)
	result = 0;
      else {
	for (i = 1;
	     i < signal->n && ((x - data[i]) > 0);
	     i++)
	  distance = x - data [i];
	if ((x - data[i]) <= 0)
	  {
	    if (distance < fabs (x - data[i]))
	      result = i - 1;
	    else
	      result = i;
	  }
	else
	  result = signal->n - 1;
      }
      break;
    }

  assert (result < signal->n);
  assert (result >= 0);

  return result;
}


