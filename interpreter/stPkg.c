/*
 * stephPkg.c --
 *
 *   Implements the steph package for xsmurf.
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster and Stephane Roux.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: stPkg.c,v 1.2 1999/01/08 18:14:53 decoster Exp $
 */

#include "smPkgInt.h"
#include "cv1d.h"
#include <nonlinfit.h>
#include <nr_lib.h>
#include <math.h>

#include "../main/smMalloc.h"

#include <sys/times.h>

/* 
 * The relative path of the following line is to avoid confusion with the
 * system header 'signal.h'.
 */

#include "../signal/signal.h"

/*
 * List of prototypes of C functions that define all the commands of the
 * package.
 */

static int _lf_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _poly_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _beta_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _mwc_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _bic_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _dnq_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _gauss_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _my_iinssig_Cmd_     (ClientData, Tcl_Interp*, int, char**);
static int _steph_Cmd_		(ClientData, Tcl_Interp*, int, char**);
/*
 * Command list.
 */

static cmdInfo StephCmdInfoArray[] = {
  {"lf",	       (Tcl_CmdProc *)_lf_Cmd_},
  {"polyf",      (Tcl_CmdProc *)_poly_Cmd_},
  {"betaf",      (Tcl_CmdProc *)_beta_Cmd_},
  {"mwcf",       (Tcl_CmdProc *)_mwc_Cmd_},
  {"bicf",       (Tcl_CmdProc *)_bic_Cmd_},
  {"dnqf",       (Tcl_CmdProc *)_dnq_Cmd_},
  {"gaussf",     (Tcl_CmdProc *)_gauss_Cmd_},
  {"steph",      (Tcl_CmdProc *)_steph_Cmd_},
  {"myiinssig",  (Tcl_CmdProc *)_my_iinssig_Cmd_},

  {NULL,	(Tcl_CmdProc *) NULL}
};

/*
 * Steph_pkgInit --
 *
 *  Init function of the steph package.
 *
 * Arguments :
 *   Tcl_Interp - The interpreter where the package must be init.
 *
 * Return Value :
 *   TCL_OK.
 */

int
Steph_pkgInit (Tcl_Interp * interp)
{
  register cmdInfo *cmdInfoPtr;

  Tcl_PkgProvide (interp, "steph", "0.0");

  /*
   * Create the built-in commands for the package.
   */
  for (cmdInfoPtr = StephCmdInfoArray; cmdInfoPtr->name != NULL; cmdInfoPtr++) {
    Tcl_CreateCommand(interp,
		      cmdInfoPtr->name,
		      cmdInfoPtr->proc,
		      (ClientData) 0, (void (*)()) NULL);
  }

  return TCL_OK;
}

/*
 */
static int
_steph_Cmd_ (ClientData clientData,
	  Tcl_Interp *interp,
	  int        argc,
	  char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "",
    "cmdlist", "",
    NULL
  };

  char * helpMsg = {
    (
     " List the C-defined commands of the package.\n"
     "\n"
     "Parameters :\n"
     "  none."
     )
  };

  /* Command's parameters */

  /* Options's presence */
  int isCmdlist;

  /* Options's parameters */

  /* Other variables */
  register cmdInfo *cmdInfoPtr;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get (0) == TCL_ERROR)
    return TCL_ERROR;

  isCmdlist = arg_present (1);

  /* Parameters validity and initialisation */

  /* Treatement */
  if (isCmdlist) {
    Tcl_AppendResult (interp, "{", (char *) NULL);

    cmdInfoPtr = StephCmdInfoArray;
    Tcl_AppendResult (interp, cmdInfoPtr->name, (char *) NULL);
    for (cmdInfoPtr++;
	 cmdInfoPtr->name != NULL;
	 cmdInfoPtr++) {
      Tcl_AppendResult (interp, " ", cmdInfoPtr->name, (char *) NULL);
    }
    Tcl_AppendResult (interp, "}", (char *) NULL);
  } else {
    sprintf (interp->result, "The steph package blablabla.");
  }

  return TCL_OK;
}

/* Dichotomic algorithm to find the smallest index such that X[i] >= x
   The result is in [0..size-1] (size -1 is returned if X[size-1] < x.
   (used in the case of an REALXY signal)
   
   */

static int DichXCeil_(Signal *signal,
		      real x,
		      int iMin,
		      int iMax)
{
  int ip;
  if (iMin == iMax) return(iMin);
  
  ip = iMin+(iMax-iMin+1)/2-1;

  if (signal->dataX[ip] < x) 
    return(DichXCeil_(signal,x,ip+1,iMax));    
  else 
    return(DichXCeil_(signal,x,iMin,ip));

}

