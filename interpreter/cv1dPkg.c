/*
 * cv1dPkg.c --
 *
 *   Implements the cv1d package for xsmurf.
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv1dPkg.c,v 1.12 1999/05/05 20:42:27 decoster Exp $
 */

#include "smPkgInt.h"
#include <cv1d.h>
#include <myFftw.h>
#include "../image/fftw3_inc.h"

/*#include <defunc.h>*/
#include <matheval.h>

#include <math.h>

#include <sys/times.h>
#include <time.h>

#define FIRST_SIZE 2

/*
 * Timer facility. Inspired by fftw.
 */

#define _my_get_time() clock()
#define _my_time_diff(t1,t2) ((t1) - (t2))
#define _my_time_2_sec(t) (((double) (t)) / CLOCKS_PER_SEC)

#define _my_set_begin_time() (_myTimeBegin = _my_time_2_sec(_my_get_time()))
#define _my_set_end_time() (_myTimeEnd = _my_time_2_sec(_my_get_time()))
#define _my_get_ellapse_time() (_my_time_diff(_my_time_end, _my_time_begin))

/*
 * ***VERY*** conservative constant: this says that a
 * measurement must run for 200ms in order to be valid.
 * You had better check the manual of your machine
 * to discover if it can do better than this
 */
#define _MY_TIME_MIN (20.0e-1)	/* for default clock() timer */

/* take _MY_TIME_REPEAT measurements... */
#define _MY_TIME_REPEAT 1

/* but do not run for more than _MY_TIME_LIMIT seconds while measuring something */
#ifndef _MY_TIME_LIMIT
#define _MY_TIME_LIMIT 2.0
#endif

/*static real _my_time_begin;
  static real _my_time_end;*/

static real min_time = 1;

/* 
 * The relative path of the following line is to avoid confusion with the
 * system header 'signal.h'.
 */

#include "../signal/signal.h"

/*
 * List of prototypes of C functions that define all the commands of the
 * package.
 */

static int _cv1d_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _cv1dn_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _cv1da_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _cv1dfft_Cmd_	(ClientData, Tcl_Interp*, int, char**);
static int _cv1dtime_Cmd_	(ClientData, Tcl_Interp*, int, char**);

/*
 * Command list.
 */

static cmdInfo Cv1dCmdInfoArray[] = {
  {"cv1d",	(Tcl_CmdProc *) _cv1d_Cmd_},
  {"cv1dn",	(Tcl_CmdProc *)_cv1dn_Cmd_},
  {"cv1da",	(Tcl_CmdProc *)_cv1da_Cmd_},
  {"cv1dfft",	(Tcl_CmdProc *)_cv1dfft_Cmd_},
  {"cv1dtime",	(Tcl_CmdProc *)_cv1dtime_Cmd_},

  {NULL,	(Tcl_CmdProc *) NULL}
};

/*
 * Cv1d_pkgInit --
 *
 *  Init function of the cv1d package.
 *
 * Arguments :
 *   Tcl_Interp - The interpreter where the package must be init.
 *
 * Return Value :
 *   TCL_OK.
 */

int
Cv1d_pkgInit (Tcl_Interp * interp)
{
  register cmdInfo *cmdInfoPtr;

  Tcl_PkgProvide (interp, "cv1d", "1.0");

  /*
   * Create the built-in commands for the package.
   */
  for (cmdInfoPtr = Cv1dCmdInfoArray; cmdInfoPtr->name != NULL; cmdInfoPtr++) {
    Tcl_CreateCommand(interp,
		      cmdInfoPtr->name,
		      cmdInfoPtr->proc,
		      (ClientData) 0, (void (*)()) NULL);
  }

  return TCL_OK;
}

/*
 * Here begins the C-description of all the cv1d package commands.
 */

/*
 */
