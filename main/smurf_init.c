/*
 * smurf_init.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: smurf_init.c,v 1.4 1999/03/31 09:17:00 decoster Exp $
 */

#include <tcl.h>
#ifdef X11
#include <tk.h>
#endif

extern int sm_interpreter_init (Tcl_Interp * interp);
extern int sm_interface_init (Tcl_Interp *interp, Tk_Window tk_win);

extern int noDisplayFlag;

/*----------------------------------------------------------------------
  
  Tcl_AppInit --
  
  Initialisation du programme.
  Cette procedure lance l'initialisation de toutes les librairies qui
  constituent le programme avant de laisser la main a l'interpreteur tcl.
  ----------------------------------------------------------------------*/
int Tcl_AppInit(Tcl_Interp *interp)
     /*Tcl_Interp *interp;*/		/* Tcl Interpreter for application. */
{
#ifdef X11
  Tk_Window main;
#endif

  if (Tcl_Init(interp) == TCL_ERROR) 
    return TCL_ERROR;
  
#ifdef X11
  if (!noDisplayFlag) {
    if (Tk_Init(interp) == TCL_ERROR) 
      return TCL_ERROR;

    main = Tk_MainWindow(interp);
  }
#endif

  /* we execute sm_interpreter_init to provide
   *  user specific commands such:
   *  help
   *  iload, isave, ...
   *  and so on.
   */
  if (sm_interpreter_init (interp) == TCL_ERROR) 
    return TCL_ERROR;

  if (sm_interface_init (interp, main) == TCL_ERROR) 
    return TCL_ERROR;

  Tcl_SetVar (interp, "tcl_rcFileName", "~/.smurfrc", TCL_GLOBAL_ONLY);

  return TCL_OK;
}
