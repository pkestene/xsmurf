/*
 * sm_interface_init.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: sm_interface_init.c,v 1.6 1999/03/31 09:16:57 decoster Exp $
 */

#include <tcl.h>
#include <tk.h>
/*#ifdef HP
#include "tkConfig.h"
#endif*/

#include "../interpreter/arguments.h"

int ViewDrawPlotCmd_(ClientData ,Tcl_Interp *,int,char **);

int ViewDicRangeCmd_   (ClientData,Tcl_Interp *,int ,char **);      
int ViewDicInfoCmd_    (ClientData,Tcl_Interp *,int ,char **);      
int ViewDicDeleteCmd_  (ClientData,Tcl_Interp *,int ,char **);      

int  set_colormap_TclCmd_ (ClientData,Tcl_Interp *,int,char**);
void init_colormap_ (Tk_Window);

int  ViewWidCmd_ (ClientData,Tcl_Interp*,int,char**);

int ViewConvImageCmd_   (ClientData ,Tcl_Interp *,int,char **);
int ViewConvExtImageCmd_(ClientData ,Tcl_Interp *,int,char **);
int ViewConvExtAddCmd_  (ClientData ,Tcl_Interp *,int,char **);
int ViewConvSignalCmd_  (ClientData ,Tcl_Interp *,int,char **);
int ViewConvColormapCmd_(ClientData ,Tcl_Interp *,int,char **);
int ViewConvPlotCmd_    (ClientData ,Tcl_Interp *,int,char **);

int init_sig_viewer_TclCmd_(ClientData, Tcl_Interp *, int, char **);
int draw_signal_TclCmd_(ClientData, Tcl_Interp *, int, char **);
int draw_line_TclCmd_(ClientData, Tcl_Interp *, int, char **);

int
sig_2_bitmap_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv);

int
get_rule_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv);

int ViewConvSampleGraphCmd_();

void ViewDicInit_();

int
get_size_cmd(ClientData clientData,
	     Tcl_Interp * interp,
	     int        argc,
	     char       ** argv)
{
  char * options[] = {"s",
		      NULL };

  char      *name;
  Tk_Window tk_win;

  if (arg_init(interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get(0, &name) == TCL_ERROR)
    return TCL_ERROR;

  tk_win = Tk_NameToWindow(interp, name, (Tk_Window) clientData);

  if (tk_win)
    sprintf (interp -> result, "%d %d",
	     Tk_Width (tk_win), Tk_Height (tk_win));

  return TCL_OK;
}

extern int noColorMapFlag;

int
sm_interface_init (Tcl_Interp *interp, Tk_Window tk_win) {

  ViewDicInit_();
  if (!noColorMapFlag) {
    init_colormap_(tk_win);
  }
  
  Tcl_CreateCommand(interp, "vinfo", (Tcl_CmdProc*) ViewDicInfoCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "vdelete", (Tcl_CmdProc*) ViewDicDeleteCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);
 
  Tcl_CreateCommand(interp, "viewer", (Tcl_CmdProc*) ViewWidCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);


  Tcl_CreateCommand(interp, "color", (Tcl_CmdProc*) set_colormap_TclCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);


  Tcl_CreateCommand(interp, "iconvert",(Tcl_CmdProc*) ViewConvImageCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "econvert",(Tcl_CmdProc*) ViewConvExtImageCmd_,(ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "sconvert",(Tcl_CmdProc*) ViewConvSignalCmd_,(ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);
/*  Tcl_CreateCommand(interp, "colormap",(Tcl_CmdProc*) ViewConvColormapCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);*/
  Tcl_CreateCommand(interp, "vadd",(Tcl_CmdProc*) ViewConvExtAddCmd_,(ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "plot",(Tcl_CmdProc*) ViewDrawPlotCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "iplot",(Tcl_CmdProc*) ViewConvPlotCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "init_viewer",
		    (Tcl_CmdProc*) init_sig_viewer_TclCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "draw_signal",
		    (Tcl_CmdProc*) draw_signal_TclCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "draw_line",
		    (Tcl_CmdProc*) draw_line_TclCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);
  Tcl_CreateCommand(interp, "sig2bmp",
		    (Tcl_CmdProc*) sig_2_bitmap_TclCmd_, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "w_size",(Tcl_CmdProc*) get_size_cmd, (ClientData) tk_win,
		    (Tcl_CmdDeleteProc *) NULL);

  return TCL_OK;
}
