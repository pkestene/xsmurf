/*
 * chain.c --
 *
 *   Functions to search chains (or lines) on ext images.
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: chain.c,v 1.14 1999/05/11 18:52:55 decoster Exp $
 */

#include "wt2d_int.h"


/*
 * _create_ext_array_ -- PRIVATE
 *
 *   Fill an allocated array with the pointers to the extrema of an ext image.
 * If there is no extrema at a given position i, thus array[i] equals to NULL.
 *
 * Arguments:
 *   image - Ext image to treat.
 *   array - Array to fill.
 *
 * Return value:
 *   None.
 */

static void
_create_ext_array_(ExtImage * image,
		   Extremum ** array)
{
  int i;
  
  for (i = 0; i < image->lx*image->ly; i++) {
    array[i] = NULL;
  }
  for (i = 0; i < image->extrNb; i++) {
    array[image->extr[i].pos] = &(image->extr[i]);
  }
}


/*static void
_create_line_array_(ExtImage * image,
		    Line     ** array)
{
  int i;
  Line *l;
  Extremum *e;
  
  for (i = 0; i < image->lx*image->ly; i++) {
    array[i] = NULL;
  }

  foreach (l, image->line_lst) {
    e = (Extremum *) lst_first(l->ext_lst);
    array[e->pos] = l;
    e = (Extremum *) lst_last(l->ext_lst);
    array[e->pos] = l;
  }
}
*/


/*
 *   Global variables used for the _seek_neighbors_ function.
 */

static Line     *cur_line;		/* Current line. Line that is built.  */
static int      cur_lx;			/* lx of the current ext image.       */
static int      cur_ly;			/* ly of the current ext image.       */

/*
 *   Array of extrema created from the current ext image with the function
 * _create_ext_array_. This array is used to keep trace of chained extrema.
 * During the chain search process, every time that an extremum is added to a
 * line, the corresponding value in cur_ext_array will be set to NULL.
 */

static Extremum **cur_ext_array;

/*
 *   Arrays that stores the the relative position of the 8 points that are near
 * an given point. Mainly used with _seek_neighbors_ function. The order of the
 * neighbors is chosen to get "plus (+) cross neighbors" (i.e. north, west,
 * east and south) before the "times (x) cross neighbors" (i.e. n-w, n-e, s-w
 * and s-e). This is done to have a "best" succession of extrema on a line.
 */
			/* n   w  e  s n-w n-e s-w s-e */
static int iPosArray[] = {-1,  0, 0, 1, -1, -1,  1, 1};
static int jPosArray[] = { 0, -1, 1, 0, -1,  1, -1, 1};

/*
 * _seek_neighbors_ -- PRIVATE
 *
 *   Recursive function to seek all connected extrema from a given extremum.
 *
 * Arguments:
 *   extremum - Extremum to treat.
 *
 * Return value:
 *   The last extremum that is on the line.
 */

static Extremum *
_seek_neighbors_ (register Extremum *extremum)
{
  register int i, j;
  register int k;
  register int pos;

  pos = extremum->pos;

  /*
   * Loop on neighbors. First : '+'-like cross neighbors. Then :  'x'-like cross
   * neighbors
   */
  for (k = 0; k < 8; k++) {
    /*
     * Set neighbor pixel coordinates
     */
    i = pos%cur_lx + iPosArray[k];
    j = pos/cur_lx + jPosArray[k];
    if ((i >= 0) && (i < cur_lx) && (j >= 0) && (j < cur_ly)) {
      /*
       * The neighbor pixel is in the ext image.
       */
      extremum = cur_ext_array[i + j*cur_lx];
      if (extremum) {
	/*
	 * The neighbor pixel is an extremum.
	 */
	add_extremum_to_line (extremum, cur_line, cur_lx, LINE_BEG);
	/*
	 * Remove the extremum from the trace array.
	 */
	cur_ext_array[i + j*cur_lx] = NULL;
	/*
	 * We seek the neighbors of this neighbor.
	 */
	_seek_neighbors_ (extremum);
	/*
	 * We follow only one neighbor.
	 */
	return extremum;
      }
    }
  }
}


