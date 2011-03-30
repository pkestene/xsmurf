/*
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster and Stephane Roux.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *  or  decoster@info.enserb.u-bordeaux.fr
 *
 *  $Id: files_cmds.c,v 1.14 1998/12/13 16:58:58 decoster Exp $
 */

#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <tcl.h>
#include "../signal/signal.h"
#include "../image/image.h"
#include <wt2d.h>
#include "arguments.h"
#include "hash_tables.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef PI
#define PI 3.14159265358979323846
#endif

/*void store_signal_in_dictionary (char   *name, Signal *signal_ptr);
int ExtDicImageListGet(Tcl_Interp * interp,char ** expr, 
		       ExtImage *** extImageListPtr);
void ExtMisSortImageList_(ExtImage ** list);
void ExtDicStore(char * name, ExtImage * image_ptr);
void store_image (char  *name, Image *image_ptr);
void unstore_line_by_value (Line *line);
*/
/*
 */
int
xv_file_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Is",
    "-loc", "Ef[dd]",
    NULL
  };

  char * help_msg =
  {
    ("Save an image in a format readable by xview (pbm greyscale).\n"
     "\n"
     "Parameters :\n"
     "  image  - image to save.\n"
     "  string - file name.\n"
     "Otions :\n"
     "  -loc : add a circle for each maximum of an ext image.\n"
     "     ext image    : ext image to add.\n"
     "     real         : ratio for the circle radius.\n"
     "     [2 integers] : coordinates of beginning oh the image in the ext_image.")
  };

  /* Command's parameters */
  Image    *image;
  char     *file_name;

  /* Options's presence */
  int is_loc;

  /* Options's parameters */
  ExtImage *ext_image;
  real     ratio;
  int      x_min;
  int      y_min;

  /* Other variables */
  FILE *out_file;
  real val_min, val_max;
  int  i;

  unsigned char *file_val;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &file_name) == TCL_ERROR)
    return TCL_ERROR;

  is_loc = arg_present (1);
  if (is_loc)
    {
      x_min = 0;
      y_min = 0;

      if (arg_get (1, &ext_image, &ratio, &x_min, &y_min) == TCL_ERROR)
	return TCL_ERROR;
    }

  /* Parameters validity and initialisation */
  if (is_loc)
    {
      if (ratio < 0.0)
	{
	  sprintf (interp -> result, "The ratio (%f) must be positive.", ratio);
	  return TCL_ERROR;
	}

      if (x_min < 0)      x_min = 0;
      if (y_min < 0)      y_min = 0;
    }

  /* Treatement */

  /* First we fill the file data with the image. */
  file_val = (unsigned char *) malloc (sizeof (unsigned char)*image->lx*image->ly);

  im_get_extrema (image, &val_max, &val_min);

  for (i = 0; i < image -> lx*image -> ly; i++)
      file_val[i] = 255 - (unsigned char) ((image -> data[i] - val_min)
					   *255.0/(val_max - val_min));

  /* Then we add the circles for the ext image (if needed). */
  if (is_loc)
    {
      int   position;
      int   rayon;
      int   u;
      float angle;
      int   x, y;
      int   new_x, new_y;

      for (i = 0; i < ext_image -> extrNb; i++)
	{
	  position = ext_image -> extr[i].pos;
	  rayon = (int) (ext_image -> extr[i].mod*ratio);

	  position = position - x_min - y_min * ext_image -> lx;
	  x = position % ext_image -> lx;
	  y = position / ext_image -> lx;
	  for (u = 0; u < (7*rayon); u++)
	    {
	      angle = u*2*M_PI/(7*rayon);
	      new_x = x + (int) floor (rayon * cos (angle) + 0.5);
	      new_y = y + (int) floor (rayon * sin (angle) + 0.5);

	      if (new_x < image -> lx
		  && new_y < image -> ly
		  && new_x >= 0
		  && new_y >= 0)
		file_val [new_x + new_y*image-> lx] = 0;
	    }
	}
    }

  /* Write data */
  out_file = fopen (file_name, "w");

  fprintf (out_file,
	   "P5\n"
	   "# CREATOR: XV Version 3.00  Rev: 3/30/93\n");
  fprintf (out_file,
	   "%d %d\n", image -> lx, image -> ly);
  fprintf (out_file,
	   "255\n");
  fwrite(file_val, sizeof(unsigned char), image -> lx*image -> ly, out_file);

  fclose (out_file);
  free (file_val);

  return TCL_OK;
}


