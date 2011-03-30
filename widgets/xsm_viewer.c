#include "X11/X.h"
/*#include "Xutil.h"*/
#include "stdlib.h"
#include "Widget.h"
#include "../interpreter/arguments.h"
#include "Colormap.h"

/*============================================================================
  Procedures externes au module
  ==========================================================================*/
extern int get_depth_ ();  /* definie dans Colormap.c */

/*============================================================================
  Procedures locales au module
  ==========================================================================*/
static void _ViewerConfigure_     (Viewer*);
       void _ViewerDestroy_          (char *);
static void _ViewerDisplay_       (ClientData);
static void _ViewerEventProc_     (ClientData,XEvent*);
static int  _ViewerWidgetCmd_     (ClientData,Tcl_Interp *,int,char**);
static void _ViewerSetContent_    (Viewer*,char*);
static void _DeleteCurrentContent_(Viewer*);
static void _AddViewerToList_     (Viewer*);
static void _RemoveViewerFromList_(Viewer*);

/*============================================================================
  Variables locales au module
  ==========================================================================*/
static ViewerList * _viewerListFirst_ = NULL;

/* pour virer un sal warning qui fait chier */
void XDestroyImage(XImage *ximage);

/*----------------------------------------------------------------------------
  ViewWidCmd_
  
  Cette procedure est appelee depuis l'interpreteur de commande tcl.
  Elle cree une instance de Viewer et rajoute son nom dans la table
  des commandes.
  --------------------------------------------------------------------------*/
int ViewWidCmd_(ClientData clientData,Tcl_Interp * interp,
		int argc,char ** argv)
{
  char  * options[] = { "s" , NULL };

  Tk_Window themain = (Tk_Window) clientData;
  Tk_Window tkwin;
  Viewer *viewerPtr;
  char * viewerName;
 
  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;
  
  if (arg_get(0,&viewerName) == TCL_ERROR)
    return TCL_ERROR;
  
  tkwin = Tk_CreateWindowFromPath(interp, themain, viewerName, (char *) NULL);
  
  if (tkwin == NULL) 
    return TCL_ERROR;
  
  Tk_SetClass(tkwin, "Viewer");
  
  /*
   * Allocate and initialize the widget record.
   */
  
  viewerPtr                 = (Viewer *) ckalloc(sizeof(Viewer));
  viewerPtr->tkwin          = tkwin;
  viewerPtr->display        = Tk_Display(tkwin);
  viewerPtr->interp         = interp;
  viewerPtr->gc             = None;
  viewerPtr->updatePending  = 0;
  viewerPtr->sizex          = 0;
  viewerPtr->sizey          = 0;
  viewerPtr->ximage         = NULL;
 
  Tk_CreateEventHandler(viewerPtr->tkwin, ExposureMask|StructureNotifyMask,
			_ViewerEventProc_, (ClientData) viewerPtr);
  
  Tcl_CreateCommand(interp, Tk_PathName(viewerPtr->tkwin), (Tcl_CmdProc*)_ViewerWidgetCmd_,
		    (ClientData) viewerPtr, (void (*)()) NULL);
  
  
  {
    XGCValues gcValues;  
    
    gcValues.function = GXcopy;
    gcValues.graphics_exposures = False;
    viewerPtr->gc = Tk_GetGC(viewerPtr->tkwin,
			     GCFunction|GCGraphicsExposures, &gcValues);
  }
  
  _AddViewerToList_(viewerPtr);
  _ViewerSetContent_(viewerPtr,"");
   
  _ViewerConfigure_(viewerPtr);

  interp->result = Tk_PathName(viewerPtr->tkwin);
  return TCL_OK; 
}

/*----------------------------------------------------------------------------
  _DeleteCurrentContent_

  Procedure locale appelee avant une reconfiguration ou une destruction
  de la widget et qui libere la structure ximage
  ---------------------------------------------------------------------------*/
static void _DeleteCurrentContent_(Viewer * viewerPtr)
{
  if (viewerPtr->ximage)
    {
      /* necessaire avant appel a XDestroyImage */
      viewerPtr->ximage->data = NULL; 
      
      XDestroyImage(viewerPtr->ximage);
      viewerPtr->ximage       = NULL;
    }
}

