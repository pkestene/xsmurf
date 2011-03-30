/*
 * ConvSig.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: ConvSig.c,v 1.17 1999/03/30 14:59:29 decoster Exp $
 */

#include <string.h>
#include <stdlib.h>
#include "X11/X.h"
#include "math.h"
#include "Image.h"
#include "Dictionary.h"
#include "Convert.h"
#include "../signal/signal.h"
#include "../interpreter/arguments.h"
#include "../interpreter/hash_tables.h"

#define FONTH 10
#define SMALL 4
#define LARGE 8
#define BORDER 20

#define swap(a,b) tmp=(a);(a)=(b);(b)=tmp
#define sup(a,b) (a>b?a:b)
#define inf(a,b) (a<b?a:b)

/* BOURRRIIIIINNNN : cette macro est du copier coller de Xutil.h */
#define XDestroyImage(ximage) \
        ((*((ximage)->f.destroy_image))((ximage)))

/*----------------------------------------------------------------------------
  Conversion des objets Signal en ViewImage
  --------------------------------------------------------------------------*/

/* zone couverte par la fenetre */
static int  xWinMin, xWinMax;
static int  iWinMin, iWinMax;
static real yWinMin, yWinMax;

static real  xWinMin_r, xWinMax_r;
static real  xWinAmp_r;

/* amplitude de la fenetre xWinAmp = xWinMax-xWinMin */
static real xWinAmp,yWinAmp;

/* nombre de pixels suivant X et Y pour la representation dans la fenetre */
static int wX,wY;

/*----------------------------------------------------------------------------
  _ConvertCoord_ 
  
  Convertit les coordonnees reelles en coordonnees sur la fenetre, compte
  tenu des variables globales xWinMin,xWinMax,yWinMin,yWinMax et wX, wY.
  --------------------------------------------------------------------------*/
static void
_ConvertCoord_(real x,real y,int * rX,int * rY)
{
  *rX = BORDER + (int) (.5 + (real)(wX)*(x-xWinMin)/xWinAmp );
  *rY = BORDER + wY - (int) (.5 + (real)(wY)*(y-yWinMin)/yWinAmp );
}

/*----------------------------------------------------------------------------
  _DisplayTitle_
  --------------------------------------------------------------------------*/
static void _DisplayTitle_(Display * display,Pixmap pixmap,
			   GC gc,char * title)
{
  XDrawString(display,pixmap,gc,(wX+2*BORDER-strlen(title)*FONTH)/2,
	      FONTH,title,strlen(title));
}


static double g_stsize (double,double,int *,double *,double* , double*,int*);
/*----------------------------------------------------------------------------
  _DisplayAxes_
  --------------------------------------------------------------------------*/
static void
_DisplayAxes_(Display * display,
	      Pixmap  pixmap,
	      GC      gc,
	      real    x0,
	      real    dx)
{
  double  xvmin,xvmax,rmin;
  real   pos;
  int     n12,nn0,nnHor,nnVert;
  real   horPos,vertPos;
  double  horSpace,vertSpace;
  int     x,y,dummy,n;
  char    buffer[50];
  int     vertPixel,horPixel,nbColors;
  
  /* on trace en trois niveaux d'intensite */
  nbColors = get_number_of_grey_scales_ ();

  
  _ConvertCoord_(0,0,&x,&y);

  vertSpace = g_stsize(yWinMin,yWinMax,&n12,&xvmin,&xvmax,&rmin,&nnVert);  
  vertPos   = (real) rmin;
  horSpace  = g_stsize(xWinMin,xWinMax,&n12,&xvmin,&xvmax,&rmin,&nnHor);
  horPos    = (real) rmin;

  /* 
   *  PREMIER NIVEAU 
   */

  vertPixel = get_color_intensity_ (GREY1);
  horPixel  = get_color_intensity_ (GREY1);  
  
  /* lignes verticales */

  XSetForeground(display,gc,vertPixel);
  pos = vertPos;
  for (n=0; n < n12; n++)
    {
      _ConvertCoord_(0,pos,&dummy,&y);
      XDrawLine(display,pixmap,gc,0,y,wX+2*BORDER,y);
      pos += (real) vertSpace;
    }

  /* lignes horizontales */
  
  XSetForeground(display,gc,horPixel);
  pos = horPos;
  for (n=0; n < n12; n++)
    {
      _ConvertCoord_(pos,0,&x,&dummy);
      XDrawLine(display,pixmap,gc,x,0,x,wY+2*BORDER);
      pos += (real) horSpace;
    }

  /* 
   *  DEUXIEME NIVEAU
   */
  
  vertPixel = get_color_intensity_ (GREY2);
  horPixel  = get_color_intensity_ (GREY2);  

  /* lignes verticales */

  XSetForeground(display,gc,vertPixel);
  pos = vertPos;
  nn0 = nnVert;
  for (n=0; n < n12; n++)
    {
      if (!nn0)
	{
	  _ConvertCoord_(0,pos,&dummy,&y);
	  XDrawLine(display,pixmap,gc,0,y,wX+2*BORDER,y);
	}
      pos += vertSpace;
      nn0++;
      nn0 %= 5;
    }

  /* lignes horizontales */
  
  XSetForeground(display,gc,horPixel);
  pos = horPos;
  nn0 = nnHor;
  for (n=0; n < n12; n++)
    {
      if (!nn0)
	{
	  _ConvertCoord_(pos,0,&x,&dummy);
	  XDrawLine(display,pixmap,gc,x,0,x,wY+2*BORDER);
	}
      pos += horSpace;
      nn0++;
      nn0 %= 5;
    }

  /* 
   *  TROISIEME NIVEAU
   */
  
  vertPixel = get_color_intensity_ (GREY3);
  horPixel  = get_color_intensity_ (GREY3);  

  /* labels verticaux */

  XSetForeground(display,gc,vertPixel);
  pos = vertPos;
  nn0 = nnVert;
  for (n=0; n < n12; n++)
    {
      if ((pos*pos) < (vertSpace*0.00001)) pos = 0.0;
      if (!nn0)
	{
	  _ConvertCoord_(0,pos,&dummy,&y);
	  sprintf(buffer,"%2.5g",pos);
	  XDrawString(display,pixmap,gc,0,y-2,buffer,
		      strlen(buffer));	  
	}
      pos += vertSpace;
      nn0++;
      nn0 %= 5;
    }

  /* labels horizontaux */
  
  XSetForeground(display,gc,horPixel);
  pos = horPos;
  nn0 = nnHor;
  for (n=0; n < n12; n++)
    {
      if ((pos*pos) < (vertSpace*0.00001)) pos = 0.0;
      if (!nn0)
	{
	  _ConvertCoord_(pos,0,&x,&dummy);
	  sprintf(buffer,"%2.5g",x0 + pos*dx);
	  XDrawString(display,pixmap,gc,x-strlen(buffer)*FONTH/4,
		      wY+2*BORDER-2,buffer,strlen(buffer));
	}
      pos += horSpace;
      nn0++;
      nn0 %= 5;
    }
}

