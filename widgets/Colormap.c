/*
 * Colormap.c --
 *
 *   Copyright 2001 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Pierre Kestener.
 *
 *  The author may be reached (Email) at the address
 *      kestener@crpp.u-bordeaux.fr
 * 
 *  This file is a modified version of Colormap.c to handle
 *  (i hope so) different visual class. (Especially the fact
 *  that now, very often, X servers are configured with many colors
 *  thas is visualclass equals TRUECOLOR and former code couldn't
 *  handle this.).
 */

/******************************************************************************
 *
 * some modification are taken and modified from Lastwave sources 
 * ((C) E. Bacry):
 * (LASTWAVEHOME)/unix/src/x11window.c
 * function : XXOpenGraphics, XXSetColormap.
 *
 ******************************************************************************/



#include "../interpreter/arguments.h"
#include "Colormap.h"
#include <math.h>
#include <stdlib.h>

#define MaxNColors 4000 

/*----------------------------------------------------------------------------
  Variables statiques decrivant la colormap et le display:
  _colors_        : tableau de correspondance entre numero de couleur et Pixel
  _nb_colors_     : nb de tons dans la colormap
  _grey_scales_   : tableau de correspondance entre numero de couleur et Pixel
  _nb_grey_scales_:
  _screenNb_
  _defColmp_      : colormap par defaut.
  --------------------------------------------------------------------------*/

static int              _nb_colors_ = 0;
static int              _nb_grey_scales_ = 0;
static unsigned long    _colors_      [MAX_COL_NUMBER]; 
static unsigned long    _grey_scales_ [MAX_COL_NUMBER]; 

static Display 	        *theDisplay=NULL;/* -- The display                          */
static int		theScreen;	 /* -- Which screen on the display          */
static int		theDepth;	 /* -- Number of color planes               */
static Colormap	        theColormap;	 /* -- default System color map             */
static Visual           *theVisual;      /* -- Visual color structure               */
                                         /*    note that struct Visual is usually   */
                                         /*    defined in /usr/X11R6/include/X11/   */
                                         /*    Xlib.h                               */
					 
static int 	        theVisualClass;  /* -- the class of the visual              */

static Colormap         myColormap;      /* the colormap used for all the windows   */
static int planeMask;


/**************************************************************
 * les structures qui suivent servent si on veut modifier
 * en direct live la carte de couleur... mais pas fait
 **************************************************************/


/*
 * The color structure
 */

typedef struct color {
  char *name;                    /* The name of the color (or NULL) */
  unsigned short red,blue,green; /* Values for Red Blue and Green */
  unsigned long pixel;           /* The corresponding pixel value */
  unsigned short index;          /* Index for internal use */
} Color, *COLOR;



/*
 * The colormap  structure
 */

typedef struct colorMap {
  char *name;     /* Name of the colormap */
  Color *colors;  /* The colors of the colormap */
  int size;       /* The number of colors */
} ColorMap, *COLORMAP;


/*
 * The global array of all the colormaps 
 * The first one (index 0) corresponds to the colors that are named (maximum is 256)
 * The other ones correspond to regular colormaps 
 */
#define ColorMapNMax 64
static COLORMAP theColorMaps[ColorMapNMax];

/* The current number of defined colormaps in "theColorMaps"  (except index 0) */
static int nColorMaps;



/*----------------------------------------------------------------------------
  Ces variables statiques servent a l'implementation des fonctions effectuant
  la conversion entre un reel et un numero de pixel... En clair, on a une 
  image reelle et on veut convertir la valeur de chaque point en Pixels.
  On commence par specifier le min et le max sur l'image et ensuite on
  peut convertir directement une valeur reelle en un Pixel suivant une echelle
  lineaire ou logarithmique.
  --------------------------------------------------------------------------*/
static real _amplitudeLog_,_amplitudeLin_;
static real _offsetLog_,_offsetLin_;
static unsigned long   _highlightPixel_;









/****************************************************************************
 ***************************************************************************/	
static void
_delete_colormap_ ()
{
  XFreeColors (theDisplay, myColormap, _colors_, _nb_colors_, 0);
  XFreeColors (theDisplay, myColormap, _grey_scales_, _nb_grey_scales_, 0);
}



/****************************************************************************
  Entree: grey_number : entier compris entre 0 et _nb_grey_scales_
  Sortie: numero correspondant a l'intensite du niveau de gris 'grey_number'
****************************************************************************/
unsigned long
get_grey_intensity_ (int grey_number)
{
  if (grey_number <= 0)
    return _grey_scales_[0];
  if (grey_number >= _nb_grey_scales_)
    return _grey_scales_[_nb_grey_scales_ - 1];
  return _grey_scales_[grey_number];
}

