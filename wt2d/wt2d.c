/* last modified by Pierre Kestener (2000/06/01). */

#include "wt2d_int.h"

/* crade */
void ExtDicUnlinkStamp_(int stamp);

/*----------------------------------------------------------------------
  w2_ext_new
  
  Constructeur d'une ExtImage  
  Entree : nbExtrema : nombre d'extrema a stocker
  Sortie : 0 si echec, sinon une ExtImage alouee.
  --------------------------------------------------------------------*/
ExtImage *
w2_ext_new (int extremaNb,int lx,int ly, real scale)
{
  ExtImage *im;

  im = (ExtImage *) malloc (sizeof (ExtImage));

  if (!im)
    return NULL;
  
  im -> lx      = lx;
  im -> ly      = ly;
  im -> extrNb  = extremaNb;
  if (extremaNb)
    im -> extr = (Extremum *) malloc (extremaNb*sizeof(Extremum));
  else
    im -> extr = NULL;
  im -> chain   = NULL;
  im -> chainNb = 0;
  im -> scale   = scale;
  im -> stamp   = 0;

  im -> nb_of_lines = 0;
  im -> line_lst = lst_create ();

  /*  im -> nbTagExt   = 0;
      im -> nbNotagExt = 0;*/

  if ((!im -> extr && extremaNb) || !im -> line_lst)
    {
      free (im);
      return NULL;
    }
  ExtImaUnlink_ (im);

  im->up = NULL;
  im->down = NULL;

  return im;
}

/*----------------------------------------------------------------------
  ExtImaUnlink_

  Efface les eventuels liens des extrema contenus dans l'image.
  Se relance recursivement sur tout le groupe d'image ayant le meme
  numero (stamp).
  Entree : im, une image alloue
  --------------------------------------------------------------------*/
void
ExtImaUnlink_ (im)
     ExtImage *im;
{
  register int      i;
  register Extremum *ext;

  int stamp;

  ext = im -> extr;
  for (i = 0; i < im -> extrNb; ext++, i++)
    {
      if (!ext){
	printf ("aaaaaaaaaaaaaaaaaaaaaaaaaAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaarg!\n");
      }
      else
	{
	  ext -> prev = NULL;
	  ext -> next = NULL;
	  ext -> up   = NULL;
	  ext -> down = NULL;
	}
    }
  if (im -> chainNb > 0)
    {
      im -> chainNb = 0;
      free (im -> chain);
    }
  stamp = im -> stamp;
  im -> stamp = 0;

  if (stamp)
    ExtDicUnlinkStamp_ (stamp);
}

/*----------------------------------------------------------------------
  ExtImaDelete

  Destructeur de ExtImage
  Entree : im, une image alloue
  Sortie :
  Rem.   : on verifie la validite des allocations
  --------------------------------------------------------------------*/
void
ExtImaDelete_ (im)
     ExtImage *im;
{
  Line     *line;

  if (im)			/* precautions normalement inutiles */
    {
      foreach (line, im -> line_lst)
	destroy_line (line);
      lst_destroy (im -> line_lst);

      if (im -> extr)
	{
	  free (im -> extr);
	  if (im -> stamp)
	    ExtImaUnlink_ (im);
	}
      free (im);
    }
}  

/*----------------------------------------------------------------------
  ExtImaMinMax

  Renvoie le min et le max du module des points de la ExtImage
  --------------------------------------------------------------------*/
void
ExtImaMinMax (im, min, max)
     ExtImage *im;
     real     *min;
     real     *max;
{
  register int i;
  real mod;
  
  if (im -> extrNb > 0)
    (*min) = (*max) = im -> extr[0].mod;
  else
    (*min) = (*max) = 0;
  
  for (i = 0; i < im -> extrNb; i++)
    if ((mod = im -> extr[i].mod) > (*max))
      (*max) = mod;
    else
      if (mod < (*min))
	(*min) = mod;
}  

/*----------------------------------------------------------------------
  ExtImaMinMaxArg

  Renvoie le min et le max de l'argument des points de la ExtImage
  --------------------------------------------------------------------*/
void
ExtImaMinMaxArg (im, min, max)
     ExtImage *im;
     real     *min;
     real     *max;
{
  register int i;
  real arg;
  
  if (im -> extrNb>0)
    (*min) = (*max) = im -> extr[0].arg;
  else
    (*min) = (*max) = 0;
  
  for (i = 0; i < im -> extrNb; i++)
    if ((arg = im -> extr[i].arg) > (*max))
      (*max) = arg;
    else
      if (arg < (*min))
	(*min) = arg;
}  

