/*
 * Chain3D.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *   Copyright 2002 Laboratoire de Physique, Ens Lyon.
 *  Written by Pierre Kestener.
 *
 *  The author may be reached (Email) at the address
 *       pierre.kestener@ens-lyon.fr
 *
 */

#include "math.h"
#include "wt3d_int.h"

#define INFINITE 10000000
#define INFINITEF 100000000.0

/*----------------------------------------------------------------------
  Structure PrefList
  Sert a implementer une liste chainee de pointeurs de 'Extremum'
  --------------------------------------------------------------------*/
typedef struct PrefList3Dsmall
{
  Extremum3Dsmall * ext;
  struct PrefList3Dsmall * next;
} PrefList3Dsmall;

#define YES 0
#define NO  1


/*----------------------------------------------------------------------
  _ComputeBox_3D_
  
  Calcule les intervalles suivant x, y et z pour realiser une boite
  cubique centree en center et de cote size
  rappel : index = z*lx*ly + y*lx + x
  --------------------------------------------------------------------*/
static void
_ComputeBox_3D_(int center,
		int boxsize,
		int lx,
		int ly,
		int lz,
		int * xMin,
		int * xMax,
		int * yMin,
		int * yMax,
		int * zMin,
		int * zMax) 
{
  int x,y,z,tmp;
  
  x   = center % lx;
  tmp = center / lx;
  y   = tmp    % ly;
  z   = tmp    / ly;
  
  *xMin = x-boxsize  ;  if(*xMin<0)  *xMin = 0 ;
  *xMax = x+boxsize+1;  if(*xMax>lx) *xMax = lx;
  *yMin = y-boxsize  ;  if(*yMin<0)  *yMin = 0 ;
  *yMax = y+boxsize+1;  if(*yMax>ly) *yMax = ly;
  *zMin = z-boxsize  ;  if(*zMin<0)  *zMin = 0 ;
  *zMax = z+boxsize+1;  if(*zMax>lz) *zMax = lz;
}

/*----------------------------------------------------------------------
  ExtChnDistance_3D_
  
  Calcule le carre de la distance entre deux points
  --------------------------------------------------------------------*/
int
ExtChnDistance_3D_(int lx,
		   int ly,
		   int p1,
		   int p2)
{
  int tmp, dy, dz;
  int dx = ( p1 % lx ) - ( p2 % lx );

  tmp = ( p1 / lx ) - ( p2 / lx );
  dy = tmp % ly;
  dz = tmp / ly;


  return dx * dx + dy * dy + dz * dz;
}

/*----------------------------------------------------------------------
  _AddExt3DsmallToList_

  Ajoute une reference a l'extremum srcExt dans la liste chainee 
  temporaire de dstExt. La fonction renvoie 0 si l'allocation memoire
  qu'elle contient est reussie, 1 sinon.
  --------------------------------------------------------------------*/
static int
_AddExt3DsmallToList_(Extremum3Dsmall * dstExt,
		      Extremum3Dsmall * srcExt)
{
  PrefList3Dsmall * cursor;
  PrefList3Dsmall * new = (PrefList3Dsmall *) malloc (sizeof(PrefList3Dsmall));
  
  if (!new)
    return 1;
  new->next = NULL;
  new->ext  = srcExt;
  cursor = (PrefList3Dsmall *) dstExt->down;
  if (cursor)
    {
      while (cursor->next)
	cursor = cursor->next;
      cursor->next = new;
    }
  else
    dstExt->down = (Extremum3Dsmall *) new;
  return 0;
}



/*----------------------------------------------------------------------
  _CreateExt3DArray_  
  
  A partir d'une ExtImage 3D de dimension lx*ly*lz, met dans un tableau 
  de taille lx*ly*lz une reference vers chacun des Extrema3D a leur 
  position dans l'image.
  --------------------------------------------------------------------*/
 void
