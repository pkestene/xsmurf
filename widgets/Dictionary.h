#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include "ViewerLib.h"
#include "Image.h"


#define NULL_TYPE  0
#define IMAGE_TYPE 1
#define GRAPH_TYPE 2

/*----------------------------------------------------------------------------
  Procedures publiques du module
  --------------------------------------------------------------------------*/
void        ViewDicInit_       ();
void        ViewDicStore_      (char *, ViewImage *);
ViewImage * ViewDicGet_        (char * name);
void        ViewDicRemove_     (char * name);
int         ViewDicRangeCmd_   (ClientData clientData,Tcl_Interp *interp,int argc, char *argv[]);      
int         ViewDicInfoCmd_    (ClientData,Tcl_Interp *,int ,char **);      
int         ViewDicDeleteCmd_  (ClientData,Tcl_Interp *,int ,char **);      
int         ViewDicTclGet      (Tcl_Interp *, char *, ViewImage **);

#endif
