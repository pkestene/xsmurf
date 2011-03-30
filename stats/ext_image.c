/*
 * ext_image.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: ext_image.c,v 1.3 1998/08/19 14:57:02 decoster Exp $
 */

#include "stats_int.h"

Signal *
stat_x_line_size_histo (ExtImage *ext_image,
			real     x_min,
			real     x_max,
			int      nbox)
{
  Signal *result;
  int    i;
  Line   *line;
  int    index;

  result = sig_new (REALY, 0, nbox-1);

  for (i = 0; i < result->n; i++)
    result->dataY[i] = 0.0;

  if (x_min > x_max)
    {
      line = lst_first (ext_image->line_lst);
      x_min = line->size;
      x_max = line->size;
      foreach (line, ext_image->line_lst)
	{
	  if (line->size > x_max)
	    x_max = line->size;
	  if (line->size < x_min)
	    x_min = line->size;
	}
    }

  foreach (line, ext_image->line_lst)
    {
      index = (int)((line->size - x_min)*nbox/(x_max - x_min));
      if (index < 0 || index > nbox)
	continue;
      if (index == nbox)
	index = nbox-1;
      result->dataY[index]++;
    }
    
  result->x0 = x_min + (x_max - x_min)/(2*nbox);
  result->dx = (x_max - x_min)/nbox;

  return result;
}

Signal *
stat_n_line_histo (ExtImage *ext_image,
		   real     x_min,
		   real     x_max,
		   int      nbox)
{
  Signal *result;
  int    i;
  Line   *line;
  int    index;
  float  l_n;

  result = sig_new (REALY, 0, nbox-1);

  for (i = 0; i < result->n; i++)
    result->dataY[i] = 0.0;

  if (x_min > x_max)
    {
      line = lst_first (ext_image->line_lst);
      x_min = x_max = line->size / (float) line->nb_of_gr;
      foreach (line, ext_image->line_lst)
	{
	  if (line->nb_of_gr)
	    {
	      l_n = line->size / (float) line->nb_of_gr;
	      if (l_n > x_max)
		x_max = l_n;
	      if (l_n < x_min)
		x_min = l_n;
	    }
	}
    }

  foreach (line, ext_image->line_lst)
    {
      if (line->nb_of_gr)
	{
	  l_n = line->size / (float) line->nb_of_gr;
	  index = (int)((l_n - x_min)*nbox/(x_max - x_min));
	  if (index < 0 || index > nbox)
	    continue;
	  if (index == nbox)
	    index = nbox-1;
	  result->dataY[index]++;
	}
    }
    
  result->x0 = x_min + (x_max - x_min)/(2*nbox);
  result->dx = (x_max - x_min)/nbox;

  return result;
}