/****************************************************************************
  Entree: color_number : entier compris entre 0 et _nb_colors_
  Sortie: numero correspondant a l'intensite de la couleur 'color_number'
****************************************************************************/
unsigned long
get_color_intensity_ (int color_number)
{
  if (color_number <= 0)
    return _colors_[0];
  if (color_number >= _nb_colors_)
    return _colors_[_nb_colors_ - 1];
  return _colors_[color_number];
}

/****************************************************************************/
/*--------------------------------------------------------------------------
  Entree: Une chaine de caractere representant une couleur
  Sortie: Pixel le plus proche trouve.
  --------------------------------------------------------------------------*/
/****************************************************************************/
unsigned long
get_color_by_name_ (Tk_Window tkwin, char *name)
{
  XColor thecolor;
  /*XColor c1,c2;*/

  thecolor = *Tk_GetColor(NULL,tkwin,name);
  /*XAllocNamedColor(theDisplay,theColormap,name,&c1,&c2);*/

  if (&thecolor) {
    return thecolor.pixel;
  } else {
    return -1;
  }
  /*if (&c1) {
    return c1.pixel;
  } else {
    return -1;
    }*/
}

/***************************************************************************/
/*---------------------------------------------------------------------------
  Sortie : nombre de couleurs disponibles dans la palette
  --------------------------------------------------------------------------*/
/***************************************************************************/
int
get_number_of_colors_ ()
{
  return _nb_colors_;
}

/***************************************************************************/
/*---------------------------------------------------------------------------
  Sortie : nombre de couleurs disponibles dans la palette
  --------------------------------------------------------------------------*/
/***************************************************************************/
int
get_depth_ ()
{
  return theDepth;
}

/****************************************************************************/
/*----------------------------------------------------------------------------
  ViewColBwGetColorNumber_
  
  Sortie : nombre de niveaux de gris disponibles dans la palette
  --------------------------------------------------------------------------*/
/****************************************************************************/
int
get_number_of_grey_scales_ ()
{
  return _nb_grey_scales_;
}


/********************************************/
/* From LASTWAVE                            */
/* Getting a colormap named "name"          */
/********************************************/
static int GetColorMap(char *name)
{
  int i;
  int n = nColorMaps;
  
  
  if (name == NULL) {
    if (theColorMaps[0] != NULL) return(0);
    return(-1);
  }

  if (*name == '_') printf("GetColorMap() : Weird error");
  
  for (i=1;i<ColorMapNMax && n != 0;i++) {
    if (theColorMaps[i] == NULL) continue; 
    if (!strcmp(name,theColorMaps[i]->name)) return(i);
    n--;
  }
  
  return(-1);
}

/***********************************************/
/* From LASTWAVE                               */
/* Modifying (if it exists) or creating a      */
/* colormap named "name" so that it will have  */
/* "nb" colors                                 */
/***********************************************/
static int CGetColorMap(char *name, int nb)
{
  int cm,i;
  
  if (nb <= 0) printf("CGetColorMap() : 'nb' should be positive");
  if (name != NULL && *name == '_') printf("CGetColorMap() : Sorry, you cannot create a color map whose name starts with a '_'");
  
  cm = GetColorMap(name);

  /* If Colormap already exists we delete it */
  if (cm != -1) {
    
    for (i=0;i<theColorMaps[cm]->size;i++) {
      if (theColorMaps[cm]->colors[i].name != NULL) {
        free(theColorMaps[cm]->colors[i].name);
        theColorMaps[cm]->colors[i].name = NULL;
      }
    }
    if (theColorMaps[cm]->colors != NULL) {
      free(theColorMaps[cm]->colors);
      theColorMaps[cm]->colors = NULL;
    }
  }
  
  /* Otherwise we must create the structure */
  else {
    for (i=0;i<ColorMapNMax;i++) {
      if (theColorMaps[i] == NULL) break; 
    }
    if (i == ColorMapNMax) printf("CGetColorMap() : Sorry, too many colormaps (maximum is %d)",ColorMapNMax);
    cm = i;
    theColorMaps[cm] = (COLORMAP) malloc(sizeof(struct colorMap));
    theColorMaps[cm]->colors = NULL;
    theColorMaps[cm]->size = 0;
    if (name != NULL) {
      /*theColorMaps[cm]->name = CopyStr(name);*/
      nColorMaps++;
    }
  }
  
  /* Then we allocate the array of colors */
  if (nb !=  0) {
    theColorMaps[cm]->colors = (Color *) malloc(sizeof(struct color)*nb);       
    theColorMaps[cm]->size = nb;
    for (i=0;i<nb;i++) {
      theColorMaps[cm]->colors[i].name = NULL;
      theColorMaps[cm]->colors[i].pixel = 0;
    }
  }

  return(cm);
}    





