#ifndef __SM_VIEWERLIB_H__
#define __SM_VIEWERLIB_H__

#include "tcl.h"
#include "tk.h"
#ifdef HP
#include "tkConfig.h"
#endif
/*#include "General/General.h"
#include "General/Def.h"*/
#include "Image.h"

int ViewerLibInit (Tcl_Interp *, Tk_Window);
void ViewWidReconfigureall_(char * viewImageName);

#endif
