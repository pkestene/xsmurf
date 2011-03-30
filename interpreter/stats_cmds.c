/* last modified by Pierre Kestener (2000/05/31). */

#include <tcl.h>
#include <math.h>
#include <sys/times.h>
#include "../signal/signal.h"
#include "../stats/stats.h"
#include "../image/image.h"
#include "arguments.h"
#include "hash_tables.h"
#include <stdlib.h>

/*#include <defunc.h>*/
#include <matheval.h>

/*void store_signal_in_dictionary (char   *name, Signal *signal_ptr);
void store_image (char  *name, Image *image_ptr);
*/
/*------------------------------------------------------------------------
  StatSigHistoCmd_
  compute the histogram of a signal
  ------------------------------------------------------------------------*/

int
StatSigHistoCmd_(ClientData clientData,
		 Tcl_Interp * interp,
		 int        argc,
		 char       ** argv)
{
  char * options[] = { "Ssd" ,           /* Signal, resultat, nombre de boite*/
		       "-x","ff",        /* borne xmin et xmax de l'histogramme */
		       "-w","S",         /* new signal of the same size of the first*/
		                         /* signal with the weight*/
		       "-I","",          /* for Complex Signal : histogram of the imaginary part*/ 
		       NULL };
  
  
  char * help_msg =
  {("Compute the histogram of the value of the signal and put the result in name (signal, name,nbox).\n"
    "  -x give the lower and upper bound of the histogram .\n"
    "  -w signal which give the weight for the histogram.\n"
    "  -I compute the histogram of the imaginary part for signal of type FOUR_NR or CPLX (default real part).")};
  
  Signal *signal1, *signal2 = NULL, *result;
  char   *name;
  real   xmin,xmax;
  int    nbox, flag_imaginary;
  
  xmin=1.0;
  xmax=-1.0;

 
  flag_imaginary = 0;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal1, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;
  
  if (arg_get(1, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;
  
  if (arg_get(2, &signal2) == TCL_ERROR)
    return TCL_ERROR;
  
  if ((arg_present(2) && (signal1->size != signal2 ->size))) {
    Tcl_AppendResult (interp,
		      "The weight signal should be of the same size as the input signal.",
		      (char *) NULL);
    return TCL_ERROR;
  }
  
  if (arg_present(3)) {
    flag_imaginary = 1;
    if ((signal1->type != CPLX) && (signal1->type != FOUR_NR))
      
      Tcl_AppendResult (interp,
			"Option -I only for CPLX signal.",
			(char *) NULL);
    return TCL_ERROR;
  }

  result = stat_signal_histogram(signal1, nbox ,xmin, xmax, signal2, flag_imaginary);
  
  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(name, result);
  
  return TCL_OK;
  
}

/*-------------------------------------------------------------------------
  StatImHistoCmd_
  compute the histogram of an image
  -------------------------------------------------------------------------*/

/***********************************
 * commande name in xsmurf : ihisto
 ***********************************/
/* modified by P. Kestener :
 - add option incrx and incry
 - add option masklines (16/05/2006)
 - add option mask (23/05/2006)
*/

int
StatImHistoCmd_(ClientData clientData,
		Tcl_Interp * interp,
		int        argc,
		char       ** argv)
{
  char * options[] = { "Isd" ,           /* Image, resultat, nombre de boite*/
		       "-x","ff",         /* borne xmin et xmax de l'histogramme */
		       "-w","I",         /* new Image of the same size of the first Image with the weight*/
		       "-I","",          /* for FOURIER Image : histogram of the imaginary part*/ 
		       "-grad", "I",   /* Histogram of the gradient as (mod, arg).*/ 
		       "-incrx", "d",
		       "-incry", "d",
		       "-masklines", "E",
		       "-mask", "I",
		       NULL };
  
  
  char * help_msg =
  {("Compute the histogram of the value of an Image and put the result in name (image, name,nbox).\n"
    "  -x give the lower and upper bound of the histogram .\n"
    "  -w Image which give the weight for the histogram.\n"
    "  -I compute the histogram of the imaginary part for signal of type FOURIER (default real part)\n."
    "  -grad the result is an image of the 2d histogram of the gradient as\n"
    "a vector\n"
    "  -incrx : compute increment histogram along x-axis\n"
    "  -incry : compute increment histogram along y-axis\n"
    "  -masklines [E] : compute histogram of image values along some maxima \n"
    "                   lines (according to an ext-image given as an argument)\n"
    "  -mask [I] : can be used with incrx and incry options.\n"
    )};
  
  Image  *image1, *image2 = NULL, *imagearg=NULL; 
  Image  *result_image;
  ExtImage *masklines;
  Image *mask = NULL;
  Signal *result; 
  char   *name;
  real   xmin,xmax;
  int    nbox, flag_imaginary;
  int    is_grad, is_incr_x, is_incr_y,is_masklines, is_mask;

  int    dx, dy;

  xmin=1.0;
  xmax=-1.0;

  flag_imaginary = 0;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &image1, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &image2) == TCL_ERROR)
    return TCL_ERROR;
  
  if (arg_present(2) && ((image1->size != image2 ->size) || (image1->lx != image2->lx) || (image1->ly != image2->ly))) {
    Tcl_AppendResult (interp,
		      "The weight image should be of the same size and same dimension as the input image.",
		      (char *) NULL);
    return TCL_ERROR;
  }
  
  if (arg_present(3)) {
    flag_imaginary = 1;
    if (image1->type != FOURIER)
      
    Tcl_AppendResult (interp,
		      "Option -I only for FOURIER image.",
		      (char *) NULL);
    return TCL_ERROR;
  }

  is_grad = arg_present(4);
  if (arg_get(4, &imagearg) == TCL_ERROR)
    return TCL_ERROR;
  
  if (arg_present(4) && ((image1->size != imagearg ->size) || (image1->lx != imagearg->lx) || (image1->ly != imagearg->ly))) {
    Tcl_AppendResult (interp,
		      "The arg image should be of the same size and same dimension as the input (or mod) image.",
		      (char *) NULL);
    return TCL_ERROR;
  }
  
  is_incr_x = arg_present(5);     
  if (arg_get(5, &dx) == TCL_ERROR)
    return TCL_ERROR;
  
  is_incr_y = arg_present(6);     
  if (arg_get(6, &dy) == TCL_ERROR)
    return TCL_ERROR;
     
  is_masklines = arg_present(7);
  if (arg_get(7, &masklines) == TCL_ERROR)
    return TCL_ERROR;
  
  is_mask = arg_present(8);
  if (arg_get(8, &mask) == TCL_ERROR)
    return TCL_ERROR;
     
  if (is_grad) {
    
    result_image = stat_imimage_gradient_histogram (image1,imagearg, nbox ,xmin, xmax);
    
    if (!result_image)
      return TCL_ERROR;
    store_image (name, result_image);
    
  } else if (is_incr_x) {
    
    result =  stat_imimage_increment_histogram (image1, nbox ,xmin, xmax, dx, 0, mask);
    
    if (!result)
      return TCL_ERROR;
    
    store_signal_in_dictionary(name, result);

  } else if (is_incr_y) {

    result =  stat_imimage_increment_histogram (image1, nbox ,xmin, xmax, dy, 1, mask);
    
    if (!result)
      return TCL_ERROR;
    
    store_signal_in_dictionary(name, result);

  } else if (is_masklines) {

    result =  stat_imimage_masklines_histogram (image1, masklines, nbox ,xmin, xmax);
    
    if (!result)
      return TCL_ERROR;
    
    store_signal_in_dictionary(name, result);

  } else {
  
    result =  stat_imimage_histogram (image1, nbox ,xmin, xmax, image2, flag_imaginary, mask);
    
    if (!result)
      return TCL_ERROR;  
    store_signal_in_dictionary(name, result);
  
  }
  
  return TCL_OK;

}

