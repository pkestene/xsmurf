/*
 * image_int.h --
 *
 * Internal header of the image library.
 *
 *   Copyright (c) 1999 Nicolas Decoster
 *   Copyright (c) 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *
 *   Copyright (c) 1999-2007 Pierre Kestener.
 *   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
 *   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
 *   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
 *
 */

#ifndef __IMAGE_INT_H__
#define __IMAGE_INT_H__

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <time.h>
#include <sys/time.h> // for gettimeofday
#include <sys/types.h> // for getpid
#include <unistd.h>
#include "image.h"
#include "image3D.h"
#include <gfft.h>

#include "fftw3_inc.h"

#include <matheval.h>

#ifdef TCL_MEM_DEBUG
#include "../main/smMalloc.h"
#endif

/*
 * Usefull macros.
 */
#define swap(a,b) tmp=(a);(a)=(b);(b)=tmp
#define sup(a,b) (a>b?a:b)
#define inf(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

#define SEUIL 1e-6

int next_power_of_2__(int i);

extern void *myMalloc();
extern void myFree();

/*#define malloc myMalloc
#define free myFree*/

#endif /* __IMAGE_INT_H__ */