/*----------------------------------------------------------------------
  ExtImaMinMaxMx

  Renvoie le min et le max de module*cos(arg) des points de la ExtImage
  --------------------------------------------------------------------*/
void
ExtImaMinMaxMx (im, min, max)
     ExtImage *im;
     real     *min;
     real     *max;
{
  register int i;
  real mx;
  
  if (im -> extrNb > 0)
    (*min) = (*max) = im -> extr[0].mod  * cos(im -> extr[0].arg);
  else
    (*min) = (*max) = 0;
  
  for (i = 0; i < im -> extrNb; i++)
    if ((mx = im -> extr[i].mod * cos(im -> extr[i].arg)) > (*max))
      (*max) = mx;
    else
      if (mx < (*min))
	(*min) = mx;
}  

/*----------------------------------------------------------------------
  ExtImaMinMaxMy

  Renvoie le min et le max de module*sin(arg) des points de la ExtImage
  --------------------------------------------------------------------*/
void
ExtImaMinMaxMy (im, min, max)
     ExtImage *im;
     real     *min;
     real     *max;
{
  register int i;
  real my;
  
  if (im -> extrNb > 0)
    (*min) = (*max) = im -> extr[0].mod  * sin(im -> extr[0].arg);
  else
    (*min) = (*max) = 0;
  
  for (i = 0; i < im -> extrNb; i++)
    if ((my = im -> extr[i].mod * sin(im -> extr[i].arg)) > (*max))
      (*max) = my;
    else
      if (my < (*min))
	(*min) = my;
}  





/*
 */
ExtImage *
w2_remove_extrema (ExtImage *ext_image,
		   int x_min,
		   int x_max,
		   int y_min,
		   int y_max)
{
  ExtImage *result;
  int      i, j;
  int      nb = 0;
  int      lx, ly;
  int      x, y;

  lx = ext_image -> lx;
  ly = ext_image -> ly;
  /* first loop just know nb : number of extr
     we want to keep */
  for (i = 0; i < ext_image -> extrNb; i++)
    {
      x = ext_image -> extr[i].pos % lx;
      y = ext_image -> extr[i].pos / lx;
      if ((x >= x_min) && (x <= x_max)
	  && (y >= y_min) && (y <= y_max))
	nb++;
    }
  /* memory allocation for the new extimage */
  result = w2_ext_new (nb, lx, ly, ext_image -> scale);

  /* second loop to fill the new extimage */
  if (result)
    {
      j = 0;
      for (i = 0; i < ext_image -> extrNb; i++)
	{
	  x = ext_image -> extr[i].pos % lx;
	  y = ext_image -> extr[i].pos / lx;
	  if ((x >= x_min) && (x <= x_max)
	      && (y >= y_min) && (y <= y_max))
	    {
	      result -> extr[j] = ext_image -> extr[i];
	      j++;
	    }
	}
    }

  return result;
}

/*
 * Create a new extima with extrema that are isolated : alone in box
 * of size "sizebox"
 */
ExtImage *
w2_keep_isolated (ExtImage *ext_image,
		  int boxsize)
{
  ExtImage *result;
  int      i, j, k;
  int      nb = 0;
  int      lx, ly;
  int      x, y;
  int      xx, yy;
  int      xMin, xMax, yMin, yMax; /* coordinates of the box */
  int      nb_extr_box;

  lx = ext_image -> lx;
  ly = ext_image -> ly;

  /* first loop just know nb : number of extr
     we want to keep */
  for (i = 0; i < ext_image -> extrNb; i++)
    {
      nb_extr_box=0;
      x = ext_image -> extr[i].pos % lx;
      y = ext_image -> extr[i].pos / lx;
      xMin = x-boxsize  ;  if(xMin<0)  xMin = 0 ;
      xMax = x+boxsize+1;  if(xMax>lx) xMax = lx;
      yMin = y-boxsize  ;  if(yMin<0)  yMin = 0 ;
      yMax = y+boxsize+1;  if(yMax>ly) yMax = ly;
      
      for (j = 0; j < ext_image->extrNb; j++) {
	xx = ext_image->extr[j].pos % lx;
	yy = ext_image->extr[j].pos / lx;
	if (xx >= xMin && xx <= xMax && yy >= yMin && yy <= yMax) {
	  nb_extr_box++;
	}
      }
      if (nb_extr_box==1) nb++;
    }
  /* memory allocation for the new extimage */
  result = w2_ext_new (nb, lx, ly, ext_image -> scale);

  /* second loop to fill the new extimage */
  if (result)
    {
      k = 0;
      for (i = 0; i < ext_image -> extrNb; i++)
	{
	  nb_extr_box=0;
	  x = ext_image -> extr[i].pos % lx;
	  y = ext_image -> extr[i].pos / lx;
	  xMin = x-boxsize  ;  if(xMin<0)  xMin = 0 ;
	  xMax = x+boxsize+1;  if(xMax>lx) xMax = lx;
	  yMin = y-boxsize  ;  if(yMin<0)  yMin = 0 ;
	  yMax = y+boxsize+1;  if(yMax>ly) yMax = ly;
	  
	  for (j = 0; j < ext_image->extrNb; j++) {
	    xx = ext_image->extr[j].pos % lx;
	    yy = ext_image->extr[j].pos / lx;
	    if (xx >= xMin && xx <= xMax && yy >= yMin && yy <= yMax) {
	      nb_extr_box++;
	    }
	  }
	  if (nb_extr_box==1) {
	    result -> extr[k] = ext_image -> extr[i];
	    k++;
	  }
	  
	}
    }

  return result;
}

