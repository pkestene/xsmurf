#include <string.h>
#include <stdlib.h>
#include "Dictionary.h"
#include "../interpreter/arguments.h"
#include "../interpreter/hash_tables.h"

static Tcl_HashTable _name_table_;

/*------------------------------------------------------------------------
  ViewDicInit
  
  A appeler avant tout pour initialiser la table de hachage, etc...
  ----------------------------------------------------------------------*/
void ViewDicInit_()
{
  Tcl_InitHashTable(&_name_table_,TCL_STRING_KEYS);
}

/*------------------------------------------------------------------------
  ViewDicGet_
  
  Recupere un pointeur sur un objet stocke sous le nom 'name' dans
  le dictionnaire. Renvoie NULL si inexistant.
  ----------------------------------------------------------------------*/
ViewImage *
ViewDicGet_(char * name)
{
  Tcl_HashEntry * tclHashEntryPtr;
  
  tclHashEntryPtr = Tcl_FindHashEntry(&_name_table_,name);
  if (tclHashEntryPtr)
    return  (ViewImage *) Tcl_GetHashValue(tclHashEntryPtr);
  else
    return NULL;
}

/*------------------------------------------------------------------------
  ViewDicRemove_
  
  Detruit la reference 'name' dans le dictionnaire. Libere l'objet image.
  ----------------------------------------------------------------------*/
void ViewDicRemove_(char * name)
{
  Tcl_HashEntry * tclHashEntryPtr;
  
  tclHashEntryPtr = Tcl_FindHashEntry(&_name_table_,name);
  if (tclHashEntryPtr)
    {
      ViewImaDelete_( (ViewImage *) Tcl_GetHashValue(tclHashEntryPtr) );
      Tcl_DeleteHashEntry(tclHashEntryPtr);
      
      ViewWidReconfigureall_(name);
    }
}

/*------------------------------------------------------------------------
  ViewDicStore_
  
  Sauvegarde une image dans le dictionnaire
  
  Entree: 
  name  : nom sous lequel on sauvegarde (recopie par la procedure) 
  image : pointeur sur l'image a sauvegarder 
  
  Sortie: 
  ------------------------------------------------------------------------*/
void ViewDicStore_(char * name, ViewImage * image)
{
  int new;
  Tcl_HashEntry * tclHashEntryPtr;
  
  tclHashEntryPtr = Tcl_CreateHashEntry(&_name_table_,name,&new);
  
  /* si il existe deja un objet sous ce nom dans le dictionnaire 
   * on le detruit impitoyablement */
  
  if (!new)
    ViewImaDelete_((ViewImage *) Tcl_GetHashValue(tclHashEntryPtr));
  
  Tcl_SetHashValue(tclHashEntryPtr, image);
  
  if (!new)
    ViewWidReconfigureall_(name);
}

/*------------------------------------------------------------------------
  ViewDicTclGet

  Cette commande tente de recuperer dans le dictionnaire la ViewImage
  "nom".  En cas d'echec, elle envoie un message d'erreur a
  l'interpreteur Tcl et renvoie TCL_ERROR.
  ----------------------------------------------------------------------*/
int
ViewDicTclGet(Tcl_Interp *interp,
	      char       *name,
	      ViewImage  ** viewImagePtr)
{
  (*viewImagePtr) = ViewDicGet_(name);
  if (*viewImagePtr)
    return TCL_OK;
  Tcl_AppendResult(interp, "Couldn't find ViewImage `",name, "'.",NULL);
  
  return TCL_ERROR;
}

/*------------------------------------------------------------------------
  ViewDicInfoCmd_
  
  Renvoie des informations sur les ViewImages specifiees
  ----------------------------------------------------------------------*/  
int ViewDicInfoCmd_(ClientData clientData,Tcl_Interp *interp,
		    int argc,char **argv)      
{
  char * options[] = { "L" ,
			 "-all","",
			 "-size","",
			 NULL };

  int              j,size;
  Tcl_HashEntry ** list;
  ViewImage      * viewImagePtr;
  char          ** expr, buffer[100],element[100];
  
  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;
  
  if (arg_get(0,&expr)==TCL_ERROR)
    return TCL_ERROR;
  
  size = arg_present(2);

  if (arg_present(1)) /* all */
    size=1;

  if (GenDicGetUniqueEntryList(interp,&_name_table_,expr,&list)==TCL_ERROR)
    return TCL_ERROR; 
  
  if (list)
    {
      for (j=0;list[j];j++)
	{
	  viewImagePtr = Tcl_GetHashValue(list[j]);
	  sprintf(element,"%s",Tcl_GetHashKey(&_name_table_,list[j]));
	  if (size)
	    {
	      sprintf(buffer," %d %d",viewImagePtr->lx,viewImagePtr->ly);
	      strcat(element,buffer);
	    }
	  Tcl_AppendElement(interp,element);
	}
      free(list);
    }
  return TCL_OK;
} 

/*------------------------------------------------------------------------
  ViewDicDeleteCmd_
  
  Detruit les ViewImages specifiees.
  ----------------------------------------------------------------------*/  
int ViewDicDeleteCmd_(ClientData clientData,Tcl_Interp *interp,
		      int argc,char **argv)      
{
  char * options[] = { "L" ,NULL };
  int              j;
  Tcl_HashEntry ** list;
  char          ** expr;
  
  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;
  
  if (arg_get(0,&expr)==TCL_ERROR)
    return TCL_ERROR;
  
  if (GenDicGetUniqueEntryList(interp,&_name_table_,expr,&list)==TCL_ERROR)
    return TCL_ERROR;
  
  if (!list)
    return GenErrorAppend(interp,"No matching ViewImage.",NULL);
  
  for (j=0;list[j];j++)
    ViewDicRemove_(Tcl_GetHashKey(&_name_table_,list[j]));
  
  free(list);
    
  return TCL_OK;
} 






