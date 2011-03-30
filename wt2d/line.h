/*
 * line.h --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: line.h,v 1.8 1999/05/22 17:07:20 decoster Exp $
 */

#ifndef __LINE_H__
#define __LINE_H__

#include "wt2d.h"

/*
 * Line's types.
 */

enum {
  LINE_CLOSED,
  LINE_OPENED
};


/* we define struct Line used to chain extrema
   see also ExtImage structure */
typedef struct Line
{
  int      type;        /* CLOSED or OPEN */
  List     *ext_lst;
  int      lx;          /* see Ext_image->lx */
  int      size;        /* nb of extrema in that line */
  real     scale;
  real     mass;        /* total sum of module on that line */
  real     mean_arg;
  
  /* these 2 fields are filled by hsearch command */ 
  real     gravity_center_x; /* new : march 2002 */
  real     gravity_center_y; /* new : march 2002 */
  
  real     meanradius;  /* new : may 2002 */
  real     meansquareradius;/* new : may 2002 */

  real     area; /* 02 dec 2005
		  * take care that area is non-zero only if 
		  * the line is closed */

   real     diameter;
   int      perimeter;

  ExtImage *ext_image;
  Extremum *ext_chain;
  Extremum *greater_ext;/* The extremum of the line that has the greater
			 * modulus.*/
  Extremum *smaller_ext;/* The extremum of the line that has the smaller
			 * modulus.*/
  int      nb_of_gr;	/* Number of local maxima on the line. */
  List     *gr_lst;	/* List of the local maxima on the line. */
  int      tag;
} Line;


Line * create_line	(int, ExtImage *, real);
void   destroy_line	(Line *);

/*
 *   Enum used for the value of the last argument of add_extremum_to_line.
 */

enum {
  LINE_END,
  LINE_BEG,
};

void   add_extremum_to_line (Extremum *, Line *, int, int);

/* The following prototypes must be placed somewhere else... */

Line * w2_remove_line			(ExtImage *, Line *);
void   w2_remove_lines_by_size		(ExtImage *, int, int);
void   w2_remove_lines_by_mean_mod 	(ExtImage *, real, real);
void   w2_remove_lines_by_arg		(ExtImage *);

#endif /*__LINE_H__*/