/*
 * _are_near_ -- PRIVATE
 *
 *   Check if two extrema are near (i.e. their x-distance and their y-distance
 * are less than one pixel).
 *
 * Arguments:
 *   2 Extremum * - The extrema.
 *
 * Return value:
 *   1 if they are near on a '+'-cross, 2 if they are near on a 'x'-cross. 0 if
 * not or if one extremum pointer is 0.
 */

static int
_are_near_(Extremum *ext1,
	   Extremum *ext2,
	   int      lx)
{
  int val;

  if (!ext1 || !ext2) {
    return 0;
  }

  if ( fabs(ext1->pos%lx - ext2->pos%lx) <= 1
       && fabs(ext1->pos/lx - ext2->pos/lx) <= 1 ) {
    if (fabs(ext1->pos%lx - ext2->pos%lx)
	*fabs(ext1->pos/lx - ext2->pos/lx) == 0) {
      val = 1;
    } else {
      val = 2;
    }
  } else {
    val = 0;
  }

  return val;
}


/*
 * Enumerate used by following functions.
 */

enum {
  _NO_MEET_,	/**/
  _MEET_B_B_,	/* The beginning of the first line meets the beginning of the
		 * second line. */
  _MEET_B_E_,	/* The beginning of the first line meets the end of the second
		 * line. */
  _MEET_E_B_,	/* The end of the first line meets the beginning of the second
		 * line. */
  _MEET_E_E_	/* The end of the first line meets the end of the second line.*/
};

/*
 * _ends_meet_ -- PRIVATE
 *
 *   Check which way ends of two lines meet.
 *
 * Arguments:
 *   2 Line * - The two lines.
 *
 * Return value:
 *   An integer that reflects the way the ends meet:
 *     _MEET_B_B_ - the beginning of the first line meets the beginning of the
 *                  second line;
 *     _MEET_B_E_ - the beginning of the first line meets the end of the second
 *                  line;
 *     _MEET_E_B_ - the end of the first line meets the beginning of the second
 *                  line;
 *     _MEET_E_E_ - the end of the first line meets the end of the second line.
 */

static int
_ends_meet_(Line *l1,
	    Line *l2)
{
  Extremum *b1;
  Extremum *b2;
  Extremum *e1;
  Extremum *e2;

  b1 = lst_first(l1->ext_lst);
  b2 = lst_first(l2->ext_lst);
  e1 = lst_last(l1->ext_lst);
  e2 = lst_last(l2->ext_lst);

 /* '+'-like cross check. */
  if (_are_near_(b1, b2, l1->lx) == 1) {
    return _MEET_B_B_;
  }
  if (_are_near_(b1, e2, l1->lx) == 1) {
    return _MEET_B_E_;
  }
  if (_are_near_(e1, b2, l1->lx) == 1) {
    return _MEET_E_B_;
  }
  if (_are_near_(e1, e2, l1->lx) == 1) {
    return _MEET_E_E_;
  }

 /* 'x'-like cross check. */
  if (_are_near_(b1, b2, l1->lx) == 2) {
    return _MEET_B_B_;
  }
  if (_are_near_(b1, e2, l1->lx) == 2) {
    return _MEET_B_E_;
  }
  if (_are_near_(e1, b2, l1->lx) == 2) {
    return _MEET_E_B_;
  }
  if (_are_near_(e1, e2, l1->lx) == 2) {
    return _MEET_E_E_;
  }

  return _NO_MEET_;
}


/*
 * _merge_lines_ -- PRIVATE
 *
 *   Merge two lines according to the way they meet.
 *
 * Arguments:
 *   2 Line * - The lines. The second line will be freed at the end of this
 *              function.
 *   int      - The way they meet (_NO_MEET_, _MEET_E_E_, ...)
 *
 * Return value:
 *   The resulting line.
 */