/****************************************************************************/
/*----------------------------------------------------------------------------
  Cree une colormap suivant style (couleur, nb) et demandant nbColReq 
  couleurs differentes. Le nombre de couleurs reellement alloue et renvoye.

  Pour l'instant, tres repompe sur Xlib Prog. Man, page 209
  (Nicolas : pour l'implementation sur hpcal, cad VisualClass = PSEUDOCOLOR)
  --------------------------------------------------------------------------*/
/****************************************************************************/

/* ***************************************************************************
 *  On teste la variable theVisualClass avant de faire 
 *  quoi que ce soit, le but final etant de remplir le tableau
 *  xColors !!!
 *****************************************************************************/

int 
_create_colormap_ (int nb_request_colors, int nb_request_grey, 
		   Tk_Window tkwin)
{
  unsigned long  planeMasks[1];
  XColor         tempColor;
  int            nb, i;
  unsigned short intensity;
  real           step;
  /*Status stat;*/

  myColormap = theColormap;


  /*
   * Case of read/write color cells : GrayScale, PseudoColor or DirectColor screens
   */
  if (theVisualClass == GrayScale || theVisualClass == PseudoColor || theVisualClass == DirectColor) {

    if ((_nb_colors_+_nb_grey_scales_) > 0)
      _delete_colormap_ ();
    
    _nb_colors_ = (nb_request_colors < MAX_COL_NUMBER
		   ? nb_request_colors
		   : MAX_COL_NUMBER);
    
    while(_nb_colors_ >= MIN_COL_NUMBER)
      {
	if (XAllocColorCells (theDisplay, myColormap, False, planeMasks, 0,
			      _colors_, _nb_colors_))
	  break;
	_nb_colors_--;
      }
    if (_nb_colors_ < MIN_COL_NUMBER) {
      printf ("xsmurf: color map error\n");
      printf ("problem with your X server configuration\n");
      printf ("xsmurf: use -nocolormap command line option to use xsmurf with no color map\n");
      printf ("        or try to reconfigure X server to allow TrueColor if possible.\n");
      exit(1);
    }
    
    _nb_grey_scales_ = (nb_request_grey < (MAX_COL_NUMBER - _nb_colors_)
		      ? nb_request_grey
			: (MAX_COL_NUMBER - _nb_colors_));
    
    while(_nb_grey_scales_ >= MIN_COL_NUMBER)
      {
	if (XAllocColorCells (theDisplay, myColormap, False, planeMasks, 0,
			      _grey_scales_, _nb_grey_scales_))
	  break;
      _nb_grey_scales_--;
      }
    if (_nb_colors_ < MIN_COL_NUMBER) {
      printf ("xsmurf: colormap error\n");
      exit(1);
  }
    
    /* il faudrait tester le resultat de l'allocation (>MIN_COL_NUMEBER) 
     * si inferieur a MIN_COL_NUMBER -> utiliser le noir et le blanc... */
  
    /* mine: noir et blanc avec des couleurs en plus. */
  
    /* creation d'une palette noir & blanc de _nb_grey_scales_ intensites */
    step = 45535.5 / ((real) (_nb_grey_scales_-1));
    for (nb = 0; nb < _nb_grey_scales_; nb++)
    {
      intensity = (int) (10000.0+step *((real) nb));
      tempColor.red = tempColor.green = tempColor.blue = intensity;
      tempColor.flags = DoRed | DoGreen | DoBlue;
      tempColor.pixel = _grey_scales_[nb];
      XStoreColor (theDisplay, myColormap, &tempColor);
    }
    /* un peu bourrin... */
    tempColor.red=65535.5;
    tempColor.green=65535.5;
    tempColor.blue=65535.5;
    tempColor.flags = DoRed | DoGreen | DoBlue;
    tempColor.pixel = _colors_[WHITE];
    XStoreColor(theDisplay, myColormap,&tempColor);
    tempColor.red=0;
    tempColor.green=0;
    tempColor.blue=0;
    tempColor.flags = DoRed | DoGreen | DoBlue;
    tempColor.pixel = _colors_[BLACK];
    XStoreColor(theDisplay, myColormap,&tempColor);
    tempColor.red=65535.5;
    tempColor.green=0;
    tempColor.blue=0;
    tempColor.flags = DoRed | DoGreen | DoBlue;
    tempColor.pixel = _colors_[RED];
    XStoreColor(theDisplay, myColormap,&tempColor);
    tempColor.red=0;
    tempColor.green=40000;
    tempColor.blue=55000;
    tempColor.flags = DoRed | DoGreen | DoBlue;
    tempColor.pixel = _colors_[BLUE];
    XStoreColor(theDisplay, myColormap,&tempColor);
    tempColor.red=0;
    tempColor.green=45535.5;
    tempColor.blue=0;
    tempColor.flags = DoRed | DoGreen | DoBlue;
    tempColor.pixel = _colors_[GREEN];
    XStoreColor(theDisplay, myColormap,&tempColor);
    tempColor.red=65535.5;
    tempColor.green=65535.5;
    tempColor.blue=0;
    tempColor.flags = DoRed | DoGreen | DoBlue;
    tempColor.pixel = _colors_[YELLOW];
    XStoreColor(theDisplay, myColormap,&tempColor);
    tempColor.red=17000;
    tempColor.green=17000;
    tempColor.blue=17000;
    tempColor.flags = DoRed | DoGreen | DoBlue;
    tempColor.pixel = _colors_[GREY1];
    XStoreColor(theDisplay, myColormap,&tempColor);
    tempColor.red=32000;
    tempColor.green=32000;
    tempColor.blue=32000;
    tempColor.flags = DoRed | DoGreen | DoBlue;
    tempColor.pixel = _colors_[GREY2];
    XStoreColor(theDisplay, myColormap,&tempColor);
    tempColor.red=52000;
    tempColor.green=52000;
    tempColor.blue=52000;
    tempColor.flags = DoRed | DoGreen | DoBlue;
    tempColor.pixel = _colors_[GREY3];
    XStoreColor(theDisplay, myColormap,&tempColor);

  }
  
  /*
   * Case of read-only color cells : StaticGray, StaticColor or TrueColor screens
   */

  else {

    _nb_grey_scales_ = nb_request_grey;
    _nb_colors_      = nb_request_colors;
    
    _colors_[WHITE]  = get_color_by_name_ (tkwin,"white");
    _colors_[BLACK]  = get_color_by_name_ (tkwin,"black");
    //_colors_[WHITE]  = get_color_by_name_ (tkwin,"black");
    //_colors_[BLACK]  = get_color_by_name_ (tkwin,"white");
    _colors_[RED]    = get_color_by_name_ (tkwin,"red");
    _colors_[BLUE]   = get_color_by_name_ (tkwin,"DeepSkyBlue");
    _colors_[YELLOW] = get_color_by_name_ (tkwin,"gold");
    _colors_[GREEN]  = get_color_by_name_ (tkwin,"LawnGreen");
    _colors_[GREY1]  = get_color_by_name_ (tkwin,"DimGrey");
    _colors_[GREY2]  = get_color_by_name_ (tkwin,"grey");
    _colors_[GREY3]  = get_color_by_name_ (tkwin,"LightGray");

    /* now grey levelzzz */
    for (i=0;i<nb_request_grey;i++) {
      char nom[]="grey00";
      
      if(i<10){
	nom[4] =  i +  '0';
	nom[5] = '\0';
      } else {
	int j;
	j = i / 10;
	nom[4] = j + '0';
	j = i % 10;
	nom[5] = j+ '0';
	}
      _grey_scales_[i]  = get_color_by_name_ (tkwin,nom);
      //_grey_scales_[nb_request_grey-1-i]  = get_color_by_name_ (tkwin,nom);
    }
    planeMask = 0;
  }
  
  return _nb_colors_+_nb_grey_scales_;
}

