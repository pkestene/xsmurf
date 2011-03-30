#include <stdlib.h>
#include "math.h"
#include "Image.h"
#include "Convert.h"
#include "../image/image.h"
#include "../wt2d/wt2d.h"
#include "../interpreter/arguments.h"
#include "../interpreter/hash_tables.h"
#include "Dictionary.h"
#include <string.h>

/* saaaale */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
ExtImage *ExtDicGet(char *name);

static void
_line2_(unsigned long *data,
	int  lx,
	int  val,
	int  pos,
	real size,
	real arg)
{
  int  i, new_pos;
  real x, y;
  real xstep, ystep;

  xstep = cos(arg);
  ystep = sin(arg);

  for(x = 0, y = 0, i = 0; i < (int)(size); x += xstep, y += ystep, i++)
    {
      new_pos = pos + (int)(x) + (int)(y)*lx;
      if ((new_pos > 0) && (new_pos < (lx*lx)))
	data[new_pos] = val;
    }
}

static void
_my_line_(unsigned long *data,
       int  lx,
       int  val,
       int  pos,
       int  x1,
       int  y1,
       int  x2,
       int  y2)
{
  int  x, y;
  real xstep, ystep;

  if (abs(x1-x2) > abs(y1-y2))
    {
      ystep = (y2 - y1)/abs(x2-x1);
      xstep = x2 > x1 ? 1 : -1;
    }
  else
    {
      xstep = (x2 - x1)/abs(y2-y1);
      ystep = y2 > y1 ? 1 : -1;
    }
  for (x = x1, y = y1; x < x2; x = (int)(x + xstep), y = (int)(y + ystep))
    data[pos + x + y*lx] = val;
}
/*----------------------------------------------------------------------------
  Conversion d'un objet de type Image en ViewImage
  --------------------------------------------------------------------------*/

/*enum {
  WHITE,
  BLACK,
  RED,
  BLUE,
  YELLOW,
  GREEN,
  GREY1,
  GREY2,
  GREY3
};
*/

#define NO_COLOR -1

static char * _color_strings_lst_[] =
{
  "white",
  "black",
  "red",
  "blue",
  "yellow",
  "green",
  "grey1",
  "grey2",
  "grey3",
  NULL
};

static int
_get_color_ (char * color_str)
{
  int i = 0;

  while (_color_strings_lst_[i])
    {
      if (!strcmp (_color_strings_lst_[i], color_str))
	return i;
      i++;
    }
	  
  return NO_COLOR; /* wrong disp_type_str */
}

/*----------------------------------------------------------------------------
  ViewConvImageCmd_
  
  Convertit une Image en ViewImage
  --------------------------------------------------------------------------*/
