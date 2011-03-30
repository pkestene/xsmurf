#ifndef __ARGUMENTS_H__
#define __ARGUMENTS_H__

#include <tcl.h>
#include <stdarg.h>
#include <string.h>

int arg_present (int optNumber);
int arg_init    (Tcl_Interp * interp_in,int argc,char ** argv,
		    char ** formats, char * helpfile);
int arg_get     (int option,...);

#endif
