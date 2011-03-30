/*
 * hash_tables.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: hash_tables.c,v 1.10 1998/12/16 13:15:27 decoster Exp $
 */

#include <tcl.h>
#include <stdarg.h>
#include "../signal/signal.h"
#include "../image/image.h"
#include "../image/image3D.h"
#include "../wt3d/wt3d.h"
#include "../wt2d/wt2d.h"
#include "../wt1d/wt1d.h"
#include "arguments.h"
#include <string.h>
#include <stdlib.h>

/*
 * Smurf objects hash table.
 */

static Tcl_HashTable _smurf_object_hash_table_;

/*
 * Definition and manipulation of the values of the entries of
 * the general smurf object hash table.
 */

/* different type of smurf object */
enum {
  SM_SIGNAL,
  SM_IMAGE,
  SM_EXTIMAGE,
  SM_WAVELET,
  SM_LINE,
  SM_EXTIMAGE3D,
  SM_EXTIMAGE3Dsmall,
  SM_IMAGE3D
};

/* General value of the entries of the hash table */
struct _smurf_object_
{
  int  object_type;
  void *object_ptr;
  char *description;  /* used by the user */
};
typedef struct _smurf_object_ *_smurf_object_;

/*
 */
static _smurf_object_
_create_smurf_object_ (int object_type,
		       void *object_ptr,
		       const char *description)
{
  _smurf_object_ value;
  int length;

  value = (_smurf_object_) malloc (sizeof (struct _smurf_object_));
  length = strlen (description);
  value -> description = (char *) malloc (sizeof (char)*5);

  value -> object_type = object_type;
  value -> object_ptr = object_ptr;
  strcpy (value -> description, "none\0");

  return value;
}

/*
 * Memory free of the entry value and of the object associated to
 * this entry value.
 */
static void
_free_smurf_object_ (_smurf_object_ value)
{
  free (value -> description);
  switch (value -> object_type)
    {
    case SM_SIGNAL:
      sig_free ((Signal *) value -> object_ptr);
      break;
    case SM_IMAGE:
      im_free ((Image *) value -> object_ptr);
      break;
    case SM_EXTIMAGE:
      ExtImaDelete_ ((ExtImage *) value -> object_ptr);
      break;
    case SM_WAVELET:
      wt1d_free_wavelet ((Wavelet *) value -> object_ptr);
      break;
    case SM_LINE:
      /*      line = (Line *) value -> object_ptr;
	      if (!lst_remove (line, line -> ext_image -> line_lst))
	      printf("ararararararaaaaaaaarg !\n");
	      destroy_line (line);*/
      break;
    case SM_EXTIMAGE3Dsmall:
      ExtIma3DsmallDelete_ ((ExtImage3Dsmall *) value -> object_ptr);
      break;
    case SM_IMAGE3D:
      im3D_free ((Image3D *) value -> object_ptr);
      break;
    }
  free (value);
}

/*
 */
void
hash_tables_init ()
{
  Tcl_InitHashTable (&_smurf_object_hash_table_, TCL_STRING_KEYS);
}

/*
 * Create a new entry in the hash table. If there is an entry with the
 * same key, the old one is_replaced_ by the new one and return value
 * is non 0.
 */
int
store_smurf_object (char          *object_name,
		    _smurf_object_ value)
{
  int is_new;
  Tcl_HashEntry *tclHashEntryPtr;
  
  tclHashEntryPtr = Tcl_CreateHashEntry (&_smurf_object_hash_table_,
					 object_name,
					 &is_new);
  
  if (!is_new)
    _free_smurf_object_ ((_smurf_object_) Tcl_GetHashValue (tclHashEntryPtr));
  
  Tcl_SetHashValue (tclHashEntryPtr, value);

  return is_new;
}


/*
 *  Get th value of the entry which key is OBJECT_NAME.
 *  Return 0 if there is no entry with such a key.
 */
_smurf_object_
get_smurf_object (char *object_name)
{
  _smurf_object_ value;
  
  Tcl_HashEntry *tclHashEntryPtr;
  
  tclHashEntryPtr = Tcl_FindHashEntry (&_smurf_object_hash_table_, object_name);

  if (tclHashEntryPtr)
    {
      value = (_smurf_object_) Tcl_GetHashValue (tclHashEntryPtr);
      return value;
    }
  else
    return NULL;
}

/*
 */
void
delete_entry (char *object_name)
{
  Tcl_HashEntry *tclHashEntryPtr;
  
  tclHashEntryPtr = Tcl_FindHashEntry (&_smurf_object_hash_table_, object_name);
  if (tclHashEntryPtr)
    {
      _free_smurf_object_ ((_smurf_object_) Tcl_GetHashValue (tclHashEntryPtr));
      Tcl_DeleteHashEntry (tclHashEntryPtr);
    }
}

/*
 */
char* 
display_signal_info (Signal *signal,
		     char   *expr)
{
  char *type[] = {"REALXY", "REALY", "CPLX", "FOUR_NR"};

  sprintf(expr,
	  "-> first: %d\n"
	  "-> last : %d\n"
	  "-> size : %d\n"
	  "-> type : %s\n"
	  "-> dx   : %6f\n"
	  "-> x0   : %6f\n",
	 signal->first, signal->last, signal->size,
	 type[signal->type], signal -> dx, signal -> x0);

  return expr;
}