static int DichXCeil(Signal *signal,
		     real x)
{
  if (signal->dataX[0] >= x) return(0);
  if (signal->dataX[signal->size-1] < x) return(signal->size-1);


  return(DichXCeil_(signal,x,0,signal->size-1));
}

int IsigCeil(Signal *signal,
	     real x)
{
  int i;
  
  if(signal->type == REALXY)
    return(DichXCeil(signal,x));
  else 
    {
      i = (int) ceil((double) (x - signal->x0)/signal->dx);
      if(i<0)
	return 0;
      else if(i>=signal->size)
	return signal->size-1;
      else
	return i;
    }
}


/* Dichotomic algorithm to find the biggest index such that X[i] =< x
   The result is in [0..size-1] (0 is returned if X[0] > x.
   (used in the case of an XYSIG signal)   
   */

static int DichXFloor_(Signal *signal,
		       real x,
		       int iMin,
		       int iMax)
{
  int ip;
  
  if (iMin == iMax) return(iMin);
  
  ip = iMin+(iMax-iMin+1)/2;

  if (signal->dataX[ip] <= x) 
    return(DichXFloor_(signal,x,ip,iMax));
  else 
    return(DichXFloor_(signal,x,iMin,ip-1));
} 
	
	
static int DichXFloor(Signal *signal,
		      real x)
{
	if (signal->dataX[0] > x) return(0);
	if (signal->dataX[signal->size-1] <= x) return(signal->size-1);
	
	return(DichXFloor_(signal,x,0,signal->size-1));
}

/* returns the biggest index such that X[i] <= x
   The result is in [0..size-1] (0 is returned if X[0] > x.
   */
int IsigFloor(Signal *signal,
	      real x)
{
  int i;
  
  if(signal->type == REALXY)
    return(DichXFloor(signal,x));
  else 
    {
      i = (int) floor((x - signal->x0)/signal->dx);
      if(i<0)
	return 0;
      else if(i>=signal->size)
	return signal->size-1;
      else
	return i;
    }
}


/*
 * Here begins the C-description of all the steph package commands.
 */


/*
 */
static int
_lf_Cmd_ (ClientData clientData,
	   Tcl_Interp *interp,
	   int        argc,
	   char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "Ss",
    "-x","ff",
    "-sigma","S",
    "-fit","S",
    NULL
  };

  char * helpMsg = {
    (
     "  Fit the data by a straight line (y=ax+b)."
     "\n"
     "Arguments :\n"
     "  1 signal - signal to fit.\n"
     "  string    - name of the result (a, b,siga,sigb,chi2,goodness-of-fit)\n."
     "\n"
     "Options :\n"
     "   -x     : domain to fit.\n"
     "   -sigma : signal containing the dy of each points of the signal to fit.\n"
     "   -fit   : signal containing the x-value (REALY)\n"
     "            --> return y=ax+b in this signal (REALXY).\n"
     )
  };

  /* Command's parameters */
  Signal *datasignal,*sigmasignal =  NULL, *result, *xsignal = NULL;
  char   *resultName;
  real   xMin,xMax;
  int flagWithWeight=0;
  int size,i;
  real *X, *sigmaY;
  real a,b,siga,sigb,chi2,q;
  int iMin, iMax;


  xMin=1.0;
  xMax=-1.0;

  if (arg_init(interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get(0, &datasignal, &resultName) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xMin, &xMax) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &sigmasignal) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(3, &xsignal) == TCL_ERROR)
    return TCL_ERROR;

  if ((arg_present(2) && (datasignal->size != sigmasignal ->size))) {
    Tcl_AppendResult (interp,
		      "The sigma signal should be of the same size as the input signal.",
		      (char *) NULL);
    return TCL_ERROR;
  }


  size = datasignal->size;

  if (datasignal->type == REALY) 
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) { 
	X[i] = datasignal->x0 + i*datasignal->dx;
      }
    }
  else if (datasignal->type == REALXY)
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) { 
	X[i] = datasignal->dataX[i];
      }
    }
  else {
        Tcl_AppendResult (interp,
		      "Bad type for the signal (only REALY or REALXY).",
		      (char *) NULL);
    return TCL_ERROR;
  }
    
  if (arg_present(2))
    {
      sigmaY = sigmasignal->dataY - 1;
      flagWithWeight = 1;
    }
  else
    {
      sigmaY = NULL;
      flagWithWeight = 0;
    }

    
  iMin =0;
  iMax = size -1;

  if (arg_present(1)) {
    if (xMin >= xMax) {
      Tcl_AppendResult (interp,
			"xmin should be smaller than xmax",
			(char *) NULL);
      return TCL_ERROR;
    }
    iMin = IsigCeil(datasignal,xMin);
    iMax = IsigFloor(datasignal,xMax);
  }

  size = iMax - iMin + 1;
  printf("imin=%d %d\n",iMin,iMax);
  
  if(size <= 1) {
    Tcl_AppendResult (interp,
		      "Can't fit on less than two points!!",
		      (char *) NULL);
    return TCL_ERROR;
  }

  fit(X-1+iMin,datasignal->dataY-1+iMin,size,
      ((sigmaY == NULL) ? sigmaY : sigmaY + iMin),
      flagWithWeight,&a,&b,&siga,&sigb,&chi2,&q);
 
  free(X);

  result = sig_new (REALY, 0, 5);
  result->x0 = 0.0;
  result->dx = 1;
  result->dataY[0]=a;
  result->dataY[1]=b;
  result->dataY[2]=siga;
  result->dataY[3]=sigb;
  result->dataY[4]=chi2;
  result->dataY[5]=q;
  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(resultName, result);
  
  if (arg_present(3)) {
    if (xsignal->type != REALY) {
      Tcl_AppendResult (interp,
			"Type for xsignal must be REALY!!",
			(char *) NULL);
      return TCL_ERROR;
    }
    else {
      /*    DEMAIN REGARDER COMMENT CHANGER REALY EN REALXY
	    size=xsignal->size;
    X = (float *) malloc(sizeof(float)*size);      
    X = xsignal->dataY;
    sig_free(xsignal);
    */
    }


  }
  return TCL_OK;
}

