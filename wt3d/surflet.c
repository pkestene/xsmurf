/*
 * surflet.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *   Copyright 2002 Laboratoire de Physique, ENS Lyon
 *  Written by Pierre Kestener.
 *
 *  The author may be reached (Email) at the address
 *      pierre.kestener@ens-lyon.fr
 *
 */

#include "surflet.h"
#include "wt3d_int.h"

#ifndef update_mean
#define update_mean(mean, val) (mean=(mean*surflet->size+(val))/(surflet->size+1))
#endif

/*
 * create_surflet --
 *
 *   Allocate memory and initialize the fields of a Surflet structure.
 *
 * Arguments:
 *   int        - Value of the width of the 3D ext image.
 *   ExtImage3D - Pointer to the ext image.
 *   real       - Scale of the 3D ext image.
 *
 * Return value:
 *   Pointer to the new surflet. 0 if allocation fails.
 */

Surflet *
create_surflet (int lx,
		ExtImage3D *ext_image3D,
		real scale)
{
  Surflet *surflet;

  surflet = (Surflet *) malloc (sizeof (Surflet));
  if (!surflet) {
    EX_RAISE(mem_alloc_error);
  }

  surflet->ext3D_lst = lst_create ();
  if (!surflet->ext3D_lst) {
    EX_RAISE(mem_alloc_error);
  }
  surflet->gr_lst = lst_create ();
  if (!surflet->gr_lst) {
    EX_RAISE(mem_alloc_error);
  }

  surflet->type = SURFLET_OPENED;
  surflet->size = 0;
  surflet->lx = lx;
  surflet->scale = scale;
  surflet->mass = 0.0;
  surflet->gravity_center_x = 0.0;
  surflet->gravity_center_y = 0.0;
  surflet->gravity_center_z = 0.0;
  surflet->meanradius = 0.0;
  surflet->meansquareradius = 0.0;
  surflet->ext_image3D = ext_image3D;
  surflet->ext_chain3D = NULL;
  surflet->greater_ext = NULL;
  surflet->smaller_ext = NULL;
  surflet->nb_of_gr = 0;
  surflet->tag = 0;

  return surflet;

mem_alloc_error:
  if (surflet) {
    lst_destroy (surflet->ext3D_lst);
    lst_destroy (surflet->gr_lst);
  }
  free (surflet);

  return 0;
}


/*
 * destroy_surflet --
 *
 *   Free all aloccated memory of a surflet.
 *
 * Arguments:
 *   Surflet * - The surflet.
 *
 * Return value:
 *   None.
 */

void
destroy_surflet (Surflet *surflet)
{
  if (surflet) {
    lst_destroy (surflet->ext3D_lst);
    lst_destroy (surflet->gr_lst);
  }
  free (surflet);

  return;
}


/*
 * add_extremum3D_to_surflet --
 *
 *   Add an 3D extremum to a surflet and update its statistic's fields.
 *   Called by _seek_3D_neighbors_ in $(XSMURFDIR)/wt3d/chain3d.c
 *
 * Arguments:
 *   Extremum3D * - The Extremum3D to add.
 *   Surflet *    - The surflet.
 *   int          - Where to add the extremum3D: SURFLET_BEG for the beginning 
 *                  of the surflet, SURFLET_END for the end of the surflet.
 *
 * Return value:
 *   None.
 */

void
add_extremum3D_to_surflet (Extremum3D *extremum3D,
			   Surflet    *surflet,
			   int         lx,
			   int         ly,
			   int         where)
{
  int tmp;

  if (where == SURFLET_BEG) {
    lst_add_beg (extremum3D, surflet->ext3D_lst);
  } else {
    lst_add_end (extremum3D, surflet->ext3D_lst);
  }

  surflet->mass = surflet->mass + extremum3D->mod;
  update_mean(surflet->gravity_center_x, extremum3D->pos % lx);
  tmp = extremum3D->pos / lx;
  update_mean(surflet->gravity_center_y, tmp % ly);
  update_mean(surflet->gravity_center_z, tmp / ly);
  surflet->size ++;

  if (!surflet->greater_ext
      || (extremum3D->mod) > (surflet->greater_ext->mod)) {
    surflet->greater_ext = extremum3D;
  }

  if (!surflet->smaller_ext
      || (extremum3D->mod) < (surflet->smaller_ext->mod)) {
   surflet->smaller_ext = extremum3D;
  }

  extremum3D->next = (void*) surflet;

  return;
}