/*
 */
char* 
display_wavelet_info (Wavelet *wavelet,
		      char   *expr)
{
  char *type[] = {
    "real-real",
    "real-cplx",
    "cplx-real",
    "cplx-cplx",
  };

  sprintf(expr,	"%s", type[wt1d_wavelet_type (wavelet)]);

  return expr;
}

char* 
display_image_info (Image *image,
		    char  *expr)
{
  real min,max;
  im_get_extrema (image,&min,&max);

  if (image->type == PHYSICAL) {
    sprintf(expr,
	    "-> dim: %dx%d, size: %d, \t %s,\n border: hor=%d ver=%d,\n im_extrema\n min : %f , max : %f ",image->lx, image->ly, image->size,
	    "Physical", image->border_hor,image->border_ver, image->min, image->max);
  } else if (image->type == FOURIER) {
    sprintf(expr,
	    "-> dim: %dx%d, size: %d, \t %s,\n border: hor=%d ver=%d,\n im_extrema\n min : %f , max : %f ",image->lx, image->ly, image->size,
	    "Fourier", image->border_hor,image->border_ver, image->min, image->max);
  } else {
    sprintf(expr,
	    "-> dim: %dx%d, size: %d, \t %s,\n border: hor=%d ver=%d,\n im_extrema\n min : %f , max : %f ",image->lx, image->ly, image->size,
	    "FFTW_R2C", image->border_hor,image->border_ver, image->min, image->max);
  }

  return expr;
}

char* 
display_image3D_info (Image3D *image,
		      char  *expr)
{
  real min,max;
  im3D_get_extrema (image,&min,&max);

  if (image->type == PHYSICAL) {
    sprintf(expr,
	    "-> dim: %dx%dx%d, size: %d, type: PHYSICAL,  im3D_extrema\n min : %f , max : %f ",
	    image->lx, image->ly, image->lz, image->size, image->min, image->max);
  } else if (image->type == FFTW_R2C) {
    sprintf(expr,
	    "-> dim: %dx%dx%d, size: %d, type: FFTW_R2C,  im3D_extrema\n min : %f , max : %f ",
	    image->lx, image->ly, image->lz, image->size, image->min, image->max);  
  } else {
    sprintf(expr,
	    "Unknown type of image3D, check the code...");
  }
  
  return expr;
}

char* 
display_ext_image_info (ExtImage *ext_image,
			char     *expr)
{
  sprintf(expr,
	  "-> ech: %4.2f \tdim: %dx%d,\textr_nb: %d,\tch %d\t(%d)\n",
	 ext_image->scale,ext_image->lx,ext_image->ly,
	 ext_image->extrNb,ext_image->chainNb,ext_image->stamp);

  return expr;
}

char* 
display_ext_image_3Dsmall_info (ExtImage3Dsmall *ext_image3D,
				char     *expr)
{
  sprintf(expr,
	  "-> ech: %4.2f \tdim: %dx%dx%d,\textr_nb: %d\n",
	  ext_image3D->scale,ext_image3D->lx,ext_image3D->ly,ext_image3D->lz,
	  ext_image3D->extrNb);
  
  return expr;
}


/* Some general use functions. To be placed somewhere else */

int     GenDicGetUniqueEntryList (Tcl_Interp *,Tcl_HashTable *,
				  char **,Tcl_HashEntry ***);

int     GenErrorAppend           (Tcl_Interp *,...);
int     GenErrorMemoryAlloc      (Tcl_Interp *);

/*----------------------------------------------------------------------
  GenDicGetUniqueEntryList
  
  Cree une liste de HashEntries dont le mot-clef verifie l'une des
  expressions passees en argument. Le tableau expr doit avoir NULL
  comme dernier element. Les HashEntries retournees dans la listes
  seront uniques.
  list = NULL renvoye si liste vide.
  Retourne TCL_OK en cas de succes, TCL_ERROR si probleme d'allocation
  --------------------------------------------------------------------*/
int
GenDicGetUniqueEntryList(Tcl_Interp    * interp,
			 Tcl_HashTable *tablePtr,
			 char          ** expr,
			 Tcl_HashEntry *** list)
{  
  int             n,m,numberFound = 0;
  char          * entryName;
  Tcl_HashSearch  hashSearch;
  Tcl_HashEntry * hashEntryPtr;
  
  /* un premier passage pour determiner le nombre d'elements */
  hashEntryPtr = Tcl_FirstHashEntry (tablePtr, &hashSearch);  
  while (hashEntryPtr)
    {
      entryName = Tcl_GetHashKey(tablePtr, hashEntryPtr);
      for (m=0;expr[m];m++)
	if (Tcl_StringMatch(entryName,expr[m]))
	  {
	    numberFound++;
	    break;
	  }
      hashEntryPtr = Tcl_NextHashEntry(&hashSearch);
    }
  
  if (numberFound == 0)
    {
      *list = NULL;
      return TCL_OK;
    }

  *list = (Tcl_HashEntry **) malloc (sizeof(Tcl_HashEntry*) * (numberFound+1));
  if (!(*list))
    return GenErrorMemoryAlloc(interp);
  
  (*list)[numberFound] = NULL;
  
  /* un deuxieme passage pour stocker les elements */
  hashEntryPtr = Tcl_FirstHashEntry (tablePtr, &hashSearch);  
  n=0;
  while (hashEntryPtr)
    {
      entryName = Tcl_GetHashKey(tablePtr, hashEntryPtr);
      for (m=0;expr[m];m++)
	if (Tcl_StringMatch(entryName,expr[m]))
	  {
	    (*list)[n++] = hashEntryPtr;
	    break;
	  }
      hashEntryPtr = Tcl_NextHashEntry( &hashSearch);
    }
  return TCL_OK;
}