/*----------------------------------------------------------------------------
  Fonction repompee integralement de sw, que je n'ai meme pas cherche
  a comprendre...
  --------------------------------------------------------------------------*/
#define ALMOST1		 .9999999
#define ALMOST1_DIV_5	 .1999999



static double Round[] = {1., 2., 2.5, 5., 10., 20.};


static double g_stsize (vmin, vmax, n12, xvmin, xvmax, rmin, nn0)
     double vmin, vmax;
     double *xvmin, *xvmax, *rmin;
     int *n12, *nn0;
{
  double pstep, log10, rstep, order, power, smin, use1, vdif;
  int i, rmin_old;


  vdif = vmax - vmin;
  pstep = fabs (vmax - vmin) / 6;
  log10 = log (10.0);
  order = log (pstep) / log10;

  if (order < 0)
    order = order - ALMOST1;

  order = (int) order;
  power = pow (10.0, order);

  for (i = 0; i < 6; i++)
    {
      rstep = Round[i] * power;
      if (rstep >= pstep)
	break;
    }

  smin = vmin / rstep;
  if (smin < 0)
    smin = smin - ALMOST1_DIV_5;
  if (vmax < vmin)
    smin += ALMOST1_DIV_5;
  *rmin = (int) (5 * smin) / 5.;
  rmin_old = (int) (smin);
  *nn0 = (int) ((*rmin - rmin_old) * 5);
  if (*nn0 <= 0)
    *nn0 = -*nn0;
  else
    *nn0 = 5 - *nn0;
  *rmin *= rstep;
  use1 = fabs (rstep);

  rstep = (vdif > 0) ? use1 : -use1;
  *xvmin = vmin - vdif * 1.e-5;
  *xvmax = vmax + vdif * 1.e-5;

  *n12 = (6 + 1) * (5 + 1);

  return (rstep / 5.);
}


static void
_ConvertCoord_r_(real x,real y,int * rX,int * rY)
{
  *rX = BORDER + (int) (.5 + (real)(wX)*(x-xWinMin_r)/xWinAmp_r );
  *rY = BORDER + wY - (int) (.5 + (real)(wY)*(y-yWinMin)/yWinAmp );
}

/*----------------------------------------------------------------------------
  _DisplayAxes_
  --------------------------------------------------------------------------*/
static void
_DisplayAxes_r_(Display * display,
		Pixmap  pixmap,
		GC      gc)
{
  double  xvmin,xvmax,rmin;
  real   pos;
  int     n12,nn0,nnHor,nnVert;
  real   horPos,vertPos;
  double  horSpace,vertSpace;
  int     x,y,dummy,n;
  char    buffer[50];
  int     vertPixel,horPixel,nbColors;
  
  /* on trace en trois niveaux d'intensite */
  nbColors = get_number_of_grey_scales_ ();

  
  _ConvertCoord_r_(0,0,&x,&y);

  vertSpace = g_stsize(yWinMin,yWinMax,&n12,&xvmin,&xvmax,&rmin,&nnVert);  
  vertPos   = (real) rmin;
  horSpace  = g_stsize(xWinMin_r,xWinMax_r,&n12,&xvmin,&xvmax,&rmin,&nnHor);
  horPos    = (real) rmin;

  /* 
   *  PREMIER NIVEAU 
   */

  vertPixel = get_color_intensity_ (GREY1);
  horPixel  = get_color_intensity_ (GREY1);  
  
  /* lignes */

  XSetForeground(display,gc,vertPixel);
  pos = vertPos;
  for (n=0; n < n12; n++)
    {
      _ConvertCoord_r_(0,pos,&dummy,&y);
      XDrawLine(display,pixmap,gc,0,y,wX+2*BORDER,y);
      pos += (real) vertSpace;
    }
  
  XSetForeground(display,gc,horPixel);
  pos = horPos;
  for (n=0; n < n12; n++)
    {
      _ConvertCoord_r_(pos,0,&x,&dummy);
      XDrawLine(display,pixmap,gc,x,0,x,wY+2*BORDER);
      pos += (real) horSpace;
    }

  /* 
   *  DEUXIEME NIVEAU
   */
  
  vertPixel = get_color_intensity_ (GREY2);
  horPixel  = get_color_intensity_ (GREY2);  

  /* lignes verticales */

  XSetForeground(display,gc,vertPixel);
  pos = vertPos;
  nn0 = nnVert;
  for (n=0; n < n12; n++)
    {
      if (!nn0)
	{
	  _ConvertCoord_r_(0,pos,&dummy,&y);
	  XDrawLine(display,pixmap,gc,0,y,wX+2*BORDER,y);
	}
      pos += vertSpace;
      nn0++;
      nn0 %= 5;
    }

  /* lignes horizontales */
  
  XSetForeground(display,gc,horPixel);
  pos = horPos;
  nn0 = nnHor;
  for (n=0; n < n12; n++)
    {
      if (!nn0)
	{
	  _ConvertCoord_r_(pos,0,&x,&dummy);
	  XDrawLine(display,pixmap,gc,x,0,x,wY+2*BORDER);
	}
      pos += horSpace;
      nn0++;
      nn0 %= 5;
    }

  /* 
   *  TROISIEME NIVEAU
   */
  
  vertPixel = get_color_intensity_ (GREY3);
  horPixel  = get_color_intensity_ (GREY3);  

  /* labels verticaux */

  XSetForeground(display,gc,vertPixel);
  pos = vertPos;
  nn0 = nnVert;
  for (n=0; n < n12; n++)
    {
      if ((pos*pos) < (vertSpace*0.00001)) pos = 0.0;
      if (!nn0)
	{
	  _ConvertCoord_r_(0,pos,&dummy,&y);
	  sprintf(buffer,"%2.5g",pos);
	  XDrawString(display,pixmap,gc,0,y-2,buffer,
		      strlen(buffer));	  
	}
      pos += vertSpace;
      nn0++;
      nn0 %= 5;
    }

  /* labels horizontaux */
  
  XSetForeground(display,gc,horPixel);
  pos = horPos;
  nn0 = nnHor;
  for (n=0; n < n12; n++)
    {
      if ((pos*pos) < (vertSpace*0.00001)) pos = 0.0;
      if (!nn0)
	{
	  _ConvertCoord_r_(pos,0,&x,&dummy);
	  sprintf(buffer,"%2.5g",pos);
	  XDrawString(display,pixmap,gc,x-strlen(buffer)*FONTH/4,
		      wY+2*BORDER-2,buffer,strlen(buffer));
	}
      pos += horSpace;
      nn0++;
      nn0 %= 5;
    }
}

