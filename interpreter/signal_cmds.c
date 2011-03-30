/*
 * signal_cmds.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: signal_cmds.c,v 1.52 1999/07/09 15:27:22 decoster Exp $
 */

/* last modified by Pierre Kestener (2000/06/06). */


#include <math.h>
#include <gfft.h>
#include "../signal/signal.h"
#include "smPkgInt.h"
#include <stdlib.h>

#ifdef MEM_DEBUG
#include "mem_debug.h"
#endif

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#include <matheval.h>

/* a  mettre ailleurs */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef PI
#define PI 3.141592654
#endif

#ifndef LOG2
#define log2(x) (log(x)/log(2))
#endif

/* 
 * 
 *   Commands associated to signals manipulation.
 *
 */

/*
 *  Macro that returns TRUE if s1 and s2 begin and last at the same index.
 */
#define same_x_domain(s1,s2) (s1->n==s2->n)

int
SigOpAddCmd_(ClientData clientData,
	     Tcl_Interp * interp,
	     int        argc,
	     char       ** argv)
{
  char * options[] = { "SSs" ,           /* Signaux a transformer, resultat */
			 NULL };
  char * help_msg =
    {("Add the first signal with the second and put the result in name (signal1, signal2, name).\n"
      "  The two signals must have the same x domain.\n"
      "  The two signals must have the same type.")};

  Signal *signal1, *signal2, *result;
  char   *name;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal1, &signal2, &name) == TCL_ERROR)
    return TCL_ERROR;

  if(signal1->type != signal2->type)
    {
      sprintf (interp->result,"Signals must have the same type.\n");
      return TCL_ERROR;
    }

  if(!same_x_domain(signal1, signal2))
    {
      sprintf (interp->result,"Signals must have the same x domain.\n");
      return TCL_ERROR;
    }

  result = sig_add (signal1, signal2);

  if (!result)
    return TCL_ERROR;

  store_signal_in_dictionary (name, result);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigOpMultCmd_(ClientData clientData,
	     Tcl_Interp * interp,
	     int        argc,
	     char       ** argv)
{
  char * options[] = { "SSs" ,           /* Signaux a transformer, resultat */
			 NULL };

  char * help_msg =
  {("Multiply two signals and put the result in name (signal, signal, name).\n"
    "  The two signals must have the same x domain.\n"
    "  The two signals must have the same type.")};
  
  Signal *signal1, *signal2, *result;
  char   *name;


  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal1, &signal2, &name) == TCL_ERROR)
    return TCL_ERROR;

  result = sig_mult(signal1, signal2);

  if (!result)
    return TCL_ERROR;

  store_signal_in_dictionary(name, result);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigOpScamultCmd_(ClientData clientData,
		 Tcl_Interp * interp,
		 int        argc,
		 char       ** argv)
{
  char * options[] = { "Sfs" ,
			 NULL };

  char * help_msg =
  {("Multiply a signal by a scalar and put the result in name (signal, scalar, name).")};

  Signal *signal, *result;
  char   *name;
  real   mult;


  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal, &mult, &name) == TCL_ERROR)
    return TCL_ERROR;

  result = sig_scalar_mult(signal, mult);

  if (!result)
    return TCL_ERROR;

  store_signal_in_dictionary(name, result);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigOpExtendCmd_(ClientData clientData,
		Tcl_Interp * interp,
		int        argc,
		char       ** argv)
{
  char * options[] = { "Ss" ,
			 "-size", "d",
			 "-x", "dd",
			 NULL };
 
  char * help_msg =
  {("Extend the dimension of a signal to the next power of 2 and put 0 in the border.\n"
    "   -size extend the signal to the size (default next power of 2\n"
    "   -x extend to xmin and xmax.")};
  
  Signal *signal/*, *result*/;
  char   *name;
  /*  int    newsize;
  int    xmin, xmax;*/


  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal, &name) == TCL_ERROR)
    return TCL_ERROR;

  /*  if (arg_present(1))
    if (arg_get(1, &newsize) == TCL_ERROR)
      return TCL_ERROR;
    else
      result = sig_extend_to (signal, newsize);
  else if(arg_present(2))
    if (arg_get(2, &xmin, &xmax) == TCL_ERROR)
      return TCL_ERROR;
    else
      result = sig_extend_to_x (signal, xmin, xmax);
  else
    result = sig_extend (signal);

  if (!result)
    return TCL_ERROR;

  store_signal_in_dictionary(name, result);
  */
  sprintf (interp->result, "this command doesn't exist (you dreamt it)");
  return TCL_ERROR;

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigOpCutCmd_(ClientData clientData,
	     Tcl_Interp * interp,
	     int        argc,
	     char       ** argv)
{
  char * options[] = { "Ss[dd]" ,
			 NULL };

  char * help_msg =
  {("Cut a border of dim right-dim and left-dim of the signal (signal, result, left-dim, right-dim).")};
  
  Signal *signal, *result;
  char   *name;
  int    left_cut = 0, rigth_cut = 0;


  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal, &name, &left_cut, &rigth_cut) == TCL_ERROR)
    return TCL_ERROR;

  result = sig_cut (signal, left_cut, rigth_cut);

  if (!result)
    return TCL_ERROR;

  store_signal_in_dictionary(name, result);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigOpShiftCmd_(ClientData clientData,
	       Tcl_Interp * interp,
	       int        argc,
	       char       ** argv)
{
  char * options[] = { "Ssd" ,
			 NULL };
  char * help_msg =
  {("Shift the index of a signal (signal, result, shift).")};
 
  Signal *signal/*, *result*/;
  char   *name;
  int    shift;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal, &name, &shift) == TCL_ERROR)
    return TCL_ERROR;

  sprintf (interp->result, "this command doesn't exist (you dreamt it)");
  return TCL_ERROR;
  /*  result = sig_shift (signal, shift);

  if (!result)
    return TCL_ERROR;

  store_signal_in_dictionary(name, result);
  */
  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigOpNRCmd_(ClientData clientData,
	    Tcl_Interp * interp,
	    int        argc,
	    char       ** argv)
{
  char * options[] = { "Ss" ,
			 NULL };
 
  char * help_msg =
  {("Transform a REALY signal in a FOUR_NR signal (signal, result).")};
 
  Signal *signal, *result;
  char   *name;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  if (arg_get(0, &signal, &name) == TCL_ERROR)
    return TCL_ERROR;

  result = sig_real_to_fourier (signal);

  if (!result)
    return TCL_ERROR;

  store_signal_in_dictionary(name, result);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigOpFour2cplxCmd_(ClientData clientData,
		   Tcl_Interp * interp,
		   int        argc,
		   char       ** argv)
{
  char * options[] = { "Ss" ,
			 NULL };
  
  char * help_msg =
  {("Transform a FOUR_NR signal in a COMPLEX signal (signal, result).")};

  Signal *signal;
  char   *name;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal, &name) == TCL_ERROR)
    return TCL_ERROR;

  /*  result = sig_fourier_to_cplx (signal);*/
  sprintf (interp->result, "this command doesn't exist (you dreamt it)");
  return TCL_ERROR;

  /*  if (!result)
    return TCL_ERROR;*/

  /*  store_signal_in_dictionary(name, result);*/

  /*  return TCL_OK;*/
}

/*------------------------------------------------------------------------
  SigOpConvolutionCmd_

  Convolution de deux signaux. Leurs tailles et leur types doivent 
  etre identiques.
  ----------------------------------------------------------------------*/
int
SigOpConvolutionCmd_(ClientData clientData,
		     Tcl_Interp * interp,
		     int        argc,
		     char       ** argv)
{
  char * options[] = { "Ssff" ,           /* Signaux a convoluer, resultat */
			 NULL };

  char * help_msg =
  {("Direct convolution of a signal by a gaussian of mean m and sigma s (signal1, result,m,s).")};
  
  Signal *signal1, *sigres;
  char   *name;
  int i, k;
  real x,norm,sum,y,mean,sigma;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal1, &name,&mean,&sigma) == TCL_ERROR)
    return TCL_ERROR;

  if (signal1->type == REALXY) {
    sprintf (interp->result,"This command doesn't exist for REALXY signal.\n");
      return TCL_ERROR; 
  }

  norm = exp(-0.5*log(2.0*PI*sigma*sigma));
  sigres = sig_new(REALY, signal1->first, signal1->last);

  sigres->dx = signal1->dx;
  sigres->x0 = signal1->x0;

  for (i=0; i< signal1->size; i++) {
    x=signal1->x0+(float)i*signal1->dx;
    
    sum=0;
    for(k=0;k<signal1->size;k++){
	y=exp(-1.0*(x-mean-signal1->x0-(float)k*signal1->dx)*(x-mean-signal1->x0-(float)k*signal1->dx)/(2.0*sigma*sigma));
	sum=sum+y*signal1->dataY[k];
    }
    sigres->dataY[i]=norm*sum;
    
  }
  /*  result = sig_conv(signal1, signal2);*/
  /*  sprintf (interp->result,"This command doesn't exist (blblblblblblbl!).\n");
      return TCL_ERROR; */
  
  /*  if (!result)
      return TCL_ERROR;*/

  store_signal_in_dictionary(name, sigres);
  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  SigOpConvolutionCmd_

  Convolution de deux signaux. Leurs tailles peuvent etre differentes,
  mais les types doivent etre identiques.
  ----------------------------------------------------------------------*/
int
oldSigOpConvolutionCmd_(ClientData clientData,
		     Tcl_Interp * interp,
		     int        argc,
		     char       ** argv)
{
  char * options[] = { "SSs" ,           /* Signaux a convoluer, resultat */
			 NULL };

  char * help_msg =
  {("Direct convolution of two signal with the same type (signal1, signal2, result).")};
  
  Signal *signal1, *signal2;
  char   *name;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal1, &signal2, &name) == TCL_ERROR)
    return TCL_ERROR;

  /*  result = sig_conv(signal1, signal2);*/
  sprintf (interp->result,"This command doesn't exist (blblblblblblbl!).\n");
  return TCL_ERROR;

  /*  if (!result)
    return TCL_ERROR;*/

  /*  store_signal_in_dictionary(name, result);

  return TCL_OK;*/
}

/*------------------------------------------------------------------------
  SigTopCopyCmd_

  Commande Tcl permettant de copier un signal dans un autre.
  Le deuxieme signal donne en argument est detruit puis recree.
  ----------------------------------------------------------------------*/
int
SigTopCopyCmd_(ClientData clientData,
	       Tcl_Interp *interp,
	       int        argc,
	       char       **argv)      
{
  char * options[] = { "Ss",
			 NULL };
  char * help_msg =
  {("Copy a signal in result. (signal, result).")};

  Signal *srcSignalPtr, *result;
  char   *dstSignalName;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &srcSignalPtr, &dstSignalName) == TCL_ERROR)
    return TCL_ERROR;
  
  result = sig_duplicate (srcSignalPtr);
  store_signal_in_dictionary(dstSignalName, result);
  
  return TCL_OK;
}

/*------------------------------------------------------------------------
  SigTopGetValueCmd

  Affiche la valeur d'un point d'un signal
  ----------------------------------------------------------------------*/
int
SigTopGetValueCmd_(ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)      
{
  char * options[] = { "Sd",
			 NULL };

  char * help_msg =
  {("Give the value of a point of the signal (signal, index)\n"
    "return : {signal->data[x] x_value}")};

  Signal *signal;
  int    x;
  
  real x_value;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &signal, &x) == TCL_ERROR)
    return TCL_ERROR;

  switch (signal->type) {
  case REALY :
    x_value = signal->x0 + x*signal->dx;
    break;
  case REALXY :
    x_value = signal->dataX[x];
    break;
  }

  sprintf(interp->result,"%.15g %.15g",signal->dataY[x], x_value);
  return TCL_OK;
}

int
SigTopSetValueCmd_(ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)      
{
  char * options[] = { "Sdf",
			 NULL };

  char * help_msg =
  {(" Set the value of a point of the signal (signal, index, value).")};

  Signal *signal;
  int    x;
  real   value;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &signal, &x, &value) == TCL_ERROR)
    return TCL_ERROR;

  signal->dataY[x] = value;
  
  sprintf(interp->result,"%f",signal->dataY[x]);
  return TCL_OK;
}

/*------------------------------------------------------------------------
  SigTopGetSizeCmd

  Affiche la taille d'un signal
  ----------------------------------------------------------------------*/
int
SigTopGetSizeCmd_(ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)      
{
  char * options[] = { "S",
			 NULL };

  char * help_msg =
  {("Give the size of the signal (signal).")};

  Signal * signal;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &signal) == TCL_ERROR)
    return TCL_ERROR;
  
  sprintf(interp->result,"%d", signal->n);
  return TCL_OK;
}

/*------------------------------------------------------------------------
  SigTopGetFirstCmd

  Affiche la premiere valeur en x du signal.
  ----------------------------------------------------------------------*/
int
SigTopGetFirstCmd_(ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)      
{
  char * options[] = { "S",
			 NULL };
  char * help_msg =
  {("Give the first index of the signal (signal).")};

  Signal * signal;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &signal) == TCL_ERROR)
    return TCL_ERROR;
  
  sprintf(interp->result,"%d", signal->first);
  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigTopGetLastCmd_(ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)      
{
  char * options[] = { "S",
			 NULL };

  char * help_msg =
  {("Give the last index of the signal (signal).")};

  Signal * signal;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &signal) == TCL_ERROR)
    return TCL_ERROR;
  
  sprintf(interp->result,"%d", signal->last);
  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigTopGetdxCmd_(ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)      
{
  char * options[] = { "S",
			 NULL };

  char * help_msg =
  {("Give the dx value of the signal (signal). Only for REALY signals.")};

  Signal * signal;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &signal) == TCL_ERROR)
    return TCL_ERROR;
  
  if (signal->type == REALXY) {
    sprintf(interp->result, "Signal of type REALXY");
    return TCL_ERROR;
  }
  else
    sprintf(interp->result,"%.15g", signal->dx);

  return TCL_OK;
}

/**/
int
get_sig_type_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)      
{
  char * options[] = { "S",
			 NULL };

  char * help_msg =
  {("Give the type of the signal (signal).")};

  Signal * signal;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &signal) == TCL_ERROR)
    return TCL_ERROR;

  sprintf (interp->result, "%d", signal->type);

  return TCL_OK;
}
/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigTopGetx0Cmd_(ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)      
{
  char * options[] = { "S",
			 NULL };

  char * help_msg =
  {("Give the x0 value of the signal (signal). Only for REALY signals.")};

  Signal * signal;
  
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &signal) == TCL_ERROR)
    return TCL_ERROR;
  
  if (signal->type == REALXY) {
    sprintf(interp->result, "Sorry : signal of type REALXY\n");
    return TCL_ERROR;
  }
  else
    sprintf(interp->result,"%.15g", signal->x0);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigTopPutx0Cmd_(ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)      
{
  char * options[] = { "Sf",
			 NULL };

  char * help_msg =
  {("Put a new x0 value for the signal (signal,x0).")};

  Signal * signal;
  real x0;
  char *type[] = {"REALXY", "REALY", "CPLX", "FOUR_NR"};
    
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &signal,&x0) == TCL_ERROR)
    return TCL_ERROR;

  if (type[signal->type] == REALXY) {
    sprintf (interp->result, "Sorry : signal of type REALXY\n");
    return TCL_ERROR;
  }
  else
    signal->x0=x0;

  return TCL_OK;
}

