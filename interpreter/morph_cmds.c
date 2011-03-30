/*
 * morph_cmds.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Pierre Kestener.
 *
 *  The author may be reached (Email) at the address
 *      kestener@crpp.u-bordeaux.fr
 *
 *  $Id: morph_cmds.c,v 1.?? 2000/03/13 15:20:06 kestener Exp $
 */

/* fichier cree pour interfacer les routines de morphologie mathematique
   ecrites par Richard Alan Peters II avec xsmurf */

#include <math.h>

#include "smPkgInt.h"

#include "../image/image.h"
#include "../signal/signal.h"
#include "../morph2d/morph_sub.h"


/* ******************************************************************* ** 
 * attention toto : essai pour se familiariser 
 * avec l'ajout de commandes a
 * xsmurf
 * ******************************************************************* */
int
morph_toto_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{
  /* Command line definition */
  char * options[] = {
    "II",
    NULL
  };

  char * help_msg =
  {
    (" toto of two images (real or complex), the result is stored\n"
     "in the first image.\n"
     "\n"
     "Parameters :\n"
     "  2 images - images to treat")
  };

  char resultBuffer[200];

  /* Command's parameters */
  Image *image1, *image2;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2) == TCL_ERROR)
    return TCL_ERROR;  
  
  sprintf(resultBuffer, "toto : %s\n", *argv);
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);

  if (!image1)
    return TCL_ERROR;

  return TCL_OK;
}

/* ********************************************************
   the routines for mathematical morphology
** ***************************************************** */
int
old_morph_main_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  /* Command line definition */
  char * options[] = {
    "Is",
    NULL
  };

  char * help_msg =
  {
    (" morph executes a mathematical morphology operation via R.A. Peters II\n"
     " s routines : read ~/dev/xsmurf/morph2d/docs/description.ps file.\n"
     "\n"
     "Parameters :\n"
     "  1 image  - image to treat, the data are rescaled between 0 and 255"
     "  1 string - name of the result (image of same size)"
     "  next arguments are treated like R.A. Peters II's GetArgs()"
     " routine do.\n"
     "\n"
     "usage: morph image_in dst_imageName -m e|d|o|c|r|t|b|l|m|n|p|q \n"
     "       [-i g|b] [-s g|b] [-o s|f] [-t nnn [mmm]] [-r med|nnn] \n"
     "       [-l kkk lll] [-z] [-n] [-v] -k SEFile | 3x3 | 5x5 | plus |\n"
     "       auto xxx yyy [zzz]"
     ""
     "see also commands defined in XSMURFDIR/tcl_library/morphology_proc.tcl .")
  };

  /* Command's parameters */
  /* struct rasterfile RasHd; */
  /* unsigned char *cmap; */     

   int MorphOp;   /* operation to perform */

   //char *Img;     /* image as list in row-major order */
   //int X,Y;       /* image horizontal, vertical dimensions */
   int ImgType;   /* gray-level or binary */
   int NZpad ;    /* flag.  F => zeropadding of input */
   int NoScale;   /* flag.  T => do not scale output of IntMorph */
   int Dsp;       /* flag.  T => display some info */

   char *SEname;  /* structuring element (SE) path name */
   int sX,sY;     /* SE horizontal, vertical dimensions */
   int sZ;        /* largest gray level in SE */
   int autoSE;    /* T => create a disk-shaped SE locally */
   int SEType;    /* binary or gray-level SE */
   int SorF;      /* Set operation or Function operation */

   int LThresh;   /* lower binary threshold value */
   int UThresh;   /* upper binary threshold value */
   int Rank;      /* rank for rank filter */

   char resultBuffer[300];
   char resultBuffer2[200];
   int i;
   short int *val_in;
   short int *val_out;

   Image *image_in, *image_out;
   char  *dst_imageName;
   int          lx;	/* Width of the image */
   int          ly;     /* Height of the image */
   int size;
   real min,max;
   real *data, *data2;

   float tmpvalue2;
   int argc2;
   /* char *argv2[]; */

   /* ***  BEGIN  *** */
   if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
   
   if (arg_get (0, &image_in, &dst_imageName) == TCL_ERROR)
    return TCL_ERROR;
   
   /* parametres de l'image d'entree */
   im_get_extrema (image_in,&min,&max);  /* initialise image_in->min
					    et image_in->max      */
   lx = image_in->lx;
   ly = image_in->ly;
   size = image_in->size;
   image_out = im_new (lx, ly, lx*ly, PHYSICAL);
   if (!image_out)
     return TCL_ERROR;
   
   /* allocation memoire pour les tableaux de characteres qui seront entres
      dans la routines morphsub*/

   val_in  = (short int *) malloc(lx * ly * sizeof(short int));
   val_out = (short int *) malloc(lx * ly * sizeof(short int));
   
   /* on "rescale" l'image de depart entre 0 et 255 */
   /* et on initialise val_in */

   data = image_in->data;
   data2 = image_out->data;
   for(i=0; i<size; i++)
      {
	val_in[i] = (short int) (data[i]);
      }  

   /* on initialise argc2 et argv2 */
   argc2 = argc - 2; /* image_in et dst_image ne sont pas pris en compte */
   /* il y en a peut etre pas besoin */

   if (argc < 5 + 2) /* display usage */
      {
	sprintf(resultBuffer,"usage: morph2d image_in dst_imageName -m e|d|o|c|r|t|b|l|m|n|p|q [-i g|b] [-s g|b] [-o s|f] \n       [-t nnn [mmm]] [-r med|nnn] [-l kkk lll] [-z] [-n] [-v] \n       -k SEFile | 3x3 | 5x5 | plus | auto xxx yyy [zzz]\n");
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
      }

   GetArgs(argc,argv,&SEname,&autoSE,&sX,&sY,&sZ,&MorphOp,&SorF,&SEType,
           &ImgType,&Rank,&LThresh,&UThresh,&NZpad,&NoScale,&Dsp);


   /* do the morph op */
   MorphSub2( MorphOp, val_in, val_out, lx, ly, ImgType, NZpad, LThresh, UThresh             ,SEname, autoSE, sX, sY, sZ, SEType, SorF, Rank, NoScale, Dsp);

   for(i=0; i<size; i++)
     {
	tmpvalue2 = (float) (val_out[i]);
	data2[i] = tmpvalue2;
     }

   sprintf(resultBuffer2, "morph2d s'est bien deroule\n");
      Tcl_AppendResult (interp, resultBuffer2, (char *) NULL);

   store_image (dst_imageName, image_out);
   return TCL_OK;

}