/*----------------------------------------------------------------------------
  ViewConvSignalCmd_
  
  Convertit un Signal en ViewImage. 
  La procedure est entierement consacree au TYPE1, il faudra en refaire
  une plus generale si on veut rajouter d'autres types de graphes... en
  superposer, etc...
  --------------------------------------------------------------------------*/
int
ViewConvSignalCmd_(ClientData clientData,
		   Tcl_Interp * interp,
		   int        argc,
		   char       ** argv)
{
  char * options[] = { "Ss" ,
			 "-rangeX","dd",
			 "-width","d",
			 "-height","d",
			 "-title","s",
			 "-noaxe","",
			 NULL };
  
  Signal         * signal;
  ViewImage      * viewImage;
  char           * viewImageName, * title=NULL;

  char           * data;
  real           * sdata;
  complex        * cdata;
  Tk_Window themain    = (Tk_Window) clientData;
  Pixmap         pixmap;
  Display        * display = Tk_Display(themain);
  int            bgPixel = get_grey_intensity_ (0);
  int            fgPixel = get_grey_intensity_ (get_number_of_grey_scales_());
  XImage         * ximage;
  GC             gc      = Tk_GetGC(themain,0,NULL); /* Graphic Context */
  
  int            gX1,gX2,gY1,gY2;
  int            i;

  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;
  
  if (arg_get(0,&signal,&viewImageName)==TCL_ERROR)
    return TCL_ERROR;

  sdata = signal->dataY;

  if (arg_get(4,&title)==TCL_ERROR)
    return TCL_ERROR;
  
  /* intervalle suivant X represente dans la fenetre */
  xWinMin = 0;
  xWinMax = signal->n - 1;
  if (arg_get( 1, &xWinMin, &xWinMax ) == TCL_ERROR)
    return TCL_ERROR;
  if(xWinMin < 0) {
    xWinMin = 0;
  }
  if(xWinMax >= signal->n) {
    xWinMax = signal->n;
  }
  /*  if (signal->type == REALXY) {
    
  }*/

  /* intervalle suivant Y represente dans la fenetre */
  switch (signal->type)
    {
    case REALY:
      sig_get_extrema_between(signal,
			      xWinMin, xWinMax,
			      &yWinMin, &yWinMax,&iWinMin,&iWinMax);
      break;
    case REALXY:
      sig_get_extrema_between(signal,
			      xWinMin, xWinMax,
			      &yWinMin, &yWinMax,&iWinMin,&iWinMax);
      break;
    case CPLX:
    case FOUR_NR:
      {
	complex min, max;
	sig_get_extrema_between(signal,
				   xWinMin, xWinMax,
				   &min, &max,&iWinMin,&iWinMax);
	yWinMax = sup(max.real, max.imag);
	yWinMin = inf(min.real, min.imag);
	break;
      }
    }
/*  switch (signal->type)
    {
    case REALY:
      sig_get_extrema_between(signal,
			      xWinMin, xWinMax,
			      &yWinMin, &yWinMax);
      break;
    case REALXY:
      sig_get_extrema_between(signal,
			      xWinMin, xWinMax,
			      &yWinMin, &yWinMax);
      break;
    case CPLX:
    case FOUR_NR:
      {
	complex min, max;
	sig_get_extrema_between(signal,
				   xWinMin, xWinMax,
				   &min, &max);
	yWinMax = sup(max.real, max.imag);
	yWinMin = inf(min.real, min.imag);
	break;
      }
    }*/
  /* protection contre les amplitudes nulles... */
  if (yWinMax <= yWinMin)
    {
      yWinMax = yWinMin + 0.5;
      yWinMin = yWinMax - 1.0;
    }
  xWinAmp = xWinMax-xWinMin;
  yWinAmp = yWinMax-yWinMin;

  /* dimensions de la fenetre, en Pixels */
  wX= (wY = 300)+100;
  if (arg_get(2,&wX)==TCL_ERROR)
    return TCL_ERROR;
  if (arg_get(3,&wY)==TCL_ERROR)
    return TCL_ERROR;
  
  if (wX<1 || wX>1300 || wY<1 || wY>1100)
    return GenErrorAppend(interp,"Incorrect window size.",NULL);
  
  /* maintenant commence le travail de conversion */ 
  
  /* on cree un pixmap temporaire pour pouvoir utiliser les routines
     graphiques de X */
  pixmap = XCreatePixmap(display,Tk_WindowId(themain),
			 wX+2*BORDER,wY+2*BORDER,
			 DefaultDepthOfScreen(Tk_Screen(themain)));
  
  /* on efface le contenu du pixmap */
  XSetForeground(display,gc,bgPixel);
  XFillRectangle(display,pixmap,gc,0,0,wX+2*BORDER,wY+2*BORDER);

  /* affiche les axes */
  _DisplayAxes_(display, pixmap, gc, signal->x0, signal->dx);

  /* affiche le titre */
  XSetForeground(display,gc,fgPixel);
  if (title)
    _DisplayTitle_(display,pixmap,gc,title);
  
  /* maintenant, on trace la courbe... */

  if(signal->type == REALY)
    {
      fgPixel = get_color_intensity_ (WHITE);
      XSetForeground(display,gc,fgPixel);
      _ConvertCoord_(xWinMin, sdata[xWinMin], &gX1, &gY1);
      for (i=xWinMin; i <= xWinMax; i++)
	{
/*	  _ConvertCoord_(i, sdata[i-(int)(signal->first)], &gX2, &gY2);*/
	  _ConvertCoord_(i, sdata[i], &gX2, &gY2);
	  XDrawLine(display, pixmap, gc, gX1, gY1, gX2, gY2);
	  gX1 = gX2; 
	  gY1 = gY2;
	} 
    }
/*  else if (signal->type == FOUR_NR)
    {
      int nbColors;
      int re_pixel, im_pixel;
      int N = (xWinMax - xWinMin) +1;

      cdata = (complex *) (sdata);
      re_pixel = get_color_intensity_ (RED);
      im_pixel = get_color_intensity_ (GREEN);

      XSetForeground (display, gc, re_pixel);
      _ConvertCoord_(0, cdata[(int)(N/2)].real, &gX1, &gY1);
      for (i=N/2+1; i < N; i++)
	{
	  _ConvertCoord_(i-N/2, cdata[i].real, &gX2, &gY2);
	  XDrawLine(display, pixmap, gc, gX1, gY1, gX2, gY2);
	  gX1 = gX2; 
	  gY1 = gY2;
	} 
      for (i=0; i <= (N/2); i++)
	{
	  _ConvertCoord_(i+N/2, cdata[i].real, &gX2, &gY2);
	  XDrawLine(display, pixmap, gc, gX1, gY1, gX2, gY2);
	  gX1 = gX2; 
	  gY1 = gY2;
	} 
      XSetForeground (display, gc, im_pixel);
      _ConvertCoord_(0, cdata[(int)(N/2)].imag, &gX1, &gY1);
      for (i=N/2+1; i < N; i++)
	{
	  _ConvertCoord_(i-N/2, cdata[i].imag, &gX2, &gY2);
	  XDrawLine(display, pixmap, gc, gX1, gY1, gX2, gY2);
	  gX1 = gX2; 
	  gY1 = gY2;
	} 
      for (i=0; i <= (N/2); i++)
	{
	  _ConvertCoord_(i+N/2, cdata[i].imag, &gX2, &gY2);
	  XDrawLine(display, pixmap, gc, gX1, gY1, gX2, gY2);
	  gX1 = gX2; 
	  gY1 = gY2;
	} 
    }*/
  else if ((signal->type == CPLX) || (signal->type == FOUR_NR))
    {
      unsigned long re_pixel, im_pixel;

      cdata = (complex *) (sdata);
      re_pixel = get_color_intensity_ (RED);
      im_pixel = get_color_intensity_ (GREEN);

      XSetForeground (display, gc, re_pixel);
      _ConvertCoord_(xWinMin, cdata[xWinMin].real, &gX1, &gY1);
      for (i=xWinMin+1; i <= xWinMax; i++)
	{
	  _ConvertCoord_(i, cdata[i].real, &gX2, &gY2);
	  XDrawLine(display, pixmap, gc, gX1, gY1, gX2, gY2);
	  gX1 = gX2; 
	  gY1 = gY2;
	} 
      XSetForeground(display, gc, im_pixel);
      _ConvertCoord_(xWinMin, cdata[xWinMin].imag, &gX1, &gY1);
      for (i=xWinMin+1; i <= xWinMax; i++)
	{
	  _ConvertCoord_(i, cdata[i].imag, &gX2, &gY2);
	  XDrawLine(display, pixmap, gc, gX1, gY1, gX2, gY2);
	  gX1 = gX2; 
	  gY1 = gY2;
	}
    }
  /* on convertit enfin le pixmap en XImage */
  ximage = XGetImage(display,pixmap,0,0,wX+2*BORDER,wY+2*BORDER,
		     255 /*plane mask */,ZPixmap);
  
  /* on detruit le pixmap. On ne garde de la structure XImage que le
     pointeur data qui servira a creer la ViewImage */
  
  XFreePixmap(display,pixmap);
  data = (char *) ximage->data;
  ximage->data = NULL;
  XDestroyImage(ximage);
  
  viewImage = ViewImaCreateFromData_(wX+2*BORDER,wY+2*BORDER,(unsigned long *)data);
  ViewDicStore_(viewImageName,viewImage);

  Tcl_AppendResult(interp,viewImageName,NULL);
  return TCL_OK;
}

