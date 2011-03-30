/*
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: smMalloc.h,v 1.1 1998/03/26 19:22:44 decoster Exp $
 */

#ifndef SMMALLOC_
#define SMMALLOC_

#include <tcl.h>
#undef malloc
#define malloc(x) ckalloc(x)
#undef free
#define free(x) ckfree((char*)(x))
#undef realloc
#define realloc(x) ckrealloc(x)

#endif /* SMMALLOC_ */
