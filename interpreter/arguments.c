#include "arguments.h"
#include "../signal/signal.h"
#include "../image/image.h"
#include "../image/image3D.h"
#include "../wt2d/wt2d.h"
#include "../wt3d/wt3d.h"
#include "../widgets/Widget.h"

/* a mettre ailleur ? */
extern Image    * get_image  (char *);
extern Image3D  * get_image3D(char *);
extern Signal   * get_signal (char *);
extern Line     * get_line   (char *);
extern ExtImage * ExtDicGet  (char *);
extern ExtImage3Dsmall * Ext3DsmallDicGet(char *);

/*----------------------------------------------------------------------
  Variables locales :
  Mise a jour au moment de l'appel de arg_init
  --------------------------------------------------------------------*/
#define        MAXOPT   50	/* on a fixe le nb max d'options */
static int     optPos[MAXOPT];	/* position des options dans argv.
				   Par exemple, si l'option 2 est presente
				   au 4eme element de argv, optPos[2]
				   =4. -1 si l'option est absente. */
static int           formatSize;
static int           optNb;	/* nombre d'options specifiees par le
				   programmeur */
static int           argc;	/* memorisation de la ligne de commande */
static char       ** argv;
static char       ** formats;	/* format des arguments pour les options */
static Tcl_Interp *  interp;

/*----------------------------------------------------------------------
  Quelques buffers locaux pour stocker des listes.
  Pour l'instant, parce que je n'ai pas que ca a faire, il s'agit de
  tableaux statiques. Il faudra revoir ca en dynamique.
  --------------------------------------------------------------------*/
#define BUFSIZE   1000
static char  * charList[BUFSIZE];
static int     charListPos;     

/*----------------------------------------------------------------------
  _GiveOptions_
  
  Affiche en clair le format d'option specifie en parametre
  --------------------------------------------------------------------*/
static void _GiveOptions_(char * optionString,char * format)
{
  int formatPos=0;
  char c,buffer[2000];
  if (optionString)
    sprintf(buffer,"[%s ",optionString);
  else
    sprintf(buffer,"usage : %s ",argv[0]);
  while ((c=format[formatPos++])!='\0')
    switch(c)
      {
      case '[':
	strcat(buffer,"[ ");
	break;
      case ']':
	strcat(buffer,"] ");
	break;
      case 'd':
	strcat(buffer,"int ");
	break;
      case 's':
	strcat(buffer,"str ");
	break;
      case 'f':
	strcat(buffer,"real ");
	break;
      case 'L':
	strcat(buffer,"list ");
	break;
      case 'I':
	strcat(buffer,"image ");
	break;
      case 'J':
	strcat(buffer,"image3D ");
	break;
      case 'E':
	strcat(buffer,"ext ");
	break;
      case 'T':
	strcat(buffer,"ext3dsmall ");
	break;
      case 'S':
	strcat(buffer,"signal ");
	break;
      case 'l':
	strcat(buffer,"line ");
	break;
/*      case 'V':
	strcat(buffer,"viewer ");
	break;*/ /* attention a la version X11 ou non !!! */
      }
  if (optionString)
    strcat(buffer,"] ");

  Tcl_AppendResult(interp,buffer,NULL);
}

/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/
static void
_GiveEveryOption_ ()
{
  int i;
  
  _GiveOptions_ (NULL,formats[0]);
  for (i = 1; i < formatSize; i += 2)
    _GiveOptions_ (formats[i], formats[i+1]);
}

/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/  
static void
_GiveHelp_ (char * help_msg)
{
  if (!help_msg)
    sprintf (interp->result, "Sorry, no help for `%s'.", argv[0]);
  else
    Tcl_AppendResult (interp, help_msg, NULL);
}

/*----------------------------------------------------------------------
  _NotEnoughArg_
  
  Emet vers l'interpreteur Tcl le message indiquant qu'il manque
  un argument a l'option specifiee...
  --------------------------------------------------------------------*/
static int _NotEnoughArg_(int option)
{
  if (option)
    {
      Tcl_AppendResult(interp,"Not enough paramaters",
		       " to `",argv[0],"' option :",NULL);
      _GiveOptions_(formats[2*(option-1)+1],formats[2*option]);
    }
  else
    {
      Tcl_AppendResult(interp,"Not enough paramaters to function `",
		       argv[0],"' :\n",NULL);
      _GiveOptions_(NULL,formats[0]);
    }
  return TCL_ERROR;
}

/*----------------------------------------------------------------------
  arg_present
  Renvoie vrai si l'option dont le numero optNumber est presente dans
  la ligne de commande
  --------------------------------------------------------------------*/
