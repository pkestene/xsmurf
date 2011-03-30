#ifndef __CONVERT_H__
#define __CONVERT_H__

#include "ViewerLib.h"
#include "Colormap.h"
#include "Image.h"
/*#include "SignalLib/SignalLib.h"*/
#include "../signal/signal.h"

#define LOG 0
#define LIN 1
#define DIS 2

int ViewConvImageCmd_   (ClientData ,Tcl_Interp *,int,char **);
int ViewConvExtImageCmd_(ClientData ,Tcl_Interp *,int,char **);
int ViewConvExtAddCmd_  (ClientData ,Tcl_Interp *,int,char **);
int ViewConvSignalCmd_  (ClientData ,Tcl_Interp *,int,char **);
int ViewConvColormapCmd_(ClientData ,Tcl_Interp *,int,char **);
int ViewConvPlotCmd_    (ClientData ,Tcl_Interp *,int,char **);

#endif
