/*
 * all these are modified copies of files in wt2d directory
 */

#ifndef __WT3D_INT_H__
#define __WT3D_INT_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../wt2d/list.h"

#include "wt3d.h"          /* see wt2d.h     */
#include "extremum3d.h"    /* see extremum.h */
#include "surflet.h"       /* see line.h     */
#include "assert.h"

#ifdef TCL_MEM_DEBUG
#include "../main/smMalloc.h"
#endif

#define INFINITE 10000000

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define YES 0
#define NO  1

#define TRUE 1
#define FALSE 0

enum {CARAC, DIV, EDGE};

#define EX_RAISE(exception) goto exception

#endif /* __WT3D_INT_H__ */