/*
 */
static int
_poly_Cmd_ (ClientData clientData,
	   Tcl_Interp *interp,
	   int        argc,
	   char       **argv)      

{

  /* Command line definition */
  char * options[] = {
    "SSSs",
    "-fit","S",
    NULL
  };

  char * helpMsg = {
    (
     "  Fit the data by with a polynomial of order the number of initial values.\n"
     "\n"
     "Arguments :\n"
     "  3 signals - signal to fit, \n"
     "              signal containing the incertaity for each point, \n"
     "              signal containing the initial values (size of this signal = order of the fit).\n"
     "  string    - name of the result (a1.. an,chisq,goodness,covar --> size=n+2+n*n)\n."
     "\n"
     "Options :\n"
     "   -fit   : signal containing the x-value (REALY)\n" 
     "            --> return the polynomial fit in this signal (REALXY).\n"
     )
  };

  Signal *datasignal,*sigmasignal, *valsignal;
  Signal *result, *xsignal = NULL;
  char   *resultName;
  int i,j,size;

  real *X;
  int ma;
  int *ia;
  real *a,**covar,**alpha;
  real chisq;
  int u;

  if (arg_init(interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get(0, &datasignal, &sigmasignal, &valsignal, &resultName) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xsignal) == TCL_ERROR)
    return TCL_ERROR;

  size = datasignal->size;
  if(sigmasignal->size != size) {
    Tcl_AppendResult (interp,
		      "The sigma signal should be of the same size as the input signal.",
		      (char *) NULL);
    return TCL_ERROR;
  }
  
  if (datasignal->type == REALY) 
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->x0 + i*datasignal->dx;
    }
  else if (datasignal->type == REALXY)
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->dataX[i]; 
    }
  else {
        Tcl_AppendResult (interp,
		      "Bad type for the signal (only REALY or REALXY).",
		      (char *) NULL);
    return TCL_ERROR;
  }

  ma = valsignal->size;
  NonLinFitInit(&a,&ia,ma,&covar,&alpha);
  for(i=0;i<valsignal->size;i++)
	{
	  a[i+1]=valsignal->dataY[i];
	  ia[i+1] = 1;
	}
  NonLinFit(X-1,datasignal->dataY-1,sigmasignal->dataY-1,size,a,ia,ma,covar,alpha,
		&chisq,&PolynomeFit);


  result = sig_new (REALY, 0, ma+ma*ma+1);
  result->x0 = 0.0;
  result->dx = 1;
  u =0;
  for(i=1;i<=ma;i++) {
    result->dataY[u]=a[i];
    u=u+1;
  }
  result->dataY[u++]=chisq;
  result->dataY[u++]=NonLinFitConfidence(chisq,size,ma);
  for(i=1;i<=ma;i++) {
    	  for(j=1;j<=ma;j++)
	    {	      
	      result->dataY[u++]=covar[i][j];
	    }
  }


  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(resultName, result);
  
  if (arg_present(1)) {
    if (xsignal->type != REALY) {
      Tcl_AppendResult (interp,
			"Type for xsignal must be REALY!!",
			(char *) NULL);
      return TCL_ERROR;
    }
    else {
      sig_realy2realxy(xsignal);
      sig_put_y_in_x(xsignal);
      for (i=0;i<xsignal->size;i++) 
	PolynomeFit(xsignal->dataX[i],a,&(xsignal->dataY[i]),NULL,ma);

    }


  }

  NonLinFitDelete(a,ia,ma,covar,alpha);

  return TCL_OK;

}


