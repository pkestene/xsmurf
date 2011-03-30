/*
 * Extrema.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: Extrema.c,v 1.7 1999/03/31 14:23:56 decoster Exp $
 */

#include "wt2d_int.h"

#define EPSILON 0.1
#define NO_ARG 10

#define TRUE 1
#define FALSE 0

#define DISTANCE(x1, y1, x2, y2) (sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)))

/*----------------------------------------------------------------------
  _Int_

  Renvoie l'entier le plus proche du reel passe en argument
  --------------------------------------------------------------------*/
int
_Int_ (real x)
{
  return (x > 0) ? (int) (x + .5) : (int) (x - .5);
}

static unsigned char * _maximaTable_  = NULL;
static int             _maximaNumber_ = 0;

/*----------------------------------------------------------------------
  _InitMaximaTable_

  Initialise une structure memorisant la position des maxima d'une image
  de dimension lx*ly.
  --------------------------------------------------------------------*/
void
_InitMaximaTable_(int size)
{
  int          bufferSize = size/8+1;
  register int i;

  _maximaNumber_= 0;
  _maximaTable_ = (unsigned char *) malloc (bufferSize*sizeof(unsigned char));
  for (i=0;i<bufferSize;i++)
    _maximaTable_[i] = (unsigned char) 0;
}

/*----------------------------------------------------------------------
  _DeleteMaximaTable_
  --------------------------------------------------------------------*/
 void
_DeleteMaximaTable_()
{
  if (_maximaTable_)
    free(_maximaTable_);
}

/*----------------------------------------------------------------------
  _IsMaximum_  
  
  Renvoie vrai si il y a un maximum a la position k.
  --------------------------------------------------------------------*/
int
_IsMaximum_(int k)
{
  return _maximaTable_[k/8] & ((unsigned char) (1 <<(k % 8)) );
}

/*----------------------------------------------------------------------
  _RegisterMaximum_  
  
  Enregistre la position k comme etant celle d'un maximum dans l'image.
  --------------------------------------------------------------------*/
static void
_RegisterMaximum_(int k)
{
  _maximaNumber_++;
  _maximaTable_[k/8] |= (unsigned char) (1<<(k % 8));
}

/*----------------------------------------------------------------------
  _RemoveMaximum_  
  
  Enleve de la position k la marque d'un maximum
  --------------------------------------------------------------------*/
static void
_RemoveMaximum_(int k)
{
  _maximaNumber_--;
  _maximaTable_[k/8] &= (unsigned char) (~(1<<(k % 8)));
}

/*********************** minima *******************************/


static unsigned char * _minimaTable_  = NULL;
static int             _minimaNumber_ = 0;

/*----------------------------------------------------------------------
  _InitMinimaTable_

  Initialise une structure memorisant la position des minima d'une image
  de dimension lx*ly.
  --------------------------------------------------------------------*/
void
_InitMinimaTable_(int size)
{
  int          bufferSize = size/8+1;
  register int i;

  _minimaNumber_= 0;
  _minimaTable_ = (unsigned char *) malloc (bufferSize*sizeof(unsigned char));
  for (i=0;i<bufferSize;i++)
    _minimaTable_[i] = (unsigned char) 0;
}

/*----------------------------------------------------------------------
  _DeleteMinimaTable_
  --------------------------------------------------------------------*/
void
_DeleteMinimaTable_()
{
  if (_minimaTable_)
    free(_minimaTable_);
}

/*----------------------------------------------------------------------
  _IsMinimum_  
  
  Renvoie vrai si il y a un minimum a la position k.
  --------------------------------------------------------------------*/
int
_IsMinimum_(int k)
{
  return _minimaTable_[k/8] & ((unsigned char) (1 <<(k % 8)) );
}

/*----------------------------------------------------------------------
  _RegisterMinimum_  
  
  Enregistre la position k comme etant celle d'un minimum dans l'image.
  --------------------------------------------------------------------*/
void
_RegisterMinimum_(int k)
{
  _minimaNumber_++;
  _minimaTable_[k/8] |= (unsigned char) (1<<(k % 8));
}

/*----------------------------------------------------------------------
  _RemoveMinimum_  
  
  Enleve de la position k la marque d'un minimum
  --------------------------------------------------------------------*/
void
_RemoveMinimum_(int k)
{
  _minimaNumber_--;
  _minimaTable_[k/8] &= (unsigned char) (~(1<<(k % 8)));
}