int arg_present(int optNumber)
{
  if (optNumber==0)
    return 1;
  return optPos[optNumber-1]>0;
}

/*----------------------------------------------------------------------
  arg_init
  --------------------------------------------------------------------*/
int arg_init(Tcl_Interp * interp_in,int argc_in,char ** argv_in,
		 char ** formats_in, char * help_msg)
{
  int i,j;

  /* mise a jour des variables statiques */
  interp      = interp_in ;
  argc        = argc_in   ;
  argv        = argv_in   ;
  formats     = formats_in;
  charListPos = 0;
  /* calcul du nb d'elements dans le tableau `formats' */
  for (formatSize=0;formats[formatSize];formatSize++);
  
  /* mise a jour du nombre d'options correspondant a la taille de `formats' */
  optNb = formatSize/2;
  if (optNb)
    {
      for (i=0;i<optNb;i++)
	optPos[i] = -1;
      
      /* reperage de la position des arguments dans la ligne de commande*/
      for (i=1;i<argc;i++)
	for (j=1;j<formatSize;j+=2)
	  if (argv[i][0]==formats[j][0])
	    if (!strcmp(argv[i],formats[j]))
	      optPos[j/2]=i;
    }

  /* avant de rendre la main, on teste la presence des options
     -help et -options */
  if (argc==2)
    {
      if (!strcmp(argv[1],"-options"))
	{
	  _GiveEveryOption_();
	  return 1;
	}
      else if (!strcmp(argv[1],"-help"))
	{
	  _GiveEveryOption_();
	  Tcl_AppendResult(interp, "\n\n", NULL);
	  _GiveHelp_(help_msg);
	  return 1;
	}
    }

  return 0;
}

/*----------------------------------------------------------------------
  arg_get
  
  Aide au traitement des options et de leurs arguments.
  format:
    --------------------------------------------------------------------*/
int arg_get(int option,...)
{
  va_list ap;
  int argPos,argEnd,formatPos,optional,i;
  char c,* format;

  /* On sort si l'option n'est pas presente dans la ligne de commandes */
  if (!arg_present(option))
    return TCL_OK;
  
  /* s'il s'agit d'une option, on positionne argPos a l'endroit ou
     elle se trouve dans la ligne de commande. `format' contient le
     format des arguments attendus */
  if (option)
    {
      argPos = optPos[option-1] + 1;
      format = formats[ option * 2 ];
    }
  else 
    {
      argPos = 1;
      format = formats[0];
    }

  formatPos=0;

  /* La variable argEnd contient le numero de la prochaine option ou argc,
     c'est-a-dire le numero de argPos auquel le traitement se termine.*/  
  
  for (argEnd=argc,i=0;i<optNb;i++)
    if ((optPos[i]>=argPos) && (optPos[i]<argEnd))
      argEnd=optPos[i];

  va_start(ap,option);

  /* traitement : on lit format et on identifie les parametres... */
  optional=0;
  while ((c=format[formatPos++])!='\0')
    switch(c)
      {
      case '[':
	optional=1;
	break;
      
      case ']':
	optional=0;
	break;
	
      case 'd':			/* lecture d'un entier */
	{
	  int intValue, * resultIntPtr;

	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else 
		return _NotEnoughArg_(option);
	    }
	  if (Tcl_GetInt(interp,argv[argPos++],&intValue)==TCL_ERROR)
	    return TCL_ERROR;
	  
	  resultIntPtr  = va_arg(ap, int *);
	  *resultIntPtr = intValue;
	}
	break;
	
      case 's':			/* lecture d'un `char *' */
	{
	  char ** resultStringPtr;
	  
	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else 
		return _NotEnoughArg_(option);
	    }
	  
	  resultStringPtr  = va_arg (ap, char **);
	  *resultStringPtr = argv[argPos++];
	}
	break;
      
      case 'f':			/* lecture d'un real */
	{
	  double  doubleValue;
	  real * resultrealPtr;
	  
	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else 
		return _NotEnoughArg_(option);
	    }
	  
	  if (Tcl_GetDouble(interp,argv[argPos++],&doubleValue) == TCL_ERROR)
	    return TCL_ERROR;
	  
	  resultrealPtr  = va_arg(ap, real *);
	  *resultrealPtr = (real) doubleValue;
	}
	break;

      case 'L':			/* lecture d'une liste de `char *'   */
	{
	  char ** stringListValue, *** resultStringListPtr;

	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else
		return _NotEnoughArg_(option);
	    }
	  /* On recopie argv dans charList. Rq: pas de verification de
	     depassement !!! */
	  stringListValue = charList + charListPos;
	  while (argPos<argEnd)
	    charList[charListPos++] = argv[argPos++];
	  charList[charListPos++] = NULL;
	  resultStringListPtr  = va_arg(ap, char ***);
	  *resultStringListPtr = stringListValue;
	}
	break;
	
      case 'I':
	{
	  Image * i;
	  Image ** res;
	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else 
		return _NotEnoughArg_(option);
	    }
	  i = get_image (argv[argPos++]);
	  if (!i)
	    {
	      Tcl_AppendResult(interp,
			       "Couldn't find image `",
			       argv[argPos-1], "'.", NULL);
	      return TCL_ERROR;
	    }