/*----------------------------------------------------------------------
  GenDicGetUniqueEntryListByType
  
  Cree une liste de HashEntries dont le mot-clef verifie l'une des
  expressions passees en argument. Le tableau expr doit avoir NULL
  comme dernier element. Les HashEntries retournees dans la listes
  seront uniques.
  list = NULL renvoye si liste vide.
  Retourne TCL_OK en cas de succes, TCL_ERROR si probleme d'allocation
  --------------------------------------------------------------------*/
int
GenDicGetUniqueEntryListByType(Tcl_Interp    * interp,
			       Tcl_HashTable *tablePtr,
			       char          ** expr,
			       Tcl_HashEntry *** list,
			       int           object_type)
{  
  _smurf_object_ value;
  int             n,m,numberFound = 0;
  char          * entryName;
  Tcl_HashSearch  hashSearch;
  Tcl_HashEntry * hashEntryPtr;
  
  /* un premier passage pour determiner le nombre d'elements */
  hashEntryPtr = Tcl_FirstHashEntry (tablePtr, &hashSearch);  
  while (hashEntryPtr)
    {
      entryName = Tcl_GetHashKey(tablePtr, hashEntryPtr);
      value = (_smurf_object_) Tcl_GetHashValue(hashEntryPtr);
      for (m=0;expr[m];m++)
	if (Tcl_StringMatch(entryName,expr[m])
	    && (value -> object_type == object_type))
	  {
	    numberFound++;
	    break;
	  }
      hashEntryPtr = Tcl_NextHashEntry(&hashSearch);
    }
  
  if (numberFound == 0)
    {
      *list = NULL;
      return TCL_OK;
    }

  *list = (Tcl_HashEntry **) malloc (sizeof(Tcl_HashEntry*) * (numberFound+1));
  if (!(*list))
    return GenErrorMemoryAlloc(interp);
  
  (*list)[numberFound] = NULL;
  
  /* un deuxieme passage pour stocker les elements */
  hashEntryPtr = Tcl_FirstHashEntry (tablePtr, &hashSearch);  
  n=0;
  while (hashEntryPtr)
    {
      entryName = Tcl_GetHashKey(tablePtr, hashEntryPtr);
      value = (_smurf_object_) Tcl_GetHashValue(hashEntryPtr);
      for (m=0;expr[m];m++)
	if (Tcl_StringMatch(entryName,expr[m])
	    && (value -> object_type == object_type))
	  {
	    (*list)[n++] = hashEntryPtr;
	    break;
	  }
      hashEntryPtr = Tcl_NextHashEntry( &hashSearch);
    }
  return TCL_OK;
}

/*----------------------------------------------------------------------
  GenErrorAppend
  
  
  --------------------------------------------------------------------*/
int
GenErrorAppend(Tcl_Interp * interp,...)
{
  va_list ap;
  char * string;

  va_start(ap,interp);
 
  string = va_arg(ap, char *);
  while (string)
    {
      Tcl_AppendResult(interp,string,NULL);
      string = va_arg(ap, char *);
    }

  return TCL_ERROR;
}

/*----------------------------------------------------------------------
  GenErrorMemoryAlloc
  
  Cette fonction renvoie TCL_ERROR, tout en affichant dans interp 
  un message d'erreur.
  --------------------------------------------------------------------*/
int
GenErrorMemoryAlloc(Tcl_Interp * interp)
{
  Tcl_AppendResult(interp,"Memory allocation error.",NULL);
  return TCL_ERROR;
}

/************************************
 * Command name in xsmurf : ginfo
 ************************************/