static int
_cv1dn_Cmd_ (ClientData clientData,
	     Tcl_Interp *interp,
	     int        argc,
	     char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "SSs",
    "-mi", "",
    "-pa", "",
    "-di", "",
    "-mp", "[f]",
    "-ft", "",
    "-pe", "",
    "-0p", "",
    "-old", "",
    "-mptest", "d",
    "-fttest", "",
    "-fft", "",
    "-ditest", "",
    NULL
  };

  char * helpMsg = {
    (
     "  Compute the convolution (using the 'cv1d' library) between 2 signals. "
     "The default 'border effect' used is periodic border effect. The method "
     "used depends on the sizes of both signals. See the cv1d library "
     "documentation for more information on 'border effects' and on "
     "convolution methods. The filter of the convolution will be the smaller "
     "signal.\n"
     "\n"
     "Arguments :\n"
     "  2 signals - signals to compute.\n"
     "  string    - name of the result.\n"
     "\n"
     "Options :\n"
     "  -pe : Peridodic border effect.\n"
     "  -mi : Mirror border effect.\n"
     "  -pa : Padding border effect.\n"
     "  -di : Direct convolution method.\n"
     "  -mp : Multi-part convolution method.\n"
     "  -ft : Fourier transform convolution method.\n"
     "  -0p : Zero padding border effect."
     )
  };

  /* Command's parameters */
  Signal *source1;
  Signal *source2;
  char   *resultName;

  /* Options's presence */
  int isMi;
  int isPa;
  int isDi;
  int isMp;
  int isFt;
  int isPe;
  int is0P;
  int isOld;
  int isMptest;
  int isFttest;
  int isFft;
  int isDitest;

  /* Options's parameters */
  real mp_mult = 2;
  int  P;

  /* Other variables */
  Signal *result;
  Signal *signal;
  Signal *filter;

  int    borderEffect = CV1D_PERIODIC;
  int    fZeroIndex;

  /*int i;*/

  int form;
  int firstExact;
  int lastExact;

  int size;

  /*FILE *wisdFile;*/

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get (0, &source1, &source2, &resultName) == TCL_ERROR)
    return TCL_ERROR;
  
  isMi = arg_present(1);
  isPa = arg_present(2);
  isDi = arg_present(3);
  isMp = arg_present(4);
  if (isMp) {
    if (arg_get (4, &mp_mult) == TCL_ERROR)
      return TCL_ERROR;
  }
  isFt = arg_present(5);
  isPe = arg_present(6);
  is0P = arg_present(7);
  isOld = arg_present(8);
  isMptest = arg_present(9);
  if (isMptest) {
    if (arg_get (9, &P) == TCL_ERROR)
      return TCL_ERROR;
  }
  isFttest = arg_present(10);
  isFft = arg_present(11);
  isDitest = arg_present(12);

  /* Parameters validity and initialisation */
  if (isOld) {
    cv1dFlag = 1;
  } else {
    cv1dFlag = 0;
  }

  /*
   * Set the filter to the smaller signal.
   */
  if (source1->size >= source2->size) {
    signal = source1;
    filter = source2;
  } else {
    signal = source2;
    filter = source1;
  }

  if ((filter->x0 > 0)
      || ((filter->x0 + filter->dx*filter->n) < 0)) {
    sprintf (interp->result,"Filter domain must contain zero. (o yes)");
    return TCL_ERROR;
  }

  if (filter->type == REALXY) {
    sprintf (interp->result, "bad filter type");
    return TCL_ERROR;
  }

  if (signal->type == REALXY) {
    sprintf (interp->result,"bad signal type");
    return TCL_ERROR;
  }

  /* Treatement */

  /*
   * Set the method (if forced). By default the method is undefined.
   */
  cv1d_set_method (CV1D_UNDEFINED);
  if (isDi) {
    cv1d_set_method (CV1D_DI);
  }
  if (isMp) {
    cv1d_set_method (CV1D_MP);
  }
  if (isFt) {
    cv1d_set_method (CV1D_FT);
  }

  if (isMi) {
    borderEffect = CV1D_MIRROR;
  }
  if (isPa) {
    borderEffect = CV1D_PADDING;
  }
  if (isPe) {
    borderEffect = CV1D_PERIODIC;
  }
  if (is0P) {
    borderEffect = CV1D_0_PADDING;
  }

  /*
   * Filter init.
   */
  switch (filter->type) {
  case REALY:
    form = CV1D_RC_FORM;
    size = filter->size;
    break;
  case FOUR_NR:
  case CPLX:
    form = CV1D_CC_FORM;
    size = filter->size/2;
    break;
  default:
    /* Never reached */
    break;
  }
  fZeroIndex = sig_get_index (filter, 0.0);
  cv1d_flt_init_n (form, size, fZeroIndex, 0, 0, filter->dataY, 0);

  /*
   * Signal init.
   */
  switch (signal->type) {
  case REALY:
    form = CV1D_RC_FORM;
    size = signal->size;
    break;
  case FOUR_NR:
  case CPLX:
    form = CV1D_CC_FORM;
    size = signal->size/2;
    break;
  default:
    /* Never reached */
    break;
  }
  cv1d_sig_init (form, signal->dataY, 0, size);

  if ((signal->type == REALY) && (filter->type == REALY)) {
    result = sig_new (REALY, 0, size - 1);
  } else {
    result = sig_new (FOUR_NR, 0, size - 1);
  }

  cv1d_mp_mult = mp_mult;

  if ((isDitest && isDi) || (isFttest && isFt) || (isMptest && isMp) || isFft) {
    real t = 0;
    int  i;
    int  iter = 1;
    int  begin;
    int  end;

    /*void (* zeCvFct) (int, void *, int  *, int  *); 
      (void *) (* zeCvFct) (int, void *, int  *, int  *); 
      (void *)  zeCvFct (int, void *, int  *, int  *);*/
    void *  (* zeCvFct)() ;

    cv1dFlag = 2;
    cv1d_mp_mult = P;

    if (isFttest) {
      zeCvFct = cv1d_n_real_ft;
    } else if (isDitest) {
      zeCvFct = cv1d_n_real_d;
    } else if (isMptest) {
      zeCvFct = cv1d_n_real_mp;
    }

    while (t < min_time) {
      if (isFft) {
	begin = _my_get_time ();
	for (i = 0; i < iter; ++i) {
	  cv1d_fft_r_i ((complex *)signal->dataY, result->dataY, result->n);
	}
	end = _my_get_time ();
      } else {
	begin = _my_get_time ();
	for (i = 0; i < iter; ++i) {
	  zeCvFct (borderEffect, result->dataY, &firstExact, &lastExact);
	}
	end = _my_get_time ();
      }
      t = _my_time_2_sec (_my_time_diff (end, begin));
      iter *= 2;
    }
    t /= (double) iter;

    sprintf (interp->result, "%f %d", t, iter/2);
  } else {
    smSetBeginTime();
    cv1d_compute (borderEffect, result->dataY, &firstExact, &lastExact);
    smSetEndTime();

    sprintf (interp->result, "%f", smGetEllapseTime());
  }

  cv1d_mp_mult = 2;

  /*  result->first = firstExact;
  result->last  = lastExact;*/

  /*  wisdFile = fopen("haha.wisdom","w");
  my_fftw_export_wisdom_to_file (wisdFile);
  fclose(wisdFile);*/

  store_signal_in_dictionary (resultName, result);

  return TCL_OK;
}