/*
*/
int
init_sig_viewer_TclCmd_(ClientData clientData,
			Tcl_Interp * interp,
			int        argc,
			char       ** argv)
{
  char * options[] = {"sffffdd",
		      NULL };
  
  ViewImage * viewImage;
  char      * viewImageName;
  Tk_Window themain    = (Tk_Window) clientData;
  Pixmap    pixmap;
  Display   * display = Tk_Display(themain);
  int       bgPixel = get_color_intensity_ (BLACK);
  XImage    * ximage;
  GC        gc      = Tk_GetGC(themain,0,NULL); /* Graphic Context */
  
  char      * data;

  real      x_min, x_max;
  real      y_min, y_max;

  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get (0, &viewImageName,
	       &x_min, &x_max,
	       &y_min, &y_max,
	       &wX, &wY) == TCL_ERROR)
    return TCL_ERROR;
  
  /*  wX = 400;
  wY = 300;
  */
  /* protection contre les amplitudes nulles... */
  if (y_max <= y_min)
    {
      y_max = y_min + 0.5;
      y_min = y_max - 1.0;
    }

  yWinMin = y_min;
  yWinMax = y_max;

  xWinMin_r = x_min;
  xWinMax_r = x_max;

  xWinAmp_r = xWinMax_r - xWinMin_r;
  yWinAmp   = yWinMax - yWinMin;

  pixmap = XCreatePixmap(display,Tk_WindowId(themain),
			 wX+2*BORDER,wY+2*BORDER,
			 DefaultDepthOfScreen(Tk_Screen(themain)));
  XSetForeground(display,gc,bgPixel);
  XFillRectangle(display,pixmap,gc,0,0,wX+2*BORDER,wY+2*BORDER);

  /*  XSetForeground(display,gc,fgPixel);
  XDrawLine(display, pixmap, gc, 0, 0, wX+2*BORDER-1,wY+2*BORDER-1);*/

  /* affiche les axes */
  _DisplayAxes_r_(display, pixmap, gc);

  /* on convertit enfin le pixmap en XImage */
  ximage = XGetImage(display,pixmap,0,0,wX+2*BORDER,wY+2*BORDER,
		     255 /*plane mask */,ZPixmap);
  
  /* on detruit le pixmap. On ne garde de la structure XImage que le
     pointeur data qui servira a creer la ViewImage */
  
  XFreePixmap(display,pixmap);
  data = (char *) ximage->data;
  ximage->data = NULL;
  XDestroyImage(ximage);
  
  viewImage = ViewImaCreateFromData_(wX+2*BORDER,wY+2*BORDER,(unsigned long *) data);
  ViewDicStore_(viewImageName,viewImage);

  Tcl_AppendResult(interp,viewImageName,NULL);
  return TCL_OK;
}

