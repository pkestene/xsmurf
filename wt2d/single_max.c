/*
 * single_max.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: single_max.c,v 1.14 1999/04/13 15:18:18 decoster Exp $
 */

#include "wt2d_int.h"

static int pos_incr[10];

/*
 * Clockwise order for neighbour points of a given points.
                0   1   2

                7   9   3

		6   5   4
   Note that 0 and 8 describes the same position
*/
static void
init_pos_incr(int lx)
{
  pos_incr[0] = -1 - lx;
  pos_incr[1] =    - lx;
  pos_incr[2] =  1 - lx;
  pos_incr[3] =  1;
  pos_incr[4] =  1 + lx;
  pos_incr[5] =      lx;
  pos_incr[6] = -1 + lx;
  pos_incr[7] =  -1;
  pos_incr[8] = -1 - lx;
  pos_incr[9] =  0;
}

/* function used bellow in search_single_max */
/* create a copy of array image->ext */
static void
_create_ext_array_(image, array)
     ExtImage * image;
     Extremum ** array;
{
  int i;
  
  for (i = 0; i < image->lx*image->ly; i++) {
    array[i] = NULL;
  }
  for (i = 0; i < image->extrNb; i++) {
    array[image->extr[i].pos] = &(image->extr[i]);
  }
}


static int
_is_in_image_(int lx,
	      int ly,
	      int pos)
{
  int x = pos%lx;
  int y = pos/lx;

  return ((x >= 0) && (x < lx) && (y >= 0) && (y < ly));
}


static int
_is_single_max_ (Extremum *ext_ptr,
		 Extremum **array,
		 int      lx,
		 int      ly)
{
  int i;
  int flag = 0;
  int pos1, pos2;

  for (i = 0; i < 8; i++) {
    /* Position of the first neighbour point. */
    pos1 = ext_ptr->pos + pos_incr[i];
    /* Position of the second neighbour point. */
    pos2 = ext_ptr->pos + pos_incr[i+1];

    /*
     * The first neighbour point must be in the image. If this is true, it must
     * be an extrema AND the second neighbour point must not contain an extrema
     * (or it must be outside of the image).
     */
    if (_is_in_image_(lx, ly, pos1)
	&& (array[pos1] && (!_is_in_image_(lx, ly, pos2) || !array[pos2]) ) ) {
      flag ++;
    }
  }
  if (flag != 2) {
    return 0;
  }
  for (i = 0; i < 8; i++) {
    pos1 = ext_ptr->pos + pos_incr[i];
    if (_is_in_image_(lx, ly, pos1)
	&& array[pos1]
	&& ext_ptr->mod < array[pos1]->mod) {
      return 0;
    }
  }
  return 1;
}


/*
 * search_single_max --
 *
 *   Search the maxima of the modulus along lines of ext images. Foreach line
 * the field gr_lst is fill with all these maxima. The ends of a line can't be
 * in this list.
 *
 * Arguments :
 *   image - Ext image to treat
 *
 * Return value :
 *   The number of maxima found in all the image.
 */

int
search_single_max (ExtImage *image)
{
  Extremum **array;
  Extremum *ext_ptr;
  Line     *line_ptr;
  int      lx, ly;
  int      nb_of_sm = 0;

  lx = image->lx;
  ly = image->ly;
  array = (Extremum **) malloc (lx*ly*sizeof(Extremum *));
  if (!array) {
    EX_RAISE(mem_alloc_error);
  }

  /* create a copy of array image->ext precisely named array !!!*/
  _create_ext_array_ (image, array);
  /* fill pos_incr array with neighbour positions */
  init_pos_incr (lx);

  /* */
  foreach (line_ptr, image->line_lst) {
    foreach (ext_ptr, line_ptr->ext_lst) {
      if (_is_single_max_ (ext_ptr, array, lx, ly)) {
	lst_add (ext_ptr, line_ptr->gr_lst);
	line_ptr->nb_of_gr++;
	nb_of_sm++;
      }
    }
  }

  free (array);

  return nb_of_sm;

mem_alloc_error:
  free (array);

  return 0;
}


/* A mettre ailleur... */

extern int min_max_flag;
enum {
  _MAX_,
  _MIN_
};

/*
 * search_single_max2 --
 *
 *   Search the maxima of the modulus along lines of ext images. Foreach line
 * the field gr_lst is fill with all this maxima. The ends of a line can't be
 * in this list.
 *
 * Arguments :
 *   image      - Ext image to treat
 *   ze_eps     - real parameter to test ...
 *   middleFlag - integer
 *
 * Return value :
 *   The number of maxima found in all the image.
 */

