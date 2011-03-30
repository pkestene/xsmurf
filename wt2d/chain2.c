/*
 * chain2.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: chain2.c,v 1.10 1999/05/22 17:01:27 decoster Exp $
 */

#include "wt2d_int.h"

#define INFINITE 10000000
#define INFINITEF 100000000.0

/*----------------------------------------------------------------------
  Structure PrefList
  Sert a implementer une liste chainee de pointeurs de 'Extremum'
  --------------------------------------------------------------------*/
typedef struct PrefList
{
  Extremum * ext;
  struct PrefList * next;
} PrefList;

#define YES 0
#define NO  1

#define RIEN -1
#define DEBUT_CHAINE -2
#define BOUCLE -3

int DISC_SIN[] = {0,1,1,1,0,-1,-1,-1};

/*----------------------------------------------------------------------
  _GetNewStamp_
  
  Compteur renvoyant un nouvel entier a chaque appel
  --------------------------------------------------------------------*/
int
_GetNewStamp_()
{
  static int counter = 1;
  return counter++;
}

/*----------------------------------------------------------------------
  _ComputeBox_
  
  Calcule les intervalles suivant x et y pour realiser une boite carree
  centree en center et de cote size
  --------------------------------------------------------------------*/
static void
_ComputeBox_(int center,
	     int boxsize,
	     int lx,
	     int ly,
	     int * xMin,
	     int * xMax,
	     int * yMin,
	     int * yMax) 
{
  int x,y;

  x = center % lx;
  y = center / lx;
  *xMin = x-boxsize  ;  if(*xMin<0)  *xMin = 0 ;
  *xMax = x+boxsize+1;  if(*xMax>lx) *xMax = lx;
  *yMin = y-boxsize  ;  if(*yMin<0)  *yMin = 0 ;
  *yMax = y+boxsize+1;  if(*yMax>ly) *yMax = ly;
}

/*----------------------------------------------------------------------
  ExtChnDistance_
  
  Calcule le carre de la distance entre deux points
  --------------------------------------------------------------------*/
int
ExtChnDistance_(int lx,
		int p1,
		int p2)
{
  int dx = ( p1 % lx ) - ( p2 % lx );
  int dy = ( p1 / lx ) - ( p2 / lx );

  return dx * dx + dy * dy;
}

/*----------------------------------------------------------------------
  _AddExtToList_

  Ajoute une reference a l'extremum srcExt dans la liste chainee 
  temporaire de dstExt. La fonction renvoie 0 si l'allocation memoire
  qu'elle contient est reussie, 1 sinon.
  --------------------------------------------------------------------*/
static int
_AddExtToList_(Extremum * dstExt,
	       Extremum * srcExt)
{
  PrefList * cursor;
  PrefList * new = (PrefList *) malloc (sizeof(PrefList));
  
  if (!new)
    return 1;
  new->next = NULL;
  new->ext  = srcExt;
  cursor = (PrefList *) dstExt->down;
  if (cursor)
    {
      while (cursor->next)
	cursor = cursor->next;
      cursor->next = new;
    }
  else
    dstExt->down = (Extremum *) new;
  return 0;
}

/*----------------------------------------------------------------------
  _GetBestFromList_

  Recupere de la liste chainee de srcExt le meilleur candidat au chainage
  --------------------------------------------------------------------*/
static Extremum *
_GetBestFromList_(Extremum * srcExt,
		  int      lx)
{
  PrefList * cursor  = (PrefList *) srcExt->down;
  Extremum * bestExt = NULL;
  int dist, distMin  = INFINITE;
  int position       = srcExt->pos;

  while (cursor) {
    dist = ExtChnDistance_(lx,cursor->ext->pos,position);
    if ( (dist < distMin) && (!cursor->ext->down) ) {
      distMin = dist;
      bestExt = cursor->ext;
    }
    cursor = cursor -> next ;
  }
  return bestExt;
}

/*----------------------------------------------------------------------
  _DeleteList_

  Detruit la liste chainee temporaire associee a l'extremum srcExt
  --------------------------------------------------------------------*/
static void
_DeleteList_(Extremum * srcExt)
{
  PrefList * toBeDeleted, * cursor = (PrefList *) srcExt->down;
  
  srcExt->down=NULL;
  while (cursor) {
    toBeDeleted = cursor;
    cursor      = cursor->next;
    free ( toBeDeleted );
  }
}

/*----------------------------------------------------------------------
  VerticalChain
  
  Etablissement des liens entre les extrema pointes par up et down.
  lx,ly    : dimensions de up et down.
  argSimil : caracteristique de la difference maximale d'argument
     qui est acceptable lors du chainage de 2 extrema. 1.0 -> ils doivent
     avoir exactement le meme argument, 0.0-> peuvent avoir n'importe
     lequel.
  boxsize  : taille d'une boite dans laquelle on cherche un extremum a
  chainer.
  --------------------------------------------------------------------*/