static void
_purge_(int  k,
	int  lx,
	real *data,
	real niv,
	real eps)
{
  int  tab[9];
  int  count = 0, i, j, ii;
  real tmp;
  int  k_prime;
  
  for (i = -1; i <= 1; i++)
    for (j = -1; j <= 1; j++)
      {
/*	printf("%d %d %d %g\n", i, j, k+i+j*lx, data[k+i+j*lx]);
	fflush(stdout);*/
	if (_IsMaximum_(k+i+j*lx))
	  {
	    k_prime = k+i+j*lx;
	    for (ii = 0; ii < count; ii++)
	      if (fabs(data[tab[ii]]-niv) < fabs(data[k_prime]-niv))
		{
		  tmp = k_prime;
		  k_prime = tab[ii];
		  tab[ii] = tmp;
		}
	    tab[count] = k_prime;
	    
	    count++;
	  }
      }
  if (count>4)
    for (i = 0; i < (count-4); i++)
      _RemoveMaximum_(tab[i]);
}

/*----------------------------------------------------------------------
  _FindExtrema_
  
  Cherche les maxima du gradient dans la direction du gradient.
  modul,arg  : tableau du module et de l'argument du gradient de l'image.
  lx,ly      : dimensions de l'image.
  fLevel     : booleen indiquant si on est au plus petit niveau.
  --------------------------------------------------------------------*/
int
_FindExtrema_(real *modul,
	      real *arg,
	      int   lx,
	      int   ly,
	      int   fLevel,
	      real  thresh,
	      real  rthresh,
	      real  min,
	      real  max,
	      real  eps)
{
  register int k;
  int          dir, size = lx*ly;
  real        maxMod = 0.0;
  real        d, m1, m2;
  int          kp1, kp2, km1, km2;
  real        epsilon = (max-min)*eps;

/*
  for (k = 0; k < lx*ly; k++)
    {
      dir = _Int_ (arg[k] * 2.0 / M_PI + 4.0);      
      
      switch (dir % 2)
        {
        case 0:
          k1 = k - 1;
          k2 = k + 1;
          break;
        case 1:
          k1 = k - lx;
          k2 = k + lx;
          break;
        }
      if ((k1<0) || (k2>=size))
        continue;
      
      if ((modul[k]>modul[k1]) && (modul[k]>modul[k2]))
        _RegisterMaximum_(k);
    }
*/  

  /* on parcourt toute l'image */
  for (k = 0; k < size; k++) {
    if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
      continue;
    if(arg[k] == NO_ARG)
      continue;

    if(arg[k]>M_PI)
      arg[k] -= 2*M_PI;
    if(arg[k]<-M_PI)
      arg[k] += 2*M_PI;

    dir = (int) (arg[k] * 4.0 / M_PI + 4.0);

    switch (dir % 4) {
    case 0:
      if (arg[k]>=0)
	d = 4*arg[k]/M_PI;
      else
	d = fabs(4*(arg[k]+M_PI)/M_PI);
      kp1 = k + 1;
      kp2 = k - 1;
      km1 = k + lx + 1;
      km2 = k - lx - 1;
      break;
    case 1:
      if (arg[k]>=0)
	d = 4*(arg[k]-M_PI/4)/M_PI;
      else
	d = 4*(arg[k]+3*M_PI/4)/M_PI;
      kp1 = k+lx+1;
      kp2 = k-lx-1;
      km1 = k+lx;
      km2 = k-lx;
      break;
    case 2:
      if (arg[k]>=0)
	d = 4*(arg[k]-M_PI/2)/M_PI;
      else
	d = 4*(arg[k]+M_PI/2)/M_PI;
      kp1 = k+lx;
      kp2 = k-lx;
      km1 = k+lx-1;
      km2 = k-lx+1;
      break;
    case 3:
      if (arg[k]>=0)
	d = 4*(arg[k]-3*M_PI/4)/M_PI;
      else
	d = 4*(arg[k]+M_PI/4)/M_PI;
      kp1 = k+lx-1;
      kp2 = k-lx+1;
      km1 = k-1;
      km2 = k+1;
      break;
      }

    /* pour eviter l'eventuelle utilisation de points en dehors de l'image */
/*    if ((kp1<1) || (km1<1) || (kp2>=(size-1)) || (km2>=(size-1)))
      continue;*/
/*    if(((kp1%lx) <= 0) || ((kp1%lx) == ) ||
       ((kp1/lx) <= 0) || (kp1 >= (size-1)))
  */  
    m1 = ((1-d)*modul[kp1]+d*modul[km1]);
    m2 = ((1-d)*modul[kp2]+d*modul[km2]);
    if ((modul[k]>m1) && (modul[k]>m2) && ((modul[k]-inf(m1,m2))>epsilon)) {
      _RegisterMaximum_(k);	
      if (modul[k]>maxMod)
	maxMod=modul[k];
    }
  }
  /* un premier passage du seuillage : on elimine tous les maxima inferieurs
     a thresh ou maxMod*rthresh */
  if ((rthresh>0) && (rthresh * maxMod / 100.0 > thresh))
    thresh = rthresh * maxMod / 100.0;

  if (thresh>0)
    for ( k = 0 ; k < size ; k++ )
      if ((_IsMaximum_(k)) && (modul[k]<thresh))
	_RemoveMaximum_(k);

  /*  traitement specifique, que l'on effectue a la plus petite echelle,
   *  pour recuperer quelques maxima de plus. 
   *  Pour un point donne,
   *        . si son gradient a une direction diagonale
   *        . si il est entoure de maxima dans la direction de son gradient
   *  Alors on teste si le point n'est pas un maximum dans les directions
   *  horizontale et verticale. */

/*  if (fLevel)
    for (x = 1; x < lx - 1; x++)
      for (y = 1; y < ly - 1; y++)
	if (_IsMaximum_(k = x + y * lx)==0)
	  if ((thresh<0) || (modul[k]>=thresh))
	  {  
	    dir = _Int_(arg[k]*4.0/M_PI+8.0)%8;
	    
	    switch (dir)
	      {
	      case 1:
	      case 5:
		bool = 
		  (modul[k-1+lx] &&
		   (modul[k-lx] || modul[k-lx+1] || modul[k+1]))
		    ||
		      (modul[k+1-lx] &&
		       (modul[k+lx] || modul[k+lx-1] || modul[k-1]));
		break;
	      case 3:
	      case 7:
		bool = 
		  (modul[k+1+lx] &&
		   (modul[k-lx] || modul[k-lx-1] || modul[k-1]))
		    ||
		      (modul[k-1-lx] &&
		       (modul[k+lx] || modul[k+lx+1] || modul[k+1]));
		break;
	      default:
		continue;
	      }
	    if (bool)
	      if (((modul[k]>modul[k-lx]) && (modul[k]>modul[k+lx]))
		  || ((modul[k]>modul[k-1]) && (modul[k]>modul[k+1])))
		_RegisterMaximum_(k);
	  }*/
  return _maximaNumber_;
}