static Line *
_merge_lines_(Line *l1,
	      Line *l2,
	      int  meeting)
{
  Extremum *ext;

  switch (meeting) {
  case _MEET_B_B_:
    /*
     * Forward loop on the second list, add at the beginning of the first one.
     */

    for(ext = lst_first(l2->ext_lst); ext; ext = lst_next(l2->ext_lst)) {
      add_extremum_to_line(ext, l1, l1->lx, LINE_BEG);
    }
    break;

  case _MEET_B_E_:
    /*
     * Backward loop on the second list, add at the beginning of the first one.
     */

    for(ext = lst_last(l2->ext_lst); ext; ext = lst_prev(l2->ext_lst)) {
      add_extremum_to_line(ext, l1, l1->lx, LINE_BEG);
    }
    break;

  case _MEET_E_B_:
    /*
     * Forward loop on the second list, add at the end of the first one.
     */

    for(ext = lst_first(l2->ext_lst); ext; ext = lst_next(l2->ext_lst)) {
      add_extremum_to_line(ext, l1, l1->lx, LINE_END);
    }
    break;

  case _MEET_E_E_:
    /*
     * Backward loop on the second list, add at the end of the first one.
     */

    for(ext = lst_last(l2->ext_lst); ext; ext = lst_prev(l2->ext_lst)) {
      add_extremum_to_line(ext, l1, l1->lx, LINE_END);
    }
    break;
  }
  destroy_line(l2);

  return l1;
}

/*
 * _merge_line_array_ -- PRIVATE
 *
 *   Merge all possible lines from the array.
 *
 * Arguments:
 *   Line** - The array where the lines ares stored. This array will contains
 *            the new lines.
 *   int    - The number of lines in the array.
 *
 * Return value:
 *   Number of "perfect" lines.
 */

static int
_merge_line_array_(Line **line_arr,
		   int  nb_of_lines)
{
  /*
   * Remark : We need an array of lines (instead of a list) because we must do 2
   *   loops on the lines; and this is not possible with a list: you can't use 2
   *   imbricated "foreach" with the same list.
   */

  int i;
  int j;
  int meeting; /* Which way the ends meet (_NO_MEET_, _MEET_E_E_, ...) */

  for (i = 0; i < nb_of_lines; i++) {
    for (j = i+1; j < nb_of_lines; j++) {
      meeting = _ends_meet_ (line_arr[i], line_arr[j]);
      if (meeting != _NO_MEET_) {
	line_arr[i] = _merge_lines_ (line_arr[i], line_arr[j], meeting);
	line_arr[j] = line_arr[nb_of_lines-1];
	nb_of_lines--;
	return _merge_line_array_(line_arr, nb_of_lines);
      }
    }
  }

  return nb_of_lines;
}


/*
 */

int _nb_of_neighbors_ (int pos)
{
  int k, i, j;
  int nb = 0;

  for (k = 0; k < 8; k++) {
    i = pos%cur_lx + iPosArray[k];
    j = pos/cur_lx + jPosArray[k];
    if (i >= 0 && i < cur_lx && j >= 0 && j < cur_ly
	&& cur_ext_array[i + j*cur_lx]) {
      nb++;
    }
  }

  return nb;
}


/*
 */