/*
 */
static int
_beta_Cmd_ (ClientData clientData,
	   Tcl_Interp *interp,
	   int        argc,
	   char       **argv)      

{

  /* Command line definition */
  char * options[] = {
    "SSSs",
    "-fit","S",
    NULL
  };

  char * helpMsg = {
    (
     "  Fit the data with a beta model (model for agregat : y=ln(beta*(x-lnMo)/dzeta +1)/beta).\n"
     "\n"
     "Arguments :\n"
     "  3 signals - signal to fit, \n"
     "              signal containing the incertaity for each point, \n"
     "              signal containing the initial values (dzeta,beta,lnM0).\n"
     "  string    - name of the result (dzeta,beta,lnM0,chisq,goodness,covar --> size=14)\n."
     "\n"
     "Options :\n"
     "   -fit   : signal containing the x-value (REALY)\n" 
     "            --> return the beta fit in this signal (REALXY).\n"
     )
  };

  Signal *datasignal,*sigmasignal, *valsignal;
  Signal *result, *xsignal = NULL;
  char   *resultName;
  int i,j,size;
  
  real *X;
  int ma;
  int *ia;
  real *a,**covar,**alpha;
  real chisq;
  
  int u;

  if (arg_init(interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get(0, &datasignal, &sigmasignal, &valsignal, &resultName) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xsignal) == TCL_ERROR)
    return TCL_ERROR;

  size = datasignal->size;
  if(sigmasignal->size != size) {
    Tcl_AppendResult (interp,
		      "The sigma signal should be of the same size as the input signal.",
		      (char *) NULL);
    return TCL_ERROR;
  }
  
  if (datasignal->type == REALY) 
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->x0 + i*datasignal->dx;
    }
  else if (datasignal->type == REALXY)
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->dataX[i]; 
    }
  else {
        Tcl_AppendResult (interp,
		      "Bad type for the signal (only REALY or REALXY).",
		      (char *) NULL);
    return TCL_ERROR;
  }

  ma = valsignal->size;
  if (ma != 3) {
    Tcl_AppendResult (interp,
		      "Bad number of initial value (we need 3 values dzeta,beta and lnM0).",
		      (char *) NULL);
    return TCL_ERROR;
  }
  NonLinFitInit(&a,&ia,ma,&covar,&alpha);
  for(i=0;i<valsignal->size;i++)
	{
	  a[i+1]=valsignal->dataY[i];
	  ia[i+1] = 1;
	}
  NonLinFit(X-1,datasignal->dataY-1,sigmasignal->dataY-1,size,a,ia,ma,covar,alpha,
		&chisq,&ScaleInvariance);


  result = sig_new (REALY, 0, ma+ma*ma+1);
  result->x0 = 0.0;
  result->dx = 1;
  u =0;
  for(i=1;i<=ma;i++) {
    result->dataY[u]=a[i];
    u=u+1;
  }
  result->dataY[u++]=chisq;
  result->dataY[u++]=NonLinFitConfidence(chisq,size,ma);
  for(i=1;i<=ma;i++) {
    	  for(j=1;j<=ma;j++)
	    {	      
	      result->dataY[u++]=covar[i][j];
	    }
  }


  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(resultName, result);
  
  if (arg_present(1)) {
    if (xsignal->type != REALY) {
      Tcl_AppendResult (interp,
			"Type for xsignal must be REALY!!",
			(char *) NULL);
      return TCL_ERROR;
    }
    else {
      sig_realy2realxy(xsignal);
      sig_put_y_in_x(xsignal);
      for (i=0;i<xsignal->size;i++) 
	ScaleInvariance(xsignal->dataX[i],a,&(xsignal->dataY[i]),NULL,ma);

    }


  }

  NonLinFitDelete(a,ia,ma,covar,alpha);

  return TCL_OK;

}