/*
 */
static int
_cv1da_Cmd_ (ClientData clientData,
	     Tcl_Interp *interp,
	     int        argc,
	     char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "Ssssffssff",
    "-mi", "",
    "-pa", "",
    "-di", "",
    "-mp", "",
    "-ft", "",
    "-pe", "",
    "-0p", "",
    NULL
  };

  char * helpMsg = {
    (
     "  Compute the convolution (using the 'cv1d' library) between a signal and "
     "an analytical filter. The default 'border effect' used is periodic "
     "border effect. The method used depends on the sizes of both signals. See "
     "the cv1d library documentation for more information on 'border effects' "
     "and on convolution methods.\n"
     "\n"
     "Arguments :\n"
     "  signal     - signal to compute.\n"
     "  string     - name of the result.\n"
     "  string     - math expr of the real part of the direct form of the\n"
     "               filter.\n"
     "  string     - math expr of the imaginary part of the direct form of\n"
     "               the filter.\n"
     "  2 integers - domain of the direct form.\n"
     "  string     - math expr of the real part of the Fourier form of the\n"
     "               filter.\n"
     "  string     - math expr of the imaginary part of the Fourier form of\n"
     "               the filter.\n"
     "  2 integers - domain of the Fourier form.\n"
     "\n"
     "Options :\n"
     "  -pe : Peridodic border effect.\n"
     "  -mi : Mirror border effect.\n"
     "  -pa : Padding border effect.\n"
     "  -di : Direct convolution method.\n"
     "  -mp : Multi-part convolution method.\n"
     "  -ft : Fourier transform convolution method.\n"
     "  -0p : Zero padding border effect."
     )
  };

  /* Command's parameters */
  Signal *signal;
  char   *resultName;
  char   *dRealExpr;
  char   *dImagExpr;
  float  dBegin;
  float  dEnd;
  char   *fRealExpr;
  char   *fImagExpr;
  float  fBegin;
  float  fEnd;

  /* Options's presence */
  int isMi;
  int isPa;
  int isDi;
  int isMp;
  int isFt;
  int isPe;
  int is0P;

  /* Options's parameters */

  /* Other variables */
  Signal *result;

  /*double (* dRealFctPtr)() = NULL;
  double (* dImagFctPtr)() = NULL;
  double (* fRealFctPtr)() = NULL;
  double (* fImagFctPtr)() = NULL;*/
  void *dRealFctPtr;
  void *dImagFctPtr;
  void *fRealFctPtr;
  void *fImagFctPtr;


  int    borderEffect = CV1D_PERIODIC;

  int firstExact;
  int lastExact;

  int form;
  int size;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, helpMsg)) {
    return TCL_OK;
  }
  
  if (arg_get (0, &signal, &resultName,
	       &dRealExpr, &dImagExpr, &dBegin, &dEnd,
	       &fRealExpr, &fImagExpr, &fBegin, &fEnd) == TCL_ERROR) {
    return TCL_ERROR;
  }
  
  isMi = arg_present(1);
  isPa = arg_present(2);
  isDi = arg_present(3);
  isMp = arg_present(4);
  isFt = arg_present(5);
  isPe = arg_present(6);
  is0P = arg_present(7);

  /* Parameters validity and initialisation */

  /*
   * Set the filter to the smaller signal.
   */
  if (dBegin > 0 || dEnd < 0) {
    sprintf (interp->result,"Filter direct domain must contain zero. (o yes)");
    goto tclError;
  }

  if (fBegin > 0 || fEnd < 0) {
    sprintf (interp->result,"Filter Fourier domain must contain zero. (o yes)");
    goto tclError;
  }

  if (strcmp(dRealExpr, "null") != 0) {
    /*dRealFctPtr = dfopen(dRealExpr);*/
    dRealFctPtr = evaluator_create(dRealExpr);
    if (!dRealFctPtr) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ", dRealExpr, (char *) NULL);
      goto tclError;
    }
  }
  if (strcmp(dImagExpr, "null") != 0) {
    /*dImagFctPtr = dfopen(dImagExpr);*/
    dImagFctPtr = evaluator_create(dImagExpr);
    if (!dImagFctPtr) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ", dImagExpr, (char *) NULL);
      goto tclError;
    }
  }
  if (strcmp(fRealExpr, "null") != 0) {
    /*fRealFctPtr = dfopen(fRealExpr);*/
    fRealFctPtr = evaluator_create(fRealExpr);
    if (!fRealFctPtr) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ", fRealExpr, (char *) NULL);
      goto tclError;
    }
  }
  if (strcmp(fImagExpr, "null") != 0) {
    /*fImagFctPtr = dfopen(fImagExpr);*/
    fImagFctPtr = evaluator_create(fImagExpr);
    if (!fImagFctPtr) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ", fImagExpr, (char *) NULL);
      goto tclError;
    }
  }

  if (!dRealFctPtr && !dImagFctPtr && !fRealFctPtr && !fImagFctPtr) {
    Tcl_AppendResult (interp, "one of the math expression must be non \"null\"", NULL);
    goto tclError;
  }

  /* Treatement */

  /*
   * Set the method (if forced). By default the method is undefined.
   */
  cv1d_set_method (CV1D_UNDEFINED);
  if (isDi) {
    cv1d_set_method (CV1D_DI);
  }
  if (isMp) {
    cv1d_set_method (CV1D_MP);
  }
  if (isFt) {
    cv1d_set_method (CV1D_FT);
  }

  if (isMi) {
    borderEffect = CV1D_MIRROR;
  }
  if (isPa) {
    borderEffect = CV1D_PADDING;
  }
  if (isPe) {
    borderEffect = CV1D_PERIODIC;
  }
  if (is0P) {
    borderEffect = CV1D_0_PADDING;
  }

  /*
   * Filter init.
   */
  cv1d_flt_init_a (dBegin, dEnd, fBegin, fEnd,
		   dRealFctPtr, dImagFctPtr,
		   fRealFctPtr, fImagFctPtr,
		   1/signal->dx);

  /*
   * Signal init.
   */
  switch (signal->type) {
  case REALY:
    form = CV1D_RC_FORM;
    size = signal->size;
    break;
  case FOUR_NR:
  case CPLX:
    form = CV1D_CC_FORM;
    size = signal->size/2;
    break;
  default:
    /* Never reached */
    break;
  }
  cv1d_sig_init (form, signal->dataY, 0, size);

  if ((signal->type == REALY) && (dImagFctPtr == NULL)) {
    result = sig_new (REALY, 0, size - 1);
  } else {
    result = sig_new (FOUR_NR, 0, size - 1);
  }

  smSetBeginTime();
  cv1d_compute (borderEffect, result->dataY, &firstExact, &lastExact);
  smSetEndTime();

  /*  result->first = firstExact;
  result->last  = lastExact;*/

  sprintf (interp->result, "%f", smGetEllapseTime());

  store_signal_in_dictionary (resultName, result);

  /*dfclose(dRealFctPtr);
  dfclose(dImagFctPtr);
  dfclose(fRealFctPtr);
  dfclose(fImagFctPtr);*/
  evaluator_destroy(dRealFctPtr);
  evaluator_destroy(dImagFctPtr);
  evaluator_destroy(fRealFctPtr);
  evaluator_destroy(fImagFctPtr);

  return TCL_OK;

