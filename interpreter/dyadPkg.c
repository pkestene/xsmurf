/*
 * dyadPkg.c --
 *
 *   Implements the dyadique package for xsmurf.
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster and Stephane Roux.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: dyadPkg.c,v 1.3 1999/01/08 17:59:14 decoster Exp $
 */

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/times.h>
#include <tcl.h>
#include "../image/image.h"
#include "arguments.h"
#include "hash_tables.h"
#include "smPkgInt.h"
#include <wt2d.h>

/* 
 * The relative path of the following line is to avoid confusion with the
 * system header 'signal.h'.
 */

#include "../signal/signal.h"
#include <dyadique.h>

static int _ddecomp_Cmd_	(ClientData, Tcl_Interp*, int, char**);
static int _drecons_Cmd_	(ClientData, Tcl_Interp*, int, char**);
static int _dyad_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _precons_Cmd_	(ClientData, Tcl_Interp*, int, char**);

extern int     dyadic_decomposition (Image *, char *, int, int);
extern Image * get_image  (char *);
extern Image * dyadic_reconstruction(char *,int,int);

/*
 * Command list.
 */

static cmdInfo dyadCmdInfoArray[] = {
  {"iddecomp", (Tcl_CmdProc *)_ddecomp_Cmd_},
  {"idrecons", (Tcl_CmdProc *)_drecons_Cmd_},
  {"dyad",     (Tcl_CmdProc *)_dyad_Cmd_},
  {"precons",  (Tcl_CmdProc *)_precons_Cmd_},

  {NULL,	(Tcl_CmdProc *) NULL}
};

/*
 * dyad_pkgInit --
 *
 *  Init function of the dyad package.
 *
 * Arguments :
 *   Tcl_Interp - The interpreter where the package must be init.
 *
 * Return Value :
 *   TCL_OK.
 */

int
dyad_pkgInit (Tcl_Interp * interp)
{
  register cmdInfo *cmdInfoPtr;

  Tcl_PkgProvide (interp, "dyad", "0.0");

  /*
   * Create the built-in commands for the package.
   */
  for (cmdInfoPtr = dyadCmdInfoArray; cmdInfoPtr->name != NULL; cmdInfoPtr++) {
    Tcl_CreateCommand(interp,
		      cmdInfoPtr->name,
		      cmdInfoPtr->proc,
		      (ClientData) 0, (void (*)()) NULL);
  }

  return TCL_OK;
}

/*
 */
static int
_dyad_Cmd_ (ClientData clientData,
	  Tcl_Interp *interp,
	  int        argc,
	  char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "",
    "cmdlist", "",
    NULL
  };

  char * helpMsg = {
    (
     " List the C-defined commands of the package.\n"
     "\n"
     "Parameters :\n"
     "  none."
     )
  };

  /* Command's parameters */

  /* Options's presence */
  int isCmdlist;

  /* Options's parameters */

  /* Other variables */
  register cmdInfo *cmdInfoPtr;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get (0) == TCL_ERROR)
    return TCL_ERROR;

  isCmdlist = arg_present (1);

  /* Parameters validity and initialisation */

  /* Treatement */
  if (isCmdlist) {
    Tcl_AppendResult (interp, "{", (char *) NULL);

    cmdInfoPtr = dyadCmdInfoArray;
    Tcl_AppendResult (interp, cmdInfoPtr->name, (char *) NULL);
    for (cmdInfoPtr++;
	 cmdInfoPtr->name != NULL;
	 cmdInfoPtr++) {
      Tcl_AppendResult (interp, " ", cmdInfoPtr->name, (char *) NULL);
    }
    Tcl_AppendResult (interp, "}", (char *) NULL);
  } else {
    sprintf (interp->result, "The dyadique package blablabla.");
  }

  return TCL_OK;
}

static int
_ddecomp_Cmd_ (ClientData clientData,
	   Tcl_Interp *interp,
	   int        argc,
	   char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "Isd",
    NULL
  };

  char * helpMsg = {
    (
     "  dyadic wavelet decomposition.\n"
     "  The filters used for  this  decomposition  are  the  default filters."
     "  (nameapp_ is the approximation at octave considered,\n"
     "  namehor_ is the wavelet transform in the x direction,\n"
     "  namever_ is the wavelet transform in the y direction,\n"
     "  namemod_ is the modulus of the wavelet transform,\n"
     "  namearg_ is the argument of the wavelet transform,\n"
     "\n"
     "Arguments :\n"
     "  image   - image to analyze.\n"
     "  string  - name of the result\n" 
     "            (app_name, x_name,y_name, mod_name,arg_name.\n"
     "  integer - number of octave\n"
     "\n"
     )
  };

  /* Command's parameters */
  Image *image;
  char *name;
  int noct;

  if (arg_init(interp, argc, argv, options, helpMsg))
    return TCL_OK;

  if (arg_get(0, &image, &name, &noct) == TCL_ERROR)
    return TCL_ERROR;

  if (!dyadic_decomposition(image,name,0,noct)) {
    sprintf(interp->result, "memory allocation error");
    return TCL_ERROR;
  }

  /*   polar_repr(cur_wtrans2,1,noct); 
  */
  return TCL_OK;
}

