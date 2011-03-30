/*
 * wt2d.h --
 *
 *   Header for the wt2d library.
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: wt2d.h,v 1.11 1999/04/01 13:33:34 decoster Exp $
 */

#ifndef __WT2D_H__
#define __WT2D_H__

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

/* define struct point used in chain.c, above all
 * in routine compute_discrete_tangent
 */
typedef struct point{
  int x;
  int y;
  real z;
} point;

#define swap(a,b) tmp=(a);(a)=(b);(b)=tmp
#define sup(a,b) (a>b?a:b)
#define inf(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)


#include "extremum.h"
#include "list.h"

/*----------------------------------------------------------------------
  Structure ExtImage <--- changer ce nom de structure.
  --------------------------------------------------------------------*/
typedef struct ExtImage
{
  real     scale;	/* echelle de l'image                 */
  int      lx,ly;	/* dimensions de l'image originale    */
  int      extrNb;	/* nombre d'extrema dans l'image      */
  Extremum *extr ;	/* tableau contenant tous les extrema *
			 * de ce niveau                       */
  int      chainNb;	/* nombre de chaines dans le niveau   */
  Extremum **chain;	/* acces aux premiers elements des    *
			 * chaines du niveau                  */
  int      nb_of_lines;
  List     *line_lst;
  int      stamp;	/* cle commune a un groupe d'images   *
			 * liees entre elles ...              *
			 * 0 si l'image n'est pas chainee     */
  /*  int      nbTagExt;*/    /* apres avoir utilise eitagvc        */
  /*int      nbNotagExt;*/	/* l'extimage max_scaleId contient    *
			 * nbTagExt extrema sur une vert chain*
			 * qui sont tagges et                 *
			 * nbNotagExt extrema non tagges      *
			 * !!! attention !!!                  *
			 * tout ca ne concerne que les extrema*
			 * situes sur des vert chain !!!      */

  struct  ExtImage       /* c'est un champ que je rajoute (26/06/2001)*/
  * up, *down;           /* que je vais utiliser dans vc2s avec */
                         /* l'option -meanmod                   */

} ExtImage;

/* a laisser ici */
#include "line.h"

void         ExtImaMinMax      (ExtImage *,real *,real *);
void         ExtImaMinMaxArg   (ExtImage *,real *,real *);
void         ExtImaMinMaxMx    (ExtImage *,real *,real *);
void         ExtImaMinMaxMy    (ExtImage *,real *,real *);

Extremum *   ExtMisClosestChain(ExtImage *, int, int *, void **);
Extremum *   ExtMisClosestExt(ExtImage *, int);
Extremum *   ExtMisClosestExtr (ExtImage *, int);
Extremum *   w2_closest_gr_ext (ExtImage *, int);
Extremum *   w2_closest_vc_ext (ExtImage *, int);

ExtImage * w2_ext_new   (int,int,int,real);
void       ExtImaUnlink_(ExtImage *);
void       ExtImaDelete_(ExtImage *);

int ExtChnDistance_(int,int,int);

/* a voir ... */
void _CreateExtArray_(ExtImage *, Extremum **);
void _HorizontalChain_(Extremum **, ExtImage *, int, int);
int  _GetNewStamp_();
void VerticalChain(Extremum **, ExtImage *, int, int,	int, real);
void VerticalChainNew(Extremum **, ExtImage *, int, int,	int, real);

int search_lines (ExtImage *);
/* new routine (april 2nd 2002) by pierre Kestener */
int compute_discrete_tangent (ExtImage *);
//int compute_discrete_tangent (ExtImage *,Line *);
//int compute_discrete_tangent (Line *);

void lines_to_chains (ExtImage *);

void _InitMaximaTable_(int size);
int _IsMaximum_(int k);
void _DeleteMaximaTable_();

int _FindNiv_(real *data,
	  int   lx,
	  int   ly,
	  real  min,
	  real  max,
	  real  eps,
	  real  niv);
int _FindExtrema2_(real *modul,
	       real *arg,
	       int   lx,
	       int   ly,
	       int   fLevel,
	       real  thresh,
	       real  rthresh,
	       real  min,
	       real  max,
	       real  eps);
int _FindMaximaKapap_(real *kapa,
		  real *kapap,
		  real *mod,
		  int   lx,
		  int   ly,
		  real  min,
		  real  max,
		  real  eps,
		  real  mult);
int _FindExtremaKapa_(real *kapa,
		  real *mod,
		  int   lx,
		  int   ly,
		  real  min,
		  real  max,
		  real  eps,
		  real  mult);
int _FindMinimaKapap_(real *kapa,
		  real *kapap,
		  real *mod,
		  int   lx,
		  int   ly,
		  real  min,
		  real  max,
		  real  eps,
		  real  mult);
int _FindContourLine_(real *kapa,
		  real *kapap,
		  int   lx,
		  int   ly,
		  int   min_or_max);
int _FindExtrema_(real *modul,
	      real *arg,
	      int   lx,
	      int   ly,
	      int   fLevel,
	      real  thresh,
	      real  rthresh,
	      real  min,
	      real  max,
	      real  eps);

int
w2_local_maxima (real *data,
		 int  lx,
		 int  ly);
int
w2_local_extrema (real *data,
		 int  lx,
		 int  ly);

int
w2_local_space_scale_maxima (real *data,
			     real *prev,
			     real *next,
			     real min_value,
			     int  lx,
			     int  ly,
			     int  x_min,
			     int  y_min,
			     int  x_max,
			     int  y_max);

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
	    real rthresh);

ExtImage *
w2_remove_extrema (ExtImage *ext_image,
		   int x_min,
		   int x_max,
		   int y_min,
		   int y_max);

ExtImage *
w2_keep_isolated (ExtImage *ext_image,
		  int boxsize);
ExtImage *
w2_keep_circle (ExtImage *ext_image,
		int radius_min, int radius_max);

ExtImage *
w2_keep_circle_simple (ExtImage *ext_image,
		int radius_min, int radius_max);

int
w2_mask_ext_image (ExtImage *ext_image,
		   unsigned char *mask_data,
		   int      mask_lx,
		   int      mask_ly,
		   int      x_0,
		   int      y_0);

enum
{
  W2_MOD,
  W2_ARG
};

int
w2_keep_by_value (ExtImage *ext_image,
		  real     value,
		  int      flag);
int
w2_keep_by_value_low (ExtImage *ext_image,
		  real     value,
		  int      flag);

int
w2_folow_contour (real *kapa,
		  real *kapap,
		  int   lx,
		  int   ly,
		  int   min_or_max);

int search_single_max (ExtImage *image);
int search_single_max2 (ExtImage *image, real, int);

void
vert_chain (ExtImage *do_ext_im,
	    ExtImage *up_ext_im,
	    int      lx,
	    int      ly,
	    int      box_size,
	    real     arg_simil,
	    int      is_first);

void
pt_to_pt_vert_chain (ExtImage *do_ext_im,
		     ExtImage *up_ext_im,
		     int      lx,
		     int      ly,
		     int      box_size,
		     real     arg_simil,
		     int      is_first);

#endif /* __WT2D_H__ */