int
ViewConvImageCmd_(ClientData clientData,
		  Tcl_Interp * interp,
		  int argc,
		  char ** argv)
{
  char * options[] = { "Is" ,
			 "-zoom", "d",
			 "-arg", "",
			 "-ext", "E",
			 "-pos", "[dd]",
			 "-niv", "ff",
			 "-2lvl", "f",
			 "-loc_max", "Ef",
			 "-loc_max2", "f",
			 "-cross", "ddd",
			 "-lext", "s",
			 "-exttag", "",
			 NULL };
  char * help_msg =
    {("Convert tha data of an image into a widget that can be displayed.\n"
      " -zoom set the zoom value.\n"
      " -arg  set a specific map of color for the argument type images.\n"
      " -ext  include an ext image.\n"
      " -pos  color the pixel given by the two integer.\n"
      " -niv  draw the courbe de niveau (hehe) given by the first real\n"
      "       with an jhgjkhdgfhgfkbsbdlblblb\n"
      "\n This help must be updated...")};

  Image       * imagePtr;
  ExtImage    * extImagePtr;
  ViewImage   * viewImagePtr;
  real       min, max, val;
  real        lvl;
  char        * viewImageName;
  int         i, j, isArg = 0, isExt = 0, isPos = 0, isNiv = 0, is2lvl = 0;
  int         isCross;
  int         is_loc;
  int         is_loc2;
  unsigned long bgPixel, extPixel, posPixel;
  int         zoom = 1;
  int         xpos=-1, ypos=-1;
  int         u, v, position1, position2, position;
  real  niveau, eps;
  float       factor;
  float       factor2;
  int crossI, crossJ;
  int crossSize;
  char *e_lst_str;
  int isLext;
  int    e_lst_size, code;
  char   **e_elt;
  char *extName;
  char *colorName;
  int color;
  int isExttag;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &imagePtr, &viewImageName)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &zoom)==TCL_ERROR)
    return TCL_ERROR;

  isArg = arg_present(2);

  isExt = arg_present (3);
  if (isExt)
    if (arg_get(3, &extImagePtr)==TCL_ERROR)
	return TCL_ERROR;

  isPos = arg_present (4);
  if (isPos)
    {
      if (arg_get(4, &xpos, &ypos)==TCL_ERROR)
	return TCL_ERROR;
    }

  isNiv = arg_present (5);
  if (isNiv)
    if (arg_get(5, &niveau, &eps)==TCL_ERROR)
	return TCL_ERROR;

  is2lvl = arg_present (6);
  if (is2lvl)
    if (arg_get(6, &lvl)==TCL_ERROR)
	return TCL_ERROR;

  is_loc = arg_present (7);
  if (is_loc)
    if (arg_get(7, &extImagePtr, &factor)==TCL_ERROR)
	return TCL_ERROR;

  is_loc2 = arg_present (8);
  if (is_loc)
    if (arg_get(8, &factor2)==TCL_ERROR)
	return TCL_ERROR;

  isCross = arg_present (9);
  if (isCross)
    if (arg_get(9, &crossI, &crossJ, &crossSize)==TCL_ERROR)
	return TCL_ERROR;

  isLext = arg_present (10);
  if (isLext)
    if (arg_get(10, &e_lst_str) == TCL_ERROR)
	return TCL_ERROR;

  isExttag = arg_present (11);

  if(isArg){
    min = 0;
    max = M_PI;
  }
  else
    im_get_extrema (imagePtr, &min, &max); /* le min et le max de l'image */

  if (imagePtr->type == FOURIER)
    viewImagePtr = ViewImaCreate_(imagePtr->lx*zoom, imagePtr->ly*zoom*2);
  else if (imagePtr->type == FFTW_R2C)
    viewImagePtr = ViewImaCreate_(imagePtr->lx*zoom, 2*(imagePtr->ly/2+1)*zoom);    
  else
    viewImagePtr = ViewImaCreate_(imagePtr->lx*zoom, imagePtr->ly*zoom);

  if (!viewImagePtr)
    return GenErrorMemoryAlloc(interp);

  if (!ViewColConvertToPixelInit_(min, max))
    {
      bgPixel = get_color_intensity_ (BLACK);
      for (i = 0;i<imagePtr->size;i++)
	{
	  position = zoom*((i/imagePtr->lx) * zoom * imagePtr->lx +
			   (i % imagePtr->lx));
	  for (u = 0; u < zoom; u++)
	    for (v = 0; v < zoom; v++)
	      viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = bgPixel;
	}
    }
  else
    /*    if (isLog)
      {
	for (i = 0;i<imagePtr->size;i++){
	  position = zoom*((i/imagePtr->lx) * zoom * imagePtr->lx + (i % 
						imagePtr->lx));
	  for (u = 0; u < zoom; u++)
	    for (v = 0; v < zoom; v++)
	      viewImagePtr->data[position+v+zoom*imagePtr->lx*u] =
		                 ViewColLogConvertToPixel_(imagePtr->data[i]);
	}
      }
    else*/
      {
	if (imagePtr->type == FOURIER) {
	  for (i = 0;i<imagePtr->lx*imagePtr->lx;i++)
	    {
	      position1 = zoom*((int)(i/imagePtr->lx) * zoom * imagePtr->lx +
				(i % imagePtr->lx));
	      position2 = zoom*(imagePtr->lx*imagePtr->lx*zoom+
				(int)(i/imagePtr->lx) * zoom * imagePtr->lx +
				(i % imagePtr->lx));
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  {
		    if(isArg){
		      return TCL_OK; /* achtung!!!!! */
		    }
		    else
		      {
			viewImagePtr->data[position1+v+zoom*imagePtr->lx*u] =
			  ViewColLinConvertToPixel_(imagePtr->data[2*i+1]);
			viewImagePtr->data[position2+v+zoom*imagePtr->lx*u] =
			  ViewColLinConvertToPixel_(imagePtr->data[2*i+2]);
		      }
		  }
	    }
	} else if (imagePtr->type == FFTW_R2C) {
	  for (i = 0;i<imagePtr->size;i++){
	    position = zoom*((i/imagePtr->lx) * zoom * imagePtr->lx +
			     (i % imagePtr->lx));
	    for (u = 0; u < zoom; u++)
	      for (v = 0; v < zoom; v++){
		if(isArg){
		  if (imagePtr->data[i] == NO_ARG)
		    val = 0;
		  else if (imagePtr->data[i]>(M_PI/2))
		    val = 3*M_PI/2 - imagePtr->data[i];
		  else if (imagePtr->data[i]<(-M_PI/2))
		    val = -M_PI/2 - imagePtr->data[i];
		  else
		    val = M_PI/2 + imagePtr->data[i];
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] =
		    ViewColLinConvertToPixel_(val);
		}
		else
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] =
		    ViewColLinConvertToPixel_(imagePtr->data[i]);
	    }
	  }
	} else {
	  for (i = 0;i<imagePtr->size;i++){
	    position = zoom*((i/imagePtr->lx) * zoom * imagePtr->lx +
			     (i % imagePtr->lx));
	    for (u = 0; u < zoom; u++)
	      for (v = 0; v < zoom; v++){
		if(isArg){
		  if (imagePtr->data[i] == NO_ARG)
		    val = 0;
		  else if (imagePtr->data[i]>(M_PI/2))
		    val = 3*M_PI/2 - imagePtr->data[i];
		  else if (imagePtr->data[i]<(-M_PI/2))
		    val = -M_PI/2 - imagePtr->data[i];
		  else
		    val = M_PI/2 + imagePtr->data[i];
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] =
		    ViewColLinConvertToPixel_(val);
		}
		else
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] =
		    ViewColLinConvertToPixel_(imagePtr->data[i]);
	    }
	  }
	}
      }
  if (isNiv)
    {
      int nivPixel;

      real epsilon = (max-min) * eps;
      nivPixel  =  get_color_intensity_ (GREEN);
      for (i = 0;i<imagePtr->size;i++)
	{
	  if(fabs(imagePtr->data[i]-niveau) < epsilon)
	    {
	      position = zoom*((i/imagePtr->lx) * zoom * imagePtr->lx +
			       (i % imagePtr->lx));
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++){
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = nivPixel;
		}
	    }
	}
    }
  if (is2lvl)
    {
      int nivPixel;

      nivPixel  =  get_color_intensity_ (GREEN);
      for (i = 0;i<imagePtr->size;i++)
	{
	  if(imagePtr->data[i] < lvl)
	    {
	      position = zoom*((i/imagePtr->lx) * zoom * imagePtr->lx +
			       (i % imagePtr->lx));
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++){
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = nivPixel;
		}
	    }
	}
    }
  if (isExt)
    {
      /*extPixel  =  get_color_intensity_ (BLACK);*/
      extPixel  =  get_color_intensity_ (RED);
      for (i=0;i<extImagePtr->extrNb;i++)
	{
	  position = zoom*((extImagePtr->extr[i].pos/extImagePtr->lx) *
			   zoom * extImagePtr->lx +(extImagePtr->extr[i].pos % 
						    extImagePtr->lx));
	  for (u=0;u<zoom;u++)
	    for (v=0;v<zoom;v++)
	      viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = extPixel;
	}
    }
  if (isExttag)
    {
      Extremum *ext;
      Line *l;
      real mmax;
      real mmin;
      real norm;

      ExtImaMinMax(extImagePtr,&mmin,&mmax);
      norm = 1.0/mmax;
      mmin=0.0;

      foreach (l, extImagePtr->line_lst) {
	foreach (ext, l->gr_lst) {
	  if (!ext->up && !ext->down) {
	    continue;
	  }
	  if (ext->next) {
	    extPixel  =  get_color_intensity_ (BLUE);
	  } else {
	    extPixel  =  get_color_intensity_ (RED);
	  }
	  position = zoom*((ext->pos/extImagePtr->lx) *
			   zoom * extImagePtr->lx +(ext->pos % 
						    extImagePtr->lx));
	  for (u=0;u<zoom;u++) {
	    for (v=0;v<zoom;v++) {
	      viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = extPixel;
	    }
	  }
	  _line2_(viewImagePtr->data, imagePtr->lx*zoom, extPixel,
		  position+(int)(zoom/2)*(imagePtr->lx*zoom+1),
		  3*zoom+30*zoom*(ext->mod)*norm, ext->arg);

	}
      }
    }
  if (is_loc)
    {
      extPixel  =  get_color_intensity_ (BLACK);
      for (i=0;i<extImagePtr->extrNb;i++)
	{
	  int rayon, new_pos;
	  int x_add, y_add;
	  float angle;

	  position = zoom*((extImagePtr->extr[i].pos/extImagePtr->lx) *
			   zoom * extImagePtr->lx +(extImagePtr->extr[i].pos % 
						    extImagePtr->lx));
	  rayon = (int) (extImagePtr->extr[i].mod*zoom*factor);

	  for (u = 0; u < (7*rayon); u++)
	    {
	      angle = u*2*M_PI/(7*rayon);
	      x_add = zoom/2 + rayon * cos (angle);
	      y_add = zoom/2 + rayon * sin (angle);
					    
	      new_pos = position + x_add + y_add*imagePtr->lx*zoom;

	      if (new_pos >= 0
		  && new_pos < (viewImagePtr -> lx*viewImagePtr -> ly))
		viewImagePtr->data[new_pos] = extPixel;
	    }
	}
    }
  if (is_loc2)
    {
      extPixel  =  get_color_intensity_ (RED);
      for (i=0;i<extImagePtr->extrNb;i++)
	{
	  int rayon, new_pos;
	  int x_add, y_add;
	  float angle;

	  position = zoom*((extImagePtr->extr[i].pos/extImagePtr->lx) *
			   zoom * extImagePtr->lx +(extImagePtr->extr[i].pos % 
						    extImagePtr->lx));
	  rayon = (int) (extImagePtr->extr[i].arg*zoom*factor2);

	  for (u = 0; u < (7*rayon); u++)
	    {
	      angle = u*2*M_PI/(7*rayon);
	      x_add = zoom/2 + rayon * cos (angle);
	      y_add = zoom/2 + rayon * sin (angle);
					    
	      new_pos = position + x_add + y_add*imagePtr->lx*zoom;

	      if (new_pos >= 0
		  && new_pos < (viewImagePtr -> lx*viewImagePtr -> ly))
		viewImagePtr->data[new_pos] = extPixel;
	    }
	}
    }
  if (isPos)
    {
      position = zoom*(ypos*imagePtr->lx*zoom + xpos);
      if(position >= 0)
	{
	  if (imagePtr->data[xpos+imagePtr->lx*ypos] >
	      ((min+(max-min)*0.9))      )
	      posPixel  =  get_color_intensity_ (RED);
	    else
	      posPixel  =  get_color_intensity_ (RED);
	  for (u=0;u<zoom;u++)
	    for (v=0;v<zoom;v++)
	      viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = posPixel;
	}
    }
  if (isCross)
    {
      position = zoom*((int)((crossI+0.5)*zoom)*imagePtr->lx + crossJ + zoom/2);
      posPixel  =  get_color_intensity_ (RED);
      _line2_ (viewImagePtr->data,
	       viewImagePtr->lx,
	       posPixel,
	       position,
	       crossSize*zoom/2, 0);
      _line2_ (viewImagePtr->data,
	      viewImagePtr->lx,
	      posPixel,
	      position,
	      crossSize*zoom/2, M_PI/2);
      _line2_ (viewImagePtr->data,
	       viewImagePtr->lx,
	       posPixel,
	       position,
	       crossSize*zoom/2, M_PI);
      _line2_ (viewImagePtr->data,
	      viewImagePtr->lx,
	      posPixel,
	      position,
	      crossSize*zoom/2, 3*M_PI/2);
    }
  if (isLext) {
    code = Tcl_SplitList(interp, e_lst_str, &e_lst_size, (CONST char ***)&e_elt);
    if (code == TCL_ERROR)
      return TCL_ERROR;
    for (j = 0; j < e_lst_size; j++) {
      extName = (char *) malloc (sizeof(char)*(strlen(e_elt[j])+1));
      colorName = (char *) malloc (sizeof(char)*(strlen(e_elt[j])+1));
      /*      strcpy (colorName, "black\0");*/
      sscanf( e_elt[j], "%s %s", extName, colorName);
      extImagePtr = ExtDicGet(extName);
      if (!extImagePtr) {
	sprintf (interp->result, "Couldn't find extrema image `%s'", extName);
	return TCL_ERROR;
      }
      color = _get_color_ (colorName);
      if (color == NO_COLOR) {
	color = BLACK;
      }
      extPixel = get_color_intensity_ (color);
      for (i=0;i<extImagePtr->extrNb;i++)
	{
	  position = zoom*((extImagePtr->extr[i].pos/extImagePtr->lx) *
			   zoom * extImagePtr->lx +(extImagePtr->extr[i].pos % 
						    extImagePtr->lx));
	  for (u=0;u<zoom;u++)
	    for (v=0;v<zoom;v++)
	      viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = extPixel;
	}
      free (extName);
      free (colorName);
    }
  }

  ViewDicStore_(viewImageName, viewImagePtr);
  
  Tcl_AppendResult(interp, viewImageName, NULL);
  return TCL_OK;
}