static int
_is_line_end_ (int pos)
{
  int k;
  int i1, j1;
  int i2, j2;
  //int nb = 0;
  int nb_of_neighbors;

  nb_of_neighbors = _nb_of_neighbors_(pos);
  if (nb_of_neighbors > 2) {
    return 0;
  }

  if (nb_of_neighbors <= 1) {
    return 1;
  }

  for (k = 0; k < 8; k++) {
    i1 = pos%cur_lx + iPosArray[k];
    j1 = pos/cur_lx + jPosArray[k];
    if (i1 >= 0 && i1 < cur_lx && j1 >= 0 && j1 < cur_ly
	&& cur_ext_array[i1 + j1*cur_lx]) {
      break;
    }
  }

  for (k = k + 1; k < 8; k++) {
    i2 = pos%cur_lx + iPosArray[k];
    j2 = pos/cur_lx + jPosArray[k];
    if (i2 >= 0 && i2 < cur_lx && j2 >= 0 && j2 < cur_ly
	&& cur_ext_array[i2 + j2*cur_lx]) {
      break;
    }
  }

  if ((abs(i1-i2) == 0 && abs(j1-j2) == 1)
      || (abs(i1-i2) == 1 && abs(j1-j2) == 0)) {
    return 1;
  }

  return 0;
}


/*
 * _line_split_ -- PRIVATE
 *
 *   Split a set (that is stored in a Line struct). A set has several pieces of
 * lines with no special order (i.e. some "real lines" can be broken in several
 * parts). This function first split the set into independant lines. Then it
 * finds which ones have ends that are near and merge these lines.
 *
 * Arguments:
 *   Line * - The struct that contains the set.
 *
 * Return value:
 *   The list of "perfect lines".
 */

static List *
_line_split_ (Line *line)
{
  List *line_list; /* List to return. */
  Line *a_line;
  Line **line_arr;

  Extremum *ext;     /* An extremum and...    */
  Extremum *old_ext; /*   ... its predecessor. */

  int nb_of_lines = 0;
  int i;

  line_list = lst_create ();

  /*
   *   We compute each extremum of the set to build "allready"
   *  connected pieces of lines.
   */

  old_ext = 0;
  foreach (ext, line->ext_lst) {
    if (_are_near_ (ext, old_ext, line->lx)) {
      /*
       * The extremum stay in the current line.
       */

      add_extremum_to_line (ext, a_line, line->lx, LINE_BEG) ;
      old_ext = ext;
    } else {
      /*
       * The extremum starts a new line.
       */

      a_line = create_line (line->lx, line->ext_image, line->scale);
      nb_of_lines++;
      lst_add (a_line, line_list);
      add_extremum_to_line (ext, a_line, line->lx, LINE_BEG) ;
    }
  }

  /*
   *   Now we merge lines which ends meet. We need to store them in an array
   * first.
   */

  line_arr = (Line **) malloc (sizeof(Line *)*nb_of_lines);
  i = 0;
  foreach (a_line, line_list) {
    line_arr[i] = a_line;
    i++;
  }
  lst_destroy(line_list);
  nb_of_lines = _merge_line_array_ (line_arr, nb_of_lines);

  line_list = lst_create();
  for (i = 0; i < nb_of_lines; i++) {
    lst_add(line_arr[i], line_list);
  }

  return line_list;
}


/*
 * main routine
 * called by search_lines (see bellow...)
 */
static int
_find_lines_ (ExtImage *ext_image,
	      int      noClosedLinesFlag)
{
  int      pos;
  Line     *line;
  Extremum *extremum;
  int      nb_of_lines_found = 0;

  assert (ext_image);

  /*
   * Loops on all position in the ext image.
   */

  pos = 0;
  for (pos = 0; pos < cur_lx*cur_ly; pos++) {
    if (cur_ext_array[pos]) {
      /*
       * There is an extremum at the current position
       */

      if (noClosedLinesFlag && !_is_line_end_(pos)) {
	continue;
      }

      extremum = cur_ext_array[pos];

      /*
       * Init a new line with this extremum, and mark it.
       */

      line = create_line (ext_image->lx, ext_image, ext_image->scale);
      add_extremum_to_line (extremum, line, cur_lx, LINE_BEG);
      cur_ext_array[pos] = NULL;

      /*
       * Seek _all_ the extrema that are in this line.
       */

      cur_line = line;
      _seek_neighbors_ (extremum);

      /*
       * Check if line is closed.
       */

      if (line->size > 3
	  && _are_near_(lst_first(line->ext_lst),
			lst_last(line->ext_lst), 
			line->lx)) {
	line->type = LINE_CLOSED;
      }

      lst_add (line, ext_image->line_lst);
      ext_image->nb_of_lines ++;
      nb_of_lines_found++;
    }
  }

  return nb_of_lines_found;
}