/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
SigTopPutdxCmd_(ClientData clientData,
		Tcl_Interp *interp,
		int        argc,
		char       **argv)      
{
  char * options[] = { "Sf",
			 NULL };

  char * help_msg =
  {("Put a new dx value for the signal (signal,dx).")};

  Signal * signal;
  real dx;
  char *type[] = {"REALXY", "REALY", "CPLX", "FOUR_NR"};

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &signal,&dx) == TCL_ERROR)
    return TCL_ERROR;

  if (type[signal->type] == REALXY) {
    sprintf (interp->result, "Sorry : signal of type REALXY\n");
    return TCL_ERROR;
  }
  else if (dx != 0)
    signal->dx=dx;
  else
    {
      Tcl_AppendResult (interp,
			"The new dx value must be non zero.",
			(char *) NULL);
    }
  
  return TCL_OK;
}

#ifndef PI
#define PI 3.141592654
#endif

#define mypow(x,alp)   (exp(alp*mylog(x)))

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

enum {RAN1, RAN3, GAUSS, EXPO};

/*----------------------------------------------------------------------
  SigGenSinCmd_
  
  Commande qui genere un signal suivant une expression mathematique.
  --------------------------------------------------------------------*/
int
SigGenExprrCmd_(ClientData clientData,
		Tcl_Interp *interp,
	        int        argc,
		char       **argv)      
{
  char * options[] = { "ssffd",     /* nom et taille du signal           */
			 "-nb", "d",
			 NULL };

  char * help_msg =
  {("Create a signal according to a mathematical expression :\n"
    "  - string : name of the result\n"
    "  - string : a valid expression\n"
    "      example : exp(-x*x)\n"
    "      see file defunc.man for details\n"
    "  - float  : xmin\n"
    "  - float  : xmax\n"
    "  - integer: number of points in signal\n"
    "Options :\n"
    "  -nb integer : ??\n"
    )};


  Signal *signal;
  char   *name, *expr;
  /*double (*fct)();*/
  void   *fct;
  int    nb, i, j, nbb=1;
  real   begin, end;
  real   x;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &name, &expr, &begin, &end, &nb)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_present (1))
    if (arg_get (1, &nbb) == TCL_ERROR)
      return TCL_ERROR;

  /*fct = dfopen (expr);*/
  fct = evaluator_create(expr);

  if (fct)
    {
      real dx;

      dx = (end - begin)/(nb-1);

      signal = sig_new(REALY, 0, nb - 1);
      signal->dx = dx;
      signal->x0 = begin;
      
      for (j = 0; j < nbb; j++)
	for (i = 0; i < signal->size; i++)
	  {
	    x = sig_index_to_real (signal, i);
	    signal->dataY[i] = evaluator_evaluate_x_y(fct, (double)x,(double)x);
	  }

      /*dfclose (fct);*/
      evaluator_destroy(fct);

      store_signal_in_dictionary(name,signal);
  
      Tcl_AppendResult(interp,name,NULL);
      return TCL_OK;
    }
  else
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ", expr, (char *) NULL);

      return TCL_ERROR;
    }
}

/*----------------------------------------------------------------------
  SigGenSinCmd_
  
  Commande qui construit un signal sinus.
  --------------------------------------------------------------------*/
int
SigGenSinCmd_(ClientData clientData,Tcl_Interp *interp,
	      int argc,char **argv)      
{
  char * options[] = { "sd[d]",     /* nom et taille du signal           */
			 NULL };

  char * help_msg =
  {("Create a sine signal (name size [dilatation]).")};

  Signal *signal;
  char   *name;
  int    size;
  int    i, nb = 1;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &name, &size, &nb)==TCL_ERROR)
    return TCL_ERROR;

  signal = sig_new(REALY, 0, size-1);
  signal->x0 = 0.0; 
  signal->dx = 2*PI*nb/(signal->size-1);

  for (i = 0; i < signal->n; i++)
    signal->dataY[i] = sin ( (real)(i)*2*nb*PI/(size-1));

  store_signal_in_dictionary(name,signal);
  
  Tcl_AppendResult(interp,name,NULL);
  return TCL_OK;
}


/*----------------------------------------------------------------------
  SigFlatCmd_
  
  Commande qui construit un signal plat (constant) !.
  --------------------------------------------------------------------*/
int
SigFlatCmd_(ClientData clientData,Tcl_Interp *interp,
	      int argc,char **argv)      
{
  char * options[] = { "sfd",     /* nom, valeur  et taille du signal */
			 NULL };

  char * help_msg =
  {("Create a flat signal (name value size).\n"
    "Don't forget to set (properly) x0 and dx\n"
    "using command sputx0 and sputdx\n")};

  Signal *signal;
  char   *name;
  int    size;
  real   value;
  int    i;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &name, &value, &size)==TCL_ERROR)
    return TCL_ERROR;

  signal = sig_new(REALY, 0, size-1);
  signal->x0 = 0.0; 
  signal->dx = 1.0;

  for (i = 0; i < signal->n; i++)
    signal->dataY[i] = value;

  store_signal_in_dictionary(name,signal);
  
  Tcl_AppendResult(interp,name,NULL);
  return TCL_OK;
}



/*----------------------------------------------------------------------
  SigGenDiracCmd_
  
  Commande qui construit un signal dirac centre en 0.
  --------------------------------------------------------------------*/
int
SigGenDiracCmd_(ClientData clientData,Tcl_Interp *interp,
		int argc,char **argv)      
{
  char * options[] = { "sd",     /* nom du signal + un entier. L'entier    */
			         /* sert pour calculer la taille.          */
		       "-pos", "d",
		       NULL };
  
  char * help_msg =
  {("Create a signal with dirac shape (name size).\n"
    "i.e. value at first index is 1 (zero otherwise)\n"
    "\n"
    "Don't forget to (properly) set x0 and dx\n"
    "using command sputx0 and sputdx\n"
    "Options :\n"
    " -pos integer : dirac at another index\n")};
  
  Signal *signal;
  char   *name;
  int    size;
  int    i;
  int pos;
  
  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &name, &size)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_present(1))
    {
      if (arg_get(1, &pos)==TCL_ERROR)
	return TCL_ERROR;

      signal = sig_new(REALY, 0, size-1);
      
      for (i = 0; i < signal->n; i++)
	signal->dataY[i] = 0.0;
      signal->dataY[pos] = 1.0;
    }
  else
    {
      signal = sig_new(REALY, 0, size-1);
      
      for (i = 0; i < signal->n; i++)
	signal->dataY[i] = 0.0;
      signal->dataY[0] = 1.0;
    }

  store_signal_in_dictionary(name,signal);
  
  Tcl_AppendResult(interp,name,NULL);
  return TCL_OK;
}

/*----------------------------------------------------------------------
  SigGenBoxCmd_
  
  Commande qui construit une boite (si si) centree en 0.
  --------------------------------------------------------------------*/
int
SigGenBoxCmd_(ClientData clientData,Tcl_Interp *interp,
		int argc,char **argv)      
{
  char * options[] = { "sd",     /* nom du signal + un entier. L'entier    */
			         /* sert pour calculer la taille.          */
		       "-period", "",
		       "-odd", "",
		       NULL };
  
  char * help_msg =
  {("create a box like shape signal (name size)\n")};
  Signal *signal;
  char   *name;
  int    size;
  int    i;
  int    is_period, is_odd;

  if (arg_init(interp,argc,argv,options,help_msg))
    return TCL_OK;
  
  if (arg_get(0, &name, &size)==TCL_ERROR)
    return TCL_ERROR;

  is_period = arg_present (1);
  is_odd    = arg_present (2);

  if (is_period || is_odd)
    {
      signal = sig_new(REALY, 0, size-1);

      for (i = 0; i < size/4; i++)
	signal->dataY[i] = 1.0;

      for (i = size/4; i < size; i++)
	signal->dataY[i] = 0.0;

      if (is_period)
	for (i = 3*size/4+1; i < size; i++)
	  signal->dataY[i] = 1.0;
    }
  else
    {
      signal = sig_new(REALY, 0, 2*size);

      for (i = 0; i < signal->n; i++)
	signal->dataY[i] = 0.0;

      for (i=size/2+1; i<=3*size/2; i++)
	signal->dataY[i] = 1.0;
      signal->x0 = - size;
    }

  store_signal_in_dictionary(name,signal);
  
  Tcl_AppendResult(interp,name,NULL);
  return TCL_OK;
}

/*----------------------------------------------------------------------
  SigGenLineCmd_
  
  Commande qui construit une droite.
  --------------------------------------------------------------------*/
int
SigGenLineCmd_(ClientData clientData,
	      Tcl_Interp *interp,
	      int argc,
	      char **argv)      
{
  char * options[] = { "sdff",     /* nom, taille, pente, val en zero. */
			 NULL };
  Signal *signal;
  char   *name;
  int    size;
  real  scale, shift;
  int    i;


  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;
  
  if (arg_get(0, &name, &size, &scale, &shift)==TCL_ERROR)
    return TCL_ERROR;

  signal = sig_new(REALY, 0, size - 1);

  for (i = 0; i < signal->n; i++)
    signal->dataY[i] = i * scale + shift;
  
  store_signal_in_dictionary(name,signal);
  
  Tcl_AppendResult(interp,name,NULL);
  
  return TCL_OK;
}


/*----------------------------------------------------------------------
  --------------------------------------------------------------------*/
/*int
  SigGenBrBlcCmd_(ClientData clientData,
  Tcl_Interp *interp,
  int        argc,
  char       **argv)      
  { 
  char * options[] = { "sd",
  "-alea", "d",
  "-sigma", "f",
  "-type", "d",
  NULL };
  
  int   dim, border = 0;
  Signal *signal;
  char  *signalName;
  int   alea;
  int   type = GAUSS;
  real sigma = 1;

  srand(time(NULL));
  alea = (int)(rand()/327.67);

  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;
  
  if (arg_get(0, &signalName, &dim)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &alea)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &type)==TCL_ERROR)
    return TCL_ERROR;

  signal = sig_new (REALY, 0, dim-1);

  if (!signal)
  return GenErrorMemoryAlloc(interp);
  sig_free (signal);
  
  _bruit_blanc_(signal->dataY, dim, sigma, type, alea);

  store_signal_in_dictionary(signalName,signal);

  Tcl_AppendResult(interp,signalName,NULL);
  return TCL_OK;
}
*/
/*----------------------------------------------------------------------
  SigGenGaussCmd_
  
  Commande qui construit une gaussienne.
  --------------------------------------------------------------------*/
int
SigGenGaussCmd_(ClientData clientData,Tcl_Interp *interp,
		int argc,char **argv)      
{
  char * options[] = { "sd[f]",     /* nom et taille du signal           */
			 NULL };
  Signal *signal;
  char   *name;
  int    size;
  int    i;
  real   sigma = 1;

  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;
  
  if (arg_get(0, &name, &size, &sigma)==TCL_ERROR)
    return TCL_ERROR;

  signal = sig_new(REALY, 0, size-1);

  for (i = 0; i <= size/2; i++)
    signal->dataY[i] = exp(-i*i/(sigma*sigma));
  for (; i < signal->n; i++)
    signal->dataY[i] = exp(-(i-size)*(i-size)/(sigma*sigma));

  store_signal_in_dictionary(name,signal);
  
  Tcl_AppendResult(interp,name,NULL);
  return TCL_OK;
}


/***********************************
 * commande name in xsmurf : ssave
 ***********************************/