/*
 */
static int
_mwc_Cmd_ (ClientData clientData,
	   Tcl_Interp *interp,
	   int        argc,
	   char       **argv)      

{

  /* Command line definition */
  char * options[] = {
    "SSSs",
    "-fit","S",
    NULL
  };

  char * helpMsg = {
    (
     "  Fit the data by with a mwc model (model for tensio-actif membrane).\n"
     "  (y=f0*prefactor*(85*(h0/x-0.5 +0.1 -x^5/(h0^5*320)- 0.5 + x^2/(h0^2*8.))\n"
     " + 0.5*(h0-x/2.+h0*log(2.*h0/x)) - (h0-x/2.)/10. + (64.*h0^6 - x^6)/(3840.*h0^5)\n"
     " + (h0-x/2.)/2. - (8*h0^3-x^3)/(48*h0^2)).\n"
     "\n"
     "Arguments :\n"
     "  3 signals - signal to fit, \n"
     "              signal containing the incertaity for each point, \n"
     "              signal containing the initial values (h0,f0).\n"
     "  string    - name of the result (h0,f0,chisq,goodness,covar --> size=8)\n."
     "\n"
     "Options :\n"
     "   -fit   : signal containing the x-value (REALY)\n" 
     "            --> return the mwc fit in this signal (REALXY).\n"
     )
  };

  Signal *datasignal,*sigmasignal, *valsignal;
  Signal *result, *xsignal = NULL;
  char   *resultName;
  int i,j,size;
  
  real *X;
  int ma;
  int *ia;
  real *a,**covar,**alpha;
  real chisq;
  
  int u;

  if (arg_init(interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get(0, &datasignal, &sigmasignal, &valsignal, &resultName) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xsignal) == TCL_ERROR)
    return TCL_ERROR;

  size = datasignal->size;
  if(sigmasignal->size != size) {
    Tcl_AppendResult (interp,
		      "The sigma signal should be of the same size as the input signal.",
		      (char *) NULL);
    return TCL_ERROR;
  }
  
  if (datasignal->type == REALY) 
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->x0 + i*datasignal->dx;
    }
  else if (datasignal->type == REALXY)
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->dataX[i]; 
    }
  else {
        Tcl_AppendResult (interp,
		      "Bad type for the signal (only REALY or REALXY).",
		      (char *) NULL);
    return TCL_ERROR;
  }

  ma = valsignal->size;
  if (ma != 2) {
    Tcl_AppendResult (interp,
		      "Bad number of initial value (we need 2 values h0 and f0).",
		      (char *) NULL);
    return TCL_ERROR;
  }
  NonLinFitInit(&a,&ia,ma,&covar,&alpha);
  for(i=0;i<valsignal->size;i++)
	{
	  a[i+1]=valsignal->dataY[i];
	  ia[i+1] = 1;
	}
  NonLinFit(X-1,datasignal->dataY-1,sigmasignal->dataY-1,size,a,ia,ma,covar,alpha,
		&chisq,&MWC);


  result = sig_new (REALY, 0, ma+ma*ma+1);
  result->x0 = 0.0;
  result->dx = 1;
  u =0;
  for(i=1;i<=ma;i++) {
    result->dataY[u]=a[i];
    u=u+1;
  }
  result->dataY[u++]=chisq;
  result->dataY[u++]=NonLinFitConfidence(chisq,size,ma);
  for(i=1;i<=ma;i++) {
    	  for(j=1;j<=ma;j++)
	    {	      
	      result->dataY[u++]=covar[i][j];
	    }
  }


  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(resultName, result);
  
  if (arg_present(1)) {
    if (xsignal->type != REALY) {
      Tcl_AppendResult (interp,
			"Type for xsignal must be REALY!!",
			(char *) NULL);
      return TCL_ERROR;
    }
    else {
      sig_realy2realxy(xsignal);
      sig_put_y_in_x(xsignal);
      for (i=0;i<xsignal->size;i++) 
	MWC(xsignal->dataX[i],a,&(xsignal->dataY[i]),NULL,ma);

    }


  }

  NonLinFitDelete(a,ia,ma,covar,alpha);

  return TCL_OK;

}

/*
 */
static int
_bic_Cmd_ (ClientData clientData,
	   Tcl_Interp *interp,
	   int        argc,
	   char       **argv)      

