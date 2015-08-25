/*
 * image_cmds.c --
 *
 *   Copyright (c) 1999 Nicolas Decoster
 *   Copyright (c) 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *
 *   Copyright (c) 1999-2007 Pierre Kestener.
 *   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
 *   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
 *   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
 *
 */

#include <math.h>

#include "smPkgInt.h"

#include "../image/image.h"
#include "../image/image3D.h"
/*#include "../image/image_int.h"*/
#include "../signal/signal.h"

/*#include <defunc.h>*/
#include <matheval.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>

#include <string.h>
#include <assert.h>

// for HDF5 file format reading
#ifdef USE_HDF5
#include <hdf5.h>
#endif // USE_HDF5


/*void store_image (char  *name, Image *image_ptr);
  void store_signal_in_dictionary (char   *name, Signal *signal_ptr);
*/
#define SEUIL 10e-6

#define log2(x) (log(x)/log(2))

int next_power_of_2__(int i);

/* ***********************************************
   declaration de structures et fonctions utilisees
   pour la lecture d'images tiff 16 bits
   *********************************************** */


typedef struct TIFFHEADER{
  unsigned short int byteorder;
  unsigned short int version;
  unsigned long int ifdoffset;
} TIFFHEADER ;

typedef struct TIFFTAG{
  unsigned short int tag;
  unsigned short int type;
  unsigned long int length;
  unsigned long int valoffset;
} TIFFTAG;

typedef struct TIFFIFD{
  unsigned short int count;
  unsigned long int directory; /* struct TIFFTAG *directory; */
  unsigned long int nextifd;   /* struct TIFFIFD *nextifd;   */
} TIFFIFD;


unsigned short int get_byte_order()
{
  unsigned short endian = 0x0001;
  unsigned char *littleEndian = (unsigned char *)&endian;

  if( *littleEndian ){
    /* printf( "\nIntel machine"); */
    return((unsigned short int)0x4949);
  }
  else{
    /* printf( "Not Intel machine"); */
    return((unsigned short int)0x4d4d);
  }
}

/* ***************************************
   0x4949 -> Intel Machine Format
   -> Least Signifiant Byte First
   -> Little Endian

   0x4d4d -> Motorola Format
   -> Most Significant Byte First
   -> Big Endian
	  
   * ****************************************/

unsigned short int tiff_read_short(FILE * fp, unsigned short int byteorder)
{
  /* byteorder must be 0x4949 or 4d4d */
  char c1, c2;
  unsigned short int val;
  
  if (fread(&c1, sizeof(char), 1,fp) && fread(&c2, sizeof(char), 1,fp))
    {
      if( byteorder == 0x4d4d) {
	val = ((unsigned char)c1 << 8) | (unsigned char)c2;
      } else if (byteorder == 0x4949) {
	val = ((unsigned char)c2 << 8) | (unsigned char)c1;
      } else {
	return TCL_ERROR;
      }
    }
  return val;
}


unsigned short int tiff_read_short_crm(FILE * fp)
{
  /* byteorder must be 0x4949 */
  /* pour les fichiers de Montreal (little endian)
     premier byte lu est l'octet de poids faible */
  char c1, c2;
  unsigned short int tmp;

  if (fread(&c1, sizeof(char), 1,fp) && fread(&c2, sizeof(char), 1,fp)) {
    tmp = ((unsigned char)c2 << 8) | (unsigned char)c1;
    tmp = tmp >>4;
  } else {
    return TCL_ERROR;
  }
  return tmp;
}

unsigned long int tiff_read_long(FILE * fp, unsigned short int byteorder)
{
  char c1, c2, c3, c4;
  unsigned long int val;

  if (fread(&c1, sizeof(char), 1,fp) && fread(&c2, sizeof(char), 1,fp) &&
      fread(&c3, sizeof(char), 1,fp) && fread(&c4, sizeof(char), 1,fp) )
    {
      if( byteorder == 0x4d4d )
	val = ((unsigned char)c1 << 24) | ((unsigned char)c2 << 16) |
	  ((unsigned char)c3 << 8) | (unsigned char)c4;
      else
	val = ((unsigned char)c4 << 24) | ((unsigned char)c3 << 16) |
	  ((unsigned char)c2 << 8) | (unsigned char)c1;
    }
  return val;
}

/* ***********************************************
   fin de declaration
   *********************************************** */


/* ***********************************************
   fonction pour echanger les octets
   Big-->LitEndian et inversement
   repris dans Lastwave 1.7
   *********************************************** */

/* 
 * Function to convert a Big (resp. Little) Endian array of values to a Little (resp. Big) array of values 
 *
 *    array   : is the array of values
 *    n       : the number of values
 *    sizeval : the number of bytes of each value
 */
void BigLittleValues(void *array, int n, size_t sizeval)
{
  size_t i;
  int j;
  unsigned char c;
  unsigned char *pvar;
  
  for (j=0;j<n*sizeval;j+=sizeval) {
    
    pvar = ((unsigned char *) array) + j;
  
    for(i=0;i<sizeval/2;i++) {
      c = *(pvar+i);
      *(pvar+i) = *(pvar+sizeval-1-i);
      *(pvar+sizeval-1-i) = c;
    }
  }
}

/* *******************************************
**********************************************
**********************************************
**********************************************
********************************************** */

/*
 */
int
im_real_to_complex_TclCmd_ (ClientData clientData,
			    Tcl_Interp *interp,
			    int        argc,
			    char       **argv)
{
  /* Command line definition */
  char * options[] =
    {
      "Is",
      NULL
    };

  char * help_msg =
    {
      (" Convert a real image in a complex Image.\n"
       "\n"
       "Parameters :\n"
       "  image  - source image.\n"
       "  string - name of the new image.")
    };

  /* Command's parameters */
  Image *src_image;
  char  *dst_name;

  /* Other variables */
  Image *dst_image;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &src_image, &dst_name) == TCL_ERROR)
    return TCL_ERROR;
  
  /* Parameters validity and initialisation */
  if (src_image -> type == FOURIER)
    return GenErrorAppend(interp, "`", argv[0],
			  "' already applied to `",
                          argv[1], "'.", NULL);

  /* Treatement */
  dst_image = im_real_to_complex (src_image);

  if (!dst_image)
    return TCL_ERROR;

  store_image (dst_name, dst_image);
  
  return TCL_OK;
}

/*
 */
int
im_mult_TclCmd_ (ClientData clientData,
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
      (" Multiplication of two images (real or complex), the result is stored\n"
       "in the first image.\n"
       "\n"
       "Parameters :\n"
       "  2 images - images to treat")
    };

  /* Command's parameters */
  Image *image1, *image2;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2) == TCL_ERROR)
    return TCL_ERROR;  
  
  /* Parameters validity and initialisation */
  if ((image1 -> lx != image2 -> lx) || (image1 -> ly != image2 -> ly))
    return GenErrorAppend(interp, "Images have not the same size.", NULL);

  if (image1 -> type != image2 -> type)
    return GenErrorAppend(interp, "Images have not the same type.", NULL);

  /* Treatement */
  image1 = im_mult (image1, image2);

  if (!image1)
    return TCL_ERROR;

  return TCL_OK;
}

/* attention toto
 */
int
im_toto_TclCmd_ (ClientData clientData,
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



/*
 */
int
im_scalar_mult_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  /* Command line definition */
  char * options[] = {
    "If",
    NULL
  };
  
  char * help_msg =
    {
      (" Multiply an image by a scalar.\n"
       "\n"
       "Parameters :\n"
       "  image - image to treat.\n"
       "  real  - _the_ scalar.")
    };

  Image *image;
  real  scalar;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &scalar) == TCL_ERROR)
    return TCL_ERROR;

  image = im_scalar_mult (image, scalar);

  if (!image)
    return TCL_ERROR;

  return TCL_OK;
}

/*
 */
int
im_scalar_add_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{
  char * options[] = {
    "If",
    NULL
  };

  char * help_msg =
    {("Add a scalar to an image.")};

  Image *image;
  real scalar;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &scalar)==TCL_ERROR)
    return TCL_ERROR;  

  image = im_scalar_add (image, scalar);

  if (!image)
    return TCL_ERROR;

  return TCL_OK;
}

/* **************************************
 * Command name in xsmurf : iadd
 * **************************************/
int
im_add_TclCmd_ (ClientData clientData,
		Tcl_Interp *interp,
		int        argc,
		char       **argv)
{
  char * options[] = { "II",
		       NULL };

  char * help_msg =
    {("Addition of two images, the result is in the first image.")}; 

  Image *image1, *image2;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2) == TCL_ERROR)
    return TCL_ERROR;  

  if((image1 -> lx != image2 -> lx) || (image1 -> ly != image2 -> ly))
    return GenErrorAppend(interp,"Images have not the same size.",NULL);
  else if (image1 -> type != image2 -> type)
    return GenErrorAppend(interp,"Images have not the same type.",NULL);
  
  image1 = im_add (image1, image2);

  if (!image1)
    return TCL_ERROR;

  return TCL_OK;
}

/***************************************
 * Command name in xsmurf : imax
 ***************************************/
int
im_max_TclCmd_ (ClientData clientData,
		Tcl_Interp *interp,
		int        argc,
		char       **argv)
{
  char * options[] = { "II",
		       NULL };

  char * help_msg =
    {("Take the maximum pixel values of two images, the result is in the first image.")}; 

  Image *image1, *image2;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2) == TCL_ERROR)
    return TCL_ERROR;  

  if((image1 -> lx != image2 -> lx) || (image1 -> ly != image2 -> ly))
    return GenErrorAppend(interp,"Images have not the same size.",NULL);
  else if (image1 -> type != image2 -> type)
    return GenErrorAppend(interp,"Images have not the same type.",NULL);
  
  image1 = im_max (image1, image2);

  if (!image1)
    return TCL_ERROR;

  return TCL_OK;
}

/*
 * Commande name in xsmurf :
 */
int
im_filter_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{
  char * options[] = { "If",
		       NULL };
 
  char * help_msg =
    {("Thresh the values of an image (image threshold).")};

  Image *image;
  real  value;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &value) == TCL_ERROR)
    return TCL_ERROR;

  image = im_filter (image, value);

  if (!image)
    return TCL_ERROR;

  return TCL_OK;
}

/*
 * Commande name in xsmurf :
 */
int
im_conv_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char **argv)      
{
  char * options[] = { "IIs",
		       NULL };
  
  char * help_msg =
    {("Compute the discret convolution between two PHYSICAL images ant put it in an image (image, image, result).")};

  Image *image1, *image2, *dst_image;
  char  *dst_imageName;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2, &dst_imageName) == TCL_ERROR)
    return TCL_ERROR;

  dst_image = im_conv (image1, image2);

  if (!dst_image)
    return TCL_ERROR;

  store_image (dst_imageName, dst_image);

  return TCL_OK;
}

/************************************
  command name in xsmurf : icreate
  modified : 19/03/2001
***********************************/
int
im_create_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char **argv)      
{
  char * options[] = { "s[df]",
		       "-user", "ddS",
		       NULL };
  char * help_msg =
    {("Create a new image (name, [dim, value]).\n"
      "Options :\n"
      "   -user [ddS] : specify lx, ly and values\n"
      "         Take care that signal size must be at least lx*ly\n")}; 
  
  Image *image;
  char  *imageName;
  int   dim = 32;
  real   value = 0.0;
  int   i;
  int lx, ly;
  Signal *sig;
  int isUser;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName, &dim, &value) == TCL_ERROR)
    return TCL_ERROR;

  isUser = arg_present(1);
  if (isUser) {    if (arg_get (1, &lx, &ly, &sig) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  if (isUser) {
    image = im_new (lx, ly, lx*ly, PHYSICAL);
  } else {
    image = im_new (dim, dim, dim*dim, PHYSICAL);
  }
  if (!image)
    return TCL_ERROR;

  /*  im_set_0 (image);*/
  if(isUser) {
    for (i = 0; i < image->size; i++)
      image -> data[i] = sig->dataY[i];
  } else {
    for (i = 0; i < image->size; i++)
      image -> data[i] = value;
  }

  store_image (imageName, image);

  return TCL_OK;
}

/*
 * Commande name in xsmurf :
 */
int
im_insert_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char **argv)      
{
  char * options[] = { "IIdd",
		       NULL };

  char * help_msg =
    {("Insert image2 in image1 at the position x,y (image1, image2, x, y).")}; 
 
  Image *image1, *image2;
  int   x_pos, y_pos;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2, &x_pos, &y_pos) == TCL_ERROR)
    return TCL_ERROR;

  im_insert (image1, image2, x_pos, y_pos);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int ImaCrt2GradModCmd_(ClientData clientData,Tcl_Interp *interp,
		       int argc,char **argv)      
{
  char * imageName;
  Image * image, * imageDdx,* imageDdy;
  
  char * options[] = { "IIs", NULL };

  char * help_msg =
    {("Compute the modulus from the x-derivative and the y-derivative images (image image name).")};
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0,&imageDdx,&imageDdy,&imageName)==TCL_ERROR)
    return TCL_ERROR;
  
  if ((imageDdx->lx!=imageDdy->lx) || (imageDdx->ly!=imageDdy->ly))
    return GenErrorAppend(interp, "Error: `",argv[1],"' and `",argv[2],
			  "' must have the same size.",NULL);
  
  image = im_xy_to_mod (imageDdx, imageDdy);

  if (!image)
    return GenErrorMemoryAlloc(interp);

  store_image(imageName,image);
  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}

/*********************************
 * Commande name in xsmurf : garg
 *********************************/
int ImaCrt2GradArgCmd_(ClientData clientData,Tcl_Interp *interp,
		       int argc,char **argv)      
{ 
  real thresh=0.001;
  char * imageName;
  Image * image, * imageDdx,* imageDdy;
  
  char * options[] = { "IIs",
		       "-thresh", "f",
		       NULL};

  char * help_msg =
    {("Compute the gradient from the x-derivative and the y-derivative images (image image name)\n"
      "   -thresh threshold of the gradient (default =1e-6).")};

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0,&imageDdx,&imageDdy,&imageName)==TCL_ERROR)
    return TCL_ERROR;
  
  if ((imageDdx->lx!=imageDdy->lx) || (imageDdx->ly!=imageDdy->ly))
    return GenErrorAppend(interp, "Error: `",argv[1],"' and `",argv[2],
			  "' must have the same size.",NULL);
  
  if (arg_get(1, &thresh) == TCL_ERROR)
    return TCL_ERROR;

  image = im_xy_to_arg (imageDdx, imageDdy, thresh);

  if (!image)
    return GenErrorMemoryAlloc(interp);
  
  store_image(imageName,image);
  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}



/*------------------------------------------------------------------------
  enleve les bords (attention!! pour images carrees)
  ----------------------------------------------------------------------*/
int 
ImaCvlCutEdgeCmd_(ClientData clientData,Tcl_Interp *interp,
		  int argc,char **argv)      
{ 
  char * options[] = { "Isd",
		       NULL };
  char  *dst_name;
  Image *src_image, *dst_image;
  int   cut_size;

  char * help_msg =
    {("Cut a border of size l of the image1 (only for squared image), the result is in image2 (image1, image2, l).")};

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &src_image, &dst_name, &cut_size) == TCL_ERROR)
    return TCL_ERROR;

  dst_image = im_cut_edge (src_image, cut_size);

  store_image(dst_name, dst_image);

  return TCL_OK;
}

/*
 * Commande name in xsmurf :
 */
int
im_diamond_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  char * options[] = { "sd",
		       NULL };

  char * help_msg =
    {("Create a dirac of a shape of diamond (name size).")};
 
  int dim;
  Image * image;
  char * imageName;
 
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName, &dim) == TCL_ERROR)
    return TCL_ERROR;

  if (dim < 0)
    return GenErrorAppend (interp, "size = ", argv[2], " ? ", NULL);

  image = im_diamond (dim);

  if (!image)
    return GenErrorMemoryAlloc (interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}

/* ********************************
 * Command name in xsmurf : idirac
 * ********************************/
int
im_dirac_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  char * options[] = { "sd",
		       "-dd", "d",
		       "-y", "d",
		       "-td", "dd",
		       NULL };
  
  char * help_msg =
    {("Create a dirac (name size)\n"
      " -dd double dirac at a distance of int pixel from the center, real equal the rapport of the amplitude\n"
      " -y  y-dimension \n"
      " -td triple dirac at a distance of int1 pixel in the direction of x\n"
      "     and int2 pixels in the direction of y.")};

  int dimx, dimy, dist = -1, distx=-1, disty;
  Image * image;
  char * imageName;
 
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName, &dimx) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (1, &dist) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (3, &distx, &disty) == TCL_ERROR)
    return TCL_ERROR;

  dimy = dimx;

  if (arg_get (2, &dimy) == TCL_ERROR)
    return TCL_ERROR;

  if (dimx < 0 || dimy < 0)
    return GenErrorAppend (interp, "size = ", argv[2], " ? ", NULL);

  image = im_dirac (dimx, dimy, dist, distx, disty);

  if (!image)
    return GenErrorMemoryAlloc (interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}

/* ********************************
 * Command name in xsmurf : igauss
 * ********************************/
int
im_gauss_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  char * options[] = { "sdddf[f]",
		       "-dx", "",
		       "-dy", "",
		       NULL };
  
  char * help_msg =
    {("Create a square gaussian image (name size x0 y0 scale [factor])\n"
      "gauss(x,y)= scale * exp (- ((x-x0)^2 + (y-y0)^2)/scale^2)\n"
      "\n"
      "Options:\n"
      "  -dx : x derivative of a gaussian\n"
      "  -dy : y derivative of a gaussian\n"
      )};

  int size, x0, y0;
  Image * image;
  char * imageName;
  real scale, factor=1.0;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName, &size, &x0, &y0, &scale, &factor) == TCL_ERROR)
    return TCL_ERROR;


  if (x0 < 0 || y0 < 0 || x0 >= size || y0 >= size)
    return GenErrorAppend (interp, "wrong x0 or y0 !! ", NULL);

  if (arg_present(1)) {
    image = im_gauss (size, x0, y0, scale, 1, factor);
  } else if (arg_present(2)) {
    image = im_gauss (size, x0, y0, scale, 2, factor);
  } else {
    image = im_gauss (size, x0, y0, scale, 0, factor);
  }

  if (!image)
    return GenErrorMemoryAlloc (interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}

/*********************************
 * Command name in xsmurf : igaussellipse
 *********************************/
int
im_gauss_ellipse_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  char * options[] = { "sdddfff",
		       NULL };
  
  char * help_msg =
    {("Create a square ellipso-gaussian image (name size x0 y0 scale a b)\n"
      "gauss_ellipse(x,y)= scale * exp (- (a(x-x0)^2 + b(y-y0)^2)/scale^2)\n"
      "\n"
      )};

  int size, x0, y0;
  Image * image;
  char * imageName;
  real scale, a, b;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName, &size, &x0, &y0, &scale, &a, &b) == TCL_ERROR)
    return TCL_ERROR;


  if (x0 < 0 || y0 < 0 || x0 >= size || y0 >= size)
    return GenErrorAppend (interp, "wrong x0 or y0 !! ", NULL);

  image = im_gauss_ellipse (size, x0, y0, scale, a, b );

  if (!image)
    return GenErrorMemoryAlloc (interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}

/* ********************************
 * Command name in xsmurf : icell
 ********************************/
int
im_cell_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{ 
  char * options[] = { "sddddfff",
		       NULL };
  
  char * help_msg =
    {("Create a simulated cell nucleus image with two chromosome territories.\n"
      "Give the size of the image, the radius of the nucleus \n"
      "and the size of objects A & B.\n"
      "Also give the density of the background (nucleus), and for objects A & B.\n"
      )};

  int size, rad_nuc, rad_A, rad_B;
  Image * image;
  char * imageName;
  real dens_nuc, dens_A, dens_B;

  /* command line analysis and options' presence */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName, &size, &rad_nuc, &rad_A, &rad_B, &dens_nuc, &dens_A, &dens_B) == TCL_ERROR) {
    if (rad_A+rad_B>rad_nuc)
      sprintf (interp -> result, "rad_A+rad_B should be less than rad_nuc !!!");
    return TCL_ERROR;
  }

  image = im_cell (size, rad_nuc, rad_A, rad_B, dens_nuc, dens_A, dens_B);

  if (!image)
    return GenErrorMemoryAlloc (interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}

/*********************************
 * Command name in xsmurf : icellelli
 *********************************/
int
im_cell_elli_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 


  char * options[] = { "sddddddfff",
		       NULL };
  
  char * help_msg =
    {("Create a simulated cell nucleus image with two chromosome territories.\n"
      "(See icell). But here, the territories are ellipses. \n"
      "Give the size of the image, the a and b for both objects (a > b) \n"
      "Also give the density of the background (nucleus), and for objects A & B.\n"
      )};

  int size, rad_nuc, a_A, b_A, a_B, b_B;
  Image * image;
  char * imageName;
  real dens_nuc, dens_A, dens_B;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName, &size, &rad_nuc, &a_A, &b_A, &a_B, &b_B, &dens_nuc, &dens_A, &dens_B) == TCL_ERROR)
    return TCL_ERROR;

  image = im_cell_elli (size, rad_nuc, a_A, b_A, a_B, b_B, dens_nuc, dens_A, dens_B);

  if (!image)
    return GenErrorMemoryAlloc (interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}



/*********************************
 * Command name in xsmurf : icell3D
 *********************************/
/*
  int
  im_cell3D_TclCmd_ (ClientData clientData,
  Tcl_Interp *interp,
  int        argc,
  char       **argv)
  { 
  char * options[] = { "sddddfff",
  NULL };
  
  char * help_msg =
  {("Create a 3D simulated cell nucleus image with two chromosome territories,\n"
  "Give the size of the image, the radius of the nucleus \n"
  "and the size of objects A & B.\n"
  "Also give the density of the background (nucleus), and for objects A & B.\n"
  )};
  
  FILE  *fileOut; 
  int size, rad_nuc, rad_A, rad_B;
  Image * image;
  char * imageName;
  real dens_nuc, dens_A, dens_B;

  if (arg_init (interp, argc, argv, options, help_msg))
  return TCL_OK;
  
  if (arg_get (0, &imageName, &size, &rad_nuc, &rad_A, &rad_B, &dens_nuc, &dens_A, &dens_B) == TCL_ERROR)
  return TCL_ERROR;

  if (!(fileOut = fopen(fileName, "w")))
  return GenErrorAppend(interp, "Couldn't open `", fileName,
  "' for writing.", NULL);
  
  im_cell3D (fileOut, size, rad_nuc, rad_A, rad_B, dens_nuc, dens_A, dens_B);
  
  if (!image)
  return GenErrorMemoryAlloc (interp);
  
  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);
  
  return TCL_OK;
  }

*/

/*********************************
 * Command name in xsmurf : icell3Dproj
 *********************************/
int
im_cell3Dproj_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  char * options[] = { "sddddfff",
		       NULL };
  
  char * help_msg =
    {("Create a 2D simulated cell nucleus image with two chromosome territories,\n"
      "but from the projection (sum) of a 3D object.\n"
      "Give the size of the image, the radius of the nucleus \n"
      "and the size of objects A & B.\n"
      "Also give the density of the background (nucleus), and for objects A & B.\n"
      )};

  int size, rad_nuc, rad_A, rad_B;
  Image * image;
  char * imageName;
  real dens_nuc, dens_A, dens_B;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName, &size, &rad_nuc, &rad_A, &rad_B, &dens_nuc, &dens_A, &dens_B) == TCL_ERROR)
    return TCL_ERROR;

  image = im_cell3Dproj (size, rad_nuc, rad_A, rad_B, dens_nuc, dens_A, dens_B);

  if (!image)
    return GenErrorMemoryAlloc (interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}



/********************************
 * Command name in xsmurf : istep
 ********************************/
int
im_step_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{ 
  char * options[] = {"sd",
		      NULL};

  char * help_msg =
    {("Create a step in the y direction (name size).")};

  int dim;
  Image * image;
  char * imageName;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName, &dim) == TCL_ERROR)
    return TCL_ERROR;

  if (dim < 0)
    return GenErrorAppend (interp, "size = ", argv[2], " ? ", NULL);
  
  image = im_step (dim);

  if (!image)
    return GenErrorMemoryAlloc(interp);
    
  
  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}

/* *********************************
 * Command name in xsmurf : icircle
 * *********************************/
int
im_circle_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{
  char * options[] = {"sd[d]",
		      "-center","dd",
		      "-radius","d",
		      NULL};
 
  char * help_msg =
    {("Create a dirac of a shape of a full circle (name size)."
      "\n"
      "Options:\n"
      " -center [dd]: change center of the disK\n"
      " -radius [d] : ...\n")};
  
  int lx, ly=0;
  Image * image;
  char * imageName;
  
  int isCenter, isRadius;
  int cx, cy, radius;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName,  &lx, &ly) == TCL_ERROR)
    return TCL_ERROR;
  if (ly == 0) ly = lx;

  isCenter = arg_present(1);
  if (isCenter) {
    if (arg_get (1,  &cx, &cy) == TCL_ERROR)
      return TCL_ERROR;
  } else {
    cx = lx/2;
    cy = ly/2;
  }

  isRadius = arg_present(2);
  if (isRadius) {
    if (arg_get (2,  &radius) == TCL_ERROR)
      return TCL_ERROR;
  } else {
    radius = lx/8;
  }


  if (lx < 0)
    return GenErrorAppend (interp, "size = ", argv[2], " ? ", NULL);

  image = im_circle (lx, ly, cx, cy, radius);

  if (!image)
    return GenErrorMemoryAlloc(interp);
  
  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}

/************************************
 * Command name in xsmurf : iellipse
 * 04/01/2000
 *************************************/
int
im_ellipse_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{
  char * options[] = {"sdf",
		      NULL};
 
  char * help_msg =
    {("Create a dirac of a shape of a full ellipse (name size angle)\n"
      "angle : angle in degrees between -180 and 180 degrees.")};

  int dim;
  Image * image;
  char * imageName;
  real theta;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName,  &dim,  &theta) == TCL_ERROR)
    return TCL_ERROR;

  if (dim < 0)
    return GenErrorAppend (interp, "size = ", argv[2], " ? ", NULL);

  image = im_ellipse (dim,theta);

  if (!image)
    return GenErrorMemoryAlloc(interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}

/************************************
 * Command name in xsmurf : myiellipse
 * 07/12/2005 Andre Khalil
 *************************************/
int
my_im_ellipse_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{
  char * options[] = {"sdddfff",
		      NULL};
 
  char * help_msg =
    {("Create a dirac of a shape of a full ellipse (name size x y a b angle)\n"
      "angle : angle in degrees between -180 and 180 degrees.\n"
      "NOTE : a must be less than b!!!")};

  int dim;
  Image * image;
  char * imageName;
  int x;
  int y;
  real a;
  real b;
  real theta;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName,  &dim, &x, &y,  &a, &b, &theta) == TCL_ERROR)
    return TCL_ERROR;

  if (dim < 0)
    return GenErrorAppend (interp, "size = ", argv[2], " ? ", NULL);

  image = my_im_ellipse (dim,x, y, a,b,theta);

  if (!image)
    return GenErrorMemoryAlloc(interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}


/************************************
 * Command name in xsmurf : myiellipse3Dproj
 * 07/17/2006 Andre Khalil
 *************************************/
int
my_im_ellipse_3Dproj_TclCmd_ (ClientData clientData,
			      Tcl_Interp *interp,
			      int        argc,
			      char       **argv)
{
  char * options[] = {"sddddffff",
		      NULL};
 
  char * help_msg =
    {("Create a dirac of a shape of a full ellipse, but from a 3D projection\n"
      "(name size x y z a b theta phi)\n"
      "angle : angle in degrees between -180 and 180 degrees.\n"
      "NOTE : a must be less than b!!!")};

  int dim;
  Image * image;
  char * imageName;
  int x;
  int y;
  int z;
  real a;
  real b;
  real theta;
  real phi;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName,  &dim, &x, &y, &z, &a, &b, &theta, &phi) == TCL_ERROR)
    return TCL_ERROR;

  if (dim < 0)
    return GenErrorAppend (interp, "size = ", argv[2], " ? ", NULL);

  image = my_im_ellipse_3Dproj (dim,x, y, z, a, b, theta, phi);

  if (!image)
    return GenErrorMemoryAlloc(interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}


/*****************************************
 * Command name in xsmurf : icellellipsoid
 * 07/24/2007 Andre Khalil
 *****************************************/
int
im_cell_ellipsoid_proj_TclCmd_ (ClientData clientData,
				Tcl_Interp *interp,
				int        argc,
				char       **argv)
{
  char * options[] = {"sddddddddfffd",
		      NULL};
 
  char * help_msg =
    {("Create a cell nucleus with two randomly positioned and oriented \n"
      "ellipsoidal-shaped CTs.\n"
      "Give the axis lengths for both objects, their Poisson densities, \n"
      "the Poisson density of the nuclear background, and the number of z-slices. \n"
      "(name size rad_nuc a1 b1 c1 a2 b2 c2 dens1 dens2 dens_nuc num_slices)\n")};

  Image * image;
  char * imageName;
  int dim;
  int rad_nuc;
  int a1;
  int b1;
  int c1;
  int a2;
  int b2;
  int c2;
  real dens1;
  real dens2;
  real dens_nuc;
  int num_slices;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName,  &dim, &rad_nuc, &a1, &b1, &c1, &a2, &b2, &c2, &dens1, &dens2, &dens_nuc, &num_slices) == TCL_ERROR)
    return TCL_ERROR;

  if (dim < 0)
    return GenErrorAppend (interp, "size = ", argv[2], " ? ", NULL);

  image = im_cell_ellipsoid_proj (dim, rad_nuc, a1, b1, c1, a2, b2, c2, dens1, dens2, dens_nuc, num_slices);

  if (!image)
    return GenErrorMemoryAlloc(interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}


/***********************************************
 * Command name in xsmurf : icellellipsoid_slice
 * 07/24/2007 Andre Khalil
 ***********************************************/
int
im_cell_ellipsoid_slice_TclCmd_ (ClientData clientData,
				 Tcl_Interp *interp,
				 int        argc,
				 char       **argv)
{
  char * options[] = {"sddddddddddddddfffffffd",
		      NULL};
 
  char * help_msg =
    {("Create a cell nucleus with two ellipsoidal-shaped CTs.\n"
      "Give the (x,y,z) positions for both objects, their axis lengths, \n"
      "their rotation angles theta_x and theta_y, their Poisson densities, \n"
      "the Poisson density of the nuclear background, and the z-slice number. \n"
      "(name size rad_nuc x1 y1 z1 x2 y2 z2 a1 b1 c1 a2 b2 c2 ... \n"
      "... theta_x1 theta_y1 theta_x2 theta_y2 dens1 dens2 dens_nuc slice_number)\n"
      "\n"
      " (ouf!) \n"
      "\n"
      "IMPORTANT: The user has to make sure that the ellipsoids are inside the nucleus\n"
      "by making sure that (x,y,z) is at least a distance c from the edge of the nucleus.\n"
      "Also, the user has to make sure that there is no possibility of overlap.\n"
      "\n"
      "Good luck!\n")};
  
  Image * image;
  char * imageName;
  int dim;
  int rad_nuc;
  int x1;
  int y1;
  int z1;
  int x2;
  int y2;
  int z2;
  int a1;
  int b1;
  int c1;
  int a2;
  int b2;
  int c2;
  real theta_x1;
  real theta_y1;
  real theta_x2;
  real theta_y2;
  real dens1;
  real dens2;
  real dens_nuc;
  int slice_number;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName,  &dim, &rad_nuc, &x1, &y1, &z1, &x2, &y2, &z2, &a1, &b1, &c1, &a2, &b2, &c2, &theta_x1, &theta_y1, &theta_x2, &theta_y2, &dens1, &dens2, &dens_nuc, &slice_number) == TCL_ERROR)
    return TCL_ERROR;

  if (dim < 0)
    return GenErrorAppend (interp, "size = ", argv[2], " ? ", NULL);

  image = im_cell_ellipsoid_slice (dim, rad_nuc, x1, y1, z1, x2, y2, z2, a1, b1, c1, a2, b2, c2, theta_x1, theta_y1, theta_x2, theta_y2, dens1, dens2, dens_nuc, slice_number);

  if (!image)
    return GenErrorMemoryAlloc(interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}



/**************************************
 * Command name in xsmurf : isingul2D
 * 28/09/2004
 **************************************/
int
im_singul2D_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)
{
  char * options[] = {"sd",
		      "-pos"  , "dd",
		      "-H"    , "f",
		      "-coef" , "f",
		      "-linear", "f",
		      NULL};
 
  char * help_msg =
    {("Create 2D data of punctual singularity to test the 2D methodology\n"
      " First argument is image name and second is size of data.\n"
      "\n"
      "Parameter:\n"
      "   string  : image name\n"
      "   integer : N (size of data)\n"
      "Options:\n"
      " -pos [dd] : set position of the singularity.\n"
      " -H   [f]   : set H\"older exponent.\n"
      " -coef [f]  : set multiplicative scalar.\n" 
      " -linear [f] : replace the power law singularity by a linear one\n"
      "               parameter is the slope of decrease from the summit.\n"
      "\n")};
  
  int dim;
  Image * image;
  char * imageName;
  int   i,j;
  int   pos_singul_x,pos_singul_y;
  real H,A,slope;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imageName,  &dim) == TCL_ERROR)
    return TCL_ERROR;

  if (dim < 0)
    return GenErrorAppend (interp, "size = ", argv[2], " ? ", NULL);

  //image = im_singul2D (dim,);

  if (!image)
    return GenErrorMemoryAlloc(interp);

  store_image (imageName, image);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}


/**********************************
 * Command name in xsmurf : ibro
 **********************************/
int
im_brownian_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)
{ 
  char * options[] = { "sd",
		       "-fft", "d",
		       "-alea", "d",
		       "-h", "f",
		       "-midpt", "f", 
		       "-anis", "s",
		       "-special", "f",
		       "-anis2", "f",
		       "-anis3", "ff",
		       "-bsheet", "ff",
		       "-montseny", "fff",
		       "-fft2", "s",
		       "-yaf", "ffff",
		       "-yafmont", "fffff",
		       NULL };

  char * help_msg =
    {("Create a brownian image of exponent h=0.5  (name size)\n"
      " -fft     dim of the fft\n"
      " -alea    integer between (0..99) ?????\n"
      " -h       Brownian image of exponent h\n"
      " -midpt   Brownian image by mid-point displacement.\n"
      " -anis    Create an anisotropic brownian.\n"
      "          Use the function as a function of h(theta).\n"
      " -special algorithm is almost the same as midpt.\n"
      "          (see image/generator.c)\n"
      " -anis2   Create an anisotropic brownian\n"
      "          using fractional integration along x-axis.\n"
      " -anis3   Create an anisotropic brownian\n"
      "          See code in $(XSMURFPATH)/image/generator.c\n"
      " -bsheet  Create an anisotropic Brownian Sheet in a FFT way\n"
      "          the two parameters are H_x and H_y.\n"
      " -montseny Create an anisotropic surface \"a la\" Montseny\n"
      "          the parameters are H_x, H_y and beta.\n"
      " -fft2\n"
      " -yafmont Create an anisotropic surface \"a la\" Montseny\n"
      "          but with control over the variability of the\n"
      "          x and y components.\n"
      "          (Enter Hx, Hy, and prefactors for x and y\n"
      "          and then the general beta factor).\n"
      " -yaf     Create an anisotropic surface \"a la\" Makse\n"
      "          but with control over the variability of the\n"
      "          x and y components.\n"
      "          (Enter Hx, Hy, and prefactors for x and y.)\n")};

  int   dim;
  Image *image, *image2;
  char  *imageName, *imageName2;
  int   alea;
  real  h_bro = 0.5, sigma;
  int   fft_dim = 0;
  char * h_fct_str;
  void  *h_fct;
  float  deltaH;
  real   h1, h2, mux, muy, beta;

  srand(time(NULL));
  alea = (int)(rand()/327.67); /* entre 0 et 99 */

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &fft_dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &alea)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &h_bro)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(4, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(5, &h_fct_str)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(6, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(7, &deltaH)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(8, &h1, &h2)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(9, &h1, &h2)==TCL_ERROR)
    return TCL_ERROR;

  /* montseny */
  if (arg_get(10, &h1, &h2, &beta)==TCL_ERROR)
    return TCL_ERROR;

  /* fft2 */
  if (arg_get(11, &imageName2)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(12, &h1, &h2, &mux, &muy)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(13, &h1, &h2, &mux, &muy, &beta)==TCL_ERROR)
    return TCL_ERROR;

  if (!arg_present (5)) {
    h_fct_str = (char *) malloc (sizeof (char)*2);
    h_fct_str[0] = '1';
    h_fct_str[1] = '\0';
  }
  h_fct = evaluator_create(h_fct_str);
  if (!h_fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error",
			" in expression ", h_fct_str, (char *) NULL);
      return TCL_ERROR;
    }
  
  image=im_new(dim,dim,dim*dim,PHYSICAL);

  if (!image)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (image);

  if (arg_present(4)) {
    im_construit_brownien_mid_point(image->data,dim,h_bro,alea,sigma);
  } else if (arg_present(6)) {
    im_construit_brownien_mid_point_special(image->data,dim,h_bro,alea,sigma);
  } else if (arg_present(7)) {
    im_construit_brownien_fft_anis(image->data,dim,h_bro,alea, fft_dim, h_fct, deltaH);
  } else if (arg_present(8)) {
    im_construit_brownien_fft_anis2(image->data,dim,h1,h2,alea, fft_dim);
  } else if (arg_present(9)) {
    im_construit_brownien_fft_bsheet(image->data,dim,h1,h2,alea, fft_dim);
  } else if (arg_present(10)) {
    im_construit_brownien_fft_anis_montseny(image->data,dim,h1,h2,beta,alea, fft_dim);
  } else if (arg_present(11)) {
    image2=im_new(dim,dim,dim*dim,PHYSICAL);
    if (!image2)
      return GenErrorMemoryAlloc(interp);
    im_set_0 (image2);
    im_construit_brownien_fft2(image->data, image2->data, dim, h_bro, alea, fft_dim, h_fct);
    store_image(imageName2,image2);
  } else if (arg_present(12)) {
    im_construit_brownien_fft_anis_yaf(image->data,dim,h1,h2,mux,muy,alea,fft_dim);
  } else if (arg_present(13)) {
    im_construit_brownien_fft_anis_yafmont(image->data,dim,h1,h2,mux,muy,beta,alea,fft_dim);
  } else {
    im_construit_brownien_fft(image->data, dim, h_bro, alea, fft_dim, h_fct);
  }

  store_image(imageName,image);

  Tcl_AppendResult(interp,imageName,NULL);
  evaluator_destroy(h_fct);
  return TCL_OK;
}

/***************************************
 * Command name in xsmurf : ibro2Dfield
 ***************************************/
int
im_brownian_2D_field_TclCmd_ (ClientData clientData,
			      Tcl_Interp *interp,
			      int        argc,
			      char       **argv)
{ 
  char * options[] = { "ssd",
		       "-fft", "d",
		       "-alea", "d",
		       "-h", "f",
		       "-midpt", "f", 
		       "-anis", "s",
		       "-special", "f",
		       "-pedagogic","ssfssf",
		       "-h2", "ff",
		       "-divfree", "ssf",
		       NULL };

  char * help_msg =
    {("Create a 2D brownian field (2 images for the 2 components) \n"
      "of exponent h=0.5  (name1 name2 size)\n"
      " -fft         dim of the fft\n"
      " -alea [ddd]  integer between (0..99) ?????\n"
      " -h           Brownian image of exponent h\n"
      " -midpt       Brownian image by mid-point displacement."
      " -anis        Create an anisotropic brownian.\n"
      "              Use the function as a function of h(theta).\n"
      " -special     algorithm is almost the same as midpt.\n"
      "              (see image/generator.c)\n"
      " -pedagogic   Generate three brownian fields, relying on the same\n"
      "              Gaussian noise, but with different Fourien integration.\n"
      " -h2 [ff]     Generate an anisotropic brownian field with one Husrt\n"
      "              exponent for each component.\n"
      " -divfree[ssf]     \n")};

  int   dim;
  Image *image1, *image2;
  char  *imageName1, *imageName2;
  int   alea1, alea2, alea3;
  real  h_bro = 0.5, h_brob = 0.5, sigma;
  int   fft_dim = 0;
  char * h_fct_str;
  void *h_fct;

  /* other variable */
  int isPedagogic, isDivfree;
  Image *image3,*image4,*image5,*image6;
  char  *imageName3, *imageName4,*imageName5, *imageName6;
  real   h2,h3;

  srand(time(NULL));
  alea1 = (int)(rand()/327.67); /* entre 0 et 99 */
  alea2 = (int)(rand()/327.67); /* entre 0 et 99 */
  alea3 = (int)(rand()/327.67); /* entre 0 et 99 */

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName1, &imageName2, &dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &fft_dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &alea1, &alea2, &alea3)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &h_bro)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(4, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(5, &h_fct_str)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(6, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  isPedagogic = arg_present(7);
  if (arg_present (7)) {
    if (arg_get(7, &imageName3, &imageName4, &h2,&imageName5, &imageName6, &h3) == TCL_ERROR)
      return TCL_ERROR;
  }
  
  if (arg_present (8)) {
    if (arg_get(8, &h_bro, &h_brob) == TCL_ERROR)
      return TCL_ERROR;
  }

  isDivfree = arg_present(9);
  if (arg_present (9)) {
    if (arg_get(9, &imageName3, &imageName4, &h2) == TCL_ERROR)
      return TCL_ERROR;
  }
  


  if (!arg_present (5)) {
    h_fct_str = (char *) malloc (sizeof (char)*2);
    h_fct_str[0] = '1';
    h_fct_str[1] = '\0';
  }
  h_fct = evaluator_create(h_fct_str);
  if (!h_fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error",
			" in expression ", h_fct_str, (char *) NULL);
      return TCL_ERROR;
    }
  
  image1=im_new(dim,dim,dim*dim,PHYSICAL);
  image2=im_new(dim,dim,dim*dim,PHYSICAL);

  if (!image1 || !image2)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (image1);
  im_set_0 (image2);
  
  if (arg_present(4)) {
    //im_construit_brownien_mid_point(image->data,dim,h_bro,alea,sigma);
  } else if (arg_present(6)) {
    //im_construit_brownien_mid_point_special(image->data,dim,h_bro,alea,sigma);
  } else if (isPedagogic) {
    image3=im_new(dim,dim,dim*dim,PHYSICAL);
    image4=im_new(dim,dim,dim*dim,PHYSICAL);
    image5=im_new(dim,dim,dim*dim,PHYSICAL);
    image6=im_new(dim,dim,dim*dim,PHYSICAL);
    im_construit_brownien_2D_field_fft_pedagogic(image1->data, image2->data, image3->data, image4->data, image5->data, image6->data, dim, h_bro, h2, h3,alea1, alea2, alea3, fft_dim, h_fct);
  } else if (isDivfree) {
    image3=im_new(dim,dim,dim*dim,PHYSICAL);
    image4=im_new(dim,dim,dim*dim,PHYSICAL);
    im_construit_brownien_2D_field_fft_div_free_pedagogic(image1->data, image2->data, image3->data, image4->data, dim, h_bro, h2,alea1, alea2, alea3, fft_dim, h_fct);
  } else {
    im_construit_brownien_2D_field_fft(image1->data, image2->data, dim, h_bro, h_bro, alea1, alea2, alea3, fft_dim, h_fct);
  }

  store_image(imageName1,image1);
  store_image(imageName2,image2);
  if (isPedagogic) {
    store_image(imageName3,image3);
    store_image(imageName4,image4);
    store_image(imageName5,image5);
    store_image(imageName6,image6);
  } else if (isDivfree) {
    store_image(imageName3,image3);
    store_image(imageName4,image4);
  }

  Tcl_AppendResult(interp,imageName1," ",imageName2,NULL);
  evaluator_destroy(h_fct);
  return TCL_OK;
}

/* *********************************
 * Command name in xsmurf : ibro3D
 * *********************************/
/* created on october the 2nd 2002 by Pierre Kestener */
int
im_brownian3D_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  char * options[] = { "sd",
		       "-fft", "d",
		       "-alea", "d",
		       "-h", "f",
		       "-midpt", "f", 
		       "-anis", "s",
		       "-special", "f",
		       "-savefile", "s",
		       NULL };

  char * help_msg =
    {("Create 3D brownian data of exponent h=0.5 and create an image3D object\n"
      "The First argument is the result name and the second is the\n"
      "size of the data (never try something larger than 512!!! )\n"
      " -fft     dim of the fft\n"
      " -alea    integer between (0..99) ?????\n"
      " -h       Brownian image of exponent h\n"
      " -midpt   Brownian image by mid-point displacement."
      " -anis    Create an anisotropic brownian.\n"
      "          Use the function as a function of h(theta).\n"
      " -special algorithm is almost the same as midpt.\n"
      "          (see image/generator.c)\n"
      " -savefile [s] : save data in a file.\n")};

  Image3D *image;
  char    *imageName;
  int   dim,size;
  FILE  *fileOut;
  char  *fileName;
  int   alea;
  int   i;
  real  h_bro = 0.5, sigma;
  int   fft_dim = 0;
  char * h_fct_str;
  void *h_fct;
  int   isSaveFile;

  srand(time(NULL));
  alea = (int)(rand()/327.67); /* entre 0 et 99 */

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &fft_dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &alea)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &h_bro)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(4, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(5, &h_fct_str)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(6, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  isSaveFile = arg_present(7);
  if (isSaveFile) {
    if (arg_get(7, &fileName)==TCL_ERROR)
      return TCL_ERROR;    
  }

  /* check if dim is not large than 512 */
  if (dim>512) {
    return GenErrorAppend(interp, "Couldn't process data larger than 512^3 !!!", NULL);
  }

  size = dim*dim*dim;

  /* memory allocation */
  image = im3D_new(dim,dim,dim,size,PHYSICAL);

  if (!arg_present (5)) {
    h_fct_str = (char *) malloc (sizeof (char)*2);
    h_fct_str[0] = '1';
    h_fct_str[1] = '\0';
  }
  h_fct = evaluator_create(h_fct_str);
  if (!h_fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error",
			" in expression ", h_fct_str, (char *) NULL);
      return TCL_ERROR;
    }
  
  
  /* memory allocation is done in sub-routine */
  /*data = (float *) malloc(sizeof (float)*dim*dim*dim);
    if (!data)
    return GenErrorMemoryAlloc(interp);
    for (i=0;i<dim*dim*dim;i++)
    data[i]=0.0;
  */
  
  if (arg_present(4)) {
    /*im_construit_brownien3D_mid_point(fileOut, dim, h_bro, alea, sigma);*/
  } else {
    im_construit_brownien3D_fft(image->data, dim, h_bro, alea, fft_dim, h_fct);
  }
  
  /* do we save data into a file ?? */
  if (isSaveFile) {
    if (!(fileOut = fopen(fileName, "w")))
      return GenErrorAppend(interp, "Couldn't open `", fileName,
			    "' for writing.", NULL);
    /* write header */
    fprintf(fileOut, "Binary %d %dx%dx%d %d(%d byte reals)\n",
	    image->type, dim, dim, dim, size, (int) sizeof(real));
    
    /* write data */
    if (fwrite(image->data, sizeof(float), size, fileOut)!=size)
      return GenErrorAppend(interp, "Couldn't properly write `", fileName,
			    "'. Writing unsuccessful.", NULL);
    
    fclose(fileOut);
  }

  evaluator_destroy(h_fct);

  store_image3D(imageName, image);
  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}

/* *********************************
 * Command name in xsmurf : idirac3D
 * *********************************/
/* created on october the 28th 2002 by Pierre Kestener */
int
im_dirac3D_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  char * options[] = { "sd",
		       "-pos", "ddd",
		       NULL };

  char * help_msg =
    {("Create 3D dirac data and save it into a file\n"
      "(using fwrite for float data). First argument is file name and second\n"
      "is the size of the data (never try something larger than 512!!! )\n"
      " -pos [ddd] : position in 3D space of the Dirac\n"
      "\n")};

  int   dim;
  FILE  *fileOut;
  float *data;
  char  *fileName;
  int   i,j,k;
  int   posx,posy,posz;

  posx=posy=posz=0;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &fileName, &dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &posx,&posy,&posz)==TCL_ERROR)
    return TCL_ERROR;

  if (dim>256) {
    return GenErrorAppend(interp, "Couldn't process data larger than 256^3 !!!", NULL);
  }
  
  
  data = (float *) malloc(sizeof (float)*dim*dim*dim);
  if (!data)
    return GenErrorMemoryAlloc(interp);
  for (i=0;i<dim*dim*dim;i++)
    data[i]=0.0;


  if (!(fileOut = fopen(fileName, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName,
			  "' for writing.", NULL);
  
  for (i=0;i<2;i++)
    for (j=0;j<2;j++)
      for (k=0;k<2;k++)
	data[posx+i + (posy+j)*dim + (posz+k)*dim*dim] = 1000.0;
  
  fwrite(data, sizeof(float), dim*dim*dim, fileOut);
  
  free(data);
  fclose(fileOut);


  Tcl_AppendResult(interp,fileName,NULL);
  return TCL_OK;
}

/***********************************
 * Command name in xsmurf : itest3D
 ***********************************/
/* created on october the 28th 2002 by Pierre Kestener */
int
im_test3D_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  char * options[] = { "sdf",
		       "-posS", "ddd",
		       "-sigma" , "f",
		       "-coef" , "ff",
		       "-posG", "ddd",
		       NULL };

  char * help_msg =
    {("Create an Image3D data image to test the 3D methodology\n"
      "First argument is the name of the object created and the second\n"
      "is the size of the data (never try something larger than 512!!! )\n"
      "Parameter:\n"
      "   string  : filename of output\n"
      "   integer : N (size of data is then N^3)\n"
      "   float   : holder exponent of the singularity\n"
      "             must be between 0 and 1 !!!\n"
      "Options:\n"
      " -posS [ddd] : specify position of singularity S.\n"
      " -sigma [f]  : specify sigma of Gaussian.\n"
      " -coef [ff]  :\n"
      " -posG [ddd] : specify position of Gaussian G.\n"
      "\n")};

  int   dim;
  real  H,sigma;
  real  A,B;
  int   ds,dg;
  FILE  *fileOut;
  Image3D *image = NULL;
  float *data;
  char  *imageName;
  int   i,j,k;
  int   posx,posy,posz;
  int   pos_sing_x,pos_sing_y,pos_sing_z;
  int   pos_gaus_x,pos_gaus_y,pos_gaus_z;

  posx=posy=posz=0;
  A=-75.0;
  B=100.0;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &dim,&H)==TCL_ERROR)
    return TCL_ERROR;

  // sigma of the gaussian
  sigma = dim/4.0;

  // position of the |M-M0|^(-h) singularity
  pos_sing_y=pos_sing_z=pos_sing_x=1*dim/4;
  // position of the center of the gaussian
  //pos_gaus_y=pos_gaus_z=dim/2;
  pos_gaus_y=pos_gaus_z=3*dim/4;
  pos_gaus_x=pos_gaus_y;

  if (arg_present(1)) {
    if (arg_get(1, &pos_sing_x,&pos_sing_y,&pos_sing_z)==TCL_ERROR)
      return TCL_ERROR;
    /*printf("%d\n",pos_sing_x);*/
  }

  if (arg_get(2, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &A, &B)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(4, &pos_gaus_x,&pos_gaus_y,&pos_gaus_z)==TCL_ERROR)
    return TCL_ERROR;

  if (dim>256) {
    return GenErrorAppend(interp, "Couldn't process data larger than 256^3 !!!", NULL);
  }


  image = im3D_new(dim,dim,dim,dim*dim*dim,PHYSICAL);

  /*data = (float *) malloc(sizeof (float)*dim*dim*dim);*/
  data = image->data;
  /*if (!data)
    return GenErrorMemoryAlloc(interp);*/
  for (i=0;i<dim;i++) 
    for (j=0;j<dim;j++) 
      for (k=0;k<dim;k++) {
	ds = (i-pos_sing_x)*(i-pos_sing_x)+
	  (j-pos_sing_y)*(j-pos_sing_y)+
	  (k-pos_sing_z)*(k-pos_sing_z);
	dg = (i-pos_gaus_x)*(i-pos_gaus_x)+
	  (j-pos_gaus_y)*(j-pos_gaus_y)+
	  (k-pos_gaus_z)*(k-pos_gaus_z);
	
	data[i + j*dim + k*dim*dim] = (float) (A*pow(ds,H/2) \
					       + B*exp(-dg/2.0/sigma/sigma));
      }
  
  /*if (!(fileOut = fopen(fileName, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName,
    "' for writing.", NULL);
  
    fwrite(data, sizeof(float), dim*dim*dim, fileOut);
    free(data);
  
    fclose(fileOut);*/
  
  store_image3D(imageName, image);

  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}

/*****************************************
 * Command name in xsmurf : itest3Dvector
 *****************************************/
/* created on august the 8th 2003 by Pierre Kestener */
int
im_test3D_vector_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  char * options[] = { "sssdf",
		       "-pos", "ddd",
		       "-sigma" , "f",
		       "-coef" , "ff",
		       "-h", "ff",
		       NULL };

  char * help_msg =
    {("Create 3D vector data to test the 3D tensorial methodology and save\n"
      "it into a file (using fwrite for float data). First argument is file\n"
      "name and second is the size of the data (never try something larger\n"
      "than 512!!! )\n"
      "Parameter:\n"
      " 3 strings : filenames of the 3 outputs\n"
      "   integer : N (size of data is then N^3)\n"
      "   float   : holder exponent of the singularity\n"
      "             must be between 0 and 1 !!!\n"
      "Options:\n"
      " -pos [ddd] : no use up to now\n"
      " -sigma [f] : sigma of the Gaussian shape\n"
      " -coef [ff] : scalar multiplicative coefficients for the singularity\n"
      "              and the Gausian shape\n"
      " -h   [ff]  : specify 3 different holder exponent (one each component)\n"
      "\n")};

  int   dim;
  real  H=0.5,H2,H3,sigma;
  real  A,B;
  int   ds,dg;
  /*FILE  *fileOut1, *fileOut2, *fileOut3;*/
  Image3D *image1, *image2, *image3;
  float *data1, *data2, *data3;
  char  *imageName1, *imageName2, *imageName3;
  int   i,j,k;
  int   posx,posy,posz;
  int   pos_sing_x,pos_sing_y,pos_sing_z;
  int   pos_gaus_x,pos_gaus_y,pos_gaus_z;

  posx=posy=posz=0;
  A=-75.0;
  B=100.0;
  // sigma of the gaussian
  sigma = dim/4.0;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName1, &imageName2, &imageName3, &dim,&H)==TCL_ERROR)
    return TCL_ERROR;

  H2 = H;
  H3 = H;

  if (arg_get(1, &posx,&posy,&posz)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &A, &B)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(4, &H2, &H3)==TCL_ERROR)
    return TCL_ERROR;

  if (dim>256) {
    return GenErrorAppend(interp, "Couldn't process data larger than 256^3 !!!", NULL);
  }

  // position of the |M-M0|^(-h) singularity
  pos_sing_y=pos_sing_z=pos_sing_x=1*dim/4;
  // position of the center of the gaussian
  //pos_gaus_y=pos_gaus_z=dim/2;
  pos_gaus_y=pos_gaus_z=3*dim/4;
  pos_gaus_x=pos_gaus_y;

  image1 = im3D_new(dim,dim,dim,dim*dim*dim,PHYSICAL);
  image2 = im3D_new(dim,dim,dim,dim*dim*dim,PHYSICAL);
  image3 = im3D_new(dim,dim,dim,dim*dim*dim,PHYSICAL);
  
  /*data1 = (float *) malloc(sizeof (float)*dim*dim*dim);
    if (!data1)
    return GenErrorMemoryAlloc(interp);
    data2 = (float *) malloc(sizeof (float)*dim*dim*dim);
    if (!data2)
    return GenErrorMemoryAlloc(interp);
    data3 = (float *) malloc(sizeof (float)*dim*dim*dim);
    if (!data3)
    return GenErrorMemoryAlloc(interp);*/
  data1 = image1->data;
  data2 = image1->data;
  data3 = image1->data;
  for (i=0;i<dim;i++) 
    for (j=0;j<dim;j++) 
      for (k=0;k<dim;k++) {
	ds = (i-pos_sing_x)*(i-pos_sing_x)+
	  (j-pos_sing_y)*(j-pos_sing_y)+
	  (k-pos_sing_z)*(k-pos_sing_z);
	dg = (i-pos_gaus_x)*(i-pos_gaus_x)+
	  (j-pos_gaus_y)*(j-pos_gaus_y)+
	  (k-pos_gaus_z)*(k-pos_gaus_z);
	
	data1[i + j*dim + k*dim*dim] = A*pow(ds,H/2) \
	  + B*exp(-dg/2.0/sigma/sigma) ;
	data2[i + j*dim + k*dim*dim] = A*pow(ds,H2/2) \
	  + B*exp(-dg/2.0/sigma/sigma) ;
	data3[i + j*dim + k*dim*dim] = A*pow(ds,H3/2) \
	  + B*exp(-dg/2.0/sigma/sigma) ;
      }
  
  /*if (!(fileOut1 = fopen(fileName1, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName1,
    "' for writing.", NULL);
    if (!(fileOut2 = fopen(fileName2, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName2,
    "' for writing.", NULL);
    if (!(fileOut3 = fopen(fileName3, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName3,
    "' for writing.", NULL);
  
    fwrite(data1, sizeof(float), dim*dim*dim, fileOut1);
    fwrite(data2, sizeof(float), dim*dim*dim, fileOut2);
    fwrite(data3, sizeof(float), dim*dim*dim, fileOut3);
  
    free(data1);
    free(data2);
    free(data3);

    fclose(fileOut1);
    fclose(fileOut2);
    fclose(fileOut3);*/

  store_image3D(imageName1, image1);
  store_image3D(imageName2, image2);
  store_image3D(imageName3, image3);

  /*Tcl_AppendResult(interp,imageName1,NULL);*/
  return TCL_OK;
}

/************************************
 * Command name in xsmurf : igauss3D
 ************************************/
/* created on january the 22nd 2003 by Pierre Kestener */
int
im_gauss3D_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  char * options[] = { "sd",
		       "-pos", "ddd",
		       "-sigma" , "f",
		       "-coef" , "f",
		       NULL };

  char * help_msg =
    {("Create 3D Gaussian data to test the 3D methodology and save it into\n"
      " a file (using fwrite for float data). First argument is file name \n"
      " and second is the size of the data (never try something larger than \n"
      " 512!!! )\n"
      "Parameter:\n"
      "   string  : filename of output\n"
      "   integer : N (size of data is then N^3)\n"
      "Options:\n"
      " -pos [ddd] : set center of gaussian.\n"
      " -sigma [f] : set sigma  of gaussian.\n"
      " -coef  [f] : set some multiplicative scalar.\n"
      "\n")};

  int   dim;
  real  sigma;
  real  A;
  int   dg;
  FILE  *fileOut;
  float *data;
  char  *fileName;
  int   i,j,k;
  int   pos_gaus_x,pos_gaus_y,pos_gaus_z;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &fileName, &dim)==TCL_ERROR)
    return TCL_ERROR;

  A=1.0;
  // sigma of the gaussian
  sigma = dim/4.0;
  // position of the center of the gaussian
  //pos_gaus_y=pos_gaus_z=dim/2;
  pos_gaus_x=pos_gaus_y=pos_gaus_z=dim/2;


  if (arg_get(1, &pos_gaus_x,&pos_gaus_y,&pos_gaus_z)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &A)==TCL_ERROR)
    return TCL_ERROR;

  if (dim>256) {
    return GenErrorAppend(interp, "Couldn't process data larger than 256^3 !!!", NULL);
  }

    
  data = (float *) malloc(sizeof (float)*dim*dim*dim);
  if (!data)
    return GenErrorMemoryAlloc(interp);
  for (i=0;i<dim;i++) 
    for (j=0;j<dim;j++) 
      for (k=0;k<dim;k++) {
	dg = (i-pos_gaus_x)*(i-pos_gaus_x)+
	  (j-pos_gaus_y)*(j-pos_gaus_y)+
	  (k-pos_gaus_z)*(k-pos_gaus_z);
	
	data[i + j*dim + k*dim*dim] = A*exp(-dg/2.0/sigma/sigma) ;
      }
  
  if (!(fileOut = fopen(fileName, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName,
			  "' for writing.", NULL);
  
  fwrite(data, sizeof(float), dim*dim*dim, fileOut);
  
  free(data);
  fclose(fileOut);


  Tcl_AppendResult(interp,fileName,NULL);
  return TCL_OK;
}


/*************************************
 * Command name in xsmurf : isingul3D
 *************************************/
/* created on january the 22nd 2003 by Pierre Kestener */
/* modified (option -linear added) 
 * on september 28th  2004 by Pierre Kestener */
int
im_singul3D_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)
{ 
  char * options[] = { "sd",
		       "-pos"  , "ddd",
		       "-H"    , "f",
		       "-coef" , "f",
		       "-linear", "f",
		       NULL };

  char * help_msg =
    {("Create 3D data of punctual singularity to test the 3D methodology\n"
      " and save it into a file (using fwrite for float data). First \n"
      " argument is file name and second is the size of the data (never\n"
      " try something larger than 512!!! ).\n"
      "Parameter:\n"
      "   string  : filename of output\n"
      "   integer : N (size of data is then N^3)\n"
      "Options:\n"
      " -pos [ddd] : set position of the singularity.\n"
      " -H   [f]   : set H\"older exponent.\n"
      " -coef [f]  : set multiplicative scalar.\n" 
      " -linear [f] : replace the power law singularity by a linear one\n"
      "               parameter is the slope of decrease from the summit.\n"
      "\n")};

  int   dim;
  real  H;
  real  A;
  int   ds;
  FILE  *fileOut;
  float *data;
  char  *fileName;
  int   i,j,k;
  int   pos_singul_x,pos_singul_y,pos_singul_z;
  float slope;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &fileName, &dim)==TCL_ERROR)
    return TCL_ERROR;

  A = 1.0;
  // H of the sigularity
  H = 0.5;
  // position of the center of the singularity
  pos_singul_x=pos_singul_y=pos_singul_z=dim/2;

  slope = 0.0;

  if (arg_get(1, &pos_singul_x,&pos_singul_y,&pos_singul_z)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &H)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &A)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(4, &slope)==TCL_ERROR)
    return TCL_ERROR;

  if (dim>256) {
    return GenErrorAppend(interp, "Couldn't process data larger than 256^3 !!!", NULL);
  }

    
  data = (float *) malloc(sizeof (float)*dim*dim*dim);
  if (!data)
    return GenErrorMemoryAlloc(interp);
  if (arg_present(4)) {
    for (i=0;i<dim;i++) 
      for (j=0;j<dim;j++) 
	for (k=0;k<dim;k++) {
	  ds = (i-pos_singul_x)*(i-pos_singul_x)+
	    (j-pos_singul_y)*(j-pos_singul_y)+
	    (k-pos_singul_z)*(k-pos_singul_z);
	  
	  data[i + j*dim + k*dim*dim] = A + slope * ds;
	}
  } else {
    for (i=0;i<dim;i++) 
      for (j=0;j<dim;j++) 
	for (k=0;k<dim;k++) {
	  ds = (i-pos_singul_x)*(i-pos_singul_x)+
	    (j-pos_singul_y)*(j-pos_singul_y)+
	    (k-pos_singul_z)*(k-pos_singul_z);
	   
	  data[i + j*dim + k*dim*dim] = A*pow(ds,H/2.0) ;
	}
  }
    
  if (!(fileOut = fopen(fileName, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName,
			  "' for writing.", NULL);
  
  fwrite(data, sizeof(float), dim*dim*dim, fileOut);
  
  free(data);
  fclose(fileOut);


  Tcl_AppendResult(interp,fileName,NULL);
  return TCL_OK;
}


/************************************
 * Command name in xsmurf : ivortex
 ************************************/
/* created on June 19th 2003 */
int
im_vortex_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  char * options[] = { "ssd",
		       "-radius", "d",
		       "-v0", "f",
		       NULL };

  char * help_msg =
    {("Create two images (Vx,Vy) which are the components of the velocity\n"
      " field vector associated with a vortex de Rankin (nameX nameY size)\n"
      " -radius  size of the core of the vortex\n"
      " -v0      velocity amplitude at r=radius"
      "\n")};

  int    dim, i, j, ic1, jc1, ic2, jc2;
  Image *imageX, *imageY;
  char  *imageNameX, *imageNameY;
  int    rr1, rr2, radius2, radius = 5;
  double  alpha, theta1, theta2;
  float v0 = 1.0;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageNameX, &imageNameY, &dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &radius)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &v0)==TCL_ERROR)
    return TCL_ERROR;

  
  imageX=im_new(dim,dim,dim*dim,PHYSICAL);
  imageY=im_new(dim,dim,dim*dim,PHYSICAL);

  if (!imageX ||!imageY)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (imageX);
  im_set_0 (imageY);

  /* the vortex */
  ic1 = dim/4;
  jc1 = dim/4;
  ic2 = 3*dim/4;
  jc2 = 3*dim/4;
  radius2 = radius*radius;
  alpha = 1.0/radius2;
  for(i=0;i<dim;i++) 
    for(j=0;j<dim;j++) {
      rr1 = (i-ic1)*(i-ic1)+(j-jc1)*(j-jc1);
      rr2 = (i-ic2)*(i-ic2)+(j-jc2)*(j-jc2);
      theta1 = atan2(-i+ic1,-j+jc1);
      theta2 = atan2(-i+ic2,-j+jc2);
      
      if (rr1 == 0) {

	if (rr2 < radius2) {
	  imageX->data[i+j*dim] = -sin(theta2)*v0/radius*sqrt(rr2);
	  imageY->data[i+j*dim] =  cos(theta2)*v0/radius*sqrt(rr2);
	} else {
	  imageX->data[i+j*dim] = -sin(theta2)*v0*radius/sqrt(rr2);
	  imageY->data[i+j*dim] =  cos(theta2)*v0*radius/sqrt(rr2);	
	}

      } 
      
      if (rr2 == 0) {
	
	if (rr1 < radius2) {
	  imageX->data[i+j*dim] = -sin(theta1)*v0/radius*sqrt(rr1);
	  imageY->data[i+j*dim] =  cos(theta1)*v0/radius*sqrt(rr1);
	} else {
	  imageX->data[i+j*dim] = -sin(theta1)*v0*radius/sqrt(rr1);
	  imageY->data[i+j*dim] =  cos(theta1)*v0*radius/sqrt(rr1);
	}
      }
	
      if (rr1 < radius2 && rr2 < radius2) {

	imageX->data[i+j*dim] = -sin(theta1)*v0/radius*sqrt(rr1)
	  -sin(theta2)*v0/radius*sqrt(rr2);
	imageY->data[i+j*dim] =  cos(theta1)*v0/radius*sqrt(rr1)
	  +cos(theta2)*v0/radius*sqrt(rr2);
	  
      } else if (rr1 >= radius2 && rr2 >= radius2) {
	
	imageX->data[i+j*dim] = -sin(theta1)*v0*radius/sqrt(rr1)
	  -sin(theta2)*v0*radius/sqrt(rr2);
	imageY->data[i+j*dim] =  cos(theta1)*v0*radius/sqrt(rr1)
	  +cos(theta2)*v0*radius/sqrt(rr2);
	
      } else if (rr1 < radius2 && rr2 >= radius2) {
	
	imageX->data[i+j*dim] = -sin(theta1)*v0/radius*sqrt(rr1)
	  -sin(theta2)*v0*radius/sqrt(rr2);
	imageY->data[i+j*dim] =  cos(theta1)*v0/radius*sqrt(rr1)
	  +cos(theta2)*v0*radius/sqrt(rr2);
	
      } else {
	
	imageX->data[i+j*dim] = -sin(theta1)*v0*radius/sqrt(rr1)
	  -sin(theta2)*v0/radius*sqrt(rr2);
	imageY->data[i+j*dim] =  cos(theta1)*v0*radius/sqrt(rr1)
	  +cos(theta2)*v0/radius*sqrt(rr2);
	
      }
    }


  store_image(imageNameX,imageX);
  store_image(imageNameY,imageY);
  
  return TCL_OK;
}

/********************************************
 * Command name in xsmurf : ivectorcascade2D
 ********************************************/
/* created on June 28th 2003 */
int
im_vector_cascade_2D_TclCmd_ (ClientData clientData,
			      Tcl_Interp *interp,
			      int        argc,
			      char       **argv)
{ 
  char * options[] = { "ssd",
		       "-radius", "d",
		       NULL };

  char * help_msg =
    {("Create two images (Mx,My) which are the components of the vector-valued\n"
      " mesure associated with cascade process as in reference\n"
      "\"Vector-valued multifractal measures\" of Falconer and O'Neil,\n"
      "Proc. Lond. Math. Soc., 73 (1998), 68\n"
      " (nameX nameY size)\n"
      "NOT IMPLEMENTED !!!\n"
      "Options:\n"
      "\n")};

  int    dim, i, j, ic, jc;
  Image *imageX, *imageY;
  char  *imageNameX, *imageNameY;
  int    r2, radius2, radius = 5;
  float  alpha, theta;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageNameX, &imageNameY, &dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &radius)==TCL_ERROR)
    return TCL_ERROR;

  
  imageX=im_new(dim,dim,dim*dim,PHYSICAL);
  imageY=im_new(dim,dim,dim*dim,PHYSICAL);

  if (!imageX ||!imageY)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (imageX);
  im_set_0 (imageY);


  store_image(imageNameX,imageX);
  store_image(imageNameY,imageY);
  
  return TCL_OK;
}

/**************************************
 * Command name in xsmurf : iUrand
 **************************************/
/* added on march 21st 2002*/
/* peut-etre redondant avec
   la commande iwhite
   a regarder une autre fois */
int
im_Urand_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  char * options[] = { "sd[d]",
		       NULL };

  char * help_msg =
    {("Create a Uniform Random image\n"
      "\n")};
  
  int   lx,ly=0;
  int   i,j;
  Image *image;
  char  *imageName;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &lx, &ly)==TCL_ERROR)
    return TCL_ERROR;

  if (ly==0) ly=lx;
  image=im_new(lx,ly,lx*ly,PHYSICAL);

  if (!image)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (image);


  for(i=0;i<lx;i++) 
    for(j=0;j<ly;j++)
      image->data[i+j*lx] = Urand();

  store_image(imageName,image);

  Tcl_AppendResult(interp,imageName,NULL);
  
  return TCL_OK;
}

/* *************************************
 * Command name in xsmurf : iGrand
 * *************************************/
/* added on march 21st 2002*/
int
im_Grand_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  char * options[] = { "sd[d]",
		       NULL };

  char * help_msg =
    {("Create a Gaussian Random image\n"
      "mean value : 0\n"
      "sigma      : 1\n")};
  
  int   lx,ly=0;
  int   i,j;
  Image *image;
  char  *imageName;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &lx, &ly)==TCL_ERROR)
    return TCL_ERROR;

  if (ly==0) ly=lx;
  image=im_new(lx,ly,lx*ly,PHYSICAL);

  if (!image)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (image);


  for(i=0;i<lx;i++) 
    for(j=0;j<ly;j++)
      image->data[i+j*lx] = Grand(1.0);

  store_image(imageName,image);

  Tcl_AppendResult(interp,imageName,NULL);
  
  return TCL_OK;
}

/* *************************************
 * Command name in xsmurf : i3DGrand
 * *************************************/
/* added on may 29th 2003*/
int
im3D_Grand_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  char * options[] = { "sd[dd]",
		       NULL };
  
  char * help_msg =
    {("Create a Gaussian Random 3D image and save it in a file.\n"
      "mean value : 0\n"
      "sigma      : 1\n"
      "Parameters:\n"
      "   string : filename\n"
      "   integer: linear size along x (which is used as default\n"
      "            value for sizes along y and z-axis).\n"
      "\n"
      "Optional parameters:\n"
      "   integer: y-size\n"
      "   integer: z-size\n")};
  
  int   lx,ly=0,lz=0;
  int   i,j,k;
  float *data;
  char  *fileName;
  FILE  *fileOut;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &fileName, &lx, &ly, &lz)==TCL_ERROR)
    return TCL_ERROR;

  if (ly==0) ly=lx;
  if (lz==0) lz=lx;

  if (lx*ly*lz>512*512*512) {
    return GenErrorAppend(interp, "Couldn't process data too large > 512^3 !!!", NULL);
  }
  
  data = (float *) malloc(sizeof (float)*lx*ly*lz);

  if (!data) 
    return GenErrorMemoryAlloc(interp);

  for(i=0;i<lx;i++) 
    for(j=0;j<ly;j++)
      for(k=0;k<lz;k++)
	data[i+j*lx+k*lx*ly] = Grand(1.0);

  if (!(fileOut = fopen(fileName, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName,
			  "' for writing.", NULL);
  
  fwrite(data, sizeof(float), lx*ly*lz , fileOut);


  fclose(fileOut);
  free(data);

  Tcl_AppendResult(interp,fileName,NULL);
  
  return TCL_OK;
}

/*******************************************
 * Command name in xsmurf : i3DvectorGrand
 *******************************************/
/* added on august 11th 2003*/
int
im3D_vector_Grand_TclCmd_ (ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)
{ 
  char * options[] = { "sssd[dd]",
		       NULL };
  
  char * help_msg =
    {("Create a 3D Gaussian Random vector field.\n"
      "mean value : 0\n"
      "sigma      : 1\n"
      "Parameters:\n"
      "   string : filename for x-component\n"
      "   string : filename for y-component\n"
      "   string : filename for z-component\n"
      "   integer: linear size along x (which is used as default\n"
      "            value for sizes along y and z-axis).\n"
      "\n"
      "Optional parameters:\n"
      "   integer: y-size\n"
      "   integer: z-size\n")};
  
  int   lx,ly=0,lz=0;
  int   i,j,k;
  float radius, theta, phi;
  float *data1,*data2,*data3;
  char  *fileName1,*fileName2,*fileName3;
  FILE  *fileOut1,*fileOut2,*fileOut3;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &fileName1, &fileName2, &fileName3, &lx, &ly, &lz)==TCL_ERROR)
    return TCL_ERROR;

  if (ly==0) ly=lx;
  if (lz==0) lz=lx;

  if (lx*ly*lz>512*512*512) {
    return GenErrorAppend(interp, "Couldn't process data too large > 512^3 !!!", NULL);
  }
  
  data1 = (float *) malloc(sizeof (float)*lx*ly*lz);
  data2 = (float *) malloc(sizeof (float)*lx*ly*lz);
  data3 = (float *) malloc(sizeof (float)*lx*ly*lz);

  if (!data1) 
    return GenErrorMemoryAlloc(interp);
  if (!data2) 
    return GenErrorMemoryAlloc(interp);
  if (!data3) 
    return GenErrorMemoryAlloc(interp);

  for(i=0;i<lx;i++) 
    for(j=0;j<ly;j++)
      for(k=0;k<lz;k++) {
	//radius = Grand(1.0);
	radius = Urand();
	theta = 2.0*asin(1.0)*Urand();
	phi = 4.0*asin(1.0)*Urand();
	data1[i+j*lx+k*lx*ly] = radius*sin(theta)*cos(phi);
	data2[i+j*lx+k*lx*ly] = radius*sin(theta)*sin(phi);
	data3[i+j*lx+k*lx*ly] = radius*cos(theta);
      }

  if (!(fileOut1 = fopen(fileName1, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName1,
			  "' for writing.", NULL);
  if (!(fileOut2 = fopen(fileName2, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName2,
			  "' for writing.", NULL);
  if (!(fileOut3 = fopen(fileName3, "w")))
    return GenErrorAppend(interp, "Couldn't open `", fileName3,
			  "' for writing.", NULL);
  
  fwrite(data1, sizeof(float), lx*ly*lz , fileOut1);
  fwrite(data2, sizeof(float), lx*ly*lz , fileOut2);
  fwrite(data3, sizeof(float), lx*ly*lz , fileOut3);


  fclose(fileOut1);
  free(data1);
  fclose(fileOut2);
  free(data2);
  fclose(fileOut3);
  free(data3);

  Tcl_AppendResult(interp,fileName1,NULL);
  
  return TCL_OK;
}

/**************************************
 * Command name in xsmurf : iprim
 **************************************/
/* added on march 21st 2002*/
int
im_Prim_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{ 
  char * options[] = { "Is",
		       NULL };

  char * help_msg =
    {("Integrate image\n"
      "\n")};
  
  int   lx,ly=0;
  int   i,j;
  Image *im_in, *im_out;
  char  *imageName;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &im_in, &imageName)==TCL_ERROR)
    return TCL_ERROR;
  
  lx = im_in->lx;
  ly = im_in->ly;

  im_out=im_new(lx,ly,lx*ly,PHYSICAL);

  if (!im_out)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (im_out);

  /* initialization for i=0 */
  for (j=0;j<ly;j++) im_out->data[j*lx] = im_in->data[j*lx];
  /* first pass */
  for(i=1;i<lx;i++) 
    for(j=0;j<ly;j++)
      im_out->data[i+j*lx] = im_in->data[i+j*lx] + im_out->data[i-1+j*lx]; 

  /* initializationfor i=0 once again */
  for (j=1;j<ly;j++) 
    im_out->data[j*lx] += im_out->data[(j-1)*lx];
  
  /* second pass */
  for(i=1;i<lx;i++) 
    for(j=1;j<ly;j++)
      im_out->data[i+j*lx] += im_out->data[i+(j-1)*lx]; 

  store_image(imageName,im_out);

  Tcl_AppendResult(interp,imageName,NULL);
  
  return TCL_OK;
}


/*
 * on garde ??
 */
int
im_dla_TclCmd_ (ClientData clientData,
		Tcl_Interp *interp,
		int        argc,
		char       **argv)
{ 
  char * options[] = { "sd",
		       "-border","d",
		       NULL };
  
  char * help_msg =
    {("read an agregat DLA (name size)\n"
      " -border create a border around the DLA.")};
  
  int dim, size, border = 0;
  int taille_amas;
  Image * image;
  char * imageName;
  char * nom_fichier="dla6.dat";
  real angle = 0;
  double P_PI=3.14;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0,&imageName,&dim,&taille_amas)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1,&border)==TCL_ERROR)
    return TCL_ERROR;
  
  if ((dim<0) || (border<0))
    return GenErrorAppend(interp,"Incorrect values.",NULL);

  angle *= P_PI/180;	

  size = dim + 2*border;
  image=im_new(size,size,size*size,PHYSICAL);

  if (!image)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (image);
  
  im_Lit_dla_ (image->data,nom_fichier,size,size,angle);  
  
  store_image(imageName,image);

  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}

/*************************************
 * Command name in xsmurf : iwhite 
 *************************************/
int
im_white_noise_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  char * options[] = { "sd",
		       "-alea", "d",
		       "-sigma", "f",
		       "-type", "d",
		       NULL };
  
  char * help_msg =
    {("Create a white noise image (exponent h=0.5) (name size)\n"
      " -alea   integer between (0..99) ?????\n"
      " -sigma  standart deviation of the Gaussian law (default=1)\n"
      " -type   0 random law ???\n"
      "         1 random law ???\n"
      "         2 Gaussian law (default)\n"
      "         3 exponential law.")};
    
  int   dim;
  Image *image;
  char  *imageName;
  int   alea;
  int   type = 2; /* = GAUSS. C'est crade, a changer... */
  real sigma = 1;

  srand(time(NULL));
  alea = (int)(rand()/327.67); /* entre 0 et 99 */

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &alea)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &type)==TCL_ERROR)
    return TCL_ERROR;

  image = im_new (dim, dim, dim*dim, PHYSICAL);

  if (!image)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (image);
  
  im_bruit_blanc_(image->data, dim, sigma, type, alea);

  store_image(imageName,image);

  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}

/*------------------------------------------------------------------------
  ImaCrtSnowFlakeCmd_

  Commande Tcl permettant de creer l'image d'un flocon de neige mathematique
  ----------------------------------------------------------------------*/
int
ImaSnfSnowFlakeCmd_(ClientData clientData,
		    Tcl_Interp *interp,
		    int argc,
		    char **argv)      
{ 
  char  * options[] = { "sd",
			"-border","d",
			NULL };
  char * help_msg =
    {("Create an mathematical snowflake (?!?!?!) (name size)\n"
      " -border create a border around the snowflake.")};

  int   dim, size, border = 0;
  Image * image;
  char  * imageName;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &dim) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &border) == TCL_ERROR)
    return TCL_ERROR;
  
  if ((dim < 0) || (border < 0))
    return GenErrorAppend(interp, "Incorrect values.", NULL);

  size = dim+2*border;
  image=im_new(size, size, size*size, PHYSICAL);

  if (!image)

    return GenErrorMemoryAlloc(interp);
  im_set_0 (image);
  
  
  _SFlake_(image->data, size,
	   (real)border, (real)(border+dim),
	   (real)border, (real)(border+dim),
	   100.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
  store_image(imageName, image);

  Tcl_AppendResult(interp, imageName, NULL);
  return TCL_OK;
}

/*------------------------------------------------------------------------
  ImaCrtSuperSnowFlakeCmd_

  Commande Tcl permettant de creer l'image d'un flocon
  ----------------------------------------------------------------------*/
int 
ImaSnfSuperSnowFlakeCmd_(ClientData clientData,
			 Tcl_Interp *interp,
			 int argc,
			 char **argv)      
{ 
  char * options[] = { "sd",
		       "-border","d",
		       "-etape", "d",
		       NULL };
  
  char * help_msg =
    {("Create an mathematical snowflake (name size)\n"
      " -border create a border around the snowflake\n"
      " -etape number of step.")};
  
  int dim, size, border = 0;
  Image * image;
  char * imageName;
  int nb;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0,&imageName,&dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1,&border)==TCL_ERROR)
    return TCL_ERROR;

  nb = (int) (log2 ((double) (dim)) / 2);

  if (arg_present (2))
    if (arg_get(2, &nb)==TCL_ERROR)
      return TCL_ERROR;
  
  if ((dim<0) || (border<0))
    return GenErrorAppend(interp,"Incorrect values.",NULL);

  size = dim + 2*border;
  image=im_new(size,size,size*size,PHYSICAL);

  if (!image)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (image);
  
  fl2(image->data,size,(real) (border),(real)(border), 
      ((real) dim)/ 4., ((real) dim) / 4., 255.0, nb);


  /*  _SFlake_(image->data,size,
      (real)border,(real)(border+dim),(real)border,(real)(border+dim),
      100.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0);*/
  store_image(imageName,image);

  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}

/* a mettre ailleurs (bordel de merde ! */
#define FFT_ERROR 10e-6

/*------------------------------------------------------------------------
  ImaTopCopyCmd_

  Commande Tcl permettant de copier une image dans une autre.
  La deuxieme image donnee en argument est detruite puis recreee.
  ----------------------------------------------------------------------*/
/***************************************
 * Command name in xsmurf : icopy
 ***************************************/
int ImaTopCopyCmd_(ClientData clientData,Tcl_Interp *interp,
		   int argc,char **argv)      
{
  char * options[] = { "Is", NULL };

  char * help_msg =
    {("copy image1 in image2 (image1, image2).")};

  Image * srcImagePtr;
  char * dst_imageName;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0,&srcImagePtr,&dst_imageName)==TCL_ERROR)
    return TCL_ERROR;
  
  store_image(dst_imageName,im_duplicate (srcImagePtr));
  
  return TCL_OK;
}

/***************************************
 * Command name in xsmurf : i3Dcopy
 ***************************************/
int Ima3DTopCopyCmd_(ClientData clientData,Tcl_Interp *interp,
		     int argc,char **argv)      
{
  char * options[] = { "Js", NULL };

  char * help_msg =
    {("copy image1 in image2 (image1, image2).")};

  Image3D * srcImagePtr;
  char * dst_imageName;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0,&srcImagePtr,&dst_imageName)==TCL_ERROR)
    return TCL_ERROR;
  
  store_image3D(dst_imageName,im3D_duplicate (srcImagePtr));
  
  return TCL_OK;
}

/*------------------------------------------------------------------------
  command name in xsmurf : value or igetvalue
  modified : 19/03/2001 (otion -user)
  ImaTopGetValueCmd

  Affiche la valeur d'un point d'une image
  ----------------------------------------------------------------------*/
int ImaTopGetValueCmd_(ClientData clientData,Tcl_Interp *interp,
		       int argc,char **argv)      
{
  char * options[] = { "I[dd]",
		       "-all", "",
		       NULL };

  char * help_msg =
    {("Give the value of the point x y of the image (image [x y]).\n"
      "Default value for x,y is 0,0.\n"
      "Options:\n"
      "  -all : Give all value\n")};

  /* Options's presence */
  int isAll;
  
  Image * image;
  real min,max;
  int x=0,y=0;
  int k,l;
  char val_str[30];
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;

  if (arg_get(0,&image,&x,&y)==TCL_ERROR)
    return TCL_ERROR;
  
  isAll = arg_present(1);
  
  im_get_extrema (image,&min,&max);

  if (isAll) {
    for (k=0; k < image->lx; k++) {
      for (l=0; l < image->ly; l++) {
	sprintf(val_str,"%g",image->data[k+l*image->lx]);
	Tcl_AppendElement(interp, val_str);
      }
    }
  } else {
    sprintf(interp->result,"%g",image->data[x+y*image->lx]);
  }

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ImaTopGetSizeCmd

  Affiche la valeur d'un point d'une image
  ----------------------------------------------------------------------*/
int ImaTopGetSizeCmd_(ClientData clientData,Tcl_Interp *interp,
		      int argc,char **argv)      
{
  char * options[] = { "I",
		       NULL };

  char * help_msg =
    {("Give the size of the image in the direction of x and y (image).")};

  Image * image;
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0,&image)==TCL_ERROR)
    return TCL_ERROR;
 
  sprintf(interp->result,"%d", image->lx);
  return TCL_OK;
}

/***************************************
 * Command name in xsmurf : im_extrema
 ***************************************/
int ImaTopGetminmaxCmd_(ClientData clientData,Tcl_Interp *interp,
			int argc,char **argv)      
{
  char * options[] = { "I", NULL };

  char * help_msg =
    {("Give the min and the max value of image (image).")};

  Image * image;
  real min,max;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;

  if (arg_get(0,&image)==TCL_ERROR)
    return TCL_ERROR;
  
  im_get_extrema (image,&min,&max);
  /*  Tcl_AppendResult(interp, image->min, image->max, (char *) NULL);*/
  sprintf(interp->result,"%g %g", image->min, image->max);

  return TCL_OK;
}

/***************************************
 * Command name in xsmurf : im_average
 ***************************************/
int ImaTopGetavgCmd_(ClientData clientData,Tcl_Interp *interp,
		     int argc,char **argv)      
{
  char * options[] = { "I", NULL };

  char * help_msg =
    {("Give the average value of image (image).")};

  Image * image;
  real avg;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;

  if (arg_get(0,&image)==TCL_ERROR)
    return TCL_ERROR;
  
  im_get_average (image,&avg);
  /*  Tcl_AppendResult(interp, image->min, image->max, (char *) NULL);*/
  sprintf(interp->result,"%g", image->avg);

  return TCL_OK;
}




/****************************************
 * Command name in xsmurf : i3D_extrema
 ****************************************/
int Ima3DTopGetminmaxCmd_(ClientData clientData,Tcl_Interp *interp,
			  int argc,char **argv)      
{
  char * options[] = { "J", NULL };
  
  char * help_msg =
    {("Give the min and the max value of image3D (image3D).")};

  Image3D * image;
  real min,max;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;

  if (arg_get(0,&image)==TCL_ERROR)
    return TCL_ERROR;
  
  im3D_get_extrema (image,&min,&max);
  /*  Tcl_AppendResult(interp, image->min, image->max, (char *) NULL);*/
  sprintf(interp->result,"%g %g", image->min, image->max);

  return TCL_OK;
}

/************************************
 * Command name in xsmurf : igetlx
 ************************************/
int ImaGetlxCmd_(ClientData clientData,Tcl_Interp *interp,
		 int argc,char **argv)      
{
  char * options[] = { "I", NULL };

  char * help_msg =
    {("Give the x-dimension of image (image).")};

  Image * image;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;

  if (arg_get(0,&image)==TCL_ERROR)
    return TCL_ERROR;
  
  sprintf(interp->result,"%d", image->lx);

  return TCL_OK;
}

/* **************************************
 * Command name in xsmurf : igetly
 * **************************************/
int ImaGetlyCmd_(ClientData clientData,Tcl_Interp *interp,
		 int argc,char **argv)      
{
  char * options[] = { "I", NULL };

  char * help_msg =
    {("Give the y-dimension of image (image).")};

  Image * image;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;

  if (arg_get(0,&image)==TCL_ERROR)
    return TCL_ERROR;
  
  sprintf(interp->result,"%d", image->ly);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
ImaTopFftFilterCmd_(ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)      
{
  char * options[] = { "I", NULL };
  
  char * help_msg =
    {("Thresh the image value : put all the value inferior to 1e-6 to 0.")};

  int i;
  Image * image;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;

  if (arg_get(0,&image)==TCL_ERROR)
    return TCL_ERROR;
  
  for (i = 0; i < image->size; i++)
    if (fabs(image->data[i]) < FFT_ERROR)
      image->data[i] = 0;

  return TCL_OK;
}

enum {
  INT_FRMT,
  REAL_FRMT,
  CHAR_FRMT
};

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int 
ImaNuaLoadCmd_(ClientData clientData,Tcl_Interp *interp,
	       int argc,char **argv)      
{ 
  char * options[] = { "ssd",        /* Nom, dimension de l'image resultat */
		       "-pas", "d",  /* Pas entre les diff. pixels a		                                       prendre dans l'image d'origine    */
		       "-nrow", "d", /* Dimension de l'image d'origine     */
		       "-pos", "dd", /* Position de l'image resultat dans
		                        dans l'image d'origine           */
		       "-file", "s",
		       "-int", "",
		       "-newformat", "",
		       NULL };
  
  char * help_msg =
    {("read the cloud filename (filename name size)\n"
      " -pas   step between the pixels read\n"
      " -nrow  dimension of the original cloud\n"
      " -pos   position of the cloud read in the original cloud\n"
      " -file  filename.\n"
      " -int   je ne sais pas ce que fait cette option...\n"
      " -newformat read the new format cloud image.")};
  
  int   dim, new_dim;
  Image *image;
  char  *imageName, file_name[200], *nu_name;
  /* a foutre ailleur */
  char  *file_name2 = "";
  int   pas = 1;
  int   nrow = 4096;
  int   x0 = 0, y0 = 0;
  int   format = CHAR_FRMT;
  int is_new_format;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &nu_name, &dim) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &pas) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &nrow) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &x0, &y0) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(4, &file_name) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_present(5)) {
    format = INT_FRMT;
  }

  is_new_format=arg_present(6);

  sprintf(file_name,"%s%s",file_name2,nu_name);

  new_dim = dim/pas;
  image=im_new(new_dim,new_dim,new_dim*new_dim,PHYSICAL);
  if (!image)
    return GenErrorMemoryAlloc(interp);
  im_set_0(image);

  _ImaNuaLoadFunction_(image, file_name,
		       dim,nrow,x0,y0, 0, pas, 0, is_new_format); 

  store_image(imageName,image);
  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}

/*------------------------------------------------------------------------

  ----------------------------------------------------------------------*/
int 
ImaSimNuaLoadCmd_(ClientData clientData,Tcl_Interp *interp,
		  int argc,char **argv)      
{ 
  char * options[] = { "ssd",    /* Nom du fichier, nom et dimension de
		                    l'image resultat.                */
		       NULL };
  char * help_msg =
    {("Read a simulation of a cloud (filename name size).")};
  
  int   dim;
  Image *image;
  char  *fileName;
  char  *imageName;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &fileName, &imageName, &dim) == TCL_ERROR)
    return TCL_ERROR;

  image=im_new(dim, dim, dim*dim, PHYSICAL);
  if (!image)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (image);

  _ImaSimNuaLoadFunction_(image, fileName, dim);

  store_image(imageName,image);
  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}
/*
  static double
  _gauss_r_ (double v,
  double w)
  {
  return exp (-v*v-w*w);
  }

  static double
  _dx_gauss_i_ (double v,
  double w)
  {
  return v*exp (-v*v-w*w);
  }

  static double
  _dy_gauss_i_ (double v,
  double w)
  {
  return w*exp (-v*v-w*w);
  }

  static double
  _dxx_gauss_r_ (double v,
  double w)
  {
  return -v*v*exp (-v*v-w*w);
  }

  static double
  _dxy_gauss_r_ (double v,
  double w)
  {
  return -v*w*exp (-v*v-w*w);
  }

  static double
  _dyy_gauss_r_ (double v,
  double w)
  {
  return -w*w*exp (-v*v-w*w);
  }

  static double
  _dxxx_gauss_i_ (double v,
  double w)
  {
  return -v*v*v*exp (-v*v-w*w);
  }

  static double
  _dxxy_gauss_i_ (double v,
  double w)
  {
  return -v*v*w*exp (-v*v-w*w);
  }

  static double
  _dxyy_gauss_i_ (double v,
  double w)
  {
  return -v*w*w*exp (-v*v-w*w);
  }

  static double
  _dyyy_gauss_i_ (double v,
  double w)
  {
  return -w*w*w*exp (-v*v-w*w);
  }

  static double
  _null_ (double v,
  double w)
  {
  return 0;
  }
*/


/**************************************
 * Command name in xsmurf : iconvol
 **************************************/
int ImaCvlConvolutionCmd_(ClientData clientData,
			  Tcl_Interp *interp,
			  int argc,
			  char **argv)      
{
  char * options[] = {"Isf",
		      "-dirac", "",
		      "-new", "ss",
		      "-widefield", "ss",
		      NULL };

  char * help_msg =
    {("Compute the convolution between the image and the filter at one scale (image, filter, scale)\n"
      " the filter can be   gauss mexican\n"
      "                     dx, dy\n"
      "                     dxx, dyy,dxy\n"
      "                     dxxx,dxxy,dxyy,dyyy.\n"
      " -dirac:     take the derivative of a dirac and not of a gaussian.\n"
      " -new:       filters according to the user-defined real and imaginary. \n"
      " -widefield: filters according to the psf of the widefield microscope \n"
      "             (use after creating simulated images with icell). \n")};
    
  Image *image;
  real  Scale;
  char  *waveType;
  int   is_dirac;
  int   is_new;
  int   is_widefield;
  char  *fct_r_str, *fct_i_str;
  void *fct_r, *fct_i;
  void *wide_fct_r, *wide_fct_i;
    
  /* image type_ondelette echelle */
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0,&image,&waveType,&Scale)==TCL_ERROR)
    return TCL_ERROR;  

  is_dirac = arg_present (1);
  is_new = arg_present (2);
  is_widefield = arg_present (3);
    
  if (is_new)
    {
      if (arg_get (2, &fct_r_str, &fct_i_str) == TCL_ERROR)
	return TCL_ERROR;
      fct_r = evaluator_create(fct_r_str);
      if (!fct_r)
	{
	  Tcl_AppendResult (interp, "libmatheval : error",
			    " in expression ", fct_r_str, (char *) NULL);
	  return TCL_ERROR;
	}
      fct_i = evaluator_create(fct_i_str);      
      if (!fct_i)
	{
	  Tcl_AppendResult (interp, "libmatheval : error",
			    " in expression ", fct_i_str, (char *) NULL);
	  return TCL_ERROR;
	}
    }

  if (!is_new && image->type!=FOURIER)
    return GenErrorAppend(interp,"`",argv[1],"' has not FOURIER type.",NULL);
 
  if (is_new) {
    image = im_mult_analog (image, Scale, 1, fct_r, fct_i);
    evaluator_destroy(fct_r);
    evaluator_destroy(fct_i);
  } else if (is_widefield) {
    if (arg_get (3, &fct_r_str, &fct_i_str) == TCL_ERROR)
      return TCL_ERROR;
    wide_fct_r = evaluator_create (fct_r_str);
    if (!wide_fct_r) {
      Tcl_AppendResult (interp, "libmatheval : error",
			" in expression ", fct_r_str, (char *) NULL);
      return TCL_ERROR;
    }
    wide_fct_i = evaluator_create (fct_i_str);
    if (!wide_fct_i) {
      Tcl_AppendResult (interp, "libmatheval : error",
			" in expression ", fct_i_str, (char *) NULL);
      return TCL_ERROR;
    }
    image = im_mult_analog (image, Scale, 1, wide_fct_r, wide_fct_i);
    evaluator_destroy(wide_fct_r);
    evaluator_destroy(wide_fct_i);
  } else {
    image = im_fourier_conv_ (image, waveType, Scale, is_dirac);
  }

  return TCL_OK;
}

/****************************************
 * Command name in xsmurf : ifftwfilter
 ****************************************/
int ImaFftwFilterCmd_(ClientData clientData,Tcl_Interp *interp,
		      int argc,char **argv)      
{
  char * options[] = {"Ifss",
		      NULL };

  char * help_msg =
    {("Compute the convolution between the image and an analytical filter at one scale (image, scale, expr1, expr2).\n"
      "Input must be an image of type FFTW_R2C (output of ifftw2d command)\n"
      "\n"
      "This routines performs the multiplication of the Fourier coefficients\n"
      "by an analytical filter as defined by two expressions (real and\n"
      "imaginary part).\n")};
  
  Image *image;
  real  Scale;
  char  *fct_r_str, *fct_i_str;
  /*double (*fct_r)();
    double (*fct_i)();*/
  void *fct_r, *fct_i;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0,&image,&Scale,&fct_r_str, &fct_i_str)==TCL_ERROR)
    return TCL_ERROR;  

  /* check that input image has the correct type */
  if (image->type!=FFTW_R2C)
    return GenErrorAppend(interp,"`",argv[1],"' has not FFTW_R2C type.",NULL);

  /* open filter expressions */
  /*fct_r = dfopen (fct_r_str);*/
  fct_r = evaluator_create(fct_r_str);
  if (!fct_r)
    {
      Tcl_AppendResult (interp, "libmatheval : error",
			" in expression ", fct_r_str, (char *) NULL);
      return TCL_ERROR;
    }
  /*fct_i = dfopen (fct_i_str);*/
  fct_i = evaluator_create(fct_i_str);
  if (!fct_i)
    {
      Tcl_AppendResult (interp, "libmatheval : error",
			" in expression ", fct_i_str, (char *) NULL);
      return TCL_ERROR;
    }

  /* computation */
  im_fftw_filter (image, Scale, fct_r, fct_i);
 
  /* close filter expressions */
  /*dfclose (fct_r);
    dfclose (fct_i);*/
  evaluator_destroy(fct_r);
  evaluator_destroy(fct_i);

  return TCL_OK;
}

/****************************************
 * Command name in xsmurf : ifftw3dfilter
 ****************************************/
int ImaFftw3DFilterCmd_(ClientData clientData,Tcl_Interp *interp,
			int argc,char **argv)      
{
  char * options[] = {"Jf[ss]",
		      "-gaussian","",
		      "-mexican","",
		      "-x","",
		      "-y","",
		      "-z","",
		      NULL };

  char * help_msg =
    {("Compute the convolution between the image and an analytical filter at one scale (image, scale).\n"
      "Input must be a 3D image of type FFTW_R2C (output of ifftw3d command)\n"
      "\n"
      "This routines performs the multiplication of the Fourier coefficients\n"
      "by an analytical filter corresponding to one of the gaussian/mexican\n"
      "partial derivative filter).\n")};
  
  /* Command's parameters */
  Image3D *image;
  real  Scale;
  void *fct_r, *fct_i;
  
  /* Options's presence */
  int isGaussian=1;
  int isMexican=0;
  int isX=1, isY, isZ;

  /* other parameters */
  char  *fct_r_str, *fct_i_str;

  /* Command line analysis */
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0,&image,&Scale,&fct_r_str, &fct_i_str)==TCL_ERROR)
    return TCL_ERROR;  

  isGaussian = arg_present(1);
  isMexican  = arg_present(2);
  isX        = arg_present(3);
  isY        = arg_present(4);
  isZ        = arg_present(5);

  /* check that input image has the correct type */
  if (image->type!=FFTW_R2C)
    return GenErrorAppend(interp,"`",argv[1],"' has not FFTW_R2C type.",NULL);

  /* open filter expressions (if necessary) */
  if (!isGaussian && !isMexican) {
    fct_r = evaluator_create(fct_r_str);
    if (!fct_r)
      {
	Tcl_AppendResult (interp, "libmatheval : error",
			  " in expression ", fct_r_str, (char *) NULL);
	return TCL_ERROR;
      }
    fct_i = evaluator_create(fct_i_str);
    if (!fct_i)
      {
	Tcl_AppendResult (interp, "libmatheval : error",
			  " in expression ", fct_i_str, (char *) NULL);
	return TCL_ERROR;
      }

    im_fftw3d_filter_fct(image, Scale, fct_r, fct_i);

  } else {
    
    /* computation */
    if (isGaussian) { /* derivative of Gaussian filter */
      if (isX)
	im_fftw3d_filter (image, Scale, 0, 0);
      else if (isY)
	im_fftw3d_filter (image, Scale, 0, 1);
      else
	im_fftw3d_filter (image, Scale, 0, 2);
    } else { /* derivative of Mexican hat filter */
      if (isX)
	im_fftw3d_filter (image, Scale, 1, 0);
      else if (isY)
	im_fftw3d_filter (image, Scale, 1, 1);
      else
	im_fftw3d_filter (image, Scale, 1, 2);
    }
  }
 
  return TCL_OK;
}

/*
 * Command name in xsmurf : itofft
 */
/*------------------------------------------------------------------------
  ImaFftImageToFftCmd_
  
  En entree, un nom d'image source et un nom d'image destination.
  On transforme source dans un format particulier a l'algo fft
  de N.R.. On stocke le resultat dans l'image destination et on lance
  la fft la d'sus.
  ----------------------------------------------------------------------*/
int
ImaFftImageToFftCmd_(ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)      
{
  char * options[] = { "Is",
		       "-dim", "d",
		       NULL };
  
  char * help_msg =
    {("Transform the image in a FOURIER image (image name)\n"
      " -dim  dimension of the fft.\n")};

  Image *srcImage, *dst_image;
  char  *dst_imageName;
  int   dim=0;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &srcImage, &dst_imageName) == TCL_ERROR)
    return TCL_ERROR;
  
  if (arg_present(1))
    if (arg_get(1, &dim) == TCL_ERROR)
      return TCL_ERROR;
  
  if (srcImage->type == FOURIER)
    return GenErrorAppend(interp, "`", argv[0], "' already applied to `",
			  argv[1], "'.", NULL);
  
  dst_image = im_direct_to_fourier (srcImage, dim);
  if (!dst_image)
    GenErrorMemoryAlloc(interp);

  store_image (dst_imageName, dst_image);
  return TCL_OK;
}


/*
 * Command name in xsmurf : itofft2
 */
/*------------------------------------------------------------------------
  ImaFftImageToFft2Cmd_
  
  En entree, deux noms d'images source (partie reelle et imaginaire) et
  un nom d'image destination.
  On transforme source dans un format particulier a l'algo fft
  de N.R.. On stocke le resultat dans l'image destination et on lance
  la fft la d'sus  (ou la fft inverse d'ailleurs).

  Pierre Kestener.
  ----------------------------------------------------------------------*/
int
ImaFftImageToFft2Cmd_(ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)      
{
  char * options[] = { "IIs",
		       "-dim", "d",
		       NULL };
  
  char * help_msg =
    {("Transform the two images (real and imaginary parts) in a FOURIER \n"
      "image (image name)\n"
      "\n"
      "Options:\n"
      " -dim  dimension of the fft.\n")};

  Image *srcImage, *src2Image, *dst_image;
  char  *dst_imageName;
  int   dim=0;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &srcImage,&src2Image, &dst_imageName) == TCL_ERROR)
    return TCL_ERROR;
  
  if (arg_present(1))
    if (arg_get(1, &dim) == TCL_ERROR)
      return TCL_ERROR;
  
  if (srcImage->type == FOURIER || src2Image->type == FOURIER )
    return GenErrorAppend(interp, "Images must be of type PHYSICAL\n", NULL);
  
  dst_image = im_direct_to_fourier2 (srcImage, src2Image, dim);
  if (!dst_image)
    GenErrorMemoryAlloc(interp);

  store_image (dst_imageName, dst_image);
  return TCL_OK;
}

/*
 * command name in xsmurf : ffttoi
 */
/*------------------------------------------------------------------------
  ImaFftFftToImageCmd_

  Transforme l'image source au format utilisable par _Fourn_ en une image
  de type normal.
  ----------------------------------------------------------------------*/
int
ImaFftFftToImageCmd_(ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)      
{
  char * options[] = { "Is", NULL };

  char * help_msg =
    {("Transform the FOURIER image in an image (image name).\n"
      "In fact, just keep real part of input FOURIER image\n")};

  Image *srcImage, *dst_image;
  char  *dst_imageName;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &srcImage, &dst_imageName) == TCL_ERROR)
    return TCL_ERROR;
  
  if (srcImage->type == PHYSICAL)
    return GenErrorAppend(interp, "`", argv[0], "' already applied to `",
			  argv[1], "'.", NULL);
  
  dst_image = im_fourier_to_direct (srcImage);
  if (!dst_image)
    GenErrorMemoryAlloc(interp);  

  store_image (dst_imageName, dst_image);
  return TCL_OK;
}

/*------------------------------------------------------------------------
  ImaFftFftCmd_

  Lance la fonction _Fourn_ sur une image au format approprie obtenue
  par l'une des deux fonctions precedentes. _Fourn_ effectue une trans.
  de Fourier directe ou inverse sur l'image passee en argument.
  ----------------------------------------------------------------------*/
int
ImaFftFftCmd_(ClientData clientData,
	      Tcl_Interp *interp,
	      int        argc,
	      char       **argv)      
{
  char * options[] = { "I",
		       "-direct", "",
		       "-reverse", "",
		       "-dim", "d",
		       NULL };

  char * help_msg =
    {("Compute the Fourier Transform (by fft) of the FOURIER image (image)\n"
      " -direct   (default)\n"
      " -reverse  inverse transform\n"
      " -dim      dimension of the fft.")};
  

  unsigned long dimArray[3];
  int           way = DIRECT;
  Image         *srcImage;
  int           dim;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &srcImage) == TCL_ERROR)
    return TCL_ERROR;
  
  if (arg_present(2))
    way = REVERSE;

  if (arg_get(3, &dim) == TCL_ERROR)
    return TCL_ERROR;
  
  if (srcImage->type != FOURIER)
    return GenErrorAppend(interp, "`", argv[1],
			  "' has not FOURIER type.", NULL);
  
  /* On cree ou recupere du dictionnaire une image dst_image aux dim. voulues */
  dimArray[1] = (unsigned long)next_power_of_2__(srcImage->lx);
  dimArray[2] = (unsigned long)next_power_of_2__(srcImage->ly);

  if(arg_present(3)) {
    dimArray[1] = dim;
    dimArray[2] = dim;
  }

  smSetBeginTime();
  _Fourn_(srcImage->data, dimArray, 2, way);
  smSetEndTime();

  sprintf( interp->result, "%f", smGetEllapseTime());

  return TCL_OK;
}

/*------------------------------------------------------------------------
  Fonctions appellees par ImaFileSaveCmd_ suivant les options.
  ----------------------------------------------------------------------*/

void
_ImaFileSippFormat_(FILE*  fileOut,
		    int    lx,
		    int    ly,
		    real* data,
		    int    step,
		    int    div)
{
  int x, y;

  for (x = step; x < lx - step; x += step)
    for (y = step; y < ly - step; y += step) {
      if (data[ x + lx * y ] > div){
	fprintf(fileOut,
		"v %d %d %f\n"
		"v %d %d %f\n"
		"v %d %d %f\np\n",
		x     , y     , data[ x+lx*y ] / div,
		x+step, y     , data[ x+step+lx*y ] / div,
		x     , y+step, data[ x+lx*(y+step) ] / div);
	fprintf(fileOut,
		"v %d %d %f\n"
		"v %d %d %f\n"
		"v %d %d %f\np\n",
		x     , y     , data[ x+lx*y ] / div,
		x-step, y     , data[ x-step+lx*y ] / div,
		x     , y-step, data[ x+lx*(y-step) ] / div);
      }
    }
  fprintf(fileOut, "S\n");
}

/*------------------------------------------------------------------------
  ImaFileSaveCmd_
  command name in xsmurf : isave

  Sauvegarde d'une image sur disque
  ----------------------------------------------------------------------*/

/* ***********************
   modification du 07/02/2000

   on rajoute des options possibles:
   -hbin  : sauvegarde en binaire sans header
   -hchar : sauvegarde en characteres sans header
   -char  : sauvegarde en characteres avec header pgm (compatible LastWave)

   pour le moment ces trois options ne peuvent etre utilisees que toutes seules,
   cad sans les combiner avec les autres
   et elles supposent que l'image asauver est de type "PHYSICAL"

   * ********************** */

int
ImaFileSaveCmd_(ClientData clientData,
		Tcl_Interp *interp,
		int argc,
		char **argv)      
{ 
  /* command line definition */
  char * options[] = { "I[s]",           /* Sauvegarde binaire par defaut. */
		       "-ascii","",      /* Sauvegarde ascii.              */
		       "-spec", "",      /* bidouille fft (format Re Im)  */
		       "-chris", "",
		       "-in_pos", "dddd",
		       "-out_pos", "dd",
		       "-4greg", "[ff]",
		       "-ascii2","d",    /* sauvegarde ascii without header */
		       "-seek","dd",
		       "-hbin","",       /*sauvegarde en binaire sans header
					   les donnees sont des reels 4 octets
					   par pixel*/
		       "-hchar","[f]",      /*sauvegarde en binaire : les donnees
					      sont tronquees 1 octets/pixel */
		       "-char","[f]",     /* meme chose mais avec header pgm */
		       /* le parametre est un facteur d'echelle : un reel
			  compris entre 0 et 1 */
		       "-bin_16bit","",  /* sauv. bin. 2 octets/pixel avec 
					    header pgm */
		       "-pgm8bit","[f]",    /* nouveau nom pour char */
		       "-pgm16bit","",   /* nouveau nom pour bin_16bit */
		       "-vtk","[d]",
		       "-4fd","",    /* pour le calcul de dimension fractale
					par box-counting;
					programme de John Sarraille 
					et  Peter DiFalco */ 
		       "-metric","",     /* Sauvegarde pour analyse Metric. */
		       "-ascii8bit","",  /* Sauvegarde ascii, no header, [0,255]. */
		       NULL};
  
  char * help_msg =
    {("write the image in a BINARY file of name image or name (image [name])\n"
      "  -ascii         write in an ASCII mode\n"
      "  -ascii8bit     write in an ASCII mode with noheader, in range [0,255] \n"
      "  -metric        write in ascii mode for Metric analysis \n"
      "  -ascii2        write in ASCII mode without header\n"
      "  -hbin          write data in binary mode using fwrite, 4 byte reals, without header\n"
      "  -char [f]      write data as unsigned char (8 bit), using fprintf,\n "
      "                 with a header pgm-like, compatible LastWave.\n"
      "                 parameter is a scale factor : float between 0 and 1\n"
      "  -pgm8bit [f]   same as above\n"
      "  -hchar [f]     write data as unsigned char (8 bit) without header.\n"
      "  -bin_16bit     write data in binary mode using fwrite, u-short \n"
      "                 int (16 bit) \n"
      "                 with a header pgm-like (P5), compatible ddsm.\n"
      "                 take care that XVIEW (v3.10a) only displays the most\n"
      "                 significant byte for each pixel\n"
      "  -pgm16bit      same as above\n"
      "  -sipp          write in a sipp (3D) mode (interval of the high, interval of the space)\n"
      "  -ext           write the image and the extImage ext\n"
      "  -geo           write in a geomview mode (interval of the high, interval of the space)\n"
      "  -spec          write a fft file (Re and Im part) ????\n"
      "  -chris         write in chris mode.")};

  /* command's parameters */
  char     *imageFilename = NULL;
  FILE     *fileOut;
  Image    *image;
  int      lx, ly, size;
  int      i, j;
  real    *data;
  int is_chris;
  int is_in_pos;
  int is_out_pos;
  int is_hbin, is_hchar, is_char, is_pgm8bit, is_bin_16bit, is_pgm16bit;
  int is_vtk, is_4fd;
  int is_Ascii2, ascii2_param=0;
  int is_metric;
  int is_ascii8bit;
  int x_in_pos, y_in_pos, x_in_size, y_in_size;
  int x_out_pos, y_out_pos;
  char    tempBuffer[100], saveFormat[10];
    
  int is_seek;
  int startPos;
  int length;

  float themin;
  float themax;

  int reduce_param=1;
  
  real x0 = 0;
  real dx = 1;

  real scale_factor = 1.0;


  /* command line analysis and options' presence */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &image, &imageFilename) == TCL_ERROR)
    return TCL_ERROR;
  
  if (!imageFilename)
    imageFilename = argv[1];

  is_chris = arg_present(3);
  is_in_pos = arg_present(4);
  if (is_in_pos)
    if (arg_get (4, &x_in_pos, &y_in_pos,
		 &x_in_size, &y_in_size) == TCL_ERROR)
      return TCL_ERROR;
  is_out_pos = arg_present(5);
  if (is_out_pos)
    if (arg_get (5, &x_out_pos, &y_out_pos) == TCL_ERROR)
      return TCL_ERROR;

  is_Ascii2 = arg_present(7);
  if (is_Ascii2) {
    if (arg_get (7, &ascii2_param) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  is_seek = arg_present(8);
  if (is_seek) {
    if (arg_get (8, &startPos, &length) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  is_metric = arg_present(17);
  is_ascii8bit = arg_present(18);

  lx   = image->lx;
  ly   = image->ly;
  size = image->size;
  data = image->data;

  if (is_seek) {
    int type;
    int realSize;

    fileOut = fopen(imageFilename, "rb+");
    fgets(tempBuffer, 100, fileOut);
    sscanf(tempBuffer, "%s %d %dx%d %d(%d",
	   saveFormat, &type, &lx, &ly, &size, &realSize);
    if ((type != FOURIER) && (type != PHYSICAL))
      return GenErrorAppend(interp, "`", imageFilename,
			    "' doesn't seem to be an Image.", NULL);
    if ((startPos+length) > lx*ly || startPos < 0 || length <= 0) {
      return GenErrorAppend(interp, "bad start position and/or length", NULL);
    }
    if (realSize != sizeof(real)) {
      return GenErrorAppend(interp, "real size problem...", NULL);
    }

    fseek(fileOut, startPos*realSize, SEEK_CUR);
    fwrite(data, sizeof(real), length, fileOut);

    fclose(fileOut);

    Tcl_AppendResult(interp, imageFilename, NULL);

    return TCL_OK;
  } else if (is_chris)
    {
      fileOut = fopen(imageFilename, "w");
      fwrite(&lx, sizeof(int), 1, fileOut);
      fwrite(&ly, sizeof(int), 1, fileOut);
      fwrite(data, sizeof(float), size, fileOut);

      return TCL_OK;
    }
  if (is_out_pos)
    fileOut = fopen(imageFilename, "r+");
  else
    fileOut = fopen(imageFilename, "w");
  if (!fileOut)
    return GenErrorAppend(interp, "Couldn't open `", imageFilename,
			  "' for writing.", NULL);

  if (arg_present(6)) {  /* -4greg  !!! */
    /* ca aurait pu etre mieux commente ! merci! */
    if (arg_get (6, &x0, &dx) == TCL_ERROR) {
      return TCL_ERROR;
    }
    fprintf(fileOut, "%d %f 0 %f\nhaha\n%d %f 0 %f\nhoho\n", lx, x0, dx, ly, x0, dx);

    for (i = 0; i < size; i++) {
      fprintf(fileOut, "%#.3g\n", data[i]);
    }
    fclose(fileOut);

    Tcl_AppendResult(interp, imageFilename, NULL);

    return TCL_OK;
  }

  if (is_Ascii2) {
    if (ascii2_param == 1) {
      for (i=0;i<size;i++)
	fprintf(fileOut, "%d %d %g\n", i%lx,i/lx,data[i]);
    } else {
      for (i=0;i<size;i++)
	fprintf(fileOut, "%g\n", data[i]);
    }
  } else if (arg_present(1))	/* si sauvegarde en mode ascii */
    {
      if (arg_present(2))
	{
	  for (i=0;i<(size/2);i++)
	    fprintf(fileOut, "%g %g\n", data[i], data[i+1]);
	}
      else
	{
	  fprintf(fileOut, "Ascii  %d %dx%d %d\n", image->type, lx, ly, size);
	  for (i=0;i<size;i++) {
	    if (data[i] == 'NULL') {
	      data[i] = -999.0;
	    }
	    fprintf(fileOut, "%g\n", data[i]);
	  }
	}
    }
  else
    {
      if (is_in_pos && is_out_pos)
	{
	  int type;
	  int realSize;

	  fgets(tempBuffer, 100, fileOut);
	  sscanf(tempBuffer, "%s %d %dx%d %d(%d",
		 saveFormat, &type, &lx, &ly, &size, &realSize);
	  if ((type != FOURIER) && (type != PHYSICAL))
	    return GenErrorAppend(interp, "`", imageFilename,
				  "' doesn't seem to be an Image.", NULL);

	  if ((x_out_pos + x_in_size) > lx)
	    x_in_size = lx - x_out_pos;
	  if ((y_out_pos + y_in_size) > ly)
	    y_in_size = ly - y_out_pos;

	  data = data + x_in_pos + y_in_pos*image -> lx;
	  fseek (fileOut, (lx*y_out_pos + x_out_pos)*realSize, SEEK_CUR);
	  for (i = 0; i < y_in_size; i++)
	    {
	      fwrite (data + i*image -> lx, realSize, x_in_size, fileOut);
	      fseek (fileOut, (lx - x_in_size)*realSize, SEEK_CUR);
	    }
	}
      else if (is_in_pos)
	{
	  /* ("A faire....\n");*/
	}
      else if (is_out_pos)
	{
	  /* ("A faire....\n");*/
	}
      else
	{
	  is_hbin      = arg_present(9);
	  is_hchar     = arg_present(10);
	  if (arg_get (10, &scale_factor) == TCL_ERROR) {
	    return TCL_ERROR;
	  }
	  is_char      = arg_present(11);
	  if (arg_get (11, &scale_factor) == TCL_ERROR) {
	    return TCL_ERROR;
	  }
	  is_bin_16bit = arg_present(12);
	  is_pgm8bit   = arg_present(13);
	  if (arg_get (13, &scale_factor) == TCL_ERROR) {
	    return TCL_ERROR;
	  }
	  is_pgm16bit  = arg_present(14);
	  is_vtk  = arg_present(15);
	  if (arg_get (15, &reduce_param) == TCL_ERROR) {
	    return TCL_ERROR;
	  }
	  is_4fd  = arg_present(16);


	  /* test if scale_factor is correct */
	  if (scale_factor < 0 || scale_factor >1)
	    return GenErrorAppend(interp, "scale_factor MUST be ranged between 0 and 1.", NULL);

	  if (is_hbin) /* write binary 4 byte reals, no header */
	    {
	      fwrite(data, sizeof(real), size, fileOut);
	    }
	  else if (is_hchar)  /* write unsigned char but no header */
	    {
	      im_get_extrema (image,&themin,&themax);
	      for(i = 0; i < size; i++)
		{
		  fprintf(fileOut,"%c",(unsigned char)((int)((data[i]-themin)/(themax-themin)*255*scale_factor)));
		  /*fprintf(fileOut,"%c",(unsigned char)((int)(data[i])));*/
	        }
	    }
	  else if (is_char || is_pgm8bit)  /* write unsigned char and a header compatible with Lastwave */
	    {
	      im_get_extrema (image,&themin,&themax);
	      fprintf(fileOut,"P5\n%d %d\n",lx,ly);
	      fprintf(fileOut,"255\n");
	      for(i = 0; i < size; i++)
		{
		  fprintf(fileOut,"%c",(unsigned char)((int)((data[i]-themin)/(themax-themin)*255*scale_factor)));
	        }
	    }
	  else if (is_bin_16bit || is_pgm16bit)  /* write unsigned short and a header compatible with ddsm */
	    {
	      /* check data range of Image image */
	      /*real themax;
		real themin;*/
	      unsigned short int  *thisrow_ushort;
	      im_get_extrema (image,&themin,&themax);
	      if ((image->min < 0) || (image->max > 65535))
		return GenErrorAppend(interp, "`", imageFilename,
				      "' must range in 0 65535. You must rescale it !!", NULL);
	      
	      /* write header */
	      fprintf(fileOut,"P5\n%d %d\n",lx,ly);
	      fprintf(fileOut,"65535\n");
	      /* end of header */

	      /* write data raw by raw !*/
	      thisrow_ushort = (unsigned short int *) calloc(ly, sizeof(unsigned short int));
	      for(i=0;i<lx;i++)
		{
		  for(j=0;j<ly;j++)
		    {
		      thisrow_ushort[j] = (unsigned short int) data[i*ly+j];  
		    }
		  if ( fwrite(thisrow_ushort, sizeof(unsigned short int), ly, fileOut) < ly )
		    {
		      sprintf (interp -> result, "Error in writing file; problem with fwrite");
		      return TCL_ERROR;
		    }
		}
	      free(thisrow_ushort);
	    }
	  else if (is_vtk) {
	    /* we suppose that original image is 512x512 */
	    real *thedata;
	    int ii, jj, nb_points;
	    
	    nb_points = 0;
	    for (ii=0;ii<lx;ii++) {
	      for (jj=0;jj<ly;jj++) {
		if ((ii%reduce_param)==0 && (jj%reduce_param)==0)
		  nb_points++;
	      }
	    }

	    fprintf(fileOut, "# vtk DataFile Version 2.0 \n");
	    fprintf(fileOut, "data generated by xsmurf.\n");
	    fprintf(fileOut, "ASCII\n");
	    fprintf(fileOut, "\n");
	    fprintf(fileOut, "DATASET STRUCTURED_POINTS\n");
	    fprintf(fileOut, "DIMENSIONS  %d %d 1\n", lx/reduce_param, ly/reduce_param);
	    //fprintf(fileOut, "DIMENSIONS  %d %d 1\n", 128, 128);
	    fprintf(fileOut, "ORIGIN  0.000 0.000 0.000\n");
	    fprintf(fileOut, "SPACING  1.000 1.000 0.01\n");
	    fprintf(fileOut, "\n");
	    fprintf(fileOut, "POINT_DATA %d\n",nb_points);
	    //fprintf(fileOut, "POINT_DATA %d\n",128*128);
	    fprintf(fileOut, "SCALARS scalars float\n");
	    fprintf(fileOut, "LOOKUP_TABLE default\n\n");

	    for(jj=0;jj<ly;jj++) {
	      for(ii=0;ii<lx;ii++) {
		if ((ii%reduce_param)==0 && (jj%reduce_param)==0) 
		  fprintf(fileOut, "%f ",data[ii*ly+jj]);
	      }
	      fprintf(fileOut, "\n");
	    }
	  }
	  else if (is_4fd)
	    {
	      int compt=0;
	      im_get_extrema (image,&themin,&themax);
	      
	      /* write header */
	      for(i=0; i<lx; i++){
		for(j=0; j<ly; j++){
		  if (data[i*ly+j]<themax/2) {
		    compt++;
		  }
		}
	      }
	      fprintf(fileOut,"%d\n",compt);
	      /* end of header */

	      /* write data raw by raw !*/
	      for(i=0; i<lx; i++){
		for(j=0; j<ly; j++){
		  if (data[i*ly+j]<themax/2) {
		    fprintf(fileOut,"%f %f\n",(float) i,(float) j); 
		  }
		}
	      }
	    }
	  else if (is_metric) { /* -metric */
	    fprintf(fileOut, "%d \n%d \n\n", lx, ly);
	    im_get_extrema (image,&themin,&themax);
	    fprintf(fileOut,"%g \n%g \n\n", themin, themax);
	    for (i=0;i<size;i++)
	      fprintf(fileOut, "%g\n", data[i]);
	  }
	  else if (is_ascii8bit) { /* -metric */
	    //fprintf(fileOut, "%d \n%d \n\n", lx, ly);
	    //im_get_extrema (image,&themin,&themax);
	    //fprintf(fileOut,"%g \n%g \n\n", themin, themax);
	    for (i=0;i<size;i++) {
	      if (i%512 == 0) {
		fprintf(fileOut, "\n");
	      }
	      fprintf(fileOut, "%g ", floor(256*data[i]));
	    }
	  }
	  
	  else
	    {
	      fprintf(fileOut, "Binary %d %dx%d %d(%d byte reals)\n",
		      image->type, lx, ly, size, (int) sizeof(real)); 
	      fwrite(data, sizeof(real), size, fileOut);
	    }
	}
    }
  
  fclose(fileOut);
  
  Tcl_AppendResult(interp, imageFilename, NULL);
  
  return TCL_OK;
}

/***********************************
 * Command name in xsmurf : iload
 ***********************************/
/*------------------------------------------------------------------------
  ImaFileLoadCmd_
  
  Lecture d'une image depuis le disque.
  ----------------------------------------------------------------------*/

/* ***********************
   modification du 07/02/2000

   on rajoute des options possibles:
   -hbin lx ly  : lecture en binaire sans header (on suppose que la taille de
   l'image est lx*ly, et que l'on a des "4 byte real")
   -hchar lx ly : lecture d'une image codee en characteres sans header
   -char        : lecture d'une image codee en characteres avec header 
   (compatible LastWave)

   pour le moment ces trois options ne peuvent etre utilisees que toutes seules,
   cad sans les combiner avec les autres
   et elles creent une image de type "PHYSICAL"

   modif du 14/06/2000
   -tiff    : pour lire les images scannees a Montreal (12 bits codes sur
   16 bits)
   remarque : XV ne lit que les 8bits de poids forts
   je n'arrive pas a utiliser un logiciel classique pour 
   recuperer le tableau des donnees en ascii

   ca marche bien maintenant en tiff (fin 2001) !!!

   -P2_ASCII_16bit

   * ********************** */

int
ImaFileLoadCmd_(ClientData clientData,
		Tcl_Interp *interp,
		int argc,
		char **argv)      
{ 
  char * options[] = { "s[s]",  
		       "-ascii", "",
		       "-chris", "",
		       "-chris2", "[d]",
		       "-pos", "dddd",
		       "-size", "",
		       "-xv", "",
		       "-coma", "",
		       "-seek", "dd[d]",
		       "-hbin", "dd",
		       "-hchar", "dd",
		       "-char", "",
		       "-bin_16bit", "",
		       "-tiff", "[d]",
		       "-pgm8bit","",
		       "-pgm16bit","",
		       "-swap_bytes", "",
		       "-turbulence","d[d]",
		       "-turbulence2","d[d]",
		       "-turbulence3","d[d]",
		       "-bin_16bit_signed", "",
		       "-pgm_ascii", "",
		       "-monin", "",
		       "-3dchar",  "d[d]",
		       "-3dfloat", "d[d]",
		       "-3ddouble", "d[d]",
		       "-leguer", "d",
		       "-dbl", "d",
		       "-xy", "",
		       "-xz", "",
		       "-zy", "",

		       NULL};
  
  char * help_msg =
    {("read a binary file and put it in an image of name filename or name (filename,[name])\n"
      "  -ascii      read an ASCII file\n"
      "      -pgm    : (valid only if -ascii used) read header 'pgm format' like.\n"
      "  -chris      read a chris file\n"
      "  -chris2     read a chris2 file\n"
      "  -hbin       read data (4 byte real) in binary file (use fread) without header\n"
      "  -hchar      read data (unsigned char 8 bit) in file without header\n"
      "         - int : lx (x-dimension)\n"
      "         - int : ly (y-dimension)\n"
      "\n"
      "  -char (or -pgm8bit)\n"
      "              read data (unsigned char 8 bit) in file with header compatible\n"
      "              LastWave\n"
      "  -bin_16bit (or -pgm16bit)\n"
      "              read data (unsign short int 16 bit) in binary file (use fread) \n"
      "              with header 'pgm format' (P5)  like (originally to read ddsm \n"
      "              files).\n"
      "              take care that XVIEW (v3.10a) only displays the most\n"
      "              significant byte for each pixel of a PGM-images\n"
      "  -swap_bytes : when using unsigned short data, you can get value\n"
      "                and swap bytes!\n"
      "                compatible with -bin_16bit or -turbulence option\n"
      "                ONLY !!\n"
      "  -tiff [d]   read tiff images (gray-scale)\n"
      "              take care that XVIEW (v3.10a) only displays the most\n"
      "              significant byte for each pixel of a tiff-images\n"
      "              integer parameter is to select nth image in file\n" 
      "              this must replace -chris2 option\n"
      "  -turbulence : see -hbin option (almost the same)\n"
      "                first parameter must lie between 0 and 511\n"
      "                orthogonal cut to x-axis\n"
      "               second parameter (optionnal) is the size of the data\n"
      "               (default is 512)\n"
      "  -pgm_ascii:\n"
      "  -monin:\n"
      "  -3dchar [d [d]]: read file (3D data), assuming that each voxel value\n"
      "           is coded with an unsigned char, and that matrix of \n"
      "           points is cubic.\n"
      "            - 1st parameter (called seek_index) is the index of the\n"
      "              2D plane to be read \n"
      "            - 2nd parameter (optional) is the size of the 3D matrix \n"
      "              default is 64\n"
      "  -3dfloat  [d [d]]: same as -3dchar, but interpreting data as float.\n"
      "  -3ddouble [d [d]]: same as -3dchar, but interpreting data as double.\n"
      "  -leguer   [d] : read Yves Leguer's files.\n"
      "  -dbl [d]: Read .dbl files. Specify the z-slice number. \n"
      "            Add option -xy, -xz, or -zy to get the max projection\n"
      "            in one of these directions.\n"
      "\n"
      "\n"
      "\n"
      "\n"
      "PS: when using -hbin or -hchar you MUST know the size of\n"
      "the image: lx and ly")};
  
  char  * imageFilename = NULL;
  char  * imageName     = NULL; 
  int     lx, ly, lz, i, size, slice, type=-1, realSize;
  int     llx =0, lly=0;      /* used with option -hbin or -hchar or -bin_16bit
				 or -tiff */
  char    tempBuffer[100], saveFormat[10];
  char resultBuffer[200];
  char char_header[200]; /* to be compatible with LastWave and pgm headers*/
  char char_car[200];    /* to be compatible with LastWave and pgm headers*/
  int countRC;           /* to be compatible with LastWave and pgm headers*/
  
  FILE  * fileIn;
  Image * image = NULL;
  real * data;
  int x_pos, y_pos;
  int x_size, y_size;
  
  int is_pos = 0;
  int is_chris = 0;
  int is_chris2 = 0;
  int is_size = 0;
  int is_xv = 0;
  int is_coma = 0;
  int is_seek = 0;
  int image_nb = 1;
  int seekLx = 0;
  int seekLy = 0;
  int is_hbin = 0;
  int is_hchar = 0;
  int is_char = 0;
  int is_bin_16bit = 0;
  int is_tiff = 0;
  int is_pgm8bit = 0;
  int is_pgm16bit = 0;
  int is_swap = 0;
  int is_turbulence = 0, size_turbulence = 512;
  int is_turbulence2 = 0;
  int is_turbulence3 = 0;
  int is_bin_16bit_signed = 0;
  int is_ascii = 0;
  int is_pgm_ascii = 0;
  int is_monin = 0;
  int is_3dchar = 0, size_3ddata = 64;
  int is_3dfloat = 0;
  int is_3ddouble = 0;
  int is_leguer = 0;
  int is_dbl = 0;
  int is_xy = 0;
  int is_xz = 0;
  int is_zy = 0;

  /* variable used with is_bin_16bit, is_pgm8bit and
     is_bin_16bit_signed options */
  unsigned short int * thisrow_ushort;
  unsigned char      * thisrow_uchar;
  short int          * thisrow_sshort;

  /* variable used with is_swap and is_tiff option */
  unsigned short int byteorder;

  /* variables with turbulence options and 3d datas */
  int seek_x_index = 0;
  /*int turbu_size;*/

  unsigned char tmp_value;

  int startPos;
  int length;


  /* begin chris2*/
#define byte unsigned char

  unsigned char *imagefile;
  unsigned char gah_data[1000];
  int npar,idata,ntotdonnees,ilecture,idecal,ntot;
  /*   idecal parametre de decalage eventuel du tableau (multiple de 128)  */
  unsigned char *charo;
  
  int Xpixels, Ypixels, nframes, ind_frames;
  int j;
  int i1;

  /* end  chris2 */

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &imageFilename, &imageName) == TCL_ERROR)
    return TCL_ERROR;
  
  if (!imageName)
    imageName = imageFilename;

  if (!(fileIn = fopen(imageFilename, "r")))
    return GenErrorAppend(interp, "Couldn't open `", imageFilename,
			  "' for reading.", NULL);
  
  is_ascii  = arg_present(1);
  is_chris  = arg_present(2);
  is_chris2 = arg_present(3);
  if (is_chris2)
    if (arg_get (3, &image_nb) == TCL_ERROR)
      return TCL_ERROR;

  is_pos    = arg_present(4);
  is_size   = arg_present(5);
  is_xv     = arg_present(6);
  is_coma   = arg_present(7);
  is_hbin   = arg_present(9);
  if (is_hbin)
    if (arg_get (9, &llx, &lly) == TCL_ERROR)
      return TCL_ERROR;

  is_hchar  = arg_present(10);
  if (is_hchar)
    if (arg_get (10, &llx, &lly) == TCL_ERROR)
      return TCL_ERROR;

  is_char       = arg_present(11);
  is_bin_16bit  = arg_present(12);
  is_tiff       = arg_present(13);
  if (is_tiff)
    if (arg_get (13, &image_nb) == TCL_ERROR)
      return TCL_ERROR;
  is_pgm8bit    = arg_present(14);
  is_pgm16bit   = arg_present(15);
  is_swap       = arg_present(16);
  is_turbulence = arg_present(17);
  if (is_turbulence)
    if (arg_get (17, &seek_x_index, &size_turbulence) == TCL_ERROR)
      return TCL_ERROR;
  is_turbulence2 = arg_present(18);
  if (is_turbulence2)
    if (arg_get (18, &seek_x_index, &size_turbulence) == TCL_ERROR)
      return TCL_ERROR;
  is_turbulence3 = arg_present(19);
  if (is_turbulence3)
    if (arg_get (19, &seek_x_index, &size_turbulence) == TCL_ERROR)
      return TCL_ERROR;
  is_bin_16bit_signed  = arg_present(20);
  is_pgm_ascii  = arg_present(21);
  is_monin      = arg_present(22);
  is_3dchar = arg_present(23);
  if (is_3dchar)
    if (arg_get (23, &seek_x_index, &size_3ddata) == TCL_ERROR)
      return TCL_ERROR;
  is_3dfloat = arg_present(24);
  if (is_3dfloat)
    if (arg_get (24, &seek_x_index, &size_3ddata) == TCL_ERROR)
      return TCL_ERROR;
  is_3ddouble = arg_present(25);
  if (is_3ddouble)
    if (arg_get (25, &seek_x_index, &size_3ddata) == TCL_ERROR)
      return TCL_ERROR;
  is_leguer = arg_present(26);
  if (is_leguer)
    if (arg_get (26, &size) == TCL_ERROR)
      return TCL_ERROR;

  is_dbl = arg_present(27);
  if (is_dbl)
    if (arg_get (27, &slice) == TCL_ERROR)
      return TCL_ERROR;
  
  is_xy = arg_present(28);
  is_xz = arg_present(29);
  is_zy = arg_present(30);

  if (is_pos)
    if (arg_get (4, &x_pos, &y_pos, &x_size, &y_size) == TCL_ERROR)
      return TCL_ERROR;
  
  is_seek = arg_present(8);
  if (is_seek) {
    if (arg_get (8, &startPos, &length, &seekLx) == TCL_ERROR) {
      return TCL_ERROR;
    }
    if (seekLx > 0) {
      seekLy = ceil(length*1.0/seekLx);
    } else {
      seekLx = length;
      seekLy = 1;
    }
  }

  if (is_seek) {

    fgets(tempBuffer, 100, fileIn);
    sscanf(tempBuffer, "%s %d %dx%d %d(%d",
	   saveFormat, &type, &lx, &ly, &size, &realSize);

    if ((type != FOURIER) && (type != PHYSICAL))
      return GenErrorAppend(interp, "`", imageFilename,
			    "' doesn't seem to be an Image.", NULL);

    if ((startPos+length) > lx*ly || startPos < 0 || length <= 0) {
      return GenErrorAppend(interp, "bad start position and/or length", NULL);
    }

    if (realSize != sizeof(real)) {
      return GenErrorAppend(interp, "real size problem...", NULL);
    }

    image = im_new(seekLx, seekLy, seekLx*seekLy, type);

    if (!image)
      return GenErrorMemoryAlloc(interp);

    data = image->data;
    fseek(fileIn, startPos*realSize, SEEK_CUR);
    fread(data, realSize, length, fileIn);

  } else if (is_xv)
    {
      unsigned char * char_data;

      fgets(tempBuffer, 100, fileIn);
      sscanf(tempBuffer, "%d %d", &lx, &ly);
      size = lx*ly;
      type = PHYSICAL;
      image = im_new(lx, ly, size, type);
      data = image->data;
      char_data = (unsigned char *) malloc (size*sizeof (unsigned char));
      if (!image)
	return GenErrorMemoryAlloc(interp);
      fread(char_data, sizeof(char), size, fileIn);
      for (i = 0; i < size; i++)
	data[i] = (real) char_data [i];
      free(char_data);
    }
  else if (is_chris)
    {
      fread(&lx, sizeof(int), 1, fileIn);
      fread(&ly, sizeof(int), 1, fileIn);
      size = lx*ly;
      type = PHYSICAL;
      image = im_new(lx, ly, size, type);
      data = image->data;
      if (!image)
	return GenErrorMemoryAlloc(interp);
      fread(data, sizeof(float), size, fileIn);
    }
  else if (is_chris2)
    {
      /* on modifie quelques parametres pour pouvoir lire les fichiers
	 de Francois */
      /*idecal = 128;
	idata  = 896;*/
      idecal = 0;
      idata  = 768; /* pour les images de Francois */
      /*idata  = 1000;*/
      npar =1000;
      ntot = 300;
      /* on lit les idata premiers bytes du fichiers !*/
      ilecture = fread(gah_data,1,idata,fileIn);
      if (ilecture==npar) {
	sprintf(resultBuffer, "la lecture des %d donnees s'est bien effectuee \n",ilecture);
	Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
      } /* je sais pas a quoi ca sert,
	   c'est pas moi qui ait ecrit ca ! (pierre)*/

      charo = &gah_data[idecal + 30];
      sprintf(resultBuffer, "%d   %d\n",charo[0],charo[1]);
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
      Xpixels = charo[0]*256 + charo[1];
      sprintf(resultBuffer, "largeur de l'image : %d   \n",Xpixels);
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
      charo = &gah_data[idecal + 42];
      sprintf(resultBuffer, "%d   %d\n",charo[0],charo[1]);
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
      Ypixels = charo[0]*256 + charo[1];
      sprintf(resultBuffer, "longueur de l'image %d   \n",Ypixels);
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
      /*      charo = &gah_data[idecal + 516];*/
      charo = &gah_data[idecal + 516];
      sprintf(resultBuffer, "%d   %d\n",charo[0],charo[1]);
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
      nframes = charo[0]*256 + charo[1];
      sprintf(resultBuffer, "nombre d'images %d   \n",nframes);
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
      ntotdonnees = Xpixels * Ypixels;
      if (image_nb > nframes)
	{
	  sprintf (interp -> result, "Wrong number of the image.");
	  return TCL_ERROR;
	}
      /* image_nb indique le numero de l'image dans le film */

      imagefile = (unsigned char *) malloc(ntotdonnees*sizeof(char));
      sprintf(resultBuffer, "taille de l'image  %d  \n",ntotdonnees);
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);

      lx = Xpixels;
      ly = Ypixels;
      size = lx*ly;
      type = PHYSICAL;
      image = im_new(lx, ly, size, type);
      data = image->data;
      if (!image)
	return GenErrorMemoryAlloc(interp);

      /* on passe a travers toutes les images qui precedent
	 celle que l'on veut charger : l'image numero image_nb !!*/
      for (ind_frames = 0; ind_frames<image_nb; ind_frames++){
	ilecture = fread(imagefile,1,ntotdonnees,fileIn);
	if (ilecture!=ntotdonnees) {
	  sprintf(resultBuffer, "pb lecture image: %d \n",ind_frames+1);
	  Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
	}
      }
      /* on finit par la lire pour de bon */
      for (i=0; i<Xpixels; i++) {
	for (j=0; j<Ypixels; j++) {
	  i1 =14 + (int) (((float) imagefile[i + j*Xpixels])* 241.0 / 255.0);
	  data[i + j*Xpixels] = (real) i1;
	}
      }  
      free (imagefile);
    }
  else if (is_size)
    {
      fgets(tempBuffer, 100, fileIn);
      sscanf(tempBuffer, "%s %d %dx%d %d(%d",
	     saveFormat, &type, &lx, &ly, &size, &realSize);
      if ((type != FOURIER) && (type != PHYSICAL))
	return GenErrorAppend(interp, "`", imageFilename,
			      "' doesn't seem to be an Image.", NULL);
      sprintf (interp -> result, "%d", lx);
    }
  else
    {
      if (is_hbin)
	{
	  image = im_new(llx, lly, llx*lly, PHYSICAL);
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  fread(data, 4, llx*lly,fileIn);
	}
      else if (is_turbulence)
	{
	  if (seek_x_index < 0 || seek_x_index > size_turbulence-1 ) {
	    return GenErrorAppend(interp, "bad parameter, see help", NULL);
	  }
	  image = im_new(size_turbulence, size_turbulence, size_turbulence*size_turbulence, PHYSICAL );
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  fseek(fileIn,(long) 4*size_turbulence*size_turbulence*seek_x_index ,SEEK_SET);
	  fread(data, sizeof(float), size_turbulence*size_turbulence,fileIn);
	  
	  if (is_swap) { /* utilisation de la routine de lastwave 1.7*/
	    BigLittleValues(image->data,size_turbulence*size_turbulence,sizeof(float));
	  }
	    
	}
      else if (is_turbulence2)
	{
	  int index_line;
	  if (seek_x_index < 0 || seek_x_index > size_turbulence-1 ) {
	    return GenErrorAppend(interp, "bad parameter, see help", NULL);
	  }
	  image = im_new(size_turbulence, size_turbulence, size_turbulence*size_turbulence, PHYSICAL );
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  for (index_line=0;index_line<size_turbulence;index_line++) {
	    fseek(fileIn,(long) 4*(size_turbulence*seek_x_index+size_turbulence*size_turbulence*index_line) ,SEEK_SET);
	    fread(data+size_turbulence*index_line, sizeof(float), size_turbulence,fileIn);
	  }
	  if (is_swap) { /* utilisation de la routine de lastwave 1.7*/
	    BigLittleValues(image->data,size_turbulence*size_turbulence,sizeof(float));
	  }
	    
	}
      else if (is_turbulence3) /* a   faire  !!!! */
	{
	  int index_line1;
	  int index_line2;

	  if (seek_x_index < 0 || seek_x_index > size_turbulence-1 ) {
	    return GenErrorAppend(interp, "bad parameter, see help", NULL);
	  }
	  image = im_new(size_turbulence, size_turbulence, size_turbulence*size_turbulence, PHYSICAL );
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;

	  fseek(fileIn,(long) 4*seek_x_index ,SEEK_SET);

	  for (index_line1=0;index_line1<size_turbulence;index_line1++) {
	    for (index_line2=0;index_line2<size_turbulence;index_line2++) {
	      fread(data+index_line2+size_turbulence*index_line1, 4, 1,fileIn);
	      fseek(fileIn,(long) 4*size_turbulence ,SEEK_CUR);
	    }
	  }
	  if (is_swap) { /* utilisation de la routine de lastwave 1.7*/
	    BigLittleValues(image->data,size_turbulence*size_turbulence,sizeof(float));
	  }
	    
	}
      else if (is_3dchar)
	{
	  unsigned char *datachar;
	  int i;
	  datachar = (unsigned char *) malloc(sizeof(unsigned char)*size_3ddata*size_3ddata);
	  if (seek_x_index < 0 || seek_x_index > size_3ddata-1 ) {
	    return GenErrorAppend(interp, "bad parameter, see help", NULL);
	  }
	  image = im_new(size_3ddata, size_3ddata, size_3ddata*size_3ddata, PHYSICAL );
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  fseek(fileIn,(long) size_3ddata*size_3ddata*seek_x_index ,SEEK_SET);
	  fread(datachar, 1, size_3ddata*size_3ddata,fileIn);  
	  for (i=0;i<size_3ddata*size_3ddata;i++) {
	    data[i] = (float) (datachar[i]);
	  }
	}
      else if (is_3dfloat)
	{
	  if (seek_x_index < 0 || seek_x_index > size_3ddata-1 ) {
	    return GenErrorAppend(interp, "bad parameter seek_index, see help", NULL);
	  }
	  image = im_new(size_3ddata, size_3ddata, size_3ddata*size_3ddata, PHYSICAL );
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  fseek(fileIn,(long) 4*size_3ddata*size_3ddata*seek_x_index ,SEEK_SET);
	  fread(data, sizeof(float), size_3ddata*size_3ddata,fileIn);
	  if (is_swap) {
	    BigLittleValues(image->data,image->lx*image->ly,sizeof(float));
	  }
	}
      else if (is_3ddouble)
	{
	  double *data_double;
	  int index;
	  if (seek_x_index < 0 || seek_x_index > size_3ddata-1 ) {
	    return GenErrorAppend(interp, "bad parameter seek_index, see help", NULL);
	  }
	  image = im_new(size_3ddata, size_3ddata, size_3ddata*size_3ddata, PHYSICAL );
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;

	  data_double = (double *) malloc(sizeof (double)*size_3ddata*size_3ddata);
	  fseek(fileIn,(long) sizeof(double)*size_3ddata*size_3ddata*seek_x_index ,SEEK_SET);
	  fread(data_double, sizeof(double), size_3ddata*size_3ddata,fileIn);
	  if (is_swap) {
	    BigLittleValues(data_double,size_3ddata*size_3ddata,sizeof(double));
	  }
	  for (index=0;index<size_3ddata*size_3ddata;index++)
	    data[index] = (float) data_double[index];
	  free(data_double);
	}
      else if (is_leguer)
	{
	  float tmp1,tmp2,tmp3;
	  int index;
	  image = im_new(size, size, size*size, PHYSICAL );
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;

	  for (index=0;index<size*size;index++) {
	    fscanf(fileIn,"%f %f",&tmp1,&tmp2);
	    fscanf(fileIn,"%f\n",&tmp3);
	    data[index] = tmp3;
	  }
	}
      else if (is_hchar)
	{
	  image = im_new(llx, lly, llx*lly, PHYSICAL);
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  for(i =0; i < llx*lly; i++)
	    {
	      fscanf(fileIn,"%c",&tmp_value);
	      data[i] = (float)tmp_value;
	    }
	}
      else if (is_char || is_pgm8bit)
	{
	  /* reading header of pgm file */
	  fgets(char_header, 190, fileIn); // should read P5
	  countRC = 0;
	  unsigned int maxval;
	  while (countRC<3) {
	    fgets(char_header,190,fileIn);
	    //countRC = countRC + (strchr(char_header,'\n')!=NULL);
	    if (char_header[0] == '#')
	      continue;
	    
	    if(countRC == 0) 
	      {
                countRC += sscanf( char_header, "%d %d %u", &llx, &lly, &maxval);
	      }
            else if (countRC == 1) 
	      {
                countRC += sscanf( char_header, "%d %u", &lly, &maxval);
	      }
            else if (countRC == 2) 
	      {
                countRC += sscanf( char_header, "%u", &maxval);
	      }
	  }
	  /* reading data char : 8bit */
	  image = im_new(llx, lly, llx*lly, PHYSICAL);
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  for(i =0; i < llx*lly; i++)
	    {
	      fscanf(fileIn,"%c",&tmp_value);
	      data[i] = (float)tmp_value;
	    }
	}
      else if (is_bin_16bit || is_pgm16bit) 
	{
	  /* reading pgm like header of BINARY file */
	  /* be careful that the guy who wrote this put first ncolumns and then nraws */
	  countRC = 0;
	  while (countRC!=3) {
	    fgets(char_header,190,fileIn);
	    countRC = countRC + (strchr(char_header,'\n')!=NULL);
	    if (strchr(char_header,'#')) countRC--;
	    else {
	      if (countRC==2) {
		llx=atoi(char_header);
		sprintf(char_car,"%d",llx);
		for (i=0;i<(strlen(char_car)+1);i++) char_header[i]=' ';
		lly=atoi(char_header);
	      }
	    }
	  }
	  /* reading data char */
	  image = im_new(llx, lly, llx*lly, PHYSICAL);
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  thisrow_ushort = (unsigned short int *) calloc(lly, sizeof(unsigned short int));

	  for(i=0;i<llx;i++)
	    {
	      if ( fread(thisrow_ushort, sizeof(unsigned short int), lly, fileIn) < lly )
		{
		  sprintf (interp -> result, "Error in reading file; problem of size");
		  return TCL_ERROR;
		}
	      if (is_swap)
		BigLittleValues(thisrow_ushort,lly,sizeof(unsigned short int));
	      for(j=0;j<lly;j++)
		{
		  data[i*lly+j] = (float) thisrow_ushort[j];
		}
	    }
	  free(thisrow_ushort);
	}
      else if (is_bin_16bit_signed) 
	{
	  /* reading pgm like header of BINARY file */
	  /* SIGNED short int data's */
	  countRC = 0;
	  while (countRC!=3) {
	    fgets(char_header,190,fileIn);
	    countRC = countRC + (strchr(char_header,'\n')!=NULL);
	    if (strchr(char_header,'#')) countRC--;
	    else {
	      if (countRC==2) {
		llx=atoi(char_header);
		sprintf(char_car,"%d",llx);
		for (i=0;i<(strlen(char_car)+1);i++) char_header[i]=' ';
		lly=atoi(char_header);
	      }
	    }
	  }
	  /* reading data char */
	  image = im_new(llx, lly, llx*lly, PHYSICAL);
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  thisrow_sshort = (short int *) calloc(lly, sizeof(short int));

	  for(i=0;i<llx;i++)
	    {
	      if ( fread(thisrow_sshort, sizeof(short int), lly, fileIn) < lly )
		{
		  sprintf (interp -> result, "Error in reading file; problem of size");
		  return TCL_ERROR;
		}
	      for(j=0;j<lly;j++)
		{
		  data[i*lly+j] = (float) thisrow_sshort[j];
		}
	    }
	  free(thisrow_sshort);
	  if (is_swap) { /* utilisation de la routine de lastwave 1.7*/
	    BigLittleValues(image->data,llx*lly,sizeof(float));
	  }
	}
      else if (is_pgm_ascii) 
	{
	  int read_val;
	  /* reading pgm like header of ASCII file */
	  /* be careful that the guy who wrote this put first*/
	  /* ncolumns and then nraws */
	  /*	      sprintf (interp->result, "toto1\n");*/
	  countRC = 0;
	  while (countRC!=3) {
	    fgets(char_header,190,fileIn);
	    countRC = countRC + (strchr(char_header,'\n')!=NULL);
	    if (strchr(char_header,'#')) countRC--;
	    else {
	      if (countRC==2) {
		llx=atoi(char_header);
		sprintf(char_car,"%d",llx);
		for (i=0;i<(strlen(char_car)+1);i++) char_header[i]=' ';
		lly=atoi(char_header);
	      }
	    }
	  }
	  image = im_new(llx, lly, llx*lly, PHYSICAL);
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  for (i=0;i<llx;i++) {
	    for (j=0;j<lly;j++) {
	      fscanf(fileIn, "%d ", &read_val);
	      data[i*lly+j] = read_val;
	    }
	    fscanf(fileIn, "\n"); 
	  }
	}
      else if (is_monin) 
	{
	  real read_val;
	  /* reading pgm like header of ASCII file */
	  /* be careful that the guy who wrote this put first*/
	  /* ncolumns and then nraws */
	  countRC = 0;
	  while (countRC!=4) {
	    fgets(char_header,190,fileIn);
	    sprintf(resultBuffer, char_header);
	    Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
	    countRC = countRC + (strchr(char_header,'\n')!=NULL);
	    if (strchr(char_header,'#')) countRC--;
	    else {
	      if (countRC==2) {
		lly=atoi(char_header+4);
	      } else if (countRC==3) {
		llx=atoi(char_header+4);
	      }
	    }
	  }
	  image = im_new(llx, lly, llx*lly, PHYSICAL);
	  if (!image)
	    return GenErrorMemoryAlloc(interp);
	  data = image->data;
	  for (i=0;i<llx;i++) {
	    for (j=0;j<lly;j++) {
	      fscanf(fileIn, "%f ", &read_val);
	      data[i*lly+j] = read_val;
	    }
	    fscanf(fileIn, "\n"); 
	  }
	}
      else if (is_tiff) {
	unsigned short int byteorder;
	unsigned short int version;
	unsigned long  int ifdoffset;
	unsigned short int NumberOfTags;
	unsigned short int TagId;
	unsigned short int DataType;
	unsigned long  int DataCount;
	unsigned long  int DataOffset;
	
	int BitsPerSample  = 0;
	int StripesInImage = 0;
	int RowsPerStrip   = 0;
	unsigned long int StripOffsets;
	unsigned short int tmp_us;
	
	/* read header and thus know header.byteorder i.e.
	   how data were written */
	/*fread(&header, sizeof(TIFFHEADER), 1, fileIn);*/
	byteorder = tiff_read_short(fileIn,0x4949);
	version   = tiff_read_short(fileIn,byteorder);
	ifdoffset = tiff_read_long(fileIn,byteorder);

	fseek(fileIn,(long) (ifdoffset), SEEK_SET);

	/* then read NumberOfTags in current IFD
	   (Image File Directory */
	NumberOfTags = tiff_read_short(fileIn,byteorder);
	for (i=0; i< (int) NumberOfTags; i++) {
	  TagId      = tiff_read_short(fileIn,byteorder);
	  DataType   = tiff_read_short(fileIn,byteorder);
	  DataCount  = tiff_read_long(fileIn,byteorder);
	  if (DataType == 3) {
	    DataOffset = tiff_read_short(fileIn,byteorder);
	    tmp_us     = tiff_read_short(fileIn,byteorder);

	  } else {
	    DataOffset = tiff_read_long(fileIn,byteorder);
	  }
	  /*printf("%d %d %d %d\n", TagId,DataType,DataCount,DataOffset);*/
	  if(TagId == 256) {             /* get ImageWidth */
	    llx = DataOffset;
	  } else if (TagId == 257) {     /* get ImageHeight or number of Rows */
	    lly = DataOffset;
	  } else if (TagId == 258) {     /* get BitsperSample */
	    BitsPerSample = DataOffset;
	    sprintf(resultBuffer, "BitsPerSample : %ld\n", DataOffset);
	    Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
	  } else if (TagId == 273) {     /* get StripOffsets  */
	    StripOffsets = DataOffset;
	    /*sprintf(resultBuffer, "gnagna : %d\n", DataOffset);
	      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);*/
	  } else if (TagId == 278) {     /* get RowsPerStrip */
	    RowsPerStrip = DataOffset;
	    sprintf(resultBuffer, "gnagna : %d\n", RowsPerStrip);
	    Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
	  } else if (TagId == 34337) {   /* get Softinfo ???*/
	  }
	}
	if ((llx == 0) | (lly == 0)) {
	  sprintf (interp -> result, "Error in reading file\n");
	  return TCL_ERROR;
	} else {
	  sprintf(resultBuffer, "lx and ly : %d %d\n", llx, lly);
	  Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
	  /*sprintf (interp -> result, "lx and ly : %d %d\n",llx,lly);*/
	}
	
	/* up to now i will only considere images that have
	   just one strip per image; so it's simpler to handle
	   StripOffsets array because there is just one value */
	/* see test just below */
	
	/* on identifie RowsPerStrip  a lly (ImageHeight) !!! */
	RowsPerStrip = lly;
	
	if (RowsPerStrip > 0) {
	  StripesInImage = floor ((lly + RowsPerStrip - 1) / RowsPerStrip);
	  if (StripesInImage != 1) {
	    sprintf (interp -> result, "Error StripesInImage != 1 !!!\n");
	    return TCL_ERROR;
	  } else {
	    sprintf (resultBuffer, "nombre de bandes dans l'image : %d\n",StripesInImage);
	    Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
	    sprintf (resultBuffer, "nombre de lignes par bande : %d\n",RowsPerStrip);
	    Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
	  }
	} else {
	  sprintf (interp -> result, "Error in reading RowsPerStrip\n");
	  return TCL_ERROR;
	}
	
	/* renvoie StripOffsets */
	sprintf (resultBuffer, "StripOffsets : %ld\n",StripOffsets);
	Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
	
	/* allocation pour l'image */
	image = im_new(llx, lly, llx*lly, PHYSICAL);
	if (!image)
	  return GenErrorMemoryAlloc(interp);
	data = image->data;
	
	/* allocation pour lire les donnes */
	if (BitsPerSample == 8) {
	  thisrow_uchar = (unsigned char *) calloc(llx, sizeof(unsigned char));
	  fseek(fileIn, StripOffsets+(image_nb-1)*llx*lly, SEEK_SET); /* on se positionne au bon endroit */
	  for(i=0;i<lly;i++)
	    {
	      if ( fread(thisrow_uchar, sizeof(unsigned char), llx, fileIn) < llx )
		{
		  sprintf (interp -> result, "Error in reading file; problem of size");
		  return TCL_ERROR;
		}
	      for(j=0;j<llx;j++)
		{
		  data[i*llx+j] = (float) thisrow_uchar[j];
		}
	    }
	  free(thisrow_uchar);
	  
	} else if (BitsPerSample == 16) {
	  thisrow_ushort = (unsigned short int *) calloc(llx, sizeof(unsigned short int)); /* just for plus tard si on ameliore la routine */
	  fseek(fileIn, StripOffsets+(image_nb-1)*llx*lly*2, SEEK_SET); /* on se positionne au bon endroit */
	  /* il n'y a qu'une bande non compressee on peut tout lire d'un coup */
	  for(i=0;i<lly;i++)
	    {
	      for(j=0;j<llx;j++)
		{
		  /* i had probleme to understand quite well how datas were written */
		  /* datas from crm were 12 bits coded on 16 bits + shift 4 bits*/
		  data[i*llx+j] = (float) tiff_read_short_crm(fileIn);
		} 
	    }
	  free(thisrow_ushort);
	  
	} else {
	  sprintf (interp -> result, "Error in reading file (BitsPerSample)");
	  return TCL_ERROR;
	}
      }
      else if (is_dbl) 
	{
	  float phys_x, phys_y, phys_z;
	    
	  if(fseek(fileIn, 8, SEEK_CUR)){
	    printf("Seek error.\n");
	    exit(1);
	  }
	    
	  fread(&phys_z, sizeof(float), 1, fileIn);
	  fread(&phys_y, sizeof(float), 1, fileIn);
	  fread(&phys_x, sizeof(float), 1, fileIn);
	      
	  lz= 1000*phys_z/75;
	  ly= 1000*phys_y/75;
	  lx= 1000*phys_x/75;

	    
	    
	  if(fseek(fileIn, 128, SEEK_SET)){
	    printf("Seek error.\n");
	    exit(1);
	  }
	    
	  if ((is_xy==0)&&(is_xz==0)&&(is_zy==0)) {

	    size = lx*ly;
	    type = PHYSICAL;
	    image = im_new(lx, ly, size, type);
	    data = image->data;
	    if (!image)
	      return GenErrorMemoryAlloc(interp);
		
	    fseek (fileIn, (long) 4*(lx)*(ly)*slice, SEEK_CUR);
	    fread(data, sizeof(float), size, fileIn);	      
	  } else {
		
	    float *tmp_data;
	    tmp_data = (float *) malloc(sizeof(float)*lx*ly*lz);
	    fseek (fileIn, 0 , SEEK_CUR);
	    fread(tmp_data, sizeof(float), lx*ly*lz, fileIn);	      
	    int i, j, k;
		
	    if (is_xy) {
	      size = lx*ly;
	      type = PHYSICAL;
	      image = im_new(lx, ly, size, type);
	      data = image->data;
	      if (!image)
		return GenErrorMemoryAlloc(interp);
		    
	      for(j=0;j<ly;j++){
		for(i=0;i<lx;i++){
		  data[j*lx + i]=0;
		  for(k=0;k<lz;k++){
		    if (tmp_data[k*ly*lx + j*lx + i]>data[j*lx + i]) {
		      data[j*lx + i] = tmp_data[k*ly*lx + j*lx + i];
		    }
		  }
		}
	      }
	    }
	    if (is_xz) {
	      size = lx*lz;
	      type = PHYSICAL;
	      image = im_new(lx, lz, size, type);
	      data = image->data;
	      if (!image)
		return GenErrorMemoryAlloc(interp);

	      for(k=0;k<lz;k++){
		for(i=0;i<lx;i++){
		  data[k*lx + i]=0;
		  for(j=0;j<ly;j++){
		    if (tmp_data[k*ly*lx + j*lx + i]>data[k*lx + i]) {
		      data[k*lx + i] = tmp_data[k*ly*lx + j*lx + i];
		    }
		  }
		}
	      }
	    }
	    if (is_zy) {
	      size = lz*ly;
	      type = PHYSICAL;
	      image = im_new(lz, ly, size, type);
	      data = image->data;
	      if (!image)
		return GenErrorMemoryAlloc(interp);

	      for(j=0;j<ly;j++){
		for(k=0;k<lz;k++){
		  data[j*lz + k]=0;
		  for(i=0;i<lx;i++){
		    if (tmp_data[k*ly*lx + j*lx + i]>data[j*lz + k]) {
		      data[j*lz + k] = tmp_data[k*ly*lx + j*lx + i];
		    }
		  }
		}
	      }
	    }

	    free(tmp_data);

	  }
	    
	    
	    
	}
      else {
	    
	fgets(tempBuffer, 100, fileIn);
	sscanf(tempBuffer, "%s %d %dx%d %d(%d",
	       saveFormat, &type, &lx, &ly, &size, &realSize);
	if ((type != FOURIER) && (type != PHYSICAL) && (type != FFTW_R2C))
	  return GenErrorAppend(interp, "`", imageFilename,
				"' doesn't seem to be an Image.", NULL);
	/*  if (strcmp(type, "Image"))
	    return GenErrorAppend(interp, "`", imageFilename,
	    "' doesn't seem to be an Image.", NULL);*/
	/*  size  = lx*ly;*/
	if (is_pos)
	  {
	    image = im_new(x_size, y_size, x_size*y_size, type);
	    memset (image -> data, '0', image -> size * sizeof (real));
	    if ((x_pos + x_size) > lx)
	      x_size = lx - x_pos;
	    if ((y_pos + y_size) > ly)
	      y_size = ly - y_pos;
	  }
	else
	  image = im_new(lx, ly, size, type);
	if (!image)
	  return GenErrorMemoryAlloc(interp);
	
	data = image->data;
	if (arg_present(1))	/* lecture image ascii */
	  if (is_pos)
	    {
	      sprintf (interp->result, "No \"-ascii -pos\" for the moment.\n");
	      im_free (image);
	      return TCL_ERROR;
	    }
	  else {
	    if (is_coma) {
	      for (i=0;i<size;i++) {
		fscanf(fileIn, "%f,", data+i);
	      }   
	    } else {
	      for (i=0;i<size;i++) {
		fscanf(fileIn, "%f ", data+i);
	      }
	    }
	  }
	else
	  {
	    if (realSize != sizeof(real))
	      {
		im_free (image);
		return GenErrorAppend(interp, "real size problem...", NULL);
	      }
	    if (is_pos)
	      {
		fseek (fileIn, (lx*y_pos + x_pos)*realSize, SEEK_CUR);
		for (i = 0; i < y_size; i++)
		  {
		    fread (data + i*image -> lx, realSize, x_size, fileIn);
		    fseek (fileIn, (lx - x_size)*realSize, SEEK_CUR);
		  }
	      }
	    else
	      fread(data, realSize, size, fileIn);
	    if (is_swap) {
	      BigLittleValues(image->data,image->lx*image->ly,sizeof(float));
	    }
	  }
      }
    }
  
  fclose(fileIn);
  if (!is_size)
    {
      store_image(imageName, image);
      Tcl_AppendResult(interp, imageName, NULL);
    }
  
  return TCL_OK;
}

/**************************************
 * Command name in xsmurf : iloadHdf5
 **************************************/
/*------------------------------------------------------------------------
  ImaFileLoadHdf5Cmd_
  ----------------------------------------------------------------------*/
int
ImaFileLoadHdf5Cmd_(ClientData clientData,
		    Tcl_Interp *interp,
		    int argc,
		    char **argv)      
{ 
  char * options[] = { "ss[s]",
		       "-size","ddd",
		       NULL};

  char * help_msg =
    {("Read a HDF5 file (variable=fieldname) and put it in an image named after fieldname by default or optional 3rd parameter\n"
      "\n"
      "\n")};

#ifdef USE_HDF5

  char  * imageFilename = NULL;
  char  * fieldName     = NULL; 
  char  * imageName     = NULL; 
  
  int     lx, ly, lz;
  int     i, j, k, size;

  Image * image = NULL;
  real  * data;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &imageFilename, &fieldName, &imageName) == TCL_ERROR)
    return TCL_ERROR;
  
  if (!imageName)
    imageName = fieldName;

  /*
   * Try to read HDF5 file.
   */
  
  /* Open the file */
  hid_t file_id = H5Fopen(imageFilename, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file_id >= 0)
    return GenErrorAppend(interp, "H5Fopen ",imageFilename," failed !",NULL);

  /* get dataset */
  herr_t status;
  hid_t  dataset_id;
  char str[128]; strcpy(str, "/"); strcat(str, fieldName);
  dataset_id = H5Dopen(file_id, str, H5P_DEFAULT);

  /* get dataspace */
  hid_t space_id = H5Dget_space(dataset_id);
  
  /* get datatype */
  hid_t dataType  = H5Dget_type(dataset_id);

  // check datatype (convert double to float)

  /* get dimensions for memory allocation */
  hsize_t dims[3];
  int ndims = H5Sget_simple_extent_dims(space_id, dims, NULL);

  if (ndims == 2) {
    char message[128];
    sprintf(message,"Dataset %s has rank 2 and dimensions %d %d\n",
	    fieldName,lx,ly);
    Tcl_AppendResult (interp, message, (char *) NULL);
  } else if (ndims==3) {
    char message[128];
    sprintf(message,"Dataset %s has rank 3 and dimensions %d %d %d\n",
	    fieldName,lx,ly,lz);
    Tcl_AppendResult (interp, message, (char *) NULL);
  } else {
    // not handled here
    Tcl_AppendResult (interp, "Error: ", fieldName, " has not rank 2 or 3", (char *) NULL);
  }

  /* now we can do memory allocation */
  

  /* close Id's */
  H5Sclose(space_id);
  H5Dclose(dataset_id);
  H5Fclose(file_id);

#else
  
  Tcl_AppendResult (interp, "Xsmurf was not built with HDF5 support...\n", (char *) NULL);
  Tcl_AppendResult (interp, "Try rebuild xsmurf with HDF5 enabled.\n", (char *) NULL);

#endif // USE_HDF5

  return TCL_OK;
}


/************************************
 * Command name in xsmurf : i3Dload
 ************************************/
/* pierre kestener : creee le 10 decembre 2002 */
int
Ima3DFileLoadCmd_(ClientData clientData,
		  Tcl_Interp *interp,
		  int argc,
		  char **argv)      
{ 
  char * options[] = { "s[s]",  
		       "-double","",
		       "-noheader", "dddd",
		       NULL};
  char * help_msg =
    {("read a binary file and put it in an image3D of name filename or name \n"
      "(filename,[name]). Reading is done assume data is a float\n"
      " array of size : lx,ly,lz (from header).\n"
      "\n"
      "Options :\n"
      " -double [] :  read file assuming data are double.\n"
      " -noheader [dddd] : read file without header. User must provide\n"
      "                    lx, ly, lz and type (1->Physical, 3->FFTW_R2C)\n"
      )};

  char  * imageFilename = NULL;
  char  * imageName     = NULL; 
  int     lx, ly, lz, i, size, realSize;
  unsigned int type;
  FILE  * fileIn;
  Image3D * image = NULL;
  real * data;
  char    tempBuffer[100], saveFormat[10];
  int     isDouble, isNoheader;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageFilename, &imageName) == TCL_ERROR)
    return TCL_ERROR;

  if (!imageName)
    imageName = imageFilename;

  isDouble = arg_present(1);
  isNoheader = arg_present(2);
  if (isNoheader) {
    if (arg_get (2, &lx, &ly, &lz, &type) == TCL_ERROR)
      return TCL_ERROR; 
  }

  if (!imageName)
    imageName = imageFilename;
  
  if (!(fileIn = fopen(imageFilename, "r")))
    return GenErrorAppend(interp, "Couldn't open `", imageFilename,
			  "' for reading.", NULL);
  
  /* read header */
  if (!isNoheader) {
    fgets(tempBuffer, 100, fileIn);
    sscanf(tempBuffer, "%s %d %dx%dx%d %d(%d",
	   saveFormat, &type, &lx, &ly, &lz, &size, &realSize);
    
    if ((type != FFTW_R2C) && (type != PHYSICAL))
      return GenErrorAppend(interp, "`", imageFilename,
			    "' doesn't seem to be an Image.", NULL);
  }

  if (type == PHYSICAL)
    size = lx*ly*lz;
  else
    size = lz*ly*2*(lx/2+1);

  image = im3D_new(lx,ly,lz,size,type);

  data = image->data;

  if (isDouble) {
    double *datad;
    datad = (double *) malloc(sizeof(double)*size);
    if (!datad)
      return GenErrorMemoryAlloc(interp);
    if (fread(datad, sizeof(double), size, fileIn)!=size)
      return GenErrorAppend(interp, "Couldn't properly read data (double) `", imageFilename, "'. File has not the correct size.", NULL);
    for (i=0;i<size;i++) 
      data[i] = (float) datad[i];
    free(datad);
  } else {
    if (fread(data, sizeof(float), size, fileIn)!=size)
      return GenErrorAppend(interp, "Couldn't properly read `", imageFilename,
			    "'. File has not the correct size.", NULL);
  }  

  store_image3D(imageName, image);
  Tcl_AppendResult(interp, imageName, NULL);
  

  fclose(fileIn);
  
  return TCL_OK;
}

/************************************
 * Command name in xsmurf : i3Dsave
 ************************************/
/* pierre kestener : creee le 23 janvier 2004 */
int
Ima3DSaveFileCmd_(ClientData clientData,
		  Tcl_Interp *interp,
		  int argc,
		  char **argv)      
{ 
  char * options[] = { "J[s]",  
		       "-noheader", "",
		       NULL};
  char * help_msg =
    {("write a binary file of name filename or name, with data of an image3D.\n"
      "(Image3D,[filename]). Write float data.\n"
      "\n"
      "Options :\n"
      "  -noheader []: dont write header!\n"
      )};

  char  * imageFilename = NULL;
  int     lx, ly, lz, i, size, size_edge;
  unsigned int type;
  FILE  * fileOut;
  Image3D * image = NULL;
  real * data;

  int isNoheader;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &imageFilename) == TCL_ERROR)
    return TCL_ERROR;

  isNoheader = arg_present(1);



  if (!imageFilename)
    imageFilename = argv[1];
  
  if (!(fileOut = fopen(imageFilename, "w")))
    return GenErrorAppend(interp, "Couldn't open `", imageFilename,
			  "' for writing.", NULL);
  
  lx = image->lx;
  ly = image->ly;
  lz = image->lz;
  size = image->size;
  type = image->type;

  data = image->data;

  /* write header */
  if (!isNoheader)
    fprintf(fileOut, "Binary %d %dx%dx%d %d(%d byte reals)\n",
	    image->type, lx, ly, lz, size, (int) sizeof(real));
  
  /* write data */
  if (fwrite(data, sizeof(float), size, fileOut)!=size)
    return GenErrorAppend(interp, "Couldn't properly write `", imageFilename,
			  "'. Writing unsuccessful.", NULL);

  fclose(fileOut);

  Tcl_AppendResult(interp, imageFilename, NULL);  
  return TCL_OK;
}


/* *************************************************
   routine creee le 05/09/2000 par pierre Kestener
   pour faire des fichiers-images couleur au format
   pgm (magic number = P6) contenant des cartes de
   risques pour les images de mammographie.
   *********************************************** */

/*****************************************
 * command name in xsmurf : icolorizerect
 *****************************************/

int ImaColorizeRectangleCmd_(ClientData clientData,
			     Tcl_Interp *interp,
			     int argc,
			     char **argv)      
{ 
  /* command line definition */
  char * options[] = { "IsSdS",
		       "-color2", "",
		       "-color3", "",
		       NULL};

  char * help_msg =
    {("takes a zoomed image (typically of mammography)\n"
      "and colorizes some rectangular area according to \n"
      "signals localising these rectangles.\n"
      "\n"
      "parameters :\n"
      "  - string  : name of source image\n"
      "  - string  : name of result (path)\n"
      "  - signal  : signal of position of up left corners\n"
      "             see iicut 's help message\n"
      "  - integer : size of rectangle (in pixel)\n"
      "  - signal  : signalcolor\n"
      "\n"
      "Options:\n"
      "    -color2 :\n"
      "    -color3 :\n"
      )};

  /* command's parameters */
  char     *imageFilename = NULL;
  FILE     *fileOut;
  Image    *image;
  Signal   *sig_posxy;
  Signal   *sig_color;
  int      size_rect;  /* dimension of rectangle (square in fact) */
  int      lx, ly, size;
  int      i, j, k, kx, ky;
  float    image_min, image_max;
  real     *data;
  real     *newdata;
  int      posX, posY; /* position of current square */
  int      pos;
  float    color[3], color2[3], color3[3];
  int      step = 1;

  int is_color2 = 0;
  int is_color3 = 0;
  
  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &imageFilename, &sig_posxy, &size_rect, &sig_color) == TCL_ERROR)
    return TCL_ERROR;

  if (sig_posxy->size == sig_color->size)  {
  } else {
    return GenErrorAppend(interp, "signals posxy and sigcolor must have the same size!\n", NULL);
  }
  
  is_color2 = arg_present(1);
  is_color3 = arg_present(2);

  /* treatement */
  lx   = image->lx;
  ly   = image->ly;
  size = image->size;
  data = image->data;
  im_get_extrema(image, &image_min, &image_max);

  fileOut = fopen(imageFilename, "w");
  newdata = (float *) malloc(sizeof(real)*lx*ly*3);
  /* la nouvelle image est trois fois plus grande : 3 octets/pixel
     un octet par pixel et par couleur */
  
  /* on commence par recopier toute l'image 
     en prenant toutes les couleurs egales (ca revient a faire des
     niveaux de gris) */
  for (i=0; i<lx; i++) {
    for (j=0; j<ly; j++) {
      newdata[3*(i+j*lx)]   = (data[i+j*lx]-image_min)/(image_max-image_min)*255;
      newdata[3*(i+j*lx)+1] = (data[i+j*lx]-image_min)/(image_max-image_min)*255;
      newdata[3*(i+j*lx)+2] = (data[i+j*lx]-image_min)/(image_max-image_min)*255;
    }
  }

  /*sprintf(resultBuffer, "toto %d \n", sig_color->size);
    Tcl_AppendResult (interp, resultBuffer, (char *) NULL);*/
  
  /* now we fill the squares */ 
  for (k=0; k<(sig_posxy->size); k++) {
    posX = (int) sig_posxy->dataX[k];
    posY = (int) sig_posxy->dataY[k];
    
    /* test if the upper left corner is inside the image */
    if (posX < 0 || posX >= lx || posY < 0 || posY >= ly) {
      sprintf (interp -> result,"Wrong coordinates for upper left corner number %d .\n",k+1);
      return TCL_ERROR;
    }
    /* test if the image can contain the rectangle */
    if (posX+size_rect >= lx || posY+size_rect >= ly) {
      sprintf (interp -> result,"Wrong dimensions of rectangle.\n");
      return TCL_ERROR;
    }

    /* on initialise le tableau color par du rouge */
    color[0] = 255.0;
    color[1] = 0.0;
    color[2] = 0.0;

    /* ordre des couleurs : rouge orange jaune vert bleu */

    /* on determine la couleur de remplissage */
    if (sig_color->dataY[k] >= 0.2) { /* rouge */ 
      color[0] = 255.0;
      color[1] = 0.0;
      color[2] = 0.0;
    } else if (sig_color->dataY[k] >= 0.1 ) { /* orange */
      color[0] = 255.0;
      color[1] = 128.0;
      color[2] = 0.0;
    } else if (sig_color->dataY[k] >= 0.0) { /* jaune */
      color[0] = 255.0;
      color[1] = 255.0;
      color[2] = 0.0;
    } else if (sig_color->dataY[k] >= -0.1) { /* vert */
      color[0] = 0.0;
      color[1] = 255.0;
      color[2] = 0.0;
    } else  if (sig_color->dataY[k] >= -0.3) { /* bleu ciel */                               
      color[0] = 0.0;
      color[1] = 255.0;
      color[2] = 220.0;
    } else  if (sig_color->dataY[k] >= -0.5) { /* bleu */                               
      color[0] = 0.0;
      color[1] = 0.0;
      color[2] = 255.0;
    } else {                                /* indigo-violet */
      color[0] = 255.0;
      color[1] = 0.0;
      color[2] = 255.0;
    }

    /* on determine la deuxieme couleur de remplissage */
    if (is_color2) {
      if (sig_color->dataY[k] >= 0.4) { /* rouge */ 
	color2[0] = 255.0;
	color2[1] = 0.0;
	color2[2] = 0.0;
      } else if (sig_color->dataY[k] >= 0.3 ) { /* orange */
	color2[0] = 255.0;
	color2[1] = 128.0;
	color2[2] = 0.0;
      } else if (sig_color->dataY[k] >= 0.2) { /* jaune */
	color2[0] = 255.0;
	color2[1] = 255.0;
	color2[2] = 0.0;
      } else if (sig_color->dataY[k] >= 0.1) { /* vert */
	color2[0] = 0.0;
	color2[1] = 255.0;
	color2[2] = 0.0;
      } else  if (sig_color->dataY[k] >= 0.0) { /* bleu ciel */
	color2[0] = 0.0;
	color2[1] = 255.0;
	color2[2] = 220.0;
      } else  if (sig_color->dataY[k] >= -0.2) { /* bleu */
	color2[0] = 0.0;
	color2[1] = 0.0;
	color2[2] = 255.0;
      } else {                                /* indigo-violet */
	color2[0] = 255.0;
	color2[1] = 0.0;
	color2[2] = 255.0;
      }
    }
    
    /*if (sig_color->dataY[k] <= -0.5) {
      color[0] = 255.0;
      color[1] = 0.0;
      color[2] = 0.0;
      } else if (sig_color->dataY[k] <= -0.4 ) {
      color[0] = 255.0;
      color[1] = 128.0;
      color[2] = 0.0;
      } else if (sig_color->dataY[k] <= -0.3) {
      color[0] = 255.0;
      color[1] = 255.0;
      color[2] = 0.0;
      } else if (sig_color->dataY[k] <= 0.15) { 
      color[0] = 0.0;
      color[1] = 255.0;
      color[2] = 0.0;
      } else  {                                
      color[0] = 0.0;
      color[1] = 0.0;
      color[2] = 255.0;
      }*/

    /* on determine la deuxieme couleur de remplissage */

    if (is_color3) {
      if (sig_color->dataY[k] >= 0.55) { /* rouge */ 
	color3[0] = 255.0;
	color3[1] = 0.0;
	color3[2] = 0.0;
      } else if (sig_color->dataY[k] >= 0.45) { /* orange */
	color3[0] = 255.0;
	color3[1] = 100.0;
	color3[2] = 0.0;
      } else if (sig_color->dataY[k] >= 0.38) { /* bleu ciel */
	color3[0] = 0.0;
	color3[1] = 60.0;
	color3[2] = 255.0;
      } else  { /* bleu */
	color3[0] = 0.0;
	color3[1] = 0.0;
	color3[2] = 255.0;
      }
    }
    

    /* on remplit le carre de cote size_rect */
    for (kx=0;kx<size_rect;kx+=step) {
      for (ky=0;ky<size_rect;ky+=step) {
	pos = (posX+kx) + lx*(posY+ky);
	if (is_color2) {
	  newdata[3*pos]   = color2[0];
	  newdata[3*pos+1] = color2[1];
	  newdata[3*pos+2] = color2[2];
	} else if (is_color3) {
	  newdata[3*pos]   = color3[0];
	  newdata[3*pos+1] = color3[1];
	  newdata[3*pos+2] = color3[2];	
	} else {
	  newdata[3*pos]   = color[0];
	  newdata[3*pos+1] = color[1];
	  newdata[3*pos+2] = color[2];
	}
      }
    }
    
  }
  
  /* finally we write the colorized image */
  fprintf(fileOut,"P6\n%d %d\n",lx,ly);
  fprintf(fileOut,"255\n");
  for(i = 0; i < 3*lx*ly; i++)
    {
      fprintf(fileOut,"%c",(unsigned char)((int)(newdata[i])));
    }

  fclose(fileOut);
  free(newdata);

  return TCL_OK;
  /*  Tcl_AppendResult(interp, imageFilename, NULL);*/
  
   
}

/* *************************************************
   routine creee le 31/03/2003 par pierre Kestener
   pour faire des fichiers-images couleur au format
   pgm (magic number = P6) contenant des cartes de
   risques pour les images de mammographie.
   *********************************************** */

/******************************************
 * command name in xsmurf : icolorizerect2
 ******************************************/

int ImaColorizeRectangle2Cmd_(ClientData clientData,
			      Tcl_Interp *interp,
			      int argc,
			      char **argv)      
{ 
  /* command line definition */
  char * options[] = { "IsSdSSSS",
		       "-hminmax", "ff",
		       NULL};

  char * help_msg =
    {("takes a zoomed image (typically of mammography)\n"
      "and colorizes some rectangular area according to \n"
      "signals localising these rectangles.\n"
      "\n"
      "parameters :\n"
      "  - string  : name of source image\n"
      "  - string  : name of result (path)\n"
      "  - signal  : signal of position of up left corners\n"
      "             see iicut 's help message\n"
      "  - integer : size of rectangle (in pixel)\n"
      "  - signal  : hvalues signal\n"
      "  - 3 signals : R,G and B (signals to encode colors.\n"
      "\n"
      "Options:\n"
      " -hminmax [ff] : this option allows to modify hmin and hmax values\n"
      "                 to rescale colors.\n"
      )};

  /* command's parameters */
  char     *imageFilename = NULL;
  FILE     *fileOut;
  Image    *image;
  Signal   *sig_posxy;
  Signal   *sig_hvalues;
  Signal   *sigR, *sigG, *sigB;
  int      size_rect;  /* dimension of rectangle (square in fact) */
  int      lx, ly, size;
  int      i, j, k, kx, ky;
  float    image_min, image_max;
  real     *data;
  real     *newdata;
  int      posX, posY; /* position of current square */
  int      pos;
  float    color[3], color2[3], color3[3];
  int      step = 3;
  int      nbColor, indexColor;

  int    isHminmax;
  float  hmin = 0.0;
  float  hmax = 1.0;
  
  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &imageFilename, &sig_posxy, &size_rect, &sig_hvalues, &sigR, &sigG, &sigB) == TCL_ERROR)
    return TCL_ERROR;

  if (sig_posxy->size == sig_hvalues->size)  {
  } else {
    return GenErrorAppend(interp, "signals posxy and sigcolor must have the same size!\n", NULL);
  }


  
  isHminmax = arg_present(1);
  if (isHminmax)
    if (arg_get (1, &hmin, &hmax) == TCL_ERROR)
      return TCL_ERROR;
  
  /* treatement */
  lx   = image->lx;
  ly   = image->ly;
  size = image->size;
  data = image->data;
  im_get_extrema(image, &image_min, &image_max);

  
  
  fileOut = fopen(imageFilename, "w");
  newdata = (float *) malloc(sizeof(real)*lx*ly*3);
  /* la nouvelle image est trois fois plus grande : 3 octets/pixel
     un octet par pixel et par couleur 
     (c'est la difference entre les nombres "magiques" P5 (niveaux de gris ou
     PGM) et P6 (couleurs ou PPM)...
     sinon faire man pgm
  */
  
  /* on commence par recopier toute l'image 
     en prenant toutes les couleurs egales (ca revient a faire des
     niveaux de gris) */
  for (i=0; i<lx; i++) {
    for (j=0; j<ly; j++) {
      newdata[3*(i+j*lx)]   = (data[i+j*lx]-image_min)/(image_max-image_min)*255;
      newdata[3*(i+j*lx)+1] = (data[i+j*lx]-image_min)/(image_max-image_min)*255;
      newdata[3*(i+j*lx)+2] = (data[i+j*lx]-image_min)/(image_max-image_min)*255;
    }
  }

  /*sprintf(resultBuffer, "toto %d \n", sig_hvalues->size);
    Tcl_AppendResult (interp, resultBuffer, (char *) NULL);*/
  
  /* now we fill the squares */ 
  for (k=0; k<(sig_posxy->size); k++) {

    /* determine filling color according to th H value ! */
    if (sig_hvalues->dataY[k]<hmin) {
      indexColor = 0;
    } else if (sig_hvalues->dataY[k]>=hmax) {
      indexColor = 255;
    } else {
      indexColor = (int) 255/(hmax-hmin)*(sig_hvalues->dataY[k]-hmin);
    }

    posX = (int) sig_posxy->dataX[k];
    posY = (int) sig_posxy->dataY[k];
    
    /* test if the upper left corner is inside the image */
    if (posX < 0 || posX >= lx || posY < 0 || posY >= ly) {
      sprintf (interp -> result,"Wrong coordinates for upper left corner number %d .\n",k+1);
      return TCL_ERROR;
    }
    /* test if the image can contain the rectangle */
    if (posX+size_rect >= lx || posY+size_rect >= ly) {
      sprintf (interp -> result,"Wrong dimensions of rectangle.\n");
      return TCL_ERROR;
    }

 
    /* on remplit le carre de cote size_rect */
    for (kx=0;kx<size_rect;kx+=step) {
      for (ky=0;ky<size_rect;ky+=step) {
	pos = (posX+kx) + lx*(posY+ky);
	
	newdata[3*pos]   = sigR->dataY[indexColor];
	newdata[3*pos+1] = sigG->dataY[indexColor];
	newdata[3*pos+2] = sigB->dataY[indexColor];
	
      }
    }
    
  }
  
  /* finally we write the colorized image */
  fprintf(fileOut,"P6\n%d %d\n",lx,ly);
  fprintf(fileOut,"255\n");
  for(i = 0; i < 3*lx*ly; i++)
    {
      fprintf(fileOut,"%c",(unsigned char)((int)(newdata[i])));
    }

  fclose(fileOut);
  free(newdata);

  return TCL_OK;
  /*  Tcl_AppendResult(interp, imageFilename, NULL);*/
  
   
}

/* **************************************************
 * Command name in xsmurf : igrondf
 * **************************************************/

int Ima_g_rond_f_Cmd_(ClientData clientData,
		      Tcl_Interp *interp,
		      int argc,
		      char **argv)      
{ 
  /* command line definition */
  char * options[] = { "IsS",
		       NULL};

  char * help_msg =
    {("takes an image (typically of mammography)\n"
      "and applies a point-by-point composition operation \n"
      "so that image i(x,y) becomes j(x,y)=f(i(x,y).\n"
      "f is a piecewise linear function defined by\n"
      "an XY-SIGNAL.\n"
      "\n"
      "parameters :\n"
      "  - image   : name of source image\n"
      "  - string  : name of result (new image)\n"
      "  - signal  : signal defining function f.\n"
      "\n"
      "See srealloc in tcl_library/hpcal_proc.tcl!!\n"
      "Options:\n"
      "    no_option :\n"
      )};

  /* command's parameters */
  Image    *image;
  Image    *res;
  char     *ima_res_name;
  Signal   *sig_f;
  int      lx, ly, size;
  int      i, j;
  float    image_min, image_max;
  int      f_size;
  Signal   *slope; /* signal contenant les pentes des morceaux */
  real     *data, *datares, *datafx, *datafy;

  
  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &ima_res_name, &sig_f) == TCL_ERROR)
    return TCL_ERROR;

  /* treatement */
  lx     = image->lx;
  ly     = image->ly;
  size   = image->size;
  data   = image->data;
  im_get_extrema(image, &image_min, &image_max);
  f_size = sig_f->size;
  datafx = sig_f->dataX;
  datafy = sig_f->dataY;

  res     = im_new (lx, ly, lx*ly, PHYSICAL);
  im_set_0(res); /* initialisation a zero (pourquoi pas !?!)*/
  datares = res->data;

  /* calcul des pentes */
  slope = sig_new(REALY, 0 , f_size-2);
  for (j=0;j<f_size-1;j++) {
    slope->dataY[j] = (datafy[j+1]-datafy[j])/(datafx[j+1]-datafx[j]);
  }
  
  /* main : on remplit l'image res*/
  for (i=0;i<size;i++) { /* boucle sur les points de l'image */
    for (j=0;j<f_size;j++) { /* on cherche sur quel segment on est */
      if (data[i] >= datafx[j] && data[i] < datafx[j+1]) {
	datares[i] = datafy[j] + slope->dataY[j]*(data[i]-datafx[j]);
      } else {
      }      
    }
  }
  
  store_image (ima_res_name, res);
  sig_free(slope);
  
  return TCL_OK;
  
}



/* **************************************************
   routine creee le 31/08/2000 par Pierre Kestener
   pour superposer sur les images des billes de latex 
   (de Francois) leur trajectoires
   * **************************************************/
/*
 * Command name in xsmurf : iaddtraj
 */
int
AddTrajBillesLatexCmd_(ClientData clientData,
		       Tcl_Interp *interp,
		       int argc,
		       char **argv)      
{ 
  char * options[] = { "ss",  
		       "-chris2", "SS",
		       NULL};
  char * help_msg =
    {("read a binary file (filenameIn) of a film of moving latex marbles and write another (filenameOut).\n"
      "adding the trajectories of each marble.\n"
      "  -chris2     : this option is there only to keep in mind that file\n"
      "                must be chris2 file.\n"
      "        - Signal Signal : two XY-signals (trajectories of the marbles).\n")};
  char  * imageFilenameIn = NULL;
  char  * imageFilenameOut = NULL;
  int     lx, ly, i, size, type=-1;
  char resultBuffer[200];
  
  FILE  * fileIn;
  FILE  * fileOut;
  Image * image = NULL;
  real * data;
  real * dataX1;
  real * dataX2;
  real * dataY1;
  real * dataY2;

  int is_chris2 = 0;
  Signal * signal1;
  Signal * signal2;

  /* begin chris2*/
  /* #define byte unsigned char */
  unsigned char *imagefile;
  unsigned char gah_data[1000];
  int npar,idata,ntotdonnees,ilecture,idecal,ntot;
  /*   idecal parametre de decalage eventuel du tableau (multiple de 128)  */
  unsigned char *charo;
  
  int Xpixels, Ypixels, nframes, ind_frames;
  int j;
  int i1;
  float min, max;

  /* end  chris2 */

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageFilenameIn, &imageFilenameOut) == TCL_ERROR)
    return TCL_ERROR;
  
  if (!(fileIn = fopen(imageFilenameIn, "r")))
    return GenErrorAppend(interp, "Couldn't open `", imageFilenameIn,
			  "' for reading.", NULL);

  if (!(fileOut = fopen(imageFilenameOut, "w")))
    return GenErrorAppend(interp, "Couldn't open `", imageFilenameOut,
			  "' for writing.", NULL);

  
  is_chris2  = arg_present(1);
  /*is_chris2 = 1;*/
  if (is_chris2) {
    if (arg_get (1, &signal1, &signal2) == TCL_ERROR)
      return TCL_ERROR;
  } else {
    return GenErrorAppend(interp, "No chris2 option !!", NULL);
  }

  idecal = 0;
  idata  = 768;
  npar =1000;
  ntot = 300;
  /* on lit les idata premiers bytes du fichiers !*/
  ilecture = fread(gah_data,1,idata,fileIn);
  fwrite(gah_data,1,idata,fileOut);

  charo = &gah_data[idecal + 30];
  sprintf(resultBuffer, "%d   %d\n",charo[0],charo[1]);
  Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
  Xpixels = charo[0]*256 + charo[1];
  sprintf(resultBuffer, "largeur de l'image : %d   \n",Xpixels);
  Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
  charo = &gah_data[idecal + 42];
  sprintf(resultBuffer, "%d   %d\n",charo[0],charo[1]);
  Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
  Ypixels = charo[0]*256 + charo[1];
  sprintf(resultBuffer, "longueur de l'image %d   \n",Ypixels);
  Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
  /*      charo = &gah_data[idecal + 516];*/
  charo = &gah_data[idecal + 516];
  sprintf(resultBuffer, "%d   %d\n",charo[0],charo[1]);
  Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
  nframes = charo[0]*256 + charo[1];
  sprintf(resultBuffer, "nombre d'images %d   \n",nframes);
  Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
  ntotdonnees = Xpixels * Ypixels;
  
  imagefile = (unsigned char *) malloc(ntotdonnees*sizeof(char));
  sprintf(resultBuffer, "taille de l'image  %d  \n",ntotdonnees);
  Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
  
  lx = Xpixels;
  ly = Ypixels;
  size = lx*ly;
  type = PHYSICAL;
  image = im_new(lx, ly, size, type);
  data = image->data;
  if (!image)
    return GenErrorMemoryAlloc(interp);
  dataX1 = signal1->dataX;
  dataY1 = signal1->dataY;
  dataX2 = signal2->dataX;
  dataY2 = signal2->dataY;


  
  /* on lit chacune des images du film et on trace les trajectoires */
  for (ind_frames = 0; ind_frames<nframes-1; ind_frames++){
    ilecture = fread(imagefile,1,ntotdonnees,fileIn);
    if (ilecture!=ntotdonnees) {
      sprintf(resultBuffer, "pb lecture image: %d \n",ind_frames+1);
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);
    }

    /* on lit l'image courante */
    for (i=0; i<Xpixels; i++) {
      for (j=0; j<Ypixels; j++) {
	i1 =14 + (int) (((float) imagefile[i + j*Xpixels])* 241.0 / 255.0);
	data[i + j*Xpixels] = (real) i1;
      }
    }
    /* cherche le minimun de l'image */
    im_get_extrema (image,&min,&max);
    /* on trace les trajectoires */
    for (i=0; i<signal1->n; i++) {
      imagefile[(int) (dataX1[i] + Xpixels * dataY1[i])] = (unsigned char)((int)(image->min));
      imagefile[(int) (dataX2[i] + Xpixels * dataY2[i])] = (unsigned char)((int)(image->min));
    }
    /* on revient en arriere pour ecrire dans le fichier */
    /*fseek(fileIn, (long)(-ntotdonnees), SEEK_CUR);*/
    /* on ecrit */
    fwrite(imagefile, 1, ntotdonnees, fileOut);
    /* et voila !! */
    /*sprintf(resultBuffer, "toto  \n");
      Tcl_AppendResult (interp, resultBuffer, (char *) NULL);*/
  }
  free(imagefile);

  fclose(fileOut);
  return TCL_OK;
}




#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
ImaAdnLoadCmd_(ClientData clientData,
	       Tcl_Interp *interp,
	       int        argc,
	       char       **argv)      
{ 
  char * options[] = { "s[s]",
		       NULL };

  char * help_msg =
    {("read an dna file and make an image (filename,[name]).")};

  int dim;
  Image * image;
  char * imageName = NULL, *fileName = NULL;

  int  x_max = 0, y_max = 0;
  int  x_min = 0, y_min = 0;
  int  x = 0, y = 0;
  FILE *f_in;
  FILE *f2_in;
  char header[100];
  int  size;
  int  i;
  char c, c2;
 
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0,&fileName,&imageName)==TCL_ERROR)
    return TCL_ERROR;

  if(!imageName)
    imageName = fileName;

  f_in = fopen (fileName, "r");
  fgets (header, 100, f_in);
  sscanf(header, "%d", &size);
  for (i = 0; i < size; i++)
    {
      fscanf(f_in, "%c", &c);
      switch (c)
	{
	case 'A':
	  x++;
	  break;
	case 'G':
	  y++;
	  break;
	case 'T':
	  x--;
	  break;
	case 'C':
	  y--;
	  break;
	default:
	  break;
	}
      x_max = max (x_max, x);
      y_max = max (y_max, y);
      x_min = min (x_min, x);
      y_min = min (y_min, y);
    }
  fclose (f_in);
  dim = max (x_max - x_min, y_max - y_min);
  image = im_new (dim,dim,dim*dim,PHYSICAL);
  if (!image)
    return GenErrorMemoryAlloc(interp);

  im_set_0 (image);
  f2_in = fopen (fileName, "r");
  fgets (header, 100, f2_in);
  sscanf(header, "%d", &size);
  x = y = 0;
  for (i = 0; i < size; i++)
    {
      fscanf(f2_in, "%c", &c2);
      switch (c2)
	{
	case 'A':
	  x++;
	  break;
	case 'G':
	  y++;
	  break;
	case 'T':
	  x--;
	  break;
	case 'C':
	  y--;
	  break;
	default:
	  break;
	}
      image->data[(x-x_min)+(y-y_min)*dim] += 1.0;
    }

  store_image(imageName,image);
  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}

/*
 */
int
im_null_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{ 
  char * options[] = { "sd[d]",
		       NULL };

  char * help_msg =
    {("Create a null image.\n")};

  int   lx, ly = -1;
  Image *image;
  char  *imageName;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &lx, &ly)==TCL_ERROR)
    return TCL_ERROR;
  if (ly == -1) {
    ly = lx;
  }

  image = im_new (lx, ly, lx*ly, PHYSICAL);

  if (!image)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (image);
  
  store_image(imageName,image);

  Tcl_AppendResult(interp,imageName,NULL);
  return TCL_OK;
}

/*
 */
int
im_set_border_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  char * options[] = { "Idddd[f]",
		       NULL };

  char * help_msg =
    {("Set points outside a box to a specific value (default is 0.0).\n")};

  Image *image;
  int   x_min;
  int   y_min;
  int   x_max;
  int   y_max;
  real  value = 0.0;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image,
	      &x_min, &y_min, &x_max, &y_max,
	      &value)==TCL_ERROR)
    return TCL_ERROR;

  image = im_set_border (image, x_min, y_min, x_max, y_max, value);

  return TCL_OK;
}


/*
 * command name in xsmurf : igfft
 */
int
im_gfft2d_real_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  char * options[] = { "Is",
		       "-reverse", "",
		       NULL };

  char * help_msg =
    {("Fourier transform of an image using the gfft lib.\n"
      "\n"
      "Options :\n"
      "  -reverse\n")};

  int is_reverse;

  Image *image;
  Image *result;
  char *res_name;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &res_name)==TCL_ERROR)
    return TCL_ERROR;

  is_reverse = arg_present (1);

  result = im_new (image -> lx, image -> ly, image -> size, PHYSICAL);

  smSetBeginTime();
  if (is_reverse)
    im_gifft2d_real ((complex *)image -> data, result -> data,
		     image -> lx, image -> ly);
  else
    im_gfft2d_real (image -> data, (complex *) result -> data,
		    image -> lx, image -> ly);
  smSetEndTime();

  store_image(res_name, result);
  sprintf( interp->result, "%f", smGetEllapseTime());
  return TCL_OK;
}


/************************************
 * command name in xsmurf : ifftw2d
 ************************************/
int
im_fftw2d_real_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  char * options[] = { "I[s]",
		       "-threads", "d",
		       NULL };

  char * help_msg =
    {("Fourier transform of an image using the fftw2 lib.\n"
      "Computations are done in place if no name for result is provided.\n"
      "\n"
      "Notice :\n"
      "  when doing in place computation, take care that image type\n"
      "  is toggled PHYSICAL/FFTW_R2C after !!!\n"
      "\n"
      "Options :\n"
      "  -threads [d]: specify the number of threads to use (this option is\n"
      "                only valid when xsmurf has been compiled the threaded\n"
      "                version of fftw).\n")};

  int is_inplace;

  Image *image;
  Image *result;
  char *res_name = NULL;

  int lx,ly,Lx,Ly;

  int isThreaded=0;
  int nThreads=1;
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &res_name)==TCL_ERROR)
    return TCL_ERROR;

  if (!res_name)
    is_inplace = 1;
  else
    is_inplace = 0;
  
  if (!res_name)
    res_name = argv[1];
  
  isThreaded = arg_present(1);
  if (isThreaded)
    if (arg_get (1, &nThreads) == TCL_ERROR)
      return TCL_ERROR;
  
  lx=image->lx;
  ly=image->ly;

  /*
   * take care that we have to swap lx and ly for fftw 
   * because fftw uses row-major whereas xsmurf uses col-major
   * 
   */
  Lx = ly;
  Ly = lx;

  /* allocation a new image for result when out_of_place */
  if ((!is_inplace) && (image->type==PHYSICAL)) {
    result = im_new (Lx, Ly, Lx*2*(Ly/2+1), FFTW_R2C);
  } else if ((!is_inplace) && (image->type==FFTW_R2C)) {
    result = im_new (Lx, Ly, Lx*Ly, PHYSICAL);
  }

  /* enlarge data when doing inplace transform */
  if ((is_inplace == 1) && (image->type==PHYSICAL)) {
    image->data = (float *) realloc (image->data, Lx*2*(Ly/2+1)*sizeof(float));
    if (image->data == NULL)
      return GenErrorMemoryAlloc(interp);
    /* NOTICE : data re-ordering is done inside im_fftw2d_real */
  }
    
  smSetBeginTime();
  if (is_inplace) {
    if (image->type == PHYSICAL) { /* direct */
      im_fftw2d_real (image->data, NULL, Lx, Ly, 0, is_inplace, nThreads);
    } else if (image->type == FFTW_R2C) { /* reverse */
      im_fftw2d_real (image->data, NULL, Lx, Ly, 1, is_inplace, nThreads);
    }
  } else {
    if (image->type == PHYSICAL) { /* direct */
      im_fftw2d_real (image->data, result->data, Lx, Ly, 0, is_inplace, nThreads);
    } else if (image->type == FFTW_R2C) { /* reverse */
      im_fftw2d_real (image->data, result->data, Lx, Ly, 1, is_inplace, nThreads);
    }
  }
  smSetEndTime();

  if (!is_inplace) {
    /* register the new image */
    store_image(res_name, result);
  } else { /* in place */
    if (image->type==PHYSICAL) {
      /* change image type and size */
      image->type = FFTW_R2C;
      /*image->lx = Lx;
	image->ly = Ly;*/
      image->size = Lx*2*(Ly/2+1);
    } else if (image->type==FFTW_R2C) {
      /* shorten data */
      image->data = (float *) realloc (image->data, Lx*Ly*sizeof(float));
      image->type = PHYSICAL;
      /*image->lx = Lx;
	image->ly = Ly;*/
      image->size = Lx*Ly;
    }
  }
  sprintf( interp->result, "%f", smGetEllapseTime());
  return TCL_OK;
}


/************************************
 * command name in xsmurf : ifftw3d
 ************************************/
int
im_fftw3d_real_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  char * options[] = { "J[s]",
		       "-threads", "d",
		       NULL };

  char * help_msg =
    {("Fourier transform of a 3D image using the fftw2 lib.\n"
      "Computations are done in place if no name for result is provided.\n"
      "\n"
      "Notice :\n"
      "  when doing in place computation, take care that image type\n"
      "  is toggled PHYSICAL/FFTW_R2C after !!!\n"
      "\n"
      "Options :\n"
      "  -threads [d]: specify the number of threads to use (this option is\n"
      "                only valid when xsmurf has been compiled the threaded\n"
      "                version of fftw).\n")};

  int is_inplace;

  Image3D *image;
  Image3D *result;
  char *res_name = NULL;

  int lx,ly,lz,Lx,Ly,Lz;

  int isThreaded=0;
  int nThreads=1;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &res_name)==TCL_ERROR)
    return TCL_ERROR;

  if (!res_name)
    is_inplace = 1;
  else
    is_inplace = 0;

  if (!res_name)
    res_name = argv[1];
  
  isThreaded = arg_present(1);
  if (isThreaded)
    if (arg_get (1, &nThreads) == TCL_ERROR)
      return TCL_ERROR;

  lx=image->lx;
  ly=image->ly;
  lz=image->lz;

  /*
   * take care that we have to swap lx and lz for fftw 
   * because fftw uses row-major whereas xsmurf uses col-major
   * 
   */
  Lx = lz;
  Ly = ly;
  Lz = lx;

  /* allocation a new image for result when out_of_place */
  if ((!is_inplace) && (image->type==PHYSICAL)) {
    result = im3D_new (Lx, Ly, Lz, Lx*Ly*2*(Lz/2+1), FFTW_R2C);
  } else if ((!is_inplace) && (image->type==FFTW_R2C)) {
    result = im3D_new (Lx, Ly, Lz, Lx*Ly*Lz, PHYSICAL);
  }

  /* enlarge data when doing inplace transform */
  if ((is_inplace == 1) && (image->type==PHYSICAL)) {
    image->data = (float *) realloc (image->data, Lx*Ly*2*(Lz/2+1)*sizeof(float));
    if (image->data == NULL)
      return GenErrorMemoryAlloc(interp);
    /* NOTICE : data re-ordering is done inside im_fftw3d_real */
  }
    
  smSetBeginTime();
  if (is_inplace) {
    if (image->type == PHYSICAL) { /* direct */
      im_fftw3d_real (image->data, NULL, Lx, Ly, Lz, 0, is_inplace, nThreads);
    } else if (image->type == FFTW_R2C) { /* reverse */
      im_fftw3d_real (image->data, NULL, Lx, Ly, Lz, 1, is_inplace, nThreads);
    }
  } else {
    if (image->type == PHYSICAL) { /* direct */
      im_fftw3d_real (image->data, result->data, Lx, Ly, Lz, 0, is_inplace, nThreads);
    } else if (image->type == FFTW_R2C) { /* reverse */
      im_fftw3d_real (image->data, result->data, Lx, Ly, Lz, 1, is_inplace, nThreads);
    }
  }

  smSetEndTime();

  if (!is_inplace) {
    /* register the new image */
    store_image3D(res_name, result);
  } else { /* in place */
    if (image->type==PHYSICAL) {
      /* change image type and size */
      image->type = FFTW_R2C;
      /*image->lx = Lx;
	image->ly = Ly;*/
      image->size = Lx*Ly*2*(Lz/2+1);
    } else if (image->type==FFTW_R2C) {
      /* shorten data */
      image->data = (float *) realloc (image->data, Lx*Ly*Lz*sizeof(float));
      image->type = PHYSICAL;
      /*image->lx = Lx;
	image->ly = Ly;*/
      image->size = Lx*Ly*Lz;
    }
  }
  sprintf( interp->result, "%f", smGetEllapseTime());
  return TCL_OK;
}


/*----------------------------------------------------------------------
  _QuickSort_
  
  Fonction qui classe dans l'ordre croissant le tableau x de taille n.
  --------------------------------------------------------------------*/
static void 
_QuickSort_(double *x,
	    int    n)
{
  int    l, j, ir, i;
  double xx;
  
  l = (n>>1) + 1;
  ir = n;

  /*  L'index l est decremente de sa valeur initiale vers 1 pendant
   *  la periode de creation de l'arbre, une fois 1 atteint, l'index ir
   *  decremente de sa valeur initiale vers 1 pendant la phase de
   *  selection entre branches */

  for(;;) {
    if(l > 1)
      xx = x[--l];
    else {
      xx = x[ir];
      x[ir] = x[1];
      if(--ir == 1) {
	x[1] = xx;
	return;
      }
    }
    i = l;
    j = l<<1;
    while(j <= ir) {
      if(j < ir && x[j] < x[j+1])
	++j;
      if(xx < x[j]) {
	x[i] = x[j];
	j += (i = j);
      }
      else j = ir + 1;
    }
    x[i] = xx;
  }
}

/*----------------------------------------------------------------------
  _QuickSort_
  
  Fonction qui classe dans l'ordre croissant le tableau x de taille n.
  --------------------------------------------------------------------*/
/*static void 
  _QuickSort_float_(float *x,
  int    n)
  {
  int    l, j, ir, i;
  float xx;
  
  l = (n>>1) + 1;
  ir = n;

  for(;;) {
  if(l > 1)
  xx = x[--l];
  else {
  xx = x[ir];
  x[ir] = x[1];
  if(--ir == 1) {
  x[1] = xx;
  return;
  }
  }
  i = l;
  j = l<<1;
  while(j <= ir) {
  if(j < ir && x[j] < x[j+1])
  ++j;
  if(xx < x[j]) {
  x[i] = x[j];
  j += (i = j);
  }
  else j = ir + 1;
  }
  x[i] = xx;
  }
  }
*/

/*********************************
 * Commande name in xsmurf : ifct
 *********************************/
int
apply_fct_to_i_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Is[f]",
      "-domain_out", "ff",
      "-weight", "I",
      NULL
    };

  char * help_msg =
    {
      (" Applies a function to each value of an image and sums all the\n"
       "values. The function can have two parameters.\n"
       "\n"
       "Parameters :\n"
       "  image  - image to treat.\n"
       "  string - an expression designing the function (defunc syntax).\n"
       "  [real] - facultative value of a the second parameter of the\n"
       "           function.\n"
       "\n"
       "Options :\n"
       "  -domain_out : Define a domain. The function only applies to values\n"
       "                that are out of this domain.\n"
       "     2 integers - bounds of the domain.\n"
       "  -weight     : does a weighted sum....\n"
       "     1 image  : the weights\n"
       )
    };

  /* Command's parameters */
  Image *image;
  char  *fct_expr;
  real  scd_value = 0;

  /* Options's presence */
  int is_domain_out;
  int is_weight;

  /* Options's parameters */
  real out_min;
  real out_max;
  Image *image_weight;

  /* Other variables */
  void     *fct;
  double   result = 0.0;
  double   *values;
  int      i;
  int      nb_of_values = 0;
  real     *data;
  real     *data_weight;

  char      buffer[] = "x*x";

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

  is_domain_out = arg_present (1);
  if (is_domain_out)
    if (arg_get (1, &out_min, &out_max) == TCL_ERROR)
      return TCL_ERROR;
  
  is_weight = arg_present (2);
  if (is_weight)
    if (arg_get (2, &image_weight) == TCL_ERROR)
      return TCL_ERROR;
  
  /* Parameters validity and initialisation */
  fct = evaluator_create (fct_expr);
  assert (fct);

  if (is_domain_out && out_max < out_min) {
    sprintf (interp->result, "Bad domain ([%f, %f]).", out_min, out_max);
    return TCL_ERROR;
  }

  /* Treatement */
  data = image -> data;
  values = (double *) malloc (image -> size*sizeof (double));
  smSetBeginTime();
  if (is_weight) {
    data_weight = image_weight -> data;
    for (i = 0; i < image -> size; i++)
      if (!is_domain_out || data[i] < out_min || data[i] > out_max) {
	values[nb_of_values] = (float) evaluator_evaluate_x_y (fct, data[i], scd_value) * data_weight[i];
	nb_of_values++;
      }
  } else {
    for (i = 0; i < image -> size; i++)
      if (!is_domain_out || data[i] < out_min || data[i] > out_max) {
	values[nb_of_values] = (float) evaluator_evaluate_x_y (fct, data[i], scd_value);
	nb_of_values++;
      }
  }
  smSetEndTime();

  smSetBeginTime();
  if (nb_of_values > 1)
    _QuickSort_ (values-1, nb_of_values);
  smSetEndTime();

  for (i = 0; i < nb_of_values; i++)
    result += values[i];

  evaluator_destroy (fct);
  free (values);

  if (result == 0) {
    sprintf (interp -> result, "0");
  } else {
    sprintf (interp -> result, "%.15e", result);
  }

  return TCL_OK;
}


/****************************************
 * Commande name in xsmurf : ifct_vector
 ****************************************/
int
apply_fct_to_i_vector_TclCmd_ (ClientData clientData,
			       Tcl_Interp *interp,
			       int        argc,
			       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "IIds[f]",
      NULL
    };
  
  char * help_msg =
    {
      (" Applies a function to the modulus of a coarse-grained version of 2D\n"
       " vector-valued field (two images, one for each component of the \n"
       " field), that is it sums all the vectors before applying the function.\n"
       " Note : the function can have two parameters.\n"
       "\n"
       "Parameters :\n"
       "  two images  - images to treat.\n"
       "  integer     - coarse-grained parameter.\n"
       "  string      - an expression designing the function (defunc syntax).\n"
       "  [real]      - facultative value of a the second parameter of the\n"
       "           function.\n"
       "\n"
       "Options :\n"
       "\n"
       )
    };

  /* Command's parameters */
  Image *image1, *image2;
  int depth;
  char  *fct_expr;
  real  scd_value = 0;

  /* Options's presence */
  
  /* Options's parameters */
  
  /* Other variables */
  void     *fct;
  double   result = 0.0;
  double   *values1,*values2;
  int      i,j;
  real     *data1, *data2;
  int cell_size;
  int cell_edge;
  int nb_cells ;
  int lx,ly;
  int x,y,xx,yy;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2, &depth, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

     
  /* Parameters validity and initialisation */
  fct = evaluator_create(fct_expr);
  if (!fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  
  /* Treatement */
  lx = image1 -> lx;
  ly = image1 -> ly;
  data1 = image1 -> data;
  data2 = image2 -> data;
  values1 = (double *) malloc (image1 -> size*sizeof (double));
  values2 = (double *) malloc (image2 -> size*sizeof (double));
 
  cell_size = (int) pow(2.0,(double) depth);
  cell_edge = lx / cell_size;
  nb_cells  = (int) pow(4.0,(double) depth);
  for (j = 0; j < nb_cells ; j++) {
    values1[j] = 0.0;
    values2[j] = 0.0;
  }

  for (i = 0; i < lx*ly; i++) {
    x  = i % lx;
    y  = i / lx;
    xx = x / cell_edge;
    yy = y / cell_edge;
    j = xx + yy*cell_size;
    values1[j] += (double) data1[i];
    values2[j] += (double) data2[i];
    //nb_of_values++;
  }

  {
    double tmp;
    for (j = 0; j < nb_cells ; j++) {
      tmp = sqrt(values1[j]*values1[j]+values2[j]*values2[j]);
      values1[j] = evaluator_evaluate_x_y (fct, tmp, scd_value);
    }
  }
    
  
  for (i = 0; i < nb_cells; i++)
    result += values1[i];

  evaluator_destroy (fct);
  free (values1);
  free (values2);

  if (result == 0) {
    sprintf (interp -> result, "0");
  } else {
    sprintf (interp -> result, "%.15e", result);
  }

  return TCL_OK;
}


/**************************************
 * Commande name in xsmurf : i3Dfct
 **************************************/
/* created by pierre kestener : december 10th 2002 */
/* modified by pierre kestener : january 9th 2003 : option is_coarse */
int
apply_fct_to_i3D_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Js[f]",
      "-domain", "dddddd",
      "-weight", "J",
      "-coarse", "d",
      NULL
    };

  char * help_msg =
    {
      (" Applies a function to each value of an image3D and sums all the\n"
       "values. The function can have two parameters.\n"
       "\n"
       "Parameters :\n"
       "  image3D  - image3D to treat.\n"
       "  string   - an expression designing the function (defunc syntax).\n"
       "  [real]   - facultative value of a the second parameter of the\n"
       "             function.\n"
       "\n"
       "Options :\n"
       "  -domain [] : Define a domain a la \"ecut\". See corresponding help\n"
       "               message. The function only applies to values\n"
       "               that are inside the domain.\n"
       "     6 integers - x1, y1, z1 and x2, y2, z2.\n"
       "  -weight [] : does a weighted sum....\n"
       "     1 image3D  : the weights\n"
       "  -coarse [] : see help message of \"ifct3D\".\n"
       "     1 integer  : depth\n"
       )
    };

  /* Command's parameters */
  Image3D *image;
  char  *fct_expr;
  real  scd_value = 0;
  int lx, ly, lz;

  /* Options's presence */
  int is_domain;
  int is_weight;
  int is_coarse;

  /* Options's parameters */
  int x1, y1, z1, x2, y2, z2;
  int posx, posy, posz;
  Image3D *image_weight;
  int depth;
  int cell_size;
  int cell_edge;
  int nb_cells ;

  /* Other variables */
  void     *fct;
  double   result = 0.0;
  double   *values;
  int      i, j;
  int      nb_of_values = 0;
  real     *data;
  real     *data_weight;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

  is_domain = arg_present (1);
  if (is_domain)
    if (arg_get (1, &x1, &y1, &z1, &x2, &y2, &z2) == TCL_ERROR)
      return TCL_ERROR;
  
  is_weight = arg_present (2);
  if (is_weight)
    if (arg_get (2, &image_weight) == TCL_ERROR)
      return TCL_ERROR;
  
  is_coarse = arg_present (3);
  if (is_coarse)
    if (arg_get (3, &depth) == TCL_ERROR)
      return TCL_ERROR;
  
  /* Parameters validity and initialisation */
  fct = evaluator_create(fct_expr);
  if (!fct)
    {
      Tcl_AppendResult (interp, "libmatheval :error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }

  lx = image->lx;
  ly = image->ly;
  lz = image->lz;
  
  if (is_domain) {
    if (x1 < 0 || y1 < 0 || z1 < 0 || x2 >= lx || y2 >= ly || z2 >= lz ) {
      sprintf (interp->result, "Bad domain ([%d %d %d %d %d %d]).", x1, y1, z1, x2, y2, z2 );
      return TCL_ERROR;
    }
  }
  
  /* Treatement */
  data = image -> data;
  if (is_coarse) {
    cell_size = (int) pow(2.0,(double) depth);
    cell_edge = lx / cell_size;
    nb_cells  = (int) pow(8.0,(double) depth);
    
    values = (double *) malloc (nb_cells*sizeof (double));
    for (j = 0; j < nb_cells ; j++)
      values[j] = 0.0;
  } else {
    values = (double *) malloc (image -> size*sizeof (double));
    for (j = 0; j < image -> size ; j++)
      values[j] = 0.0;
  }

  smSetBeginTime();
  if (is_domain) {
    for (i = 0; i < image -> size; i++) {
      /* this is column-major format */
      posx = i % lx;
      posy = (i / lx) % ly;
      posz = (i / lx) / ly;
      if (posx >= x1 && posx <= x2 && posy >= y1 && posy <= y2 && posz >= z1 && posz <= z2) {
	if (data[i]!=0) {
	  values[nb_of_values] = evaluator_evaluate_x_y(fct, data[i], scd_value);
	} else { 
	  values[nb_of_values] = 0.0;
	}
	nb_of_values++;
      }
    }
  } else if (is_coarse) {
    int x,y,z, xx,yy,zz;
    //int cell_size = (int) pow(2.0,(double) depth);
    //int cell_edge = lx / cell_size;
    //int nb_cells  = (int) pow(8.0,(double) depth);

    nb_of_values = nb_cells;

    for (i = 0; i < image -> size; i++) {
      /* column-major format */
      x  = i % lx;
      y  = (i / lx) % ly;
      z  = (i / lx) / ly;
      xx = x / cell_edge;
      yy = y / cell_edge;
      zz = z / cell_edge;
      j = xx + yy*cell_size + zz*cell_size*cell_size;
      values[j] += (double) data[i];
      //nb_of_values++;
    }
    for (j = 0; j < nb_cells ; j++) {
      if (values[j]!=0) {
	values[j] = evaluator_evaluate_x_y (fct, values[j], scd_value);
      } else {
	values[j] = 0.0;
      }
    }
    
  } else {
    for (i = 0; i < image -> size; i++) {
      if (data[i]!=0) {
	values[nb_of_values] = evaluator_evaluate_x_y (fct, data[i], scd_value);
      } else {
	values[nb_of_values] = 0.0;
      }
      nb_of_values++;
    }
  }
  smSetEndTime();
  
  
  //if (is_coarse) {
  //} else {
  smSetBeginTime();
  if (nb_of_values > 1)
    _QuickSort_ (values-1, nb_of_values);
  smSetEndTime();
  
  for (i = 0; i < nb_of_values; i++)
    result += values[i];
  //}
  
  evaluator_destroy(fct);
  free (values);
  
  if (result == 0) {
    sprintf (interp -> result, "0");
  } else {
    sprintf (interp -> result, "%.15e", result);
  }
  
  return TCL_OK;
}


/*******************************************
 * Commande name in xsmurf : i3Dfct_vector
 *******************************************/
/* created by pierre kestener : july 15th 2003 */

int
apply_fct_to_i3D_vector_TclCmd_ (ClientData clientData,
				 Tcl_Interp *interp,
				 int        argc,
				 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "JJJs[f]",
      "-domain", "dddddd",
      "-weight", "J",
      "-coarse", "d",
      NULL
    };

  char * help_msg =
    {
      (" Applies a function to each modulus value of a 3D vector field (one\n"
       " image3D for each of the 3 components) and sums all the values.\n"
       "This function was primarily designed to do vectorial box-counting.\n"
       "\n"
       "Parameters :\n"
       "  3 image3D  - the 3D vector field to treat.\n"
       "    string   - an expression designing the function (defunc syntax).\n"
       "    [real]   - facultative value of a the second parameter of the\n"
       "             function.\n"
       "\n"
       "Options :\n"
       "  -domain [] : Define a domain a la \"ecut\". See corresponding help\n"
       "               message. The function only applies to values\n"
       "               that are inside the domain.\n"
       "     6 integers - x1, y1, z1 and x2, y2, z2.\n"
       "  -weight [] : does a weighted sum....\n"
       "     1 image3D  : the weights\n"
       "  -coarse [] : see help message of \"ifct3D\".\n"
       "     1 integer  : depth\n"
       )
    };

  /* Command's parameters */
  Image3D *image1,*image2,*image3;
  char  *fct_expr;
  real  scd_value = 0;
  int lx, ly, lz;

  /* Options's presence */
  int is_domain;
  int is_weight;
  int is_coarse;

  /* Options's parameters */
  int x1, y1, z1, x2, y2, z2;
  int posx, posy, posz;
  Image3D *image_weight;
  int depth;
  int cell_size;
  int cell_edge;
  int nb_cells ;

  /* Other variables */
  void     *fct;
  double   result = 0.0;
  double   *values1,*values2,*values3;
  int      i, j;
  int      nb_of_values = 0;
  real     *data1,*data2,*data3;
  real     *data_weight;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2, &image3, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

  is_domain = arg_present (1);
  if (is_domain)
    if (arg_get (1, &x1, &y1, &z1, &x2, &y2, &z2) == TCL_ERROR)
      return TCL_ERROR;
  
  is_weight = arg_present (2);
  if (is_weight)
    if (arg_get (2, &image_weight) == TCL_ERROR)
      return TCL_ERROR;
  
  is_coarse = arg_present (3);
  if (is_coarse)
    if (arg_get (3, &depth) == TCL_ERROR)
      return TCL_ERROR;
  
  /* Parameters validity and initialisation */
  fct = evaluator_create(fct_expr);
  if (!fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  lx = image1->lx;
  ly = image1->ly;
  lz = image1->lz;
  
  if (is_domain) {
    if (x1 < 0 || y1 < 0 || z1 < 0 || x2 >= lx || y2 >= ly || z2 >= lz ) {
      sprintf (interp->result, "Bad domain ([%d %d %d %d %d %d]).", x1, y1, z1, x2, y2, z2 );
      return TCL_ERROR;
    }
  }
  
  /* Treatement */
  data1 = image1 -> data;
  data2 = image2 -> data;
  data3 = image3 -> data;
  if (is_coarse) {
    cell_size = (int) pow(2.0,(double) depth);
    cell_edge = lx / cell_size;
    nb_cells  = (int) pow(8.0,(double) depth);
    
    values1 = (double *) malloc (nb_cells*sizeof (double));
    values2 = (double *) malloc (nb_cells*sizeof (double));
    values3 = (double *) malloc (nb_cells*sizeof (double));
    for (j = 0; j < nb_cells ; j++) {
      values1[j] = 0.0;
      values2[j] = 0.0;
      values3[j] = 0.0;
    }
  } else {
    values1 = (double *) malloc (image1 -> size*sizeof (double));
    values2 = (double *) malloc (image2 -> size*sizeof (double));
    values3 = (double *) malloc (image3 -> size*sizeof (double));
    for (j = 0; j < image1 -> size ; j++) {
      values1[j] = 0.0;
      values2[j] = 0.0;
      values3[j] = 0.0;
    }
  }
  
  smSetBeginTime();
  if (is_domain) {
    for (i = 0; i < image1 -> size; i++) {
      /* column-major format */
      posx = i % lx;
      posy = (i / lx) % ly;
      posz = (i / lx) / ly;
      if (posx >= x1 && posx <= x2 && posy >= y1 && posy <= y2 && posz >= z1 && posz <= z2) {
	values1[nb_of_values] = evaluator_evaluate_x_y(fct, data1[i], scd_value);
	nb_of_values++;
      }
    }
  } else if (is_coarse) {
    int x,y,z, xx,yy,zz;
    //int cell_size = (int) pow(2.0,(double) depth);
    //int cell_edge = lx / cell_size;
    //int nb_cells  = (int) pow(8.0,(double) depth);
    
    nb_of_values = nb_cells;
    
    for (i = 0; i < image1 -> size; i++) {
      /* column-major format */
      x  = i % lx;
      y  = (i / lx) % ly;
      z  = (i / lx) / ly;
      xx = x / cell_edge;
      yy = y / cell_edge;
      zz = z / cell_edge;
      j = xx + yy*cell_size + zz*cell_size*cell_size;
      values1[j] += (double) data1[i];
      values2[j] += (double) data2[i];
      values3[j] += (double) data3[i];
    }
    {
      double tmp;
      for (j = 0; j < nb_cells ; j++) {
	tmp = sqrt(values1[j]*values1[j]+values2[j]*values2[j]+values3[j]*values3[j]);
	values1[j] = evaluator_evaluate_x_y(fct, tmp, scd_value);
      }
    }
    
  } else {
    for (i = 0; i < image1 -> size; i++) {
      values1[nb_of_values] = evaluator_evaluate_x_y(fct, data1[i], scd_value);
      nb_of_values++;
    }
  }
  smSetEndTime();
  
  
  smSetBeginTime();
  if (nb_of_values > 2)
    _QuickSort_ (values1-1, nb_of_values);
  smSetEndTime();
  
  for (i = 0; i < nb_of_values; i++)
    {
      result += values1[i];
      //printf("%.15E\n",result);
    }
  
  evaluator_destroy(fct);
  free (values1);
  free (values2);
  free (values3);

  //printf("putain %.16e\n",result);
 
  if (result == 0) {
    sprintf (interp -> result, "0");
  } else {
    //sprintf (interp -> result, "%.15E", result);
    //printf("%.16e\n",result);
    sprintf (interp -> result, "%.15e", result);
  }
  
  return TCL_OK;
}


/***********************************
 * command name in xsmurf : izoom
 ***********************************/
int
im_zoom_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Isd",
      NULL
    };

  char * help_msg =
    {
      (" Apply a kind of zoom on an image. Create a new image where each points\n"
       "contains the sum of the value of the points that are on a box (if you\n"
       "see what I mean).\n"
       "\n"
       "Parameters :\n"
       "  image   - image to treat.\n"
       "  string  - the name of the new image.\n"
       "  integer - value by wich you divide the original size to get the new\n"
       "            one.")
    };

  /* Command's parameters */
  Image *image;
  char  *name;
  int   div;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image *new_im;
  int   x, y;
  int   new_x, new_y;
  int   new_lx, new_ly;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name, &div) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  /*if (image -> lx % div != 0 || image -> ly % div != 0)
    {
    sprintf (interp -> result, "%d must divide %d and %d", div, image -> lx, image -> ly);
    return TCL_ERROR;
    }
  */

  /* Treatement */
  new_lx = image -> lx / div;
  new_ly = image -> ly / div;

  new_im = im_new (new_lx, new_ly, new_lx*new_ly, PHYSICAL);
  im_set_0 (new_im);

  /*for (new_x = 0; new_x < new_lx; new_x++)
    for (new_y = 0; new_y < new_ly; new_y++)
    for (x = new_x*div; x < (new_x+1)*div; x++)
    for (y = new_y*div; y < (new_y+1)*div; y++)
    new_im -> data[new_x+new_y*new_lx] += image -> data[x+y*image -> lx];
  */
  for (x = 0; x < new_lx*div; x++)
    for (y = 0; y < new_ly*div; y++) {
      new_x = x / div;
      new_y = y / div;
      new_im -> data[new_x+new_y*new_lx] += image -> data[x+y*image -> lx];
    }

  store_image (name, new_im);

  return TCL_OK;
}


/*************************************
 * command name in xsmurf : iswapraw
 *************************************/
int
im_swap_raws_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Is",
      NULL
    };

  char * help_msg =
    {
      (" Revert an image top to bottom.\n"
       "\n"
       "Parameters :\n"
       "  image   - image to treat.\n"
       "  string  - the name of the new image.\n"
       "\n.")
    };

  /* Command's parameters */
  Image *image;
  char  *name;
  int   lx,ly;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image *new_im;
  int   x, y;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  
  /* Treatement */
  lx = image -> lx ;
  ly = image -> ly ;

  new_im = im_new (lx, ly, lx*ly, PHYSICAL);
  im_set_0 (new_im);

  for (y = 0; y < ly; y++)
    for (x = 0; x < lx; x++)
      new_im -> data[x+y*lx] += image -> data[x+(ly-1-y)*lx];
  
  store_image (name, new_im);

  return TCL_OK;
}



/*
  command name in xsmurf : iswapcol
*/
int
im_swap_cols_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Is",
      NULL
    };

  char * help_msg =
    {
      (" Revert an image left to right.\n"
       "\n"
       "Parameters :\n"
       "  image   - image to treat.\n"
       "  string  - the name of the new image.\n"
       "\n.")
    };

  /* Command's parameters */
  Image *image;
  char  *name;
  int   lx,ly;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image *new_im;
  int   x, y;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  
  /* Treatement */
  lx = image -> lx ;
  ly = image -> ly ;

  new_im = im_new (lx, ly, lx*ly, PHYSICAL);
  im_set_0 (new_im);

  for (y = 0; y < ly; y++)
    for (x = 0; x < lx; x++)
      new_im -> data[x+y*lx] += image -> data[(lx-1-x)+y*lx];
  
  store_image (name, new_im);

  return TCL_OK;
}


/*
  command name in xsmurf : imirror
*/
int
im_mirror_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Is",
      NULL
    };

  char * help_msg =
    {
      (" Transform image by symetry along the line y equals x.\n"
       " In other words, swap roles of y and x.\n"
       "\n"
       "Parameters :\n"
       "  image   - image to treat.\n"
       "  string  - the name of the new image.\n"
       "\n.")
    };

  /* Command's parameters */
  Image *image;
  char  *name;
  int   lx,ly;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image *new_im;
  int   x, y;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  
  /* Treatement */
  lx = image -> lx ;
  ly = image -> ly ;

  new_im = im_new (ly, lx, lx*ly, PHYSICAL);
  im_set_0 (new_im);

  for (y = 0; y < ly; y++)
    for (x = 0; x < lx; x++)
      new_im -> data[x+y*ly] += image -> data[y+x*lx];
  
  store_image (name, new_im);

  return TCL_OK;
}




/*
 */
int
im_cut_TclCmd_ (ClientData clientData,
		Tcl_Interp *interp,
		int        argc,
		char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Isd",
      "-y", "",
      "-45", "[d]",
      NULL
    };

  char * help_msg =
    {
      (" Cut an image and create a signal. The default drirection is the X axis.\n"
       "\n"
       "Parameters :\n"
       "  image   - image to cut.\n"
       "  string  - the name of the signal.\n"
       "  integer - position of the cut.\n"
       "\n"
       "Options :\n"
       "  -y : The direction is the Y axis.\n"
       "  -45 : The direction is 45 degree.\n"
       "     [integer] - request signal size.")
    };

  /* Command's parameters */
  Image *image;
  char  *name;
  int   pos;

  /* Options's presence */
  int is_y;
  int is_45;

  /* Options's parameters */
  int sig_size;

  /* Other variables */
  Signal *new_sig;
  int     x, y;
  int     lx;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name, &pos) == TCL_ERROR)
    return TCL_ERROR;

  is_y = arg_present (1);
  is_45 = arg_present (2);
  if (is_45) {
    if (arg_get (2, &sig_size) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  /* Parameters validity and initialisation */
  if ((!is_y && pos >= image -> ly)
      || (is_y && pos >= image -> lx)
      || (pos < 0))
    {
      sprintf (interp -> result,
	       "Wrong value of the position of the cut (%d).", pos);
      return TCL_ERROR;
    }

  /* Check if we have a good <<position/request_size/image_dimensions>>
   * combination. */
  if (is_45) {
    if (!is_y) {
      if (pos < image->ly) {
	if (sig_size > image->ly) {
	  sprintf (interp -> result,
		   "can't cut a signal of size %d at requested position (%d)",
		   sig_size, pos);
	  return TCL_ERROR;
	}
      } else {
	if ((sig_size+pos) > (2*image->ly)) {
	  sprintf (interp -> result,
		   "can't cut a signal of size %d at requested position (%d)",
		   sig_size, pos);
	  return TCL_ERROR;
	}
      }
    } else {
      if (pos < image->lx) {
	if (sig_size > image->lx) {
	  sprintf (interp -> result,
		   "can't cut a signal of size %d at requested position (%d)",
		   sig_size, pos);
	  return TCL_ERROR;
	}
      } else {
	if ((sig_size+pos) > (2*image->lx)) {
	  sprintf (interp -> result,
		   "can't cut a signal of size %d at requested position (%d)",
		   sig_size, pos);
	  return TCL_ERROR;
	}
      }
    }
  }

  if (!is_45) {
    if (is_y)
      sig_size = image->ly;
    else
      sig_size = image->lx;
  }

  /* Treatement */
  lx = image->lx;
  new_sig = sig_new (REALY, 0, sig_size - 1);

  if (is_y) {
    x = pos;
    for (y = 0; y < sig_size; y++) {
      new_sig->dataY[y] = image->data[x+y*lx];
      x = (is_45 ? x+1 : x);
    }
  } else {
    y = pos;
    for (x = 0; x < sig_size; x++) {
      new_sig->dataY[x] = image->data[x+y*lx];
      y = (is_45 ? y+1 : y);
    }
  }

  store_signal_in_dictionary (name, new_sig);

  return TCL_OK;
}


#define same_domain(i1,i2) ((i1->lx==i2->lx)&&(i1->ly==i2->ly))

/************************************
 * command name in xsmurf : icomb
 ************************************/
int
comb_images_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "IIss",
      NULL
    };

  char * help_msg =
    {
      (" Create an image as a combination of two images.\n"
       "\n"
       "Parameters :\n"
       "  2 Images - images to combine.\n"
       "  string   - expression of the combination (defunc expression). The\n"
       "             images are designed by the tokens 'x' and 'y' (ex.\n"
       "             x*cos(y) compute the first image times the cosine of the\n"
       "             second one.\n"
       "  string   - name of the result.")
    };

  /* Command's parameters */
  Image  *image1, *image2;
  char   *fct_expr;
  char   *res_name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image  *result;
  void *fct;
  int    i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2, &fct_expr, &res_name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (image1 -> type != image2 -> type)
    {
      sprintf (interp -> result,"Images must have the same type.\n");
      return TCL_ERROR;
    }
  if (!same_domain (image1, image2))
    {
      sprintf (interp -> result,"Images must have the same x domain.\n");
      return TCL_ERROR;
    }
  fct = evaluator_create(fct_expr);
  if (!fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }


  /* Treatement */
  result = im_new(image1->lx, image1->ly, image1->size, image1->type);
  for (i = 0; i < result->size; i++) {
    /*result -> data[i] = fct (image1 -> data[i], image2 -> data[i]);*/
    result -> data[i] = evaluator_evaluate_x_y(fct, image1 -> data[i], image2 -> data[i]);
  }

  evaluator_destroy(fct);
  store_image (res_name, result);
  return TCL_OK;
}


/*
 * Command name in xsmurf : myifilter
 */
int
myifilter_image_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Is",
      "-mean33", "",
      "-max33", "",
      "-gauss", "",
      "-airy", "",
      NULL
    };

  char * help_msg =
    {
      (" It filters image1 and create image2 ; it computes\n"
       "each pixel value of image2 by applying a filter in\n"
       "the neighbourhood of corresponding pixel in image1"
       "Parameters :\n"
       "  1 Image  - image to filter.\n"
       "  string   - name of the resulting image\n"
       "\n"
       "Options :\n"
       "  -mean33 : computes each pixel value as a mean\n"
       "            of the neighbourhood's value (3x3)\n"
       "  -max33  : computes each pixel value as the maximun\n"
       "            value among the neighbourhood\n"
       "  -gauss  : filters the image with the gaussian filter\n"
       "            given by the matrix\n"
       "            [1  4  6  4 1]\n"
       "            [4 16 24 16 4]\n"
       "            [6 24 36 24 6]\n"
       "            [4 16 24 16 4]\n"
       "            [1  4  6  4 1]\n"
       "  -airy   : filters the image with a 7 x 7 matrix \n"
       "            corresponding to the Airy Disk PSF\n")
    };
    
  /* Command's parameters */
  Image  *imagein;
  char   *res_name;
  double *tabval;
  double *tabval_gauss;
  double *tabval_airy;
  float  meanvalue;
  float  airymeanvalue;
    
  /* Options's presence */
  int is_mean33 = 0;
  int is_max33 = 0;
  int is_gauss = 0;
  int is_airy = 0;
    
  /* Options's parameters */

  /* Other variables */
  Image  *result;
  int    i, ix, iy, pos;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &imagein, &res_name) == TCL_ERROR)
    return TCL_ERROR;

  is_mean33 = arg_present (1);
  is_max33  = arg_present (2);
  is_gauss  = arg_present (3);
  //    is_airy   = arg_present (4);

  /* Parameters validity and initialisation */
  /* fct = dfopen (); */

  /* Treatement */
  result = im_new(imagein->lx, imagein->ly, imagein->size, imagein->type);
  tabval = (double *) malloc(9*sizeof(double));
  tabval_gauss = (double *) malloc(25*sizeof(double));
  //    tabval_airy  = (double *) malloc(49*sizeof(double));
    
  for (ix = 0; ix < result->lx; ix++) {
    for (iy = 0; iy < result->ly; iy++) {
      pos = ix+iy*imagein->lx;
      if ((ix == 0) | (ix == (result->lx - 1)) | (iy == 0) | (iy == (result->ly-1))) {
	result->data[pos] = imagein->data[pos];
      }
      else {
	/* on recopie dans le tableau tabval les valeurs environnantes */
	tabval[0] = imagein->data[ix-1+(iy-1)*imagein->lx];
	tabval[1] = imagein->data[ix+(iy-1)*imagein->lx];
	tabval[2] = imagein->data[ix+1+(iy-1)*imagein->lx];
	tabval[3] = imagein->data[ix-1+iy*imagein->lx];
	tabval[4] = imagein->data[ix+iy*imagein->lx];
	tabval[5] = imagein->data[ix+1+iy*imagein->lx];
	tabval[6] = imagein->data[ix-1+(iy+1)*imagein->lx];
	tabval[7] = imagein->data[ix+(iy+1)*imagein->lx];
	tabval[8] = imagein->data[ix+1+(iy+1)*imagein->lx];
	_QuickSort_(tabval-1,9);
	meanvalue = 0;
	for (i = 0; i < 9; i++) meanvalue += tabval[i];
	meanvalue /= 9;
		
	/* Truc similaire, mais pour tabval_gauss */
	tabval_gauss[0]  = 0.1*imagein->data[ix-2+(iy-2)*imagein->lx];
	tabval_gauss[1]  = 0.4*imagein->data[ix-1+(iy-2)*imagein->lx];
	tabval_gauss[2]  = 0.6*imagein->data[ix+0+(iy-2)*imagein->lx];
	tabval_gauss[3]  = 0.4*imagein->data[ix+1+(iy-2)*imagein->lx];
	tabval_gauss[4]  = 0.1*imagein->data[ix+2+(iy-2)*imagein->lx];
	tabval_gauss[5]  = 0.4*imagein->data[ix-2+(iy-1)*imagein->lx];
	tabval_gauss[6]  = 1.6*imagein->data[ix-1+(iy-1)*imagein->lx];
	tabval_gauss[7]  = 2.4*imagein->data[ix+0+(iy-1)*imagein->lx];
	tabval_gauss[8]  = 1.6*imagein->data[ix+1+(iy-1)*imagein->lx];
	tabval_gauss[9]  = 0.4*imagein->data[ix+2+(iy-1)*imagein->lx];
	tabval_gauss[10] = 0.6*imagein->data[ix-2+(iy-0)*imagein->lx];
	tabval_gauss[11] = 2.4*imagein->data[ix-1+(iy-0)*imagein->lx];
	tabval_gauss[12] = 3.6*imagein->data[ix+0+(iy-0)*imagein->lx];
	tabval_gauss[13] = 2.4*imagein->data[ix+1+(iy-0)*imagein->lx];
	tabval_gauss[14] = 0.6*imagein->data[ix+2+(iy-0)*imagein->lx];
	tabval_gauss[15] = 0.4*imagein->data[ix-2+(iy+1)*imagein->lx];
	tabval_gauss[16] = 1.6*imagein->data[ix-1+(iy+1)*imagein->lx];
	tabval_gauss[17] = 2.4*imagein->data[ix+0+(iy+1)*imagein->lx];
	tabval_gauss[18] = 1.6*imagein->data[ix+1+(iy+1)*imagein->lx];
	tabval_gauss[19] = 0.4*imagein->data[ix+2+(iy+1)*imagein->lx];
	tabval_gauss[20] = 0.1*imagein->data[ix-2+(iy+2)*imagein->lx];
	tabval_gauss[21] = 0.4*imagein->data[ix-1+(iy+2)*imagein->lx];
	tabval_gauss[22] = 0.6*imagein->data[ix+0+(iy+2)*imagein->lx];
	tabval_gauss[23] = 0.4*imagein->data[ix+1+(iy+2)*imagein->lx];
	tabval_gauss[24] = 0.1*imagein->data[ix+2+(iy+2)*imagein->lx];
		
	_QuickSort_(tabval_gauss-1,25);


	/* Maintenant pour tabval_Airy -- Voir fonction icell */
	/*		tabval_airy[0]  = -0.0723*imagein->data[ix-3+(iy-3)*imagein->lx];
			tabval_airy[1]  =  0.0514*imagein->data[ix-2+(iy-3)*imagein->lx];
			tabval_airy[2]  =  0.1745*imagein->data[ix-1+(iy-3)*imagein->lx];
			tabval_airy[3]  =  0.2257*imagein->data[ix+0+(iy-3)*imagein->lx];
			tabval_airy[4]  =  0.1745*imagein->data[ix+1+(iy-3)*imagein->lx];
			tabval_airy[5]  =  0.0514*imagein->data[ix+2+(iy-3)*imagein->lx];
			tabval_airy[6]  = -0.0723*imagein->data[ix+3+(iy-3)*imagein->lx];
			tabval_airy[7]  =  0.0514*imagein->data[ix-3+(iy-2)*imagein->lx];
			tabval_airy[8]  =  0.2826*imagein->data[ix-2+(iy-2)*imagein->lx];
			tabval_airy[9]  =  0.4921*imagein->data[ix-1+(iy-2)*imagein->lx];
			tabval_airy[10] =  0.5764*imagein->data[ix+0+(iy-2)*imagein->lx];
			tabval_airy[11] =  0.4921*imagein->data[ix+1+(iy-2)*imagein->lx];
			tabval_airy[12] =  0.2826*imagein->data[ix+2+(iy-2)*imagein->lx];
			tabval_airy[13] =  0.0514*imagein->data[ix+3+(iy-2)*imagein->lx];
			tabval_airy[14] =  0.1745*imagein->data[ix-3+(iy-1)*imagein->lx];
			tabval_airy[15] =  0.4921*imagein->data[ix-2+(iy-1)*imagein->lx];
			tabval_airy[16] =  0.7687*imagein->data[ix-1+(iy-1)*imagein->lx];
			tabval_airy[17] =  0.8799*imagein->data[ix+0+(iy-1)*imagein->lx];
			tabval_airy[18] =  0.7697*imagein->data[ix+1+(iy-1)*imagein->lx];
			tabval_airy[19] =  0.4921*imagein->data[ix+2+(iy-1)*imagein->lx];
			tabval_airy[20] =  0.1745*imagein->data[ix+3+(iy-1)*imagein->lx];
			tabval_airy[21] =  0.2257*imagein->data[ix-3+(iy-0)*imagein->lx];
			tabval_airy[22] =  0.5764*imagein->data[ix-2+(iy-0)*imagein->lx];
			tabval_airy[23] =  0.8799*imagein->data[ix-1+(iy-0)*imagein->lx];
			tabval_airy[24] =  1.0000*imagein->data[ix+0+(iy-0)*imagein->lx];
			tabval_airy[25] =  0.8799*imagein->data[ix+1+(iy-0)*imagein->lx];
			tabval_airy[26] =  0.5764*imagein->data[ix+2+(iy-0)*imagein->lx];
			tabval_airy[27] =  0.2257*imagein->data[ix+3+(iy-0)*imagein->lx];
			tabval_airy[28] =  0.1745*imagein->data[ix-3+(iy+1)*imagein->lx];
			tabval_airy[29] =  0.4921*imagein->data[ix-2+(iy+1)*imagein->lx];
			tabval_airy[30] =  0.7697*imagein->data[ix-1+(iy+1)*imagein->lx];
			tabval_airy[31] =  0.8799*imagein->data[ix+0+(iy+1)*imagein->lx];
			tabval_airy[32] =  0.7697*imagein->data[ix+1+(iy+1)*imagein->lx];
			tabval_airy[33] =  0.4921*imagein->data[ix+2+(iy+1)*imagein->lx];
			tabval_airy[34] =  0.1745*imagein->data[ix+3+(iy+1)*imagein->lx];
			tabval_airy[35] =  0.0514*imagein->data[ix-3+(iy+2)*imagein->lx];
			tabval_airy[36] =  0.2826*imagein->data[ix-2+(iy+2)*imagein->lx];
			tabval_airy[37] =  0.4921*imagein->data[ix-1+(iy+2)*imagein->lx];
			tabval_airy[38] =  0.5764*imagein->data[ix+0+(iy+2)*imagein->lx];
			tabval_airy[39] =  0.4921*imagein->data[ix+1+(iy+2)*imagein->lx];
			tabval_airy[40] =  0.2826*imagein->data[ix+2+(iy+2)*imagein->lx];
			tabval_airy[41] =  0.0514*imagein->data[ix+3+(iy+2)*imagein->lx];
			tabval_airy[42] = -0.0723*imagein->data[ix-3+(iy+3)*imagein->lx];
			tabval_airy[43] =  0.0514*imagein->data[ix-2+(iy+3)*imagein->lx];
			tabval_airy[44] =  0.1745*imagein->data[ix-1+(iy+3)*imagein->lx];
			tabval_airy[45] =  0.2257*imagein->data[ix+0+(iy+3)*imagein->lx];
			tabval_airy[46] =  0.1745*imagein->data[ix+1+(iy+3)*imagein->lx];
			tabval_airy[47] =  0.0514*imagein->data[ix+2+(iy+3)*imagein->lx];
			tabval_airy[48] = -0.0723*imagein->data[ix+3+(iy+3)*imagein->lx];
	*/
		
	//		_QuickSort_(tabval_airy-1,49);
	//		airymeanvalue = 0;
	//		for (i = 0; i < 49; i++) airymeanvalue += tabval_airy[i];
	//		airymeanvalue /= 49;

		
	if (is_max33) {
	  result->data[pos] = (float) tabval[0];
	}
	else if (is_mean33) {
	  result->data[pos] = (float) meanvalue;
	}
	else if (is_gauss) {
	  result->data[pos] = (float) tabval_gauss[12];
	}
	//		else if (is_airy) {
	//		    //result->data[pos] = (float) airymeanvalue;
	//		    result->data[pos] = (float) tabval_airy[25];
	//		}
	else {
	  result->data[pos] = 0.;
	}
      }
    }
  }
  store_image (res_name, result);
  free(tabval);
  free(tabval_gauss);
  //    free(tabval_airy);
  return TCL_OK;
}

/********************************************************
 *  command name in xsmurf : itracerect
 ********************************************************/

/*ca sert pour les mammographies pour tracer un rectangle
  autour d'une zone particuliere */
int
itrace_rectangle_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Idddd",
    "-white", "",
    NULL
  };

  char * help_msg = {
    (" Trace a black rectangle in an image.\n"
     "\n"
     "Parameters :\n"
     "  image  - image.\n"
     "  2 int  - position of the upper left corner of rectangle\n"
     "           in image.\n"
     "  2 int  - width and heigth of the rectangle.\n"
     "  see iicut's help message.\n"
     "  we just suppose that the position and the dimensions of the\n"
     "  rectangle are compatible with those of the image."
     "Options : \n"
     "  -white : white rectangle.\n")
  };

  /* Command's parameters */
  Image *image; 
  int   posX, posY;
  int   width, height;
  real themin, themax, thecolor;

  /* Options's presence */
  int isWhite = 0;

  /* Options's parameters */

  /* Other variables */
  
  int    i, j, pos;


  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &posX, &posY, &width, &height) == TCL_ERROR)
    return TCL_ERROR;

  isWhite = arg_present (1);

  /* Parameters validity and initialisation */

  /* Treatement */
  im_get_extrema (image,&themin,&themax);
  if (isWhite)
    thecolor = themax;
  else
    thecolor = themin;

  /* test if the upper left corner is inside the image */
  if (posX < 0 || posX >= image->lx || posY < 0 || posY >= image->ly) {
    sprintf (interp -> result,"Wrong coordinates for upper left corner.\n");
    return TCL_ERROR;
  }
  
  /* test if the image can contain the rectangle */
  if (posX+width >= image->lx || posY+height >= image->ly) {
    sprintf (interp -> result,"Wrong dimensions of rectangle.\n");
    return TCL_ERROR;
  }

  /* now we can assume that all arguments are correct */
  
  /* we trace up border */
  for (i=0; i <= width; i++) {
    pos = posX + i + image->lx*posY;
    image->data[pos] = thecolor;
  }

  /* we trace low border */
  for (i=0; i <= width; i++) {
    pos = posX + i + image->lx*(posY + height);
    image->data[pos] = thecolor;
  }

  /* we trace left border */
  for (j=0; j <= height; j++) {
    pos = posX + image->lx*(posY + j);
    image->data[pos] = thecolor;
  }

  /* we trace right border */
  for (j=0; j <= height; j++) {
    pos = posX + width + image->lx*(posY + j);
    image->data[pos] = thecolor;
  }

  /*  Tcl_AppendResult(interp,name,NULL); */

  return TCL_OK;
}


/* *******************************************************
   command name in xsmurf : itracesegment
   **************************************************** */

/*ca sert pour les images de Francois sur les billes de latex
  pour tracer la trajectoire de la bille */
int
itrace_segment_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Idddd",
    NULL
  };

  char * help_msg = {
    (" Trace a black segment in an image.\n"
     "\n"
     "Parameters :\n"
     "  image  - image.\n"
     "  2 int  - x and y coordinates of start point\n"
     "  2 int  - x and y coordinates of end   point\n"
     "  \n")
  };

  /* Command's parameters */
  Image *image; 
  int   posX1, posY1, posX2, posY2;
  real themin, themax;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  
  int    pos;
  int    x, y;
  real   slopexy;
  real   slopeyx;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &posX1, &posY1, &posX2, &posY2) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  im_get_extrema (image,&themin,&themax);

  if (posX2 == posX1) {
    for (y=posY1; y<=posY2; y++) {
      pos = posX1 + image->lx*y;
      image->data[pos] = themin;
    }
  } else {
    slopexy = 1.0*(posY2-posY1)/(posX2-posX1);
    for (x=posX1; x<=posX2; x++) {
      y = posY1 + (int)(slopexy*(x-posX1)); 
      pos = x + image->lx*y;
      image->data[pos] = themin;
    }
  }

  if (posY2 == posY1) {
    for (x=posX1; x<=posX2; x++) {
      pos = x + image->lx*posY1;
      image->data[pos] = themin;
    }
  } else {
    slopeyx = 1.0*(posX2-posX1)/(posY2-posY1);
    for (y=posY1; y<=posY2; y++) {
      x = posX1 + (int)(slopeyx*(y-posY1)); 
      pos = x + image->lx*y;
      image->data[pos] = themin;
    }
  }
 
  /*  Tcl_AppendResult(interp,name,NULL); */
  
  return TCL_OK;
}


/* *******************************************************
 * Command name in xsmurf : itracecurve
 * ******************************************************* */

/*  ca sert pour les mammographies pour tracer une courbe
    autour d'une zone particuliere */
int
itrace_curve_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Idds[d]",
    NULL
  };

  char * help_msg = {
    (" Trace a black curve on an image.\n"
     "\n"
     "Parameters :\n"
     "  image   - image.\n"
     "  2 int   - position of the first point.\n"
     "  string  - a Tcl list of integers between 0 and 7.\n"
     "  integer - optional parameter : zoom\n")
  };

  /* Command's parameters */
  Image *image;
  int   posX, posY;
  real themin, themax;
  char *direction_lst_str;
  int zoom = 1;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int    dir_lst_size, code;
  char   **dir_elt;
  int    i, pos, int_val;


  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &posX, &posY, &direction_lst_str, &zoom) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  im_get_extrema (image,&themin,&themax);
  
  /* now we can assume that all arguments are correct */
  
  code = Tcl_SplitList(interp, direction_lst_str, &dir_lst_size, (CONST char ***)&dir_elt);
  if (code == TCL_ERROR)
    return TCL_ERROR;

  /* test if the first point is inside the image */
  if (posX/zoom < 0 || posX/zoom >= image->lx || posY/zoom < 0 || posY/zoom >= image->ly) {
    sprintf (interp -> result,"Wrong coordinates for first point: is not inside!.\n");
    return TCL_ERROR;
  }
  /* we trace first point */
  pos = (int)(posX/zoom) + image->lx*(int)(posY/zoom);
  image->data[pos] = themin;
  
  for (i = 0; i < dir_lst_size; i++)
    {
      if (Tcl_GetInt (interp, dir_elt[i], &int_val) == TCL_ERROR)
	{
	  return TCL_ERROR;
	}
      /* test direction */
      if (int_val < 0 || int_val > 7 ) {
	sprintf (interp->result,"last argument MUST be a list of integer between 0 and 7!!\n");
	return TCL_ERROR;
      } else {
	if (int_val == 0) {
	  posY--;
	} else if (int_val == 1) {
	  posX++;
	  posY--;
	} else if (int_val == 2) {
	  posX++;
	} else if (int_val == 3) {
	  posX++;
	  posY++;
	} else if (int_val == 4) {
	  posY++;
	} else if (int_val == 5) {
	  posX--;
	  posY++;
	} else if (int_val == 6) {
	  posX--;
	} else if (int_val == 7) {
	  posX--;
	  posY--;
	}
	/* test if point is inside the image */
	if (posX/zoom < 0 || posX/zoom >= image->lx || posY/zoom < 0 || posY/zoom >= image->ly) {
	  sprintf (interp -> result,"Wrong coordinates for current point.\n");
	  return TCL_ERROR;
	}
	/* we trace point */
	pos = (int)(posX/zoom) + image->lx*(int)(posY/zoom);
	image->data[pos] = themin;
  
      }
      
    }
  Tcl_Free((char *) dir_elt);
  

  return TCL_OK;
}



/* **************************************************
   command name in xsmurf : iblackbox
   *********************************************** */

/*  ca sert pour les mammographies pour remplir un 
    rectangle avec du noir. */
int
iblackbox_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Idddd",
    NULL
  };

  char * help_msg = {
    (" Fill a box (in mage) with black.\n"
     "\n"
     "Parameters :\n"
     "  image  - image.\n"
     "  2 int  - position of the upper left corner of rectangle\n"
     "           in image.\n"
     "  2 int  - width and heigth of the rectangle.\n"
     "  see iicut's help message.\n"
     "  we just suppose that the position and the dimensions of the\n"
     "  rectangle are compatible with those of the image.")
  };

  /* Command's parameters */
  Image *image; 
  int   posX, posY;
  int   width, height;
  real themin, themax;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  
  int    i, j, pos;


  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &posX, &posY, &width, &height) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  im_get_extrema (image,&themin,&themax);

  /* test if the upper left corner is inside the image */
  if (posX < 0 || posX >= image->lx || posY < 0 || posY >= image->ly) {
    sprintf (interp -> result,"Wrong coordinates for upper left corner.\n");
    return TCL_ERROR;
  }
  
  /* test if the image can contain the rectangle */
  if (posX+width >= image->lx || posY+height >= image->ly) {
    sprintf (interp -> result,"Wrong dimensions of rectangle.\n");
    return TCL_ERROR;
  }

  /* now we can assume that all arguments are correct */
  
  for (i=0; i <= width; i++) {
    for (j=0; j <= height; j++) {
      pos = posX + i + image->lx*(posY + j);
      image->data[pos] = themin;
    }
  }

  /*  Tcl_AppendResult(interp,name,NULL); */

  return TCL_OK;
}


/* *******************************************************
 * Command name in xsmurf : iaddima
 * **************************************************** */

/*ca sert pour les mammographies pour tracer un rectangle
  autour d'une zone particuliere */
int
iadd_ima_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "IIdd",
    NULL
  };

  char * help_msg = {
    (" Add a small image to a bigger one at location (x,y).\n"
     "\n"
     "Parameters :\n"
     "  image  - big image.\n"
     "  image  - small image\n"
     "  2 int   - position of the upper left corner where we will\n"
     "           add the small image.\n"
     "\n"
     "  result is store in first image.\n"
     "  we just suppose that the position and the dimensions of the\n"
     "  small image are compatible with those of the big one.\n")
  };

  /* Command's parameters */
  Image *image1, *image2; 
  int   posX, posY;
  int   lx1, lx2, ly1, ly2;
  real themin, themax;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int    i, j, pos;


  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2, &posX, &posY) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  lx1 = image1->lx;
  ly1 = image1->ly;
  lx2 = image2->lx;
  ly2 = image2->ly;

  /* Treatement */
  im_get_extrema (image1,&themin,&themax);

  /* test if the upper left corner is inside the image */
  if (posX < 0 || posX >= lx1 || posY < 0 || posY >= ly1) {
    sprintf (interp -> result,"Wrong coordinates for upper left corner.\n");
    return TCL_ERROR;
  }
  
  /* test if image1 can "contain" image2  */
  if (posX+lx2 >= lx1 || posY+ly2 >= ly1) {
    sprintf (interp -> result,"Impossible, try to reduce image2 size.\n");
    return TCL_ERROR;
  }

  /* now we can assume that all arguments are correct */
  
  /* we trace up border */
  for (i=0; i < lx2; i++) {
    for (j=0; j < ly2; j++) {
      pos = posX + i + lx1*(posY + j);
      image1->data[pos] += image2->data[i+lx2*j];
    }
  }

  /*  Tcl_AppendResult(interp,name,NULL); */

  return TCL_OK;
}


/********************************************************
 *  command name in xsmurf : icombline
 ********************************************************/

int
icomb_line_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Iss",
    "-line", "d",
    "-recurrence", "",
    NULL
  };

  char * help_msg = {
    (" Combine each vertical line with the first one according to a \n"
     " function given as a parameter.\n"
     "\n"
     "Parameters :\n"
     "  image  - image.\n"
     "  string - result image name\n"
     "  string - function expression\n"
     "Options :\n"
     "  -line [d]\n"
     "  -recurrence [] : see code\n")
  };

  /* Command's parameters */
  Image *image, *image2;
  int   posX, posY;
  int   width, height;
  real themin, themax;
  char *resname;
  char *fct_expr;
  void *fct;

  /* Options's presence */
  int isLine;
  int isRecurrence;

  /* Options's parameters */
  int    num = 0;

  /* Other variables */
  int    i, j, pos;
  int    lx,ly;
  float  *data, *data2;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &resname, &fct_expr) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  fct = evaluator_create(fct_expr);
  if (!fct) {
    Tcl_AppendResult (interp,
		      "libmatheval : error", " in expression ",
		      fct_expr, (char *) NULL);
    return TCL_ERROR;
  }

  isLine = arg_present(1);

  if (isLine)
    if (arg_get (1, &num) == TCL_ERROR)
      return TCL_ERROR;

  isRecurrence = arg_present(2);

  /* Treatement */
  im_get_extrema (image,&themin,&themax);

  data = image->data;
  lx   = image->lx;
  ly   = image->ly;
  
  image2 = im_new (lx, ly, lx*ly, PHYSICAL);
  if (!image2)
    return TCL_ERROR;
  data2 = image2->data;
  
  if (isRecurrence) {
    for (i=1;i<lx;i++) {
      for (j=0;j<ly;j++) {
	/*data2[i+j*lx] = fct (data[i+j*lx], data[i-1+j*lx]);*/
	data2[i+j*lx] = evaluator_evaluate_x_y (fct, data[i+j*lx], data[i-1+j*lx]);
      }
    }
    for (j=0;j<ly;j++) {
      data2[j*lx] = data[j*lx];
    }
  } else {
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	/*data2[i+j*lx] = fct (data[i+j*lx], data[num+j*lx]);*/
	data2[i+j*lx] = evaluator_evaluate_x_y (fct, data[i+j*lx], data[num+j*lx]);
      }
    }  
  }
  
  
  evaluator_destroy(fct);
  store_image (resname, image2);


  return TCL_OK;
}



/*
 *  ca sert pour les images de neurones
 */
int
iadd_border_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Isdf",
    NULL
  };

  char * help_msg = {
    (" Add a border arround the image.\n"
     "\n"
     "Parameters :\n"
     "  image  - image of size (lx, ly).\n"
     "  string - name of the result : image of size (lx+border,ly+border)\n"
     "           .\n"
     "  int    - thickness of the border.\n"
     "  float  - value of image (result) inside the border.\n"
     ".\n")
  };

  /* Command's parameters */
  Image *image; 
  Image *res;
  char *name;
  int border;
  real value;

  int i, j;
  int lx, ly;
  int pos, old_pos;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  
  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name, &border, &value) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  lx = image->lx + 2 * border;
  ly = image->ly + 2 * border;
  res = im_new (lx,ly,lx*ly, PHYSICAL);
  
  for (i = 0; i < lx; i++) {
    for (j = 0; j < ly; j++) {
      pos     = i+j*lx;
      old_pos = (i-border)+(j-border)*image->lx;
      if ( (i>=border) && i<(border+image->lx) && (j>=border) && j<(border+image->ly) ) {
	res->data[pos] = image->data[old_pos];
      } else {
	res->data[pos] = value;
      }
    }
  }
  

  /*  Tcl_AppendResult(interp,name,NULL); */

  store_image(name,res);
  Tcl_AppendResult(interp,name,NULL);
  
  return TCL_OK;
}




/*
 * Command name in xsmurf : icopyraw
 * ca sert un peu
 */
int
icopy_raw_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "I[dd]",
    "-col", "dd",
    NULL
  };

  char * help_msg = {
    (" Copy raw number i in raw number j.\n"
     "\n"
     "Parameters :\n"
     "  image  - image.\n"
     "  2 int  - position of raw i and j\n"
     "           take care raw are numbered from 0 to dim-1\n"
     "\n."
     "Options :\n"
     "  -col    : do the same thing on columns!\n")
  };

  /* Command's parameters */
  Image *image; 
  int   rawi = -1 , rawj = -1;
  int   coli, colj;
  real themin, themax;

  /* Options's presence */
  int is_col = 0;

  /* Options's parameters */

  /* Other variables */
  int    k;


  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &rawi, &rawj) == TCL_ERROR)
    return TCL_ERROR;
  
  is_col = arg_present(1);

  if (is_col)
    if (arg_get (1, &coli, &colj) == TCL_ERROR)
      return TCL_ERROR;

  if ((rawi == -1) & (!is_col) ) {
    sprintf (interp -> result,"you must give parameters rawi and rawj!.\n");
    return TCL_ERROR;
  }

  /* Parameters validity and initialisation */
  

  /* Treatement */
  im_get_extrema (image,&themin,&themax);

  /* test if the raw involved exist !! */
  if (is_col) {
    if (coli < 0 || coli >= image->ly || colj < 0 || colj >= image->ly ) {
      sprintf (interp -> result,"Check your parameters.\n");
      return TCL_ERROR;
    } 
  } else {
    if (rawi < 0 || rawi >= image->lx || rawj < 0 || rawj >= image->lx ) {
      sprintf (interp -> result,"Check your parameters.\n");
      return TCL_ERROR;
    }
  }
  
  /* we can now assume that all arguments are correct */
  
  if (is_col) {
    for (k=0; k < image->ly; k++) {
      image->data[colj + image->lx*k] = image->data[coli + image->lx*k];
    }
  } else {
    for (k=0; k < image->lx; k++) {
      image->data[k + image->lx*rawj] = image->data[k + image->lx*rawi];
    }
  }

  /*  Tcl_AppendResult(interp,name,NULL); */

  return TCL_OK;
}



/* ****************************************
 * Command name in xsmurf : iconvert_size
 * **************************************** */
int
iconvert_size_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Is",
    NULL
  };

  char * help_msg = {
    (" Convert size for Vincent's images.\n"
     "\n"
     "Parameters :\n"
     "  image  - image.\n"
     "  string - name of resulting image\n"
     "\n."
     "Options :\n"
     )
  };

  /* Command's parameters */
  Image *im_in, *im_out; 
  char   *out_name;  
  int i, j , oldi, oldj;
  int lx_in , ly_in;
  real themin, themax;


  /* Options's presence */

  /* Options's parameters */

  /* Other variables */

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &im_in, &out_name) == TCL_ERROR)
    return TCL_ERROR;
  
  /* Parameters validity and initialisation */
  

  /* Treatement */
  im_get_extrema (im_in,&themin,&themax);
  lx_in = im_in->lx;
  ly_in = im_in->ly;
 
  im_out = im_new(100, 512, 100*512, PHYSICAL);

  /* we can now assume that all arguments are correct */
  
  for (i=0; i < 100; i++) {
    for (j=0; j < 512; j++) {
      /*oldi = j/2*100+i;
	oldj = j%2;*/
      oldi = (i+j*100)%25600;
      oldj = (i+j*100)/25600;
      im_out->data[i+100*j] = im_in->data[oldi+25600*oldj];
    }
  }

  store_image(out_name,im_out);
  Tcl_AppendResult(interp,out_name,NULL);

  return TCL_OK;
}




/*
 * command name in xsmurf : igfft2ri
 */
int
im_gfft_to_2real_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Iss",
      NULL
    };

  char * help_msg =
    {
      (" Transfrom an gfft-format image into 2 images (real part and imaginary\n"
       "part).\n"
       "\n"
       "Parameters :\n"
       "  Image     - image to treat.\n"
       "  2 strings - names of the results (real and imaginary, respectivly).")
    };

  /* Command's parameters */
  Image  *source;
  char   *real_name;
  char   *imag_name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image  *res_real;
  Image  *res_imag;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &source, &real_name, &imag_name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  res_real = im_new(source->lx, source->ly, source->lx*source->ly, PHYSICAL);
  res_imag = im_new(source->lx, source->ly, source->lx*source->ly, PHYSICAL);

  im_gfft_to_2real (source, res_real, res_imag);

  store_image (real_name, res_real);
  store_image (imag_name, res_imag);

  return TCL_OK;
}


/*
 * Command name in xsmurf : inr2ri
 */
int
im_nr_to_2real_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Iss",
      NULL
    };

  char * help_msg =
    {
      (" Transfrom an NR-format image into 2 images (real part and imaginary\n"
       "part).\n"
       "\n"
       "Parameters :\n"
       "  Image     - image to treat.\n"
       "  2 strings - names of the results (real and imaginary, respectivly).")
    };

  /* Command's parameters */
  Image  *source;
  char   *real_name;
  char   *imag_name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image  *res_real;
  Image  *res_imag;
  int    i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &source, &real_name, &imag_name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  res_real = im_new(source->lx, source->ly, source->lx*source->ly, PHYSICAL);
  res_imag = im_new(source->lx, source->ly, source->lx*source->ly, PHYSICAL);

  for (i = 0; i < res_real->size; i++) {
    res_real->data[i] = source->data[2*i];
    res_imag->data[i] = source->data[2*i+1];
  }

  store_image (real_name, res_real);
  store_image (imag_name, res_imag);

  return TCL_OK;
}


static real _tmp_r_;
#define _swap_r_(a,b) _tmp_r_=(a);(a)=(b);(b)=_tmp_r_

/*****************************************
 * command name in xsmurf : iswap
 *****************************************/
int
im_swap_part_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "I",
      NULL
    };

  char * help_msg =
    {
      (" Swap upper-left part with lower-right part and swap lower-left part\n"
       "with upper-right part.\n"
       "\n"
       "Parameters :\n"
       "  Image     - image to treat.")
    };

  /* Command's parameters */
  Image  *source;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int i;
  int j;
  int lx;
  int ly;
  real *data;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &source) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  /*
   * Swap parts of the images so that the point corresponding to frequencies
   * (0,0) will be in the center of the images.
   */
  data = source->data;
  lx = source->lx;
  ly = source->ly;
  for (i = 0; i < lx/2; i++) {
    for (j = 0; j < ly/2; j++) {
      /* Swap upper-left with lower-right. */
      _swap_r_(data[i+j*lx], data[(i+lx/2)+(j+ly/2)*lx]);
      /* Swap upper-right with lower-left. */
      _swap_r_(data[(i+lx/2)+j*lx], data[i+(j+ly/2)*lx]);
    }
  }

  return TCL_OK;
}


/*
 * Command name in xsmurf : iinssig
 */
int
im_insert_sig_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "ISd",
      "-value", "f",
      NULL
    };

  char * help_msg =
    {
      (" Insert a signal in an image at a given position.\n"
       "\n"
       "Parameters :\n"
       "  Image   - image to treat.\n"
       "  Signal  - signal to add.\n"
       "  integer - position.\n"
       "\n"
       "Options :\n"
       "  -value : don't use the signal value.\n"
       "     real - value to use.")
    };

  /* Command's parameters */
  Image  *im;
  Signal *sig;
  int    position;

  /* Options's presence */
  int isValue;

  /* Options's parameters */
  real value;

  /* Other variables */
  int i;
  int lx;
  int x;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &im, &sig, &position) == TCL_ERROR)
    return TCL_ERROR;

  isValue = arg_present(1);
  if (isValue) {
    if (arg_get(1, &value) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */
  if (sig->size > im->lx) {
    sprintf(interp->result,
	    "The signal size (%d) must be lesser than the image width (%d).",
	    sig->size, im->lx);
    return TCL_ERROR;
  }

  /* Treatement */

  lx = im->lx;
  for (i = 0; i < sig->size; i++) {
    if (sig->type == REALXY) {
      x = (int) sig->dataX[i];
    } else {
      x = i;
    }
    if (isValue) {
      im->data[x+lx*position] = value;
    } else {
      im->data[x+lx*position] = sig->dataY[i];
    }
  }

  return TCL_OK;
}


/*
 * command name in xsmurf : iinfo
 */
int
im_get_info_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "I",
      NULL
    };

  char * help_msg =
    {
      (" Get info about an image as a list. The list has the following order : type, lx, ly, min, max.\n"
       "\n"
       "Parameters :\n"
       "  Image - image to treat.")
    };

  /* Command's parameters */
  Image *im;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  char string[100];
  char *type[] = {"NONE", "PHYSICAL", "FOURIER"};


  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &im) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  sprintf(string, "%s", type[im->type]);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%d", im->lx);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%d", im->ly);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%g", im->min);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%g", im->max);
  Tcl_AppendElement (interp, string);

  return TCL_OK;
}


/*
 */
int
foreachi_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Is",
      NULL
    };

  char * help_msg =
    {
      (" Execute a script foreach point of an image. In this script the "
       "variables x and y refer to the coordinates of the point and value to "
       "its, huh, value....\n"
       "\n"
       "Parameters :\n"
       "  Image - image to treat.\n"
       "  string - script to execute.")
    };

  /* Command's parameters */
  Image *image;
  char  *scriptStr;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int result;
  int imInd;
  Tcl_Obj *xStrObj, *xObj;
  Tcl_Obj *yStrObj, *yObj;
  Tcl_Obj *valueStrObj, *valueObj;
  int x, y;
  int lx, ly;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &scriptStr) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  imInd = 0;
  lx = image->lx;
  ly = image->ly;

  xStrObj = Tcl_NewStringObj("x", 1);
  xObj = Tcl_NewLongObj(0);
  yStrObj = Tcl_NewStringObj("y", 1);
  yObj = Tcl_NewLongObj(0);
  valueStrObj = Tcl_NewStringObj("value", 5);
  valueObj = Tcl_NewDoubleObj(0.0);

  while (1) {
    if (imInd == image->size) {
      result = TCL_OK;
      break;
    }
    x = imInd%lx;
    y = imInd/lx;
    Tcl_SetLongObj(xObj, x);
    Tcl_ObjSetVar2(interp, xStrObj, NULL, xObj, 0);
    Tcl_SetLongObj(yObj, y);
    Tcl_ObjSetVar2(interp, yStrObj, NULL, yObj, 0);
    Tcl_SetDoubleObj(valueObj, image->data[imInd]);
    Tcl_ObjSetVar2(interp, valueStrObj, NULL, valueObj, 0);

    result = Tcl_Eval(interp, scriptStr);
    if ((result != TCL_OK) && (result != TCL_CONTINUE)) {
      if (result == TCL_ERROR) {
	char msg[60];
	sprintf(msg, "\n    (\"foreachi\" body line %d)",interp->errorLine);
	Tcl_AddErrorInfo(interp, msg);
      }
      break;
    }
    imInd++;
  }

  if (result == TCL_BREAK) {
    result = TCL_OK;
  }
  if (result == TCL_OK) {
    Tcl_ResetResult(interp);
  }

  return result;
}


/*********************************
  command name in xsmurf : iexpr
**********************************/
int
create_im_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "sds",
      NULL
    };

  char * help_msg =
    {
      (" Create a new image from a function expression.\n"
       "\n"
       "Parameters :\n"
       "  string  - image name.\n"
       "  integer - image size.\n"
       "  string  - function expression.")
    };

  /* Command's parameters */
  char *name;
  int  size;
  char *fct_expr;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image *image;
  void *fct;
  int x,y;
  int pos;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &name, &size, &fct_expr) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  fct = evaluator_create(fct_expr);
  if (!fct) {
    Tcl_AppendResult (interp,
		      "libmatheval : error", " in expression ",
		      fct_expr, (char *) NULL);
    return TCL_ERROR;
  }

  /* Treatement */

  image = im_new (size, size, size*size, PHYSICAL);

  for (x = 0; x < image->lx; x++) {
    for (y = 0; y < image->ly; y++) {
      pos = x+y*image->lx;
      image->data[pos] = evaluator_evaluate_x_y (fct, (double)x,(double)y);
    }
  }

  evaluator_destroy(fct);

  store_image(name,image);
  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}



/*
 */
int
dxbox_conv_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Ids",
      "-dy", "",
      NULL
    };

  char * help_msg =
    {
      (" Compute the convolution between an image and the derivative of a box. "
       "By default the derivative is along the x axis.\n"
       "\n"
       "Parameters :\n"
       "  image   - image to treat.\n"
       "  int     - size of the box. 2 must divide this size.\n"
       "  string  - result name.\n"
       "\n"
       "Options :\n"
       "  -dy : Use the y derivative instead of th x derivative.")
    };

  /* Command's parameters */
  Image *image;
  int   boxSize;
  char  *name;

  /* Options's presence */
  int isDy;

  /* Options's parameters */

  /* Other variables */
  Image *result;
  int x,y;
  int u,v;
  int pos;
  int uvpos;
  real norm;
  int   boxSize_2;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &boxSize, &name) == TCL_ERROR)
    return TCL_ERROR;

  isDy = arg_present(1);

  /* Parameters validity and initialisation */
  if (boxSize%2 == 1) {
    Tcl_AppendResult (interp,
		      "the size of the box must be a <<multiple>> of 2.", (char *) NULL);
    return TCL_ERROR;
  }

  /* Treatement */

  result = im_new (image->lx, image->lx, image->lx*image->lx, PHYSICAL);
  im_set_0(result);

  boxSize_2 = boxSize/2;
  norm = 1.0/boxSize;
  for (x = boxSize_2; x < (image->lx-boxSize_2); x++) {
    for (y = boxSize_2; y < (image->ly-boxSize_2); y++) {
      pos = x+y*image->lx;
      result->data[pos] = 0.0;
      u = -boxSize_2;
      for (v = -boxSize_2; v < boxSize_2; v++) {
	if (isDy) {
	  uvpos = (x+v)+(y+u)*image->lx;
	} else {
	  uvpos = (x+u)+(y+v)*image->lx;
	}
	result->data[pos] -= image->data[uvpos];
      }
      u = boxSize_2-1;
      for (v = -boxSize_2; v < boxSize_2; v++) {
	if (isDy) {
	  uvpos = (x+v)+(y+u)*image->lx;
	} else {
	  uvpos = (x+u)+(y+v)*image->lx;
	}
	result->data[pos] += image->data[uvpos];
      }
      result->data[pos] *= norm;
    }
  }

  store_image(name, result);

  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}


/*
 * Command name in xsmurf : isp2ssp
 */
int
isp_2_ssp_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Isd",
      NULL
    };

  char * help_msg =
    {
      (" Compute the 1d energy spectrum from the 2d energy spectrum (i.e. "
       "log(S(k)) / |k| )\n"
       "\n"
       "Parameters :\n"
       "  image   - image to treat.\n"
       "  string  - signal result name.\n"
       "  int     - size of the 1d spectrum.")
    };

  /* Command's parameters */
  Image *image;
  char  *name;
  int   resSize;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *result;
  real   logk;
  int    i;
  int    x, y;
  int    newX, newY;
  int    *nb;
  int    pos;
  real   dx, max;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name, &resSize) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  result = sig_new (REALY, 0, resSize - 1);
  nb = (int *) malloc(sizeof(int)*resSize);
  for (i = 0; i < resSize; i++) {
    nb[i] = 0;
    result->dataY[i] = 0.0;
  }

  max = sqrt(2.0*(image->lx*image->ly/4));
  dx = max/(resSize-1);
  result->x0 = 0.0;
  result->dx = dx;

  for (x = 0; x < image->lx; x++) {
    for (y = 0; y < image->ly; y++) {
      pos = x + y*image->lx;
      newX = x - image->lx/2;
      newY = y - image->ly/2;
      logk = sqrt(newX*newX + newY*newY);
      i = (int) (logk/dx+0.5);
      result->dataY[i] += image->data[pos];
      nb[i]++;
    }
  }

  for (i = 0; i < resSize; i++) {
    if (nb[i] != 0) {
      result->dataY[i] /= nb[i];
    }
  }

  free(nb);
  
  store_signal_in_dictionary (name, result);

  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}


/* **********************************************************
   nouvelle fonction : spectre de puissance calcule dans un secteur
   angulaire du plan kx-ky (anisotropie) modif du 03/01/2000 
   par pierre kestener
   ************************************************************* */
/*
 * Command name in xsmurf : myisp2ssp
 */
int
my_isp_2_ssp_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Issffd",
      NULL
    };

  char * help_msg =
    {
      (" Compute the 1d energy spectrum from the 2d energy spectrum (i.e. "
       "log(S(k)) / |k| ) in an angular sector of (kx,ky) plane.\n"
       "\n"
       "Parameters :\n"
       "  image   - image to treat.\n"
       "  string  - signal1 result name.\n"
       "  string  - signal2 result name. \n"
       "  real    - theta1 angle in degrees : from -90 to 90\n"
       "  real    - theta2  (to define the angular sector)\n"
       "  int     - size of the 1d spectrum.")
    };

  /* Command's parameters */
  Image *image;
  char  *name1;
  char  *name2;
  real  theta1;
  real  theta2;
  int   resSize;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *result1;
  Signal *result2;
  real   logk;
  real   cos1, cos2, sin1, sin2;
  int    i;
  int    x, y;
  int    newX, newY;
  int    *nb1;
  int    *nb2;
  int    pos;
  real   dx, max;
  FILE   *fp;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name1, &name2, &theta1, &theta2, &resSize) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  result1 = sig_new (REALY, 0, resSize - 1);
  result2 = sig_new (REALY, 0, resSize - 1);
  nb1 = (int *) malloc(sizeof(int)*resSize);
  nb2 = (int *) malloc(sizeof(int)*resSize);
  for (i = 0; i < resSize; i++) {
    nb1[i] = 0;
    nb2[i] = 0;
    result1->dataY[i] = 0.0;
    result2->dataY[i] = 0.0;
  }

  max = sqrt(2.0*(image->lx*image->ly/4));
  dx = max/(resSize-1);
  result1->x0 = 0.0;
  result2->x0 = 0.0;
  result1->dx = dx;
  result2->dx = dx;

  fp = fopen("/user41/kestener/theta","w");
  fprintf(fp,"%f %f\n", theta1, theta2);
 
  for (x = 0; x < image->lx; x++) {
    for (y = 0; y < image->ly; y++) {
      pos = x + y*image->lx;
      newX = x - image->lx/2;
      newY = y - image->ly/2;
      logk = sqrt(newX*newX + newY*newY);
      i = (int) (logk/dx+0.5);

      /* if (newX == 0) theta = 90.0;
	 else { theta = (180/3.14159265*atan((float)(newY)/(float)(newX))); }
      
	 if (theta >= theta1 & theta <= theta2 ) {
	 result1->dataY[i] += image->data[pos];
	 nb1[i]++;
	
	 }
	 else {
	 result2->dataY[i] += image->data[pos];
	 nb2[i]++;
	 fprintf(fp,"%d %d %f\n",newX,newY,theta);
	 } */

      cos1 = cos(theta1);
      cos2 = cos(theta2);
      sin1 = sin(theta1);
      sin2 = sin(theta2);
      
      if ( (-newX*sin1+newY*cos1 >= 0) & (-newX*sin2+newY*cos2 <= 0) ) {
	result1->dataY[i] += image->data[pos];
	nb1[i]++;
      }
      else {
        result2->dataY[i] += image->data[pos];
	nb2[i]++;
      }  
      if (newX*newY == 0) {
	result2->dataY[i] += image->data[pos];
	nb2[i]++;
      }
      else { }

      /* fprintf(fp,"%d %d \n",newX,newY); */
      /* fprintf(fp,"%d %d %f\n",newX,newY,theta); */
    }
  }

  fclose(fp);


  for (i = 0; i < resSize; i++) {
    if (nb1[i] != 0) {
      result1->dataY[i] /= nb1[i];
    }
    else { };
    if (nb2[i] != 0) {
      result2->dataY[i] /= nb2[i];
    }
    else { };
  }

  store_signal_in_dictionary (name1, result1);
  store_signal_in_dictionary (name2, result2);
  Tcl_AppendResult(interp,name1,NULL);
  Tcl_AppendResult(interp,name2,NULL);

  free(nb1);
  free(nb2);

  return TCL_OK;
}

/*
 * Command name in xsmurf : ipowspec
 */
int
ipowspec_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Isd",
      NULL
    };

  char * help_msg =
    {
      (" Compute the 1d power spectrum of a 2D Fourier image (of type FFTW_R2C).\n"
       "\n"
       "\n"
       "Parameters :\n"
       "  image   - Fourier image to treat.\n"
       "  string  - signal result name.\n"
       "  int     - size of the power spectrum.")
    };

  /* Command's parameters */
  Image *image;
  char  *name;
  int   resSize;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *result;
  real   logk;
  int    i;
  int    x, y;
  int    newX, newY;
  int    *nb;
  int    pos;
  real   dx, max;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name, &resSize) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (image->type!=FFTW_R2C) {
    sprintf (interp -> result,"Image type must be FFTW_R2C (apply first ifftw2d).\n");
    return TCL_ERROR;
  }

  /* Treatement */

  result = sig_new (REALY, 0, resSize - 1);
  nb = (int *) malloc(sizeof(int)*resSize);
  for (i = 0; i < resSize; i++) {
    nb[i] = 0;
    result->dataY[i] = 0.0;
  }

  max = sqrt(2.0*(image->lx*image->ly/4));
  dx = max/(resSize-1);
  result->x0 = 0.0;
  result->dx = dx;

  /* compute Fourier power spectrum of input DFT image */
  im_fftw_powerspectrum(image,result->dataY,resSize,dx,nb);

  free(nb);
  
  store_signal_in_dictionary (name, result);

  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}

/*
 * Command name in xsmurf : i3Dpowspec
 */
int
i3Dpowspec_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Jsd",
      NULL
    };

  char * help_msg =
    {
      (" Compute the 1d power spectrum of a 3D Fourier image (of type FFTW_R2C).\n"
       "\n"
       "\n"
       "Parameters :\n"
       "  image   - Fourier image (3D) to treat.\n"
       "  string  - signal result name.\n"
       "  int     - size of the power spectrum.")
    };

  /* Command's parameters */
  Image3D *image;
  char  *name;
  int   resSize;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *result;
  real   logk;
  int    i;
  int    x, y;
  int    newX, newY;
  int    *nb;
  int    pos;
  real   dx, max;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name, &resSize) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (image->type!=FFTW_R2C) {
    sprintf (interp -> result,"Image type must be FFTW_R2C (apply first ifftw3d).\n");
    return TCL_ERROR;
  }

  /* Treatement */

  result = sig_new (REALY, 0, resSize - 1);
  nb = (int *) malloc(sizeof(int)*resSize);
  for (i = 0; i < resSize; i++) {
    nb[i] = 0;
    result->dataY[i] = 0.0;
  }

  //max = sqrt(3.0*(image->lx*image->ly*image->lz/4));
  max = (real) sqrt(image->lx*image->lx+image->ly*image->ly+image->lz*image->lz )/2.0;
  dx = max/(resSize-1);
  result->x0 = 0.0;
  result->dx = dx;

  /* compute Fourier power spectrum of input DFT image */
  im3D_fftw_powerspectrum(image,result->dataY,resSize,dx,nb);

  free(nb);
  
  store_signal_in_dictionary (name, result);

  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}

/*
 */
int
i_angular_mean_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Isd",
      "-x0", "f",
      "-y0", "f",
      NULL
    };

  char * help_msg =
    {
      (" Compute an angular mean of an image.\n"
       "\n"
       "Parameters:\n"
       "  image   - image to treat.\n"
       "  string  - signal result name.\n"
       "  int     - size of the resulting signal.\n"
       "\n"
       "Options:\n"
       "  -x0: Give the value for the x-position of the center in the image.\n"
       "     Default is lx/2-1.\n"
       "  -y0: Give the value for the y-position of the center in the image.\n"
       "     Default is ly/2-1.\n"
       "\n"
       "Return value:\n"
       "  Name of the resulting signal.")
    };

  /* Command's parameters */
  Image *image;
  char  *name;
  int   resSize;

  /* Options's presence */
  int isX0;
  int isY0;

  /* Options's parameters */
  real x0;
  real y0;

  /* Other variables */
  Signal *result;
  real   dist;
  int    i;
  int    x, y;
  int    newX, newY;
  int    *nb;
  int    pos;
  real   dx;
  real max;
  real max1;
  real max2;
  real corner1;
  real corner2;
  real corner3;
  real corner4;
  int lx;
  int ly;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name, &resSize) == TCL_ERROR)
    return TCL_ERROR;

  isX0 = arg_present(1);
  if (isX0) {
    if (arg_get(1, &x0) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } else {
    x0 = image->lx/2-1;
  }
  isY0 = arg_present(2);
  if (isY0) {
    if (arg_get(2, &y0) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } else {
    y0 = image->ly/2-1;
  }

  /* Parameters validity and initialisation */

  /* Treatement */

  lx = image->lx;
  ly = image->ly;

  result = sig_new (REALY, 0, resSize - 1);
  nb = (int *) malloc(sizeof(int)*resSize);
  for (i = 0; i < resSize; i++) {
    nb[i] = 0;
    result->dataY[i] = 0.0;
  }

  corner1 = sqrt(x0*x0 + y0*y0);
  corner2 = sqrt((lx - x0)*(lx - x0) + y0*y0);
  corner3 = sqrt((lx - x0)*(lx - x0) + (ly - y0)*(ly - y0));
  corner4 = sqrt(x0*x0 + (ly - y0)*(ly - y0));

  max1 = max(corner1, corner2);
  max2 = max(corner3, corner4);
  max = max(max1, max2);

  dx = max/(resSize-1);
  result->x0 = 0.0;
  result->dx = dx;

  for (x = 0; x < image->lx; x++) {
    for (y = 0; y < image->ly; y++) {
      pos = x + y*image->lx;
      newX = x - x0;
      newY = y - y0;
      dist = sqrt(newX*newX + newY*newY);
      i = (int) (dist/dx+0.5);
      result->dataY[i] += image->data[pos];
      nb[i]++;
    }
  }

  for (i = 0; i < resSize; i++) {
    if (nb[i] != 0) {
      result->dataY[i] /= nb[i];
    }
  }

  store_signal_in_dictionary (name, result);

  Tcl_AppendResult(interp,name,NULL);

  free (nb);

  return TCL_OK;
}


/**********************************
 * Command name in xsmurf : iicut
 **********************************/
int
iicut_TclCmd_ (ClientData clientData,
	       Tcl_Interp *interp,
	       int        argc,
	       char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Isdddd",
    NULL
  };

  char * help_msg = {
    (" Cut an image from an other image.\n"
     "\n"
     "Parameters :\n"
     "  image  - image to cut (source).\n"
     "  string - image result name.\n"
     "  2 int  - position of the result in the source.\n"
     "  2 int  - width and heigth of the result.")
  };

  /* Command's parameters */
  Image *image;
  char  *name;
  int   posX, posY;
  int   width, heigth;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image *result;
  int    xs, ys;
  int    xr, yr;
  int    poss;
  int    posr;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name, &posX, &posY, &width, &heigth) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  result = im_new (width, heigth, width*heigth, PHYSICAL);

  if (!result)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (result);

  /* The following loop if for negatives value of posX. */
  /* si posX < 0 */
  /* a la fin de cette boucle xs vaut 0 si posX etait <0 */
  /* et xr est incremente de abs(posX) */
  
  /* si posX >=0 , on ne rentre pas dans la boucle */

  for (xr = 0, xs = posX; xr < width && xs < 0; xr++, xs++) {
  }
  for (; xr < width && xs < image->lx; xr++, xs++) {
    /* The following loop if for negatives value of posY. */
    for (yr = 0, ys = posY; yr < heigth && ys < 0; yr++, ys++) {
    }
    for (; yr < heigth && ys < image->ly; yr++, ys++) {
      posr = xr + width*yr;
      poss = xs + image->lx*ys;
      result->data[posr] = image->data[poss];
    }
  }

  store_image(name, result);

  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}

/**************************************
 * Command name in xsmurf : iperiodize
 **************************************/
int
iperiodize_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Is",
    NULL
  };

  char * help_msg = {
    (" Return a new image twice as large as the original, but periodic.\n"
     "\n"
     "Parameters :\n"
     "  image  - image to cut (source).\n"
     "  string - image result name.\n")
  };

  /* Command's parameters */
  Image *image;
  char  *name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image *result;
  int    lx,ly;
  int    i,j;
  int    pos, pos2;
  real   data;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  lx = image->lx;
  ly = image->ly;

  /* Treatement */

  result = im_new (2*lx, 2*ly, 4*lx*ly, PHYSICAL);

  if (!result)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (result);

  for (j=0; j<ly; j++)
    for (i=0; i<lx; i++) {
      
      if (i<lx/2 && j<ly/2) {
	pos = i + lx*j;
	data = image->data[pos];
	result->data[lx/2  +i + 2*lx*(ly/2  +j)] = data;
	result->data[lx/2-1-i + 2*lx*(ly/2  +j)] = data;
	result->data[lx/2  +i + 2*lx*(ly/2-1-j)] = data;
	result->data[lx/2-1-i + 2*lx*(ly/2-1-j)] = data;

      }

      if (i<lx/2 && j>=ly/2) {
	pos = i + lx*j;
	data = image->data[pos];
	result->data[lx/2  +i + 2*lx*(     ly/2  +j)] = data;
	result->data[lx/2-1-i + 2*lx*(     ly/2  +j)] = data;
	result->data[lx/2  +i + 2*lx*(2*ly+ly/2-1-j)] = data;
	result->data[lx/2-1-i + 2*lx*(2*ly+ly/2-1-j)] = data;

      }

      if (i>=lx/2 && j<ly/2) {
	pos = i + lx*j;
	data = image->data[pos];
	result->data[     lx/2  +i + 2*lx*(ly/2  +j)] = data;
	result->data[2*lx+lx/2-1-i + 2*lx*(ly/2  +j)] = data;
	result->data[     lx/2  +i + 2*lx*(ly/2-1-j)] = data;
	result->data[2*lx+lx/2-1-i + 2*lx*(ly/2-1-j)] = data;

      }

      if (i>=lx/2 && j>=ly/2) {
	pos = i + lx*j;
	data = image->data[pos];
	result->data[     lx/2  +i + 2*lx*(     ly/2  +j)] = data;
	result->data[2*lx+lx/2-1-i + 2*lx*(     ly/2  +j)] = data;
	result->data[     lx/2  +i + 2*lx*(2*ly+ly/2-1-j)] = data;
	result->data[2*lx+lx/2-1-i + 2*lx*(2*ly+ly/2-1-j)] = data;

      }

    }

  store_image(name, result);

  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}


/*************************************
 * Command name in xsmurf : ithresh
 *************************************/
int
im_thresh_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{
  /* Command line definition */
  char * options[] = {
    "Iff",
    "-val", "f",
    NULL
  };
 
  char * help_msg =
    {("  Thresh the values of an image that are within a domain."
      "\n"
      "Parameters :\n"
      "  image    - image to treat.\n"
      "  2 floats - domain of the threshold\n"
      "\n"
      "Options :\n"
      "  -val : Change the \"replacing\" value (which default is 0).\n"
      "     float - The value.\n"
      "\n"
      "Return Value :\n"
      "  None.")};

  /* Command's parameters */
  Image *image;
  real  xmin;
  real  xmax;

  /* Options's presence */

  /* Options's parameters */
  real  value = 0.0;

  /* Other variables */

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (1, &value) == TCL_ERROR)
    return TCL_ERROR;

  /* Treatement */

  image = im_thresh (image, xmin, xmax, value);

  return TCL_OK;
}

/*************************************
 * Command name in xsmurf : ithresh3D
 *************************************/
int
im_thresh_3D_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{
  /* Command line definition */
  char * options[] = {
    "Jff",
    "-val", "f",
    NULL
  };
 
  char * help_msg =
    {("  Thresh the values of a 3D image that are within a domain."
      "\n"
      "Parameters :\n"
      "  image 3D    - image to treat.\n"
      "  2 floats    - domain of the threshold\n"
      "\n"
      "Options :\n"
      "  -val : Change the \"replacing\" value (which default is 0).\n"
      "     float - The value.\n"
      "\n"
      "Return Value :\n"
      "  None.")};

  /* Command's parameters */
  Image3D *image;
  real     xmin;
  real     xmax;

  /* Options's presence */

  /* Options's parameters */
  real  value = 0.0;

  /* Other variables */
  real *data;
  int   i;
  int size;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &xmin, &xmax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (1, &value) == TCL_ERROR)
    return TCL_ERROR;

  /* Treatement */
  size = image->size;
  data = image->data;
  for (i = 0; i < size; i++) {
    if (data[i] < xmin || data[i] > xmax) {
      data[i] = value;
    }
  }


  return TCL_OK;
}


/*
 */
int
isubsample_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Isd",
    NULL
  };

  char * help_msg = {
    (" Subsample an image (in x and y direction).\n"
     "\n"
     "Parameters :\n"
     "  image  - image to cut (source).\n"
     "  string - image result name.\n"
     "  int    - step of the subsample.")
  };

  /* Command's parameters */
  Image *image;
  char  *name;
  int   step;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image *result;
  int    x, y,i, new_i;
  int    new_size, new_lx, new_ly;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name, &step) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  new_lx = image->lx/step;
  new_ly = image->ly/step;
  new_size = new_lx*new_ly;

  result = im_new (new_lx, new_ly, new_size, PHYSICAL);

  if (!result)
    return GenErrorMemoryAlloc(interp);
  im_set_0 (result);

  /* The following loop if for negatives value of posX. */
  i=0;
  new_i=0;
  for (x = 0; x < image->lx; x++) {
    for (y = 0; y < image->ly; y++) {
      if (x%step == 0) {
	if (y%step == 0) {
	  result->data[new_i] = image->data[i];
	  new_i++;
	}
      }
      i++;
    }
  }

  store_image(name, result);

  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}


/*
 */
int
i_apply_ipa_table_TclCmd_ (ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "ISs",
    NULL
  };

  char * help_msg = {
    (" Apply to an image an IPA table stored in a signal.\n"
     "\n"
     "Parameters :\n"
     "  image  - Image to treat. It must be of type PHYSICAL\n"
     "  signal - Signal where the IPA table is stored.\n"
     "  string - Image result name.")
  };

  /* Command's parameters */
  Image  *image;
  Signal *signal;
  char   *name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image *result;
  real  *idata;
  real  *sdata;
  int    pos;
  int    index;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &signal, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (image->type != PHYSICAL) {
    sprintf (interp->result, "bad image type");
    return TCL_ERROR;
  }

  /* Treatement */

  result = im_new (image->lx, image->ly, image->size, PHYSICAL);
  if (!result) {
    return GenErrorMemoryAlloc(interp);
  }

  idata = image->data;
  sdata = signal->dataY;
  for (pos = 0; pos < image->ly*image->lx; pos++) {
    index = min(max((int)(idata[pos]*10), 0), signal->n-2);
    result->data[pos] =  sdata[index+1]*(idata[pos]*10 - index)
      + sdata[index]*(index + 1 - idata[pos]*10);
  }

  store_image(name, result);

  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}


/*
 */
int
i_inverse_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Is",
    NULL
  };

  char * help_msg = {
    (" Invert an image. f(-x,-y).\n"
     "\n"
     "Parameters :\n"
     "  image  - Image to treat.\n"
     "  string - Image result name.")
  };

  /* Command's parameters */
  Image  *image;
  char   *name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image *result;
  int    pos1;
  int    pos2;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  result = im_new (image->lx, image->ly, image->size, PHYSICAL);
  if (!result) {
    return GenErrorMemoryAlloc(interp);
  }

  for (pos1 = 0, pos2 = image->ly*image->lx-1;
       pos1 < image->ly*image->lx;
       pos1++, pos2--) {
    result->data[pos2] = image->data[pos1];
  }

  store_image(name, result);

  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}


/*******************************************************
 * Command name in xsmurf : vector2D_2_vtk 
 *******************************************************/
/* created  on june 30th 2003*/
int 
vector2D_2_vtk_structured_points_TclCmd_ (ClientData clientData,
					  Tcl_Interp *interp,
					  int        argc,
					  char       **argv)
{     
  /* Command line definition */
  char * options[] =
    {
      "IIs",
      "-function", "s",
      "-border", "d",
      "-special", "df",
      "-reduce", "d",
      NULL
    };

  char * help_msg =
    {
      (" Convert two images (float, same size) in a file\n" 
       "using VTK file format (STRUCTURED_POINTS) with field VECTORS.\n"
       "See : http://public.kitware.com/FileFormats.pdf\n"
       "see also command \"vector3D_2_vtk\"\n"
       "\n"
       "Parameters :\n"
       "  2 strings      - Names of the two input images.\n"
       "  string         - Name of output file.\n"
       "                   example: \"toto.vtk\"\n"
       "\n"
       "Options:\n"
       "   -function [s]: fuction to rescale data\n"
       "   -border   [d]: cut border of thickness the parameter\n"
       "   -special  [df]: set to special_value (second parameter) the field\n"
       "                   \"scalars\" when point is outside a sphere of\n"
       "                    some radius (given as first parameter).\n"
       "   -reduce [d] : save only one over several points...\n"
       "\n")
    };

  /* Command's parameters */
  Image    *image1, *image2;
  char     *filenameOut = NULL;
  FILE     *fileOut;
  real     *data1, *data2;
  int       i,j;
  int       lx,ly;

  /* Option */
  int isFunction;
  int isBorder;
  int isSpecial;
  int isReduce;

  /* Other variables */
  char     *fct_expr;
  void     *fct;
  int      thickness;
  int      nb_points;
  int      radius;
  float    special_value;
  int      reduce_param=1;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2, &filenameOut) == TCL_ERROR)
    return TCL_ERROR;
  
  
  isFunction  = arg_present (1);
  if (isFunction) {
    if (arg_get(1, &fct_expr) == TCL_ERROR) {
      return TCL_ERROR;
    }
    fct = evaluator_create(fct_expr);
    if (!fct) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  }
  isBorder = arg_present (2);
  if (isBorder) {
    if (arg_get(2, &thickness) == TCL_ERROR) {
      return TCL_ERROR;
    }
    if (thickness<0 || thickness>lx/2-1) {
      Tcl_AppendResult (interp, "Error in parameter of option \"-border\" \n",(char *) NULL);
      return TCL_ERROR;
    }
  }

  isSpecial = arg_present (3);
  if (isSpecial) {
    if (arg_get(3, &radius, &special_value) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isReduce = arg_present (4);
  if (isReduce) {
    if (arg_get(4, &reduce_param) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  lx = image1->lx;
  ly = image1->ly;

  data1 = image1->data;
  data2 = image2->data;
  
  /* find min max */
  if (isFunction) {
    float rescale1, rescale2;
    for (i=0;i<lx*ly;i++) {
      rescale1 = sqrt(data1[i]*data1[i] + data2[i]*data2[i]);
      rescale2 = evaluator_evaluate_x_y (fct,rescale1,0.0);
      data1[i] *= rescale2/rescale1;
      data2[i] *= rescale2/rescale1;
    }
  }
  
  
  if (isBorder) {
    nb_points = 0;
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	if (i<thickness || j<thickness ||  i>lx-1-thickness || j>ly-1-thickness) {
	} else {
	  nb_points++;
	}
      }
    }
  } else {
    nb_points = 0;
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	if ((i%reduce_param)==0 && (j%reduce_param)==0)
	  nb_points++;
      }
    }
  }

  /* fill header */
  fileOut = fopen(filenameOut, "w");
  fprintf(fileOut, "# vtk DataFile Version 1.0 \n");
  fprintf(fileOut, "data generated by xsmurf.\n");
  fprintf(fileOut, "ASCII\n");
  //fprintf(fileOut, "BINARY\n");
  fprintf(fileOut, "\n");
  fprintf(fileOut,"DATASET STRUCTURED_POINTS\n");
  if (isBorder) {
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx-2*thickness, ly-2*thickness, 1);
  } else {
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx/reduce_param, ly/reduce_param, 1);
  }
  fprintf(fileOut,"ORIGIN %f %f %f\n", 0.0, 0.0, 0.0);
  fprintf(fileOut,"SPACING %f %f %f\n", (float) reduce_param, (float) reduce_param, 1.0);
  fprintf(fileOut," \n");

  // first scalar field : gradient modulus
  if (isBorder) {
    fprintf(fileOut,"POINT_DATA %d\n", nb_points);
  } else {
    fprintf(fileOut,"POINT_DATA %d\n", nb_points);
    //fprintf(fileOut,"POINT_DATA %d\n", lx*ly);
  }
  fprintf(fileOut,"SCALARS scalars float\n");
  fprintf(fileOut,"LOOKUP_TABLE default\n");


  if (isBorder && !isSpecial) {
    float tmp;
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	if (i< thickness || j< thickness || i>lx-1-thickness || j>ly-1-thickness) {
	} else {
	  tmp = sqrt(data1[i+j*lx]*data1[i+j*lx]+data2[i+j*lx]*data2[i+j*lx]);
	  fprintf(fileOut, "%f ", tmp);
	}
      }
    }
  } else if (isBorder && isSpecial) {
    float tmp;
    int   rad2;
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	rad2  = (i-lx/2)*(i-lx/2) + (j-ly/2)*(j-ly/2);
	if (i< thickness || j< thickness || i>lx-1-thickness || j>ly-1-thickness) {
	} else {
	  tmp = sqrt(data1[i+j*lx]*data1[i+j*lx]+data2[i+j*lx]*data2[i+j*lx]);
	  if (rad2>radius*radius) {
	    fprintf(fileOut, "%f ", special_value);
	  } else {
	    fprintf(fileOut, "%f ", tmp);
	  }
	}
      }
    }
  } else {
    float tmp;
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	if ((i%reduce_param)==0 && (j%reduce_param)==0) { 
	  tmp = sqrt(data1[i+j*lx]*data1[i+j*lx]+data2[i+j*lx]*data2[i+j*lx]);
	  fprintf(fileOut, "%f ", tmp);
	}
      }
    }
  }
  
  // next vector field !!!
  fprintf(fileOut,"VECTORS vectors float\n"); 
  
  if (isBorder) {
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	if (i< thickness || j< thickness || i>lx-1-thickness || j>ly-1-thickness) {
	} else {
	  fprintf(fileOut, "%f %f %f\n", data1[i+j*lx], data2[i+j*lx], 0.0);
	}
      }
    }
  } else {
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	if ((i%reduce_param)==0 && (j%reduce_param)==0) 
	  fprintf(fileOut, "%f %f %f\n", data1[i+j*lx], data2[i+j*lx], 0.0);
      }
    }
  }


  evaluator_destroy(fct);

  fclose(fileOut);
  
  return TCL_OK;
  
}

/*******************************************************
 * Command name in xsmurf : vector2D_2_jvx 
 *******************************************************/
/* created  on december 30th 2005*/
int 
vector2D_2_jvx_vectorField_TclCmd_ (ClientData clientData,
				    Tcl_Interp *interp,
				    int        argc,
				    char       **argv)
{     
  /* Command line definition */
  char * options[] =
    {
      "IIs",
      NULL
    };

  char * help_msg =
    {
      (" Convert two images (float, same size) in a file\n" 
       "using JVX file format (Javaview software).\n"
       "See : http://www.javaview.de/guide/formats/Format_Jvx.html\n"
       "see also command \"vector2D_2_vtk\"\n"
       "\n"
       "Parameters :\n"
       "  2 strings      - Names of the two input images.\n"
       "  string         - Name of output file.\n"
       "                   example: \"myVectorField.jvx\"\n"
       "\n"
       "Options:\n"
       "\n")
    };

  /* Command's parameters */
  Image    *image1, *image2;
  char     *filenameOut = NULL;
  FILE     *fileOut;
  real     *data1, *data2;
  int       i,j;
  int       lx,ly;

  /* Options */

  /* Other variables */
  time_t   rawtime;
  char    *userName;
  char    *myTime;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2, &filenameOut) == TCL_ERROR)
    return TCL_ERROR;
  
    
  lx = image1->lx;
  ly = image1->ly;

  data1 = image1->data;
  data2 = image2->data;
  
  /* set userName */    
  {
    struct passwd *passwd;
    /* Get the uid of the running
     * process and use it to get
     * a record from /etc/passwd
     */  
    passwd = getpwuid(geteuid());
    userName = passwd->pw_gecos;
  }

  /* fill header */
  fileOut = fopen(filenameOut, "w");
  fprintf(fileOut, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"yes\"?>\n");
  fprintf(fileOut, "<!DOCTYPE jvx-model SYSTEM \"http://www.javaview.de/rsrc/jvx.dtd\">\n");
  fprintf(fileOut, "<jvx-model>\n");
  fprintf(fileOut, "\t<meta generator=\"Xsmurf\"/>\n");
  time ( &rawtime );
  myTime = ctime (&rawtime);
  myTime[strlen(myTime)-1] = '\0';
  fprintf(fileOut, "\t<meta date=\"%s\"/>\n", myTime);
  fprintf(fileOut, "\t<version type=\"final\">2.00</version>\n");
  fprintf(fileOut, "\t<title>Vector field data</title>\n");
  fprintf(fileOut, "\t<description>\n");
  fprintf(fileOut, "\t\t<abstract>Vector field data</abstract>\n");
  fprintf(fileOut, "\t</description>\n");  
    
  fprintf(fileOut, "\t<authors>\n");
  fprintf(fileOut, "\t\t<author>\n");
  fprintf(fileOut, "\t\t\t<firstname></firstname>\n");
  fprintf(fileOut, "\t\t\t<lastname>%s</lastname>\n", userName);
  fprintf(fileOut, "\t\t\t<affiliation>\n");
  fprintf(fileOut, "\t\t\t\t<organization>CEA Saclay</organization>\n");
  fprintf(fileOut, "\t\t\t\t<address>\n");
  fprintf(fileOut, "\t\t\t\t\t<line>Centre de Saclay</line>\n");
  fprintf(fileOut, "\t\t\t\t\t<line>91191 Gif-Sur-Yvette, FRANCE</line>\n");
  fprintf(fileOut, "\t\t\t\t</address>\n");
  fprintf(fileOut, "\t\t\t</affiliation>\n");
  fprintf(fileOut, "\t\t\t<email>someone@somewhere.suffix</email>\n");
  fprintf(fileOut, "\t\t\t<url>http://www.somewhere.suffix/</url>\n");
  fprintf(fileOut, "\t\t</author>\n");
  fprintf(fileOut, "\t</authors>\n");

  fprintf(fileOut, "\t<geometries>\n");
  fprintf(fileOut, "\t\t<geometry name=\"points data\">\n");

  /* pointSet data */
  fprintf(fileOut, "\t\t\t<pointSet dim=\"2\" point=\"hide\">\n");
  fprintf(fileOut, "\t\t\t\t<points>\n");
  for (i=0;i<lx+1;i++) {
    for (j=0;j<ly+1;j++) {
      fprintf(fileOut, "\t\t\t\t\t<p>%d %d</p>\n", i, j);
    }
  }
  fprintf(fileOut, "\t\t\t\t</points>\n");
  fprintf(fileOut, "\t\t\t</pointSet>\n");

  /* faceSet data */
  fprintf(fileOut, "\t\t\t<faceSet face=\"show\" edge=\"hide\">\n");
  fprintf(fileOut, "\t\t\t\t<faces>\n");
  /* face rectangular, triangulation is done in javaview ! */
  for (i=0;i<lx;i++) {
    for (j=0;j<ly;j++) {
      fprintf(fileOut, "\t\t\t\t\t<f>%d %d %d %d</f>\n", i+j*lx, i+1+j*lx, i+(j+1)*lx, i+1+(j+1)*lx);

    }
  }
  fprintf(fileOut, "\t\t\t\t</faces>\n");
  fprintf(fileOut, "\t\t\t</faceSet>\n");
  
  
  /* vectorField data */
  fprintf(fileOut,"\t\t\t<vectorField name=\"some vector field\" arrow=\"show\" base=\"element\">\n"); 
  fprintf(fileOut, "\t\t\t\t<vectors>\n");
  for (i=0;i<lx;i++) {
    for (j=0;j<ly;j++) {
      fprintf(fileOut, "\t\t\t\t\t<v>%f %f</v>\n", data1[i+j*lx], data2[i+j*lx]);
    }
  }
  fprintf(fileOut, "\t\t\t\t</vectors>\n");
  fprintf(fileOut, "\t\t\t</vectorField>\n");
  fprintf(fileOut, "\t\t</geometry>\n");
  fprintf(fileOut, "\t</geometries>\n");
  fprintf(fileOut, "</jvx-model>\n");
 
  fclose(fileOut);
  
  return TCL_OK;
  
}

/*******************************************************
 * Command name in xsmurf : vector2D_2_flowvis
 *******************************************************/
/* created  on february 17th 2004*/
int 
vector2D_2_flow_vis_TclCmd_ (ClientData clientData,
			     Tcl_Interp *interp,
			     int        argc,
			     char       **argv)
{     
  /* Command line definition */
  char * options[] =
    {
      "IIs",
      NULL
    };

  char * help_msg =
    {
      (" Convert two images (float, same size) in a file\n" 
       "using Flow Vis file format  for vector fields.\n"
       "See : http://numerik.math.uni-duisburg.de/exports/flowVis/index.html\n"
       "(personal web page of Tobias Preusser).\n"
       "See also command \"vector2D_2_vtk\"\n"
       "\n"
       "Parameters :\n"
       "  2 strings      - Names of the two input images.\n"
       "  string         - Name of output file.\n"
       "                   example: \"toto.vtk\"\n"
       "\n"
       "Options:\n"
       "\n")
    };

  /* Command's parameters */
  Image    *image1, *image2;
  char     *filenameOut = NULL;
  FILE     *fileOut;
  real     *data1, *data2;
  int       i,j;
  int       lx,ly;

  /* Option */

  /* Other variables */

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2, &filenameOut) == TCL_ERROR)
    return TCL_ERROR;
  
    
  lx = image1->lx;
  ly = image1->ly;

  data1 = image1->data;
  data2 = image2->data;
  
 

  /* fill header */
  fileOut = fopen(filenameOut, "w");
  fprintf(fileOut, "V2\n");
  fprintf(fileOut,"%d %d\n", lx, ly);
  
  
  // next vector field !!!
  for (i=0;i<lx;i++) {
    for (j=0;j<ly;j++) {
      fprintf(fileOut, "%f %f\n", data1[i+j*lx], data2[i+j*lx]);
    }
  }

  fclose(fileOut);
  
  return TCL_OK;
  
}

/************************************
 * command name in xsmurf : i3Dcomb
 ************************************/
int
comb_images_3D_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "JJss",
      NULL
    };

  char * help_msg =
    {
      (" Create a 3D image as a combination of two 3D images.\n"
       "\n"
       "Parameters :\n"
       "  2 3D Images - 3D images to combine.\n"
       "  string      - expression of the combination (defunc expression). The\n"
       "                images are designed by the tokens 'x' and 'y' (ex.\n"
       "                x*cos(y) compute the first image times the cosine of\n"
       "                the second one.\n"
       "  string      - name of the result.")
    };

  /* Command's parameters */
  Image3D  *image1, *image2;
  char     *fct_expr;
  char     *res_name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image3D  *result;
  void     *fct;
  int       i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image1, &image2, &fct_expr, &res_name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  
  if (image1->lx!=image2->lx || image1->ly!=image2->ly || image1->lz!=image2->lz)
    {
      sprintf (interp -> result,"Images must have the same sizes.\n");
      return TCL_ERROR;
    }
  fct = evaluator_create(fct_expr);
  if (!fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }


  /* Treatement */
  result = im3D_new(image1->lx, image1->ly, image1->lz, image1->size, PHYSICAL);
  if (!result)
    return GenErrorMemoryAlloc(interp);
  
  for (i = 0; i < result->lx*result->ly*result->lz; i++) {
    /*result -> data[i] = fct (image1 -> data[i], image2 -> data[i]);*/
    result -> data[i] = evaluator_evaluate_x_y (fct, image1 -> data[i], image2 -> data[i]);
  }
  
  /*dfclose (fct);*/
  evaluator_destroy(fct);
  store_image3D (res_name, result);
  Tcl_AppendResult(interp, res_name, NULL);

  return TCL_OK;
}

/************************************
 * Command name in xsmurf : iicut3D
 ************************************/
int
iicut_3D_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Jsdddddd",
    NULL
  };

  char * help_msg = {
    (" Cut a 3D image from an other 3D image.\n"
     "\n"
     "Parameters :\n"
     "  image  - image to cut (source).\n"
     "  string - image result name.\n"
     "  3 int  - position of the result in the source.\n"
     "  3 int  - width, height and large of the result.\n")
  };

  /* Command's parameters */
  Image3D *image;
  char  *name;
  int   posX, posY, posZ;
  int   lx, ly, lz;
  int   width, height, large;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Image3D *result;
  int    xs, ys, zs; /* source */
  int    xr, yr, zr; /* result */
  int    poss;
  int    posr;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name, &posX, &posY, &posZ, &width, &height, &large) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (posX <0 || posY <0 || posZ <0)
    return GenErrorAppend(interp, "error upper left corner must have positive coordinates...", NULL);
  if (posX+width>=image->lx || posY+height>=image->ly || posZ+large>=image->lz)
    return GenErrorAppend(interp, "error lower right corner must be inside input 3D image...", NULL);
  

  /* Treatement */
  result = im3D_new (width, height, large, width*height*large, PHYSICAL);

  if (!result)
    return GenErrorMemoryAlloc(interp);
  im3D_set_0 (result);
  
  lx = image -> lx;
  ly = image -> ly;
  lz = image -> lz;

  for (xs = posX, xr = 0; xs < posX+width;   xs++, xr++) {
    for (ys = posY, yr = 0; ys < posY+height; ys++, yr++) {
      for (zs = posZ, zr = 0; zs < posZ+large; zs++, zr++) {
	posr = xr + width*yr + width*height*zr;
	poss = xs + lx*ys + lx*ly*zs;
	/*poss = xs + lx*ys;*/
	result->data[posr] = image->data[poss];
      }
    }
  }
  
  store_image3D(name, result);

  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}

/*****************************************
 * Command name in xsmurf : iappodisation
 *****************************************/
int
im_appodisation_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{
  char * options[] = { "If",
		       NULL };
  
  char * help_msg =
    {("Appodization of an image (name) using function 1/2*(1-erfc(x/k))\n"
      "where k is the parameter given as second argument\n"
      "\n")};

  int lx, ly;
  int i,j;
  Image * image;
  real    param, *data;
 
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &param) == TCL_ERROR)
    return TCL_ERROR;

  lx = image->lx;
  ly = image->ly;
  data = image->data;

  for (i=0; i<lx; i++)
    for (j=0; j<ly; j++) {
      
      data [i+j*lx] *= (1-erfcf((float) i/param))/2 * (1-erfcf((float) (lx-i)/param))/2 * (1-erfcf((float) j/param))/2 * (1-erfcf((float) (ly-j)/param))/2;

    }
    
  return TCL_OK;

}

/*******************************************
 * Command name in xsmurf : i3Dappodisation
 *******************************************/
int
im_3D_appodisation_TclCmd_ (ClientData clientData,
                            Tcl_Interp *interp,
                            int        argc,
                            char       **argv)
{
  char * options[] = { "Jf",
                       NULL };

  char * help_msg =
    {("Appodization of a 3D image (name) using function 1/2*(1-erfc(x/k))\n"
      "where k is the parameter given as second argument\n"
      "\n")};

  int lx, ly, lz;
  int i,j,k;
  Image3D * image;
  real      param, *data;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &image, &param) == TCL_ERROR)
    return TCL_ERROR;

  lx = image->lx;
  ly = image->ly;
  lz = image->lz;
  data = image->data;

  for (i=0; i<lx; i++)
    for (j=0; j<ly; j++)
      for (k=0; k<lz; k++) {
	
        data [i+j*lx+k*lx*ly] *= (1-erfcf((float) i/param))/2 * (1-erfcf((float) (lx-i)/param))/2 * (1-erfcf((float) j/param))/2 * (1-erfcf((float) (ly-j)/param))/2 * (1-erfcf((float) k/param))/2 * (1-erfcf((float) (lz-k)/param))/2;
	
      }
  
  return TCL_OK;
  
}

/*************************************
 * Commande name in xsmurf : imlogincr
 *************************************/
int
image_moment_of_log_increments_TclCmd_ (ClientData clientData,
					Tcl_Interp *interp,
					int        argc,
					char       **argv)
{ 
  /* Command line definition */
  char * options[] =
    {
      "Is",
      "-baselog", "f",
      "-direction", "d",
      "-power", "d",
      "-mask", "I",
      "-nolog", "",
      NULL
    };

  char * help_msg =
    {
      (" This routine returns a signal with the nth-moment of the\n"
       " logarithm of the absolute value of increments along x-axis (resp. \n"
       " y-axis) and over various distances ranging from 1 pixel to \n"
       " x-axis size-1 pixels (resp. y-axis size-1).\n"
       "\n"
       "Parameters :\n"
       "  image  - image to treat.\n"
       "  string - name of result (signal)\n"
       "\n"
       "Options :\n"
       "  -baselog [f] : base of the logarithm\n"
       "             (default is neperian log).\n"
       "  -direction [d]: direction of the increments\n"
       "                  0 -> x-axis (default)\n"
       "                  1 -> y-axis\n"
       "  -power [d] : order of the moment\n"
       "  -mask [I]  : use only point such that pixel value of mask is\n"
       "               positive\n"
       "  -nolog []  : standard moment\n"
       )
    };

  /* Command's parameters */
  Image *image;
  char  *name;

  /* Options's presence */
  int is_baselog;
  int is_direction;
  int is_power;
  int is_mask;
  int is_nolog;

  /* Options's parameters */
  real baselog = exp(1);
  int direction = 0; 
  int power = 1;
  Image *mask;

  /* Other variables */
  Signal *result;
  int     x, y;
  int     i, j, lx, ly, sig_size, count, dist;
  int     index1, index2;
  double  incr, sumIncr;
  real   *data;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &name) == TCL_ERROR)
    return TCL_ERROR;

  is_baselog = arg_present (1);
  if (is_baselog)
    if (arg_get (1, &baselog) == TCL_ERROR)
      return TCL_ERROR;
  
  is_direction = arg_present (2);
  if (is_direction)
    if (arg_get (2, &direction) == TCL_ERROR)
      return TCL_ERROR;
  
  is_power = arg_present (3);
  if (is_power)
    if (arg_get (3, &power) == TCL_ERROR)
      return TCL_ERROR;
  
  is_mask = arg_present (4);
  if (is_mask)
    if (arg_get (4, &mask) == TCL_ERROR)
      return TCL_ERROR;
  is_nolog = arg_present (5);

  /* Parameters validity and initialisation */
  if (is_direction && (direction != 0 && direction != 1)) {
    sprintf (interp->result, "Bad direction ([%d]). Should be 0 or 1.", direction);
    return TCL_ERROR;
  }

  
  /* Treatement */
  data = image -> data;
  lx = image->lx;
  ly = image->ly;

  if (direction == 0) {
    sig_size = image->lx;
  } else {
    sig_size = image->ly;    
  }
  result = sig_new (REALY, 0, sig_size - 1);
  result->dataY[0] = 0.0;


  if (direction == 0) {
    for (dist = 1; dist < lx; dist++) {
      count = 0;
      sumIncr = 0.0;
      for (i=0;i<lx-dist;i++) {
	for (j=0;j<ly;j++) {
	  count++;
	  index1 = i + j*lx;
	  index2 = index1 + dist;
	  
	  if (is_mask && (mask->data[index1] <= 0.0 || mask->data[index2] <= 0.0))
	    continue;

	  if (data[index1] != data[index2]) {
	    if (is_nolog) {
	      incr = pow(fabs(data[index1] - data[index2]),(double) power);
	    } else {
	      incr = pow(log(fabs(data[index1] - data[index2]))/log((double)baselog),(double) power);
	    }
	  } else {
	    incr = 0.0;
	  }
	  /*incr = abs(data[index1] - data[index2]);
	    incr = log(incr)/log(baselog);
	    incr = pow(incr,(double) power);*/

	  sumIncr += incr;
	}
      }
      result->dataY[dist] = (real) sumIncr/count;
      /*printf("result[%d] = %f (%d)\n",dist,result->dataY[dist],count);*/
    }
  } else {
    for (dist = 1; dist < ly; dist++) {
      count = 0;
      sumIncr = 0.0;
      for (i=0;i<lx;i++) {
	for (j=0;j<ly-dist;j++) {
	  count++;
	  index1 = i + j*lx;
	  index2 = index1+dist*lx;
	  
	  if (data[index1] != data[index2]) {
	    if (is_nolog) {
	      incr = pow(fabs(data[index1] - data[index2]),(double)power);
	    } else {
	      incr = pow(log(fabs(data[index1] - data[index2]))/log((double) baselog),(double)power);
	    }
	  } else {
	    incr = 0.0;
	  }
	  sumIncr += incr;
	}
      }
      result->dataY[dist] = (real) sumIncr/count;
    }
  }

  store_signal_in_dictionary (name, result);

  return TCL_OK;
}