int
save_signal_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int argc,
		     char **argv)    
{
  /* Command line definition */
  char * options[] = {
    "S[s]",
    "-ascii", "",
    "-noheader", "",
    "-sw", "",
    "-step", "f",
    "-offset", "d",
    "-lw", "",
    "-errorbars","S",
    NULL
  };

  char * help_msg = {
    (" Write a signal in a binary file.\n"
     "\n"
     "Arguments :\n"
     "  signal   - signal to write.\n"
     "  [string] - name of the file. Default is the signal name.\n"
     "\n"
     "Options :\n"
     "  -ascii    : Write une an ascii file.\n"
     "  -noheader : Doesn't write the header.\n"
     "  -sw       : Use the format of sw.\n"
     "  -step     : Subsample the signal. This option doesn't work with "
     "binary format.\n"
     "  -offset   : Start saving signal at a given record.\n"
     "  -lw       : Use the format of lw.\n"
     "  -errorbars[S] : use it only with -ascii and -noheader to add a 3rd\n"
     "                  column (signal given in argument) which represents\n"
     "                  error bars on the y-axis.\n"
     "\n"
     "Return value :\n"
     "  Name of the file.")
  };

  /* Command's parameters */
  Signal *signal,*signal_errors;
  char * file_name = 0;

  /* Options's presence */
  int is_ascii;
  int is_noheader;
  int is_sw;
  int is_step;
  int is_offset;
  int is_lw;
  int is_errorbars;

  /* Options's parameters */
  real step = 1;
  int offset = 0;

  /* Other variables */
  FILE *out;
  int  size, first, last;
  int  i;
  real  ri;
  real *data;
  real  dx;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &file_name)==TCL_ERROR)
    return TCL_ERROR;

  is_ascii = arg_present(1);
  is_noheader = arg_present(2);
  is_sw = arg_present(3);
  is_step = arg_present(4);
  if (is_step) {
    if (arg_get(4, &step) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  is_offset = arg_present(5);
  if (is_offset) {
    if (arg_get(5, &offset) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  is_lw = arg_present(6);
  is_errorbars = arg_present(7);
  if (arg_get(7, &signal_errors) == TCL_ERROR) {
    return TCL_ERROR;
  }

  /* Parameters validity and initialisation */
  if (!is_sw && !is_lw && !is_ascii) {
    step = 1;
  }

  if (step < 1) {
    sprintf (interp->result, "the step must be greater or equal to 1");
    return TCL_ERROR;
  }

  if (offset < 0) {
    sprintf (interp->result, "the offset must be greater or equal to 0");
    return TCL_ERROR;
  }

  /* If no file name is specified, use the name of the signal */
  if (!file_name) {
    file_name = argv[1];
  }

  /* Treatement */

  size  = signal->size;
  first = signal->first;
  last  = signal->last;
  data  = signal->dataY;
  dx = signal->dx;

  out = fopen (file_name, "w");
  if (!out) {
    return GenErrorAppend (interp, "Couldn't open `", file_name,
			   "' for writing.", NULL);
  }

  /* Write the header if requested. */
  if (!is_noheader) {
    if (is_sw) {
      fprintf (out,
	       "HEADER\n"
	       "size %d\n"
	       "name %s\n"
	       "firstp %d\n"
	       "lastp %d\n"
	       "dx %g\n"
	       "x0 %g\n"
	       "END HEADER\n",
	       size, argv[1], first, last, dx, signal->x0);
    } else if (is_lw) {
      fprintf (out,
	       "LastWave Header\n"
	       "size %d\n"
	       "name %s\n"
	       "firstp %d\n"
	       "lastp %d\n"
	       "dx %g\n"
	       "x0 %g\n"
	       "End of Header\n",
	       size, argv[1], first, last, dx, signal->x0);
    } else if (is_ascii) {
      fprintf (out,
	       "Ascii %d %d %d %d %f %f\n",
	       signal->type, size, first, last, signal->x0, dx);
    } else { /* Binary header (default) */
      fprintf (out,
	       "Binary %d %d %d %d %f %f (%d byte reals)\n",
	       signal->type, size, first, last, signal->x0, dx,
	       (int)sizeof(real));
    }
  }

  /* Write data. */
  if (is_sw || is_lw || is_ascii) {
    /* Ascii format */
    switch (signal->type) {
    case REALY:
      if (is_noheader) {
	for (ri = offset; ri < signal->size; ri += step) {
	  i = (int) floor(ri);
	  if (is_errorbars)
	    fprintf (out, "%g %g %g\n", signal->x0+i*dx, data[i], signal_errors->dataY[i]);
	  else
	    fprintf (out, "%g %g\n", signal->x0+i*dx, data[i]);
	}
      } else {
	for (ri = offset; ri < signal->size; ri += step) {
	  i = (int) floor(ri);
	  fprintf (out, "%g\n", data[i]);
	}
      }
      break;
    case REALXY:
      for (ri = offset; ri < signal->size; ri += step) {
	i = (int) floor(ri);
	fprintf (out, "%g %g\n",
		 signal->dataX[i], data[i]);
      }
      break;
    case CPLX:
    case FOUR_NR:
      for (ri = offset; ri < signal->size; ri += step) {
	i = (int) floor(ri);
	fprintf (out, "%g %g\n", data[2*i], data[2*i+1]);
      }
      break;
    }
  } else {
    /* Binary format */

    switch (signal->type) {
    case REALY:
    case CPLX:
    case FOUR_NR:
      fwrite (data, sizeof(real), size, out);
      break;
    case REALXY:
      fwrite (data, sizeof(real), size, out);
      fwrite (signal->dataX, sizeof(real), size, out);
      break;
    }
  }

  fclose (out);
  
  Tcl_AppendResult (interp, file_name, NULL);

  return TCL_OK;
}


/*
 * Enum used by load_signal_TclCmd_ function.
 */
enum {
  SM_UNKNOWN_FORMAT,
  SM_ASCII_FORMAT,
  SM_BINARY_FORMAT,
  SM_SW_FORMAT,
  SM_LW_FORMAT
};

static int
_get_format_ (char *format_str)
{
  if (strcmp(format_str, "Ascii") == 0) {
    return SM_ASCII_FORMAT;
  } else if (strcmp(format_str, "Binary") == 0) {
    return SM_BINARY_FORMAT;
  } else if (strcmp(format_str, "HEADER") == 0) {
    return SM_SW_FORMAT;
  } else if (strcmp(format_str, "LastWave") == 0) {
    return SM_LW_FORMAT;
  } else {
    return SM_UNKNOWN_FORMAT;
  }

  /* Never reached */
  return -1;
}

/*
 */
int
load_signal_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int argc,
		     char **argv)    
{
  /* Command line definition */
  char * options[] = {
    "s[s]",
    NULL
  };

  char * help_msg = {
    (" Read a signal from a file.\n"
     "\n"
     "Arguments :\n"
     "  string   - name of the file.\n"
     "  [string] - name of the signal. Default is the file name.\n"
     "\n"
     "Return value :\n"
     "  Name of the signal.")
  };

  /* Command's parameters */
  char *signal_name = 0;
  char *file_name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *signal;
  FILE   *in;
  char   buffer[200]; 
  char   format_str[200]; 
  int    format;
  int    size;
  int    first;
  int    last;
  int    i;
  int    type;
  int    real_size;
  real   dx;
  real   x0;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &file_name, &signal_name)==TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  in = fopen (file_name, "r");
  if (!in) {
    return GenErrorAppend (interp, "Couldn't open `", file_name,
			   "' for reading.", NULL);
  }

  fgets (buffer, 200, in);

  /* Read the format. */
  sscanf (buffer, "%s", format_str);
  format = _get_format_ (format_str);

  /* Read the header. */
  switch (format) {
  case SM_UNKNOWN_FORMAT:
    fclose (in);
    return GenErrorAppend (interp, "unknown format", NULL);
    break;
  case SM_ASCII_FORMAT:
    sscanf (buffer,
	    "%s %d %d %d %d %f %f",
	    format_str, &type, &size, &first, &last, &x0, &dx);
    break;
  case SM_BINARY_FORMAT:
    sscanf (buffer, "%s %d %d %d %d %f %f (%d byte reals)",
	    format_str, &type, &size, &first, &last, &x0, &dx, &real_size);
    break;
  case SM_SW_FORMAT:
  case SM_LW_FORMAT:
    fscanf(in, "size %d ", &size);
    if (!signal_name) {
      /* We get the signal name from the 'name' field in the file */
      fscanf(in, "name %[^\n] ", file_name);
    } else {
      /* Skip to the next line. */
      fgets (buffer, 200, in);
    }
    /* These 2 lines must be modified to catch these 2 values */
    fscanf(in, "firstp %d ", &first);
    fscanf(in, "lastp %d ", &last);
    fscanf(in, "dx %f ", &dx);
    fscanf(in, "x0 %f ", &x0);
    /* Skip the line 'END HEADER'. This line contains no information. */
    fgets (buffer, 200, in);
    /* The following is a 'smurf' convention. Must be modified. */
    if ((dx == 0) && (x0 == 0)) {
      type = REALXY;
    } else {
      type = REALY;
    }
    first = 0;
    last = size-1;
    break;
  }

  /* If no signal name is specified, use the name of the file. This is only 
   * done for non sw files. For sw files the signal name is already set (see
   * above). */
  if (!signal_name) {
    signal_name = argv[1];
  }

  /* Create the signal from header parameters. */
  signal = sig_new (type, 0, last-first);
  if (!signal) {
    fclose (in);
    return GenErrorMemoryAlloc (interp);
  }
  signal->x0 = x0;
  signal->dx = dx;

  /* Read the data */

  switch (format) {
  case SM_ASCII_FORMAT:
    switch (type) {
    case REALY:
      for (i = 0; i < signal->n; i++) {
	fscanf (in, "%f", &signal->dataY[i]);
      }
      break;
    case REALXY:
      for (i = 0; i < signal->n; i++) {
	fscanf (in, "%f %f", &signal->dataX[i], &signal->dataY[i]);
      }
      break;
    case CPLX:
    case FOUR_NR: 
      for (i = 0; i < signal->n; i++) {
	fscanf (in, "%f %f", &signal->dataY[2*i], &signal->dataY[2*i+1]);
      }
      break;
    }
    break;
  case SM_BINARY_FORMAT:
    if (real_size != sizeof (real)) {
      sig_free (signal);
      fclose (in);
      return GenErrorAppend (interp, "size of float data in the file is no correct", NULL);
    }
    switch (type) {
    case REALY:
      fread (signal->dataY, real_size, size, in);
      break;
    case REALXY:
      fread (signal->dataY, real_size, size, in);
      fread (signal->dataX, real_size, size, in);
      break;
    case CPLX:
    case FOUR_NR: 
      fread (signal->dataY, real_size, size, in);
      break;
    }
    break;
  case SM_SW_FORMAT:
  case SM_LW_FORMAT:
    switch (type) {
    case REALY:
      for (i = 0; i < signal->size; i++)
	fscanf (in, "%f", &signal->dataY[i]);
      break;
    case REALXY:
      for (i = 0; i < signal->size; i++)
	fscanf (in, "%f %f", &signal->dataX[i], &signal->dataY[i]);
      break;
    }
    break;
  }

  fclose (in);

  store_signal_in_dictionary (signal_name, signal);

  Tcl_AppendResult (interp, signal_name, NULL);

  return TCL_OK;
}


/*------------------------------------------------------------------------
  SigFileLoadCmd_

  Lecture d'un signal depuis le disque.
  ----------------------------------------------------------------------*/
int
SigFileLoadCmd_(ClientData clientData,
		Tcl_Interp *interp,
		int argc,
		char **argv)      
{ 
  char * options[] = { "s[s]",  
		       "-ascii", "",
		       "-sw", "",
		       "-frap", "s", /* File format for antoine. */
		       NULL};
  
  char   *file_name = NULL;
  char   *signalName     = NULL; 
  char   *signal_piezoName     = NULL; 
  int    i;
  int    size, first, last;
  int    type = -1, realSize;
  real  *data;
  real  *pdata;
  char   tempBuffer[100], saveFormat[10];
  FILE   *fileIn;
  Signal *signal;
  Signal *piezo;
  int    is_sw = 0;
  int    is_frap = 0;
  real dx, x0;
    
  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;

  if (arg_get (0, &file_name, &signalName) == TCL_ERROR)
    return TCL_ERROR;
  
  is_sw = arg_present (2);
  is_frap = arg_present (3);
  if (is_frap)
    if (arg_get (3, &signal_piezoName) == TCL_ERROR)
      return TCL_ERROR;
  

  fileIn = fopen (file_name, "r");
  if (!fileIn)
      return GenErrorAppend (interp, "Couldn't open `", file_name,
			     "' for reading.", NULL);

  if (is_frap)
    {
      real tmp1 = 0, tmp2;

      fgets (tempBuffer, 200, fileIn);
      sscanf(tempBuffer,"%d",&size);
      if (!signalName)
	signalName = file_name;

      type = REALY;

      signal = sig_new (type, 0, size-1);
      if (!signal)
	return GenErrorMemoryAlloc (interp);
      piezo = sig_new (type, 0, size-1);
      if (!piezo)
	return GenErrorMemoryAlloc (interp);

      data = signal->dataY;
      pdata = piezo->dataY;
      for (i = 0; i <= size-1; i++)
	{
	  tmp1 = tmp2;
	  fscanf (fileIn, "%f", &tmp2);
	  fscanf (fileIn, "%g", data+i);
	  fscanf (fileIn, "%g", pdata+i);
	}

      signal->dx = tmp2 - tmp1;
      signal->x0 = 0;
      piezo->dx = tmp2 - tmp1;
      piezo->x0 = 0;

      store_signal_in_dictionary (signal_piezoName, piezo);
    }
  else if (is_sw)
    {
      fgets (tempBuffer, 200, fileIn);
      fscanf(fileIn,"size %d ",&size);
      if (!signalName)
	{
	  signalName = (char *) malloc (sizeof (char)*250);
	  fscanf(fileIn,"name %[^\n] ",signalName);
	}
      else
	fgets (tempBuffer, 200, fileIn);
      fscanf(fileIn,"firstp %d ",&first);
      fscanf(fileIn,"lastp %d ",&last);
      fscanf(fileIn,"dx %f ",&dx);
      fscanf(fileIn,"x0 %f ",&x0);
      fgets (tempBuffer, 200, fileIn);

      type = REALY;

      signal = sig_new (type, 0, size - 1);
      if (!signal)
	return GenErrorMemoryAlloc (interp);
      signal->dx = dx;
      signal->x0 = x0;

      data = signal->dataY;
      for (i = 0; i < size; i++)
	fscanf (fileIn, "%g", data+i);
    }
  else
    {
      if (!signalName)
	signalName = file_name;

      fgets (tempBuffer, 200, fileIn);
      if (arg_present (1))	/* lecture signal ascii */
	sscanf (tempBuffer, "%s %d %d %d %d %f %f",
		saveFormat, &type, &size, &first, &last, &x0, &dx);
      else
	sscanf (tempBuffer, "%s %d %d %d %d %f %f (%d byte reals)",/* %f %f,*/
		saveFormat, &type, &size, &first, &last, &x0, &dx, &realSize);/*, &x0, &dx);*/
      if (type != REALY && type != REALXY)
	return GenErrorAppend (interp, "`", file_name,
			       "' doesn't seem to be an Signal.", NULL);

      signal = sig_new (type, 0, last - first);
      signal->x0 = x0;
      signal->dx = dx;
      if (!signal)
	return GenErrorMemoryAlloc (interp);
  
      data = signal->dataY;
      if (arg_present (1))	/* lecture signal ascii */
	{
	  if (type == REALXY) {
	    for (i = first; i <= last; i++)
	      fscanf (fileIn, "%f %f", signal->dataX+i, data+i);
	  } else {
	    for (i = first; i <= last; i++)
	      fscanf (fileIn, "%f", data+i);
	  }
	}
      else
	{
	  if (realSize != sizeof (real))
	    {
	      sig_free (signal);
	      return GenErrorAppend (interp, "real size problem...", NULL);
	    }
	  fread ((data+first), realSize, size, fileIn);
	}
    }
  fclose (fileIn);
  store_signal_in_dictionary (signalName, signal);
  
  Tcl_AppendResult (interp, signalName, NULL);
  return TCL_OK;
}


#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

static void 
_FourNR_ (float *data,
	int   nn,
	int   isign)
{
  int n, mmax, m, j, istep, i;
  double wtemp, wr, wpr, wpi, wi, theta;
  float tempr, tempi;

  n = nn << 1;
  j = 1;
  for (i = 1; i < n; i += 2)
    {
      if (j > i)
	{
	  SWAP (data[j], data[i]);
	  SWAP (data[j + 1], data[i + 1]);
	}
      m = n >> 1;
      while (m >= 2 && j > m)
	{
	  j -= m;
	  m >>= 1;
	}
      j += m;
    }
  mmax = 2;
  while (n > mmax)
    {
      istep = 2 * mmax;
      theta = 6.28318530717959 / (isign * mmax);
      wtemp = sin (0.5 * theta);
      wpr = -2.0 * wtemp * wtemp;
      wpi = sin (theta);
      wr = 1.0;
      wi = 0.0;
      for (m = 1; m < mmax; m += 2)
	{
	  for (i = m; i <= n; i += istep)
	    {
	      j = i + mmax;
	      tempr = wr * data[j] - wi * data[j + 1];
	      tempi = wr * data[j + 1] + wi * data[j];
	      data[j] = data[i] - tempr;
	      data[j + 1] = data[i + 1] - tempi;
	      data[i] += tempr;
	      data[i + 1] += tempi;
	    }
	  wr = (wtemp = wr) * wpr - wi * wpi + wr;
	  wi = wi * wpr + wtemp * wpi + wi;
	}
      mmax = istep;
    }
}
/*------------------------------------------------------------------------
  Commande qui lance le calcul de la FFT (Numerical Recipes) d'un
  signal reel.
  ----------------------------------------------------------------------*/
int
sig_fft_nr_TclCmd_ (ClientData clientData,
		Tcl_Interp * interp,
		int        argc,
		char       ** argv)
{
  /* Command line definition */
  char * options[] = {
    "Ss" ,
    "-reverse", "",
    "-nb", "d",
    "-speed", "",
    NULL
  };
  
  char * help_msg = {
    (
     "  Fourier transform of a signal, using Numerical Recipes algorithm.\n"
     "\n"
     "Arguments :\n"
     "  signal - Signal to transform.\n"
     "  string - Name of the resulting signal.\n"
     "\n"
     "Return Value :\n"
     "  Internal time used for the computation"
     )
  };

  /* Command's parameters */
  Signal *signal;
  char   *name;

  /* Options's presence */
  int isNb;
  int isSpeed;

  /* Options's parameters */
  int nb = 1;

  /* Other variables */
  Signal *res;
  int    way;
  int    i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &name) == TCL_ERROR)
    return TCL_ERROR;

  if(arg_present (1)) {
    way = REVERSE;
  } else {
    way = DIRECT;
  }

  isNb = arg_present(2);
  if (isNb) {
    if (arg_get (2, &nb) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isSpeed = arg_present(3);

  if (isSpeed) {
    complex *data;
    int size = signal->n;

    res = sig_new (FOUR_NR, 0, signal->n - 1);
    data = (complex *) res->dataY;
    for(i = 0; i < size; i++) {
      data[i].real = signal->dataY[i];
      data[i].imag = 0.0;
    }
	  
    smSetBeginTime();
    for (i = 0; i < nb; i++){
      _FourNR_ ((real *)(data)-1, signal->n, DIRECT);
    }
    smSetEndTime();
  } else {
    smSetBeginTime();
    for (i = 0; i < nb; i++){
      res = sig_fft_nr(signal, way);
    }
    smSetEndTime();
  }

  store_signal_in_dictionary (name, res);

  sprintf(interp->result, "%f", smGetEllapseTime());  
  
  return TCL_OK;
}

/*------------------------------------------------------------------------
  Commande qui lance le calcul de la FFT (Strandh) d'un
  signal reel.
  ----------------------------------------------------------------------*/
int
sig_gfft_TclCmd_ (ClientData clientData,
	      Tcl_Interp * interp,
	      int        argc,
	      char       ** argv)
{
  char * options[] = { "Ss" ,            /* Signal a transformer, resultat */
		       "-reverse", "", /* FFT inverse                    */
		       "-nb", "d",
		       "-cplx", "",
		       "-speed", "",
		       NULL };
  
  Signal *signal, *res = NULL;
  char   *name;
  int    way;
  int    i, nb = 1;
  int    flag = REAL_CPLX;

  int isSpeed;

  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get (0, &signal, &name) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (2, &nb) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_present (1))
    way = REVERSE;
  else
    way = DIRECT;

  if (arg_present (3))
    flag = CPLX_CPLX;

  isSpeed = arg_present(4);

  if (isSpeed) {
    res = sig_new (FOUR_NR, 0, signal->n/2 - 1);
    smSetBeginTime();
    for (i = 0; i < nb; i++){
      gfft_real (signal->dataY, (complex *) res->dataY, signal->n);
    }
    smSetEndTime();
  } else {
    smSetBeginTime();
    for (i = 0; i < nb; i++){
      res = sig_gfft (signal, way, flag);
    }
    smSetEndTime();
  }


  if (!res) {
    sprintf (interp->result, "Errrooooor.\n");
    return TCL_ERROR;
  }

  store_signal_in_dictionary (name, res);

  sprintf(interp->result ,"%f", smGetEllapseTime());

  return TCL_OK;
}