/*
 * search_lines --
 *
 *   Connect all the extrema of an ext image into lines. This is done (stored)
 * in the ext image structure using the line_lst field.
 *
 * Arguments:
 *   ext_image - Ext image to treat.
 *
 * Return value:
 *   The number of lines that are created.
 */

int
search_lines (ExtImage *ext_image)
{
  Extremum **ext_array;
  int nb_of_lines_found;

  assert (ext_image);

  cur_lx = ext_image->lx;
  cur_ly = ext_image->ly;

  /*
   * Init global variable for the _seek_neighbors_ function.
   */

  ext_array = (Extremum **) malloc (sizeof(Extremum*) * cur_lx * cur_ly);
  if (!ext_array) {
    EX_RAISE(mem_alloc_error);
  }
  _create_ext_array_ (ext_image, ext_array);
  cur_ext_array = ext_array;

  do {
    nb_of_lines_found = _find_lines_(ext_image, 1);
  } while (nb_of_lines_found != 0);

  nb_of_lines_found = _find_lines_(ext_image, 0);

  free (ext_array);
  return ext_image->nb_of_lines;

mem_alloc_error:
  free (ext_array);

  return 0;
}


/*
 * called by compute_discrete_tangent (see bellow...)
 */
/* added by Pierre Kestener
   march 20th 2002*/
static int
_direction_ (int X1, int Y1, int X2, int Y2)
{
  int dir;
  if (     X2-X1 == 1  && Y2-Y1==0)  dir = 0;
  else if (X2-X1 == 1  && Y2-Y1==1)  dir = 1;
  else if (X2-X1 == 0  && Y2-Y1==1)  dir = 2;
  else if (X2-X1 == -1 && Y2-Y1==1)  dir = 3;
  else if (X2-X1 == -1 && Y2-Y1==0)  dir = 4;
  else if (X2-X1 == -1 && Y2-Y1==-1) dir = 5;
  else if (X2-X1 == 0  && Y2-Y1==-1) dir = 6;
  else if (X2-X1 == 1  && Y2-Y1==-1) dir = 7;
  else dir = -1;

  return dir;
}

/*
 * called by compute_discrete_tangent (see bellow...)
 */
/* added by Pierre Kestener
   march 20th 2002*/
static int
_octant_ (int d1, int d2)
{
  int octant, tmp;

  if (d2<d1) {
    tmp=d1; d1=d2; d2=tmp;
  }
  /* now we have d1<=d2 */

  if (d2-d1==7) octant = 7;
  else if (d2-d1>1 && d2-d1<7) octant = -1; //undefined
  else if (d2-d1==1) octant = d1;
  else if (d2-d1==0) octant = d1;
  else {
    octant = -2;
    //printf("qq %d %d\n",d1,d2);
  }
  return octant;
}

/*
 * called by compute_discrete_tangent (see bellow...)
 */
/* added by Pierre Kestener
   march 20th 2002*/
static int
_normal_vector_x_coord_ (int octant, int a, int b)
{
  int x;

  if (octant==0)      x = a;
  else if (octant==1) x = b;
  else if (octant==2) x = b;
  else if (octant==3) x = a;
  else if (octant==4) x = -a;
  else if (octant==5) x = -b;
  else if (octant==6) x = -b;
  else if (octant==7) x = -a;
  
  return x;
}

/*
 * called by compute_discrete_tangent (see bellow...)
 */
/* added by Pierre Kestener
   march 20th 2002*/