tclError:
  if (dRealFctPtr) {
    /*dfclose(dRealFctPtr);*/
    evaluator_destroy(dRealFctPtr);
  }
  if (dImagFctPtr) {
    /*dfclose(dImagFctPtr);*/
    evaluator_destroy(dImagFctPtr);
  }
  if (fRealFctPtr) {
    /*dfclose(fRealFctPtr);*/
    evaluator_destroy(fRealFctPtr);
  }
  if (fImagFctPtr) {
    /*dfclose(fImagFctPtr);*/
    evaluator_destroy(fImagFctPtr);
  }

  return TCL_ERROR;
}


/*
 */

/*
static int
_is_power_of_2_ (int i)
{
  real j;

  for (j = i; j > 1; j /= 2);

  return (j == 1);
}
*/

 /*extern int zeNb;*/

/*
 */
static int
_cv1dfft_Cmd_ (ClientData clientData,
	       Tcl_Interp *interp,
	       int        argc,
	       char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "Ss",
    "-reverse", "",
    "-getwisdom", "",
    "-nb", "d",
    NULL
  };

  char * helpMsg = {
    (
     "  Compute the fft of a signal using the fft algorithm used by the cv1d "
     "library. This command only compute fft's on real signal.\n"
     "\n"
     "Arguments :\n"
     "  signal - Tignal to transform Must be of type REALY (or CPLX for "
     "           inverse fft).\n"
     "  string - Name of the result.\n"
     "\n"
     "Options :\n"
     "  -reverse : Compute the inverse fft."
     )
  };

  /* Command's parameters */
  Signal *signal;
  char   *resultName;

  /* Options's presence */
  int isReverse;
  int isGetwisdom;
  int isNb;

  /* Options's parameters */
  int nb = 1;

  /* Other variables */
  Signal *result;
  /*int    begin;
    int    end;
    struct tms time;*/
  int i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, helpMsg)) {
    return TCL_OK;
  }
  
  if (arg_get (0, &signal, &resultName) == TCL_ERROR) {
    return TCL_ERROR;
  }
  
  isReverse = arg_present(1);
  isGetwisdom = arg_present(2);
  isNb = arg_present(3);
  if (isNb) {
    if (arg_get (3, &nb) == TCL_ERROR)
      return TCL_ERROR;
  }

  /* Parameters validity and initialisation */
  if (!isReverse && signal->type != REALY) {
    sprintf (interp->result,"bad signal type: must be REALY");
    return TCL_ERROR;
  }

  if (isReverse && signal->type != CPLX) {
    sprintf (interp->result,"bad signal type: must be CPLX");
    return TCL_ERROR;
  }

  /*  if (!_is_power_of_2_(signal->n)) {
    sprintf (interp->result,"bad signal size: must be a power of 2");
    return TCL_ERROR;
  }*/

  /* Treatement */

  if (!isReverse) {
    if (signal->n%2 == 0) {
      result = sig_new (CPLX, 0, signal->n/2-1);
    } else {
      result = sig_new (CPLX, 0, signal->n/2);
      result->dataY[signal->n] = 0;
    }

    smSetBeginTime();

    /*    for (i = 0; i < nb; i++) {*/
    /*zeNb = nb;*/
    cv1d_fft_r (signal->dataY, (complex *)result->dataY, signal->n);
    /*    }*/

    smSetEndTime();
  } else {
    result = sig_new (REALY, 0, signal->n*2-2);

    smSetBeginTime();

    for (i = 0; i < nb; i++) {
      cv1d_fft_r_i ((complex *)signal->dataY, result->dataY, result->n);
    }

    smSetEndTime();
  }

  /*  result->first = firstExact;
  result->last  = lastExact;*/

  if (isGetwisdom) {
    char *string;
    string = my_fftw_export_wisdom_to_string();
    Tcl_AppendResult(interp, string, (char *) NULL);
    fftwf_free(string);
  } else {
    sprintf (interp->result, "%f", smGetEllapseTime());
  }

  store_signal_in_dictionary (resultName, result);

  return TCL_OK;

  /*tclError:*/

  return TCL_ERROR;
}


