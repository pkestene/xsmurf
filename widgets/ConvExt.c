#include <stdlib.h>
#include "math.h"
#include "Image.h"
#include "Convert.h"
#include "../wt2d/wt2d.h"
#include "../interpreter/arguments.h"
#include "../interpreter/hash_tables.h"
#include "Dictionary.h"
#include <string.h>

void store_line_in_dictionary      (char *name, Line *line_ptr);


/*----------------------------------------------------------------------------
  Conversion des objets ExtImage en ViewImage
  --------------------------------------------------------------------------*/

void
_line_(unsigned long *data,
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

void
_line2_(unsigned long *data,
	int  lx,
	int  ly,
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
      if ((new_pos > 0) && (new_pos < (lx*ly)))
	data[new_pos] = val;
    }
}

/*----------------------------------------------------------------------------
  ViewConvExtImageCmd_
  
  Convertit une ExtImage en ViewImage

  modifie par P. Kestener (option min_inv, histoimage)
  --------------------------------------------------------------------------*/
int ViewConvExtImageCmd_(ClientData clientData,Tcl_Interp * interp,
			 int argc,char ** argv)
{
  char * options[] = { "Es" ,
		       "-h","dds",
		       "-log","",
		       "-2","",
		       "-zoom","d",
		       "-p","dd",
		       "-next","dd",
		       "-prev","dd",
		       "-min","[f]",
		       "-arg","",
		       "-grad","",
		       "-emin", "E",
		       "-g", "",
		       "-ga", "",
		       "-bg", "I[f]",
		       "-q", "f",
		       "-min_inv", "f",
		       "-histoimage", "f",
			 NULL };
  
  ExtImage  *     imagePtr;
  ExtImage  *     eminPtr;
  ViewImage *     viewImagePtr;
  real            min,max,min2,value,mult=10;
  char *          viewImageName;
  int             i,u,v,position,highlight=0,param=0,next=0,prev=0,mini=0;
  int             is_emin=0, bin=0;
  int             is_grad = 0;
  int             hlX,hlY;
  unsigned long   hlPixel,pixel,linePixel;
  int             zoom = 1;
  int             arg=0;
  unsigned long   (*convertFunction)(real);
  char            *line_name;
  int             is_greater;
  int             is_greater_all;
  float           norm;

  int             is_bg;
  Image      *    bg_image;
  real            fact = 2;

  int             is_q;
  real            q = 1;
  real            zeMin;
  real            zeMax;
  real            tmp;

  int             is_mini_inv;            
  int             is_histo_image;
  real            histoTh;

  int             color_number=WHITE;
  /*int             color_number=BLACK;*/
  
  linePixel = get_color_intensity_(RED);

  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;
  
  if (arg_get(0,&imagePtr,&viewImageName)==TCL_ERROR)
    return TCL_ERROR;

  highlight = arg_present (1);
  if (highlight)
    if (arg_get(1,&hlX,&hlY, &line_name) == TCL_ERROR)
      return TCL_ERROR;
  
  bin = arg_present (3);

  param = arg_present (5);
  if (param)
    if (arg_get(5,&hlX,&hlY) == TCL_ERROR)
      return TCL_ERROR;
  
  next = arg_present (6);
  if (next)
    if (arg_get(6,&hlX,&hlY) == TCL_ERROR)
      return TCL_ERROR;
  
  prev = arg_present (7);
  if (prev)
    if (arg_get(7,&hlX,&hlY) == TCL_ERROR)
      return TCL_ERROR;
  
  mini = arg_present (8);
  if (mini)
    if (arg_get(8,&mult) == TCL_ERROR)
      return TCL_ERROR;
  
  arg = arg_present(9);
  is_grad = arg_present(10);

  is_emin = arg_present (11);
  if (is_emin)
    if (arg_get(11, &eminPtr) == TCL_ERROR)
      return TCL_ERROR;
  
  if (arg_get(4,&zoom)==TCL_ERROR)
    return TCL_ERROR;

  is_greater     = arg_present (12);
  is_greater_all = arg_present (13);

  is_bg = arg_present (14);
  if (is_bg) {
    if (arg_get(14, &bg_image, &fact) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  is_q = arg_present (15);
  if (is_q) {
    if (arg_get(15, &q) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  is_mini_inv = arg_present (16);
  if (is_mini_inv) {
    if (arg_get(16, &mult) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  is_histo_image = arg_present(17);
  if (is_histo_image) {
    if (arg_get(17, &histoTh) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }


  if(!next && !prev) {
    hlX = hlX / zoom;
    hlY = hlY / zoom;
  }

  /* type LOG DIS LIN */  
  if (arg_present(2)) /* log */
    convertFunction = ViewColLogConvertToPixel_;
  else if (bin || is_greater_all || is_greater || param || prev || next || highlight)
    convertFunction = ViewColDisConvertToPixel_;
  else
    convertFunction =  ViewColLinConvertToPixel_;

  /* on cree la viewImage, initialisee a la couleur de fond */
  viewImagePtr = ViewImaCreate_(imagePtr->lx*zoom,imagePtr->ly*zoom); 
  if (!viewImagePtr)
    return GenErrorMemoryAlloc(interp);
  ViewImaClear_(viewImagePtr);

  /* si l'option bg est activee, on utilise l'image donne en argument
     pour determiner la couleur de fond */
  if (is_bg) {
    if (fact < 0) {
      fact = 1;
    }
    im_get_extrema (bg_image, &min, &max);
    ViewColConvertToPixelInit_(min, fact*max);
    for (i = 0; i < bg_image->size; i++){
      position = zoom*((i/bg_image->lx) * zoom * bg_image->lx +
		       (i % bg_image->lx));
      for (u = 0; u < zoom; u++) {
	for (v = 0; v < zoom; v++){
	  viewImagePtr->data[position+v+zoom*bg_image->lx*u] =
	    ViewColLinConvertToPixel_(bg_image->data[i]);
	}
      }
    }
  }


  ExtImaMinMax(imagePtr, &min, &max);
  zeMin = pow(min,q);
  zeMax = pow(max,q);

  if (zeMin > zeMax) {
    tmp = zeMin;
    zeMin = zeMax;
    zeMax = tmp;
  }


  /* on fixe tyranniquement le min a 0 (module >0 de toutes facons) */
  /* choose one of the following */
  /*  norm = max - min;*/
  /*  norm = pow(imagePtr -> scale, 1.0/3)/500.0;*/
  norm = 1.0/max;
  min2 = min;
  min=0.0;
  if (ViewColConvertToPixelInit_(min,max))
    {
      Extremum *chainExt;
      if (mini || is_mini_inv || highlight || param || next || prev || (is_bg && is_histo_image))
	ViewColConvertHighlight_(2.0); /* diminue intensite */
      
      for (i=0;i<imagePtr->extrNb;i++)
	{
	  if(arg)
	    value = imagePtr->extr[i].arg;
	  else
	    value = imagePtr->extr[i].mod;
	  position = zoom*((imagePtr->extr[i].pos/imagePtr->lx) *
			   zoom * imagePtr->lx + (imagePtr->extr[i].pos % 
						 imagePtr->lx));
	  pixel = convertFunction(value);
	  for (u=0;u<zoom;u++)
	    for (v=0;v<zoom;v++)
	      viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = pixel;
	  chainExt = &(imagePtr->extr[i]);
	  if(is_grad && !highlight)
	    {
	      if (imagePtr -> extr[i].up || imagePtr -> extr[i].down)
		linePixel  = get_color_intensity_(RED);
	      else
		linePixel  = get_color_intensity_(YELLOW);

	    _line2_(viewImagePtr->data, imagePtr->lx*zoom,
		    imagePtr->ly*zoom, linePixel, 
 		    position+(int)(zoom/2)*(imagePtr->lx*zoom+1),
		    3*zoom+30*zoom*(chainExt->mod)*norm, chainExt->arg);
	    }
	}
    }
  
  if (highlight)
    {
      Extremum * chainExt;
      int      index;
      Line     *best_line;

      chainExt = ExtMisClosestChain(imagePtr,
				    hlX +imagePtr->lx * hlY,
				    &index,
				    (void *) &best_line);

/*      display_line_stats (imagePtr -> line_ens, chainExt);*/
            if (best_line)
	{
	  hlPixel  = get_color_intensity_ (color_number); //WHITE
      
	  foreach (chainExt, best_line -> ext_lst)
	    {
	      position = zoom*((chainExt->pos/imagePtr->lx) *
			       zoom * imagePtr->lx + (chainExt->pos %
						      imagePtr->lx));
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = hlPixel;
	      if(is_grad)
		_line2_(viewImagePtr->data, imagePtr->lx*zoom,
			imagePtr->ly*zoom, linePixel,
			position+(int)(zoom/2)*(imagePtr->lx*zoom+1),
			3*zoom+30*zoom*(chainExt->mod)*norm, chainExt->arg);
	    }


	  if (best_line)
	    {
	      store_line_in_dictionary (line_name, best_line);
	      sprintf (interp -> result, "%s", line_name);
	    }
	}
	    /*      if (chainExt)
	{
	  hlPixel  = get_color_intensity_ (WHITE);
      
	  firstExt = chainExt;
	  do
	    {
	      position = zoom*((chainExt->pos/imagePtr->lx) *
			       zoom * imagePtr->lx + (chainExt->pos %
						      imagePtr->lx));
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = hlPixel;
	      if(is_grad)
		_line2_(viewImagePtr->data, imagePtr->lx*zoom,
		imagePtr->ly*zoom,  linePixel,
			position+(int)(zoom/2)*(imagePtr->lx*zoom+1),
			3*zoom+30*zoom*(chainExt->mod)*norm, chainExt->arg);
	      chainExt = chainExt->next;
	    }
	  while (chainExt && (chainExt != firstExt));

	  if (best_line)
	    sprintf (interp -> result, "%d", best_line);
	}*/
    }
  else
    {
      if (mini) {
	hlPixel  = get_color_intensity_ (BLUE);      
	for (i=0;i<imagePtr->extrNb;i++)
	  {
	    position = zoom*((imagePtr->extr[i].pos/imagePtr->lx) *
			     zoom * imagePtr->lx + (imagePtr->extr[i].pos % 
						    imagePtr->lx));
	    if(imagePtr->extr[i].mod <= (min2+(max-min2)*mult/100)) {
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = hlPixel;
	    } else {
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = get_color_intensity_ (color_number);
	    }
	  }
      } else if (is_mini_inv) {
	hlPixel  = get_color_intensity_ (RED);      
	for (i=0;i<imagePtr->extrNb;i++)
	  {
	    position = zoom*((imagePtr->extr[i].pos/imagePtr->lx) *
			     zoom * imagePtr->lx + (imagePtr->extr[i].pos % 
						    imagePtr->lx));
	    if(imagePtr->extr[i].mod >= (min2+(max-min2)*mult/100)) {
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = hlPixel;
	    }else {
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = get_color_intensity_ (color_number);
	    }
	  }
      } else if (is_bg && is_histo_image) {
	int extPos;

	hlPixel  = get_color_intensity_ (YELLOW);
	for (i=0;i<imagePtr->extrNb;i++)
	  {
	    position = zoom*((imagePtr->extr[i].pos/imagePtr->lx) *
			     zoom * imagePtr->lx + (imagePtr->extr[i].pos % 
						    imagePtr->lx));
	    extPos = imagePtr->extr[i].pos;
	    if(bg_image->data[extPos] >= histoTh) {
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = hlPixel;
	    }else {
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = get_color_intensity_ (color_number);
	    }
	  }
      }


      if (param || next || prev)
	{
	  Extremum * chainExt;
	  
	  if(imagePtr->chainNb !=0 )
	    {
	      chainExt = ExtMisClosestExt(imagePtr,hlX +imagePtr->lx * hlY);

	      hlPixel  = get_color_intensity_ (color_number);
	  
	      if(next && chainExt->next)
		chainExt = chainExt->next;
	      
	      if(prev && chainExt->prev)
		chainExt = chainExt->prev;

	      position = zoom*((chainExt->pos/imagePtr->lx) *
			       zoom * imagePtr->lx + (chainExt->pos %
						      imagePtr->lx));
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = hlPixel;

	      sprintf(interp->result, "%f %f %d %d %f",
		      chainExt->mod, chainExt->arg,
		      chainExt->pos % imagePtr->lx, chainExt->pos / imagePtr->lx,chainExt->bend);
	    }
	  else
	    {
	      chainExt = ExtMisClosestExtr(imagePtr,hlX +imagePtr->lx * hlY);
	      hlPixel    = get_color_intensity_(color_number);
	  
	      position = zoom*((chainExt->pos/imagePtr->lx) *
			       zoom * imagePtr->lx + (chainExt->pos %
						      imagePtr->lx));
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++)
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = hlPixel;
	      sprintf(interp->result, "%f %f %d %d %f",
		      chainExt->mod, chainExt->arg,
		      chainExt->pos % imagePtr->lx, chainExt->pos / imagePtr->lx,chainExt->bend);
	      /*	  _line2_(viewImagePtr->data,
			  imagePtr->lx*zoom, imagePtr->ly*zoom,
			  linePixel, 
			  position+(int)(zoom/2)*(imagePtr->lx*zoom+1), chainExt->arg);*/
	    }
	  _line2_(viewImagePtr->data, imagePtr->lx*zoom,
		  imagePtr->ly*zoom,  linePixel,
		  position+(int)(zoom/2)*(imagePtr->lx*zoom+1),
		  3*zoom+30*zoom*(chainExt->mod)*norm, chainExt->arg);
	}
    }
  if (imagePtr -> nb_of_lines)
    {
      Line *line_ptr;
      Extremum *gr_ext;

      if (is_greater_all)
	{
	  norm = norm*max/zeMax;

	  hlPixel  = get_color_intensity_ (RED);
	  foreach (line_ptr, imagePtr -> line_lst)
	    {
	      /*	  gr_ext = line_ptr -> greater_ext;*/
	      foreach (gr_ext, line_ptr -> gr_lst){
	  
		position = zoom*((gr_ext->pos/imagePtr->lx) *
				 zoom * imagePtr->lx + (gr_ext->pos %
							imagePtr->lx));
		for (u = 0; u < zoom; u++)
		  for (v = 0; v < zoom; v++)
		    viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = hlPixel;
		if (gr_ext -> up || gr_ext -> down) {
		  if (gr_ext->next) {
		    linePixel  = get_color_intensity_(BLUE);/*tagged extrema */
		  } else {
		    linePixel  = get_color_intensity_(RED);/* others*/
		  }
		  _line2_(viewImagePtr->data, imagePtr->lx*zoom,
			  imagePtr->ly*zoom, linePixel, 
			  position+(int)(zoom/2)*(imagePtr->lx*zoom+1),
			  3*zoom+30*zoom*(pow(gr_ext->mod,q))*norm, gr_ext->arg);
		  _line2_(viewImagePtr->data, imagePtr->lx*zoom,
			  imagePtr->ly*zoom, linePixel, 
			  position+1+(int)(zoom/2)*(imagePtr->lx*zoom+1),
			  3*zoom+30*zoom*(pow(gr_ext->mod,q))*norm, gr_ext->arg);
		  _line2_(viewImagePtr->data, imagePtr->lx*zoom,
			  imagePtr->ly*zoom, linePixel, 
			  position-1+(int)(zoom/2)*(imagePtr->lx*zoom+1),
			3*zoom+30*zoom*(pow(gr_ext->mod,q))*norm, gr_ext->arg);
	      
		} else {
		  linePixel  = get_color_intensity_(YELLOW);
		}

	      }
	    }
	}
      if (is_greater)
	{
	  hlPixel  = get_color_intensity_ (BLUE);
	  foreach (line_ptr, imagePtr -> line_lst)
	    {
	      gr_ext = line_ptr -> greater_ext;
	  
	      position = zoom*((gr_ext->pos/imagePtr->lx) *
			       zoom * imagePtr->lx + (gr_ext->pos %
						      imagePtr->lx));
	      for (u = 0; u < zoom; u++)
		for (v = 0; v < zoom; v++) {
		  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = hlPixel;
		  /*_line2_(viewImagePtr->data, imagePtr->lx*zoom,
		    imagePtr->ly*zoom, hlPixel,
		    position+v+zoom*imagePtr->lx*u,
		    3*zoom+30*zoom*(gr_ext->mod)*norm, gr_ext->arg);*/
		}
	      _line2_(viewImagePtr->data, imagePtr->lx*zoom,
		      imagePtr->ly*zoom, hlPixel,
		      position+(int)(zoom/2)*(imagePtr->lx*zoom+1),
		      3*zoom+30*zoom*(gr_ext->mod)*norm, gr_ext->arg);
	      /*_line2_(viewImagePtr->data, imagePtr->lx*zoom,
		      imagePtr->ly*zoom, hlPixel,
		      position+1+(int)(zoom/2)*(imagePtr->lx*zoom+1),
		      3*zoom+30*zoom*(gr_ext->mod)*norm, gr_ext->arg);*/
	    }
	}
    }



  ViewDicStore_(viewImageName,viewImagePtr);
  
  /*  Tcl_AppendResult(interp,viewImageName,NULL);*/
  return TCL_OK;
}

/*----------------------------------------------------------------------------
  --------------------------------------------------------------------------*/
int
ViewConvExtAddCmd_(ClientData clientData,
		   Tcl_Interp * interp,
		   int argc,
		   char ** argv)
{
  char * options[] = { "VE" ,
			 "-zoom", "d",
			 "-color", "d",
			 NULL };
  
  ExtImage    * imagePtr;
  ViewImage   * viewImagePtr;
  Extremum *chainExt;
  int         i, isColor = 0;
  int         zoom = 1;
  int lx;
  int         u, v, position;
  int color=get_color_intensity_ (RED);

  if (arg_init(interp, argc, argv, options, NULL))
    return TCL_OK;

  if (arg_get(0, &viewImagePtr, &imagePtr)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &zoom)==TCL_ERROR)
    return TCL_ERROR;

  isColor = arg_present(2);
  if (arg_get(2, &color)==TCL_ERROR)
    return TCL_ERROR;

  lx = imagePtr->lx;

  for (i=0;i<imagePtr->extrNb;i++)
    {
      position = zoom*((imagePtr->extr[i].pos/imagePtr->lx) *
		       zoom * imagePtr->lx + (imagePtr->extr[i].pos % 
					      imagePtr->lx));
      for (u=0;u<zoom;u++)
	for (v=0;v<zoom;v++)
	  viewImagePtr->data[position+v+zoom*imagePtr->lx*u] = color;
      chainExt = &(imagePtr->extr[i]);
    }
  
/*  Tcl_AppendResult(interp, posPixel, NULL);*/
  return TCL_OK;
}
