/*
 * smPkgInt.h --
 *
 *   Internal header for all the xsmurf packages.
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: smPkgInt.h,v 1.4 1999/03/31 15:02:17 decoster Exp $
 */

#ifndef _SMPKGINT_H_
#define _SMPKGINT_H_

#include <tcl.h>
#include "arguments.h"
#include "hash_tables.h"
#include <sys/times.h>
#include <time.h>

/*
 * The following structure associates a command with a C procedure.
 */

typedef struct {
  char        *name; /* Name of command. */
  Tcl_CmdProc *proc; /* Procedure that executes command. */
} cmdInfo;


/*
 * Timer facility. Inspired by fftw.
 */

#define smGetTime() clock()
#define smTimeDiff(t1,t2) ((t1) - (t2))
#define smTime2Sec(t) (((double) (t)) / CLOCKS_PER_SEC)

#define smSetBeginTime() (smTimeBegin = smTime2Sec(smGetTime()))
#define smSetEndTime() (smTimeEnd = smTime2Sec(smGetTime()))
#define smGetEllapseTime() (smTimeDiff(smTimeEnd, smTimeBegin))

/*
 * ***VERY*** conservative constant: this says that a
 * measurement must run for 200ms in order to be valid.
 * You had better check the manual of your machine
 * to discover if it can do better than this
 */
#define SM_TIME_MIN (2.0e-1)	/* for default clock() timer */

/* take SM_TIME_REPEAT measurements... */
#define SM_TIME_REPEAT 4

/* but do not run for more than SM_TIME_LIMIT seconds while measuring something */
#ifndef SM_TIME_LIMIT
#define SM_TIME_LIMIT 2.0
#endif

extern float smTimeBegin;
extern float smTimeEnd;

#endif /* _SMPKGINT_H_ */