/*
 */
static int
_cv1dtime_Cmd_ (ClientData clientData,
		Tcl_Interp *interp,
		int        argc,
		char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "sdd",
    "-nb", "d",
    "-border", "d",
    NULL
  };

  char * helpMsg = {
    (
     "  Time stats.\n"
     )
  };

  /* Command's parameters */
  int n_min;
  int n_max;
  char *fileName;

  /* Options's presence */
  int isNb;
  int isBorder;

  /* Options's parameters */
  int nb = 1;
  int borderEffect = CV1D_PERIODIC;

  /* Other variables */
  /*complex *s;
    complex *f;
    int  begin;
    int  end;
    struct tms time;
    int i;*/
  int code;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, helpMsg)) {
    return TCL_OK;
  }
  
  if (arg_get (0, &fileName, &n_min, &n_max) == TCL_ERROR) {
    return TCL_ERROR;
  }
  
  isNb = arg_present(1);
  if (isNb) {
    if (arg_get (1, &nb) == TCL_ERROR)
      return TCL_ERROR;
  }

  isBorder = arg_present(2);
  if (isBorder) {
    if (arg_get (2, &borderEffect) == TCL_ERROR)
      return TCL_ERROR;
  }

  /* Parameters validity and initialisation */

  if (borderEffect != CV1D_PERIODIC
      && borderEffect != CV1D_MIRROR
      && borderEffect != CV1D_PADDING
      && borderEffect != CV1D_0_PADDING) {
    sprintf (interp->result,"bad border effect");
    return TCL_ERROR;
  }

  /* Treatement */

  /*  code = cv1d_compute_and_save(fileName, n_min, n_max, borderEffect);*/

  if (code == 0) {
    return TCL_ERROR;
  }

  return TCL_OK;
}