/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/
int
_FindExtrema2_(real *modul,
	       real *arg,
	       int   lx,
	       int   ly,
	       int   fLevel,
	       real  thresh,
	       real  rthresh,
	       real  min,
	       real  max,
	       real  eps)
{
  register int k;
  int          dir, size = lx*ly;
  real        d, m1, m2;
  int          kp1, kp2, km1, km2;

  /* on parcourt toute l'image */
  for (k = 0; k < size; k++) {
    if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
      continue;
    if(arg[k] == NO_ARG)
      continue;

    if(arg[k]>M_PI)
      arg[k] -= 2*M_PI;
    if(arg[k]<-M_PI)
      arg[k] += 2*M_PI;

    dir = (int) (arg[k] * 4.0 / M_PI + 4.0);

    switch (dir % 4) {
    case 0:
      if (arg[k]>=0)
	d = 4*arg[k]/M_PI;
      else
	d = fabs(4*(arg[k]+M_PI)/M_PI);
      kp1 = k + 1;
      kp2 = k - 1;
      km1 = k + lx + 1;
      km2 = k - lx - 1;
      break;
    case 1:
      if (arg[k]>=0)
	d = 4*(arg[k]-M_PI/4)/M_PI;
      else
	d = 4*(arg[k]+3*M_PI/4)/M_PI;
      kp1 = k+lx+1;
      kp2 = k-lx-1;
      km1 = k+lx;
      km2 = k-lx;
      break;
    case 2:
      if (arg[k]>=0)
	d = 4*(arg[k]-M_PI/2)/M_PI;
      else
	d = 4*(arg[k]+M_PI/2)/M_PI;
      kp1 = k+lx;
      kp2 = k-lx;
      km1 = k+lx-1;
      km2 = k-lx+1;
      break;
    case 3:
      if (arg[k]>=0)
	d = 4*(arg[k]-3*M_PI/4)/M_PI;
      else
	d = 4*(arg[k]+M_PI/4)/M_PI;
      kp1 = k+lx-1;
      kp2 = k-lx+1;
      km1 = k-1;
      km2 = k+1;
      break;
      }

    m1 = ((1-d)*modul[kp1]+d*modul[km1]);
    m2 = ((1-d)*modul[kp2]+d*modul[km2]);
    if ((modul[k]<m1) && (modul[k]<m2))
      _RegisterMaximum_(k);	
  }
  /* un premier passage du seuillage : on elimine tous les maxima inferieurs
     a thresh ou maxMod*rthresh */
/*  if ((rthresh>0) && (rthresh * maxMod / 100.0 > thresh))
    thresh = rthresh * maxMod / 100.0;
*/
/*  if (thresh>0)
    for ( k = 0 ; k < size ; k++ )
      if ((_IsMaximum_(k)) && (modul[k]<thresh))
	_RemoveMaximum_(k);*/
  return _maximaNumber_;
}