/*----------------------------------------------------------------------------
  ViewConvColormapCmd_
  
  Visualisation de la colormap
  --------------------------------------------------------------------------*/
int
ViewConvColormapCmd_(ClientData clientData,
		     Tcl_Interp * interp,
		     int argc,
		     char ** argv)
{
  char * options[] = { "s" ,
			 NULL };
  
  ViewImage   * viewImagePtr;
  char        * viewImageName;
  int         i, j;
  int         Pixel;
  int         u, v;
  int         size = 20;
  int         colorNb;
  int         lx;

  if (arg_init(interp, argc, argv, options, NULL))
    return TCL_OK;

  if (arg_get(0, &viewImageName)==TCL_ERROR)
    return TCL_ERROR;

  colorNb = get_number_of_colors_ () + get_number_of_grey_scales_();
  lx = (int)sqrt(colorNb) +1;
  /* ("%d %d %d %d\n",lx, colorNb, get_number_of_colors_ (), get_number_of_grey_scales_());*/

  viewImagePtr = ViewImaCreate_(lx*size, lx*size);
  if (!viewImagePtr)
    return GenErrorMemoryAlloc(interp);

  for(i=0;i<lx;i++)
      for(j=0;(j<lx)&&((i*lx+j)<colorNb);j++)
	{
	  if((i*lx+j)<get_number_of_colors_ ())
	    Pixel = get_color_intensity_ (i*lx+j);
	  else
	    Pixel = get_grey_intensity_ (i*lx+j-get_number_of_colors_ ());

	  for (u = 0; u < size; u++)
	    for (v = 0; v < size; v++)
	      viewImagePtr->data[(i*size+u)*(lx*size)+j*size+v] = Pixel;
	}

  ViewDicStore_(viewImageName, viewImagePtr);
  
  Tcl_AppendResult(interp, viewImageName, NULL);
  return TCL_OK;
}