static int
_normal_vector_y_coord_ (int octant, int a, int b)
{
  int y;

  if (octant==0)      y = -b;
  else if (octant==1) y = -a;
  else if (octant==2) y = a;
  else if (octant==3) y = b;
  else if (octant==4) y = b;
  else if (octant==5) y = a;
  else if (octant==6) y = -a;
  else if (octant==7) y = -b;
  
  return y;
}

/*
 * called by compute_discrete_tangent (see bellow...)
 */
/* added by Pierre Kestener
   march 20th 2002*/
static float
_normal_theta_ (int a, int b)
{
  float theta;

  if (a==0 && b>0) theta = (float) asin(1);
  else if (a==0 && b<0) theta = (float) asin(1);
  else if (a==0 && b==0) theta = 0.0;
  else theta = (float) atan((double) b/a);

  return theta;
}


/*
 * compute_discrete_tangent --
 *
 *   Compute discrete tangent parameters (from Vialard's algorithm)
 * around given extremum belonging to given line.
 * See articles to download from :
 * http://dept-info.labri.u-bordeaux.fr/~vialard/
 * 
 * For each extremun of each line, one computes :
 *   -  a and b, such that ratio b/a is the slope of approximated tangent
 *   -  param mu
 *   -  octant (integer between 0 and 7)
 *
 * Arguments:
 *   ext_image - Ext image to treat.
 *
 * Return value:
 *   Zero.
 */

/* note pour plus tard
 * traiter differemment le cas ou la ligne est fermee
 */

/* added by Pierre Kestener
   march 20th 2002*/