/*
 */
static int
_cv1d_Cmd_ (ClientData clientData,
	    Tcl_Interp *interp,
	    int        argc,
	    char       **argv)
{ 
  /* Command line definition */
  char * options[] = {
    "",
    "cmdlist", "",
    "getsum", "",
    "comp", "d[d]",
    "wisload", "s",
    "wissave", "s",
    "fftwtime", "sdd",
    NULL
  };

  char * helpMsg = {
    (
     " cmdlist - List the C-defined commands of the package.\n"
     " getsum  - Get the last computed sum of the squared of the modulus of the filter."
     )
  };

  /* Command's parameters */

  /* Options's presence */
  int isCmdlist;
  int isGetsum;
  int isComp;
  int isWisload;
  int isWissave;
  int isFftwtime;

  /* Options's parameters */
  int n;
  int nMin;
  int nMax;
  int nb = 1;
  char *fName;

  /* Other variables */
  register cmdInfo *cmdInfoPtr;
  FILE *fWis;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, helpMsg))
    return TCL_OK;
  
  if (arg_get (0) == TCL_ERROR)
    return TCL_ERROR;

  isCmdlist = arg_present (1);
  isGetsum = arg_present (2);
  isComp = arg_present (3);
  if (isComp) {
    if (arg_get (3, &n, &nb) == TCL_ERROR)
      return TCL_ERROR;
  }
  isWisload = arg_present (4);
  if (isWisload) {
    if (arg_get (4, &fName) == TCL_ERROR)
      return TCL_ERROR;
  }
  isWissave = arg_present (5);
  if (isWissave) {
    if (arg_get (5, &fName) == TCL_ERROR)
      return TCL_ERROR;
  }
  isFftwtime = arg_present (6);
  if (isFftwtime) {
    if (arg_get (6, &fName, &nMin, &nMax) == TCL_ERROR)
      return TCL_ERROR;
  }

  /* Parameters validity and initialisation */

  /* Treatement */

  if (isCmdlist) {

    cmdInfoPtr = Cv1dCmdInfoArray;
    Tcl_AppendResult (interp, cmdInfoPtr->name, (char *) NULL);
    for (cmdInfoPtr++;
	 cmdInfoPtr->name != NULL;
	 cmdInfoPtr++) {
      Tcl_AppendResult (interp, " ", cmdInfoPtr->name, (char *) NULL);
    }

  } else if (isGetsum) {

    sprintf (interp->result, "%g", cv1d_get_flt_squared_mod_sum ());

  } else if (isComp) {

    int i;
    FFTW_REAL *gah;
    FFTW_PLAN p;

    gah = (FFTW_REAL*) malloc(n*sizeof(real));
        for (i = 0; i< n; i++) {
      gah[i] = 1;
    }
    smSetBeginTime();
    for (i = 0; i < nb; i++) {
      p = my_fftw_plan_dft_r2c_1d(n, gah, (FFTW_COMPLEX*) gah, FFTW_MEASURE);
      /*      cv1d_fft_r(gah, (complex *) gah, n);*/
      /*      for (j = 0; j < n/2; j++) {
	gah[2*j] = gah[j];
	gah[2*j+1] = gah[n-j];
      }*/
    }
    smSetEndTime();
    free(gah);
    sprintf (interp->result, "%f", smGetEllapseTime());

  } else if (isWisload) {

    fWis = fopen(fName, "r");
    my_fftw_import_wisdom_from_file(fWis);
    fclose(fWis);

    sprintf (interp->result, "%s", fName);

  } else if (isWissave) {

    fWis = fopen(fName, "w");
    my_fftw_export_wisdom_to_file(fWis);
    fclose(fWis);

    sprintf (interp->result, "%s", fName);

  } else if (isFftwtime) {

    fWis = fopen(fName, "w"); 
    my_fftw_init (nMin, nMax, fWis);
    fclose(fWis);

    sprintf (interp->result, "%s", fName);

  } else {

    sprintf (interp->result, "The cv1d package blablabla.");

  }

  return TCL_OK;
}
