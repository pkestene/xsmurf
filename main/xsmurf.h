#ifndef __XSMURF_H__
#define __XSMURF_H__

#include "smurf.h"

/*#ifndef __COLORMAP2_H__
#define __COLORMAP2_H__
*/
/*#include "ViewerLib.h"
*/
#define MAX_COL_NUMBER 256
#define MIN_COL_NUMBER   2

enum {WHITE, BLACK, RED, BLUE, YELLOW, GREEN, GREY1, GREY2, GREY3};
//enum {BLACK, WHITE, RED, BLUE, YELLOW, GREEN, GREY1, GREY2, GREY3};

/*int  ViewColGetPixel_            (int);*/
int  ViewColBwGetPixel_          (int);
int  ViewColNoBwGetPixel_        (int);
int  ViewColGetPixelByName_      (char *);
int  get_number_of_colors_       ();
int  get_number_of_grey_scales_  ();
int  ViewColGetNumberOfColors_   ();
int  ViewColBwGetNumberOfColors_ ();
/*int  ViewColGetForegroundPixel_  ();*/
int  ViewColGetBackgroundPixel_  ();
int  set_colormap_TclCmd_         (ClientData,Tcl_Interp *,int,char**);
void init_colormap_                (Tk_Window);

int  ViewColConvertToPixelInit_  (real,real);
int  ViewColLogConvertToPixel_   (real);
int  ViewColLinConvertToPixel_   (real);
int  ViewColDisConvertToPixel_   (real);
void ViewColConvertHighlight_    (real);

/*#endif  __COLORMAP2_H__*/






/*#ifndef __VL_IMAGE_H__
#define __VL_IMAGE_H__
*/
/*#include "Colormap.h"
*/
/*----------------------------------------------------------------------------
  Definition d'une image
  --------------------------------------------------------------------------*/
typedef struct
{
  int lx,ly;
  char * data;
} ViewImage;

/*----------------------------------------------------------------------------
  Procedures publiques du module
  --------------------------------------------------------------------------*/
void        ViewImaDelete_        (ViewImage *);/* destructeur de ViewImage */
ViewImage * ViewImaCreate_        (int,int);
ViewImage * ViewImaCreateFromData_(int,int,char *);
void        ViewImaClear_         (ViewImage *);

/*#endif __VL_IMAGE_H__*/

/*#ifndef __SM_VIEWERLIB_H__
#define __SM_VIEWERLIB_H__
*/
#ifdef HP
#include "tkConfig.h"
#endif

int ViewerLibInit (Tcl_Interp *, Tk_Window);

/*#endif __SM_VIEWERLIB_H__*/

/*#ifndef __WIDGET_H__
#define __WIDGET_H__*/

/*#include "ViewerLib.h"

#include "Dictionary.h"
#include "Colormap.h"
*/
/*----------------------------------------------------------------------------
  Widget viewer:
  Permettra, si tout se passe bien, d'afficher des images et des graphes.
  --------------------------------------------------------------------------*/
typedef struct 
{
  Tk_Window tkwin;		/* Fenetre qui contient la widget viewer.
				   NULL si la fenetre a ete detruite mais pas
				   encore la widget. */
  Display *display;		/* affichage sur lequel on travaille */
  Tcl_Interp *interp;		/* Interpreteur associe a la widget */
  GC gc;			/* Contexte graphique dont on se serait passe
				   mais qui est necessaire a qq fonctions X */

  int updatePending;		/* Si vrai, un reaffichage de la widget est
				   deja prevu */
 
  int sizex,sizey;		/* dimensions de la fenetre */
  char name[100];		/* nom de l'objet affiche
				   pour l'instant, tableau statique bovin */
  
  XImage * ximage;		/* XImage representant l'image */
  
} Viewer;

/* implementation d'une liste chainee memorisant toutes les widgets pour
   pouvoir toutes les parcourir rapidement */
typedef struct ViewerList
{
  struct ViewerList * next;
  Viewer * content;
} ViewerList;

/*void      ViewWidReconfigureall_ (char *);
int       ViewWidCmd_            (ClientData,Tcl_Interp*,int,char**);
*/
/*#endif*/


#endif /* __XSMURF_H__ */