{

  /* Command line definition */
  char * options[] = {
    "SSSs",
    "-fit","S",
    NULL
  };

  char * helpMsg = {
    (
     "  Fit the data by with a bic model (correlation functions of fractionnal brownian).\n"
     " y=s^2*pow(fabs(x+d),2.*h)+pow(fabs(x-d),2.*h)-2.*pow(fabs(x),2.*h)/2.\n"
     "\n"
     "Arguments :\n"
     "  3 signals - signal to fit, \n"
     "              signal containing the incertaity for each point, \n"
     "              signal containing the initial values (s,h,d).\n"
     "  string    - name of the result (s,h,d,chisq,goodness,covar --> size=14)\n."
     "\n"
     "Options :\n"
     "   -fit   : signal containing the x-value (REALY)\n" 
     "            --> return the bic fit in this signal (REALXY).\n"
     )
  };

  Signal *datasignal,*sigmasignal, *valsignal;
  Signal *result, *xsignal = NULL;
  char   *resultName;
  int i,j,size;
  
  real *X;
  int ma;
  int *ia;
  real *a,**covar,**alpha;
  real chisq;
  
  int u;

  if (arg_init(interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get(0, &datasignal, &sigmasignal, &valsignal, &resultName) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xsignal) == TCL_ERROR)
    return TCL_ERROR;

  size = datasignal->size;
  if(sigmasignal->size != size) {
    Tcl_AppendResult (interp,
		      "The sigma signal should be of the same size as the input signal.",
		      (char *) NULL);
    return TCL_ERROR;
  }
  
  if (datasignal->type == REALY) 
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->x0 + i*datasignal->dx;
    }
  else if (datasignal->type == REALXY)
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->dataX[i]; 
    }
  else {
        Tcl_AppendResult (interp,
		      "Bad type for the signal (only REALY or REALXY).",
		      (char *) NULL);
    return TCL_ERROR;
  }

  ma = valsignal->size;
  if (ma != 3) {
    Tcl_AppendResult (interp,
		      "Bad number of initial value (we need 3 values s,h and d).",
		      (char *) NULL);
    return TCL_ERROR;
  }
  NonLinFitInit(&a,&ia,ma,&covar,&alpha);
  for(i=0;i<valsignal->size;i++)
	{
	  a[i+1]=valsignal->dataY[i];
	  ia[i+1] = 1;
	}
  NonLinFit(X-1,datasignal->dataY-1,sigmasignal->dataY-1,size,a,ia,ma,covar,alpha,
		&chisq,&BrowIncrCorr);


  result = sig_new (REALY, 0, ma+ma*ma+1);
  result->x0 = 0.0;
  result->dx = 1;
  u =0;
  for(i=1;i<=ma;i++) {
    result->dataY[u]=a[i];
    u=u+1;
  }
  result->dataY[u++]=chisq;
  result->dataY[u++]=NonLinFitConfidence(chisq,size,ma);
  for(i=1;i<=ma;i++) {
    	  for(j=1;j<=ma;j++)
	    {	      
	      result->dataY[u++]=covar[i][j];
	    }
  }


  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(resultName, result);
  
  if (arg_present(1)) {
    if (xsignal->type != REALY) {
      Tcl_AppendResult (interp,
			"Type for xsignal must be REALY!!",
			(char *) NULL);
      return TCL_ERROR;
    }
    else {
      sig_realy2realxy(xsignal);
      sig_put_y_in_x(xsignal);
      for (i=0;i<xsignal->size;i++) 
	BrowIncrCorr(xsignal->dataX[i],a,&(xsignal->dataY[i]),NULL,ma);

    }


  }

  NonLinFitDelete(a,ia,ma,covar,alpha);

  return TCL_OK;

}

/*
 */
static int
_dnq_Cmd_ (ClientData clientData,
	   Tcl_Interp *interp,
	   int        argc,
	   char       **argv)      