int
object_info_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)      
{
  char * options[] = {"L",
		      "-list", "",
		      NULL };

  char * help_msg =
  {("Give information on the list of object name (name).")};

  int is_list;

  Tcl_HashEntry **list;
  _smurf_object_ value;
  char          **expr;
  int          j;
  char         *sign_expr;
  char         *dispExpr;
  char *key;
  
  sign_expr = (char *) malloc (sizeof (char)*200);
  dispExpr = (char *) malloc (sizeof (char)*200);

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &expr) == TCL_ERROR)
    return TCL_ERROR;
  
  is_list = arg_present (1);

  if (GenDicGetUniqueEntryList (interp, &_smurf_object_hash_table_, expr, &list) == TCL_ERROR)
    return TCL_ERROR;
 
  if (list)
    {
      for (j = 0; list[j]; j++)
	{
	  value = (_smurf_object_) Tcl_GetHashValue (list[j]);
	  if (!list[j])
	    continue;
	  if (is_list)
	    {
	      key = (char *) Tcl_GetHashKey (&_smurf_object_hash_table_,
					     list[j]);
	      if (j != 0)
		Tcl_AppendResult (interp, " ", (char *) NULL);
	      Tcl_AppendResult (interp, key, (char *) NULL);
	    }
	  else
	    {
	      switch (value -> object_type)
		{
		case SM_SIGNAL:
		  sign_expr = display_signal_info ((Signal *) value -> object_ptr,
						   sign_expr);
		  key = (char *) Tcl_GetHashKey (&_smurf_object_hash_table_,
						 list[j]);
		  Tcl_AppendResult (interp,
				    "\n Signal ",
				    key,
				    "\n",
				    sign_expr,
				    (char *) NULL);
		  break;
	    
		case SM_IMAGE:	
		  dispExpr = display_image_info ((Image *) value -> object_ptr, dispExpr);
		  key = (char *) Tcl_GetHashKey (&_smurf_object_hash_table_,
						 list[j]);
		  Tcl_AppendResult (interp,
				    "\n Image ",
				    key,
				    "\n",
				    dispExpr,
				    (char *) NULL);
		  break;
	    
		case SM_IMAGE3D:	
		  dispExpr = display_image3D_info ((Image3D *) value -> object_ptr, dispExpr);
		  key = (char *) Tcl_GetHashKey (&_smurf_object_hash_table_,
						 list[j]);
		  Tcl_AppendResult (interp,
				    "\n Image3D ",
				    key,
				    "\n",
				    dispExpr,
				    (char *) NULL);
		  break;
	    
		case SM_EXTIMAGE:
		  dispExpr = display_ext_image_info ((ExtImage *) value -> object_ptr, dispExpr);
		  key = (char *) Tcl_GetHashKey (&_smurf_object_hash_table_,
						 list[j]);
		  Tcl_AppendResult (interp,
				    "\n ExtImage ",
				    key,
				    "\n",
				    dispExpr,
				    (char *) NULL);
		  break;

		case SM_EXTIMAGE3Dsmall:
		  dispExpr = display_ext_image_3Dsmall_info ((ExtImage3Dsmall *) value -> object_ptr, dispExpr);
		  key = (char *) Tcl_GetHashKey (&_smurf_object_hash_table_,
						 list[j]);
		  Tcl_AppendResult (interp,
				    "\n ExtImage3Dsmall ",
				    key,
				    "\n",
				    dispExpr,
				    (char *) NULL);
		  break;

		case SM_WAVELET:
		  sign_expr =
		    display_wavelet_info ((Wavelet *) value -> object_ptr,
					  sign_expr);
		  key = (char *) Tcl_GetHashKey (&_smurf_object_hash_table_,
						 list[j]);
		  Tcl_AppendResult (interp,
				    "\n Wavelet ",
				    sign_expr,
				    " ",
				    key,
				    (char *) NULL);
		  break;
	    
		case SM_LINE:
		  key = (char *) Tcl_GetHashKey (&_smurf_object_hash_table_,
						 list[j]);
		  Tcl_AppendResult (interp,
				    "\n Line ",
				    key,
				    (char *) NULL);
		  break;
		default:
		  break;
		}
	    }
	}
      free (list);
    }
  else
    return GenErrorAppend (interp, "No matching object", NULL);
  
  return TCL_OK;
}

/****************************************
 * Command name in xsmurf : describe
 ****************************************/
int
object_display_description_TclCmd_ (ClientData clientData,
				    Tcl_Interp *interp,
				    int        argc,
				    char       **argv)      
{
  char * options[] = { "L",
		       NULL };
  
  char * help_msg =
  {("Give the description field of the list of object (list).")};
  
  Tcl_HashEntry **list;
  _smurf_object_ value;
  char          **expr;
  int           j;
  //char string[200];
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &expr) == TCL_ERROR)
    return TCL_ERROR;
  
  if (GenDicGetUniqueEntryList (interp, &_smurf_object_hash_table_, expr, &list)
      == TCL_ERROR)
    return TCL_ERROR;
  
  if (list)
    {
      for (j = 0; list[j]; j++)
	{
	  value = (_smurf_object_) Tcl_GetHashValue (list[j]);
	  /*	  switch (value -> object_type)
	    {
	    case SM_SIGNAL:
	      sprintf(string, "Signal %-15s ",
		      Tcl_GetHashKey (&_smurf_object_hash_table_, list[j]));
	      break;
	    case SM_IMAGE:	
	      sprintf(string, "Image %-15s ",
		      Tcl_GetHashKey (&_smurf_object_hash_table_, list[j]));
	      break;
	    case SM_EXTIMAGE:
	      sprintf(string, "Extrema image %-15s ",
		      Tcl_GetHashKey (&_smurf_object_hash_table_, list[j]));
	      break;
	    case SM_EXTIMAGE3Dsmall:
	      sprintf(string, "Extrema3Dsmall image %-15s ",
		      Tcl_GetHashKey (&_smurf_object_hash_table_, list[j]));
	      break;
	    case SM_WAVELET:
	      sprintf(string, "Wavelet %-15s ",
		      Tcl_GetHashKey (&_smurf_object_hash_table_, list[j]));
	      break;
	    case SM_LINE:
	      sprintf(string, "Line %-15s ",
		      Tcl_GetHashKey (&_smurf_object_hash_table_, list[j]));
	      break;
	    }*/
	  /* a revoir, tout ca...*/
	  Tcl_AppendResult (interp, value->description, (char *) NULL);
	}
      free (list);
    }
  else
    return GenErrorAppend (interp, "No matching object", NULL);
  
  return TCL_OK;
}