/*----------------------------------------------------------------------------
  _ViewerSetContent_
  
  Fixe le contenu de la widget. On passe le nom d'un objet affichable
  qui est cherche dans le dictionnaire defini dans ViewerDic. L'objet
  trouve fixe le type de la widget. Par defaut, un bitmap sera affiche.
  La valeur de retour est
  ---------------------------------------------------------------------------*/
static void _ViewerSetContent_(Viewer * viewerPtr,char * name)
{
  ViewImage       * viewImagePtr;
  Tk_Window tkwin = viewerPtr->tkwin;
  
  _DeleteCurrentContent_(viewerPtr); /* efface la structure actuelle */
  
  viewImagePtr = ViewDicGet_(name);
  if (viewImagePtr)
    {
      char          * thedata;
      unsigned long * data;
      int             i;
      int             shift;
      unsigned long   pixel;
      int             thedepth = get_depth_ ();
      int             bitmap_pad;
      int             bytes_per_line;

      if (thedepth == 8) {              /* mode 256 couleurs */
	thedata = (char *) malloc(viewImagePtr->lx*viewImagePtr->ly*sizeof(char));
	data = viewImagePtr->data;
	bitmap_pad = 8;
	bytes_per_line = viewImagePtr->lx;

	for (i=0; i<viewImagePtr->lx*viewImagePtr->ly;i++) {
	  shift = 2*i;
	  * (thedata + shift) = (char) data[i];
	}

      } else if (thedepth == 16) {      /* mode 65000 couleurs */	

	thedata = (char *) malloc(2*viewImagePtr->lx*viewImagePtr->ly*sizeof(char));
	data = viewImagePtr->data;
	
	bitmap_pad = 16;
	bytes_per_line = 0; /* en fait je ne sais ce qu'il faut mettre */

	for (i=0; i<viewImagePtr->lx*viewImagePtr->ly;i++) {
	  pixel = data[i];
	  shift = 2*i;
	  if (ImageByteOrder(viewerPtr->display)==LSBFirst) {
	    *(thedata+shift+1) = (pixel & 0xFF00)>>8;  
	    *(thedata+shift)   =  pixel & 0xFF;  
	  } 
	  else {
	    *(thedata+shift)   = (pixel & 0xFF00)>>8;
	    *(thedata+shift+1) = (pixel & 0xFF);  
	  }
	}
      } else if (thedepth == 24) {
    	thedata = (char *) malloc(4*viewImagePtr->lx*viewImagePtr->ly*sizeof(char));
	data = viewImagePtr->data;
	
	bitmap_pad = 32;
	bytes_per_line = 0; 
	
	for (i=0; i<viewImagePtr->lx*viewImagePtr->ly;i++) {
	  pixel = data[i];
	  shift = 4*i;
	  if (ImageByteOrder(viewerPtr->display)==LSBFirst) {
	    *(thedata+shift+3) = (pixel & 0xFF000000)>>24;
	    *(thedata+shift+2) = (pixel & 0xFF0000)>>16;  
	    *(thedata+shift+1) = (pixel & 0xFF00)>>8;  
	    *(thedata+shift)   =  pixel & 0xFF;  
	  } 
	  else {
	    *(thedata+shift)   = (pixel & 0xFF000000)>>24;
	    *(thedata+shift+1) = (pixel & 0xFF0000)>>16;  
	    *(thedata+shift+2) = (pixel & 0xFF00)>>8;  
	    *(thedata+shift+3) =  pixel & 0xFF;  
	  }
	}
      } else {
	printf ("xsmurf: color map error\n");
	printf ("problem with your X server configuration\n");
	printf ("xsmurf: use -nocolormap command line option to use xsmurf with no color map\n");
	printf ("        or try to reconfigure X server to allow TrueColor if possible.\n");
	printf ("\n");
	printf ("_ViewerSetContent_ : Sorry this function has not been impleted for screen depth %d",thedepth);
	return;
      }
      
      strcpy(viewerPtr->name,name);
      
      viewerPtr->sizex  = viewImagePtr->lx;
      viewerPtr->sizey  = viewImagePtr->ly;
      viewerPtr->ximage = XCreateImage
	(viewerPtr->display,		/* display */
	 Tk_Visual(tkwin),	        /* visual */
	 get_depth_ (),		       	/* depth */
	 ZPixmap,        	       	/* type */
	 0,				/* offset */
	 thedata,                       /* data */
	 viewImagePtr->lx,		/* width */
	 viewImagePtr->ly,		/* height */
	 bitmap_pad,		       	/* bitmap pad */
	 bytes_per_line);       	/* length of line*/   
      return;
    }
  
  /* contenu `vide' */
  strcpy(viewerPtr->name,"");
  viewerPtr->sizex  = 0;
  viewerPtr->sizey  = 0;
  
  return ;
}