/*************************************
 * commande name in xsmurf : ihisto2d
 *************************************/

int
StatImHisto2DCmd_(ClientData clientData,
		  Tcl_Interp * interp,
		  int        argc,
		  char       ** argv)
{
  char * options[] = { "IIs" ,           /* Image, resultat, nombre de boite*/
		       "-x","ff",         /* borne xmin et xmax de l'histogramme */
		       "-y","ff",         /* borne ymin et ymax de l'histogramme */
		       "-nx", "d",
		       "-ny", "d",
		       NULL };
  
  
  char * help_msg =
  {("Compute the 2D histogram of the values of 2 Images and put the result in image name (image, image, name).\n"
    "  -x gives the lower and upper bound of the histogram along x-axis (defaults are taken from image extrema) .\n"
    "  -y gives the lower and upper bound of the histogram along y-axis (defaults are taken from image extrema).\n"
    "  -nx gives the number of boxes along x-axis (default is 128)\n"
    "  -nx gives the number of boxes along y-axis (default is 128)\n"
    )};
  
  Image  *image1, *image2; 
  Image  *result_image;
  Signal *result; 
  char   *name;
  real    xmin, xmax, ymin, ymax;
  int     nboxx, nboxy;

  /* set defaults */
  nboxx = nboxy = 128;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &image1, &image2, &name) == TCL_ERROR)
    return TCL_ERROR;

  if ( (image1->lx != image2->lx) || (image1->ly != image2->ly) ) {
    Tcl_AppendResult (interp,
		      "Image1 and Image2 must have the same dimensions.",
		      (char *) NULL);
    return TCL_ERROR;
  }

  /* compute xmin, xmax, and ymin, ymax */
  im_get_extrema (image1,&xmin,&xmax);
  im_get_extrema (image2,&ymin,&ymax);
  

  /* parse options */
  if (arg_get(1, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &ymin, &ymax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &nboxx) == TCL_ERROR)
    return TCL_ERROR;
  
  if (arg_get(4, &nboxy) == TCL_ERROR)
    return TCL_ERROR;
  
  
  result_image = stat_imimage_histogram2D (image1, xmin, xmax, nboxx, image2, ymin, ymax, nboxy);
  
  if (!result_image)
    return TCL_ERROR;
  store_image (name, result_image);
  
  
  return TCL_OK;

}

/**************************************
 * commande name in xsmurf : i3Dhisto
 **************************************/
/* created January, 30 th , 2003 */
/* modified September 16th, 2005 : function stat_imimage3D_histogram2D created
 with option twod */