/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/
int
_FindMaximaKapap_(real *kapa,
		  real *kapap,
		  real *mod,
		  int   lx,
		  int   ly,
		  real  min,
		  real  max,
		  real  eps,
		  real  mult)
{
  register int k;
  int          size = lx*ly;

  /* on parcourt toute l'image */
  for (k = 0; k < size; k++) {
    if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
      continue;
/*    if ((kapap[k] < eps) && (fabs(kapa[k]) < (mult*fabs(kapap[k]))))*/
    if ((kapap[k] < 0) && (fabs(kapa[k]) < (mult/mod[k]*(-kapap[k]))))
      _RegisterMaximum_(k);	
  }
  return _maximaNumber_;
}

/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/
int
_FindMinimaKapap_(real *kapa,
		  real *kapap,
		  real *mod,
		  int   lx,
		  int   ly,
		  real  min,
		  real  max,
		  real  eps,
		  real  mult)
{
  register int k;
  int          size = lx*ly;

  /* on parcourt toute l'image */
  for (k = 0; k < size; k++) {
    if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
      continue;
/*    if ((kapap[k] < eps) && (fabs(kapa[k]) < (mult*fabs(kapap[k]))))*/
    if ((kapap[k] > 0) && (fabs(kapa[k]) < (mult/mod[k]*(kapap[k]))))
      _RegisterMaximum_(k);	
  }
  return _maximaNumber_;
}

/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/
int
_FindContourLine_(real *kapa,
		  real *kapap,
		  int   lx,
		  int   ly,
		  int   min_or_max)
{
  register int current, x, y;
  int          size = lx*ly;
  int          nb_of_better_values;
  int          register_ok, sign_ok, old_sign;
  int          better_x[3], better_y[3];

  for (current = 0; current < size; current++)
    {
      if ((current < lx) || (current > (size-lx)) ||
	  ((current%lx) == 0) || ((current%lx) == (lx-1)))
	continue; /* we avoid borders */

      if ((kapap[current]*min_or_max) < 0)
	register_ok = TRUE;
      else
	register_ok = FALSE;
      nb_of_better_values = 0;
      sign_ok = FALSE;
      old_sign = 0;
      for (x = -1; x <= 1 && register_ok; x++)
	for (y = -1; y <= 1 && register_ok; y++)
	  {
	    if ((kapa[current+x+y*lx]*old_sign) < 0)
	      sign_ok = TRUE;
	    if (kapa[current+x+y*lx] != 0)
	      old_sign = fabs (kapa[current+x+y*lx])/kapa[current+x+y*lx];
	    else if (x == 0 && y == 0)
	      sign_ok = TRUE;
	    if ((x != 0 || y != 0) &&
		(fabs (kapa[current+x+y*lx]) < fabs (kapa[current])))
	      {
		if (nb_of_better_values >= 3)
		  register_ok = FALSE;
		else
		  {
		    better_x[nb_of_better_values] = x;
		    better_y[nb_of_better_values] = y;
		    nb_of_better_values++;
		  }
	      }
	  }
      if (register_ok && sign_ok)
	{
	  real somme;
	
	  if (nb_of_better_values == 3)
	    {
	      somme = (DISTANCE (better_x[0], better_y[0],
				 better_x[1], better_y[1]) +
		       DISTANCE (better_x[0], better_y[0],
				 better_x[2], better_y[2]) +
		       DISTANCE (better_x[2], better_y[2],
				 better_x[1], better_y[1]));
	      if (somme >= 5)
		_RegisterMaximum_ (current);
	    }
	  else if ((nb_of_better_values == 2) &&
		   (DISTANCE (better_x[0], better_y[0],
			      better_x[1], better_y[1]) >= 2 ))
	    _RegisterMaximum_ (current);
	  else
	    _RegisterMaximum_ (current);
	}
    }
  return _maximaNumber_;
}

