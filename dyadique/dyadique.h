/*
 * dyadique.h --
 *
 *  $Id: dyadique.h,v 1.2 1998/12/16 13:23:12 decoster Exp $
 */


#include <stdio.h>  /* Basic include files */
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <sys/types.h>
#include <stdlib.h>

#ifdef sparc
/* #include <strings.h>  */
#include <alloca.h>
#endif

#define NO 0
#define YES 1
#define ERROR (-1)
#define STRING_SIZE 100
/* types of border */
#define SYMEVN 0
#define SYMODD 1
#define ASYEVN 2
#define ASYODD 3


/**********************/
/* Some constants ... */
/**********************/

#define FILT_SIZE 128 /* max size of a filter */
#define NFACT 12    

typedef struct filter {
  int size;
  int shift;            /* shift half grid point to left: 1; to right: -1. */
  float symmetry;       /* symmetry: 1.0; antisymmetry: -1.0. */
  float values[FILT_SIZE];
  char name[STRING_SIZE];
  float factors[NFACT];
} *FILTER;