int
StatIm3DHistoCmd_(ClientData clientData,
		  Tcl_Interp * interp,
		  int        argc,
		  char       ** argv)
{
  char * options[] = { "Jsd" ,        /* Image3D, resultat, nombre de boites*/
		       "-x","ff",     /* borne xmin et xmax de l'histogramme */
		       "-y","ff",     /* borne ymin et ymax de l'histogramme */
		       "-twod", "Jd", /* compute a 2d histo */
		       "-mask", "T",
		       NULL };
  
  
  char * help_msg =
  {("Compute the histogram of the value of an 3D Image and put the \n"
    "result in name (image3D, name,nbox).\n"
    "  -x    : give the lower and upper bound of the histogram .\n"
    "  -y    : give the lower and upper bound of the histogram (use this "
    "          option with -twod).\n"
    "  -twod : compute a 2d histo.\n"
    "  -mask : use an extimage3Dsmall to constraint histogram.\n"
    "\n")};
  
  Image3D *image1, *image2=NULL;
  Signal *result; 
  Image  *result_image;
  ExtImage3Dsmall *extima_mask;
  char   *name;
  real   xmin, xmax, ymin, ymax;
  int    nboxx, nboxy;

  int isTwod;
  int isMask;

  /* compute default values for xmin, xmax, and ymin, ymax */
  /*im3D_get_extrema (image1,&xmin,&xmax);
    im3D_get_extrema (image2,&ymin,&ymax);*/
  

  /* parse arguments */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  /* parse options */
  if (arg_get(0, &image1, &name, &nboxx) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &ymin, &ymax) == TCL_ERROR)
    return TCL_ERROR;

  isTwod = arg_present(3);
  if (arg_get(3, &image2, &nboxy) == TCL_ERROR)
    return TCL_ERROR;
  
  if (arg_present(3) && ((image1->size != image2 ->size) || (image1->lx != image2->lx) || (image1->ly != image2->ly) || (image1->lz != image2->lz))) {
    Tcl_AppendResult (interp, "Second image should be of the same size and same dimension as the input image.", (char *) NULL);
    return TCL_ERROR;
  }
  
  isMask = arg_present(4);
  if (arg_get(4, &extima_mask) == TCL_ERROR)
    return TCL_ERROR;
  if (arg_present(3) && arg_present(4) && (image1->size != (extima_mask ->lx*extima_mask ->ly*extima_mask ->lz)) ) {
    Tcl_AppendResult (interp, "Extimage_mask should be of the same size and same dimension as the input image.", (char *) NULL);
    return TCL_ERROR;
  }
  

  if (isTwod) {
    if (isMask) 
      /* the following function must be modified */
      result_image = stat_imimage3D_gradient_histogram_mask(image1, image2, extima_mask, nboxx ,xmin, xmax);
    else 
      result_image = stat_imimage3D_histogram2D(image1, image2, nboxx, nboxy ,xmin, xmax, ymin, ymax);
    
    if (!result_image)
      return TCL_ERROR;
    store_image (name, result_image); 
  } else {     
    result =  stat_imimage3D_histogram (image1, nboxx ,xmin, xmax);
    if (!result)
      return TCL_ERROR;
    store_signal_in_dictionary(name, result);
 }
    
  return TCL_OK;

}

/*---------------------------------------------------------------------------
  StatExtHistoCmd_
  compute the histogram of an ExtImage
  ---------------------------------------------------------------------------*/

/***********************************
 * commande name in xsmurf : ehisto
 ***********************************/
/* modified by P. Kestener :
 - add option mx and my
 - add option mask (23/06/2006)
*/

int
StatExtHistoCmd_(ClientData clientData,
		Tcl_Interp * interp,
		int        argc,
		char       ** argv)
{
  char * options[] = { "Esd" ,        /* Image, resultat, nombre de boite*/
		       "-x", "ff",    /* borne xmin et xmax de l'histogramme */
		       "-arg", "",    /* for FOURIER Image : histogram of the imaginary part*/ 
		       "-grad", "",   /* Histogram of the gradient as (mod, arg).*/ 
		       "-vc", "",
		       "-mx", "",     /* histogram of mod*cos(arg) */
		       "-my", "",     /* histogram of mod*sin(arg) */
		       "-mask", "I", /* histogram of mod constrained with a mask*/
		       NULL };
  
  
  char * help_msg =
  {("Compute the histogram of the value of an ExtImage and put the result\n"
    "in name (Extimage, name,nbox).\n"
    "  -x give the lower and upper bound of the histogram .\n"
    "  -arg compute the histogram of the argument of the Extrema (default\n"
    "module of the extrema).\n"
    "  -grad the result is an image of the 2d histogram of the gradient as\n"
    "a vector.\n"
    "  -vc Compute the histogram only on maxima that are on a vertical chain.\n"
    "  -mx Compute the histogram of modulus*cos(arg)\n"
    "  -my Compute the histogram of modulus*sin(arg)\n"
    "  -mask [I] Compute the histogram of modulus using points such that\n"
    "             mask is >0\n")
  };
  
  ExtImage *image1;
  Image    *result_image;
  Signal *result; 
  char   *name;
  real   xmin, xmax;
  int    nbox, flag_arg = MOD_STAT;
  int    max_flag = LINE_MAX_STAT;
  int is_grad;
  int is_vc;
  int is_mx, is_my;
  int is_mask;
  Image *mask=NULL;

  xmin=1.0;
  xmax=-1.0;

  flag_arg = 0;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &image1, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_present(2)) 
    flag_arg = ARG_STAT;

  is_grad = arg_present (3);
  is_vc = arg_present (4);
  is_mx = arg_present(5);
  is_my = arg_present(6);
  is_mask = arg_present(7);
  if (arg_get(7, &mask) == TCL_ERROR)
    return TCL_ERROR;
     
  if (is_vc)
    max_flag = VERT_CHAINED_MAX_STAT;
     
  if (is_mx)
    flag_arg = MX_STAT;
  if (is_my)
    flag_arg = MY_STAT;

  if (is_grad)
    {
      result_image = stat_extimage_gradient_histogram (image1, nbox ,&xmin, &xmax, max_flag);

      if (!result_image)
	return TCL_ERROR;

      store_image (name, result_image);
      sprintf (interp -> result, "%f %f", xmax, xmin);
    }
  else
    {
      result =  stat_extimage_histogram (image1, nbox ,xmin, xmax, flag_arg, max_flag, mask);
      
      if (!result)
	return TCL_ERROR;

      store_signal_in_dictionary(name, result);
    }
  
  return TCL_OK;
}