/*------------------------------------------------------------------------
  Commande qui lance le calcul de la FFT split radix d'un
  signal reel.
  ----------------------------------------------------------------------*/
int
sig_srfft_TclCmd_ (ClientData clientData,
	      Tcl_Interp * interp,
	      int        argc,
	      char       ** argv)
{
  char * options[] = { "Ss" ,            /* Signal a transformer, resultat */
		       "-reverse", "", /* FFT inverse                    */
		       "-nb", "d",
		       NULL };
  
  Signal *signal, *res = NULL;
  char   *name;
  int    way, size, i, nb = 1;
  double *x;
  double *x_real, *x_imag;

  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get (0, &signal, &name) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (2, &nb) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_present (1))
    way = REVERSE;
  else
    way = DIRECT;

  size = signal->n;

  switch (signal->type)
    {
    case REALY:
      x = (double *) malloc (sizeof (double) * size);
      x_real = (double *) malloc (sizeof (double) * (size/2 + 1));
      x_imag = (double *) malloc (sizeof (double) * (size/2 + 1));
      res = sig_new (FOUR_NR, 0, size/2);
      for (i = 0; i < size; i++)
	x[i] = (double) signal->dataY[i];
      smSetBeginTime();
      for (i = 0; i < nb; i++)
	{
	  switch (way)
	    {
	    case DIRECT:
	      TransformeFourier (x, size, log2 (size),
				 x_real, x_imag);
	      break;
	    case REVERSE:
	      sprintf(interp->result, "plus tard...\n");
	      break;
	    }
	}
      smSetEndTime();

      for (i = 0; i < size/2+1; i++)
	{
	  res->dataY [2*i] = x_real[i];
	  res->dataY [2*i+1] = x_imag[i];
	}
      free (x_real);
      free (x_imag);

      break;

    case FOUR_NR:
      x_real = (double *) malloc (sizeof (double) * size);
      x_imag = (double *) malloc (sizeof (double) * size);
      res = sig_new (FOUR_NR, 0, size-1);

      for (i = 0; i < size; i++)
	{
	  x_real[i] = (double) signal->dataY[2*i];
	  x_imag[i] = (double) signal->dataY[2*i+1];
	}
      smSetBeginTime();
      for (i = 0; i < nb; i++)
	{
	  switch (way)
	    {
	    case DIRECT:
	      srfft (x_real, x_imag, log2(size));
	      break;
	    case REVERSE:
	      srifft (x_real, x_imag, log2(size));
	      break;
	    }
	}
      smSetEndTime();

      for (i = 0; i < size; i++)
	{
	  res->dataY [2*i] = x_real[i];
	  res->dataY [2*i+1] = x_imag[i];
	}
      free (x_real);
      free (x_imag);

      break;
    }
  store_signal_in_dictionary (name, res);
  
  sprintf(interp->result, "%f", smGetEllapseTime());

  return TCL_OK;
}

/*
 *
 */
int
sig_get_index_TclCmd_ (ClientData clientData,
		       Tcl_Interp * interp,
		       int        argc,
		       char       ** argv)
{
  char * options[] = { "Sf",
			 NULL };

  Signal *signal;
  real   x;
  int    result;

  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get (0, &signal, &x) == TCL_ERROR)
    return TCL_ERROR;

  result = sig_get_index (signal, x);

  sprintf (interp->result, "%d %.15g", result, signal->dataY[result]);

  return TCL_OK;
}


/* **************************************************
 * Command name in xsmurf : sgetxfromy
 * **************************************************/
/*
int
sig_get_xfromy_TclCmd_ (ClientData clientData,
		       Tcl_Interp * interp,
		       int        argc,
		       char       ** argv)
{
  char * options[] = { "Sf",
			 NULL };

  Signal *signal;
  real   x;
  int    result;

  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get (0, &signal, &x) == TCL_ERROR)
    return TCL_ERROR;

  result = sig_get_sfromy (signal, x);

  sprintf (interp->result, "%d %.15g", result, signal->dataY[result]);

  return TCL_OK;
}

*/

/* **************************************************
 * Command name in xsmurf : sgetextr
 * **************************************************/
int
sig_get_extrema_TclCmd_ (ClientData clientData,
			 Tcl_Interp * interp,
			 int        argc,
			 char       ** argv)
{
  char * options[] = {"S",
		      "-index", "dd",
		      "-x", "ff",
		      "-y", "ff",
		      NULL};

  char * help_msg = {
    (" Computes extrema of a signal.\n"
     "\n"
     "Arguments :\n"
     "  string   - name of the file.\n"
     "  [string] - name of the signal. Default is the file name.\n"
     "\n"
     "Return value :\n"
     "  Name of the signal.")
  };


  int     is_index;

  Signal  *signal;
  int     i_min, i_max;
  real    r_min, r_max;
  real    x_min = 1.0, x_max = 0.0;
  real    y_min = 1.0, y_max = 0.0;
  complex c_min, c_max;

  int is_x;
  int is_y;

  int imin = 0;
  int imax = 0;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal) == TCL_ERROR)
    return TCL_ERROR;

  i_min = 0;
  i_max = signal->n;

  is_index = arg_present (1);
  if (is_index)
    if (arg_get (1, &i_min, &i_max) == TCL_ERROR)
      return TCL_ERROR;

  is_x = arg_present (2);
  if (is_x)
    if (arg_get (2, &x_min, &x_max) == TCL_ERROR)
      return TCL_ERROR;

  is_y = arg_present (3);
  if (is_y)
    if (arg_get (3, &y_min, &y_max) == TCL_ERROR)
      return TCL_ERROR;

  switch (signal->type)
    {
    case REALY:
      sig_get_extrema_between(signal,
			      i_min, i_max,
			      &r_min, &r_max, &imin, &imax);
      sprintf (interp->result,
	       "%.15g %.15g %d %d",
	       r_min, r_max, imin, imax);
      break;
    case REALXY:
      sig_get_xy_extrema (signal,
                          &x_min, &x_max,
                          &y_min, &y_max,&imin,&imax);
      sprintf (interp->result,
               "%.15g %.15g %.15g %.15g %d %d",
               x_min, x_max, y_min, y_max,imin,imax);
      break;
    case CPLX:
    case FOUR_NR:
      sig_get_extrema_between(signal,
			      i_min, i_max,
			      &c_min, &c_max,&imin,&imax);
      sprintf (interp->result, "%.15g %.15g %d %d",
	       min(c_min.real, c_min.imag),
	       max(c_max.real, c_max.imag), imin, imax);
      break;
    }

  /*  switch (signal->type)
    {
    case REALY:
      sig_get_extrema_between(signal,
			      i_min, i_max,
			      &r_min, &r_max);
      sprintf (interp->result, "%.15g %.15g", r_min, r_max);
      break;
    case REALXY:
      sig_get_xy_extrema (signal,
			  &x_min, &x_max,
			  &y_min, &y_max);
      sprintf (interp->result, "%.15g %.15g %.15g %.15g", x_min, x_max, y_min, y_max);
      break;
    case CPLX:
    case FOUR_NR:
      sig_get_extrema_between(signal,
			      i_min, i_max,
			      &c_min, &c_max);
      sprintf (interp->result, "%.15g %.15g",
	       min(c_min.real, c_min.imag),
	       max(c_max.real, c_max.imag));
      break;
    }*/

  return TCL_OK;
}

/********* Commandes temporaires de teste de REALXY *********/

/*
 * Ceci est une commande temporaire qui construit un signal de type
 * REALXY a la main, pour pouvoir tester tous les cas des fonctions
 * sur les signaux.
 */
int
create_xy_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{
  char * options[] = { "sd",     /* nom et taille du signal           */
			 NULL };

  char * help_msg =
  {("Create a REALXY signal (name size) for test purpose.")};

  Signal *signal;
  char   *name;
  int    size;
  int    i;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &name, &size) == TCL_ERROR)
    return TCL_ERROR;

  signal = sig_new (REALXY, 0, size - 1);
  if (!signal)
    {
      sprintf (interp->result, "No free memory space.");
      return TCL_ERROR;
    }

  for (i = 0; i < size; i++)
    {
      signal->dataX [i] = 3.01*i + (i%2+1)*1.43;
      signal->dataY [i] = i*.31;
    }

  store_signal_in_dictionary(name,signal);
  
  Tcl_AppendResult(interp,name,NULL);

  return TCL_OK;
}

int
display_realxy_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  char * options[] = { "S",
			 NULL };

  char * help_msg =
  {("Display a REALXY signal (name) for test purpose.")};

  Signal *signal;
  int    i;
  char   string[20];

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &signal) == TCL_ERROR)
    return TCL_ERROR;

  for (i = 0; i < signal->size; i++)
    {
      sprintf (string, "%8g %8g",
	       signal->dataX [i],
	       signal->dataY [i]);
      Tcl_AppendElement (interp, string);
    }

  return TCL_OK;
}


int
real_real_to_complex_TclCmd_ (ClientData clientData,
			      Tcl_Interp *interp,
			      int        argc,
			      char       **argv)
{
  char * options[] = { "SSs",
			 NULL };

  char * help_msg =
  {("Create a complex signal with 2 real signals.")};

  Signal *sig_1, *sig_2, *result;
  char   *name;
  int    i;
  complex *c_data;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &sig_1, &sig_2, &name) == TCL_ERROR)
    return TCL_ERROR;

  if (sig_1->type != REALY || sig_2->type != REALY)
    {
      sprintf (interp->result, "Signals data must be real data.");
      return TCL_ERROR;
    }

  if (sig_1->n != sig_2->n
      || sig_1->x0 != sig_2->x0
      || sig_1->dx != sig_2->dx
      || sig_1->type != sig_2->type) {
      sprintf (interp->result, "Signals don't have the same parameters.");
      return TCL_ERROR;
    }

  result = sig_new (CPLX, 0, sig_1->n - 1);
  result->x0 = sig_1->x0;
  result->dx = sig_1->dx;

  c_data = (complex *) result->dataY;

  for (i = 0; i < sig_1->n; i++)
    {
      c_data[i].real = sig_1->dataY[i];
      c_data[i].imag = sig_2->dataY[i];
    }

  store_signal_in_dictionary (name, result);

  return TCL_OK;
}

/* ***********************************************
   command name in xsmurf : screate
   ******************************************** */
int
create_signal_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "sffs",
    "-xy", "s",
    NULL
  };

  char * help_msg =
  {
    (" Create a signal.\n"
     "\n"
     "Parameters :\n"
     "  string - name of the signal.\n"
     "  float  - absciss of the first point (x0).\n"
     "  float  - step between points (dx).\n"
     "  string - the Tcl list of the points's values.\n"
     "Options :\n"
     "  -xy : The signal is an REAL_XY signal.\n"
     "    string - the Tcl list of the points's absciss values.")
  };

  /* Command's parameters */
  char *sig_name;
  real x0;
  real dx;
  char *y_lst_str;

  /* Options's presence */
  int is_xy;

  /* Options's parameters */
  char *x_lst_str;

  /* Other variables */
  Signal *signal;
  int    x_lst_size, y_lst_size, code;
  char   **y_elt;
  char   **x_elt;
  double dbl_val;
  int    i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &sig_name, &x0, &dx, &y_lst_str) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  code = Tcl_SplitList(interp, y_lst_str, &y_lst_size, (CONST char ***)&y_elt);
  if (code == TCL_ERROR)
    return TCL_ERROR;

  is_xy = arg_present (1);
  if (is_xy)
    {
      if (arg_get (1, &x_lst_str) == TCL_ERROR)
	return TCL_ERROR;
      code = Tcl_SplitList(interp, x_lst_str, &x_lst_size, (CONST char ***)&x_elt);
      if (code == TCL_ERROR)
	{
	  Tcl_Free((char *) y_elt);
	  return TCL_ERROR;
	}
      if (x_lst_size != y_lst_size)
	{
	  sprintf (interp->result, "The 2 lists must have the same length");
	  Tcl_Free((char *) y_elt);
	  Tcl_Free((char *) x_elt);
	  return TCL_ERROR;
	}
      signal = sig_new(REALXY, 0, y_lst_size - 1);
    } else {
      signal = sig_new(REALY, 0, y_lst_size - 1);
      signal->x0 = x0;
      signal->dx = dx;
    }

  /* Treatement */
  for (i = 0; i < y_lst_size; i++)
    {
      if (Tcl_GetDouble (interp, y_elt[i], &dbl_val) == TCL_ERROR)
	{
	  sig_free (signal);
	  return TCL_ERROR;
	}
      signal->dataY[i] = dbl_val;
    }
  Tcl_Free((char *) y_elt);
  if (is_xy)
    {
      for (i = 0; i < x_lst_size; i++)
	{
	  if (Tcl_GetDouble (interp, x_elt[i], &dbl_val) == TCL_ERROR)
	    {
	      sig_free (signal);
	      return TCL_ERROR;
	    }
	  signal->dataX[i] = dbl_val;
	}
      Tcl_Free((char *) x_elt);
    }

  store_signal_in_dictionary (sig_name, signal);

  return TCL_OK;
}


/*
  Command name in xsmurf: scomb
 */