void
VerticalChain(Extremum ** down,
	      ExtImage * up,
	      int      lx,
	      int      ly,
	      int      boxsize,
	      real    argSimil)
{
  int size = lx*ly;		/* taille des buffers */
  int xMin,xMax,yMin,yMax;	/* pour definir des boites */
  int distMin,dist;
  int u,d,x,y;
  Extremum * tempExt;
  Extremum * upExt,* downExt;	/* pour alleger l'ecr. de down[d] & up[u] */
  
  /* PREMIERE PASSE: on parcourt les points upExt de up.
     Pour chacun d'eux, si cela est possible, on elit un candidat au chainage
     dans une boite de taille boxsize de down. Le point downExt retenu doit :
     .      ne pas etre deja chaine a un point de up
     .      avoir un argument compatible avec celui de upExt (critere argSimil)
     .      etre le plus proche possible de upExt (calcule par ExtChnDistance_)
     Quand ce point est identifie, on ajoute une reference a upExt dans
     sa liste chainee (geree par _AddExtToList_) */

  for (u = 0; u < up -> extrNb; u++)
    {
      upExt = up -> extr + u;
      if (!upExt -> down) /* extremum sans voisin par down */
	{
	  _ComputeBox_(upExt -> pos, boxsize, lx, ly,
		       &xMin, &xMax, &yMin, &yMax);
	  distMin = INFINITE;
	  tempExt = NULL;
	
	  for (x = xMin; x < xMax ; x++) /* parcours des points de la boite */
	    for ( y = yMin ; y < yMax; y++)
	      {
		if ((downExt = down[ d = x + lx * y]) && 
		    (fabs((1. - fabs(downExt->arg - upExt->arg)/M_PI)) > argSimil)
		    && ((dist = ExtChnDistance_(lx,d,upExt->pos)) < distMin))
		  {
		    distMin = dist;
		    tempExt = downExt;
		  }
	      }
	  
	  if (tempExt && (!tempExt->up))
	    _AddExtToList_(tempExt,upExt);
	}
    }
  
  /* DEUXIEME PASSE: on parcourt maintenant les points downExt de down.
     Pour chacun d'eux, on va elir, si possible, un point de up parmi
     ceux presents dans la liste chainee creee precedemment.
     Le critere choisi est encore la distance, que l'on minimise. */
  
  for (d=0;d<size;d++)
    {
      downExt = down[d];
      if (downExt && (!downExt->up))
	{
	  upExt = _GetBestFromList_(downExt,lx);
	  if (upExt)
	    {
	      upExt->down = downExt;
	      downExt->up = upExt  ;
	    }
	  _DeleteList_(downExt);
      }
    }
}

static Extremum *
_get_best_from_list_(Extremum *srcExt,
		     int      lx)
{
  Extremum * bestExt = NULL;
  Extremum * ext;
  int dist, distMin  = INFINITE;
  int position       = srcExt->pos;
  List *lst_ptr = (List *) srcExt -> next;

  if (lst_ptr)
    foreach (ext, lst_ptr)
    {
      dist = ExtChnDistance_(lx, ext -> pos, position);
      if ( (dist < distMin) && (!ext->down) )
	{
	  distMin = dist;
	  bestExt = ext;
	}
    }
  return bestExt;
}