/*---------------------------------------------------------------------------
  StatExt3DsmallHistoCmd_
  compute the histogram of an ExtImage 3Dsmall
  ---------------------------------------------------------------------------*/

/*
 * commande name in xsmurf : ehisto3Dsmall
 */

int
StatExt3DsmallHistoCmd_(ClientData clientData,
			Tcl_Interp * interp,
			int        argc,
			char       ** argv)
{
  char * options[] = { "Tsd" ,        /* ExtImage3Dsmall, resultat,
					 nombre de boite*/
		       "-x", "ff",    /* borne xmin et xmax de l'histogramme */
		       NULL };
  
  
  char * help_msg =
  {("Compute the histogram of the value of a 3Dsmall ExtImage and put the\n"
    "result in name (Extimage3Dsmall, name,nbox).\n"
    "  -x give the lower and upper bound of the histogram .\n")};
  
  ExtImage3Dsmall *extima;
  Signal *result; 
  char   *name;
  real   xmin, xmax;
  int    nbox;

  xmin=1.0;
  xmax=-1.0;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &extima, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;

  result =  stat_extimage3Dsmall_histogram (extima, nbox ,xmin, xmax);
  
  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(name, result);
  
  return TCL_OK;
}


/*---------------------------------------------------------------------------
  StatExt3DHistoCmd_
  compute the histogram of a 3D ExtImage specified by a filename
  ---------------------------------------------------------------------------*/

/*
 * commande name in xsmurf : ehisto3D
 */

int
StatExt3DHistoCmd_(ClientData clientData,
		   Tcl_Interp * interp,
		   int        argc,
		   char       ** argv)
{
  char * options[] = { "ssd" ,    /* ExtIma3D filename, resultat, nb de boite*/
		       "-x", "ff",  /* borne xmin et xmax de l'histogramme */
		       "-arg", "",    /* for FOURIER Image : histogram of 
				       * the imaginary part */ 
		       "-grad", "",   /* Histogram of the gradient as
				       * (mod, arg).*/ 
		       "-vc", "",
		       "-mx", "",     /* histogram of mod*cos(arg) */
		       "-my", "",     /* histogram of mod*sin(arg) */
		       NULL };
  
  
  char * help_msg =
  {("Compute the histogram of the value of a 3D ExtImage specified by its\n"
    " filename and put the result\n"
    "in name (string, name,nbox).\n"
    "  -x give the lower and upper bound of the histogram .\n"
    "  -arg compute the histogram of the argument of the Extrema (default\n"
    "module of the extrema).\n"
    "  -grad the result is an image of the 2d histogram of the gradient as\n"
    "a vector.\n"
    "  -vc Compute the histogram only on maxima that are on a vertical chain.\n"
    "  -mx Compute the histogram of modulus*cos(arg)\n"
    "  -my Compute the histogram of modulus*sin(arg)\n")
  };
  
  char     * extImageFilename;
  Image    *result_image;
  Signal *result; 
  char   *name;
  real   xmin, xmax;
  int    nbox, flag_arg = MOD_STAT;
  int    max_flag = LINE_MAX_STAT;
  int is_grad;
  int is_vc;
  int is_mx, is_my;

  xmin=1.0;
  xmax=-1.0;

  flag_arg = 0;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &extImageFilename, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_present(2)) 
    flag_arg = ARG_STAT;

  is_grad = arg_present (3);
  is_vc = arg_present (4);
  is_mx = arg_present(5);
  is_my = arg_present(6);

  if (is_vc)
    max_flag = VERT_CHAINED_MAX_STAT;
     
  if (is_mx)
    flag_arg = MX_STAT;
  if (is_my)
    flag_arg = MY_STAT;

  if (is_grad)
    {
      // a faire... en particulier le traitement de l'argument,
      // ou des arguments plutot (en 3D).
      //result_image = stat_extimage_gradient_histogram (image1, nbox ,&xmin, &xmax, max_flag);

      //if (!result_image)
      //return TCL_ERROR;

      //store_image (name, result_image);
      //sprintf (interp -> result, "%f %f", xmax, xmin);
    }
  else
    {
      result =  stat_extimage3D_histogram (interp, extImageFilename, nbox, flag_arg, max_flag);
      
      if (!result)
	return TCL_ERROR;

      store_signal_in_dictionary(name, result);
    }
  
  return TCL_OK;
}


