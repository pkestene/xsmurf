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

#ifndef __STATS_INT_H__
#define __STATS_INT_H__

#include "stdio.h"
#include "math.h"
#include "assert.h"
#include "stats.h"

#ifdef TCL_MEM_DEBUG
#include "../main/smMalloc.h"
#endif

#define sup(a,b) (a>b?a:b)
#define inf(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

#ifndef ERROR
#define ERROR(argv) {printf(argv);return NULL;}
#endif

#endif /*  __STATS_INT_H__ */