_CreateExt3DArray_(ExtImage3D * image,
		   Extremum3D ** array)
{
  int i;
  
  for (i=0;i<image->lx*image->ly*image->lz;i++)
    array[i] = NULL;
  for (i=0;i<image->extrNb;i++)
    array[image->extr[i].pos] = &(image->extr[i]);
}

/* this function is called by pt_to_pt_vert_chain_3Dsmall_TclCmd_ in 
   interpreter/wt3d_cmds.c (corresponding to vchain3Dsmall command
   in xsmurf, for isolated maxima) */
int
pt_to_pt_vert_chain_3Dsmall (Extremum3Dsmall **up_array,
			     ExtImage3Dsmall *do_ext_im,
			     ExtImage3Dsmall *up_ext_im,
			     int      lx,
			     int      ly,
			     int      lz,
			     int      box_size,
			     float    arg_simil,
			     int      is_first)
{
  int               i, x, y, z;
  int               d, dist, dist1, dist2, dist_min;
  int               x_min, x_max, y_min, y_max, z_min, z_max;
  float             mod_ratio;
  //float            arg_simil=0.8;
  Extremum3Dsmall  *up_ext = NULL, *do_ext;
  Extremum3Dsmall  *tmp_ext;
  //Extremum3Dsmall **up_array;
  List             *to_compute;
  int nb_vc = 0;

  //up_array = (Extremum3Dsmall **) malloc (sizeof(Extremum3Dsmall *)*lx*ly*lz);
  for (i = 0; i < up_ext_im->lx * up_ext_im->ly * up_ext_im->lz; i++) {
    up_array[i] = NULL;
  }
  for (i = 0; i < up_ext_im -> extrNb; i++) {
    up_ext = &up_ext_im -> extr[i];
    up_array[up_ext -> pos] = up_ext;
  }
  
  to_compute = lst_create ();
  for (i = 0; i < do_ext_im -> extrNb; i++) {
    do_ext = &do_ext_im -> extr[i];
    if (do_ext -> down || is_first) {
      lst_add (do_ext, to_compute);
      nb_vc++;
      do_ext->tag=0;
    }
  }
  
  while (!lst_is_empty (to_compute))
    {
      do_ext = lst_get (to_compute);
      _ComputeBox_3D_ (do_ext -> pos, box_size, lx, ly, lz,
		       &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);
      dist_min = INFINITE;
      tmp_ext = NULL;
	
      for (x = x_min; x < x_max ; x++) /* parcours des points de la boite */
	for ( y = y_min ; y < y_max; y++)
	  for ( z = z_min ; z < z_max; z++)
	    {
	      d = x + lx*y + lx*ly*z;
	      up_ext = up_array[d];
	      if (up_ext)
		{
		  dist = ExtChnDistance_3D_ (lx,ly, d, do_ext -> pos);
		  mod_ratio = up_ext -> mod/do_ext -> mod;
		  if (up_ext 
		      && (dist < dist_min)
		      && (mod_ratio > arg_simil && mod_ratio < 1.0/arg_simil))
		    {
		      dist_min = dist;
		      tmp_ext = up_ext;
		    }
		}
	    }
      if (tmp_ext)
	{
	  if (tmp_ext -> down && tmp_ext->down->pos)
	    {
	      dist1 = ExtChnDistance_3D_ (lx,ly, do_ext -> pos, tmp_ext -> pos);
	      dist2 = ExtChnDistance_3D_ (lx,ly, tmp_ext -> down -> pos, tmp_ext -> pos);
	      if (dist1 < dist2)
		{
		  tmp_ext -> down -> up = NULL;
		  do_ext -> up    = tmp_ext;
		  tmp_ext -> down = do_ext;
		}
	    }
	  else
	    {
	      do_ext  -> up       = tmp_ext;
	      tmp_ext -> down     = do_ext;
	      tmp_ext -> down->pos = do_ext->pos;
	    }
	}
    }
  //free (up_array);

  return nb_vc;
}