/***************************************
 * Commande name in xsmurf : setdesc
 ***************************************/
int
object_set_description_TclCmd_ (ClientData clientData,
				Tcl_Interp *interp,
				int        argc,
				char       **argv)      
{
  char * options[] = { "sL",
		       NULL };
  
  char * help_msg =
  {("Set the description of the list. (description, list). Big bug for this command. Handle with care.")};
  
  Tcl_HashEntry **list;
  _smurf_object_ value;
  char          **expr;
  char          *description;
  int           j;
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &description, &expr) == TCL_ERROR)
    return TCL_ERROR;
  
  if (GenDicGetUniqueEntryList (interp, &_smurf_object_hash_table_, expr, &list)
      == TCL_ERROR)
    return TCL_ERROR;
 
  if (list)
    {
      for (j = 0; list[j]; j++)
	{
	  value = (_smurf_object_) Tcl_GetHashValue (list[j]);
	  free (value -> description);
	  value -> description =
	    (char *) malloc (sizeof (char)*strlen (description));
	  strcpy (value -> description, description);
	}
      free (list);
    }
  else
    return GenErrorAppend (interp, "No matching object", NULL);
  
  return TCL_OK;
}

/*
 */  
int
delete_entry_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "L",
    NULL
  };

  char * help_msg =
  {(
    "  Delete a list of objects.\n"
    "\n"
    "Arguments :\n"
    "  List - The list of objects.\n"
    "\n"
    "Return value :\n"
    "  None.")};

  /* Command's parameters */
  Tcl_HashEntry **list;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  char *object_name;
  char **expr;
  int  j;

  /* Command line analysis */
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &expr) == TCL_ERROR)
    return TCL_ERROR;
  
  if (GenDicGetUniqueEntryList (interp, &_smurf_object_hash_table_, expr, &list)
      == TCL_ERROR)
    return TCL_ERROR;
 
  if (list) {
    for (j = 0; list[j]; j++) {
      object_name = Tcl_GetHashKey (&_smurf_object_hash_table_, list[j]);
      delete_entry (object_name);
    }
    free (list);
  }
  else {
    return GenErrorAppend (interp, "No matching object.", NULL);
  }
  
  return TCL_OK;
} 


/***********************************************/

/*
 */
void
store_signal_in_dictionary (char   *name,
			    Signal *signal_ptr)
{
  _smurf_object_ value;
  
  value = _create_smurf_object_ (SM_SIGNAL, signal_ptr, name);
  store_smurf_object (name, value);
}

/*
 */
void
unstore_signal (char * name)
{
  delete_entry (name);
}

/*
 */
Signal *
get_signal (char * name)
{
  _smurf_object_ value;

  value = get_smurf_object (name);
  if (!value || value -> object_type != SM_SIGNAL)
    return 0;
  else
    return (Signal *) value -> object_ptr;
}

/***********************************************/

/*
 */
void
store_wavelet_in_dictionary (char   *name,
			     Wavelet *wavelet_ptr)
{
  _smurf_object_ value;
  
  value = _create_smurf_object_ (SM_WAVELET, wavelet_ptr, name);
  store_smurf_object (name, value);
}

/*
 */
void
unstore_wavelet (char * name)
{
  delete_entry (name);
}

/*
 */
Wavelet *
get_wavelet (char * name)
{
  _smurf_object_ value;

  value = get_smurf_object (name);
  if (!value || value -> object_type != SM_WAVELET)
    return 0;
  else
    return (Wavelet *) value -> object_ptr;
}

/***********************************************/

/*
 */
void
store_line_in_dictionary (char *name,
			  Line *line_ptr)
{
  _smurf_object_ value;
  
  value = _create_smurf_object_ (SM_LINE, line_ptr, name);
  store_smurf_object (name, value);
}

/*
 */
void
unstore_line (char * name)
{
  delete_entry (name);
}

/*
 */
void
unstore_line_by_value (Line *line)
{
  _smurf_object_ object;
  Tcl_HashSearch *search_ptr = NULL;

  object = (_smurf_object_) 
    Tcl_FirstHashEntry (&_smurf_object_hash_table_, search_ptr);
  while (object && line != (Line *) object -> object_ptr)
    {
      object = (_smurf_object_) Tcl_NextHashEntry (search_ptr);
    }
  if (object)
    {
      Tcl_DeleteHashEntry ((Tcl_HashEntry* ) object);
    }
}