{

  /* Command line definition */
  char * options[] = {
    "SSSs",
    "-fit","S",
    NULL
  };

  char * helpMsg = {
    (
     "  Fit the data by with a dnq model (fit the density function |x|^q).\n"
     " y=sqrt(M_2_PI)*x^(1/q)*exp(-x^(2/q)/(2*sigma*sigma))/(sigma*fabs(q)*x)"
     "\n"
     "Arguments :\n"
     "  3 signals - signal to fit, \n"
     "              signal containing the incertaity for each point, \n"
     "              signal containing the initial values (sigma (>0),q (!=0)).\n"
     "  string    - name of the result (sigma,q,chisq,goodness,covar --> size=8)\n."
     "\n"
     "Options :\n"
     "   -fit   : signal containing the x-value (REALY)\n" 
     "            --> return the dnq fit in this signal (REALXY).\n"
     )
  };

  Signal *datasignal,*sigmasignal, *valsignal;
  Signal *result, *xsignal = NULL;
  char   *resultName;
  int i,j,size;
  
  real *X;
  int ma;
  int *ia;
  real *a,**covar,**alpha;
  real chisq;
  
  int u;

  if (arg_init(interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get(0, &datasignal, &sigmasignal, &valsignal, &resultName) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xsignal) == TCL_ERROR)
    return TCL_ERROR;

  size = datasignal->size;
  if(sigmasignal->size != size) {
    Tcl_AppendResult (interp,
		      "The sigma signal should be of the same size as the input signal.",
		      (char *) NULL);
    return TCL_ERROR;
  }
  
  if (datasignal->type == REALY) 
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->x0 + i*datasignal->dx;
    }
  else if (datasignal->type == REALXY)
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->dataX[i]; 
    }
  else {
        Tcl_AppendResult (interp,
		      "Bad type for the signal (only REALY or REALXY).",
		      (char *) NULL);
    return TCL_ERROR;
  }

  ma = valsignal->size;
  if (ma != 2) {
    Tcl_AppendResult (interp,
		      "Bad number of initial value (we need 2 values sigma and q).",
		      (char *) NULL);
    return TCL_ERROR;
  }
  NonLinFitInit(&a,&ia,ma,&covar,&alpha);
  for(i=0;i<valsignal->size;i++)
	{
	  a[i+1]=valsignal->dataY[i];
	  ia[i+1] = 1;
	}
  NonLinFit(X-1,datasignal->dataY-1,sigmasignal->dataY-1,size,a,ia,ma,covar,alpha,
		&chisq,&DNQ);


  result = sig_new (REALY, 0, ma+ma*ma+1);
  result->x0 = 0.0;
  result->dx = 1;
  u =0;
  for(i=1;i<=ma;i++) {
    result->dataY[u]=a[i];
    u=u+1;
  }
  result->dataY[u++]=chisq;
  result->dataY[u++]=NonLinFitConfidence(chisq,size,ma);
  for(i=1;i<=ma;i++) {
    	  for(j=1;j<=ma;j++)
	    {	      
	      result->dataY[u++]=covar[i][j];
	    }
  }


  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(resultName, result);
  
  if (arg_present(1)) {
    if (xsignal->type != REALY) {
      Tcl_AppendResult (interp,
			"Type for xsignal must be REALY!!",
			(char *) NULL);
      return TCL_ERROR;
    }
    else {
      sig_realy2realxy(xsignal);
      sig_put_y_in_x(xsignal);
      for (i=0;i<xsignal->size;i++) 
	DNQ(xsignal->dataX[i],a,&(xsignal->dataY[i]),NULL,ma);

    }


  }

  NonLinFitDelete(a,ia,ma,covar,alpha);

  return TCL_OK;

}

/*
 */
static int
_gauss_Cmd_ (ClientData clientData,
	   Tcl_Interp *interp,
	   int        argc,
	   char       **argv)      