void
VerticalChainNew(Extremum ** down,
		 ExtImage * up,
		 int      lx,
		 int      ly,
		 int      boxsize,
		 real    argSimil)
{
  int size = lx*ly;		/* taille des buffers */
  int xMin,xMax,yMin,yMax;	/* pour definir des boites */
  int distMin,dist;
  int d,x,y;
  Extremum * tempExt;
  Extremum * upExt,* downExt;	/* pour alleger l'ecr. de down[d] & up[u] */
  List     *pref_lst;
  Line     *line_ptr;

  /* PREMIERE PASSE: on parcourt les points upExt de up.
     Pour chacun d'eux, si cela est possible, on elit un candidat au chainage
     dans une boite de taille boxsize de down. Le point downExt retenu doit :
     .      ne pas etre deja chaine a un point de up
     .      avoir un argument compatible avec celui de upExt (critere argSimil)
     .      etre le plus proche possible de upExt (calcule par ExtChnDistance_)
     Quand ce point est identifie, on ajoute une reference a upExt dans
     sa liste chainee (geree par _AddExtToList_) */

  /*  for (u = 0; u < up -> extrNb; u++)
    {*/
  foreach (line_ptr, up -> line_lst)
    foreach (upExt, line_ptr -> gr_lst)
    {
      pref_lst = lst_create ();
      if (!upExt -> down) /* extremum sans voisin par down */
	{
	  _ComputeBox_(upExt -> pos, boxsize, lx, ly,
		       &xMin, &xMax, &yMin, &yMax);
	  distMin = INFINITE;
	  tempExt = NULL;
	
	  for (x = xMin; x < xMax ; x++) /* parcours des points de la boite */
	    for ( y = yMin ; y < yMax; y++)
	      {
		downExt = down[ d = x + lx * y];
		dist = ExtChnDistance_ (lx, d, upExt -> pos);
		if (downExt && 
		    (fabs((1. - fabs(downExt->arg - upExt->arg)/M_PI)) > argSimil)
		    && (dist < distMin))
		  {
		    distMin = dist;
		    tempExt = downExt;
		  }
	      }

	  if (tempExt && (!tempExt->up))
	    {
	      /*	    _AddExtToList_(tempExt,upExt);*/
	      /* GROOOOOOOS BRICOLAGE !! */
	      if (tempExt -> next)
		lst_add (upExt, (List *) tempExt -> next);
	      else
		{
		  pref_lst = lst_create ();
		  lst_add (upExt, pref_lst);
		  tempExt -> next = (Extremum *) pref_lst;
		}
	    }
	}
    }
  
  /* DEUXIEME PASSE: on parcourt maintenant les points downExt de down.
     Pour chacun d'eux, on va elir, si possible, un point de up parmi
     ceux presents dans la liste chainee creee precedemment.
     Le critere choisi est encore la distance, que l'on minimise. */
  
  for (d=0;d<size;d++)
    {
      downExt = down[d];
      if (downExt && (!downExt->up))
	{
	  upExt = _get_best_from_list_ (downExt, lx);
	  if (upExt)
	    {
	      upExt->down = downExt;
	      downExt->up = upExt  ;
	    }
	  if (downExt -> next)
	    lst_destroy ((List *) downExt -> next);
	  downExt -> next = NULL;
      }
    }
}

/* this function is called by mlm_vert_chain_TclCmd_ in 
   interpreter/wt2d_cmds.c (corresponding to vchain command
   in xsmurf) */
/* modified on june 20th 2001 by pierre kestener
   amelioration du chainage : ajout de la condition dist < 50 */
/* modified on june 26th 2001 by pierre kestener
   a present on chaine egalement les ext-ima elles-meme */
void
vert_chain (ExtImage *do_ext_im,
	    ExtImage *up_ext_im,
	    int      lx,
	    int      ly,
	    int      box_size,
	    real     arg_simil,
	    int      is_first)
{
  int      i, x, y;
  int      d, dist, dist1, dist2, dist_min;
  int      x_min, x_max, y_min, y_max;
  float    mod_ratio;
  Line     *line_ptr;
  Extremum *up_ext, *do_ext;
  Extremum *tmp_ext;
  Extremum **up_array;
  List     *to_compute;
  List     *to_remove;

  /* on cree et initialise au pointeur NULL le tableau up_array */
  up_array = (Extremum **) malloc (sizeof(Extremum *)*lx*ly);
  for (i = 0; i < up_ext_im -> lx*up_ext_im -> ly; i++) {
    up_array[i] = NULL;
  }
  /* on remplit up_array avec les pointeurs des extrema
     qui sont des max locaux sur la ligne en question */
  foreach (line_ptr, up_ext_im -> line_lst) {
    foreach (up_ext, line_ptr -> gr_lst) {
      up_array[up_ext -> pos] = up_ext;
    }
  }

  /* on cree une liste vide */
  to_compute = lst_create ();

  /* boucle sur les extrema de l'image du bas pour garder 
     ceux qui sont deja sur une chaine verticale, ou ceux 
     qui sont sur le plancher */
  foreach (line_ptr, do_ext_im -> line_lst) {
    to_remove = lst_create ();
    foreach (do_ext, line_ptr -> gr_lst) {
      if (do_ext -> down || is_first) {
	lst_add (do_ext, to_compute);
      } else {
	lst_add (do_ext, to_remove);
      }
    }
    lst_destroy (to_remove);
  }

  /* */
  while (!lst_is_empty (to_compute)) {
    do_ext = lst_get (to_compute);
    _ComputeBox_ (do_ext -> pos, box_size, lx, ly,
		  &x_min, &x_max, &y_min, &y_max);
    dist_min = INFINITE;
    tmp_ext = NULL;

    if (do_ext->pos%lx == 48 && do_ext->pos/lx == 60) { /* ??? */
      tmp_ext = NULL;
    }

    /* on parcourt les points de la boite a la recherche d'un
       eventuel candidat au chainage */
    for ( y = y_min ; y < y_max; y++) {
      for (x = x_min; x < x_max ; x++) {
	d = x + lx * y;
	up_ext = up_array[d];
	if (up_ext) {
	  dist = ExtChnDistance_ (lx, d, do_ext -> pos);
	  mod_ratio = up_ext -> mod/do_ext -> mod;
	  /*if (up_ext
	      && ((fabs((1. - fabs(up_ext->arg - do_ext->arg)/M_PI)) > arg_simil) ||
		  (fabs((1. - fabs(up_ext->arg - do_ext->arg - M_PI)/M_PI)) > arg_simil))
	      && (mod_ratio > arg_simil && mod_ratio < 1.0/arg_simil)
	      && (dist < dist_min) && (dist < 50) ) {
	    dist_min = dist;
	    tmp_ext = up_ext;
	    }*/
	  
	  
	  if (up_ext
	      && (mod_ratio > arg_simil && mod_ratio < 1.0/arg_simil)
	      && (dist < dist_min) && (dist < 50) ) {
	    dist_min = dist;
	    tmp_ext = up_ext;
	  }
	  
	}
      }
    }

    /* le chainage a proprement dit */
    if (tmp_ext) {
      if (tmp_ext -> down) {
	dist1 = ExtChnDistance_ (lx, do_ext -> pos, tmp_ext -> pos);
	dist2 = ExtChnDistance_ (lx, tmp_ext -> down -> pos, tmp_ext -> pos);
	if (dist1 < dist2) {
	  /*   lst_add (tmp_ext->down, to_compute);*/
	  tmp_ext -> down -> up = NULL;
	  do_ext -> up    = tmp_ext;
	  tmp_ext -> down = do_ext;
	}
      } else {
	do_ext -> up    = tmp_ext;
	tmp_ext -> down = do_ext;
      }
    }
  }
  free (up_array);

  /* chainage des ext-ima elles-meme */
  do_ext_im->up   = up_ext_im;
  up_ext_im->down = do_ext_im;
  
}