/*
 */
Line *
get_line (char *name)
{
  _smurf_object_ value;

  value = get_smurf_object (name);
  if (!value || value -> object_type != SM_LINE)
    return 0;
  else
    return (Line *) value -> object_ptr;
}

/***********************************************/

/*
 */
void
store_image (char  *name,
	     Image *image_ptr)
{
  _smurf_object_ value;
  
  value = _create_smurf_object_ (SM_IMAGE, image_ptr, name);
  store_smurf_object (name, value);
}

/*
 */
Image *
get_image(char *name)
{
  _smurf_object_ value;

  value = get_smurf_object (name);
  if (!value || value -> object_type != SM_IMAGE)
    return 0;
  else
    return (Image *) value -> object_ptr;
}

/*
 */
void
unstore_image(char *name)
{
  delete_entry (name);
}

/***********************************************/

/*
 */
void
store_image3D (char  *name,
	       Image3D *image_ptr)
{
  _smurf_object_ value;
  
  value = _create_smurf_object_ (SM_IMAGE3D, image_ptr, name);
  store_smurf_object (name, value);
}

/*
 */
Image3D *
get_image3D(char *name)
{
  _smurf_object_ value;
  
  value = get_smurf_object (name);
  if (!value || value -> object_type != SM_IMAGE3D)
    return 0;
  else
    return (Image3D *) value -> object_ptr;
}

/*
 */
void
unstore_image3D(char *name)
{
  delete_entry (name);
}

/******************************************************************/

/*
 */
void
ExtDicStore(char     *name,
	    ExtImage *image_ptr)
{

  /* try to unstore before store (so that when doing ecut with input and
     output name identical, freeing memory is properly done)*/
  delete_entry(name);

  _smurf_object_ value;
  
  value = _create_smurf_object_ (SM_EXTIMAGE, image_ptr, name);
  store_smurf_object (name, value);
}

/*
 */
void
Ext3DsmallDicStore(char            *name,
		   ExtImage3Dsmall *image_ptr)
{
  _smurf_object_ value;
  
  value = _create_smurf_object_ (SM_EXTIMAGE3Dsmall, image_ptr, name);
  store_smurf_object (name, value);
}

/*
 */
ExtImage *
ExtDicGet(char *name)
{
  _smurf_object_ value;

  value = get_smurf_object (name);
  if (!value || value -> object_type != SM_EXTIMAGE)
    return 0;
  else
    return (ExtImage *) value -> object_ptr;
}

/*
 */
ExtImage3Dsmall *
Ext3DsmallDicGet(char *name)
{
  _smurf_object_ value;

  value = get_smurf_object (name);
  if (!value || value -> object_type != SM_EXTIMAGE3Dsmall)
    return 0;
  else
    return (ExtImage3Dsmall *) value -> object_ptr;
}

/*
 */
void
ExtDicRemove(char *name)
{
  delete_entry (name);
}

/*
 */
void
Ext3DsmallDicRemove(char *name)
{
  delete_entry (name);
}


/*------------------------------------------------------------------------
  ExtDicImageListGet
  
  Cette fonction, qui devra etre clarifiee, permet de recuperer un tableau
  de ExtImage dont le nom concorde avec l'expression reguliere passee en
  argument.
  ----------------------------------------------------------------------*/
int
ExtDicImageListGet(Tcl_Interp *interp,
		   char       **expr, 
		   ExtImage   ***extImageListPtr)
{
  _smurf_object_   value;
  int              i;
  Tcl_HashEntry ** list ;
  
  if (GenDicGetUniqueEntryListByType(interp,
				     &_smurf_object_hash_table_,
				     expr,
				     &list,
				     SM_EXTIMAGE) == TCL_ERROR)
    return TCL_ERROR;
  
  for (i=0;list[i];i++)
    {
      value = (_smurf_object_) Tcl_GetHashValue(list[i]); 
      list[i] = (Tcl_HashEntry *) value -> object_ptr;
    }
  
  *extImageListPtr = (ExtImage **) list;
  return TCL_OK;
  /*  _smurf_object_    value;
  int              i;
  int              nb = 0;
  Tcl_HashEntry ** list ;
  
  if (GenDicGetUniqueEntryList(interp,&_smurf_object_hash_table_,expr,&list)==TCL_ERROR)
    return TCL_ERROR;
  
  for (i=0;list[i];i++)
    {
      value = (_smurf_object_) Tcl_GetHashValue(list[i]);
      if (!list[i])
	continue;
      if (value -> object_type != SM_EXTIMAGE)
	continue;
      nb ++;
    }

  *extImageListPtr = (ExtImage **) malloc (sizeof (ExtImage *)*nb);

  for (i=0;list[i];i++)
    {
      value = (_smurf_object_) Tcl_GetHashValue(list[i]);
      if (!list[i])
	continue;
      if (value -> object_type != SM_EXTIMAGE)
	  continue;
      *extImageListPtr[i] = (ExtImage *) value -> object_ptr;
    }

  free (list);
 
  return TCL_OK;*/
}