/*
 * Create a new extima with extrema that form closed loop whose
 * mean radius is above a min threshold and lower than a max threshold.

 */
ExtImage *
w2_keep_circle (ExtImage *ext_image,int radius_min, int radius_max)
{
  ExtImage *result;
  int      k;
  int      lx, ly;
  int      x, y;
  int      nb = 0;
  real     center_x, center_y;
  real     mean_radius, r;
  Line     *l;
  Extremum *ext_ptr;
  
  lx = ext_image -> lx;
  ly = ext_image -> ly;
  
  /* first loop just know nb : number of extr
     we want to keep */

  foreach (l, ext_image->line_lst) {
    if (l->type == LINE_OPENED) continue;
    
    mean_radius = 0.0;
    center_x = l->gravity_center_x;
    center_y = l->gravity_center_y;

    foreach (ext_ptr, l->ext_lst) {
      x = ext_ptr->pos % lx;
      y = ext_ptr->pos / lx;
      r = sqrt((float)((x-center_x)*(x-center_x)+(y-center_y)*(y-center_y)));
      mean_radius += r;
    }
    mean_radius /= l->size;
    if ((mean_radius>radius_min) && (mean_radius<radius_max)) {
      /* we keep this closed loop !!!*/
      nb += l->size;
    }
  }
  
  /* memory allocation for the new extimage */
  result = w2_ext_new (nb, lx, ly, ext_image -> scale);
  
  /* second loop to fill the new extimage */
  if (result) {
    k = 0;
    foreach (l, ext_image->line_lst) {
      if (l->type == LINE_OPENED) continue;
      
      mean_radius = 0.0;
      center_x = l->gravity_center_x;
      center_y = l->gravity_center_y;
      
      foreach (ext_ptr, l->ext_lst) {
	x = ext_ptr->pos % lx;
	y = ext_ptr->pos / lx;
	r = sqrt((float)((x-center_x)*(x-center_x)+(y-center_y)*(y-center_y)));
	mean_radius += r;
      }
      mean_radius /= l->size;
      if ((mean_radius>radius_min) && (mean_radius<radius_max)) {
	/* we fill */
	foreach (ext_ptr, l->ext_lst) {
	  result -> extr[k] = *ext_ptr;
	  k++;
	}
      }
    }
  }
  
  return result;
}

/*
 * Create a new extima with extrema that form closed loop whose
 * mean radius is above a min threshold and lower than a max threshold.
 */