int
morph_main_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  /* Command line definition */
  char * options[] = {
    "Is",
    NULL
  };

  char * help_msg =
  {
    (" morph executes a mathematical morphology operation via R.A. Peters II\n"
     " s routines : read ~/dev/xsmurf/morph2d/docs/description.ps file.\n"
     "\n"
     "Parameters :\n"
     "  1 image  - image to treat, the data are rescaled between 0 and 255"
     "  1 string - name of the result (image of same size)"
     "  next arguments are treated like R.A. Peters II's GetArgs()"
     " routine do.\n"
     "\n"
     "usage: morph image_in dst_imageName -m e|d|o|c|r|t|b|l|m|n|p|q \n"
     "       [-i g|b] [-s g|b] [-o s|f] [-t nnn [mmm]] [-r med|nnn] \n"
     "       [-l kkk lll] [-z] [-n] [-v] -k SEFile | 3x3 | 5x5 | plus |\n"
     "       auto xxx yyy [zzz]"
     ""
     "see also commands defined in XSMURFDIR/tcl_library/morphology_proc.tcl .")
  };

  /* Command's parameters */
  /* struct rasterfile RasHd; */
  /* unsigned char *cmap; */     

   int MorphOp;   /* operation to perform */

   //char *Img;     /* image as list in row-major order */
   //int X,Y;       /* image horizontal, vertical dimensions */
   int ImgType;   /* gray-level or binary */
   int NZpad ;    /* flag.  F => zeropadding of input */
   int NoScale;   /* flag.  T => do not scale output of IntMorph */
   int Dsp;       /* flag.  T => display some info */

   char *SEname;  /* structuring element (SE) path name */
   int sX,sY;     /* SE horizontal, vertical dimensions */
   int sZ;        /* largest gray level in SE */
   int autoSE;    /* T => create a disk-shaped SE locally */
   int SEType;    /* binary or gray-level SE */
   int SorF;      /* Set operation or Function operation */

   int LThresh;   /* lower binary threshold value */
   int UThresh;   /* upper binary threshold value */
   int Rank;      /* rank for rank filter */

   char resultBuffer[300];
   char resultBuffer2[200];
   int i;
   unsigned char *val_in;
   unsigned char *val_out;
   //char tmp;
   Image *image_in, *image_out;
   char  *dst_imageName;
   int          lx;	/* Width of the image */
   int          ly;     /* Height of the image */
   int size;
   real min,max;
   real *data, *data2;
   real tmpvalue;
   float tmpvalue2;
   int argc2;
   /* char *argv2[]; */

   /* ***  BEGIN  *** */
   if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
   
   if (arg_get (0, &image_in, &dst_imageName) == TCL_ERROR)
    return TCL_ERROR;
   
   /* parametres de l'image d'entree */
   im_get_extrema (image_in,&min,&max);  /* initialise image_in->min
					    et image_in->max      */
   lx = image_in->lx;
   ly = image_in->ly;
   size = image_in->size;
   image_out = im_new (lx, ly, lx*ly, PHYSICAL);
   if (!image_out)
     return TCL_ERROR;
   
   /* allocation memoire pour les tableaux de characteres qui seront entres
      dans la routines morphsub*/

   val_in  = (unsigned char *) malloc(lx * ly * sizeof(char));
   val_out = (unsigned char *) malloc(lx * ly * sizeof(char));
   
   /* on "rescale" l'image de depart entre 0 et 255 */
   /* et on initialise val_in */

   data = image_in->data;
   data2 = image_out->data;
   for(i=0; i<size; i++)
      {
	if (min<max) tmpvalue = (data[i]-min)*255./(max-min);
	else tmpvalue = data[i];
	if (0>tmpvalue) tmpvalue = 0.0;
	if (255<tmpvalue) tmpvalue = 255.0;
	val_in[i] = (unsigned char) ((int)(tmpvalue));
      }  

   /* on initialise argc2 et argv2 */
   argc2 = argc - 2; /* image_in et dst_image ne sont pas pris en compte */
   /* il y en a peut etre pas besoin */

   if (argc < 5 + 2) /* display usage */
      {
	sprintf(resultBuffer,"usage: morph2d image_in dst_imageName -m e|d|o|c|r|t|b|l|m|n|p|q [-i g|b] [-s g|b] [-o s|f] \n       [-t nnn [mmm]] [-r med|nnn] [-l kkk lll] [-z] [-n] [-v] \n       -k SEFile | 3x3 | 5x5 | plus | auto xxx yyy [zzz]\n");
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
      }

   GetArgs(argc,argv,&SEname,&autoSE,&sX,&sY,&sZ,&MorphOp,&SorF,&SEType,
           &ImgType,&Rank,&LThresh,&UThresh,&NZpad,&NoScale,&Dsp);


   /* do the morph op */
   MorphSub( MorphOp, val_in, val_out, lx, ly, ImgType, NZpad, LThresh, UThresh             ,SEname, autoSE, sX, sY, sZ, SEType, SorF, Rank, NoScale, Dsp);

   for(i=0; i<size; i++)
     {
	tmpvalue2 = (float) (val_out[i]);
	data2[i] = tmpvalue2;
     }

   free(val_in);
   free(val_out);

   sprintf(resultBuffer2, "morph2d s'est bien deroule\n");
      Tcl_AppendResult (interp, resultBuffer2, (char *) NULL);

   store_image (dst_imageName, image_out);
   return TCL_OK;

}