/*
 */
void
_array_2_ps_ (char *file_name,
	      int  isShowpage,
	      int  isAdd,
	      int  xPos,
	      int  yPos,
	      int  width,
	      int  height,
	      real scale,
	      int  lx,
	      int  ly,
	      unsigned char *file_val,
	      int  drawBoxFlag)
{
  FILE *out_file;
  int  i;
  int  j;


  out_file = fopen(file_name, "r");
  if (isAdd && out_file) {
    fclose(out_file);
    out_file = fopen (file_name, "a");
  } else {
    if (out_file) {
      fclose(out_file);
    }
    out_file = fopen (file_name, "w");

    fprintf(out_file ,"%%!PS-Adobe-2.0 EPSF-2.0\n");
    fprintf(out_file ,"%%%%Title: %s\n", file_name);
    fprintf(out_file ,"%%%%Creator: xsmurf  -  by Nicolas Decoster\n");
    fprintf(out_file ,"%%%%BoundingBox: 0 0 %d %d\n", width, height);
    fprintf(out_file ,"%%%%Pages: 1\n");
    fprintf(out_file ,"%%%%DocumentFonts:\n");
    fprintf(out_file ,"%%%%EndComments\n");
    fprintf(out_file ,"%%%%EndProlog\n");
    fprintf(out_file ,"\n");
    fprintf(out_file ,"%%%%Page: 1 1\n");
    fprintf(out_file ,"\n");
  }

  fprintf(out_file ,"%% remember original state\n");
  fprintf(out_file ,"/origstate save def\n");
  fprintf(out_file ,"\n");
  fprintf(out_file ,"%% build a temporary dictionary\n");
  fprintf(out_file ,"20 dict begin\n");
  fprintf(out_file ,"\n");
  fprintf(out_file ,"%% define string to hold a scanline's worth of data\n");
  fprintf(out_file ,"/pix %d string def\n", lx);
  fprintf(out_file ,"\n");
  fprintf(out_file ,"%% define space for color conversions\n");
  fprintf(out_file ,"/grays %d string def  %% space for gray scale line\n", lx);
  fprintf(out_file ,"/npixls 0 def\n");
  fprintf(out_file ,"/rgbindx 0 def\n");
  fprintf(out_file ,"\n");
  fprintf(out_file ,"%% lower left corner\n");
  fprintf(out_file ,"%d %d translate\n", xPos, yPos);
  fprintf(out_file ,"\n");
  fprintf(out_file ,"%% size of image (on paper, in 1/72inch coords)\n");
  fprintf(out_file ,"%g %g scale\n", lx*scale, ly*scale);
  fprintf(out_file ,"\n");
  fprintf(out_file ,"%d %d 8		     %% dimensions of data\n", lx, ly);
  fprintf(out_file ,"[%d 0 0 -%d 0 %d]	     %% mapping matrix\n", lx, ly, ly);
  fprintf(out_file ,"{currentfile pix readhexstring pop}\n");
  fprintf(out_file ,"image\n");
  fprintf(out_file ,"\n");

  for (j = 0; j < ly; j++) {
    for (i = 0; i < lx; i++) {
      fprintf(out_file ,"%.2x", file_val[i+j*lx]);
    }
    fprintf(out_file ,"\n");
  }

  fprintf(out_file ,"\n");
  fprintf(out_file ,"%% stop using temporary dictionary\n");
  fprintf(out_file ,"end\n");
  fprintf(out_file ,"\n");
  fprintf(out_file ,"%% restore original state\n");
  fprintf(out_file ,"origstate restore\n");
  fprintf(out_file ,"\n");

  if (drawBoxFlag) {
    fprintf(out_file ,"newpath %d %d ", xPos, yPos);
    fprintf(out_file ,"moveto %d %d ", xPos+(int)(lx*scale), yPos);
    fprintf(out_file ,"lineto %d %d ", xPos+(int)(lx*scale), yPos+(int)(ly*scale));
    fprintf(out_file ,"lineto %d %d ", xPos, (int)(yPos+ly*scale));
    fprintf(out_file ,"lineto closepath gsave stroke grestore\n");
  }

  if (isShowpage) {
    fprintf(out_file ,"\n");
    fprintf(out_file ,"showpage\n");
  }

  fclose (out_file);
}