ExtImage *
w2_keep_circle_simple (ExtImage *ext_image,int radius_min, int radius_max)
{
  ExtImage *result;
  int      k;
  int      lx, ly;
  int      x, y;
  int      nb = 0;
  real     center_x, center_y;
  real     mean_radius, r;
  Line     *l;
  Extremum *ext_ptr;
  
  lx = ext_image -> lx;
  ly = ext_image -> ly;
  
  /* first loop just know nb : number of extr
     we want to keep */

  foreach (l, ext_image->line_lst) {
    if (l->type == LINE_OPENED) continue;
    
    mean_radius = 0.0;
    center_x = l->gravity_center_x;
    center_y = l->gravity_center_y;

    foreach (ext_ptr, l->ext_lst) {
      x = ext_ptr->pos % lx;
      y = ext_ptr->pos / lx;
      r = sqrt((float)((x-center_x)*(x-center_x)+(y-center_y)*(y-center_y)));
      mean_radius += r;
    }
    mean_radius /= l->size;
    if ((mean_radius>radius_min) && (mean_radius<radius_max)) {
      /* we keep this closed loop !!!*/
      nb += l->size;
    }
  }
  
  /* memory allocation for the new extimage */
  result = w2_ext_new (nb, lx, ly, ext_image -> scale);
  
  /* second loop to fill the new extimage */
  if (result) {
    k = 0;
    foreach (l, ext_image->line_lst) {
      if (l->type == LINE_OPENED) continue;
      
      mean_radius = 0.0;
      center_x = l->gravity_center_x;
      center_y = l->gravity_center_y;
      
      foreach (ext_ptr, l->ext_lst) {
	x = ext_ptr->pos % lx;
	y = ext_ptr->pos / lx;
	r = sqrt((float)((x-center_x)*(x-center_x)+(y-center_y)*(y-center_y)));
	mean_radius += r;
      }
      mean_radius /= l->size;
      if ((mean_radius>radius_min) && (mean_radius<radius_max)) {
	/* we fill */
	foreach (ext_ptr, l->ext_lst) {
	  result -> extr[k] = *ext_ptr;
	  k++;
	}
      }
    }
  }
  
  return result;
}


/*
 * Remove a line from the list of lines from an ext image.
 */
Line *
w2_remove_line (ExtImage *ext_image,
		Line     *line)
{
  Line *result;

  result = lst_remove (line, ext_image -> line_lst);
  if (result)
    ext_image -> nb_of_lines --;
  
  return result;
}

/*
 * Destroy a line from the list of lines from an ext image.
 */
void
w2_destroy_line (ExtImage *ext_image,
		 Line     *line)
{
  Line *result;

  result = w2_remove_line (ext_image, line);
  if (result)
    destroy_line (result);
}

/*
 * If size_min is not -1, removes all the lines which size is lesser than
 * size_min.
 * If size_max is not -1, removes all the lines which size is greater than
 * size_max.
 */
void
w2_remove_lines_by_size (ExtImage *ext_image,
			 int      size_min,
			 int      size_max)
{
  Line *line;
  List *rm_lst;

  assert (ext_image);

  rm_lst = lst_create ();
  foreach (line, ext_image -> line_lst)
    {
      if (size_min != -1 && line -> size < size_min)
	lst_add (line, rm_lst);
      if (size_max != -1 && line -> size > size_max)
	lst_add (line, rm_lst);
    }
  foreach (line, rm_lst)
    w2_destroy_line (ext_image, line);

  lst_destroy (rm_lst);
}


/*
 */
void
w2_remove_lines_by_mean_mod (ExtImage *ext_image,
			     real     mod_min,
			     real     mod_max)
{
  Line *line;
  List *rm_lst;

  assert (ext_image);

  rm_lst = lst_create ();
  foreach (line, ext_image -> line_lst)
    {
      if (mod_min != -1 && (line -> mass / line -> size) < mod_min)
	lst_add (line, rm_lst);
      if (mod_max != -1 && (line -> mass / line -> size) > mod_max)
	lst_add (line, rm_lst);
    }
  foreach (line, rm_lst)
    w2_destroy_line (ext_image, line);
  lst_destroy (rm_lst);
}

/*
 */
void
w2_remove_lines_by_arg (ExtImage *ext_image)
/*			real     scal_pr_min,
			real     scal_max)*/
{
  Extremum *ext;
  Line     *line;
  List     *rm_lst;
  int      rm_flag;
  int      x, y;

  assert (ext_image);

  rm_lst = lst_create ();
  foreach (line, ext_image -> line_lst)
    {
      rm_flag = NO;
      foreach (ext, line -> ext_lst)
	{
	  x = line -> gravity_center_x - (real) (ext -> pos % line -> lx);
	  y = line -> gravity_center_y - (real) (ext -> pos / line -> lx);
	  if ((x*cos (ext -> arg) + y*sin (ext -> arg)) < 0)
	    rm_flag = YES;
	}
      if (rm_flag == YES)
	lst_add (line, rm_lst);
    }
  foreach (line, rm_lst)
    w2_destroy_line (ext_image, line);
  lst_destroy (rm_lst);
}