int
comb_signals_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "SSss",
    "-xnull", "",
    "-ynull", "",
    NULL
  };

  char * help_msg =
  {
    (" Create a signal as a combination of two signals. If the signals are "
     "REALXY, the result gets the x-values of the first signal. \n"
     "\n"
     "Parameters :\n"
     "  2 Signals - signals to combine.\n"
     "  string    - expression of the combination (defunc expression). The\n"
     "              signals are designed by the tokens 'x' and 'y' (ex.\n"
     "              x*cos(y) compute the first signal times the cosine of the\n"
     "              second one.\n"
     "  string    - name of the result.\n"
     "\n"
     "Options :\n"
     "  -xnull : a null value in the first source gives a null value in the result.\n"
     "  -ynull : a null value in the second source gives a null value in the result.")
  };

  /* Command's parameters */
  Signal *signal1, *signal2;
  char   *fct_expr;
  char   *res_name;

  /* Options's presence */
  int is_xnull;
  int is_ynull;

  /* Options's parameters */

  /* Other variables */
  Signal *result;
  /*double (*fct)();*/
  void *fct;
  int    i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal1, &signal2, &fct_expr, &res_name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (signal1->type != signal2->type)
    {
      sprintf (interp->result,"signals must have the same type\n");
      return TCL_ERROR;
    }
  if ((signal1->type != REALY) && (signal1->type != REALXY)) {
    sprintf (interp->result, "signals must be REALY or REALXY\n");
    return TCL_ERROR;
  }
  if (!same_x_domain (signal1, signal2))
    {
      sprintf (interp->result,"signals must have the same x domain\n");
      return TCL_ERROR;
    }
  /*fct = dfopen (fct_expr);*/
  fct = evaluator_create(fct_expr);
  if (!fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  is_xnull = arg_present(1);
  is_ynull = arg_present(2);

  /* Treatement */
  result = sig_new(signal1->type, 0, signal1->n - 1);
  result->x0 = signal1->x0;
  result->dx = signal1->dx;
  for (i = 0; i < result->n; i++) {
    if ((is_xnull && signal1->dataY[i] == 0.0)
	|| (is_ynull && signal2->dataY[i] == 0.0)) {
      result->dataY[i] = 0.0;
    } else {
      result->dataY[i] = evaluator_evaluate_x_y(fct, signal1->dataY[i], signal2->dataY[i]);
    }
    if (signal1->type == REALXY) {
      result->dataX[i] = signal1->dataX[i];
    }
  }
  /*dfclose (fct);*/
  evaluator_destroy(fct);
  store_signal_in_dictionary (res_name, result);

  return TCL_OK;
}


/*
  command name in xsmurf : sfit
 */
int
fit_signal_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "S[ff]",
    NULL
  };

  char * help_msg =
  {
    (" Fit a signal by a line. The signal must be REALY or REALXY.\n"
     "\n"
     "Parameters :\n"
     "  Signal     - signals to fit.\n"
     "  [2 floats] - x domain to fit.\n"
     "\n"
     "Return value:\n"
     "  - slope\n"
     "  - intercept\n"
     "  - SigA\n"
     "  - SigB\n"
     "  - chi2\n")
  };

  /* Command's parameters */
  Signal *signal;
  float  x_min = 1;
  float  x_max = -1;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  double a, b;
  real   dx, x0;
  int    size;
  int    nb_of_points = 0;
  int    i;
  float  chi2, tmp;
  float  t;
  float  x_sum = 0.0, y_sum = 0.0, st2 = 0.0, sigdat;
  float  SigA, SigB;
  real   x;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &x_min, &x_max) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  if ((signal->type != REALXY) && (signal->type != REALY)) {
    sprintf (interp->result, "bad signal type");
    return TCL_ERROR;
  }

  if (x_max < x_min) {
    x_min = signal->x0;
    x_max = signal->x0 + signal->size*signal->dx;
  }

  /* Treatement */

  dx = signal->dx;
  x0 = signal->x0;
  size = signal->size;

  for (i = 0; i < size; i++) {
    if (signal->type == REALXY)
      x = signal->dataX[i];
    else
      x = i*dx + x0;
    if (x >= x_min && x <= x_max)
      {
	x_sum += x;
	y_sum += signal->dataY[i];
	nb_of_points++;
      }
  }
  a = 0.0;
  for (i = 0; i < size; i++){
    if (signal->type == REALXY)
      x = signal->dataX[i];
    else
      x = i*dx + x0;
    if (x >= x_min && x <= x_max)
      {
	t = x - x_sum/nb_of_points;
	st2 += t*t;
	a += t * signal->dataY[i];
      }
  }
  a /= st2;
  b = (y_sum-x_sum*a)/nb_of_points;
  SigB = sqrt ((1.0 + x_sum*x_sum/(size*st2))/nb_of_points);
  SigA =  sqrt (1.0/st2);
  chi2 = 0.0;
  for (i = 0; i < size; i++)
    {
      if (signal->type == REALXY)
	x = signal->dataX[i];
	  else
	    x = i*dx + x0;
      if (x >= x_min && x <= x_max)
	{
	  tmp = signal->dataY[i] - b - a*x;
	  chi2 += tmp*tmp;
	}
    }
  sigdat = sqrt (chi2/(nb_of_points-2));
  SigB *= sigdat;
  SigA *= sigdat;

  sprintf (interp->result, "%.15e %.15e %.15e %.15e %.15e",
	   a, b, SigA, SigB, sigdat);  

  return TCL_OK;
}

/***********************************
 * command name in xsmurf : my_sfit
 ***********************************/
int
my_fit_signal_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "S[ff]",
    NULL
  };

  char * help_msg =
  {
    (" Fit a signal by a line. The signal must be REALY or REALXY.\n"
     "\n"
     "Parameters :\n"
     "  Signal     - signals to fit.\n"
     "  [2 floats] - x domain to fit.\n"
     "\n"
     "Return value:\n"
     "  - slope\n"
     "  - Sigma\n")
  };

  /* Command's parameters */
  Signal *signal;
  float  x_min = 1;
  float  x_max = -1;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  double a, b;
  real   dx, x0;
  int    size;
  int    nb_of_points = 0;
  int    i;
  float  chi2, tmp;
  float  t;
  float  x_sum = 0.0, y_sum = 0.0, st2 = 0.0, sigdat;
  float  SigA, SigB;
  real   x;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &x_min, &x_max) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  if ((signal->type != REALXY) && (signal->type != REALY)) {
    sprintf (interp->result, "bad signal type");
    return TCL_ERROR;
  }

  if (x_max < x_min) {
    x_min = signal->x0;
    x_max = signal->x0 + signal->size*signal->dx;
  }

  /* Treatement */

  dx = signal->dx;
  x0 = signal->x0;
  size = signal->size;

  for (i = 0; i < size; i++) {
    if (signal->type == REALXY)
      x = signal->dataX[i];
    else
      x = i*dx + x0;
    if (x >= x_min && x <= x_max)
      {
	x_sum += x;
	y_sum += signal->dataY[i];
	nb_of_points++;
      }
  }
  a = 0.0;
  for (i = 0; i < size; i++){
    if (signal->type == REALXY)
      x = signal->dataX[i];
    else
      x = i*dx + x0;
    if (x >= x_min && x <= x_max)
      {
	t = x - x_sum/nb_of_points;
	st2 += t*t;
	a += t * signal->dataY[i];
      }
  }
  a /= st2;
  b = (y_sum-x_sum*a)/nb_of_points;
  SigB = sqrt ((1.0 + x_sum*x_sum/(size*st2))/nb_of_points);
  SigA =  sqrt (1.0/st2);
  chi2 = 0.0;
  for (i = 0; i < size; i++)
    {
      if (signal->type == REALXY)
	x = signal->dataX[i];
	  else
	    x = i*dx + x0;
      if (x >= x_min && x <= x_max)
	{
	  tmp = signal->dataY[i] - b - a*x;
	  chi2 += tmp*tmp;
	}
    }
  sigdat = sqrt (chi2/(nb_of_points-2));
  SigB *= sigdat;
  SigA *= sigdat;

  sprintf (interp->result, "%.3f %.3f",
	   a, SigA);  

  return TCL_OK;
}


/*
 */
int
sig_zoom_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ssd",
    NULL
  };

  char * help_msg =
  {
    (" Apply a kind of zoom on a signal. Create a new signal where each points\n"
     "contains the sum of the value of the points that are on a box (if you\n"
     "see what I mean).\n"
     "\n"
     "Parameters :\n"
     "  signal  - signal to treat.\n"
     "  string  - the name of the new signal.\n"
     "  integer - value by wich you divide the original size to get the new\n"
     "            one.")
  };

  /* Command's parameters */
  Signal *signal;
  char   *name;
  int    div;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *new_sig;
  int    x;
  int    new_x;
  int    new_size;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &name, &div) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (signal->size % div != 0)
    {
      sprintf (interp->result, "%d must divide %d.", div, signal->size);
      return TCL_ERROR;
    }

  /* Treatement */
  new_size = signal->size / div;

  new_sig = sig_new (REALY, 0,new_size-1);
  new_sig->dx = signal->dx*div;
  new_sig->x0 = signal->x0 + (div-1)/2*signal->dx;
  for (x = 0; x < new_sig->size; x++) {
    new_sig->dataY[x] = 0;
  }

  for (new_x = 0; new_x < new_size; new_x++)
    for (x = new_x*div; x < (new_x+1)*div; x++)
      new_sig->dataY[new_x] += signal->dataY[x];

  store_signal_in_dictionary (name, new_sig);

  return TCL_OK;
}




/*
 */
int
sig_colle2sig_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "SSs",
    NULL
  };

  char * help_msg =
  {
    (" merge two signal\n"
     "\n"
     "Parameters :\n"
     "  signal  - first signal to treat.\n"
     "  signal  - second signal to treat.\n"
     "  string  - the name of the new signal.\n")
  };

  /* Command's parameters */
  Signal *signal1, *signal2;
  char   *name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *new_sig;
  int    x;
  int    new_size;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal1, &signal2, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Treatement */
  new_size = signal1->size + signal2->size;

  new_sig = sig_new (REALXY, 0,new_size-1);
  for (x = 0; x < new_sig->size; x++) {
    if (x < signal1->size) {
      if (signal1->type == REALY) {
	new_sig->dataX[x]= signal1->x0+x*signal1->dx; 
      } else {
	new_sig->dataX[x]= signal1->dataX[x]; 
      }
      new_sig->dataY[x] = signal1->dataY[x];
    } else {
      if (signal2->type == REALY) {
	new_sig->dataX[x]= signal2->x0+(x-signal1->size)*signal2->dx; 
      } else {
	new_sig->dataX[x]= signal2->dataX[x-signal1->size]; 
      }
      new_sig->dataY[x] = signal2->dataY[x-signal1->size];
    }
    
  }
  


  store_signal_in_dictionary (name, new_sig);

  return TCL_OK;
}


/* **********************************************
 *  command name in xsmurf : sgetlst
 * **********************************************/
int
sig_get_value_lst_TclCmd_ (ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "S",
    "-x", "",
    "-y", "",
    NULL
  };

  char * help_msg =
  {
    (" Return the list of the values of a signal.\n"
     "\n"
     "Parameters :\n"
     "  signal - signal to treat.\n"
     "\n"
     "Options :\n"
     "  -x : Get the values of x's.\n"
     "  -y : this option is only valid used for XY signals\n"
     "Return value:\n"
     " list of values.\n")
  };

  /* Command's parameters */
  Signal *signal;

  /* Options's presence */
  int isX;
  int isY;

  /* Options's parameters */

  /* Other variables */
  int  i;
  char val_str[30];

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal) == TCL_ERROR)
    return TCL_ERROR;

  isX = arg_present(1);
  isY = arg_present(2);

  /* Parameters validity and initialisation */

  /* Treatement */
  switch (signal->type) {
  case REALY:
    if (isX) {
      for (i = 0; i < signal->size; i++) {
	sprintf(val_str, "%.15g", signal->x0+signal->dx*i);
	Tcl_AppendElement(interp, val_str);
      }
    } else {
      for (i = 0; i < signal->size; i++) {
	sprintf(val_str, "%.15g", signal->dataY[i]);
	Tcl_AppendElement(interp, val_str);
      }
    }
    break;
  case REALXY:
    if (isX) {
      for (i = 0; i < signal->size; i++) {
	sprintf(val_str, "%.15g", signal->dataX[i]);
	Tcl_AppendElement(interp, val_str);
      }
    } else if (isY) { 
     for (i = 0; i < signal->size; i++) {
        sprintf(val_str, "%g", signal->dataY[i]);
        Tcl_AppendElement(interp, val_str);
     } 
    } else {
      for (i = 0; i < signal->size; i++) {
	sprintf(val_str, "%.15g %.15g", signal->dataX[i], signal->dataY[i]);
	Tcl_AppendElement(interp, val_str);
      }
    }
    break;
  default:
    sprintf (interp->result, "wrong signal type");
    return TCL_ERROR;
    break;
  }
  return TCL_OK;
}


#define _x_convert_(real_x) (xPos+(int)((real)(width)*((real_x)-xMin)/(xMax-xMin)))
#define _y_convert_(real_y) (yPos+height-(int)((real)(height)*((real_y)-yMin)/(yMax-yMin)))
#define _is_in_domain_(x,y) ((x)>=xMin&&(x)<=xMax&&(y)>=yMin&&(y)<=yMax)

static real
_id_ (real r)
{
  return r;
}

static real
_log_ (real r)
{
  return log(r);
}

/*
  command name in xsmurf : sig2coords
 */
