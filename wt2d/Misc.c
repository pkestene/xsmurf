/*
 * Misc.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: Misc.c,v 1.6 1999/03/31 13:09:32 decoster Exp $
 */

#include "wt2d.h"
#include "line.h"

#define INFINITE 10000000

/*----------------------------------------------------------------------
  _dist_
  
  distance entre pos1 et pos2.
  --------------------------------------------------------------------*/
real
_dist_(int pos1,
       int pos2,
       int lx)
{
  int dx = ( pos1 % lx ) - ( pos2 % lx );
  int dy = ( pos1 / lx ) - ( pos2 / lx );

  return dx * dx + dy * dy;
}

/*----------------------------------------------------------------------
  ExtMisClosestChain
  
  Renvoie la chaine la plus proche du point specifie
  --------------------------------------------------------------------*/
Extremum *
ExtMisClosestChain (ExtImage    *extImage,
		    int         position,
		    int         *index,
		    void        **line_ptr)
{
  Extremum * tmpExt, *firstExt, * bestChain = 0;
  int        bestDist, bestDistAlongChain, tmpDist;
  int        i, lx = extImage->lx;
  int        toto;
  int        nb;
  Line       *line, *best_line = 0;

  bestDist = INFINITE;
  for (i=0;i<extImage->chainNb;i++)
    {
      tmpExt             = extImage->chain[i];
      bestDistAlongChain = INFINITE;

      firstExt = tmpExt;
      do
	{
	  toto = tmpExt->pos;
	  tmpDist=ExtChnDistance_(lx,position,tmpExt->pos);
	  if (tmpDist < bestDistAlongChain)
	    bestDistAlongChain=tmpDist;
	  tmpExt = tmpExt->next;
	}
      while (tmpExt && (tmpExt != firstExt));
      if (bestDistAlongChain < bestDist)
	{
	  bestDist  = bestDistAlongChain;
	  bestChain = extImage->chain[i];
	  nb = i;
	  *index = i;
	}
    }

  bestDist = INFINITE;
  if (!lst_is_empty (extImage -> line_lst))
    {
      foreach (line, extImage -> line_lst)
	{
	  Extremum *extremum;
	  bestDistAlongChain = INFINITE;

	  foreach (extremum, line -> ext_lst)
	    {
	      tmpDist = ExtChnDistance_ (lx, position, extremum -> pos);
	      if (tmpDist  < bestDistAlongChain)
		bestDistAlongChain = tmpDist;
	    }
	  if (bestDistAlongChain < bestDist)
	    {
	      bestDist  = bestDistAlongChain;
	      best_line = line;
	      bestChain = line -> ext_chain;
	      nb = i;
	      *index = i;
	    }
	}

      *line_ptr = best_line;
    }
  *line_ptr = best_line;

   return bestChain;
}

/*----------------------------------------------------------------------
  ExtMisClosestExt
  
  Renvoie la chaine la plus proche du point specifie
  --------------------------------------------------------------------*/
Extremum * ExtMisClosestExt(ExtImage * extImage, int position)
{
  Extremum * tmpExt, * firstExt, * bestExt = 0;
  int        bestDist, tmpDist;
  int        i, lx = extImage->lx;
  int        nb;

  bestDist = INFINITE;
  for (i = 0; i < extImage->chainNb; i++)
    {
      tmpExt = extImage->chain[i];

      firstExt = tmpExt;
      do
	{
	  tmpDist = ExtChnDistance_(lx, position, tmpExt->pos);
	  if (tmpDist < bestDist){
	    bestExt = tmpExt;
	    bestDist = tmpDist;
	    nb = i;
	  }
	  tmpExt = tmpExt->next;
	}
      while (tmpExt && (tmpExt != firstExt));
    }
  return bestExt;
}

/*----------------------------------------------------------------------
  ExtMisClosestExtr
  
  Renvoie l'extremum le plus proche du point specifie
  --------------------------------------------------------------------*/
Extremum * ExtMisClosestExtr(ExtImage * extImage, int position)
{
  Extremum * bestExt = 0;
  real        bestDist, tmpDist;
  int        i, lx = extImage->lx;

  bestDist = INFINITE;
  for (i = 0; i < extImage->extrNb; i++)
    {
      if ((tmpDist = _dist_(position, extImage->extr[i].pos, lx)) < bestDist)
	{
	  bestDist = tmpDist;
	  bestExt  = &(extImage->extr[i]);
	}
    }

  return bestExt;
}

/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/
Extremum *
w2_closest_vc_ext (ExtImage* extImage,
		   int       position)
{
  Extremum * bestExt = 0;
  Extremum *ext_ptr;
  real        bestDist, tmpDist;
  int        i, lx = extImage->lx;

  bestDist = INFINITE;
  for (i = 0; i < extImage->extrNb; i++)
    {
      ext_ptr = &extImage->extr[i];
      if ((ext_ptr -> down || ext_ptr -> up)
	  && (tmpDist = _dist_(position, ext_ptr -> pos, lx)) < bestDist)
	{
	  bestDist = tmpDist;
	  bestExt  = &(extImage->extr[i]);
	}
    }

  return bestExt;
}

/*
 */
Extremum *
w2_closest_gr_ext (ExtImage *ext_image,
		   int      position)
{
  Extremum * bestExt = 0;
  Extremum *ext_ptr;
  real      bestDist, tmpDist;
  int       lx = ext_image -> lx;
  Line      *line_ptr;

  bestDist = INFINITE;
  foreach (line_ptr, ext_image -> line_lst)
    foreach (ext_ptr, line_ptr -> gr_lst)
    {
      tmpDist = _dist_(position, ext_ptr -> pos, lx);
      if (tmpDist < bestDist)
	{
	  bestDist = tmpDist;
	  bestExt  = ext_ptr;
	}
    }

  return bestExt;
}


/*----------------------------------------------------------------------
  ExtMisSortImageList_
  
  Trie par ordre decroissant d'echelle la liste d'ExtImage passee en
  parametre.
  --------------------------------------------------------------------*/
void ExtMisSortImageList_(ExtImage ** list)
{
  int        m,n,maxPos,numberFound;
  real      maxScale;
  ExtImage * tmpExtImage;

  /* calcul du nb d'elements */
  for (numberFound=0;list[numberFound];numberFound++);
  
  /*  -> tri en n2, j'vais pas m'taper un quicksort aujourd'hui ! */
  for (n=0;n<numberFound-1;n++)
    {
      maxPos   = n;
      maxScale = list[n]->scale;
      for (m=n+1;m<numberFound;m++)
	if (list[m]->scale>maxScale)
	  {
	    maxPos   = m;
	    maxScale = list[m]->scale;
	  }
      if (maxPos!=n)		/* on permute */
	{
	  tmpExtImage  = list[maxPos];
	  list[maxPos] = list[n];
	  list[n]      = tmpExtImage;
	}
    }
}
