/*
 * wt3d.h --
 *
 *   Header for the wt3d library.
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *   Copyright 2002 Laboratoire de Physique, Ens Lyon.
 *  Written by Pierre Kestener.
 *
 *  The author may be reached (Email) at the address
 *       pierre.kestener@ens-lyon.fr
 *
 */

#ifndef __WT3D_H__
#define __WT3D_H__

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


#include "extremum3d.h"
#include "../wt2d/list.h"

/*----------------------------------------------------------------------
  Structure ExtImage3D <--- changer ce nom de structure.
  --------------------------------------------------------------------*/
typedef struct ExtImage3D
{
  real       scale;	/* echelle de l'image                 */
  int        lx,ly,lz;	/* dimensions de l'image originale    */
  int        extrNb;	/* nombre d'extrema3D dans l'image    */
  Extremum3D *extr ;	/* tableau contenant tous les extrema *
			 * de ce niveau                       */
  int      chainNb;	/* nombre de chaines dans le niveau   */
  Extremum3D **chain;	/* acces aux premiers elements des    *
			 * chaines du niveau                  */
  int      nb_of_surflets;
  List     *surflet_lst;
  int      stamp;	/* cle commune a un groupe d'images   *
			 * liees entre elles ...              *
			 * 0 si l'image n'est pas chainee     *
			 * situes sur des vert chain !!!      */
  struct  ExtImage3D     /* c'est un champ que je rajoute (26/06/2001)*/
  * up, *down;           /* que je vais utiliser dans vc2s3D avec *
			  * l'option -meanmod                   */

} ExtImage3D;

/*----------------------------------------------------------------------
  Structure ExtImage3Dsmall
  --------------------------------------------------------------------*/
typedef struct ExtImage3Dsmall
{
  real            scale;	/* echelle de l'image                 */
  int             lx,ly,lz;	/* dimensions de l'image originale    */
  int             extrNb;	/* nombre d'extrema3D dans l'image    */
  Extremum3Dsmall *extr ;	/* tableau contenant tous les extrema *
				 * de ce niveau                       */
  struct  ExtImage3Dsmall * up, *down;
} ExtImage3Dsmall;

/* a laisser ici */
#include "surflet.h"

void         ExtIma3DsmallMinMax      (ExtImage3Dsmall *,real *,real *);


// dans Misc3D.c
real          _dist_3D_(int, int, int, int);
Extremum3Dsmall *   ExtMisClosestExtr3Dsmall (ExtImage3Dsmall *, int);
Extremum3Dsmall *   w3_closest_vc_ext3Dsmall (ExtImage3Dsmall *, int);

// dans wt3d.c
ExtImage3Dsmall * w3_ext_small_new   (int,int,int,int,real);
void       Ext3DImaUnlink_(ExtImage3D *);  /* hash_tables.c */
void       ExtIma3DDelete_(ExtImage3D *);  /* wt3d.c */
void       ExtIma3DsmallDelete_(ExtImage3Dsmall *);  /* wt3d.c */

int ExtChnDistance_3D_(int,int,int,int);  /* Chain3D.c */
void _CreateExt3DArray_(ExtImage3D *, Extremum3D **);  /* Chain3D.c */

int pt_to_pt_vert_chain_3Dsmall (Extremum3Dsmall **, ExtImage3Dsmall *,  ExtImage3Dsmall *, int, int, int, int, float, int); /* Chain3D.c */

int search_surflets (ExtImage3D *);  /* chain3d.c */




/*

int
w3_local_maxima (real *data,
		 int  lx,
		 int  ly);
int
w3_local_extrema (real *data,
		  int  lx,
		  int  ly);

int
w3_local_space_scale_maxima (real *data,
			     real *prev,
			     real *next,
			     real min_value,
			     int  lx,
			     int  ly,
			     int  x_min,
			     int  y_min,
			     int  x_max,
			     int  y_max);

int search_single_max (ExtImage *image);
int search_single_max2 (ExtImage *image, real, int);

*/
#endif /* __WT3D_H__ */