/****************************************************************************/
/*----------------------------------------------------------------------------
  Cree une colormap suivant style (couleur, nb) et demandant nbColReq 
  couleurs differentes. Le nombre de couleurs reellement alloue et renvoye.

  Pour l'instant, tres repompe sur Xlib Prog. Man, page 209
  --------------------------------------------------------------------------*/
/****************************************************************************/
int 
_false_color_colormap_ (int nb_request_colors,
			int nb_request_grey)
{
  unsigned long  planeMasks[1];
  XColor         tempColor;
  int            nb;
  real          step;

  if ((_nb_colors_) > 0)
    _delete_colormap_ ();

  _nb_colors_ = (nb_request_colors < MAX_COL_NUMBER
		 ? nb_request_colors
		 : MAX_COL_NUMBER);

  while(_nb_colors_ >= MIN_COL_NUMBER)
    {
      if (XAllocColorCells (theDisplay, myColormap, False, planeMasks, 0,
			    _colors_, _nb_colors_))
	break;
      _nb_colors_--;
    }

  _nb_grey_scales_ = (nb_request_grey < (MAX_COL_NUMBER - _nb_colors_)
		      ? nb_request_grey
		      : (MAX_COL_NUMBER - _nb_colors_));

  while(_nb_grey_scales_ >= MIN_COL_NUMBER)
    {
      if (XAllocColorCells (theDisplay, myColormap, False, planeMasks, 0,
			    _grey_scales_, _nb_grey_scales_))
	break;
      _nb_grey_scales_--;
    }

  /* il faudrait tester le resultat de l'allocation (>MIN_COL_NUMEBER) 
   * si inferieur a MIN_COL_NUMBER -> utiliser le noir et le blanc... */
  
  /* mine: noir et blanc avec des couleurs en plus. */
  
  /* creation d'une palette noir & blanc de _nb_grey_scales_ intensites */
/*  step = 32767.5 / ((float) (_nb_colors_-1));
  for (nb=0;nb<_nb_grey_scales_;nb++)
    {
      float L = ((float)nb)/((float)_nb_grey_scales_);
      float f = 6.0;
      tempColor.red=(L<=.5)?(int)(65535.5*2.0*(.5-L)):0;
      tempColor.green=(L<=.5)?(int)(65535.5*2.0*L):(int)(65535.5*2.0*(1.0-L));
      tempColor.blue=(L<=.5)?0:(int)(65535.5*2.0*(L-0.5));
      tempColor.flags = DoRed | DoGreen | DoBlue;
      tempColor.pixel = _grey_scales_[_nb_grey_scales_-nb-1];
      XStoreColor(theDisplay, myColormap,&tempColor);
    }*/

      step = 32767.5 / ((float) (_nb_grey_scales_-1));
      for (nb=0;nb<_nb_grey_scales_;nb++)
	{
	  float L = 4.0*((float)nb)/((float)_nb_grey_scales_);
	  if (L<1.0)
	    {
	      tempColor.red=65535;
	      tempColor.green=(int)(L*65535.5);
	      tempColor.blue=0;
	    }
	  else if (L<2.0)
	    {
	      tempColor.red=(int)((2.0-L)*65535.5);
	      tempColor.green=65535.5;
	      tempColor.blue=0;
	    }
	  else if (L < 3.0)
	    {
	      tempColor.red=0;
	      tempColor.green=(int)((3.0-L)*65535.5);
	      tempColor.blue=(int)((L-2.0)*65535.5);
	    }
	  else
	    {
	      tempColor.red=0.0;
	      tempColor.green=0.0;
	      tempColor.blue=(int)((4.0-L)*65535.5);
	    }
	  tempColor.flags = DoRed | DoGreen | DoBlue;
	  tempColor.pixel = _grey_scales_[_nb_grey_scales_-nb-1];
	  XStoreColor(theDisplay, myColormap,&tempColor);
	}

  return _nb_colors_;
}