/*----------------------------------------------------------------------------
  --------------------------------------------------------------------------*/
int
ViewConvPlotCmd_(ClientData clientData,
		     Tcl_Interp * interp,
		     int argc,
		     char ** argv)
{
  char * options[] = { "VIdd" ,
			 "-zoom", "d",
			 "-color", "d",
			 NULL };
  
  Image      * imagePtr;
/*  ExtImage * extImagePtr;*/
  ViewImage  * viewImagePtr;
  real       min, max;
  int        isColor = 0;
  int        posPixel;
  int        zoom = 1;
  int        xpos=-1, ypos=-1;
  int lx;
  int         u, v, position;
  int color;

  if (arg_init(interp, argc, argv, options, NULL))
    return TCL_OK;

  if (arg_get(0, &viewImagePtr, &imagePtr, &xpos, &ypos)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &zoom)==TCL_ERROR)
    return TCL_ERROR;

  isColor = arg_present(2);
  if (arg_get(2, &color)==TCL_ERROR)
    return TCL_ERROR;

  lx = imagePtr->lx;

  position = zoom*(ypos*lx*zoom + xpos);
  if(position >= 0)
    {
      if (isColor)
	posPixel  = get_color_intensity_ (color);
      else
	{
	  im_get_extrema (imagePtr, &min, &max);
	  ViewColConvertToPixelInit_(min, max);
	  posPixel = ViewColLinConvertToPixel_(imagePtr->data[ypos*lx+xpos]);
	}
      for (u=0;u<zoom;u++)
	for (v=0;v<zoom;v++)
	  viewImagePtr->data[position+v+zoom*lx*u] = posPixel;
    }  
  sprintf(interp->result,"%d",posPixel);

/*  Tcl_AppendResult(interp, posPixel, NULL);*/
  return TCL_OK;
}