int
search_single_max2 (ExtImage *image,
		    real     ze_eps,
		    int      middleFlag)
{
  Extremum *ext_ptr;
  Line     *line_ptr;
  //int      lx, ly;
  int      nb_of_sm = 0;

  real mod;
  real min;
  real max;
  real prev_max;
  real prev_prev_max;

  /*
   * With this value of epsilon one point is always a plateau.
   */
  real eps = 0;

 /*
  * The Extremum that will reflect the values of the plateau.
  */
  Extremum *pl_ext_ptr;

  Extremum *prev_pl_ext_ptr = 0;
  Extremum *prev_prev_pl_ext_ptr = 0;
  int pl_nb_of_ext = 0;
  int i;

  /*
   * Used for closed lines. After a complete pass of a line we restart at the
   * beginning for up to 2 plateaus. This is done to correctly check the line's
   * ends as max. overflow increase for each plateaus after a complete pass of
   * the line. It is possible (in some very particular case) that this method
   * does not detect a plateau-max at the beginning of the ext_lst of the line.
   */
  int overflow;

  List *pl_ext_lst;

  eps = ze_eps;

  foreach (line_ptr, image->line_lst) {
    prev_pl_ext_ptr = 0;
    prev_prev_pl_ext_ptr = 0;
    ext_ptr = lst_first (line_ptr->ext_lst);
    overflow = 0;
    while (ext_ptr && overflow <= 3) {
      /*
       * Look for a plateau.
       */

      if (overflow != 0) {
	/*
	 * For closed lines and after having completely pass the line.
	 */

	overflow++;
      }

      min = ext_ptr->mod;
      max = ext_ptr->mod;
      pl_ext_ptr = ext_ptr;
      pl_nb_of_ext = 1;
      pl_ext_lst = lst_create ();
      lst_add_end (ext_ptr, pl_ext_lst);

      ext_ptr = lst_next (line_ptr->ext_lst);
      if (!ext_ptr && line_ptr->type == LINE_CLOSED && !overflow) {
	/*
	 * If the previous plateau just ends with the ext_lst of a closed line,
	 * we get another plateau (at the beginning) to check if the previous
	 * one is a maximum.
	 */

	ext_ptr = lst_first (line_ptr->ext_lst);
	overflow = 2;
      }

      while (ext_ptr) {
	mod = ext_ptr->mod;
	if (mod > max) {
	  if ((mod - min) > eps*mod) {
	    break;
	  } else {
	    max = mod;
	    lst_add_end (ext_ptr, pl_ext_lst);
	    pl_nb_of_ext++;
	    pl_ext_ptr = ext_ptr;
	  }
	} else if (mod < min) {
	  if ((max - mod) > eps*mod) {
	    break;
	  } else {
	    min = mod;
	    lst_add_end (ext_ptr, pl_ext_lst);
	    pl_nb_of_ext++;
	  }
	}
	ext_ptr = lst_next (line_ptr->ext_lst);
	if (!ext_ptr && line_ptr->type == LINE_CLOSED && !overflow) {
	  /*
	   * It was the last ext_ptr of the line. If the line is closed the
	   * current plateau may continue at the beginning.
	   */
	  ext_ptr = lst_first (line_ptr->ext_lst);
	  overflow = 1;
	}
      }

      if (ext_ptr) {

	if (middleFlag == YES) {
	  /* The plateau is found. We set its "value" to its middle. */

	  i = 0;
	  foreach (pl_ext_ptr, pl_ext_lst) {
	    if (i == pl_nb_of_ext/2) {
	      break;
	    }
	    i++;
	  }
	}

	/* We check if it is a maximum. */

	if (prev_pl_ext_ptr && prev_prev_pl_ext_ptr) {
	  if ((min_max_flag == _MAX_
	       && (prev_max > prev_prev_max && prev_max > max))
	      || (min_max_flag == _MIN_
		  && (prev_max < prev_prev_max && prev_max < max))){
	    if (!lst_content_value (line_ptr->gr_lst, prev_pl_ext_ptr)) {
	      /* Only one reference of a gr_ext on the list. */

	      lst_add (prev_pl_ext_ptr, line_ptr->gr_lst);
	      line_ptr->nb_of_gr++;
	      nb_of_sm++;
	    }
	  }
	}

	/* Update all values before an other pass in the loop. */

	prev_prev_max = prev_max;
	prev_prev_pl_ext_ptr = prev_pl_ext_ptr;
	prev_max = max;
	prev_pl_ext_ptr = pl_ext_ptr;
      }
      lst_destroy (pl_ext_lst);
    }
  }

  return nb_of_sm;
}