/****************************************************************************/
/*----------------------------------------------------------------------------
  see function BuildColormap in lastwave/kernel/src/color.c
  --------------------------------------------------------------------------*/
/****************************************************************************/
void
init_colormap_ (Tk_Window tkwin)
{
  int nColors,ncm;

  CGetColorMap(NULL,256);

  nColors = 0;
  ncm = nColorMaps+1;
  

  theVisual      = Tk_Visual  (tkwin);
  theVisualClass = theVisual->class;
  /* variable theVisualClass is then one of these 6 possibilities:
     StaticGray, GrayScale, StaticColor, PseudoColor, TrueColor or DirectColor
     see in /usr/X11R6/include/X11/X.h for more information
  */

  theDisplay     = Tk_Display (tkwin);
  theScreen      = Tk_ScreenNumber (tkwin);
  theColormap    = Tk_Colormap (tkwin);
  theDepth       = Tk_Depth (tkwin);
    
  myColormap     = theColormap;
  _create_colormap_ (9, 100, tkwin);

}

/****************************************************************************/
/*----------------------------------------------------------------------------
  --------------------------------------------------------------------------*/
/****************************************************************************/
int
set_colormap_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp, 
		      int        argc,
		      char       *argv[])
{
  char * options[] = { "",
			 "-grey", "d",
			 "-colors", "d",
			 "-false", "d",
			 NULL };
  
  int  nb_request_colors = _nb_colors_;
  int  nb_request_grey   = _nb_grey_scales_;
  
  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get (1, &nb_request_colors)== TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (2, &nb_request_grey) == TCL_ERROR)
    return TCL_ERROR;

  /* crade... */
  if (arg_present (3))
    if (arg_get (3, &nb_request_colors) == TCL_ERROR)
      return TCL_ERROR;
    else
      {
	_false_color_colormap_ (_nb_colors_, nb_request_colors);
      }
  else
    /*_create_colormap_ (nb_request_colors, nb_request_grey);*/

  sprintf (interp->result, "%d,%d", _nb_colors_, _nb_grey_scales_);
  return TCL_OK;
}