/* this function is called by pt_to_pt_vert_chain_TclCmd_ in 
   interpreter/wt2d_cmds.c (corresponding to vchain2 command
   in xsmurf, for isolated maxima) */
void
pt_to_pt_vert_chain (ExtImage *do_ext_im,
		     ExtImage *up_ext_im,
		     int      lx,
		     int      ly,
		     int      box_size,
		     real     arg_simil,
		     int      is_first)
{
  int      i, x, y;
  int      d, dist, dist1, dist2, dist_min;
  int      x_min, x_max, y_min, y_max;
  float    mod_ratio;
  Extremum *up_ext = NULL, *do_ext;
  Extremum *tmp_ext;
  Extremum **up_array;
  List     *to_compute;

  up_array = (Extremum **) malloc (sizeof(Extremum *)*lx*ly);
  for (i = 0; i < up_ext_im -> lx*up_ext_im -> ly; i++) {
    up_array[i] = NULL;
  }
  for (i = 0; i < up_ext_im -> extrNb; i++) {
    up_ext = &up_ext_im -> extr[i];
    up_array[up_ext -> pos] = up_ext;
  }

  to_compute = lst_create ();
  for (i = 0; i < do_ext_im -> extrNb; i++) {
    do_ext = &do_ext_im -> extr[i];
    if (do_ext -> down || is_first)
      lst_add (do_ext, to_compute);
  }

  while (!lst_is_empty (to_compute))
    {
      do_ext = lst_get (to_compute);
      _ComputeBox_ (do_ext -> pos, box_size, lx, ly,
		    &x_min, &x_max, &y_min, &y_max);
      dist_min = INFINITE;
      tmp_ext = NULL;
	
      for (x = x_min; x < x_max ; x++) /* parcours des points de la boite */
	for ( y = y_min ; y < y_max; y++)
	  {
	    d = x + lx * y;
	    up_ext = up_array[d];
	    if (up_ext)
	      {
		dist = ExtChnDistance_ (lx, d, do_ext -> pos);
		mod_ratio = up_ext -> mod/do_ext -> mod;
		if (up_ext
		    && (fabs((1. - fabs(up_ext->arg - do_ext->arg)/M_PI)) > arg_simil)
		    && (mod_ratio > arg_simil && mod_ratio < 1.0/arg_simil)
		    && (dist < dist_min))
		  {
		    dist_min = dist;
		    tmp_ext = up_ext;
		  }
	      }
	  }
      if (tmp_ext)
	{
	  if (tmp_ext -> down)
	    {
	      dist1 = ExtChnDistance_ (lx, do_ext -> pos, tmp_ext -> pos);
	      dist2 = ExtChnDistance_ (lx, tmp_ext -> down -> pos, tmp_ext -> pos);
	      if (dist1 < dist2)
		{
		  tmp_ext -> down -> up = NULL;
		  do_ext -> up    = tmp_ext;
		  tmp_ext -> down = do_ext;
		}
	    }
	  else
	    {
	      do_ext -> up    = tmp_ext;
	      tmp_ext -> down = do_ext;
	    }
	}
    }
  free (up_array);

  /* chainage des ext-ima elles-meme */
  do_ext_im->up   = up_ext_im;
  up_ext_im->down = do_ext_im;


}