/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/
int
_FindExtremaKapa_(real *kapa,
		  real *mod,
		  int   lx,
		  int   ly,
		  real  min,
		  real  max,
		  real  eps,
		  real  mult)
{
  register int k;
  int          size = lx*ly;
  real        epsilon = (max-min)*0.015;

  /* on parcourt toute l'image */
  for (k = 0; k < size; k++) {
    if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
      continue;
    if (mod[k] < 1e-12)
      continue;
/*    if ((kapap[k] < eps) && (fabs(kapa[k]) < (mult*fabs(kapap[k]))))*/
    if (fabs(kapa[k]) < epsilon)
      _RegisterMaximum_(k);	
  }
  return _maximaNumber_;
}

/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/
int
_FindNiv_(real *data,
	  int   lx,
	  int   ly,
	  real  min,
	  real  max,
	  real  eps,
	  real  niv)
{
  register int k;
  int          size = lx*ly;
  real        epsilon = (max-min)*eps;

  /* on parcourt toute l'image */
  for (k = 0; k < size; k++) {
    if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
      continue;
/*    if ((kapap[k] < eps) && (fabs(kapa[k]) < (mult*fabs(kapap[k]))))*/
    if (fabs(data[k] - niv) < epsilon)
      _RegisterMaximum_(k);
  }
  for (k = 0; k < size; k++)
    {
      if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
	continue;
      if(_IsMaximum_(k))
	_purge_(k, lx, data, niv, epsilon);
    }
  return _maximaNumber_;
}

/*----------------------------------------------------------------------
  _FindExtremaII_
  
  courbe de niveau correspondant a mod max / 2
  --------------------------------------------------------------------*/
int
_FindExtremaII_(real * modul,
		real * arg,
		int   lx,
		int   ly,
		int   fLevel,
		real thresh,
		real rthresh)
{
  register int k;
  int          size = lx*ly;
  real        maxMod = 0.0, demimaxModinf, demimaxModsup;

  /* on parcourt toute l'image */
  for (k = 0; k < size; k++) {
    if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
      continue;
    if (modul[k]>maxMod)
      maxMod=modul[k];
  }
  /* un premier passage du seuillage : on elimine tous les maxima inferieurs
     a thresh ou maxMod*rthresh */
  demimaxModinf = maxMod*(1-thresh)/2;
  demimaxModsup = maxMod*(1+thresh)/2;
  for ( k = 0 ; k < size ; k++ )
    if ((modul[k]>(demimaxModinf))&&(modul[k]<(demimaxModsup)))
      _RegisterMaximum_(k);	

  return _maximaNumber_;
}

/*----------------------------------------------------------------------
  _FindCarac_
  
  --------------------------------------------------------------------*/
int
_FindCarac_(real * x,
	    real * y,
	    real * xx,
	    real * yy,
	    real * xy,
	    real * carac,
	    int   line,
	    int   lx,
	    int   ly,
	    int   fLevel,
	    real thresh,
	    real rthresh)
{
  register int k;
  int          size = lx*ly;
  real        maxCarac = 0.0;
  real        carac2;
  FILE         *out;


  out=fopen("out","w");
  /* on parcourt toute l'image */
  switch(line)
    {
    case CARAC:
      for (k = 0; k < size; k++) {
	if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
	  continue;
	carac[k] = xy[k]*(x[k]*x[k]-y[k]*y[k])+(yy[k]-xx[k])*x[k]*y[k];
	carac2 = carac[k]*carac[k];
	fprintf(out,"%d %d -> %g %g %g %g %g - %g %g\n", (k%lx), (int)(k/lx),
		x[k], y[k], xx[k], yy[k], xy[k],
		carac[k], carac2);
	if ((carac2<thresh) && ((x[k]*x[k]+y[k]*y[k])>rthresh)) {
	  _RegisterMaximum_(k);	
	  if (carac2>maxCarac)
	    maxCarac=carac2;
	}
      }
      break;
    case DIV:
      for (k = 0; k < size; k++) {
	if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
	  continue;
	carac[k] = yy[k]*x[k]*x[k]+xx[k]*y[k]*y[k]-2*xy[k]*x[k]*y[k];
	carac2 = carac[k]*carac[k];
	fprintf(out,"%d %d -> %g %g %g %g %g - %g %g\n", (k%lx), (int)(k/lx),
		x[k], y[k], xx[k], yy[k], xy[k],
		carac[k], carac2);
	if ((carac2<thresh) && ((x[k]*x[k]+y[k]*y[k])>rthresh)) {
	  _RegisterMaximum_(k);	
	  if (carac2>maxCarac)
	    maxCarac=carac2;
	}
      }
      break;
    case EDGE:
      for (k = 0; k < size; k++) {
	if((k < lx) || (k > (size-lx)) || ((k%lx) == 0) || ((k%lx) == (lx-1)))
	  continue;
	carac[k] = xx[k]*x[k]*x[k]+yy[k]*y[k]*y[k]+2*xy[k]*x[k]*y[k];
	carac2 = carac[k]*carac[k];
/*	fprintf(out,"%d %d -> %g %g %g %g %g - %g %g\n", (k%lx), (int)(k/lx),
		x[k], y[k], xx[k], yy[k], xy[k],
		carac[k], carac2);*/
	if ((carac2<thresh) && ((x[k]*x[k]+y[k]*y[k])>rthresh)) {
	  _RegisterMaximum_(k);	
	  if (carac2>maxCarac)
	    maxCarac=carac2;
	}
      }
      break;
    }
  /* un premier passage du seuillage : on elimine tous les maxima inferieurs
     a thresh ou maxMod*rthresh */
/*  if ((rthresh>0) && (rthresh * caracMod / 100.0 > thresh))
    thresh = rthresh * maxMod / 100.0;

  if (thresh>0)
    for ( k = 0 ; k < size ; k++ )
      if ((_IsMaximum_(k)) && (modul[k]<thresh))
	_RemoveMaximum_(k);*/

  return _maximaNumber_;
}