/************************/

static void
_line3_(unsigned long *data,
       int  lx,
       unsigned long  val,
       int  pos,
       int  x1,
       int  y1,
       int  x2,
       int  y2)
{
  real x, y;
  real xstep, ystep;
  real tmp;


  if (x1 == x2 && y1 == y2)
    data[pos + x1 + y1*lx] = val;
  else
    {
      if (abs(x1-x2) > abs(y1-y2))
	{
	  if (x2 < x1) {swap (x1, x2);swap (y1, y2);}
	  ystep = (real) (y2 - y1)/abs(x2-x1);
	  xstep = x2 > x1 ? 1 : -1;
	  for (x = x1, y = y1;
	       x < x2;
	       x = (real) x + xstep, y = (real) y + ystep)
	    data[pos + (int)x + (int)(y)*lx] = val;
	}
      else
	{
	  if (y2 < y1) {swap (x1, x2);swap (y1, y2);}
	  xstep = (real) (x2 - x1)/abs(y2-y1);
	  ystep = y2 > y1 ? 1 : -1;
	  for (x = x1, y = y1;
	       y < y2;
	       x = (real) x + xstep, y = (real) y + ystep)
	    data[pos + (int)x + (int)(y)*lx] = val;
	}
    }
}

/*
 */
enum {
  DISP_PIXEL,
  DISP_PLUS,
  DISP_LINE,
  DISP_CIRCLE
};

#define NO_DISP_TYPE -1

static char * _disp_type_strings_lst_[] =
{
  "pixel",
  "plus",
  "line",
  "circle",
  NULL
};

static int
_get_disp_types_ (char * disp_type_str)
{
  int i = 0;

  while (_disp_type_strings_lst_[i])
    {
      if (!strcmp (_disp_type_strings_lst_[i], disp_type_str))
	return i;
      i++;
    }
	  
  return NO_DISP_TYPE; /* wrong disp_type_str */
}