static int
_drecons_Cmd_ (ClientData clientData,
	   Tcl_Interp *interp,
	   int        argc,
	   char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "ssd",
    NULL
  };

  char * helpMsg = {
    (
     "  dyadic wavelet reconstruction from a dyadic wavelet decomposition\n" 
     "  (made with iddecomp command).\n"
     "  The filters used for  this  reconstruction  are  the  default filters."
     "  (we use the image basenameapp_*, basenamehor_* and basenamever_*\n"
     "  where * = 00 ,01 .. 0noct-1).\n"
     "\n"
     "Arguments :\n"
     "  basename - basename of the wavelet decomposition.\n"
     "  string   - name of the reconstructed image\n" 
     "  integer - number of octave to use\n"
     "\n"
     )
  };

  /* Command's parameters */
  char *basename, *name;
  int noct;
  int end_level,i;
  char dst_name[100];
  Image * image_result;

  if (arg_init(interp, argc, argv, options, helpMsg))
    return TCL_OK;

  if (arg_get(0, &basename, &name, &noct) == TCL_ERROR)
    return TCL_ERROR;



  end_level = 0;
  for(i=0;i<noct;i++) {
    sprintf(dst_name,"%sapp_%.2d",basename, i);
    if(!get_image(dst_name)) {
      sprintf (interp->result,"Image %s doesn't exist.\n", dst_name);
      return TCL_ERROR;
    }
    sprintf(dst_name,"%shor_%.2d",basename, i);
    if(!get_image(dst_name)) {
      sprintf (interp->result,"Image %s doesn't exist.\n", dst_name);
      return TCL_ERROR;
    }
    sprintf(dst_name,"%sver_%.2d",basename, i);
    if(!get_image(dst_name)) {
      sprintf (interp->result,"Image %s doesn't exist.\n", dst_name);
      return TCL_ERROR;
    }


  }

  image_result = dyadic_reconstruction(basename, end_level, noct);

  if(!image_result)
    return TCL_ERROR;

  store_image(name,image_result);

  return TCL_OK;
}

extern Image * point_reconstruction(char *,char *,int,int,int,int,int);
extern ExtImage * ExtDicGet  (char *);

static int
_precons_Cmd_ (ClientData clientData,
	   Tcl_Interp *interp,
	   int        argc,
	   char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "ssdd",
    "-vc","",
    NULL
  };

  char * helpMsg = {
    (
     "  Reconstruct an  image  from  the  multiscale  edge representation.\n"
     "  (i.e. using ext-Image obtain with a dyadique wavelet transform.\n"
     "  The filters used for  this  reconstruction  are  the  default filters."
     "  (we use the Extimage basename* where * = 00 ,01 .. 0noct-1).\n"
     "\n"
     "Arguments :\n"
     "  basename - basename of the Extimages obtain from the wavelet decomposition.\n"
     "  string   - name of the reconstructed image\n" 
     "  integer  - number of octave to use\n"
     "  integer  - number of iteration (usually 20)\n"
     "\n"
     "Options :\n"
     "  -vc : Applies the function only on maxima that are on a vertical\n"
     "    chain.\n"
     )
  };

  /* Command's parameters */
  char *basename, *name;
  int noct,iteration;
  int lx,ly,i,is_vc;
  char dst_name[100];
  Image * image_result;
  ExtImage * ext_image;

  if (arg_init(interp, argc, argv, options, helpMsg))
    return TCL_OK;

  if (arg_get(0, &basename, &name, &noct,&iteration) == TCL_ERROR)
    return TCL_ERROR;

  is_vc = arg_present (1);

  for(i=0;i<noct;i++) {
    sprintf(dst_name,"%s%.2d",basename, i);
    if(!ExtDicGet(dst_name)) {
      sprintf (interp->result,"Extimage %s doesn't exist.\n", dst_name);
      return TCL_ERROR;
    }
  }
  ext_image = ExtDicGet(dst_name);

  lx = ext_image->lx;
  ly = ext_image->ly;

  image_result=point_reconstruction(basename, name,noct,iteration,lx,ly,is_vc);

  if(!image_result)
    return TCL_ERROR;

  store_image(name,image_result);

  return TCL_OK;
}