/*
 */
int
StatExtHisto2Cmd_(ClientData clientData,
		  Tcl_Interp * interp,
		  int        argc,
		  char       ** argv)
{
  char * options[] = { "Esd" ,           /* Image, resultat, nombre de boite*/
		       "-x","ff",         /* borne xmin et xmax de l'histogramme */
		       "-arg","",          /* for FOURIER Image : histogram of the imaginary part*/ 
		       NULL };
  
  
  char * help_msg =
  {("Compute the histogram of the value of an ExtImage and put the result in name (Extimage, name,nbox).\n"
    "  -x give the lower and upper bound of the histogram .\n"
    "  -A compute the histogram of the argument of the Extrema (default module of the extrema).")};
  
  ExtImage *image1; 
  Signal *result; 
  char   *name;
  real   xmin,xmax;
  int    nbox, flag_arg;
  
  xmin=1.0;
  xmax=-1.0;

  flag_arg = 0;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &image1, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_present(2)) 
    flag_arg = 1;
     

  result =  stat_extimage_histogram2 (image1, nbox ,xmin, xmax, flag_arg);

  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(name, result);
  
  return TCL_OK;

}


/*
 */
int
StatLineHistoCmd_(ClientData clientData,
		   Tcl_Interp * interp,
		   int        argc,
		   char       ** argv)
{
  char * options[] = { "Elsd" ,           /* Image, resultat, nombre de boite*/
		       "-x","ff",         /* borne xmin et xmax de l'histogramme */
		       "-arg","",          /* for FOURIER Image : histogram of the imaginary part*/ 
		       "-vc", "",
		       NULL };
  
  
  char * help_msg =
  {("Compute the histogram of the value of a line and put the result in name (Extimage, name,nbox).\n"
    "  -x give the lower and upper bound of the histogram .\n"
    "  -A compute the histogram of the argument of the Extrema (default module of the extrema).\n"
    "  -vc Compute the histogram only on maxima that are on a vertical chain.")
};
  
  ExtImage *image1; 
  Signal *result; 
  char   *name;
  real   xmin,xmax;
  int    nbox, flag_arg = MOD_STAT;
  int    max_flag = LINE_MAX_STAT;
  Line   *line_ptr;
  int is_vc;  
  xmin=1.0;
  xmax=-1.0;

  flag_arg = 0;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &image1, &line_ptr, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_present(2)) 
    flag_arg = 1;

  is_vc = arg_present (3);
  if (is_vc)
    max_flag = VERT_CHAINED_MAX_STAT;

  if (flag_arg == 0) {
    if (xmin > xmax)  {
      ExtImaMinMax(image1, &xmin, &xmax);
    }
    if (xmin == xmax) {
      xmin -= .5;
      xmax += .5;
    }
  }
  else {
    if (xmin > xmax)  {
      ExtImaMinMaxArg(image1, &xmin, &xmax);
    }
    if (xmin == xmax) {
      xmin -= .5;
      xmax += .5;
    }
  }
     
  result = sig_new (REALY, 0, nbox - 1);

  result =  stat_line_histogram (line_ptr, nbox, xmin, xmax,
				 flag_arg, max_flag, result);

  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(name, result);
  
  return TCL_OK;

}

/*
 */
int
stat_x_line_size_histo_TclCmd_(ClientData clientData,
			       Tcl_Interp * interp,
			       int        argc,
			       char       ** argv)
{
  char * options[] = { "Esd" ,           /* Image, resultat, nombre de boite*/
		       "-x","ff",         /* borne xmin et xmax de l'histogramme */
		       NULL };
  
  
  char * help_msg =
  {("Compute the histogram of the size of the lines of an ExtImage. \n"
    "  -x give the lower and upper bound of the histogram .")};
  
  ExtImage *ext_image; 
  Signal   *result; 
  char     *name;
  real     x_min, x_max;
  int      nbox;
  
  x_min = 1.0;
  x_max = -1.0;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &ext_image, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &x_min, &x_max) == TCL_ERROR)
    return TCL_ERROR;

  result =  stat_x_line_size_histo (ext_image, x_min, x_max, nbox);

  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(name, result);
  
  return TCL_OK;
}

/*
 */
