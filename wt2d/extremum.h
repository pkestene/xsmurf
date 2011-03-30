#ifndef __EXTREMUM_H__
#define __EXTREMUM_H__

/*#include "../smurf/def.h"*/
#ifndef __DEF_H__
#define __DEF_H__

#ifndef _REAL_
#define _REAL_

typedef float real;

#endif

#ifndef _COMPLEX_
#define _COMPLEX_
typedef struct
{
  real real;
  real imag;
} complex;
#endif

#define swap(a,b) tmp=(a);(a)=(b);(b)=tmp
#define sup(a,b) (a>b?a:b)
#define inf(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

/*enum {REAL, CPLX, FOUR_NR};*/

#endif

/*----------------------------------------------------------------------
  Structure Extremum
  --------------------------------------------------------------------*/
typedef struct Extremum
{
  struct Extremum 	/* pour le chainage des extrema        */
    * prev, * next,	/*  - a un echelle donnee              */
    * up  , * down;	/*  - entre les echelles               */

  real mod,arg;		/* module et argument de l'extremum    */
  int  pos;		/* position de l'extremum dans l'image */
  void *line;           /* pointeur sur la ligne qui contient 
			   l'extremun en question (a initialiser
			   en faisant un hsearch)              */
  /* fields added April 2nd 2002 by Pierre Kestener
     to handle computation of discrete tangent and bending */
  int      a,b,octant;   /* a and b allow to compute normal vector */
  real     xe,ye;        /* xe and ye are coordinate of euclidean
			  * point near extremum             */
  real     theta;        /* equals to atan(b/a) */
  real     bend;
  
} Extremum;

Extremum * create_extremum();
void       destroy_extremum (Extremum *);
void       copy_extremum    (Extremum *, Extremum *);

#endif /*__EXTREMUM_H__*/