int
compute_discrete_tangent (ExtImage *ext_image)
{
  Extremum  *ext, *next_ext;
  Line      *line;
  point     Pb; /* point backward */
  point     Pf; /* point forward  */
  
  point     UM, Um, LM, Lm;
  int       a, b, mu;
  int       zea, zeb, zemu;
  int       octant;
  int       *array_a,*array_b,*array_mu,*array_octant;
  float     *array_theta;
  int       dirf, dirb; /* direction : between 0 and 7 */
  int       d1 = -1;
  int       d2 = -1;

  int       *posX,*posY;
  //int       px,py;
  int       i,j,reste;
  int       go_on;

  assert (ext_image);
  
  cur_lx = ext_image->lx;
  cur_ly = ext_image->ly;

  foreach (line, ext_image->line_lst) {
    if (line->size < 3) { 
      foreach (ext, line->ext_lst) {
	ext->a = 0;
	ext->b = 0;
	ext->xe = (float) (ext->pos % cur_lx);
	ext->ye = (float) (ext->pos / cur_lx);
	ext->octant = -1;
	//ext->bend = 0.0;
      }
    } else  {
      posX = (int *) malloc (sizeof(int)*line->size);
      posY = (int *) malloc (sizeof(int)*line->size);
      array_a = (int *) malloc (sizeof(int)*line->size);
      array_b = (int *) malloc (sizeof(int)*line->size);
      array_mu = (int *) malloc (sizeof(int)*line->size);
      array_octant = (int *) malloc (sizeof(int)*line->size);
      array_theta = (float *) malloc (sizeof(float)*line->size);

      ext = lst_first(line->ext_lst);
      posX[0] = ext->pos%cur_lx;
      posY[0] = ext->pos/cur_lx;
      for (i=1;i<line->size;i++) {
	next_ext = lst_next(line->ext_lst);
	posX[i] = next_ext->pos%cur_lx;
	posY[i] = next_ext->pos/cur_lx;
      }

      /* first point */
      dirf = _direction_(posX[0],posY[0],posX[1],posY[1]);
      if (dirf%2<1) {
	a=0; b=1; mu=0;
      } else {
	a=1; b=1; mu=0;
      }
      array_a[0]  = _normal_vector_x_coord_ (dirf,a,b);
      array_b[0]  = _normal_vector_y_coord_ (dirf,a,b);
      array_mu[0] = mu;
      array_octant[0] = dirf;
      array_theta[0] = _normal_theta_ (array_a[0],array_b[0]);

      /* the algorithm */
      for (i=1; i<line->size-1;i++) {
	/*
	 * Init algorithm
	 */
	Pb.x = Pb.y = 0;
	dirf = _direction_(posX[i],posY[i],posX[i+1],posY[i+1]);
	if (dirf%2<1) {
	  a=0; b=1; mu=0;
	  zea=0; zeb=1; zemu=0;
  
	  Um.x = Um.y = Lm.x = Lm.y = 0;
	  UM.x = LM.x = 1; UM.y = LM.y = 0;
	  Pf.x = 1; Pf.y = 0;
	} else {
	  a=1; b=1; mu=0;
	  zea=1; zeb=1; zemu=0;

	  Um.x = Um.y = Lm.x = Lm.y = 0;
	  UM.x = LM.x = 1; UM.y = LM.y = 1;
	  Pf.x = 1; Pf.y = 1;
	}
	go_on = 1;
	d1 = dirf;
	octant = _octant_ (d1,d1);

	/* now try to add backward point */
	dirb = _direction_(posX[i-1],posY[i-1],posX[i],posY[i]);
	Pb.x--;
	if (dirb%2>0) Pb.y--;
	reste = a*Pb.x - b*Pb.y;

	if (mu<=reste && reste<=mu+b) {
	  if (reste == mu)     Um = Pb;
	  if (reste == mu+b-1) Lm = Pb;
	} else if (reste == mu-1) {
	  LM = Lm; Um = Pb;
	  a  = UM.y - Pb.y;
	  b  = UM.x - Pb.x;
	  mu = a*Pb.x - b*Pb.y;
	} else if (reste == mu+b) {
	  UM = Um; Lm = Pb;
	  a  = LM.y - Pb.y;
	  b  = LM.x - Pb.x;
	  mu = a*Pb.x - b*Pb.y - b +1;
	} else {
	  go_on = 0;
	}
	if (go_on>0 && dirb!=dirf) {
	  d2 = dirb;
	  octant = _octant_(d1,d2);
	}
	/* update ze param */
	zea = a; zeb = b; zemu = mu;


	/* main body of algorithm */
	for (j=2;i-j>=0 && i+j<line->size && go_on>0;j++) {
	  /* first try to add forward point */
	  dirf = _direction_ (posX[i+j-1],posY[i+j-1],posX[i+j],posY[i+j]);
	  Pf.x++;
	  if (dirf%2>0) Pf.y++;
	  reste = a*Pf.x - b*Pf.y;

	  if (mu<=reste && reste<=mu+b) {
	    if (reste == mu)     UM = Pf;
	    if (reste == mu+b-1) LM = Pf;
	  } else if (reste == mu-1) {
	    Lm = LM; UM = Pf;
	    a  = -Um.y + Pf.y;
	    b  = -Um.x + Pf.x;
	    mu = a*Pf.x - b*Pf.y;
	  } else if (reste == mu+b) {
	    Um = UM; LM = Pf;
	    a  = -Lm.y + Pf.y;
	    b  = -Lm.x + Pf.x;
	    mu = a*Pf.x - b*Pf.y - b +1;
	  } else {
	    go_on = 0; /* end of algorithm */
	    break;
	  }

	  if (d2 == -1 && dirf!=d1) {
	    d2 = dirf;
	    octant = _octant_(d1,d2);
	  }

	  /* second try to add backward point */
	  dirb = _direction_ (posX[i-j],posY[i-j],posX[i-j+1],posY[i-j+1]);
	  Pb.x--;
	  if (dirf%2>0) Pb.y--;
	  reste = a*Pb.x - b*Pb.y;

	  if (mu<=reste && reste<=mu+b) {
	    if (reste == mu)     Um = Pb;
	    if (reste == mu+b-1) Lm = Pb;
	  } else if (reste == mu-1) {
	    LM = Lm; Um = Pb;
	    a  = UM.y - Pb.y;
	    b  = UM.x - Pb.x;
	    mu = a*Pb.x - b*Pb.y;
	  } else if (reste == mu+b) {
	    UM = Um; Lm = Pb;
	    a  = LM.y - Pb.y;
	    b  = LM.x - Pb.x;
	    mu = a*Pb.x - b*Pb.y - b +1;
	  } else {
	    go_on = 0; /* end of algorithm */
	    break;
	  }

	  if (d2 == -1 && dirf!=d1) {
	    d2 = dirf;
	    octant = _octant_(d1,d2);
	  }

	  /* update ze param */
	  zea = a; zeb = b; zemu = mu;

	} /* end loop to add points */

	/* now we have parameter of ze discrete tangent !!!*/
	array_a[i]  = _normal_vector_x_coord_ (octant,a,b);
	array_b[i]  = _normal_vector_y_coord_ (octant,a,b);
	array_mu[i] = mu;
	array_octant[i] = octant;
	array_theta[i] = _normal_theta_ (array_a[i],array_b[i]);
	//if (octant < 0) {printf("toto");}

      } /* loop on points that are on the line */

      /* last point */
      dirb = _direction_(posX[line->size-2],posY[line->size-2],posX[line->size-1],posY[line->size-1]);
      if (dirb%2<1) {
	a=0; b=1; mu=0;
      } else {
	a=1; b=1; mu=0;
      }
      array_a[line->size-1]  = _normal_vector_x_coord_ (dirb,a,b);
      array_b[line->size-1]  = _normal_vector_y_coord_ (dirb,a,b);
      array_mu[line->size-1] = mu;
      array_octant[line->size-1] = dirb;
      array_theta[line->size-1] = _normal_theta_ (array_a[line->size-1],array_b[line->size-1]);


      /* now fill missing extrema fields */
      ext = lst_first(line->ext_lst);
      ext->a      = array_a[0];
      ext->b      = array_b[0];
      ext->octant = array_octant[0];
      ext->theta  = array_theta[0];

      for (i=1; i<line->size;i++) {
	ext = lst_next(line->ext_lst);
	ext->a      = array_a[i];
	ext->b      = array_b[i];
	ext->octant = array_octant[i];
	ext->theta  = array_theta[i];
 	//if (ext->octant < 0) {printf("toto%d ",ext->octant);}
     }

      free(posX);
      free(posY);
      free(array_a);
      free(array_b);
      free(array_mu);
      free(array_octant);
      free(array_theta);

    } /* if(line->size>=3...*/
  } /* foreach(line... */
  return 0;
}



/*
 * lines_to_chains --
 *
 *   This function must die as soon as possible ! It converts the "line" format
 * (new one) of an ext image to the chain format (old one).
 *
 * Arguments:
 *   ext_image - Ext image to treat.
 *
 * Return value:
 *   None.
 */

void
lines_to_chains (ExtImage *ext_image)
{
  int     i;
  Line    *line;

  ext_image->chainNb = ext_image->nb_of_lines;
  ext_image->chain = (Extremum **) malloc (sizeof (Extremum *)
					     *ext_image->chainNb);
  i = 0;
  foreach (line, ext_image->line_lst) { 
    Extremum *extremum;
    Extremum *new_extremum;

    extremum = lst_first (line->ext_lst);;
    ext_image->chain[i] = extremum;
    line->ext_chain = ext_image->chain[i];
    new_extremum = lst_next (line->ext_lst);
    for (;
	 new_extremum;
	 new_extremum = lst_next (line->ext_lst)) {
      new_extremum->prev = extremum;
      new_extremum->next = NULL;
      extremum->next = new_extremum;
      extremum = new_extremum;
    }
    i++;
  }
}