int
sig_2_coords_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "Sddddffff",
    "-imagpart", "",
    "-xlog", "",
    "-ylog", "",
    NULL
  };

  char * help_msg = {
    (" Return the list of the coordinates of the points of a signal that are\n"
     "in a discrete box.\n"
     "\n"
     "Parameters :\n"
     "  signal     - signal to treat (must be REALY).\n"
     "  2 integers - position of the box.\n"
     "  integer    - width of the box.\n"
     "  integer    - height of the box.\n"
     "  4 floats   - the real dimension of the box (x minimum and maximum, y\n"
     "minimum and maximum)."
     "\n"
     "Options :\n"
     "  -imagpart : If the signal is CPLX or FOUR_NR type, return the "
     "imaginary part.\n"
     "  -xlog : Use logaritmic display for the x axis.\n"
     "  -ylog : Use logaritmic display for the y axis.")
  };

  /* Command's parameters */
  Signal *signal;
  int  xPos;
  int  yPos;
  int  width;
  int  height;
  real xMin;
  real xMax;
  real yMin;
  real yMax;

  /* Options's presence */
  int isImagpart;
  int isXlog;
  int isYlog;

  /* Options's parameters */

  /* Other variables */
  int  i;
  char val_str[30];
  int  x;
  int  y;
  int  prev_x;
  int  prev_y;
  real x0;
  real dx;
  /* This offset is set to 1 to access imaginary part of the data in the signal
   * array. 0 is to access even index, 1 is to access odd index.*/
  int  offset = 0;
  real xReal;
  real yReal;
  real (*theXFct)(real);
  real (*theYFct)(real);

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &xPos, &yPos,
	       &width, &height,
	       &xMin, &xMax, &yMin, &yMax) == TCL_ERROR)
    return TCL_ERROR;

  isImagpart = arg_present (1);
  isXlog = arg_present (2);
  isYlog = arg_present (3);

  /* Parameters validity and initialisation */
  if ( width < 0 ) {
    sprintf( interp->result, "The width (%d) must be positive.", width );
    return TCL_ERROR;
  }
  if ( height < 0 ) {
    sprintf( interp->result, "The height (%d) must be positive.", width );
    return TCL_ERROR;
  }
  if ( xMin > xMax ) {
    sprintf( interp->result,
	     "The minimum in x (%f) must be lesser than the maximum (%f).",
	     xMin, xMax );
    return TCL_ERROR;
  }
  if ( yMin > yMax ) {
    sprintf( interp->result,
	     "The minimum in y (%f) must be lesser than the maximum (%f).",
	     yMin, yMax );
    return TCL_ERROR;
  }

  /* Treatement */

  if (isXlog) {
    theXFct = _log_;
  } else {
    theXFct = _id_;
  }
  if (isYlog) {
    theYFct = _log_;
  } else {
    theYFct = _id_;
  }

  switch (signal->type) {
  case REALY:
    x0 = signal->x0;
    dx = signal->dx;
    for (i = 0; i < signal->size; i++) {
      xReal = theXFct (x0+i*dx);
      yReal = theYFct (signal->dataY[i]);
      if (_is_in_domain_(xReal, yReal)) {
	x = _x_convert_ (xReal);
	y = _y_convert_ (yReal);
	if ((x != prev_x) || (y != prev_y)) {
	  /* If it's the same point as before, we don't put it again in the 
	   * resulting list. This is to avoid too long list where hundreds of
	   * following points are the same. */
	  sprintf(val_str, "%d", x);
	  Tcl_AppendElement(interp, val_str);
	  sprintf(val_str, "%d", y);
	  Tcl_AppendElement(interp, val_str);
	}
	prev_x = x;
	prev_y = y;
      }
    }
    break;
  case REALXY:
    for (i = 0; i < signal->size; i++) {
      xReal = theXFct (signal->dataX[i]);
      yReal = theYFct (signal->dataY[i]);
      if (_is_in_domain_(xReal, yReal)) {
	x = _x_convert_ (xReal);
	y = _y_convert_ (yReal);
	if ((x != prev_x) || (y != prev_y)) {
	  /* If it's the same point as before, we don't put it again in the 
	   * resulting list. This is to avoid too long list where hundreds of
	   * following points are the same. */
	  sprintf(val_str, "%d", x);
	  Tcl_AppendElement(interp, val_str);
	  sprintf(val_str, "%d", y);
	  Tcl_AppendElement(interp, val_str);
	}
	prev_x = x;
	prev_y = y;
      }
    }
    break;
  case CPLX:
  case FOUR_NR:
    if (isImagpart) {
      /* We want to access to the imaginary part of the data. */
      offset = 1;
    }
    x0 = signal->x0;
    dx = signal->dx;
    for (i = 0; i < signal->size/2; i++) {
      xReal = theXFct (x0+i*dx);
      yReal = theYFct (signal->dataY[2*i+offset]);
      if (_is_in_domain_(xReal, yReal)) {
	x = _x_convert_ (xReal);
	y = _y_convert_ (yReal);
	if ((x != prev_x) || (y != prev_y)) {
	  /* If it's the same point as before, we don't put it again in the 
	   * resulting list. This is to avoid too long list where hundreds of
	   * following points are the same. */
	  sprintf(val_str, "%d", x);
	  Tcl_AppendElement(interp, val_str);
	  sprintf(val_str, "%d", y);
	  Tcl_AppendElement(interp, val_str);
	}
	prev_x = x;
	prev_y = y;
      }
    }
    break;
  }

  return TCL_OK;
}


/*
 */
int
sig_gfft_to_2real_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Sss",
    NULL
  };

  char * help_msg =
  {
    (" Transfrom an gfft-format signal into 2 signals (real part and imaginary\n"
     "part).\n"
     "\n"
     "Parameters :\n"
     "  Signal    - signal to treat.\n"
     "  2 strings - names of the results (real and imaginary, respectivly).")
  };

  /* Command's parameters */
  Signal  *source;
  char   *real_name;
  char   *imag_name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal  *res_real;
  Signal  *res_imag;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &source, &real_name, &imag_name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  res_real = sig_new(REALY, 0,source->size-1);
  res_imag = sig_new(REALY, 0,source->size-1);
  res_real->x0 = -source->size/2*source->dx;
  res_real->dx = source->dx;
  res_imag->x0 = -source->size/2*source->dx;
  res_imag->dx = source->dx;

  sig_gfft_to_2real (source, res_real, res_imag);

  store_signal_in_dictionary (real_name, res_real);
  store_signal_in_dictionary (imag_name, res_imag);

  return TCL_OK;
}


/*
 */
int
sig_2real_to_gfft_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "SSs",
    NULL
  };

  char * help_msg =
  {
    (" Transfrom 2 signals (real part and imaginary part) into an gfft-format\n"
     " signal.\n"
     "\n"
     "Parameters :\n"
     "  2 Signals - Signals to treat.\n"
     "  string    - Name of the result.")
  };

  /* Command's parameters */
  Signal *rSig;
  Signal *iSig;
  char   *resName;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal  *res;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &rSig, &iSig, &resName) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  res = sig_new(REALY, 0, rSig->n-1);
  res->x0 = 0;
  res->dx = rSig->dx;

  sig_2real_to_gfft (res, rSig, iSig);

  store_signal_in_dictionary (resName, res);

  return TCL_OK;
}


/*
 */
int
sig_integration_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ss",
    NULL
  };

  char * help_msg =
  {
    (" Integrate a signal.\n"
     "\n"
     "Parameters :\n"
     "  Signal - signal to treat.\n"
     "  string - names of the result.")
  };

  /* Command's parameters */
  Signal *source;
  char   *res_name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal  *result;
  int i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &source, &res_name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  result = sig_new(REALY, 0,source->size-1);
  result->x0 = source->x0;
  result->dx = source->dx;

  result->dataY[0] = source->dataY[0];
  for (i = 1; i < result->size; i++) {
    result->dataY[i] = result->dataY[i-1] + source->dataY[i];
  }

  store_signal_in_dictionary (res_name, result);

  return TCL_OK;
}


/*
 */
int
sig_get_info_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "S",
    NULL
  };

  char * help_msg =
  {
    (" Get info about a signal as a list. The list has the following order : type, size, x0, dx.\n"
     "\n"
     "Parameters :\n"
     "  Signal - signal to treat.")
  };

  /* Command's parameters */
  Signal *sig;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  char string[100];
  char *type[] = {"REALXY", "REALY", "CPLX", "FOUR_NR"};


  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &sig) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  sprintf(string, "%s", type[sig->type]);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%d", sig->size);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%.15g", sig->x0);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%.15g", sig->dx);
  Tcl_AppendElement (interp, string);

  return TCL_OK;
}


/*
 */
int
fct_signal_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ssss",
    "-xnull", "",
    "-ynull", "",
    NULL
  };

  char * help_msg =
  {
    (" Create a signal from the values of the points of a signal.\n"
     "\n"
     "Parameters :\n"
     "  Signal    - source signal.\n"
     "  string    - name of the result.\n"
     "  2 strings - expression of the functions (defunc expression) that "
     "create the new signals. The first function is for x and the second for "
     "y. In these functions x and y correspond to the coordinates of point "
     "from the source signal.")
  };

  /* Command's parameters */
  Signal *signal;
  char   *x_fct_expr;
  char   *y_fct_expr;
  char   *res_name;

  /* Options's presence */
  int is_xnull;
  int is_ynull;

  /* Options's parameters */

  /* Other variables */
  Signal *result;
  double (*x_fct)();
  double (*y_fct)();
  int    i;
  real x;
  real y;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &res_name, &x_fct_expr, &y_fct_expr) == TCL_ERROR)
    return TCL_ERROR;

  is_xnull = arg_present(1);
  is_ynull = arg_present(2);

  /* Parameters validity and initialisation */
  if ((signal->type != REALXY) && (signal->type != REALY)) {
    sprintf (interp->result, "bad signal type");
    return TCL_ERROR;
  }

  /*x_fct = dfopen (x_fct_expr);*/
  x_fct = evaluator_create(x_fct_expr);
  if (!x_fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			x_fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  /*y_fct = dfopen (y_fct_expr);*/
  y_fct = evaluator_create(y_fct_expr);
  if (!y_fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			y_fct_expr, (char *) NULL);
      return TCL_ERROR;
    }

  /* Treatement */
  result = sig_new(REALXY, 0, signal->n - 1);
  result->x0 = signal->x0;
  result->dx = signal->dx;
  for (i = 0; i < result->n; i++) {
    if (signal->type == REALY) {
      x = signal->x0 + i*signal->dx;
    } else {
      x = signal->dataX[i];
    }
    y = signal->dataY[i];

    if (is_xnull && (x == 0.0 || y == 0.0)) {
      result->dataX[i] = 0.0;
    } else {
      result->dataX[i] = evaluator_evaluate_x_y(x_fct,x, y);
    }
    if (is_ynull && (x == 0.0 || y == 0.0)) {
      result->dataY[i] = 0.0;
    } else {
      result->dataY[i] = evaluator_evaluate_x_y(y_fct,x, y);
    }
  }
  /*dfclose (x_fct);
    dfclose (y_fct);*/
  evaluator_destroy(x_fct);
  evaluator_destroy(y_fct);
  store_signal_in_dictionary (res_name, result);
  return TCL_OK;
}


/*
  Command name in xsmurf: foreachs or sigloop
 */
int
foreachs_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ss",
    NULL
  };

  char * help_msg =
  {
    (" Execute a script foreach point of a signal. In this script the "
     "variables x and y refer to the coordinates of the point if signal is "
     "not complex. yReal and yImag refer to complex points.\n"
     "\n"
     "Parameters :\n"
     "  Signal - signal to treat.\n"
     "  string - script to execute.")
  };

  /* Command's parameters */
  Signal *signal;
  char   *scriptStr;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int result;
  int sigInd;
  int nbOfPoints;
  Tcl_Obj *xStrObj, *xObj;
  Tcl_Obj *yStrObj, *yObj;
  Tcl_Obj *yRealStrObj, *yRealObj;
  Tcl_Obj *yImagStrObj, *yImagObj;
  real x, y, yReal, yImag;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &scriptStr) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  sigInd = 0;

  xStrObj = Tcl_NewStringObj("x", 1);
  xObj = Tcl_NewDoubleObj(0);
  switch (signal->type) {
  case REALY:
  case REALXY:
    yStrObj = Tcl_NewStringObj("y", 1);
    yObj = Tcl_NewDoubleObj(0);
    nbOfPoints = signal->size;
    break;
  case CPLX:
  case FOUR_NR:
    yRealStrObj = Tcl_NewStringObj("yReal", 5);
    yRealObj = Tcl_NewDoubleObj(0.0);
    yImagStrObj = Tcl_NewStringObj("yImag", 5);
    yImagObj = Tcl_NewDoubleObj(0.0);
    nbOfPoints = signal->size/2;
    break;
  }

  while (1) {
    if (sigInd == nbOfPoints) {
      result = TCL_OK;
      break;
    }
    switch (signal->type) {
    case REALY:
      x = signal->x0 + sigInd*signal->dx;
      y = signal->dataY[sigInd];
      Tcl_SetDoubleObj(xObj, x);
      Tcl_ObjSetVar2(interp, xStrObj, NULL, xObj, 0);
      Tcl_SetDoubleObj(yObj, y);
      Tcl_ObjSetVar2(interp, yStrObj, NULL, yObj, 0);
      break;
    case REALXY:
      x = signal->dataX[sigInd];
      y = signal->dataY[sigInd];
      Tcl_SetDoubleObj(xObj, x);
      Tcl_ObjSetVar2(interp, xStrObj, NULL, xObj, 0);
      Tcl_SetDoubleObj(yObj, y);
      Tcl_ObjSetVar2(interp, yStrObj, NULL, yObj, 0);
      break;
    case CPLX:
    case FOUR_NR:
      x = signal->x0 + sigInd*signal->dx;
      yReal = signal->dataY[2*sigInd];
      yImag = signal->dataY[2*sigInd+1];
      Tcl_SetDoubleObj(xObj, x);
      Tcl_ObjSetVar2(interp, xStrObj, NULL, xObj, 0);
      Tcl_SetDoubleObj(yRealObj, yReal);
      Tcl_ObjSetVar2(interp, yRealStrObj, NULL, yRealObj, 0);
      Tcl_SetDoubleObj(yImagObj, yImag);
      Tcl_ObjSetVar2(interp, yImagStrObj, NULL, yImagObj, 0);
      break;
    }

    result = Tcl_Eval(interp, scriptStr);
    if ((result != TCL_OK) && (result != TCL_CONTINUE)) {
      if (result == TCL_ERROR) {
	char msg[60];
	sprintf(msg, "\n    (\"foreachs\" body line %d)",interp->errorLine);
	Tcl_AddErrorInfo(interp, msg);
      }
      break;
    }
    sigInd++;
  }

  if (result == TCL_BREAK) {
    result = TCL_OK;
  }
  if (result == TCL_OK) {
    Tcl_ResetResult(interp);
  }

  return result;
}


/*
 */
int
sig_derivative_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ss[d]",
    NULL
  };

  char * help_msg =
  {
    (" Compute the derivative of a signal. Works only on REALY and REALXY "
     "signals\n"
     "\n"
     "Parameters :\n"
     "  Signal  - signal to treat.\n"
     "  string  - name of the result.\n"
     "  integer - step of the derivation (default one).")
  };

  /* Command's parameters */
  Signal *source;
  char   *res_name;
  int    step = 1;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal  *result;
  int i;
  int newSize;
  real dx;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &source, &res_name, &step) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  newSize = source->size-step;
  dx = source->dx;
  switch (source->type) {
  case REALY :
    result = sig_new(REALY, 0,newSize-1);
    result->x0 = source->x0;
    result->dx = source->dx;

    for (i = 0; i < result->size; i++) {
      result->dataY[i] = (source->dataY[i+step] - source->dataY[i])/(step*dx);
    }
    break;
  case REALXY :
    result = sig_new(REALXY, 0,newSize-1);
    result->x0 = source->x0;
    result->dx = source->dx;

    for (i = 0; i < result->size; i++) {
      result->dataY[i] = (source->dataY[i+step] - source->dataY[i])
	/(source->dataX[i+step] - source->dataX[i]);
      result->dataX[i] = source->dataX[i];
    }
    break;
  default:
      Tcl_AppendResult (interp, "wrong signal type", (char *) NULL);
      return TCL_ERROR;
    break;
  }

  store_signal_in_dictionary (res_name, result);

  return TCL_OK;
}


/*
 */
int
sig_2_sig_extrema_TclCmd_ (ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ss",
    NULL
  };

  char * help_msg =
  {
    (" Search the maxima of the absolute value of a signal.\n"
     "\n"
     "Parameters :\n"
     "  Signal  - signal to treat.\n"
     "  string  - name of the result.")
  };

  /* Command's parameters */
  Signal *signal;
  char   *res_name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *result;
  int    i;
  int    j = 0;
  int    extNb = 0;
  real   *data;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &res_name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  data = signal->dataY;
  for (i = 1; i < signal->size-1; i++) {
    if (fabs(data[i-1]) < fabs(data[i]) && fabs(data[i+1]) < fabs(data[i])) {
      extNb++;
    }
  }
  result = sig_new(REALXY, 0, extNb-1);
  for (i = 1; i < signal->size-1; i++) {
    if (fabs(data[i-1]) < fabs(data[i]) && fabs(data[i+1]) < fabs(data[i])) {
      switch (signal->type) {
      case REALY:
	  result->dataX[j] = signal->x0 + i*signal->dx;
	  break;
      case REALXY:
	  result->dataX[j] = signal->dataX[i];
	break;
      default:
	result->dataX[j] = i;
	break;
      }
      result->dataY[j] = signal->dataY[i];
	
      j++;
    }
  }

  store_signal_in_dictionary (res_name, result);

  return TCL_OK;
}