/*------------------------------------------------------------------------
  Ext3DDicImageListGet
  
  Cette fonction, qui devra etre clarifiee, permet de recuperer un tableau
  de ExtImage3D dont le nom concorde avec l'expression reguliere passee en
  argument.
  ----------------------------------------------------------------------*/
int
Ext3DDicImageListGet(Tcl_Interp *interp,
		     char       **expr, 
		     ExtImage3D   ***extImage3DListPtr)
{
  _smurf_object_   value;
  int              i;
  Tcl_HashEntry ** list ;
  
  if (GenDicGetUniqueEntryListByType(interp,
				     &_smurf_object_hash_table_,
				     expr,
				     &list,
				     SM_EXTIMAGE3D) == TCL_ERROR)
    return TCL_ERROR;
  
  for (i=0;list[i];i++)
    {
      value = (_smurf_object_) Tcl_GetHashValue(list[i]); 
      list[i] = (Tcl_HashEntry *) value -> object_ptr;
    }
  
  *extImage3DListPtr = (ExtImage3D **) list;
  return TCL_OK;
  /*  _smurf_object_    value;
      int              i;
      int              nb = 0;
      Tcl_HashEntry ** list ;
      
  if (GenDicGetUniqueEntryList(interp,&_smurf_object_hash_table_,expr,&list)==TCL_ERROR)
    return TCL_ERROR;
  
  for (i=0;list[i];i++)
    {
      value = (_smurf_object_) Tcl_GetHashValue(list[i]);
      if (!list[i])
	continue;
      if (value -> object_type != SM_EXTIMAGE3D)
	continue;
      nb ++;
    }

  *extImage3DListPtr = (ExtImage3D **) malloc (sizeof (ExtImage3D *)*nb);

  for (i=0;list[i];i++)
    {
      value = (_smurf_object_) Tcl_GetHashValue(list[i]);
      if (!list[i])
	continue;
      if (value -> object_type != SM_EXTIMAGE3D)
	  continue;
      *extImage3DListPtr[i] = (ExtImage3D *) value -> object_ptr;
    }

  free (list);
 
  return TCL_OK;*/
}


/*
 */
int
get_type_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{
  char           * options[] = { "s" ,NULL };

  char           *name;
  _smurf_object_ value;
  
  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;

  if (arg_get (0, &name)==TCL_ERROR)
    return TCL_ERROR;
  
  value = get_smurf_object (name);
  if (value)
    switch (value -> object_type)
      {
      case SM_SIGNAL:
	sprintf (interp -> result, "S");
	break;
      case SM_IMAGE:
	sprintf (interp -> result, "I");
	break;
      case SM_EXTIMAGE:
	sprintf (interp -> result, "E");
	break;
      case SM_EXTIMAGE3D:
	sprintf (interp -> result, "R"); /* no reason for "R", simply
					  * because on qwerty keyboard
					  * "E" and "R" are neighbors !*/
	break;
      case SM_EXTIMAGE3Dsmall:
	sprintf (interp -> result, "T"); /* no reason for "T"...*/
      }
  
  return TCL_OK;
} 

/*------------------------------------------------------------------------
  ExtDicNameCmd_
  
  Renvoie les noms des ExtImage specifiees
  ----------------------------------------------------------------------*/  
int
ExtDicNameCmd_(ClientData clientData,
	       Tcl_Interp *interp,
	       int        argc,
	       char       **argv)      
{
  _smurf_object_    value;
  char           * options[] = { "L" ,NULL };
  int              i;
  Tcl_HashEntry ** list;
  char          ** expr;
  int is_first = 1;
  
  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;

  if (arg_get(0,&expr)==TCL_ERROR)
    return TCL_ERROR;
  
  if (GenDicGetUniqueEntryList(interp,&_smurf_object_hash_table_,expr,&list)==TCL_ERROR)
    return TCL_ERROR;
  
  if (list)
    {
      for (i=0;list[i];i++)
	{
	  value = (_smurf_object_) Tcl_GetHashValue(list[i]);
	  if (!list[i])
	    continue;
	  if (value -> object_type != SM_EXTIMAGE)
	    continue;
	  if (is_first)
	    sprintf(interp->result,"%s",
		    Tcl_GetHashKey(&_smurf_object_hash_table_,list[i]));
	  else
	    sprintf(interp->result,"%s %s", interp->result,
		    Tcl_GetHashKey(&_smurf_object_hash_table_,list[i]));
	  is_first = 0;
	}
      free(list);
    }

  return TCL_OK;
} 

/*------------------------------------------------------------------------
  Ext3DDicNameCmd_
  
  Renvoie les noms des ExtImage3D specifiees
  ----------------------------------------------------------------------*/  
