#include "wt3d_int.h"

/* crade */
void Ext3DDicUnlinkStamp_(int stamp);

/*----------------------------------------------------------------------
  w3_ext_small_new
  
  Constructeur d'une ExtImage3Dsmall
  Entree : nbExtrema : nombre d'extrema 3D a stocker
  Sortie : 0 si echec, sinon une ExtImage3D alouee.
  --------------------------------------------------------------------*/
ExtImage3Dsmall *
w3_ext_small_new (int extremaNb, int lx, int ly, int lz, real scale)
{
  ExtImage3Dsmall *im;

  im = (ExtImage3Dsmall *) malloc (sizeof (ExtImage3Dsmall));

  if (!im)
    return NULL;
  
  im -> lx      = lx;
  im -> ly      = ly;
  im -> lz      = lz;
  im -> extrNb  = extremaNb;
  if (extremaNb)
    im -> extr = (Extremum3Dsmall *) malloc (extremaNb*sizeof(Extremum3Dsmall));
  else
    im -> extr = NULL;
  im -> scale   = scale;

  im -> up = im -> down = NULL;

  if (!im -> extr && extremaNb)
    {
      free (im);
      return NULL;
    }
 
  return im;
}


/*----------------------------------------------------------------------
  ExtIma3DDelete

  Destructeur de ExtImage3D
  Entree : im, une image alloue
  Sortie :
  Rem.   : on verifie la validite des allocations
  --------------------------------------------------------------------*/
void
ExtIma3DDelete_ (im)
     ExtImage3D *im;
{
  Surflet     *surflet;

  if (im)			/* precautions normalement inutiles */
    {
      foreach (surflet, im -> surflet_lst)
	destroy_surflet (surflet);
      lst_destroy (im -> surflet_lst);

      if (im -> extr)
	{
	  free (im -> extr);
	  if (im -> stamp)
	    ;
	    // !!!!!!!!!!!
	    // attention danger : il faudrait implementer ce qui suit...
	    //ExtIma3DUnlink_ (im);
	}
      free (im);
    }
}  

/*----------------------------------------------------------------------
  ExtIma3DsmallDelete

  Destructeur de ExtImage3Dsmall
  Entree : im, une image alloue
  Sortie :
  Rem.   : on verifie la validite des allocations
  --------------------------------------------------------------------*/
void
ExtIma3DsmallDelete_ (im)
     ExtImage3Dsmall *im;
{
  if (im)  /* precautions normalement inutiles */
    {
      if (im -> extr)
	{
	  free (im -> extr);
	}
      free (im);
    }
}  

/*----------------------------------------------------------------------
  ExtIma3DsmallMinMax

  Renvoie le min et le max du module des points de la ExtImage3Dsmall
  --------------------------------------------------------------------*/
void
ExtIma3DsmallMinMax (im, min, max)
     ExtImage3Dsmall *im;
     real       *min;
     real       *max;
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

/*
 * Remove a surflet from the list of surflets from a 3D ext image.
 */
Surflet *
w3_remove_surflet (ExtImage3D *ext_image3D,
		   Surflet    *surflet)
{
  Surflet *result;

  result = lst_remove (surflet, ext_image3D -> surflet_lst);
  if (result)
    ext_image3D -> nb_of_surflets --;
  
  return result;
}

/*
 * Destroy a surflet from the list of surflets from a 3D ext image.
 */
void
w3_destroy_surflet (ExtImage3D *ext_image3D,
		    Surflet     *surflet)
{
  Surflet *result;

  result = w3_remove_surflet (ext_image3D, surflet);
  if (result)
    destroy_surflet (result);
}

/*
 * If size_min is not -1, removes all the lines which size is lesser than
 * size_min.
 * If size_max is not -1, removes all the lines which size is greater than
 * size_max.
 */
void
w3_remove_surflets_by_size (ExtImage3D *ext_image3D,
			    int      size_min,
			    int      size_max)
{
  Surflet *surflet;
  List    *rm_lst;

  assert (ext_image3D);

  rm_lst = lst_create ();
  foreach (surflet, ext_image3D -> surflet_lst)
    {
      if (size_min != -1 && surflet -> size < size_min)
	lst_add (surflet, rm_lst);
      if (size_max != -1 && surflet -> size > size_max)
	lst_add (surflet, rm_lst);
    }
  foreach (surflet, rm_lst)
    w3_destroy_surflet (ext_image3D, surflet);

  lst_destroy (rm_lst);
}


/*
 */
void
w3_remove_surflets_by_mean_mod (ExtImage3D *ext_image3D,
				real     mod_min,
				real     mod_max)
{
  Surflet *surflet;
  List *rm_lst;

  assert (ext_image3D);

  rm_lst = lst_create ();
  foreach (surflet, ext_image3D -> surflet_lst)
    {
      if (mod_min != -1 && (surflet -> mass / surflet -> size) < mod_min)
	lst_add (surflet, rm_lst);
      if (mod_max != -1 && (surflet -> mass / surflet -> size) > mod_max)
	lst_add (surflet, rm_lst);
    }
  foreach (surflet, rm_lst)
    w3_destroy_surflet (ext_image3D, surflet);
  lst_destroy (rm_lst);
}