/* -------------------- from sw -------------------- */
#define EPSILON 1e-5
#define YES 1
#define NO 0
#define SIGN(x)  ((x) < 0 ? (-1) : (x) > 0 ? 1 : 0)

static int flagStep;
static float lastNonZeroDer;

static int
IsMaxima (float prevValueI,
	  float valueI,
	  float nextValueI,
	  float threshold)
{
  int derI,derNextI;


  if (fabs(valueI-prevValueI) < EPSILON) derI = 0;
  else derI =  SIGN(valueI - prevValueI);

  if (fabs(nextValueI-valueI) < EPSILON) derNextI = 0;
  else derNextI = SIGN(nextValueI - valueI);

  if ((derI != derNextI) &&
      ((((derI ==  1) || (derNextI == -1)) && (valueI > 0)) ||
       (((derI == -1) || (derNextI ==  1)) && (valueI < 0))) &&
      fabs(valueI) > threshold)
  /* In case we detect a positive maximum or a negative minimum */
    {
      if (derI != 0 && derNextI != 0) {
        flagStep = NO;
        return(YES);
      }

      if (derI != 0) {
        flagStep = YES;
        lastNonZeroDer = derI;
        return(NO);
      }

      /* In a middle of a flat level */
      if (derI == 0 && derNextI == 0) return(NO);

      /* Inflection point (with a flat level)*/
      if (derNextI != lastNonZeroDer && flagStep == YES) {
        flagStep = NO;
        return(YES);
      }

      /* Max or Min with a flat level */
      if (derNextI == lastNonZeroDer && flagStep == YES) {
        flagStep = NO;
        return(YES);
      }
    }

  return(NO);
}
/* -------------------- end from sw -------------------- */

/*
 */
int
sw_sig_2_sig_extrema_TclCmd_ (ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ss",
    NULL
  };

  char * help_msg =
  {
    (" Search the maxima of a signal a la sw.\n"
     "\n"
     "Parameters :\n"
     "  Signal  - signal to treat.\n"
     "  string  - name of the result.")
  };

  /* Command's parameters */
  Signal *signal;
  char   *res_name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *result;
  int    i;
  int    j = 0;
  int    extNb = 0;
  real   *data;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &res_name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  flagStep = NO;
  data = signal->dataY;
  for (i = 1; i < signal->size-1; i++) {
    if (IsMaxima(signal->dataY[i-1],
		 signal->dataY[i],
		 signal->dataY[i+1],
		 0)) {
      extNb++;
    }
  }
  result = sig_new(REALXY, 0, extNb-1);
  flagStep = NO;
  for (i = 1; i < signal->size-1; i++) {
    if (IsMaxima(signal->dataY[i-1],
		 signal->dataY[i],
		 signal->dataY[i+1],
		 0)) {
      result->dataX[j] = signal->x0 + i*signal->dx;
      result->dataY[j] = signal->dataY[i];
      j++;
    }
  }

  store_signal_in_dictionary (res_name, result);

  return TCL_OK;
}


/*----------------------------------------------------------------------
  _QuickSort_
  
  Fonction qui classe dans l'ordre croissant le tableau x de taille n.
  --------------------------------------------------------------------*/
static void 
_QuickSort_(double *x,
	    int    n)
{
  int    l, j, ir, i;
  double xx;
  
  l = (n>>1) + 1;
  ir = n;

  /*  L'index l est decremente de sa valeur initiale vers 1 pendant
   *  la periode de creation de l'arbre, une fois 1 atteitn, l'index ir
   *  decremente de sa valeur initiale vers 1 pendant la phase de
   *  selection entre branches */

  for(;;) {
    if(l > 1)
      xx = x[--l];
    else {
      xx = x[ir];
      x[ir] = x[1];
      if(--ir == 1) {
	x[1] = xx;
	return;
      }
    }
    i = l;
     j = l<<1;
    while(j <= ir) {
      if(j < ir && x[j] < x[j+1])
	++j;
      if(xx < x[j]) {
	x[i] = x[j];
	j += (i = j);
      }
      else j = ir + 1;
    }
    x[i] = xx;
  }
}

/*
 */
int
apply_fct_to_s_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ss[f]",
    "-domain_out", "ff",
    NULL
  };

  char * help_msg =
  {
    (" Applies a function to each value of a signal and sums all the "
     "values. The function can have two parameters.\n"
     "\n"
     "Parameters :\n"
     "  signal - signal to treat.\n"
     "  string - an expression designing the function (defunc syntax).\n"
     "  [real] - facultative value of a the second parameter of the "
     "           function.\n"
     "\n"
     "Options :\n"
     "  -domain_out : Define a domain. The function only applies to values "
     "                that are out of this domain.\n"
     "     2 integers - bounds of the domain.")
  };

  /* Command's parameters */
  Signal *signal;
  char  *fct_expr;
  real  scd_value = 0;

  /* Options's presence */
  int is_domain_out;

  /* Options's parameters */
  real out_min;
  real out_max;

  /* Other variables */
  /*double   (*fct)();*/
  void     *fct;
  double   result = 0.0;
  double   *values;
  int      i;
  int      nb_of_values = 0;
  real     *data;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

  is_domain_out = arg_present (1);
  if (is_domain_out)
    if (arg_get (1, &out_min, &out_max) == TCL_ERROR)
      return TCL_ERROR;

  /* Parameters validity and initialisation */
  /*fct = dfopen (fct_expr);*/
  fct = evaluator_create(fct_expr);
  if (!fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  if (is_domain_out && out_max < out_min) {
    sprintf (interp->result, "Bad domain ([%f, %f]).", out_min, out_max);
    return TCL_ERROR;
  }

  /* Treatement */
  data = signal->dataY;
  values = (double *) malloc(signal->size*sizeof(double));
  for (i = 0; i < signal->size; i++)
    if (!is_domain_out || data[i] < out_min || data[i] > out_max) {
      values[nb_of_values] = evaluator_evaluate_x_y(fct, data[i], scd_value);
      nb_of_values++;
    }

  if (nb_of_values > 1)
    _QuickSort_ (values-1, nb_of_values);

  for (i = 0; i < nb_of_values; i++)
    result += values[i];

  /*dfclose (fct);*/
  evaluator_destroy(fct);
  free (values);
  if (result == 0)
    sprintf (interp->result, "0");
  else
    sprintf (interp->result, "%.15e", result);
  return TCL_OK;
}


/*
 */
int
s_swap_TclCmd_ (ClientData clientData,
		Tcl_Interp *interp,
		int        argc,
		char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ss[f]",
    NULL
  };

  char * help_msg =
  {
    (" Exchange the first part of a signal with the last part.\n"
     "\n"
     "Parameters :\n"
     "  signal - signal to treat.\n"
     "  string - name of the result.\n"
     "  [real] - value of the limit between the 2 parts. Default is 0.\n")
  };

  /* Command's parameters */
  Signal *signal;
  char   *name;
  real   lim = 0.0;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *result;
  int    signalLimIndex;
  int    i;
  int    resultLimIndex;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &name, &lim) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  result = sig_new (signal->type, 0, signal->size-1);

  /* Get the index in signal where we must cut. */
  signalLimIndex = (int) floor ((lim-signal->x0)/signal->dx);
  if (signalLimIndex < 0) {
    signalLimIndex = 0;
  }
  if (signalLimIndex >= signal->size) {
    signalLimIndex = signal->size-1;
  }

  /* Get the index in result where we must put the beginning of signal */
  resultLimIndex = signal->size - 1 - signalLimIndex;

  /* Copy the first part of signal in the second part of result */
  for (i = 0; i <= signalLimIndex; i++) {
    result->dataY[resultLimIndex + i] = signal->dataY[i];
    if (signal->type == REALXY) {
      result->dataX[resultLimIndex + i] = signal->dataX[i];
    }
  }
  /* Copy the second part of signal in the first part of result */
  for (; i < signal->size; i++) {
    result->dataY[i - signalLimIndex - 1] = signal->dataY[i];
    if (signal->type == REALXY) {
      result->dataX[i - signalLimIndex - 1] = signal->dataX[i];
    }
  }

  result->x0 = signal->x0 + (signalLimIndex+1)*signal->dx;
  result->dx = signal->dx;

  store_signal_in_dictionary (name, result);

  return TCL_OK;
}


/*
 * This macro _MUST_ be moved elsewhere.
 */
#define RAISE_EXCEPTION(exception) goto exception

/*
 */
int
sig_TclCmd_ (ClientData clientData,
	     Tcl_Interp *interp,
	     int        argc,
	     char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "sdss",
    NULL
  };

  char * help_msg = {
    (" Create a new signal from a combination of other signals. This command "
     "is very slow, so use it only with small signals.\n"
     "\n"
     "Parameters :\n"
     "  string  - name of the signal to create.\n"
     "  integer - size of the signal.\n"
     "  string  - double list of signal-variable name (ex. {sin s exp e}).\n"
     "  string  - expression that combines the variables. Warning, use curly "
     "braces ('{') because of the first parsing of the command. Example : "
     " { 2.8*$s*$e+$i }, with i a variable that must be set.\n"
     "\n"
     "Return value :\n"
     "  Name of the resulting signal, or error of the evaluation of the "
     "expression.")
  };

  /* Command's parameters */
  char *name;
  int  size;
  char *dbleListStr;
  char *expression;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal  *result;
  Signal  **sigArray;
  char    **eltArray;
  char    **sigNameArray;
  char    **sigVarNameArray;
  Tcl_Obj **sigVarObjArray;
  Tcl_Obj **sigVarStrObjArray;
  real    *sigVarArray;
  int     code;
  int     dbleListSize;
  int     sigListSize;
  int     i;
  int     j;
  double  dbleValue;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &name, &size, &dbleListStr, &expression) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  code = Tcl_SplitList(interp, dbleListStr, &dbleListSize, (CONST char ***)&eltArray);
  if (code == TCL_ERROR)
    return TCL_ERROR;
  if (dbleListSize%2 != 0) {
    Tcl_AppendResult(interp,
		     "the list must have an even number of elements", NULL);
  } else {
    sigListSize = dbleListSize/2;
  }

  /* Memory allocation of all we need. */
  sigArray = (Signal **) malloc(sizeof(Signal *)*sigListSize);
  if (!sigArray) {
    RAISE_EXCEPTION (memory_allocation_error);
  }
  sigNameArray = (char **) malloc(sizeof(char *)*sigListSize);
  if (!sigNameArray) {
    RAISE_EXCEPTION (memory_allocation_error);
  }
  sigVarArray = (real *) malloc(sizeof(real *)*sigListSize);
  if (!sigVarArray) {
    RAISE_EXCEPTION (memory_allocation_error);
  }
  sigVarNameArray = (char **) malloc(sizeof(char *)*sigListSize);
  if (!sigVarNameArray) {
    RAISE_EXCEPTION (memory_allocation_error);
  }
  sigVarObjArray = (Tcl_Obj **) malloc(sizeof(Tcl_Obj*)*dbleListSize);
  if (!sigVarObjArray) {
    RAISE_EXCEPTION (memory_allocation_error);
  }
  sigVarStrObjArray = (Tcl_Obj **) malloc(sizeof(Tcl_Obj*)*dbleListSize);
  if (!sigVarObjArray) {
    RAISE_EXCEPTION (memory_allocation_error);
  }


  /* Check existence and type validity of all the signals.  */
  for (i = 0; i < sigListSize; i++) {
    sigNameArray[i] = eltArray[2*i];
    sigVarNameArray[i] = eltArray[2*i+1];
    sigArray[i] = get_signal (sigNameArray[i]);
    if (!sigArray[i]) {
      Tcl_AppendResult(interp,
		       "Couldn't find Signal `",
		       sigNameArray[i], "'.", NULL);
      RAISE_EXCEPTION (general_error);
    }
    if ((sigArray[i]->type != REALY) && (sigArray[i]->type != REALXY)) {
      Tcl_AppendResult(interp,
		       "signal `",
		       sigNameArray[i], "' must be REALY or REALXY", NULL);
      RAISE_EXCEPTION (general_error);
    }
    sigVarStrObjArray[i] = Tcl_NewStringObj(sigVarNameArray[i],
					    strlen(sigVarNameArray[i]));
    sigVarObjArray[i] = Tcl_NewDoubleObj(0.0);
  }

  /* Treatement */
  result = sig_new(REALY, 0, size-1);

  for (j = 0; j < size; j++) {
    /* Loop on result points. */
    for (i = 0; i < sigListSize; i++) {
      /* Loop on signal list to set the variables. */
      if (j < sigArray[i]->n) {
	Tcl_SetDoubleObj(sigVarObjArray[i], sigArray[i]->dataY[j]);
      } else { /* The current signal is smaller than the result : expand with 0. */
	Tcl_SetDoubleObj(sigVarObjArray[i], 0.0);
      }
      Tcl_ObjSetVar2(interp, sigVarStrObjArray[i], NULL, sigVarObjArray[i], 0);
    }
    code = Tcl_ExprDouble(interp, expression, &dbleValue);
    if (code == TCL_ERROR) {
      return TCL_ERROR;
    }
    result->dataY[j] = (real) dbleValue;
  }

  store_signal_in_dictionary (name, result);
  sprintf (interp->result, "%s", name);

  free (sigArray);
  free (sigNameArray);
  free (sigVarArray);
  free (sigVarNameArray);
  free (sigVarObjArray);
  free (sigVarStrObjArray);

  return TCL_OK;

memory_allocation_error:
  free (sigArray);
  free (sigNameArray);
  free (sigVarArray);
  free (sigVarNameArray);
  free (sigVarObjArray);
  free (sigVarStrObjArray);

  return GenErrorMemoryAlloc(interp);

general_error:
  free (sigArray);
  free (sigNameArray);
  free (sigVarArray);
  free (sigVarNameArray);
  free (sigVarObjArray);
  free (sigVarStrObjArray);

  return TCL_ERROR;
}


/*
 */
int 
s_rmdisc_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ss",
    NULL
  };

  char * help_msg =
  {
    (" Remove discontinuities in atan, asin, acos ....\n"
     /*    (" Remove discontinuities in a signal which values (ordinates) are PI"
     "periodic. Like the evolution of an angle during the time : there are "
     "discontinuities that step from -PI to +PI. Warning : the test to detect "
     "such discontinuities is a step greater than PI/4.\n"*/
     "\n"
     "Parameters :\n"
     "  signal - signal to treat.\n"
     "  string - name of the result.\n")
  };

  /* Command's parameters */
  Signal *signal;
  char   *name;

 /* Other variables */
  Signal *result;
  int i,sized2,k,size;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  result = sig_new (signal->type, 0, signal->size-1);

  if (signal->type == REALY) {
    result->dx= signal->dx;
    result->x0= signal->x0;
  }

  size=signal->size;
  sized2=signal->size/2-2;
  for (i=0;i<signal->size;i++) {
    if (signal->type == REALXY) {
      result->dataX[i]=signal->dataX[i];
    }
    result->dataY[i]=signal->dataY[i];
  }

  for (i=sized2;i<signal->size;i++){
    if (result->dataY[i+1]-result->dataY[i]> PI/4.0)       
      for(k=i+1;k<signal->size;k++) 
	result->dataY[k] -= PI;        
    if (result->dataY[i+1]-result->dataY[i]< -PI/4.0)
       for(k=i+1;k<signal->size;k++)
          result->dataY[k] += PI;
  }
  for(i=0;i<sized2-1;i++)
    result->dataY[i] = -result->dataY[size-i-1];

  store_signal_in_dictionary (name, result);

  return TCL_OK;
}