/*----------------------------------------------------------------------
  _GETCLOSESTDIRECTIONS_
  
  Cette procedure calcule les positions des 3 plus proches voisins dans
  la direction du gradient passe en argument.
  --------------------------------------------------------------------*/
static void
_GetClosestDirections_(int   lx,
		       real a,
		       int   n[])
{
  real fdir;
  int idir;

  fdir = a * 4.0 / M_PI + 8.0;
  idir = (int) (fdir - 1.5) ;
  n[0] = DISC_SIN[(idir+6)   % 8] + DISC_SIN[ idir    % 8]*lx;
  n[1] = DISC_SIN[(idir+7)   % 8] + DISC_SIN[(idir+1) % 8]*lx;
  n[2] = DISC_SIN[(idir+13)  % 8] + DISC_SIN[(idir+7) % 8]*lx;
}
  
/*----------------------------------------------------------------------
  _CreateExtArray_  
  
  A partir d'une ExtImage de dimension lx*ly, met dans un tableau de
  taille lx*ly une reference vers chacun des Extrema a leur position
  dans l'image.
  --------------------------------------------------------------------*/
 void
_CreateExtArray_(ExtImage * image,
		 Extremum ** array)
{
  int i;
  
  for (i=0;i<image->lx*image->ly;i++)
    array[i] = NULL;
  for (i=0;i<image->extrNb;i++)
    array[image->extr[i].pos] = &(image->extr[i]);
}

/*----------------------------------------------------------------------
  _JoinChains_

  A partir du bout d'une chaine donnee on cherche si il n'y a pas un
  debut de chaine proche de cette fin de chaine.
  --------------------------------------------------------------------*/
static int
_JoinChains_(int      chainNb,
	     ExtImage * downExtImage,
	     int      * label,
	     int      * last,
	     Extremum * tmpExt,
	     int      lx,
	     int      n,
	     int      fin)
{
  int newNb = 0;

  switch(label[n]){
  case RIEN:
    label[n] = DEBUT_CHAINE;
    newNb++;
    if ( fin != n )
      switch(label[fin]){
      case DEBUT_CHAINE:
	newNb--;
	last[n] = last[fin];
	label[last[fin]] = n;
	label[fin] = n;
	if(last[fin] != fin)
	  label[fin] = 1000;
	tmpExt->next = downExtImage->chain[fin];
	downExtImage->chain[fin]->prev = tmpExt;
	break;
      case RIEN:
	label[fin] = n;
	last[n] = fin;
	last[fin] = fin;
	tmpExt->next = downExtImage->chain[fin];
	downExtImage->chain[fin]->prev = tmpExt;
	break;
      case BOUCLE:
	last[n] = n;
	break;
      default:
	last[n] = n;
	break;
      }
    break;
  case DEBUT_CHAINE:
    break;
  case BOUCLE:
    break;
  default:
    switch(label[fin]){
    case RIEN:
      label[fin] = label[n];
      last[label[n]] = fin;
      last[n] = fin;
      last[fin] = fin;
      label[n] = 1000;
      tmpExt->next = downExtImage->chain[fin];
      downExtImage->chain[fin]->prev = tmpExt;
      break;
    case DEBUT_CHAINE:
      if (last[fin] == n){
	label[fin] = BOUCLE;
	tmpExt->next = downExtImage->chain[fin];
	downExtImage->chain[fin]->prev = tmpExt;
      }
      else {
	label[fin] = label[n];
	last[label[n]] = last[fin];
	last[n] = last[fin];
	label[last[fin]] = label[n];
	label[n] = 1000;
	if (last[fin] != fin)
	  label[fin] = 1000;
	tmpExt->next = downExtImage->chain[fin];
	downExtImage->chain[fin]->prev = tmpExt;
	newNb--;
      }
    default:
      break;
    }
    break;
  }
  return newNb;
}
/*----------------------------------------------------------------------
  _LinkUsingHigherScale_
  
  Passage a l'echelle superieur pour effectuer un chainage horizontal.
  --------------------------------------------------------------------*/
static Extremum **
_LinkUsingHigherScale_(Extremum **down,
		       int lx,
		       int ly)
{
  Extremum * downExt, * tmpExt;
  int      downSize = lx*ly;
  int      d;

  for (d=0;d<downSize;d++)
    if ((downExt=down[d]) &&
	((downExt->pos%lx)>1) && ((downExt->pos%lx)<(lx-1)) &&
	((downExt->pos/lx)>1) && ((downExt->pos/lx)<(lx-1))) {
		           	          /*      --next-->             */
	if ((tmpExt = downExt->up)  &&    /*   up|         |down        */
	    (tmpExt = tmpExt->next) &&    /*     |         |            */ 
	    (tmpExt = tmpExt->down) &&    /*  downExt.....downExt->next */
	    (ExtChnDistance_(lx, tmpExt->pos, d) < 9))  {
	  downExt->next = tmpExt;
	  tmpExt->prev = downExt;
	}
	if ((tmpExt = downExt->up)  && 
	    (tmpExt = tmpExt->prev) && 
	    (tmpExt = tmpExt->down) &&
	    (ExtChnDistance_(lx, tmpExt->pos, d) < 9)){
	  downExt->prev = tmpExt;
	  tmpExt->next = downExt;
	}
      }
  return down;
}