/*
 * Register all points from data that are local maxima.
 */
/* modification done on october the 26th 2001
   when working with unsigned char data, i can have
   2 local maxima right beneath, not detect, so i added
   an else condition to handle this */
int
w2_local_maxima (real *data,
		 int  lx,
		 int  ly)
{
  int x, y, index;

  for (y = 1; y < ly - 1; y++) {
    for (x = 1; x < lx - 1; x++) {
      index = x + lx*y;
      if ((data[index] > data[index - lx - 1])
	  && (data[index] > data[index - lx])
	  && (data[index] > data[index - lx + 1])
	  && (data[index] > data[index - 1])
	  && (data[index] > data[index + 1])
	  && (data[index] > data[index + lx - 1])
	  && (data[index] > data[index + lx])
	  && (data[index] > data[index + lx + 1])) {
	_RegisterMaximum_ (index);
      } else if ((data[index] > data[index - lx - 1])
		 && (data[index] > data[index - lx])
		 && (data[index] > data[index - lx + 1])
		 && (data[index] > data[index - 1])
		 && (data[index] >= data[index + 1])
		 && (data[index] >= data[index + lx - 1])
		 && (data[index] >= data[index + lx])
		 && (data[index] >= data[index + lx + 1])) {
   	_RegisterMaximum_ (index);
      }
    }
  }

  return _maximaNumber_; 
}

/*
 * Register all points from data that are local extrema.
 */
int
w2_local_extrema (real *data,
		 int  lx,
		 int  ly)
{
  int x, y, index;

  for (y = 1; y < ly - 1; y++) {
    for (x = 1; x < lx - 1; x++) {
      index = x + lx*y;
      if (((data[index] > data[index - lx - 1])
	   && (data[index] > data[index - lx])
	   && (data[index] > data[index - lx + 1])
	   && (data[index] > data[index - 1])
	   && (data[index] > data[index + 1])
	   && (data[index] > data[index + lx - 1])
	   && (data[index] > data[index + lx])
	   && (data[index] > data[index + lx + 1]))
	  || ((data[index] < data[index - lx - 1])
	      && (data[index] < data[index - lx])
	      && (data[index] < data[index - lx + 1])
	      && (data[index] < data[index - 1])
	      && (data[index] < data[index + 1])
	      && (data[index] < data[index + lx - 1])
	      && (data[index] < data[index + lx])
	      && (data[index] < data[index + lx + 1])))
	_RegisterMaximum_ (index);
    }
  }

  return _maximaNumber_; 
}

/*
 * Register all points from data that are local maxima in space and in scale.
 */