/*----------------------------------------------------------------------------
  _ViewerWidgetCmd_
  
  Cette commande porte le nom de la widget Viewer creee. Elle permet
  d'en fixer le contenu ou de lire le nom de son contenu.

  C'est LA commande principale utilisee dans ImageViewer.tcl
  Son nom dans l'interpreteur depend de la fenetre graphique consideree.
  --------------------------------------------------------------------------*/
static int _ViewerWidgetCmd_(ClientData clientData,Tcl_Interp * interp,
			     int argc,char ** argv)
{
  char  * options[] = { "",
			  "content","[s]",
			  NULL };
  
  Viewer    * viewerPtr     = (Viewer *) clientData;
  char      * viewImageName = NULL;

  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;
  
  Tk_Preserve((ClientData) viewerPtr);
  
  /* specification ou demande du contenu de la widget */
  if (arg_present(1))
    {
      if (arg_get(1,&viewImageName)==TCL_ERROR)
	goto error;
      
      if (viewImageName)
	_ViewerSetContent_(viewerPtr,viewImageName);
      
      Tcl_AppendResult(interp,viewerPtr->name,NULL);
      
      _ViewerConfigure_(viewerPtr);
    }
  
  _ViewerConfigure_(viewerPtr);
  
  Tk_Release((ClientData) viewerPtr);
  return TCL_OK;
  
 error:
  
  Tk_Release((ClientData) viewerPtr);
  return TCL_ERROR;
}

/*----------------------------------------------------------------------------
   _ViewerConfigure_ 

   Reconfiguration de la widget viewer. Appelee quand une modification
   visible du contenu de la widget a ete effectuee
   --------------------------------------------------------------------------*/
static void _ViewerConfigure_(Viewer * viewerPtr)
{ 
  Tk_GeometryRequest(viewerPtr->tkwin, 
		     viewerPtr->sizex,viewerPtr->sizey);  
  
  if (!viewerPtr->updatePending) 
    {
      Tk_DoWhenIdle(_ViewerDisplay_, (ClientData) viewerPtr);
      viewerPtr->updatePending = 1;
    }
}

/*----------------------------------------------------------------------------
  _ViewerEventProc_ 
  
  This procedure is invoked by the Tk dispatcher for various
  events on viewers.
  Side effects:
  When the window gets deleted, internal structures get
  cleaned up.  When it gets exposed, it is redisplayed.
  --------------------------------------------------------------------------*/
static void _ViewerEventProc_( ClientData clientData,XEvent *eventPtr)
{
  Viewer *viewerPtr = (Viewer *) clientData;
  
  if (eventPtr->type == Expose) 
    {
      if (!viewerPtr->updatePending) 
	{
	  Tk_DoWhenIdle(_ViewerDisplay_, (ClientData) viewerPtr);
	  viewerPtr->updatePending = 1;
	}
    } 
  else 
    if (eventPtr->type == ConfigureNotify) 
      {
	if (!viewerPtr->updatePending) 
	  {
	    Tk_DoWhenIdle(_ViewerDisplay_, (ClientData) viewerPtr);
	    viewerPtr->updatePending = 1;
	  }
	/*	printf ("hgkjhgjhgjkh %d\n", Tk_Width (viewerPtr -> tkwin));*/
      } 
    else 
      if (eventPtr->type == DestroyNotify) 
	{
	  Tcl_DeleteCommand(viewerPtr->interp, Tk_PathName(viewerPtr->tkwin));
	  viewerPtr->tkwin = NULL;
	  if (viewerPtr->updatePending) 
	    {
	      Tk_CancelIdleCall(_ViewerDisplay_, (ClientData) viewerPtr);
	    }
	  Tk_EventuallyFree((ClientData) viewerPtr, &_ViewerDestroy_);
	}
}


