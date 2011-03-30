/*
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster and Stephane Roux.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *  or  decoster@info.enserb.u-bordeaux.fr
 *
 *  Common declaration for all the files from the signal library.
 */

#ifndef __SIGNAL_INT_H__
#define __SIGNAL_INT_H__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include "signal.h"

#define swap(a,b) tmp=(a);(a)=(b);(b)=tmp
#define sup(a,b) (a>b?a:b)
#define inf(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

/* ERROR a voir .... */
#ifndef ERROR
#define ERROR(argv) {printf(argv);return NULL;}
#endif

int next_power_of_2_ (int);
int is_power_of_2_ (int);

#ifdef TCL_MEM_DEBUG
#include "../main/smMalloc.h"
#endif

#endif /*  __SIGNAL_INT_H__ */