int
w2_local_space_scale_maxima (real *data,
			     real *prev_data,
			     real *next_data,
			     real min_value,
			     int  lx,
			     int  ly,
			     int  x_min,
			     int  y_min,
			     int  x_max,
			     int  y_max)
{
  int x, y, index;

  for (y = y_min; y < y_max; y++) {
    for (x = x_min; x <= x_max; x++) {
      index = x + lx*y;
      if ((data[index] > min_value)
	  && (data[index] > data[index - lx - 1])
	  && (data[index] > data[index - lx])
	  && (data[index] > data[index - lx + 1])
	  && (data[index] > data[index - 1])
	  && (data[index] > data[index + 1])
	  && (data[index] > data[index + lx - 1])
	  && (data[index] > data[index + lx])
	  && (data[index] > data[index + lx + 1])

	  && (data[index] > prev_data[index])
	  && (data[index] > prev_data[index - lx - 1])
	  && (data[index] > prev_data[index - lx])
	  && (data[index] > prev_data[index - lx + 1])
	  && (data[index] > prev_data[index - 1])
	  && (data[index] > prev_data[index + 1])
	  && (data[index] > prev_data[index + lx - 1])
	  && (data[index] > prev_data[index + lx])
	  && (data[index] > prev_data[index + lx + 1])

	  && (data[index] > next_data[index - lx - 1])
	  && (data[index] > next_data[index - lx])
	  && (data[index] > next_data[index - lx + 1])
	  && (data[index] > next_data[index - 1])
	  && (data[index] > next_data[index + 1])
	  && (data[index] > next_data[index + lx - 1])
	  && (data[index] > next_data[index + lx])
	  && (data[index] > next_data[index + lx + 1])
	  && (data[index] > next_data[index]))
	_RegisterMaximum_ (index);
    }
  }

  return _maximaNumber_; 
}

#define _is_in_mask_(k) (mask_data[k/8] & ((unsigned char) (128 >>(k % 8))))

/*
 * Register all points from ext_image that are outside a mask.
 */
int
w2_mask_ext_image (ExtImage *ext_image,
		   unsigned char *mask_data,
		   int      mask_lx,
		   int      mask_ly,
		   int      x_0,
		   int      y_0)
{
  int position;
  int new_position;
  int x, y;
  int i;
  int lx = ext_image -> lx;

  for (i = 0; i < ext_image -> extrNb; i++)
    {
      position = ext_image -> extr[i].pos;
      x = position % lx;
      y = position / lx;
      if (x >= x_0 && x < (x_0 + mask_lx)
	  && y >= y_0 && y < (y_0 + mask_ly))
	{
	  new_position = (x - x_0) + mask_lx*(y - y_0);
	  if (!_is_in_mask_ (new_position))
	    _RegisterMaximum_ (position);
	}
      else
	_RegisterMaximum_ (position);
    }

  return _maximaNumber_; 
}

/*
 * Register all points from ext_image that are greater than VALUE.
 */
int
w2_keep_by_value (ExtImage *ext_image,
		  real     value,
		  int      flag)
{
  int i;

  for (i = 0; i < ext_image -> extrNb; i++)
    {
      if ((flag == W2_MOD && ext_image -> extr[i].mod > value)
	  || (flag == W2_ARG && ext_image -> extr[i].arg > value))
	  _RegisterMaximum_ (ext_image -> extr[i].pos);
    }

  return _maximaNumber_; 
}

/*
 * Register all points from ext_image that are lesser than VALUE.
 it's just the contrary of w2_keep_by_value_low
 et comme j'ai pasenvie de m'embeter je recopie betement...
 */
int
w2_keep_by_value_low (ExtImage *ext_image,
		      real     value,
		      int      flag)
{
  int i;

  for (i = 0; i < ext_image -> extrNb; i++)
    {
      if ((flag == W2_MOD && ext_image -> extr[i].mod < value)
	  || (flag == W2_ARG && ext_image -> extr[i].arg < value))
	  _RegisterMaximum_ (ext_image -> extr[i].pos);
    }

  return _maximaNumber_; 
}

static int pos_incr[10];
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

/*static int pos_incr[9];
static void
init_pos_incr(int lx)
{
  pos_incr[0] = -1 - lx;
  pos_incr[1] =    - lx;
  pos_incr[2] =  1 - lx;
  pos_incr[3] = -1;
  pos_incr[4] =  0;
  pos_incr[5] =  1;
  pos_incr[6] = -1 + lx;
  pos_incr[7] =      lx;
  pos_incr[8] =  1 + lx;
}*/

static int
near_one_negative (real *kapa,
		   int  lx,
		   int  pos)
{
  int i;

  for (i = 1; i < 9; i = i+2)
    if (kapa [pos + pos_incr[i]] < 0)
      return 1;
  return 0;
}

/*
 * Register a contour line, new method.
 */
