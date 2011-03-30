#ifndef __SURFACE_H__
#define __SURFACE_H__

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

#include "line.h"
#include "list.h"

struct _surface_ {
  List *line_lst;
  real greater_scale;
  real smaller_scale;
  int  nb_of_lines; 
  int  nb_of_extrema;
  real mass;
  real mean_arg;
};


typedef struct _surface_ Surface;

#define NO_SCALE -1

Surface * create_surface ();
void      destroy_surface (Surface *);
Surface * add_line_to_surface (Line *, Surface *);
Line    * smaller_scale_line (Surface *);

#endif /*__SURFACE_H__*/