/*----------------------------------------------------------------------
  _LinkUsingGradient_

  Chainage horizontal dans la direction du gradient.
  --------------------------------------------------------------------*/
static Extremum **
_LinkUsingGradient_(Extremum **down,
		    int      lx,
		    int      ly)
{
  Extremum * downExt, * tmpExt, * bestExt;
  int      d, n, x, y;
  int      neighbours[3];


  for (x=1;x<lx-1;x++)
    for (y=1;y<ly-1;y++)
      {
	downExt = down[d = x + lx * y];
	if (downExt)
	  {
	    int dist, distMin = INFINITE;
	    /* Voisins dans la direction et dans le sens du gradient... */
	
	    bestExt     = NULL;
	    _GetClosestDirections_(lx,downExt->arg,neighbours);
	    for (n=0;n<3;n++)
	      {
		tmpExt = down[d + neighbours[n]];
		if (tmpExt)
		  {
		    dist = ExtChnDistance_(lx, downExt->pos, tmpExt->pos);
		    if (dist < distMin)
		      {
			distMin = dist;
			bestExt = tmpExt;
		      }
		  }
	      }
	    if (bestExt){
	      if(downExt->next)
		downExt->next->prev = NULL;
	      downExt->next = bestExt;
	    }

	    /*
	     * Voisins dans la direction du gradient mais dans le sens oppose 
	     * a celui du gradient.
	     */

	    bestExt     = NULL;
	    _GetClosestDirections_(lx,downExt->arg+M_PI,neighbours);
	    distMin = INFINITE;
	    for (n=0;n<3;n++)
	      {
		tmpExt = down[d + neighbours[n]];
		if (tmpExt)
		  {
		    dist = ExtChnDistance_(lx, downExt->pos, tmpExt->pos);
		    if (dist < distMin)
		      {
			distMin = dist;
			bestExt = tmpExt;
		      }
		  }
	      }
	    if (bestExt){
	      if(downExt->prev)
		downExt->prev->next = NULL;
	      downExt->prev = bestExt;
	    }
	  }
      }
  return down;
}

/*----------------------------------------------------------------------
  _DoubleLinkExtremum_

  On complete les liens qui ne sont pas dans les deux sens :
  ext->next->prev = ext
  ext->prev->next = ext  
  --------------------------------------------------------------------*/
static Extremum **
_DoubleLinkExtremum_(Extremum ** down,
		     int      lx,
		     int      ly)
{
  Extremum * downExt, * tmpExt;
  int      d;
  int      downSize = lx*ly;

  for (d=0;d<downSize;d++)
    {
      downExt = down[d];
      if (downExt)
	{
	  tmpExt = downExt->next;
	  if (tmpExt)
	    {
	      if (!tmpExt->prev)
		tmpExt->prev = downExt;
	      else {
		if (tmpExt->prev != downExt) {
		  if(tmpExt->prev->next == downExt)
		    tmpExt->prev = downExt;
		  else
		    downExt->next = NULL;
		}
	      }
	    }
	  tmpExt = downExt->prev;
	  if (tmpExt)
	    {
	      if (!tmpExt->next)
		tmpExt->next = downExt;
	      else {
		if (tmpExt->next != downExt) {
		  if(tmpExt->next->prev == downExt)
		    tmpExt->next = downExt;
		  else
		    downExt->prev = NULL;
		}
	      }
	    }
	}
    }
  return down;
}

/*----------------------------------------------------------------------
  _SearchChainBegin_

  Recherche des elements qui sont au debut des chaines (element qcq pour
  les boucles). On ne garde que ces elements dans down.
  La valeur de retour est le nombre de chaines.
  --------------------------------------------------------------------*/