{

  /* Command line definition */
  char * options[] = {
    "SSSs",
    "-fit","S",
    NULL
  };

  char * helpMsg = {
    (
     "  Fit the data by a Gaussian (y=prefactor*exp(-(x-m)*(x-m)/(sigma*sigma)).\n"
     "\n"
     "Arguments :\n"
     "  3 signals - signal to fit, \n"
     "              signal containing the incertaity for each point, \n"
     "              signal containing the initial values (sigma (!=0),m,prefactor).\n"
     "  string    - name of the result (s,h,d,chisq,goodness,covar --> size=14)\n."
     "\n"
     "Options :\n"
     "   -fit   : signal containing the x-value (REALY)\n" 
     "            --> return the bic fit in this signal (REALXY).\n"
     )
  };

  Signal *datasignal,*sigmasignal, *valsignal;
  Signal *result, *xsignal = NULL;
  char   *resultName;
  int i,j,size;
 
  real *X;
  int ma;
  int *ia;
  real *a,**covar,**alpha;
  real chisq;
  
  int u;

  if (arg_init(interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get(0, &datasignal, &sigmasignal, &valsignal, &resultName) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &xsignal) == TCL_ERROR)
    return TCL_ERROR;

  size = datasignal->size;
  if(sigmasignal->size != size) {
    Tcl_AppendResult (interp,
		      "The sigma signal should be of the same size as the input signal.",
		      (char *) NULL);
    return TCL_ERROR;
  }
  
  if (datasignal->type == REALY) 
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->x0 + i*datasignal->dx;
    }
  else if (datasignal->type == REALXY)
    {
      X = (float *) malloc(sizeof(float)*size);
      for(i=0;i<size;i++) 
	X[i] = datasignal->dataX[i]; 
    }
  else {
        Tcl_AppendResult (interp,
		      "Bad type for the signal (only REALY or REALXY).",
		      (char *) NULL);
    return TCL_ERROR;
  }

  ma = valsignal->size;
  if (ma != 3) {
    Tcl_AppendResult (interp,
		      "Bad number of initial value (we need 2 values sigma, m and prefactor).",
		      (char *) NULL);
    return TCL_ERROR;
  }
  NonLinFitInit(&a,&ia,ma,&covar,&alpha);
  for(i=0;i<valsignal->size;i++)
	{
	  a[i+1]=valsignal->dataY[i];
	  ia[i+1] = 1;
	}
  NonLinFit(X-1,datasignal->dataY-1,sigmasignal->dataY-1,size,a,ia,ma,covar,alpha,
		&chisq,&Gauss);


  result = sig_new (REALY, 0, ma+ma*ma+1);
  result->x0 = 0.0;
  result->dx = 1;
  u =0;
  for(i=1;i<=ma;i++) {
    result->dataY[u]=a[i];
    u=u+1;
  }
  result->dataY[u++]=chisq;
  result->dataY[u++]=NonLinFitConfidence(chisq,size,ma);
  for(i=1;i<=ma;i++) {
    	  for(j=1;j<=ma;j++)
	    {	      
	      result->dataY[u++]=covar[i][j];
	    }
  }


  if (!result)
    return TCL_ERROR;
  
  store_signal_in_dictionary(resultName, result);
  
  if (arg_present(1)) {
    if (xsignal->type != REALY) {
      Tcl_AppendResult (interp,
			"Type for xsignal must be REALY!!",
			(char *) NULL);
      return TCL_ERROR;
    }
    else {
      sig_realy2realxy(xsignal);
      sig_put_y_in_x(xsignal);
      for (i=0;i<xsignal->size;i++) 
	Gauss(xsignal->dataX[i],a,&(xsignal->dataY[i]),NULL,ma);

    }


  }

  NonLinFitDelete(a,ia,ma,covar,alpha);

  return TCL_OK;

}

/*
 */
int
_my_iinssig_Cmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "ISd",
    "-y", "",
    "-45","",
    "-m45","",
    NULL
  };

  char * help_msg =
  {
    (" Insert a signal in an image at a given position.\n"
     "\n"
     "Parameters :\n"
     "  Image   - image to treat.\n"
     "  Signal  - signal to add.\n"
     "  integer - position.\n"
     "\n"
     "Options :\n"
     "  -y   : Add in the y-direction.\n"
     "  -45  : The direction of insertion is 45 degree.\n"
     "  -m45 : The direction of insertion is m45 degree.")
  };

  /* Command's parameters */
  Image  *im;
  Signal *sig;
  int    position;

  /* Options's presence */
  int is_y,is_45, is_m45;


  /* Other variables */
  int i;
  int lx,ly;
  int x;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &im, &sig, &position) == TCL_ERROR)
    return TCL_ERROR;

  is_y = arg_present(1);
  is_45 = arg_present(2);
  is_m45 = arg_present(3);

  /* Parameters validity and initialisation */
  if (is_y) {
    if (sig->size > im->ly) {
      sprintf(interp->result,
	      "The signal size (%d) must be lesser than the image heigth (%d).",
	      sig->size, im->ly);
      return TCL_ERROR;
    } 
  }
  else {
    if (sig->size > im->lx) {
      sprintf(interp->result,
	      "The signal size (%d) must be lesser than the image width (%d).",
	      sig->size, im->lx);
      return TCL_ERROR;
    }
  }
  
  /* Treatement */

  lx = im->lx;
  ly = im->ly;
  for (i = 0; i < sig->size; i++) {
    x = i;
    if (is_y) {
      if ((position >= 0) && (position < ly)) {
	if (is_m45) {
	  im->data[position+lx*(ly-x-1)] = sig->dataY[i];
	} else {
	  im->data[position+lx*x] = sig->dataY[i];
	}
      }
      position = (is_45 ? position+1 : position);
      position = (is_m45 ? position+1 : position);
    } else {
      if ((position >= 0) && (position < lx)) 
	im->data[x+lx*position] = sig->dataY[i];
      position = (is_45 ? position+1 : position);
      position = (is_m45 ? position-1 : position);
    }
  }

  return TCL_OK;
}