/*----------------------------------------------------------------------------
  _ViewerDisplay_ 
  
  Affiche le contenu de la widget. Appele uniquement qd le programme n'a
  rien a faire (do when idle).
  --------------------------------------------------------------------------*/
static void _ViewerDisplay_(ClientData clientData)
{
  Viewer *viewerPtr = (Viewer *) clientData;
  Tk_Window tkwin = viewerPtr->tkwin;
  
  viewerPtr->updatePending = 0;

  if ((viewerPtr->ximage) && (Tk_IsMapped(tkwin)))
    XPutImage(Tk_Display(tkwin), Tk_WindowId(tkwin), viewerPtr->gc,
	      viewerPtr->ximage,
	      0,0,     /* upper left corner of the image */
	      0,0,     /* where to copy the image in the window*/
	      viewerPtr->ximage->width,viewerPtr->ximage->height);
}

/*----------------------------------------------------------------------------
  _ViewerDestroy_
  
  This procedure is invoked by Tk_EventuallyFree or Tk_Release
  to clean up the internal structure of a viewer at a safe time
  (when no-one is using it anymore).
  Everything associated with the viewer is freed up.
  --------------------------------------------------------------------------*/
void _ViewerDestroy_(char * clientData)
{
  Viewer *viewerPtr = (Viewer *) clientData;
  
  if (viewerPtr->gc != None) 
    Tk_FreeGC(viewerPtr->display, viewerPtr->gc);

  _RemoveViewerFromList_(viewerPtr);
  _DeleteCurrentContent_(viewerPtr);
  
  ckfree((char *) viewerPtr);
}

/*----------------------------------------------------------------------------
  _AddViewerToList_

  on maintient une liste chainee de pointeurs sur toutes les instances
  de viewers existantes. Ceci permettra de les parcourir pour les
  remettre a jour .
  Cette procedure enregistre une instance de Viewer nouvellement creee.
  --------------------------------------------------------------------------*/
static void _AddViewerToList_(Viewer * viewer)
{
  ViewerList * new = (ViewerList *) ckalloc (sizeof(ViewerList));
  
  new->next         = _viewerListFirst_;
  _viewerListFirst_ = new;
  new->content      = viewer;
}

/*----------------------------------------------------------------------------
  _RemoveViewerFromList_

  A appeler lors de la destruction de l'instance de Viewer specifiee
  --------------------------------------------------------------------------*/
static void _RemoveViewerFromList_(Viewer * viewer)
{
  ViewerList * toBeDeleted;
  ViewerList * cursor      = _viewerListFirst_;
  
  if (cursor->content == viewer)
    {
      toBeDeleted = cursor;
      _viewerListFirst_ = cursor->next;
      ckfree((char *)toBeDeleted);
    }
  else
    while (cursor != NULL)
      if ((cursor->next) && (cursor->next->content == viewer ))
	{
	  toBeDeleted=cursor->next;
	  cursor->next = cursor->next->next;
	  ckfree((char *)toBeDeleted);
	  break;
	}
      else
	cursor=cursor->next;
}

/*----------------------------------------------------------------------------
  ViewWidReconfigureall
  
  Fonction publique, accessible a l'exterieur du module. Doit etre appelee
  lors de la modification d'un objet susceptible d'etre affiche par un
  Viewer. La procedure parcourt toutes les instances de Viewer et les
  reconfigure le cas echeant.
  --------------------------------------------------------------------------*/
void ViewWidReconfigureall_(char * viewImageName)
{
  ViewerList * cursor = _viewerListFirst_;
  
  while (cursor)
    {
      /* reconfigurer le viewer pointe par cursor si il contient une viewImage
	 qui est creee ou reaffectee */
      if (!strcmp(viewImageName,cursor->content->name))
	{
	  _ViewerSetContent_(cursor->content,viewImageName);
	  _ViewerConfigure_ (cursor->content);
	}
      cursor = cursor->next;
    }
}
