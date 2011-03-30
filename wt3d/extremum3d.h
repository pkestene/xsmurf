/*
 * created september 25 2002
 * Pierre Kestener
 *
 * un extremum issu des operations lissage-gradient en 3D
 * possede theoriquement un module et 2 arguments
 * pour l'instant nous laissons de cote les aspects angulaires...
 *
 */


#ifndef __EXTREMUM3D_H__
#define __EXTREMUM3D_H__

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

#endif


/*----------------------------------------------------------------------
  Structure Extremum3D
  --------------------------------------------------------------------*/
typedef struct Extremum3D
{
  struct Extremum3D 	/* pour le chainage des extrema        */
    * prev, * next,	/*  - a un echelle donnee              */
    * up  , * down;	/*  - entre les echelles               */

  real mod;		/* module de l'extremum    */
  int  pos;		/* position de l'extremum dans l'image */
                        /* pos = i + lx*j + lx*ly*k            */
  void *surflet;        /* pointeur sur la surface qui contient 
			   l'extremun en question (a initialiser
			   en faisant un hsearch3D)            */  
} Extremum3D;

/*----------------------------------------------------------------------
  Structure Extremum3Dsmall
  --------------------------------------------------------------------*/
typedef struct Extremum3Dsmall
{
  struct Extremum3Dsmall /* pour le chainage des extrema        */
    * up  , * down;      /*  - a un echelle donnee              */
                	 /*  - entre les echelles               */
  int tag;
  real mod;		/* module de l'extremum    */
  int  pos;		/* position de l'extremum dans l'image */
                        /* pos = i + lx*j + lx*ly*k            */
} Extremum3Dsmall;


Extremum3D * create_extremum3D();
void       destroy_extremum3D (Extremum3D *);
void       copy_extremum3D    (Extremum3D *, Extremum3D *);

#endif /*__EXTREMUM3D_H__*/