int
Ext3DDicNameCmd_(ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)      
{
  _smurf_object_    value;
  char           * options[] = { "L" ,NULL };
  int              i;
  Tcl_HashEntry ** list;
  char          ** expr;
  int is_first = 1;
  
  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;

  if (arg_get(0,&expr)==TCL_ERROR)
    return TCL_ERROR;
  
  if (GenDicGetUniqueEntryList(interp,&_smurf_object_hash_table_,expr,&list)==TCL_ERROR)
    return TCL_ERROR;
  
  if (list)
    {
      for (i=0;list[i];i++)
	{
	  value = (_smurf_object_) Tcl_GetHashValue(list[i]);
	  if (!list[i])
	    continue;
	  if (value -> object_type != SM_EXTIMAGE3D)
	    continue;
	  if (is_first)
	    sprintf(interp->result,"%s",
		    Tcl_GetHashKey(&_smurf_object_hash_table_,list[i]));
	  else
	    sprintf(interp->result,"%s %s", interp->result,
		    Tcl_GetHashKey(&_smurf_object_hash_table_,list[i]));
	  is_first = 0;
	}
      free(list);
    }

  return TCL_OK;
} 

/*------------------------------------------------------------------------
  ExtDicUnlinkStamp_
  
  Cette fonction lance la procedure ExtImaUnlink_ sur tous les elements
  du dictionnaire ayant le 'stamp' donne.
  ----------------------------------------------------------------------*/
void
ExtDicUnlinkStamp_(int stamp)
{
  _smurf_object_   value;
  ExtImage      * tmpExtImage;
  Tcl_HashSearch  hashSearch;
  Tcl_HashEntry * hashEntryPtr;

  hashEntryPtr = Tcl_FirstHashEntry (&_smurf_object_hash_table_, &hashSearch);  
  while (hashEntryPtr)
    {
      value = (_smurf_object_) Tcl_GetHashValue(hashEntryPtr);
      if (value -> object_type == SM_EXTIMAGE)
	{
	  tmpExtImage = (ExtImage *) value -> object_ptr;
	  if (tmpExtImage->stamp==stamp)
	    ExtImaUnlink_(tmpExtImage);
	  hashEntryPtr = Tcl_NextHashEntry( &hashSearch);
	}
    }
}

/*------------------------------------------------------------------------
  Ext3DDicUnlinkStamp_
  
  Cette fonction lance la procedure ExtIma3DUnlink_ sur tous les elements
  du dictionnaire ayant le 'stamp' donne.
  ----------------------------------------------------------------------*/
void
Ext3DDicUnlinkStamp_(int stamp)
{
  _smurf_object_   value;
  ExtImage3D    * tmpExtImage3D;
  Tcl_HashSearch  hashSearch;
  Tcl_HashEntry * hashEntryPtr;

  hashEntryPtr = Tcl_FirstHashEntry (&_smurf_object_hash_table_, &hashSearch);

  while (hashEntryPtr)
    {
      value = (_smurf_object_) Tcl_GetHashValue(hashEntryPtr);
      if (value -> object_type == SM_EXTIMAGE3D)
	{
	  tmpExtImage3D = (ExtImage3D *) value -> object_ptr;
	  if (tmpExtImage3D->stamp==stamp)
	    // a changer !!!
	    //ExtIma3DUnlink_(tmpExtImage3D);
	  hashEntryPtr = Tcl_NextHashEntry( &hashSearch);
	}
    }
}


/*------------------------------------------------------------------------
  ExtDicUnlinkCmd_
  
  Cette commande Tcl permet de detruire le chainage des images specifiees.
  Remarque: il suffit de specifier un seul element d'un groupe d'image
  pour enlever les liens de l'ensemble...
  ----------------------------------------------------------------------*/
int
ExtDicUnlinkCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{
  char * options[] = { "L" ,NULL };

  int j;
  ExtImage ** list;
  char ** expr;
  
  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;

  if (arg_get(0,&expr)==TCL_ERROR)
    return TCL_ERROR;

  if (ExtDicImageListGet(interp,expr,&list)==TCL_ERROR)
    return TCL_ERROR;
  
  if (list)
    {
      for (j=0;list[j];j++)
	ExtImaUnlink_(list[j]);
      free(list);
    }
  else 
    return GenErrorAppend(interp,"No matching ExtImage.",NULL);
  
  return TCL_OK;
}

/*------------------------------------------------------------------------
  Ext3DDicUnlinkCmd_
  
  Cette commande Tcl permet de detruire le chainage des 3D ext images 
  specifiees.
  Remarque: il suffit de specifier un seul element d'un groupe d'image
  pour enlever les liens de l'ensemble...
  ----------------------------------------------------------------------*/
int
Ext3DDicUnlinkCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{
  char * options[] = { "L" ,NULL };

  int j;
  ExtImage3D ** list;
  char ** expr;
  
  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;

  if (arg_get(0,&expr)==TCL_ERROR)
    return TCL_ERROR;

  if (Ext3DDicImageListGet(interp,expr,&list)==TCL_ERROR)
    return TCL_ERROR;
  
  if (list)
    {
      for (j=0;list[j];j++)
	// a changer
	//Ext3DImaUnlink_(list[j]);
      free(list);
    }
  else 
    return GenErrorAppend(interp,"No matching ExtImage3D.",NULL);
  
  return TCL_OK;
}
