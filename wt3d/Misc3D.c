/*
 * Misc3D.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *   Copyright 2002 Laboratoire de Physique, Ens Lyon.
 *  Written by Pierre Kestener.
 *
 *  The author may be reached (Email) at the address
 *      pierre.kestener@ens-lyon.fr
 *
 */

#include "wt3d.h"
//#include "../wt2d/line.h"
//#include "surflet.h"

#define INFINITE 10000000

/*----------------------------------------------------------------------
  _dist_3D_
  
  distance entre pos1 et pos2.
  --------------------------------------------------------------------*/
real
_dist_3D_(int pos1,
	  int pos2,
	  int lx,
	  int ly)
{
  int tmp1, tmp2, dy, dz;
  int dx = ( pos1 % lx ) - ( pos2 % lx );
  
  tmp1 = ( pos1 / lx );
  tmp2 = ( pos2 / lx );
  dy = (tmp1 % ly) - (tmp2 % ly);
  dz = (tmp1 / ly) - (tmp2 / ly);

  return dx * dx + dy * dy + dz * dz;
}

/*----------------------------------------------------------------------
  ExtMisClosestExtr
  
  Renvoie l'extremum le plus proche du point specifie
  --------------------------------------------------------------------*/
Extremum3Dsmall * ExtMisClosestExtr3Dsmall(ExtImage3Dsmall * extImage, int position)
{
  Extremum3Dsmall * bestExt = 0;
  real              bestDist, tmpDist;
  int               i, lx = extImage->lx;
  int               ly = extImage->ly;

  bestDist = INFINITE;
  for (i = 0; i < extImage->extrNb; i++)
    {
      if ((tmpDist = _dist_3D_(position, extImage->extr[i].pos, lx,ly)) < bestDist)
	{
	  bestDist = tmpDist;
	  bestExt  = &(extImage->extr[i]);
	}
    }

  return bestExt;
}

/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/
Extremum3Dsmall *
w3_closest_vc_ext3Dsmall (ExtImage3Dsmall* extImage,
			  int       position)
{
  Extremum3Dsmall * bestExt = 0;
  Extremum3Dsmall * ext_ptr;
  real        bestDist, tmpDist;
  int        i, lx = extImage->lx;
  int        ly = extImage->ly;

  bestDist = INFINITE;
  for (i = 0; i < extImage->extrNb; i++)
    {
      ext_ptr = &extImage->extr[i];
      if ((ext_ptr -> down || ext_ptr -> up)
	  && (tmpDist = _dist_3D_(position, ext_ptr -> pos, lx, ly)) < bestDist)
	{
	  bestDist = tmpDist;
	  bestExt  = &(extImage->extr[i]);
	}
    }

  return bestExt;
}

