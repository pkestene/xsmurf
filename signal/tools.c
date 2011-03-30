/*
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster and Stephane Roux.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *  or  decoster@info.enserb.u-bordeaux.fr
 *
 */

#include "signal_int.h"

/*
 */
int
next_power_of_2_ (int i)
{
  int j;

  for (j = 1; j < i; j *= 2);
  return j;
}

/*
 */
int
is_power_of_2_ (int i)
{
  real j;

  for (j = i; j > 1; j /= 2);

  return (j == 1);
}