/*	  if (get_image_TclCmd(interp,argv[argPos++],&i)==TCL_ERROR)
	    return TCL_ERROR;*/
	  res = va_arg(ap, Image **);
	  *res = i;
	}
	break;
	
      case 'J':
	{
	  Image3D * i;
	  Image3D ** res;
	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else 
		return _NotEnoughArg_(option);
	    }
	  i = get_image3D (argv[argPos++]);
	  if (!i)
	    {
	      Tcl_AppendResult(interp,
			       "Couldn't find image3D `",
			       argv[argPos-1], "'.", NULL);
	      return TCL_ERROR;
	    }
	  res = va_arg(ap, Image3D **);
	  *res = i;
	}
	break;
	
      case 'E':
	{
	  ExtImage * e;
	  ExtImage ** res;
	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else 
		return _NotEnoughArg_(option);
	    }
	  e = ExtDicGet (argv[argPos++]);
	  if (!e)
	    {
	      Tcl_AppendResult(interp,
			       "Couldn't find extrema image `",
			       argv[argPos-1], "'.", NULL);
	      return TCL_ERROR;
	    }
/*	  if (ExtDicTclGet(interp,argv[argPos++],&e)==TCL_ERROR)
	    return TCL_ERROR;*/
	  res = va_arg(ap, ExtImage ** );
	  *res = e;
	}
	break;
	
      case 'T':
	{
	  ExtImage3Dsmall * e;
	  ExtImage3Dsmall ** res;
	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else 
		return _NotEnoughArg_(option);
	    }
	  e = Ext3DsmallDicGet (argv[argPos++]);
	  if (!e)
	    {
	      Tcl_AppendResult(interp,
			       "Couldn't find extrema image 3D small `",
			       argv[argPos-1], "'.", NULL);
	      return TCL_ERROR;
	    }
/*	  if (ExtDicTclGet(interp,argv[argPos++],&e)==TCL_ERROR)
	    return TCL_ERROR;*/
	  res = va_arg(ap, ExtImage3Dsmall ** );
	  *res = e;
	}
	break;
	
      case 'S':			/* lecture d'un Signal                */
	{
	  Signal * s;
	  Signal ** res;
	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else 
		return _NotEnoughArg_(option);
	    }
	  s = get_signal (argv[argPos++]);
	  if (!s)
	    {
	      Tcl_AppendResult(interp,
			       "Couldn't find Signal `",
			       argv[argPos-1], "'.", NULL);
	      return TCL_ERROR;
	    }
/*	  if (get_signal_from_dictionary(interp,argv[argPos++],&s)==TCL_ERROR)
	    return TCL_ERROR;*/
	  res = va_arg(ap, Signal ** );
	  *res = s;
	}
	break;

      case 'l':			/* lecture d'un Line                */
	{
	  Line * s;
	  Line ** res;
	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else 
		return _NotEnoughArg_(option);
	    }
	  s = get_line (argv[argPos++]);
	  if (!s)
	    {
	      Tcl_AppendResult(interp,
			       "Couldn't find Line `",
			       argv[argPos-1], "'.", NULL);
	      return TCL_ERROR;
	    }
/*	  if (get_line_from_dictionary(interp,argv[argPos++],&s)==TCL_ERROR)
	    return TCL_ERROR;*/
	  res = va_arg(ap, Line ** );
	  *res = s;
	}
	break;

      case 'V':
	{
	  ViewImage * v;
	  ViewImage ** res;
	  if (argPos==argEnd)
	    {
	      if (optional)
		return TCL_OK;
	      else 
		return _NotEnoughArg_(option);
	    }
	  if (ViewDicTclGet(interp,argv[argPos++],&v)==TCL_ERROR)
	    return TCL_ERROR;
	  res = va_arg(ap, ViewImage ** );
	  *res = v;
	}
	break;
      }
  
  return TCL_OK; /* tous les arguments ont ete lus */
}