/*
  command name in xsmurf : i2eps
 */
int
i_to_ps_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Is",
    "-inv", "",
    "-pos", "dd",
    "-size", "dd",
    "-add", "",
    "-showpage", "",
    "-scale", "f",
    "-nobox", "",
    NULL
  };

  char * help_msg =
  {
    (" Save an image in encapsulated postscript. The colors are from black "
     "(little values) to white (big values).\n"
     "\n"
     "Parameters :\n"
     "  image  - image to save.\n"
     "  string - file name.\n"
     "\n"
     "Options :\n"
     "  -inv  : color from white to black.\n"
     "  -pos  : indicates the position.\n"
     "    2 integers - coordinates of the position. (0,0) is the lower left "
     "corner.\n"
     "  -size : indicates the size of the result (in image pixel). For example "
     "the size of A4 format is nearly 576x828 pixels.\n"
     "    2 integers - width and height.\n"
     "  -add  : appends to file. If this option is set, option -size has no "
     "effect.\n"
     "  -showpage : add the string \"showpage\" at the end of the file.\n"
     "  -scale : scale the image.\n"
     "     real - value of the scale.\n"
     "  -nobox : no box around the image.")
  };

  /* Command's parameters */
  Image    *image;
  char     *file_name;

  /* Options's presence */
  int isInv;
  int isPos;
  int isSize;
  int isAdd;
  int isShowpage;
  int isScale;
  int isNobox;

  /* Options's parameters */
  int xPos = 0;
  int yPos = 0;
  int width;
  int height;
  real scale = 1.0;

  /* Other variables */
  real val_min, val_max;
  int  i;

  unsigned char *file_val;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &file_name) == TCL_ERROR)
    return TCL_ERROR;

  isInv = arg_present(1);
  isPos = arg_present(2);
  if (isPos) {
    if (arg_get(2, &xPos, &yPos) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isSize = arg_present(3);
  if (isSize) {
    if (arg_get(3, &width, &height) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isAdd = arg_present(4);
  isShowpage = arg_present(5);
  isScale = arg_present(6);
  if (isScale) {
    if (arg_get(6, &scale) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isNobox = arg_present(7);

  /* Parameters validity and initialisation */
  if (!isSize) {
    width =  (int)(scale*image->lx)+xPos;
    height = (int)(scale*image->ly)+yPos;
  }
  xPos++;
  yPos++;

  /* Treatement */

  /* First we fill the file data with the image. */
  file_val = (unsigned char *) malloc (sizeof (unsigned char)*image->lx*image->ly);

  im_get_extrema (image, &val_max, &val_min);

  if (isInv) {
    for (i = 0; i < image -> lx*image -> ly; i++)
      file_val[i] = (unsigned char) ((image -> data[i] - val_min)
					   *255.0/(val_max - val_min));
  } else {
    for (i = 0; i < image -> lx*image -> ly; i++)
      file_val[i] = 255 - (unsigned char) ((image -> data[i] - val_min)
					   *255.0/(val_max - val_min));
  }

  /* Write data in the file */
  _array_2_ps_ (file_name,
		isShowpage, isAdd,
		xPos, yPos,
		width, height,
		scale,
		image->lx, image->ly,
		file_val,
		!isNobox);
  free (file_val);

  return TCL_OK;
}


/*
  command name in xsmurf : e2eps
 */
int
e_to_ps_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es",
    "-inv", "",
    "-pos", "dd",
    "-size", "dd",
    "-add", "",
    "-showpage", "",
    "-scale", "f",
    "-vc", "[f]",
    "-bin", "",
    "-nobox", "",
    NULL
  };

  char * help_msg =
  {
    (" Save an ext image in encapsulated postscript. The colors are from black "
     "(little values) to white (big values).\n"
     "\n"
     "Parameters :\n"
     "  image  - image to save.\n"
     "  string - file name.\n"
     "\n"
     "Options :\n"
     "  -inv  : color from white to black.\n"
     "  -pos  : indicates the position.\n"
     "    2 integers - coordinates of the position. (0,0) is the lower left "
     "corner.\n"
     "  -size : indicates the size of the result (in image pixel). For example "
     "the size of A4 format is nearly 576x828 pixels.\n"
     "    2 integers - width and height.\n"
     "  -add  : appends to file. If this option is set, option -size has no "
     "effect.\n"
     "  -showpage : add the string \"showpage\" at the end of the file.\n"
     "  -scale : scale the image.\n"
     "     real - value of the scale.\n"
     "  -vc : add a gradient for each maxima that is on a vertical chain.\n"
     "     real - ratio for the size of the gradient.\n"
     "  -bin : binary codage.\n"
     "  -nobox : no box around the image.")
  };

  /* Command's parameters */
  ExtImage    *image;
  char     *file_name;

  /* Options's presence */
  int isInv;
  int isPos;
  int isSize;
  int isAdd;
  int isShowpage;
  int isScale;
  int isVc;
  int isBin;
  int isNobox;

  /* Options's parameters */
  int xPos = 0;
  int yPos = 0;
  int width;
  int height;
  real scale = 1.0;
  real grRatio = 1.0;

  /* Other variables */
  real val_min, val_max;
  int  i;
  unsigned char *file_val;
  FILE *out_file;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &file_name) == TCL_ERROR)
    return TCL_ERROR;

  isInv = arg_present(1);
  isPos = arg_present(2);
  if (isPos) {
    if (arg_get(2, &xPos, &yPos) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isSize = arg_present(3);
  if (isSize) {
    if (arg_get(3, &width, &height) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isAdd = arg_present(4);
  isShowpage = arg_present(5);
  isScale = arg_present(6);
  if (isScale) {
    if (arg_get(6, &scale) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isVc = arg_present(7);
  if (isVc) {
    if (arg_get(7, &grRatio) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isBin = arg_present(8);
  isNobox = arg_present(9);

  /* Parameters validity and initialisation */
  if (!isSize) {
    width =  (int)(image->lx*scale)+xPos+1;
    height = (int)(image->ly*scale)+yPos+1;
  }
  xPos++;
  yPos++;

  /* Treatement */

  /* First we fill the array with the image. */
  file_val = (unsigned char *) malloc (sizeof (unsigned char)*image->lx*image->ly);

  ExtImaMinMax(image, &val_max, &val_min);

  if (isInv) {
    for (i = 0; i < image -> lx*image -> ly; i++)
      file_val[i] = 255;
    for (i = 0; i < image->extrNb; i++)
      if (isBin) {
	file_val[image->extr[i].pos] = 0;
      } else {
	file_val[image->extr[i].pos] = (unsigned char) ((image->extr[i].mod - val_min)
							*255.0/(val_max - val_min));
      }
  } else {
    for (i = 0; i < image -> lx*image -> ly; i++)
      file_val[i] = 0;
    for (i = 0; i < image->extrNb; i++)
      if (isBin) {
	file_val[image->extr[i].pos] = 255;
      } else {
	file_val[image->extr[i].pos] = 255 -(unsigned char) ((image->extr[i].mod - val_min)
							     *255.0/(val_max - val_min));
      }
  }

  /* Write data in the file*/
  _array_2_ps_ (file_name,
		isShowpage, isAdd,
		xPos, yPos,
		width, height,
		scale,
		image->lx, image->ly,
		file_val,
		!isNobox);
  free (file_val);

  /* Add the gradient vector if requested. */
  if (isVc) {
    int x1, x2;
    int y1, y2;
    Extremum *ext;

    out_file = fopen (file_name, "a");
    for (i = 0; i < image->extrNb; i++) {
      ext = &image->extr[i];
      if (ext->up || ext->down) {
	x1 = xPos + (ext->pos%image->lx)*scale;
	y1 = yPos + (image->lx - (ext->pos/image->lx))*scale;
	x2 = x1 + grRatio*ext->mod*cos(ext->arg)*scale;
	y2 = y1 - grRatio*ext->mod*sin(ext->arg)*scale;

	fprintf(out_file ,"0.3 setlinewidth\n");
	fprintf(out_file ,"newpath %d %d ", x1, y1);
	fprintf(out_file ,"moveto %d %d ", x2, y2);
	fprintf(out_file ,"lineto closepath gsave stroke grestore\n");
      }
    }
    fclose (out_file);
  }
  return TCL_OK;
}


/*
 */
int
rectangle_to_ps_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "sffff",
    "-size", "dd",
    "-add", "",
    "-showpage", "",
    "-scale", "f",
    NULL
  };

  char * help_msg =
  {
    (" Draw a rectangle in a postscript file.\n"
     "\n"
     "Parameters :\n"
     "  string     - file name.\n"
     "  4 integers - coordinates of the rectangle.\n"
     "\n"
     "Options :\n"
     "  -size : indicates the size of the result (in image pixel). For example "
     "the size of A4 format is nearly 576x828 pixels.\n"
     "    2 integers - width and height.\n"
     "  -add  : appends to file. If this option is set, option -size has no "
     "effect.\n"
     "  -showpage : add the string \"showpage\" at the end of the file.\n"
     "  -scale : scale the rectangle.\n"
     "     real - value of the scale.")
  };

  /* Command's parameters */
  char *file_name;
  real x1, y1, x2, y2; 

  /* Options's presence */
  int isSize;
  int isAdd;
  int isShowpage;
  int isScale;

  /* Options's parameters */
  int width;
  int height;
  real scale = 1.0;

  /* Other variables */
  FILE *out_file;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &file_name, &x1, &y1, &x2, &y2) == TCL_ERROR)
    return TCL_ERROR;

  isSize = arg_present(1);
  if (isSize) {
    if (arg_get(1, &width, &height) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isAdd = arg_present(2);
  isShowpage = arg_present(3);
  isScale = arg_present(4);
  if (isScale) {
    if (arg_get(4, &scale) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */
  x1 *= scale;
  y1 *= scale;
  x2 *= scale;
  y2 *= scale;

  /* Treatement */

  out_file = fopen(file_name, "r");
  if (isAdd && out_file) {
    fclose(out_file);
    out_file = fopen (file_name, "a");
  } else {
    if (out_file) {
      fclose(out_file);
    }
    out_file = fopen (file_name, "w");

    fprintf(out_file ,"%%!PS-Adobe-2.0 EPSF-2.0\n");
    fprintf(out_file ,"%%%%Title: %s\n", file_name);
    fprintf(out_file ,"%%%%Creator: xsmurf  -  by Nicolas Decoster\n");
    fprintf(out_file ,"%%%%BoundingBox: 0 0 %d %d\n", width, height);
    fprintf(out_file ,"%%%%Pages: 1\n");
    fprintf(out_file ,"%%%%DocumentFonts:\n");
    fprintf(out_file ,"%%%%EndComments\n");
    fprintf(out_file ,"%%%%EndProlog\n");
    fprintf(out_file ,"\n");
    fprintf(out_file ,"%%%%Page: 1 1\n");
    fprintf(out_file ,"\n");
  }

  fprintf(out_file ,"newpath %d %d ", (int)x1, (int)y1);
  fprintf(out_file ,"moveto %d %d ", (int)x2, (int)y1);
  fprintf(out_file ,"lineto %d %d ", (int)x2, (int)y2);
  fprintf(out_file ,"lineto %d %d ", (int)x1, (int)y2);
  fprintf(out_file ,"lineto closepath gsave stroke grestore\n");

  if (isShowpage) {
    fprintf(out_file ,"\n");
    fprintf(out_file ,"showpage\n");
  }

  fclose (out_file);

  return TCL_OK;
}


/*
 */
int
ntime_TclCmd_ (ClientData clientData,
	       Tcl_Interp *interp,
	       int        argc,
	       char       **argv)
{ 
  /* Command line definition */
  /*  char * options[] =
  {
    "",
    NULL
  };

  char * help_msg =
  {
    ("Returns the time since 00:00:00 GMT, January 1, 1970, measured\n"
     "in seconds.")
  };
  */
  /* Other variables */
  time_t t;

  /* Treatement */
  t = time (0);
  sprintf (interp -> result, "%d", (int)t);

  return TCL_OK;
}

