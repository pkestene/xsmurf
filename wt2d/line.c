/*
 * line.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: line.c,v 1.8 1999/05/22 17:06:02 decoster Exp $
 */

#include "wt2d_int.h"

#ifndef update_mean
#define update_mean(mean, val) (mean=(mean*line->size+(val))/(line->size+1))
#endif

/*
 * create_line --
 *
 *   Allocate memory and initialize the fields of a Line structure.
 *
 * Arguments:
 *   int      - Value of the width of the ext image.
 *   ExtImage - Pointer to the ext image.
 *   real     - Scale of the ext image.
 *
 * Return value:
 *   Pointer to the new line. 0 if allocation fails.
 */

Line *
create_line (int lx,
	     ExtImage *ext_image,
	     real scale)
{
  Line *line;

  line = (Line *) malloc (sizeof (Line));
  if (!line) {
    EX_RAISE(mem_alloc_error);
  }

  line->ext_lst = lst_create ();
  if (!line->ext_lst) {
    EX_RAISE(mem_alloc_error);
  }
  line->gr_lst = lst_create ();
  if (!line->gr_lst) {
    EX_RAISE(mem_alloc_error);
  }

  line->type = LINE_OPENED;
  line->size = 0;
  line->lx = lx;
  line->scale = scale;
  line->mass = 0.0;
  line->mean_arg = 0.0;
  line->gravity_center_x = 0.0;
  line->gravity_center_y = 0.0;
  line->meanradius = 0.0;
  line->meansquareradius = 0.0;
  line->area = 0.0;
  line->diameter = 0.0;
  line->perimeter = 0;
  line->ext_image = ext_image;
  line->ext_chain = NULL;
  line->greater_ext = NULL;
  line->smaller_ext = NULL;
  line->nb_of_gr = 0;
  line->tag = 0;

  return line;

mem_alloc_error:
  if (line) {
    lst_destroy (line->ext_lst);
    lst_destroy (line->gr_lst);
  }
  free (line);

  return 0;
}


/*
 * destroy_line --
 *
 *   Free all aloccated memory of a line.
 *
 * Arguments:
 *   Line * - The line.
 *
 * Return value:
 *   None.
 */

void
destroy_line (Line *line)
{
  if (line) {
    lst_destroy (line->ext_lst);
    lst_destroy (line->gr_lst);
  }
  free (line);

  return;
}


/*
 * add_extremum_to_line --
 *
 *   Add an extremum to a line and update its statistic's fields.
 *   Called by _seek_neighbors_ in $(XSMURFDIR)/wt2d/chain.c
 *
 * Arguments:
 *   Extremum * - The extremum to add.
 *   Line *     - The line.
 *   int        - width of the ext image. (this arg must be removed...)
 *   int        - Where to add the extremum: LINE_BEG for the beginning of the
 *                line, LINE_END for the end of the line.
 *
 * Return value:
 *   None.
 */

void
add_extremum_to_line (Extremum *extremum,
		      Line     *line,
		      int      lx,
		      int      where)
{
  if (where == LINE_BEG) {
    lst_add_beg (extremum, line->ext_lst);
  } else {
    lst_add_end (extremum, line->ext_lst);
  }

  line->mass = line->mass + extremum->mod;
  update_mean(line->mean_arg, extremum->arg);
  update_mean(line->gravity_center_x, extremum->pos % lx);
  update_mean(line->gravity_center_y, extremum->pos / lx);
  line->size ++;

  if (!line->greater_ext
      || (extremum->mod) > (line->greater_ext->mod)) {
   line->greater_ext = extremum;
  }

  if (!line->smaller_ext
      || (extremum->mod) < (line->smaller_ext->mod)) {
   line->smaller_ext = extremum;
  }

  extremum->next = (void*) line;

  return;
}