/*
Color list from Colormap.h

enum {
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

void
_draw_plus_ (ViewImage* view_image,
	     int        lx,
	     int        x,
	     int        y,
	     int        fgPixel)
{
  view_image->data[x + y*lx] = fgPixel;

  view_image->data[x - 1 + y*lx] = fgPixel;
  view_image->data[x - 2 + y*lx] = fgPixel;
  view_image->data[x + 1 + y*lx] = fgPixel;
  view_image->data[x + 2 + y*lx] = fgPixel;

  view_image->data[x + (y - 1)*lx] = fgPixel;
  view_image->data[x + (y - 2)*lx] = fgPixel;
  view_image->data[x + (y + 1)*lx] = fgPixel;
  view_image->data[x + (y + 2)*lx] = fgPixel;
}

int
_is_in_image_ (int x,
	       int y)
{
  return (x <= (wX+BORDER) && x >= BORDER && y <= (wY+BORDER) && y >= BORDER);
}
/*
*/
int
draw_signal_TclCmd_(ClientData clientData,
		    Tcl_Interp * interp,
		    int        argc,
		    char       ** argv)
{
  char * options[] = {"VSffffdd",
		      "-type", "s",
		      "-color", "s",
		      NULL };

  int       is_type;
  int       is_color;

  char      *disp_type_str;
  int       disp_type = DISP_PIXEL;
  char      *color_str;
  int       color = WHITE;
  Signal    *signal;
  ViewImage * view_image;
  Tk_Window themain    = (Tk_Window) clientData;
  Display   * display = Tk_Display(themain);
  int       fgPixel = get_grey_intensity_ (get_number_of_grey_scales_());
  GC        gc      = Tk_GetGC(themain,0,NULL); /* Graphic Context */
  
  real      *sdata;

  real      x_min, x_max;
  real      y_min, y_max;
  int       first_index, last_index;
  real      x0, dx;
  int       i;
  int       x, y;
  int       prev_x, prev_y;
  int       lx;
  unsigned long re_pixel, im_pixel;
  /*  void (*_draw_symbol_)();*/

  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get (0, &view_image, &signal,
	       &x_min, &x_max,
	       &y_min, &y_max,
	       &wX, &wY) == TCL_ERROR)
    return TCL_ERROR;

  is_type = arg_present (1);
  if (is_type)
    {
      if (arg_get (1, &disp_type_str) == TCL_ERROR)
	return TCL_ERROR;
      disp_type = _get_disp_types_ (disp_type_str);
    }
  if (disp_type == NO_DISP_TYPE)
    {
      sprintf (interp->result,
	       "The display type must be one of the following :\n");
      i = 0;
      while (_disp_type_strings_lst_[i])
	{
	  sprintf (interp->result, "%s ", _disp_type_strings_lst_[i]);
	  i++;
	}
      return TCL_ERROR;
    }

  is_color = arg_present (2);
  if (is_color)
    {
      if (arg_get (2, &color_str) == TCL_ERROR)
	return TCL_ERROR;
      color = _get_color_ (color_str);
    }
  
  if (color == NO_COLOR)
    {
      sprintf (interp->result,
	       "The color must be one of the following :\n");
      i = 0;
      while (_color_strings_lst_[i])
	{
	  sprintf (interp->result, "%s ", _color_strings_lst_[i]);
	  i++;
	}
      return TCL_ERROR;
    }

  first_index = sig_get_index (signal, x_min);
  last_index = sig_get_index (signal, x_max);
  sdata = signal->dataY;
  dx = signal->dx;
  x0 = signal->x0;

  if (y_max <= y_min)
    {
      y_max = y_min + 0.5;
      y_min = y_max - 1.0;
    }
  yWinMin = y_min;
  yWinMax = y_max;

  xWinMin_r = x_min;
  xWinMax_r = x_max;

  xWinAmp_r = xWinMax_r - xWinMin_r;
  yWinAmp   = yWinMax - yWinMin;

  fgPixel = get_color_intensity_ (color);
  XSetForeground(display,gc,fgPixel);

  lx = view_image->lx;
  switch (disp_type)
    {
    case DISP_PLUS:
      switch (signal->type) {
      case REALY:
	for (i = first_index; i <= last_index; i++) {
	  _ConvertCoord_r_ (x0 + i*dx, sdata[i], &x, &y);
	  if (_is_in_image_ (x, y)) {
	    _draw_plus_ (view_image, lx, x, y, fgPixel);
	  }
	}
	break;
      case REALXY:
	for (i = 0; i < signal->size; i++) {
	  _ConvertCoord_r_ (signal->dataX[i], sdata[i], &x, &y);
	  if (_is_in_image_ (x, y)) {
	    _draw_plus_ (view_image, lx, x, y, fgPixel);
	  }
	}
	break;
      case CPLX:
      case FOUR_NR:
	re_pixel = get_color_intensity_ (RED);
	im_pixel = get_color_intensity_ (GREEN);
	for (i = first_index; i <= last_index; i++) {
	  _ConvertCoord_r_ (x0 + i*dx, sdata[2*i], &x, &y);
	  if (_is_in_image_ (x, y)) {
	    _draw_plus_ (view_image, lx, x, y, fgPixel);
	  }
	}
	for (i = first_index; i <= last_index; i++) {
	  _ConvertCoord_r_ (x0 + i*dx, sdata[2*i+1], &x, &y);
	  if (_is_in_image_ (x, y)) {
	    _draw_plus_ (view_image, lx, x, y, fgPixel);
	  }
	}
	break;
      }
      break;
    case DISP_PIXEL:
      if(signal->type == REALY)
	for (i = first_index; i <= last_index; i++)
	  {
	    _ConvertCoord_r_ (x0 + i*dx, sdata[i], &x, &y);
	    if (x <= (wX+BORDER) && x >= BORDER
		&& y <= (wY+BORDER) && y >= BORDER)
	      view_image->data[x + y*lx] = fgPixel;
	  }
      else if ((signal->type == CPLX) || (signal->type == FOUR_NR))
	{
	  unsigned long re_pixel, im_pixel;

	  re_pixel = get_color_intensity_ (RED);
	  im_pixel = get_color_intensity_ (GREEN);
	  for (i = first_index; i <= last_index; i++)
	    {
	      _ConvertCoord_r_ (x0 + i*dx, sdata[2*i], &x, &y);
	      if (x <= (wX+BORDER) && x >= BORDER
		  && y <= (wY+BORDER) && y >= BORDER)
		view_image->data[x + y*lx] = re_pixel;
	    }
	  for (i = first_index; i <= last_index; i++)
	    {
	      _ConvertCoord_r_ (x0 + i*dx, sdata[2*i+1], &x, &y);
	      if (x <= (wX+BORDER) && x >= BORDER
		  && y <= (wY+BORDER) && y >= BORDER)
		view_image->data[x + y*lx] = im_pixel;
	    }
	}
      break;
    case DISP_LINE:
      if(signal->type == REALY)
	{
	  i = first_index;
	  _ConvertCoord_r_ (x0 + i*dx, sdata[i], &prev_x, &prev_y);
	  for (i = first_index+1; i <= last_index; i++)
	    {
	      _ConvertCoord_r_ (x0 + i*dx, sdata[i], &x, &y);
	      if (x <= (wX+BORDER) && x >= BORDER
		  && y <= (wY+BORDER) && y >= BORDER
		  && prev_x <= (wX+BORDER) && prev_x >= BORDER
		  && prev_y <= (wY+BORDER) && prev_y >= BORDER)
		_line3_ (view_image->data, lx, fgPixel, 0,
			 prev_x, prev_y, x, y);
	      prev_x = x;
	      prev_y = y;
	      /*view_image->data[x + y*lx] = fgPixel;*/
	    }
	}
      else if(signal->type == REALXY)
	{
	  i = first_index;
	  _ConvertCoord_r_ (signal->dataX[i], sdata[i], &prev_x, &prev_y);
	  for (i = 1; i < signal->size; i++)
	    {
	      _ConvertCoord_r_ (signal->dataX[i], sdata[i], &x, &y);
	      if (x <= (wX+BORDER) && x >= BORDER
		  && y <= (wY+BORDER) && y >= BORDER
		  && prev_x <= (wX+BORDER) && prev_x >= BORDER
		  && prev_y <= (wY+BORDER) && prev_y >= BORDER)
		_line3_ (view_image->data, lx, fgPixel, 0,
			 prev_x, prev_y, x, y);
	      prev_x = x;
	      prev_y = y;
	      /*view_image->data[x + y*lx] = fgPixel;*/
	    }
	}
      else if ((signal->type == CPLX) || (signal->type == FOUR_NR))
	{
	  unsigned long re_pixel, im_pixel;

	  re_pixel = get_color_intensity_ (RED);
	  im_pixel = get_color_intensity_ (GREEN);

	  i = first_index;
	  _ConvertCoord_r_ (x0 + i*dx, sdata[2*i], &prev_x, &prev_y);
	  for (i = first_index+1; i <= last_index; i++)
	    {
	      _ConvertCoord_r_ (x0 + i*dx, sdata[2*i], &x, &y);
	      if (x <= (wX+BORDER) && x >= BORDER
		  && y <= (wY+BORDER) && y >= BORDER
		  && prev_x <= (wX+BORDER) && prev_x >= BORDER
		  && prev_y <= (wY+BORDER) && prev_y >= BORDER)
		_line3_ (view_image->data, lx, re_pixel, 0,
			 prev_x, prev_y, x, y);
	      prev_x = x;
	      prev_y = y;
	      /*view_image->data[x + y*lx] = fgPixel;*/
	    }
	  i = first_index;
	  _ConvertCoord_r_ (x0 + i*dx, sdata[2*i+1], &prev_x, &prev_y);
	  for (i = first_index+1; i <= last_index; i++)
	    {
	      _ConvertCoord_r_ (x0 + i*dx, sdata[2*i+1], &x, &y);
	      if (x <= (wX+BORDER) && x >= BORDER
		  && y <= (wY+BORDER) && y >= BORDER
		  && prev_x <= (wX+BORDER) && prev_x >= BORDER
		  && prev_y <= (wY+BORDER) && prev_y >= BORDER)
		_line3_ (view_image->data, lx, im_pixel, 0,
			 prev_x, prev_y, x, y);
	      prev_x = x;
	      prev_y = y;
	      /*view_image->data[x + y*lx] = fgPixel;*/
	    }
	}
      break;
    }

  return TCL_OK;
}