/****************************************************************************/
/*----------------------------------------------------------------------------
  ViewColLinConvertToPixel_
  
  Entree: value : un reel correspondant a l'intensite d'un point sur une image
  Sortie: Pixel correspondant, etant donne la colormap, le min et le max de
  l'image. La conversion est lineaire.
  --------------------------------------------------------------------------*/
/****************************************************************************/
unsigned long ViewColLinConvertToPixel_(real value)
{
  return get_grey_intensity_((int)(((real)_nb_grey_scales_-.5)*
				   (value+_offsetLin_)/_amplitudeLin_));
}

/****************************************************************************/
/*----------------------------------------------------------------------------
  ViewColLogConvertToPixel_
  
  Entree: value : un reel correspondant a l'intensite d'un point sur une image
  Sortie: Pixel correspondant, etant donne la colormap, le min et le max de
  l'image. La conversion est logarithmique.
  --------------------------------------------------------------------------*/
/****************************************************************************/
unsigned long ViewColLogConvertToPixel_(real value)
{
  return get_grey_intensity_((int)(((real)_nb_grey_scales_-.5)*
				    log(value+_offsetLog_)/_amplitudeLog_));
}

/****************************************************************************/
/*----------------------------------------------------------------------------
  ViewColDisConvertToPixel_
  
  Entree: value : un reel correspondant a l'intensite d'un point sur une image
  Sortie: Pixel correspondant, etant donne la colormap, le min et le max de
  l'image. La conversion est se fait sur deux niveaux.
  --------------------------------------------------------------------------*/
/****************************************************************************/
unsigned long ViewColDisConvertToPixel_(real value)
{
/*  if(value>0)
    return RED;
  else
    return GREEN;*/
  return _highlightPixel_;
}

/****************************************************************************/
/*----------------------------------------------------------------------------
  ViewColConvertToPixelInit_
  
  Initialisation pour la procedure de conversion reel/Pixel.
  On specifie le min et le max de l'image. Attention, les valeurs doivent
  etre correctes car les fonction ConvertToPixel ne font pas de verification.
  La fonction renvoie egalement un booleen a vrai quand l'amplitude 
  min-max est suffisante pour pouvoir discerner les couleurs a la precision
  de la machine (faux qd min egale max ou en est tres proche)
  --------------------------------------------------------------------------*/
/****************************************************************************/
int ViewColConvertToPixelInit_(real min,real max)
{
  _offsetLog_     = 1.0 - min;
  _amplitudeLog_  = log(max+_offsetLog_);
  _offsetLin_     = - min;
  _amplitudeLin_  = (max-min);
  _highlightPixel_= get_grey_intensity_ (_nb_grey_scales_);
  return (((real) _amplitudeLin_) / ((real) _nb_grey_scales_) > 0.0) &&
    (((real) _amplitudeLog_) / ((real) _nb_grey_scales_) > 0.0);
}

/****************************************************************************/
/*----------------------------------------------------------------------------
  ViewColConvertHighlight_
  
  Cette procedure diminue l'amplitude des couleurs choisie par la procedure
  de conversion. Le resultat est une image plus sombre d'un facteur f;
  --------------------------------------------------------------------------*/
/****************************************************************************/
void ViewColConvertHighlight_(real f)
{
  _amplitudeLog_ *= f;
  _amplitudeLin_ *= f;
  _highlightPixel_=  get_grey_intensity_ ( (int) ((real) _nb_grey_scales_- 0.5)/f);
}