/*
 */
int 
s_symfit_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ss",
    NULL
  };

  char * help_msg =
  {
    (" Fit the data by a1x^2 + a2x^4 + a3x^6"
     "\n"
     "Parameters :\n"
     "  signal - signal to treat.\n"
     "  string - name of the resulting fit.\n"
     "return a1, a2 and a3\n")
  };

  /* Command's parameters */
  Signal *signal;
  char   *name;

 /* Other variables */
  Signal *result;
  real a1, a2, a3;
  real s4=0.0,s6=0.0,s8=0.0,s10=0.0,s12=0.0;
  real b2=0.0,b4=0.0,b6=0.0;
  real det,inv11,inv22,inv33,inv12,inv13,inv23;
  int i;
  real x,y,x2,x4,x6,x8,x10,x12;
  
  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  for (i=0;i<signal->size;i++) {
    if (signal->type == REALY) 
      x = i*signal->dx+signal->x0;
    
    if (signal->type == REALXY) 
      x=signal->dataX[i];
    
    y = signal->dataY[i];
    x2 = x*x;
    x4 = x2*x2;
    x6 = x2*x4;
    x8 = x4*x4;
    x10 = x8*x2;
    x12 = x6*x6;
    s4 += x4;
    s6 += x6;
    s8 += x8;
    s10 += x10;
    s12 += x12;
    b2 += x2*y;
    b4 += x4*y;
    b6 += x6*y;
  }

  det = s4*(s8*s12-s10*s10) - s6*(s6*s12-s10*s8)+s8*(s6*s10-s8*s8);
  inv11 = (s8*s12-s10*s10)/det;
  inv22 = (s4*s12-s8*s8)/det;
  inv33 = (s4*s8-s6*s6)/det;
  inv12 = (s10*s8-s6*s12)/det;
  inv13 = (s6*s10-s8*s8)/det;
  inv23 = (s8*s6-s4*s10)/det;
  a1 = b2*inv11 + b4*inv12 + b6*inv13;
  a2 = b2*inv12 + b4*inv22 + b6*inv23;
  a3 = b2*inv13 + b4*inv23 + b6*inv33;

  result = sig_new (signal->type, 0, signal->size-1);

  if (signal->type == REALY) {
    result->dx= signal->dx;
    result->x0= signal->x0;
  }

  for (i=0;i<signal->size;i++) {
    if (signal->type == REALXY) {
      result->dataX[i]=signal->dataX[i];
      x=result->dataX[i];
    }
    if (signal->type == REALY)
      x=i*signal->dx+signal->x0;
    
    result->dataY[i]=a1*x*x+a2*x*x*x*x+a3*x*x*x*x*x*x;
  }

  sprintf(interp->result,"%f %f %f", a1, a2, a3);
  store_signal_in_dictionary (name, result);

  return TCL_OK;
}

/*
 */
int 
s_asymfit_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ss",
    NULL
  };

  char * help_msg =
  {
    (" Fit the data by a1x + a2x^3 + a3x^5"
     "\n"
     "Parameters :\n"
     "  signal - signal to treat.\n"
     "  string - name of the resulting fit.\n"
     "return a1, a2 and a3\n")
  };

  /* Command's parameters */
  Signal *signal;
  char   *name;

 /* Other variables */
  Signal *result;
  real a1, a2, a3;
  real s2=0.0,s4=0.0,s6=0.0,s8=0.0,s10=0.0;
  real b1=0.0,b3=0.0,b5=0.0;
  real det,inv11,inv22,inv33,inv12,inv13,inv23;
  int i;
  real x,y,x2,x4,x6,x8,x10,x3,x5;
  
  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  for (i=0;i<signal->size;i++) {
    if (signal->type == REALY) 
      x = i*signal->dx+signal->x0;
    
    if (signal->type == REALXY) 
      x=signal->dataX[i];
    
    y = signal->dataY[i];
    x2 = x*x;
    x3 = x*x*x;
    x5 = x3*x2;
    x4 = x2*x2;
    x6 = x2*x4;
    x8 = x4*x4;
    x10 = x8*x2;
    s2 += x2;
    s4 += x4;
    s6 += x6;
    s8 += x8;
    s10 += x10;
    b1 += x*y;
    b3 += x3*y;
    b5 += x5*y;
  }

  det = s2*(s6*s10-s8*s8) - s4*(s4*s10-s8*s6)+s6*(s4*s8-s6*s6);
  inv11 = (s6*s10-s8*s8)/det;
  inv22 = (s2*s10-s6*s6)/det;
  inv33 = (s2*s6-s4*s4)/det;
  inv12 = (s8*s6-s4*s10)/det;
  inv13 = (s4*s8-s6*s6)/det;
  inv23 = (s6*s4-s2*s8)/det;
  a1 = b1*inv11 + b3*inv12 + b5*inv13;
  a2 = b1*inv12 + b3*inv22 + b5*inv23;
  a3 = b1*inv13 + b3*inv23 + b5*inv33;


  result = sig_new (signal->type, 0, signal->size-1);

  if (signal->type == REALY) {
    result->dx= signal->dx;
    result->x0= signal->x0;
  }

  for (i=0;i<signal->size;i++) {
    if (signal->type == REALXY) {
      result->dataX[i]=signal->dataX[i];
      x=result->dataX[i];
    }
    if (signal->type == REALY)
      x=i*signal->dx+signal->x0;    
    result->dataY[i]=a1*x+a2*x*x*x+a3*x*x*x*x*x;
  }

  sprintf(interp->result,"%f %f %f", a1, a2, a3);
  store_signal_in_dictionary (name, result);

  return TCL_OK;
}

/*
 */
int 
s_thresh_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ssff",
    "-y","",
    NULL
  };

  char * help_msg =
  {
    (" Threshold the signal"
     "\n"
     "Parameters :\n"
     "  signal - signal to treat.\n"
     "  string - name of the resulting fit.\n"
     "  2 floats - domain of th ethresh\n"
     "  -y thresh in the y direction\n")
  };

  /* Command's parameters */
  Signal *signal;
  char   *name;
  real xmin,xmax;

 /* Other variables */
  Signal *result;
  int is_y;
  int i,j,size;
  float x,y;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &name,&xmin,&xmax) == TCL_ERROR)
    return TCL_ERROR;

  is_y=arg_present(1);

  /* Parameters validity and initialisation */

  /* Treatement */

  size=0;
  for(i=0;i<signal->size;i++){
    y=signal->dataY[i];
    if (signal->type == REALY) x=signal->x0+i*signal->dx;
    else x=signal->dataX[i];
    
    if ((is_y) && (y >= xmin) && (y <= xmax)) size++;
    if (!(is_y) && (x >= xmin) && (x <= xmax)) size++;
  }

  if (is_y) result = sig_new (REALXY, 0, size-1);
  else result = sig_new (signal->type, 0, size-1);

  for(i=0,j=0;i<signal->size;i++){
    y=signal->dataY[i];  
    if (signal->type == REALY) x=signal->x0+i*signal->dx;
    else x=signal->dataX[i];
    
    if ((is_y) && (y >= xmin) && (y <= xmax)) {
      result->dataX[j]=x;
      result->dataY[j++]=y;
    }
    else 
      if (!(is_y) && (x >= xmin) && (x <= xmax)) {
	result->dataY[j]=y;
	if (signal->type == REALXY) result->dataX[j]=x;
	else 
	  if (!(is_y) && (j == 0)) {
	    result->x0 = x;
	    result->dx =signal->dx;
	  }
	j++;	
      }
  }

  store_signal_in_dictionary (name, result);

  return TCL_OK;
}


/*
 */
int
sig_merge2sig_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "SSs",
    NULL
  };

  char * help_msg =
  {
    (" merge two signal y of first signal become the x of the result)\n"
     "\n"
     "Parameters :\n"
     "  signal  - first signal to treat.\n"
     "  signal  - second signal to treat.\n"
     "  string  - the name of the new signal.\n")
  };

  /* Command's parameters */
  Signal *signal1, *signal2;
  char   *name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *new_sig;
  int    x;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal1, &signal2, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Treatement */

  if (signal1->size != signal2->size) {
        Tcl_AppendResult (interp,
                      "The two signals should be of the same size.",
                      (char *) NULL);
    return TCL_ERROR;
  }

  new_sig = sig_new (REALXY, 0,signal1->size-1);
  for (x = 0; x < new_sig->size; x++) {
    new_sig->dataX[x]=signal1->dataY[x];
    new_sig->dataY[x]=signal2->dataY[x];
  }

  store_signal_in_dictionary (name, new_sig);

  return TCL_OK;
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

/*
 */
int
get_rule_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "ff",
    NULL
  };

  char * help_msg =
  {
    (" Get parameters that define a rule.\n"
     "\n"
     "Parameters :\n"
     "  2 float - border of the rule.")
  };

  /* Command's parameters */
  real xMin, xMax;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  double  xvmin,xvmax,rmin;
  int     n12,nnHor;
  double  horSpace;

  double pos;
  int n, nn0;

  char tmpBuffer[1000] = "\0";

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &xMin, &xMax) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  horSpace  = g_stsize(xMin, xMax, &n12, &xvmin, &xvmax, &rmin, &nnHor);

  pos = (real) rmin;
  for (n=0; n < n12; n++)
    {
      if (fabs(pos) < 0.0001*horSpace) {
	pos = 0;
      }
      if (pos >= xMin && pos <= xMax) {
	sprintf(tmpBuffer, "%s %.15g", tmpBuffer, pos);
      }
      pos += (real) horSpace;
    }
  Tcl_AppendElement(interp, tmpBuffer);

  sprintf(tmpBuffer, " ");
  pos = (real) rmin;
  nn0 = nnHor;
  for (n=0; n < n12; n++)
    {
      if (fabs(pos) < 0.0001*horSpace) {
	pos = 0;
      }
      if (!nn0)
	{
	  if (pos >= xMin && pos <= xMax) {
	    sprintf(tmpBuffer, "%s %.15g", tmpBuffer, pos);
	  }
	}
      pos += horSpace;
      nn0++;
      nn0 %= 5;
    }

  Tcl_AppendElement(interp, tmpBuffer);

  return TCL_OK;
}


/*
 */
int
s_cplx_to_2_real_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Sss",
    NULL
  };

  char * help_msg =
  {
    (" Get real and imaginary parts of a complex signal.\n"
     "\n"
     "Parameters:\n"
     "  Signal    - Signal to transform.\n"
     "  2 strings - Names of the results.\n"
     "\n"
     "Return value:\n"
     "  None.")
  };

  /* Command's parameters */
  Signal *sig;
  char   *rName;
  char   *iName;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Signal *rSig;
  Signal *iSig;
  int    i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &sig, &rName, &iName) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  if (sig->type != FOUR_NR) {
    Tcl_AppendResult (interp,
		      "signal must be of type FOUR_NR",
		      (char *) NULL);
    return TCL_ERROR;
  }

  /* Treatement */

  rSig = sig_new (REALY, 0, sig->n -1);
  iSig = sig_new (REALY, 0, sig->n -1);

  for (i = 0; i < sig->n; i++) {
    rSig->dataY[i] = sig->dataY[2*i];
    iSig->dataY[i] = sig->dataY[2*i+1];
  }

  store_signal_in_dictionary (rName, rSig);
  store_signal_in_dictionary (iName, iSig);

  return TCL_OK;
}


/*------------------------------------------------------------------------
  ----------------------------------------------------------------------*/
int
sig_pol_interp_derivative_TclCmd_(ClientData clientData,
				  Tcl_Interp * interp,
				  int        argc,
				  char       ** argv)
{
  char * options[] = { "Ss[d]" ,
		       "-spline","",
		       "-spline2","s",
		       "-spline3","s",
		       NULL };
 
  char * help_msg =
  {("Computes first (or second) derivative of signal\n"
    "  using a polynomial (cubic) interpolation (see Numerical Recipes\n"
    "  chapter 3, pages 108\n"
    "  We assume that signal size is above 10\n"
    "\n"
    "Parameters :\n"
    "  Signal     - signal to treat.\n"
    "  String     - name of the result (signal of same size).\n"
    "  [interger] - 1 if first derivative, 2 if second derivative.\n"
    "\n"
    " Options:\n"
    "  -spline      : use spline interpolation (numerical recipes)\n"
    "  -spline2 [s] : use another spline approximation (Paul Bourke)"
    "  -spline3 [s] : use another spline approximation")};
 
  /* Command's parameters */
  Signal *signal, *result, *result2;
  int order = 1;
  char   *name, *name2;

  /* Other variables */
  int is_Spline, is_Spline2, is_Spline3;;

  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  if (arg_get(0, &signal, &name, &order) == TCL_ERROR)
    return TCL_ERROR;

  is_Spline =arg_present(1);
  is_Spline2=arg_present(2);
  is_Spline3=arg_present(3);
  if (is_Spline2) {
    if (arg_get(2, &name2) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } else if (is_Spline3) {
    if (arg_get(3, &name2) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

 /* Parameters validity and initialisation */
  if (order == 1 || order ==2) {
  } else {
    Tcl_AppendResult (interp,
		      "optionnal parameter must be 1 or 2!!!",
		      (char *) NULL);
    return TCL_ERROR;
  }

  if (signal->n<10) {
    Tcl_AppendResult (interp,
		      "Signal size must be at least 10 !!",
		      (char *) NULL);
    return TCL_ERROR;
  }

   /* Treatement */
  if (is_Spline) {
    /* Check signal type */
    if (signal->type!=REALXY) {
      Tcl_AppendResult (interp,
			"Signal type must be XY with -spline option !!",
			(char *) NULL);
      return TCL_ERROR;
    }
    result = sig_to_spline_int_derivative (signal,order);
  } else if (is_Spline2) {
    /* Check signal type */
    if (signal->type!=REALXY) {
      Tcl_AppendResult (interp,
			"Signal type must be XY with -spline2 option !!",
			(char *) NULL);
      return TCL_ERROR;
    }
    result  = sig_to_spline2_int_derivative (signal,order,ALONGY);
    result2 = sig_to_spline2_int_derivative (signal,order,ALONGX);
  }  else if (is_Spline3) {
    /* Check signal type */
    if (signal->type!=REALXY) {
      Tcl_AppendResult (interp,
			"Signal type must be XY with -spline3 option !!",
			(char *) NULL);
      return TCL_ERROR;
    }
    result  = sig_to_spline3_int_derivative (signal,order,ALONGY);
    result2 = sig_to_spline3_int_derivative (signal,order,ALONGX);
  } else {
    result = sig_to_pol_int_derivative (signal,order);
  }

  if (!result)
    return TCL_ERROR;

  store_signal_in_dictionary(name, result);
  if (is_Spline2) {
    store_signal_in_dictionary(name2, result2);    
  }

  return TCL_OK;
}
