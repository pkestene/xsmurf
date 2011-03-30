/*
 * surflet.h --
 *
 *   Copyright 2002 Laboratoire de Physique, Ens Lyon.
 *  Written by Pierre Kestener.
 *
 *  The author may be reached (Email) at the address
 *      pierre.kestener@ens-lyon.fr
 *
 */

/* this file is based on ../wt2d/line.h */

#ifndef __SURFLET_H__
#define __SURFLET_H__

#include "wt3d.h"

/*
 * Surflet's types.
 */

enum {
  SURFLET_CLOSED,
  SURFLET_OPENED
};


/* we define struct Surflet used to chain extremum3D
   see also ExtImage3D structure */
typedef struct Surflet
{
  int      type;        /* CLOSED or OPEN */
  List     *ext3D_lst;
  int      lx;          /* see Ext_image3D->lx */
  int      size;        /* nb of extrema in that line */
  real     scale;
  real     mass;        /* total sum of module on that surflet */
  
  /* these 2 fields are filled by hsearch command */ 
  real     gravity_center_x;
  real     gravity_center_y;
  real     gravity_center_z;
  
  real     meanradius;  
  real     meansquareradius;
  
  ExtImage3D *ext_image3D;
  Extremum3D *ext_chain3D;
  Extremum3D *greater_ext;/* The extremum3D of the surflet that has the
			     greater modulus.*/
  Extremum3D *smaller_ext;/* The extremum of the line that has the smaller
			   * modulus.*/
  int      nb_of_gr;	  /* Number of local maxima on the line. */
  List     *gr_lst;	  /* List of the local maxima on the line. */
  int      tag;
} Surflet;


Surflet * create_surflet    (int, ExtImage3D *, real);
void   destroy_surflet      (Surflet *);

/*
 *   Enum used for the value of the last argument of add_extremum_to_line.
 */

enum {
  SURFLET_END,
  SURFLET_BEG,
};

void   add_extremum3D_to_surflet (Extremum3D *, Surflet *, int, int, int);


/* The following prototypes must be placed somewhere else... */
Surflet * w3_remove_surflet		(ExtImage3D *, Surflet *);
void   w3_remove_surflets_by_size      	(ExtImage3D *, int, int);
void   w3_remove_surflets_by_mean_mod 	(ExtImage3D *, real, real);

#endif /*__SURFLET_H__*/