int
stat_n_line_histo_TclCmd_(ClientData clientData,
			  Tcl_Interp * interp,
			  int        argc,
			  char       ** argv)
{
  /* Command line definition */
  char * options[] =
  {
    "Esd",
    "-x", "ff",
    NULL
  };
  
  char * help_msg =
  {
    (" Compute the histogram of the size of the lines of an ExtImage by the\n"
     "number of loc max.\n"
     "Parameters :\n"
     "  ext image - ext image to treat.\n"
     "  string    - name of the resulting signal.\n"
     "  integer   - number of values for the histogram.\n"
     "Options :\n"
     "  -x : domain of the histogram.\n"
     "    2 floats - lower and upper bound of the histogram .")
  };
  
  /* Command's parameters */
  ExtImage *ext_image; 
  char     *name;
  int      nbox;

  /* Options's presence */

  /* Options's parameters */
  real     x_min, x_max;

  /* Other variables */

  /* Command line analysis */
  Signal   *result; 
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &ext_image, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  x_min = 1.0;
  x_max = -1.0;
  if (arg_get (1, &x_min, &x_max) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  result =  stat_n_line_histo (ext_image, x_min, x_max, nbox);

  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(name, result);
  
  return TCL_OK;
}

/*
 */
int
s_histo_TclCmd_(ClientData clientData,
		Tcl_Interp * interp,
		int        argc,
		char       ** argv)
{
  /* Command line definition */
  char * options[] =
  {
    "Ssd",
    "-x", "ff",
    "-fct", "s",
    NULL
  };
  
  char * help_msg =
  {
    (" Compute the histogram of a signal.\n"
     "\n"
     "Parameters :\n"
     "  signal  - signal to treat (REALY or REALXY).\n"
     "  string  - name of the resulting signal.\n"
     "  integer - number of values for the histogram.\n"
     "\n"
     "Options :\n"
     "  -x : domain of the histogram.\n"
     "    2 floats - lower and upper bound of the histogram.\n"
     "  -fct : apply a function to each value.\n"
     "    string - the mathematical expression of the function.")
  };
  
  /* Command's parameters */
  Signal *signal;
  char   *name;
  int    nbox;

  /* Options's presence */
  int isX;
  int isFct;

  /* Options's parameters */
  real   x_min = 1.0;
  real   x_max = -1.0;
  char   *fct_expr;

  /* Other variables */
  Signal *result; 
  real   *weight = NULL;
  /*double (*fct_ptr)() = NULL;*/
  void   *fct_ptr;
  real   *data;
  int    imin;
  int    imax;

  /* Command line analysis */
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &signal, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  isX = arg_present(1);
  if (isX) {
    if (arg_get (1, &x_min, &x_max) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isFct = arg_present(2);
  if (isFct) {
    if (arg_get (2, &fct_expr) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */
  if (isFct) {
    /*fct_ptr = dfopen (fct_expr);*/
    fct_ptr = evaluator_create(fct_expr);
    if (!fct_ptr) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  }
  if (x_min > x_max) {
    sig_get_extrema( signal, &x_min, &x_max, &imin, &imax);
    /*    sig_get_extrema( signal, &x_min, &x_max );*/
  }


  /* Treatement */
  switch (signal->type) {
  case REALY:
  case REALXY:
    data = signal->dataY;
    break;
  case CPLX:
  case FOUR_NR:
    Tcl_AppendResult (interp,
		      "bad signal type",
		      (char *) NULL);
    return TCL_ERROR;
    break;
  }

  result = sig_new (REALY, 0, nbox-1);

  if (!result) {
    Tcl_AppendResult (interp,
		      "memory allocation error",
		      (char *) NULL);
    return TCL_ERROR;
  }

  result->dataY = 
    stat_array_histo( data,
		      signal->size,
		      x_min,
		      x_max,
		      result->dataY,
		      nbox,
		      &(result->x0),
		      &(result->dx),
		      fct_ptr,
		      weight );

  if ( isFct ) {
    /*dfclose( fct_ptr );*/
    evaluator_destroy(fct_ptr);
  }
  
  store_signal_in_dictionary( name, result );
  
  return TCL_OK;
}

/*
 */
int
i_histo_TclCmd_(ClientData clientData,
		Tcl_Interp * interp,
		int        argc,
		char       ** argv)
{
  /* Command line definition */
  char * options[] =
  {
    "Isd",
    "-x", "ff",
    "-fct", "s",
    "-mask", "dddd",
    NULL
  };
  
  char * help_msg =
  {
    (" Compute the histogram of an image.\n"
     "\n"
     "Parameters :\n"
     "  image   - image to treat (PHYSICAL).\n"
     "  string  - name of the resulting signal.\n"
     "  integer - number of values for the histogram.\n"
     "\n"
     "Options :\n"
     "  -x    : domain of the histogram.\n"
     "       2 floats - lower and upper bound of the histogram.\n"
     "  -fct  : apply a function to each value.\n"
     "       string - the mathematical expression of the function.\n"
     "  -mask : apply a rectangular mask on image (points that are inside\n"
     "          this mask are not taken into account to compute histogram !!).\n"
     "       4 integers - position of the mask (see iicut and itracerect).")
  };
  
  /* Command's parameters */
  Image *image;
  char  *name;
  int   nbox;
  int   posX, posY;
  int   width, height;

  /* Options's presence */
  int isX;
  int isFct;
  int isMask;

  /* Options's parameters */
  real   x_min = 1.0;
  real   x_max = -1.0;
  char   *fct_expr;

  /* Other variables */
  Signal *result; 
  real   *weight = NULL;
  /*double (*fct_ptr)() = NULL;*/
  void   *fct_ptr;
  real   *data;

  /* Command line analysis */
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &image, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  isX = arg_present(1);
  if (isX) {
    if (arg_get (1, &x_min, &x_max) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isFct = arg_present(2);
  if (isFct) {
    if (arg_get (2, &fct_expr) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isMask = arg_present(3);
  if (isMask) {
    if (arg_get (3, &posX, &posY, &width, &height) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */
  if (isFct) {
    /*fct_ptr = dfopen (fct_expr);*/
    fct_ptr = evaluator_create(fct_expr);
    if (!fct_ptr) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  }
  if (x_min > x_max) {
    im_get_extrema( image, &x_min, &x_max );
  }

  if (isMask) {
    /* test if the upper left corner is inside the image */
    if (posX < 0 || posX >= image->lx || posY < 0 || posY >= image->ly) {
      sprintf (interp -> result,"Wrong coordinates for upper left corner.\n");
      return TCL_ERROR;
    }

    /* other test*/
    if (width <= 0 || height <= 0) {
      sprintf (interp -> result,"Wrong sizes for the mask.\n");
      return TCL_ERROR;
    }

    /* test if the image can contain the rectangle */
    if (posX+width >= image->lx || posY+height >= image->ly) {
      sprintf (interp -> result,"Wrong dimensions of rectangle.\n");
      return TCL_ERROR;
    }
  }

  /* Treatement */
  switch (image->type) {
  case PHYSICAL:
    data = image->data;
    break;
  case FOURIER:
    Tcl_AppendResult (interp,
		      "bad image type",
		      (char *) NULL);
    return TCL_ERROR;
    break;
  }
  
  result = sig_new (REALY, 0, nbox-1);

  if (!result) {
    Tcl_AppendResult (interp,
		      "memory allocation error",
		      (char *) NULL);
    return TCL_ERROR;
  }

  if (isMask) {
    /* toto = */
    result->dataY =
      stat_array_histo_mask( data,
			     image->size,
			     x_min,
			     x_max,
			     result->dataY,
			     nbox,
			     &(result->x0),
			     &(result->dx),
			     fct_ptr,
			     weight,
			     image->lx,
			     posX,
			     posY,
			     width,
			     height);
  } else {
    result->dataY = 
      stat_array_histo( data,
			image->size,
			x_min,
			x_max,
			result->dataY,
			nbox,
			&(result->x0),
			&(result->dx),
			fct_ptr,
			weight );
  }
  if ( isFct ) {
    /*dfclose( fct_ptr );*/
    evaluator_destroy(fct_ptr);
  }
  
  store_signal_in_dictionary( name, result );
  /*  sprintf( interp->result, "%d", toto);*/
  return TCL_OK;
}

/*
 * command name in xmurf : ehisto2
 */
int
e_histo_TclCmd_(ClientData clientData,
		Tcl_Interp * interp,
		int        argc,
		char       ** argv)
{
  /* Command line definition */
  char * options[] =
  {
    "Esd",
    "-x", "ff",
    "-fct", "s",
    "-arg", "",
    "-gradx", "",
    "-grady", "",
    "-vc", "",
    "-grad", "",
    "-theta", "ff",
    "-gr", "",
    "-tag", "",
    "-notag", "",
    NULL
  };
  
  char * help_msg =
  {
    (" Compute the histogram of the modulus of the gradient of an ext image.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage - ext image to treat.\n"
     "  string   - name of the resulting signal.\n"
     "  integer  - number of values for the histogram.\n"
     "\n"
     "Options :\n"
     "  -x : domain of the histogram.\n"
     "    2 floats - lower and upper bound of the histogram.\n"
     "  -fct : apply a function to each value.\n"
     "    string - the mathematical expression of the function.\n"
     "  -arg : use argument of the gradient in place of its modulus.\n"
     "  -gradx : use the x ordinate of the gradient in place of its modulus.\n"
     "  -grady : use the y ordinate of the gradient in place of its modulus.\n"
     "  -vc : use only points that are on a vertical chain.\n"
     "  -grad : the result is an image of the 2d histogram of the gradient as "
     "a vector. This option works only with -vc and -x options.\n"
     "  -theta : only takes the gradients which angle is near a given value.\n"
     "    float - theta : the value .\n"
     "    float - dt : defines the intervall : [theta-dt, theta+dt]")
  };
  
  /* Command's parameters */
  ExtImage *extImage;
  char  *name;
  int   nbox;

  /* Options's presence */
  int isX;
  int isFct;
  int isArg;
  int isGradx;
  int isGrady;
  int isVc;
  int isGrad;
  int isTheta;
  int isGr;
  int isTag;
  int isNotag;

  /* Options's parameters */
  real   x_min = 1.0;
  real   x_max = -1.0;
  char   *fct_expr;
  real   theta;
  real   dTheta;

  /* Other variables */
  Signal *result; 
  real   *weight = NULL;
  /*double (*fct_ptr)() = NULL;*/
  void   *fct_ptr;
  real   *data;
  int    setLimits = 0;
  int    i;
  int    size = 0;
  real   arg;
  Extremum *extr;
  Line *line;

  /* Command line analysis */
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &extImage, &name, &nbox) == TCL_ERROR)
    return TCL_ERROR;

  isX = arg_present(1);
  if (isX) {
    if (arg_get (1, &x_min, &x_max) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  if (x_min > x_max) {
    /*
     * x_min and x_max are set later.
     */

    setLimits = 1;
  }

  isFct = arg_present(2);
  if (isFct) {
    if (arg_get (2, &fct_expr) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isArg = arg_present(3);
  isGradx = arg_present(4);
  isGrady = arg_present(5);
  isVc = arg_present(6);
  isGrad = arg_present(7);

  isTheta = arg_present(8);
  if (isTheta) {
    if (arg_get (8, &theta, &dTheta) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isGr = arg_present(9);
  isTag = arg_present(10);
  isNotag = arg_present(11);

  /* Parameters validity and initialisation */
  if (isFct) {
    /*fct_ptr = dfopen (fct_expr);*/
    fct_ptr =evaluator_create(fct_ptr);
    if (!fct_ptr) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  }
  if (nbox <= 0) {
    sprintf(interp->result, "bad number of values for the histogram (%d)", nbox);
    return TCL_ERROR;
  }

  if (isGrad && (isTag || isNotag)) {
    sprintf(interp->result, "-grad option does not work with -tag or notag option");
    return TCL_ERROR;
  }

  /* Treatement */

  if (isGrad) {
    /*
     * The treatment of option -grad is appart.
     * Perhaps I should move this treatment in another command.
     */

    Image *result_image;
    int   max_flag = LINE_MAX_STAT;

    if (isVc) {
      max_flag = VERT_CHAINED_MAX_STAT;
    }

    result_image = stat_extimage_gradient_histogram (extImage, nbox,
						     &x_min, &x_max, max_flag);

    if (!result_image)
      return TCL_ERROR;

    store_image (name, result_image);
    sprintf (interp -> result, "%f %f", x_max, x_min);

    return TCL_OK;
  }

  /*
   * Here begins the treatment if there is no -grad option.
   */

  result = sig_new (REALY, 0, nbox-1);
  if (!result) {
    Tcl_AppendResult (interp,
		      "memory allocation error",
		      (char *) NULL);
    return TCL_ERROR;
  }

  data = (real *) malloc( sizeof( real )*extImage->extrNb );
  if (!data) {
    Tcl_AppendResult (interp,
		      "memory allocation error",
		      (char *) NULL);
    sig_free( result );
    return TCL_ERROR;
  }

  /*
   * We select the data to compute.
   */
  if (isGr) {
    foreach (line, extImage->line_lst){
      foreach (extr, line->gr_lst) {
	if (extr->up || extr->down || !isVc) {
	  arg = extr->arg;
	  if (isTheta && fabs(arg-theta) > dTheta) {
	    continue;
	  }
	  if (isTag && !extr->next) {
	    continue;
	  }
	  if (isNotag && extr->next) {
	    continue;
	  }
	  if (isArg) {
	    data[size] = extr->arg;
	  } else if (isGradx) {
	    data[size] = extr->mod*cos(extr->arg);
	  } else if (isGrady) {
	    data[size] = extr->mod*sin(extr->arg);
	  } else {
	    data[size] = extr->mod;
	  }
	  if (setLimits) {
	    if (data[size] < x_min) {
	      x_min = data[size];
	    }
	    if (data[size] > x_max) {
	      x_max = data[size];
	    }
	  }
	  size++;
	}
      }
    }
  } else {
    for (i = 0; i < extImage->extrNb; i++) {
      if (extImage->extr[i].up || extImage->extr[i].down || !isVc) {
	arg = extImage->extr[i].arg;
	extr = &extImage->extr[i];
	if (isTheta && fabs(arg-theta) > dTheta) {
	  continue;
	}
	if (isTag && !extr->next) {
	  continue;
	}
	if (isNotag && extr->next) {
	  continue;
	}
	if (isArg) {
	  data[size] = extImage->extr[i].arg;
	} else if (isGradx) {
	  data[size] = extImage->extr[i].mod*cos(extImage->extr[i].arg);
	} else if (isGrady) {
	  data[size] = extImage->extr[i].mod*sin(extImage->extr[i].arg);
	} else {
	  data[size] = extImage->extr[i].mod;
	}
	if (setLimits) {
	  if (data[size] < x_min) {
	    x_min = data[size];
	  }
	  if (data[size] > x_max) {
	    x_max = data[size];
	  }
	}
	size++;
      }
    }
  }

  if (size == 0 && setLimits) {
      Tcl_AppendResult (interp,
			"no point to compute and no limits set",
			(char *) NULL);
      sig_free( result );
      return TCL_ERROR;
  }

  result->dataY = 
    stat_array_histo( data,
		      size,
		      x_min,
		      x_max,
		      result->dataY,
		      nbox,
		      &(result->x0),
		      &(result->dx),
		      fct_ptr,
		      weight );

  if ( isFct ) {
    /*dfclose( fct_ptr );*/
    evaluator_destroy(fct_ptr);
  }

  free(data);

  store_signal_in_dictionary( name, result );
  
  return TCL_OK;
}