static int
_SearchChainBegin_(Extremum ** down,
		   int      lx,
		   int      ly)
{
  Extremum * downExt,* last;
  int      d;
  int      downSize = lx*ly;
  int      chainNb = 0;

  for (d=0 ; d<downSize ; d++)
    {
      downExt = down[d];
      if (downExt)
	{
	  Extremum * firstExt;

	  /* cet element sera l'origine de la chaine */
	  firstExt = downExt;
      
	  /* on enleve entierement les references a cette chaine dans down */

	  /* d'abord en remontant dans le sens de next */
	  downExt = firstExt->next;
	  while ((downExt != NULL)&&(downExt != firstExt)) {
	    down[downExt->pos] = NULL;
	    last = downExt;
	    downExt = downExt->next;
	  }
	  /* puis en descendant dans le sens de prev pour les chaines qui */
	  /* ne sont pas des boucles afin de trouver le premier element.  */
	  if (downExt == NULL){
	    downExt = firstExt->prev;
	    while(downExt != NULL){
	      down[downExt->next->pos] = NULL;
	      firstExt = downExt;
	      downExt = downExt->prev;
	    }
	  }
	  /*
	   * on place en chainNb le premier element de la chaine : on memorise
	   * ainsi la chaine a un endroit ou on ne risque pas de la parcourir
	   * de nouveau. Pour les boucles cet element est quelconque, et, lors
	   * des parcours de la chaine il faut guetter un nouveau passage sur
	   * le premier element, pour achever le parcours.

	   */
 
	  down[chainNb] = firstExt;
	  chainNb++;
	  down[firstExt->pos] = NULL;
	}
    }
  return chainNb;
}
/*----------------------------------------------------------------------
  _UpdateExtImage_

  On met dans downExtImage les references des debuts de chaines.
  On retourne le nouveau nombre de chaines.
  --------------------------------------------------------------------*/
static int
_UpdateExtImage_(Extremum ** down,
		 ExtImage * downExtImage,
		 int      chainNb)
{
  int n;

  downExtImage->chain = (Extremum **) malloc (sizeof(Extremum*) * chainNb);

  if(!downExtImage->chain)
    exit(0);

  for (n=0;n<chainNb;n++)
      downExtImage->chain[n] = down[n];
  downExtImage->chainNb = chainNb;

  return chainNb;
}

int
_ChainSize_(Extremum *chain)
{
  int nb = 0;
  Extremum *ext;

  ext = chain;
  if(!ext){
    return 0;
  }
  do {
    nb++;
  }
  while((ext = ext->next) && (ext != chain));
  
  return nb;
}

/*----------------------------------------------------------------------
  _RemoveShortChains_

  Enleve toutes les chaines qui comportent 1 seul extremum.
  --------------------------------------------------------------------*/
int
_RemoveShortChains_(ExtImage * downExtImage)
{
  int i;

  for(i=0;i<downExtImage->chainNb;) {
    if(_ChainSize_(downExtImage->chain[i]) < 3){
      downExtImage->chain[i] = downExtImage->chain[downExtImage->chainNb-1];
      downExtImage->chainNb--;
    }
    else
      i++;
  }
  return 1;
}

/*----------------------------------------------------------------------
  _JoinNearestChains_

  Pour chaque chaine on regarde si la fin n'est pas proche du debut d'une
  autre chaine. Le cas echeant on reuni les deux chaines.
  On retourne le nouveau nombre de chaines.
  --------------------------------------------------------------------*/
static int
_JoinNearestChains_(ExtImage *downExtImage)
{
  Extremum ** newChain, * tmpExt;
  int      chainNb = downExtImage->chainNb;
  int      * label;
  int      * last;
  int      n, i, j, fin, stop;
  int      newNb = 0;
  int      lx = downExtImage->lx;

  label = (int*) malloc (sizeof(int)*(chainNb));
  last  = (int*) malloc (sizeof(int)*(chainNb));
  if(!last || !label)
    exit(0);

  for (n=0;n<chainNb;n++){
    last[n] = -1;
    if (!downExtImage->chain[n]->prev)
      label[n] = RIEN;
    else {
      label[n] = BOUCLE;
      newNb++;
    }
  }

  for (n=0;n<chainNb;n++){
    if (label[n] != BOUCLE){
      tmpExt = downExtImage->chain[n];
      while(tmpExt->next)
	tmpExt = tmpExt->next;
      stop = NO;
      for(i=-1;i<=1;i++)
	for(j=-1;j<=1;j++)
	  for(fin=0;fin<chainNb;fin++)
	    if((stop == NO) &&
	       ((i!=0)||(j!=0)) &&
	       (n != fin) &&
	       ((downExtImage->chain[fin]->pos%lx)>1) &&      /* pour ne pas */
	       ((downExtImage->chain[fin]->pos%lx)<(lx-1)) && /* depasser    */
	       ((downExtImage->chain[fin]->pos/lx)>1) &&      /* les bords.  */
	       ((downExtImage->chain[fin]->pos/lx)<(lx-1)) &&
	       (downExtImage->chain[fin]->pos == (tmpExt->pos +i+j*lx))){
	      newNb += _JoinChains_(chainNb, downExtImage, label, last,
				    tmpExt, lx, n, fin);
	      stop = YES;
	    }
      if(label[n] == RIEN){
	label[n] = DEBUT_CHAINE;
	newNb++;
	last[n] = n;
      }
    }
  }

  i=0;
  newChain = (Extremum **) malloc (sizeof(Extremum*) * (newNb+1));
  if(!newChain)
    exit(0);

  for (n=0;n<chainNb;n++){
    if ((label[n] == DEBUT_CHAINE)||(label[n] == BOUCLE)){
      newChain[i] = downExtImage->chain[n];
      i++;
    }
  }
  newChain[i] = downExtImage->chain[0];
  free(downExtImage->chain);
  downExtImage->chain = newChain;
  downExtImage->chainNb = newNb+1;

  free(last);
  free(label);

  return downExtImage->chainNb;
}
/*----------------------------------------------------------------------
  _JoinChainsUsingUpperLink_

  --------------------------------------------------------------------*/