static void
_line_(unsigned long *data,
       int  lx,
       int  ly,
       unsigned long  val,
       int  x1,
       int  y1,
       int  x2,
       int  y2)
{
  real x, y;
  real xstep, ystep;

  if (abs(x1-x2) > abs(y1-y2))
    {
      ystep = (y2 - y1)/(real)abs(x2-x1);
      xstep = x2 > x1 ? 1 : -1;
    }
  else
    {
      xstep = (x2 - x1)/(real)abs(y2-y1);
      ystep = y2 > y1 ? 1 : -1;
    }
  for (x = x1, y = y1; x < x2; x = x + xstep, y = y + ystep)
    if (x >= 0 && x < lx && y >= 0 && y <ly)
      data[(int)x + (int)y*lx] = val;
}

/*
*/
int
draw_line_TclCmd_(ClientData clientData,
		  Tcl_Interp * interp,
		  int        argc,
		  char       ** argv)
{
  char * options[] = {"Vffffffffdd",
		      "-color", "s",
		      NULL };

  int       is_color;

  char      *color_str;
  int       color = WHITE;
  ViewImage * view_image;
  Tk_Window themain    = (Tk_Window) clientData;
  Display   * display = Tk_Display(themain);
  int       fgPixel = get_grey_intensity_ (get_number_of_grey_scales_());
  GC        gc      = Tk_GetGC(themain,0,NULL); /* Graphic Context */
  
  real      x_min, x_max;
  real      y_min, y_max;

  real      x1, y1, x2, y2;
  int       int_x1, int_y1, int_x2, int_y2;
  int       i;

  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get (0, &view_image,
	       &x1, &y1, &x2, &y2,
	       &x_min, &x_max,
	       &y_min, &y_max,
	       &wX, &wY) == TCL_ERROR)
    return TCL_ERROR;

  is_color = arg_present (2);
  if (is_color)
    {
      if (arg_get (1, &color_str) == TCL_ERROR)
	return TCL_ERROR;
      color = _get_color_ (color_str);
    }
  
  if (color == NO_COLOR)
    {
      sprintf (interp->result,
	       "The color must be one of the following :\n");
      i = 0;
      while (_color_strings_lst_[i])
	{
	  sprintf (interp->result, "%s ", _color_strings_lst_[i]);
	  i++;
	}
      return TCL_ERROR;
    }

  if (y_max <= y_min)
    {
      y_max = y_min + 0.5;
      y_min = y_max - 1.0;
    }
  yWinMin = y_min;
  yWinMax = y_max;

  xWinMin_r = x_min;
  xWinMax_r = x_max;

  xWinAmp_r = xWinMax_r - xWinMin_r;
  yWinAmp   = yWinMax - yWinMin;

  fgPixel = get_color_intensity_ (color);
  XSetForeground (display, gc, fgPixel);
  _ConvertCoord_r_ (x1, y1, &int_x1, &int_y1);
  _ConvertCoord_r_ (x2, y2, &int_x2, &int_y2);
  _line_ (view_image->data,
	  view_image->lx, view_image->ly, fgPixel,
	  int_x1, int_y1, int_x2, int_y2);

  return TCL_OK;
}


#define _x_convert_(real_x) ((int)((real)(width)*((real_x)-xMin)/(xMax-xMin)))
#define _y_convert_(real_y) (height-(int)((real)(height)*((real_y)-yMin)/(yMax-yMin)))

static void
_bitmap_draw_pixel_ (unsigned char* source,
		     int            x,
		     int            y,
		     int            width,
		     int            height)
{
  int charWidth;

  charWidth = (width-1)/8+1;
  if (x < width && x >= 0 && y < height && y >= 0)
    source[x/8+y*charWidth] |= (unsigned char) (1<<(x%8));
}

static void
_bitmap_draw_plus_ (unsigned char* source,
		    int            x,
		    int            y,
		    int            width,
		    int            height)
{
  int charWidth;
  int i, j;

  charWidth = (width-1)/8+1;

  for (i = x-2; i < (x+3); i++)
    if (i < width && i >= 0 && y < height && y >= 0)
      source[i/8+y*charWidth] |= (unsigned char) (1<<(i%8));
  for (j = y-2; j < (y+3); j++)
    if (x < width && x >= 0 && j < height && j >= 0)
      source[x/8+j*charWidth] |= (unsigned char) (1<<(x%8));
}

static void
_bitmap_draw_line_(unsigned char* source,
		   int            x1,
		   int            y1,
		   int            x2,
		   int            y2,
		   int            width,
		   int            height)
{
  real x, y;
  int  i;
  int  j;
  real xstep;
  real ystep;
  real tmp;
  int charWidth;

  charWidth = (width-1)/8+1;

  if (x1 == x2 && y1 == y2) {
    if (x1 < width && x1 >= 0 && y1 < height && y1 >= 0) {
      source[x1/8+y1*charWidth] |= (unsigned char) (1<<(x1%8));
    }
  }
  else {
    if (abs(x1-x2) > abs(y1-y2)) {
      if (x2 < x1) {
	swap (x1, x2);
	swap (y1, y2);
      }
      ystep = (real) (y2 - y1)/abs(x2-x1);
      xstep = x2 > x1 ? 1 : -1;
      for (x = x1, y = y1;
	   x < x2;
	   x = (real) x + xstep, y = (real) y + ystep) {
	i = (int) x;
	j = (int) y;
	if (i < width && i >= 0 && j < height && j >= 0)
	  source[i/8+j*charWidth] |= (unsigned char) (1<<(i%8));
      }
    }
    else {
      if (y2 < y1) {
	swap (x1, x2);
	swap (y1, y2);
      }
      xstep = (real) (x2 - x1)/abs(y2-y1);
      ystep = y2 > y1 ? 1 : -1;
      for (x = x1, y = y1;
	   y < y2;
	   x = (real) x + xstep, y = (real) y + ystep) {
	i = (int) x;
	j = (int) y;
	if (i < width && i >= 0 && j < height && j >= 0)
	  source[i/8+j*charWidth] |= (unsigned char) (1<<(i%8));
      }
    }
  }
}