int
w2_folow_contour2 (real *kapa,
		  real *kapap,
		  int   lx,
		  int   ly,
		  int   min_or_max)
{
  int x, y, pos;

  init_pos_incr (lx);
  for (y = 1; y < (ly-1); y++) {
    for (x = 1; x < (lx-1); x++) {
      pos = x + y*lx;
      if (kapa [pos] > 0
	  && kapap[pos] < 0
	  && near_one_negative (kapa, lx, pos))
	_RegisterMaximum_ (pos);
    }
  }

  return _maximaNumber_;
}

static int
near_contour_line (real *kapa,
		   int  lx,
		   int  pos)
{
  int i;
  int n_pos;

  for (i = 1; i < 9; i = i+2)
    {
      n_pos = pos + pos_incr[i];
      if ((kapa[pos]*kapa[n_pos]) < 0
	  && fabs (kapa[pos]) < fabs (kapa[n_pos]))
	return i;
    }

  return 0;
}

/*
 * Register a contour line, new method (again).
 */
int
w2_folow_contour (real *kapa,
		   real *kapap,
		   int   lx,
		   int   ly,
		   int   min_or_max)
{
  int x, y, pos;

  init_pos_incr (lx);
  for (y = 1; y < (ly-1); y++) {
    for (x = 1; x < (lx-1); x++) {
      pos = x + y*lx;
      if (kapap[pos] < 0 && near_contour_line (kapa, lx, pos)) {
	_RegisterMaximum_ (pos);
      }
    }
  }

  return _maximaNumber_;
}

int
_near_zero_ (real *array,
		   int  pos,
		   int  lx)
{
  int i;

  for (i = 0; i < 8; i++)
    if (i != 4
	&& fabs (array[pos + pos_incr[i]]) < fabs (array[pos]))
      return 0;
  return 1;
}

#define NO_NEXT_PT -1
int
_next_contour_pt1_ (real *kapa,
		   real *kapap,
		   int  pos,
		   int  lx)
{
  int i;
  int pos1, pos2;

  for (i = 0; i < 8; i++)
    {
      pos1 = pos + pos_incr[i];
      pos2 = pos + pos_incr[i+1];
      if (kapa[pos1] < 0
	  && kapa[pos2] > 0)
	{
	  if (fabs(kapa[pos1]) < fabs(kapa[pos2])
	      && kapap[pos1] < 0)
	    return pos1;
	  if (fabs(kapa[pos2]) < fabs(kapa[pos1])
	      && kapap[pos2] < 0)
	    return pos2;
	}
    }
  return NO_NEXT_PT;
}
		   
int
_next_contour_pt2_ (real *kapa,
		   real *kapap,
		   int  pos,
		   int  lx)
{
  int i;
  int pos1, pos2;

  for (i = 7; i >= 0; i--)
    {
      pos1 = pos + pos_incr[i];
      pos2 = pos + pos_incr[i+1];
      if (kapa[pos1] < 0
	  && kapa[pos2] > 0)
	{
	  if (fabs(kapa[pos1]) < fabs(kapa[pos2])
	      && kapap[pos1] < 0)
	    return pos1;
	  if (fabs(kapa[pos2]) < fabs(kapa[pos1])
	      && kapap[pos2] < 0)
	    return pos2;
	}
    }
  return NO_NEXT_PT;
}
		   

/*
 * Register a contour line, an other new method.
 */
int
w2_folow_contour3 (real *kapa,
		   real *kapap,
		   int   lx,
		   int   ly,
		   int   min_or_max)
{
  int x, y, pos;
  int new_pos;

  init_pos_incr (lx);
  for (y = 1; y < (ly-1); y++) {
    for (x = 1; x < (lx-1); x++) {
      pos = x + y*lx;
      if (!_IsMaximum_ (pos))
	{
	  if (kapap[pos] < 0 && _near_zero_ (kapa, pos, lx))
	    {
	      new_pos = pos;
	      /*		do
				{
				_RegisterMaximum_ (new_pos);
				new_pos = _next_contour_pt1_ (kapa, kapap, new_pos, lx);
				}
				while (new_pos != NO_NEXT_PT &&	!_IsMaximum_ (new_pos));*/
	      new_pos = pos;
	      do
		{
		  _RegisterMaximum_ (new_pos);
		  new_pos = _next_contour_pt2_ (kapa, kapap, new_pos, lx);
		}
	      while (new_pos != NO_NEXT_PT &&	!_IsMaximum_ (new_pos));
	    }
	}
      /*	if (kapa [pos] > 0
		&& kapap[pos] < 0
		&& near_one_negative (kapa, lx, pos))
		_RegisterMaximum_ (pos);*/
    }
  }

  return _maximaNumber_;
}