static int
_JoinChainsUsingUpperLink_(ExtImage *downExtImage)
{
  Extremum ** newChain, * tmpExt;
  int      chainNb = downExtImage->chainNb;
  int      * label;
  int      * last;
  int      n, i, fin;
  int      newNb = 0;
  int      lx = downExtImage->lx;

  label = (int*) malloc (sizeof(int)*(chainNb));
  last  = (int*) malloc (sizeof(int)*(chainNb));

  if(!last || !label)
    exit(0);

  for (n=0;n<chainNb;n++){
    last[n] = -1;
    if (!downExtImage->chain[n]->prev)
      label[n] = RIEN;
    else {
      label[n] = BOUCLE;
      newNb++;
    }
  }

  /*
   * Dans les cas suivant:
   *
   * echelle sup       ---*----*------*----*-----*---
   *                      |                      |
   * echelle courante  ---*-----*--*    *---*----*---
   *
   * Si la distance entre les deux points non relie n'est pas trop
   * grande, on fait le lien.
   */

  for (n=0;n<chainNb;n++){
    if (label[n] != BOUCLE){
      Extremum *endExt;
      tmpExt = downExtImage->chain[n];
      while((tmpExt->next != NULL) && (tmpExt->next != downExtImage->chain[n]))
	tmpExt = tmpExt->next;
      endExt = tmpExt;
      while(!tmpExt->up &&
	    tmpExt->prev &&
	    (tmpExt->prev != downExtImage->chain[n]))
	tmpExt = tmpExt->prev;
      if(tmpExt->up){
	Extremum * upExt = tmpExt->up;
	tmpExt = tmpExt->up;
	while((tmpExt = tmpExt->next) &&
	      !tmpExt->down &&
	      (tmpExt != upExt));
	if(tmpExt->down){
	  Extremum *downExt = tmpExt->down;
	  tmpExt = tmpExt->down;
	  while(tmpExt->prev &&
		(tmpExt->prev != tmpExt) &&
		(tmpExt->prev != downExt))
	    tmpExt = tmpExt->prev;
	  if(!tmpExt->prev)
	    for(fin=0;fin<chainNb;fin++)
	      if((downExtImage->chain[fin]->pos == tmpExt->pos) &&
		 (n != fin) &&
		 (ExtChnDistance_(lx, endExt->pos, tmpExt->pos)<9))
		newNb += _JoinChains_(chainNb, downExtImage, label, last,
				      endExt, lx, n, fin);
	}
      }
      if(label[n] == RIEN){
	label[n] = DEBUT_CHAINE;
	newNb++;
	last[n] = n;
      }
    }
  }

  i=0;
  newChain = (Extremum **) malloc (sizeof(Extremum*) * newNb);
  if(!newChain)
    exit(0);

  for (n=0;n<chainNb;n++)
    if ((label[n] == DEBUT_CHAINE)||(label[n] == BOUCLE)){
      newChain[i] = downExtImage->chain[n];
      i++;
    }
  free(downExtImage->chain);
  downExtImage->chain = newChain;
  downExtImage->chainNb = newNb;

  free(last);
  free(label);

  return downExtImage->chainNb;
}


/*----------------------------------------------------------------------
  _HorizontalChain_
  
  A appeler apres _VerticalChain_.
  Chaine les points de down, en utilisant eventuellement le chainage
  de l'echelle superieure...
  --------------------------------------------------------------------*/
void
_HorizontalChain_(Extremum ** down,
		  ExtImage * downExtImage,
		  int      lx,
		  int      ly)
{
  int chainNb;

  down = _LinkUsingHigherScale_(down, lx, ly);
  down = _LinkUsingGradient_(down, lx, ly);
  /* on fait en sorte que ext->next->prev = ext et  */
  /*                      ext->prev->next = ext     */
  down = _DoubleLinkExtremum_(down,lx,ly);
  chainNb = _SearchChainBegin_(down, lx, ly);
  chainNb = _UpdateExtImage_(down, downExtImage, chainNb);
  chainNb = _JoinNearestChains_(downExtImage);
  chainNb = _JoinChainsUsingUpperLink_(downExtImage);
}