#define _is_in_domain_(x,y) ((x)>=xMin&&(x)<=xMax&&(y)>=yMin&&(y)<=yMax)

/*
 */
int
sig_2_bitmap_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ssddffff",
    "-type", "s",
    NULL
  };

  char * help_msg =
  {
    ("no comment.")
  };

  /* Command's parameters */
  Signal* signal;
  char*   bmp_name;

  /* Options's presence */
  int is_type;

  /* Options's parameters */
  char* disp_type_str;

  /* Other variables */
  int  disp_type = DISP_PIXEL;
  int  width;
  int  height;
  int  i;
  int  x;
  int  y;
  int  x1;
  int  y1;
  int  x2;
  int  y2;
  real x0;
  real dx;
  real xMin;
  real xMax;
  real yMin;
  real yMax;
  int  charWidth;
  int  code;

  unsigned char* source;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &bmp_name,
	       &width, &height,
	       &xMin, &xMax, &yMin, &yMax) == TCL_ERROR)
    return TCL_ERROR;

  is_type = arg_present (1);
  if (is_type) {
    if (arg_get (1, &disp_type_str) == TCL_ERROR)
      return TCL_ERROR;
    disp_type = _get_disp_types_ (disp_type_str);
  }

  /* Parameters validity and initialisation */
  if (disp_type == NO_DISP_TYPE) {
    sprintf (interp->result,
	     "The display type must be one of the following :\n");
    i = 0;
    while (_disp_type_strings_lst_[i]) {
      sprintf (interp->result, "%s ", _disp_type_strings_lst_[i]);
      i++;
    }
    return TCL_ERROR;
  }

  /* Treatement */
  x0 = signal->x0;
  dx = signal->dx;
  charWidth = (width-1)/8+1;

  source = (unsigned char *) malloc (sizeof(unsigned char)*charWidth*height);
  if (!source) {
    sprintf(interp->result, "Memory allocation error");
    return TCL_ERROR;
  }

  for (i = 0; i < charWidth*height; i++) {
    source[i] = 0;
  }

  switch (signal->type) {
  case REALY:
    switch (disp_type) {
    case DISP_PLUS:
      for (i = 0; i < signal->size; i++) {
	if (_is_in_domain_(x0+i*dx, signal->dataY[i])) {
	  x = _x_convert_ (x0+i*dx);
	  y = _y_convert_ (signal->dataY[i]);
	  _bitmap_draw_plus_ (source, x, y, width, height);
	}
      }
      break;
    case DISP_PIXEL:
      for (i = 0; i < signal->size; i++) {
	if (_is_in_domain_(x0+i*dx, signal->dataY[i])) {
	  x = _x_convert_ (x0+i*dx);
	  y = _y_convert_ (signal->dataY[i]);
	  _bitmap_draw_pixel_ (source, x, y, width, height);
	}
      }
      break;
    case DISP_LINE:
      for (i = 0; i < signal->size-1; i++) {
	if ( _is_in_domain_(x0+i*dx, signal->dataY[i])
	     || _is_in_domain_(x0+(i+1)*dx, signal->dataY[i+1])) {
	  x1 = _x_convert_ (x0+i*dx);
	  y1 = _y_convert_ (signal->dataY[i]);
	  x2 = _x_convert_ (x0+(i+1)*dx);
	  y2 = _y_convert_ (signal->dataY[i+1]);
	  _bitmap_draw_line_ (source, x1, y1, x2, y2, width, height);
	}
      }
      break;
    }
    break;
  case REALXY:
    switch (disp_type) {
    case DISP_PLUS:
      for (i = 0; i < signal->size; i++) {
	if (_is_in_domain_(signal->dataX[i], signal->dataY[i])) {
	  x = _x_convert_ (signal->dataX[i]);
	  y = _y_convert_ (signal->dataY[i]);
	  _bitmap_draw_plus_ (source, x, y, width, height);
	}
      }
      break;
    case DISP_PIXEL:
      for (i = 0; i < signal->size; i++) {
	if (_is_in_domain_(signal->dataX[i], signal->dataY[i])) {
	  x = _x_convert_ (signal->dataX[i]);
	  y = _y_convert_ (signal->dataY[i]);
	  _bitmap_draw_pixel_ (source, x, y, width, height);
	}
      }
      break;
    case DISP_LINE:
      for (i = 0; i < signal->size-1; i++) {
	if ( _is_in_domain_(signal->dataX[i], signal->dataY[i])
	     || _is_in_domain_(signal->dataX[i+1], signal->dataY[i+1])) {
	  x1 = _x_convert_ (signal->dataX[i]);
	  y1 = _y_convert_ (signal->dataY[i]);
	  x2 = _x_convert_ (signal->dataX[i+1]);
	  y2 = _y_convert_ (signal->dataY[i+1]);
	  _bitmap_draw_line_ (source, x1, y1, x2, y2, width, height);
	}
      }
      break;
    }
    break;
  }

  code = Tk_DefineBitmap(interp, Tk_GetUid(bmp_name), source, width, height);
  if (code == TCL_ERROR) {
    /*    Tk_Window themain    = (Tk_Window) clientData;
    Pixmap    bitmap;
    Display   *display = Tk_Display(themain);
    unsigned char* tmp_s;

    bitmap = Tk_GetBitmap(interp, themain, Tk_GetUid(bmp_name));
    tmp_s = (unsigned char *)bitmap.source;*/
    free(source);
    return TCL_ERROR;
  }

  return TCL_OK;
}


#undef _x_convert_
#undef _y_convert_
