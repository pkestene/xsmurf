#ifndef __WIDGET_H__
#define __WIDGET_H__

#include "ViewerLib.h"

#include "Dictionary.h"
#include "Colormap.h"

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

void      ViewWidReconfigureall_ (char *);
int       ViewWidCmd_            (ClientData,Tcl_Interp*,int,char**);

#endif

