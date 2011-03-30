/*
 * cv2dPkg.c --
 *
 *   Implements the cv2d package for xsmurf.
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv2dPkg.c,v 1.2 1999/05/15 15:55:05 decoster Exp $
 */

#include "smPkgInt.h"
#include <cv2d.h>

/*#include <defunc.h>*/
#include <matheval.h>

#include <math.h>

#include "../image/image.h"
#include "../image/fftw3_inc.h"

void my_fftw_init (int, int, FILE *);
void cv2d_cplx_mult_num_ana_ (complex *, double (*)(), double (*)(), int, int, real, real);


/*
 * List of prototypes of C functions that define all the commands of the
 * package.
 */

static int _cv2d_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _cv2dn_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _cv2da_Cmd_		(ClientData, Tcl_Interp*, int, char**);
static int _cv2dfft_Cmd_	(ClientData, Tcl_Interp*, int, char**);
static int _cv2dtime_Cmd_	(ClientData, Tcl_Interp*, int, char**);

/*
 * Command list.
 */

static cmdInfo Cv2dCmdInfoArray[] = {
  {"cv2d",	(Tcl_CmdProc *)_cv2d_Cmd_},
  {"cv2dn",	(Tcl_CmdProc *)_cv2dn_Cmd_},
  {"cv2da",	(Tcl_CmdProc *)_cv2da_Cmd_},
  {"cv2dfft",	(Tcl_CmdProc *)_cv2dfft_Cmd_},
  {"cv2dtime",	(Tcl_CmdProc *)_cv2dtime_Cmd_},

  {NULL,	(Tcl_CmdProc *) NULL}
};

/*
 * Cv2d_pkgInit --
 *
 *  Init function of the cv2d package.
 *
 * Arguments :
 *   Tcl_Interp - The interpreter where the package must be init.
 *
 * Return Value :
 *   TCL_OK.
 */

int
Cv2d_pkgInit (Tcl_Interp * interp)
{
  register cmdInfo *cmdInfoPtr;

  Tcl_PkgProvide (interp, "cv2d", "1.0");

  /*
   * Create the built-in commands for the package.
   */
  for (cmdInfoPtr = Cv2dCmdInfoArray; cmdInfoPtr->name != NULL; cmdInfoPtr++) {
    Tcl_CreateCommand(interp,
		      cmdInfoPtr->name,
		      cmdInfoPtr->proc,
		      (ClientData) 0, (void (*)()) NULL);
  }

  return TCL_OK;
}

/*
 * Here begins the C-description of all the cv2d package commands.
 */

/*
 */
static int
_cv2dn_Cmd_ (ClientData clientData,
	     Tcl_Interp *interp,
	     int        argc,
	     char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "IIs",
    "-mi", "",
    "-pa", "",
    "-di", "",
    "-mp", "[f]",
    "-ft", "",
    "-pe", "",
    "-0p", "",
    "-old", "",
    NULL
  };

  char * helpMsg = {
    (
     "  Compute the convolution (using the 'cv2d' library) between 2 images. "
     "The default 'border effect' used is periodic border effect. The method "
     "used depends on the sizes of both images. See the cv2d library "
     "documentation for more information on 'border effects' and on "
     "convolution methods. The filter of the convolution will be the smaller "
     "image.\n"
     "\n"
     "Arguments :\n"
     "  2 images - images to compute.\n"
     "  string   - name of the result.\n"
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
  Image *source1;
  Image *source2;
  char  *resultName;

  /* Options's presence */
  int isMi;
  int isPa;
  int isDi;
  int isMp;
  int isFt;
  int isPe;
  int is0P;
  int isOld;

  /* Options's parameters */
  real mp_mult = 2;

  /* Other variables */
  Image *result;
  Image *image;
  Image *filter;

  int    borderEffect = CV2D_PERIODIC;

  int form;
  int firstExact;
  int lastExact;

  int size;

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

  /* Parameters validity and initialisation */
  if (isOld) {
    cv2dFlag = 1;
  } else {
    cv2dFlag = 0;
  }

  /*
   * Set the filter to the smaller signal.
   */
  if (source1->lx >= source2->lx) {
    image = source1;
    filter = source2;
  } else {
    image = source2;
    filter = source1;
  }

  /* Treatement */

  /*
   * Set the method (if forced). By default the method is undefined.
   */
  cv2d_set_method (CV2D_UNDEFINED);
  if (isDi) {
    cv2d_set_method (CV2D_DI);
  }
  if (isMp) {
    cv2d_set_method (CV2D_MP);
  }
  if (isFt) {
    cv2d_set_method (CV2D_FT);
  }

  if (isMi) {
    borderEffect = CV2D_MIRROR;
  }
  if (isPa) {
    borderEffect = CV2D_PADDING;
  }
  if (isPe) {
    borderEffect = CV2D_PERIODIC;
  }
  if (is0P) {
    borderEffect = CV2D_0_PADDING;
  }

  /*
   * Filter init.
   */
  switch (filter->type) {
  case PHYSICAL:
    form = CV2D_RC_FORM;
    size = filter->lx;
    break;
  case FOURIER:
    form = CV2D_CC_FORM;
    size = filter->lx;
    break;
    default:
    /* Never reached */
      break;
  }
    /*  fZeroIndex = sig_get_index (filter, 0.0);*/
  cv2d_flt_init_n (form, size, size/2, 0, 0, filter->data, 0);

  /*
   * Signal init.
   */
  switch (image->type) {
  case PHYSICAL:
    form = CV2D_RC_FORM;
    size = image->lx;
    break;
  case FOURIER:
    form = CV2D_CC_FORM;
    size = image->lx;
    break;
    default:
    /* Never reached */
      break;
  }
  cv2d_sig_init (form, image->data, 0, size);

  if ((image->type == PHYSICAL) && (filter->type == PHYSICAL)) {
    result =  im_new (image->lx, image->ly, image->size, PHYSICAL);
  } else {
    result =  im_new (image->lx, image->ly, image->lx*image->ly*2, FOURIER);
  }

  cv2d_mp_mult = mp_mult;

  smSetBeginTime();

  cv2d_compute (borderEffect, result->data, &firstExact, &lastExact);
  smSetEndTime();

  cv2d_mp_mult = 2;

  /*  result->first = firstExact;
  result->last  = lastExact;*/

  sprintf (interp->result, "%f", smGetEllapseTime());
  /*  for (i = 1; i <= 9; i++) {
    char val_str[100];
    sprintf(val_str, "%d %d", i, zeTime[i]);
    Tcl_AppendElement(interp, val_str);
    }*/

  /*  wisdFile = fopen("haha.wisdom","w");
  my_fftw_export_wisdom_to_file (wisdFile);
  fclose(wisdFile);*/

  store_image (resultName, result);

  return TCL_OK;
}


/*
 */
static int
_cv2da_Cmd_ (ClientData clientData,
	     Tcl_Interp *interp,
	     int        argc,
	     char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "Isssffssff",
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
     "  Compute the convolution (using the 'cv2d' library) between a image and "
     "an analytical filter. The default 'border effect' used is periodic "
     "border effect. The method used depends on the sizes of both images. See "
     "the cv2d library documentation for more information on 'border effects' "
     "and on convolution methods.\n"
     "\n"
     "Arguments :\n"
     "  image     - image to compute.\n"
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
  Image *image;
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
  Image *result;

  /*double (* dRealFctPtr)() = NULL;
  double (* dImagFctPtr)() = NULL;
  double (* fRealFctPtr)() = NULL;
  double (* fImagFctPtr)() = NULL;*/
  void *dRealFctPtr;
  void *dImagFctPtr;
  void *fRealFctPtr;
  void *fImagFctPtr;


  int borderEffect = CV2D_PERIODIC;

  int firstExact;
  int lastExact;

  int form;
  int size;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, helpMsg)) {
    return TCL_OK;
  }
  
  if (arg_get (0, &image, &resultName,
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
   * Set the filter to the smaller image.
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
  cv2d_set_method (CV2D_UNDEFINED);
  if (isDi) {
    cv2d_set_method (CV2D_DI);
  }
  if (isMp) {
    cv2d_set_method (CV2D_MP);
  }
  if (isFt) {
    cv2d_set_method (CV2D_FT);
  }

  if (isMi) {
    borderEffect = CV2D_MIRROR;
  }
  if (isPa) {
    borderEffect = CV2D_PADDING;
  }
  if (isPe) {
    borderEffect = CV2D_PERIODIC;
  }
  if (is0P) {
    borderEffect = CV2D_0_PADDING;
  }

  /*
   * Filter init.
   */
  cv2d_flt_init_a (dBegin, dEnd, fBegin, fEnd,
		   dRealFctPtr, dImagFctPtr,
		   fRealFctPtr, fImagFctPtr,
		   CV2D_NO_SCALE);

  /*
   * Image init.
   */
  switch (image->type) {
  case PHYSICAL:
    form = CV2D_RC_FORM;
    size = image->lx;
    break;
  case FOURIER:
    form = CV2D_CC_FORM;
    size = image->lx;
    break;
  default:
    /* Never reached */
    break;
  }
  cv2d_sig_init (form, image->data, 0, size);

  if ((image->type == PHYSICAL) && (dImagFctPtr == NULL)) {
    result =  im_new (image->lx, image->ly, image->size, PHYSICAL);
  } else {
    result =  im_new (image->lx, image->ly, image->lx*image->ly*2, FOURIER);
  }

  smSetBeginTime();
  cv2d_compute (borderEffect, result->data, &firstExact, &lastExact);
  smSetEndTime();

  /*  result->first = firstExact;
  result->last  = lastExact;*/

  sprintf (interp->result, "%f", smGetEllapseTime());

  store_image (resultName, result);

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

/*static int
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
_cv2dfft_Cmd_ (ClientData clientData,
	       Tcl_Interp *interp,
	       int        argc,
	       char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "Is",
    "-reverse", "",
    "-getwisdom", "",
    "-nb", "d",
    NULL
  };

  char * helpMsg = {
    (
     "  Compute the fft of a signal using the fft algorithm used by the cv2d "
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
  Image *image;
  char   *resultName;

  /* Options's presence */
  int isReverse;
  int isGetwisdom;
  int isNb;

  /* Options's parameters */
  int nb = 1;

  /* Other variables */
  Image *result;
  
  int i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, helpMsg)) {
    return TCL_OK;
  }
  
  if (arg_get (0, &image, &resultName) == TCL_ERROR) {
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
/*   if (!isReverse && signal->type != REALY) { */
/*     sprintf (interp->result,"bad signal type: must be REALY"); */
/*     return TCL_ERROR; */
/*   } */

/*   if (isReverse && signal->type != CPLX) { */
/*     sprintf (interp->result,"bad signal type: must be CPLX"); */
/*     return TCL_ERROR; */
/*   } */

  /*  if (!_is_power_of_2_(signal->n)) {
    sprintf (interp->result,"bad signal size: must be a power of 2");
    return TCL_ERROR;
  }*/

  /* Treatement */

/*   if (!isReverse) { */
/*     if (signal->n%2 == 0) { */
/*       result = sig_new (CPLX, 0, signal->n/2-1); */
/*     } else { */
/*       result = sig_new (CPLX, 0, signal->n/2); */
/*       result->dataY[signal->n] = 0; */
/*     } */

  /*    smSetBeginTime();*/

    /*    for (i = 0; i < nb; i++) {*/
  /*    zeNb = nb;*/
    /*      cv2d_fft_r (signal->dataY, (complex *)result->dataY, signal->n);*/
      /*    }*/
  /*
    smSetEndTime();
  } else {*/
  result =  im_new (image->lx, image->ly+2, image->lx*(image->ly+2), PHYSICAL);

  smSetBeginTime();

  for (i = 0; i < nb; i++) {
    cv2d_fft_r (image->data, (complex *) result->data, image->lx, image->ly);
  }

    smSetEndTime();
    /*  }*/

  /*  result->first = firstExact;
  result->last  = lastExact;*/

    /*  if (isGetwisdom) {
    char *string;
    string = my_fftw_export_wisdom_to_string();
    Tcl_AppendResult(interp, string, (char *) NULL);
    my_fftw_free(string);
  } else {*/
    sprintf (interp->result, "%f", smGetEllapseTime());
    /*  }*/

  store_image (resultName, result);

  return TCL_OK;

  //tclError:

  //return TCL_ERROR;
}


/*
 */
static int
_cv2dtime_Cmd_ (ClientData clientData,
		Tcl_Interp *interp,
		int        argc,
		char       **argv)      
{
  /* Command line definition */
  char * options[] = {
    "dd",
    "-nb", "d",
    NULL
  };

  char * helpMsg = {
    (
     "  Time stats.\n"
     )
  };

  /* Command's parameters */
  int n;
  int m;

  /* Options's presence */
  int isNb;

  /* Options's parameters */
  int nb = 1;

  /* Other variables */
  complex *s;
  complex *f;
  
  int i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, helpMsg)) {
    return TCL_OK;
  }
  
  if (arg_get (0, &n, &m) == TCL_ERROR) {
    return TCL_ERROR;
  }
  
  isNb = arg_present(1);
  if (isNb) {
    if (arg_get (1, &nb) == TCL_ERROR)
      return TCL_ERROR;
  }

  /* Parameters validity and initialisation */

  /* Treatement */

  s = (complex *) malloc(sizeof(complex)*n);
  f = (complex *) malloc(sizeof(complex)*n);
  for (i = 0; i < n; i++) {
    s[i].real = i;
    s[i].imag = i;
    f[i].real = i;
    f[i].imag = i;
  }

  smSetBeginTime();

  for (i = 0; i < nb; i++) {
    cv2d_cplx_mult_num_ana_ (s, cos, sin, 0, n-1, 1, 0);
  }

  smSetEndTime();

  free (s);
  free (f);

  sprintf (interp->result, "%f", smGetEllapseTime());

  return TCL_OK;

  //tclError:

  //return TCL_ERROR;
}


/*
 */
static int
_cv2d_Cmd_ (ClientData clientData,
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

    cmdInfoPtr = Cv2dCmdInfoArray;
    Tcl_AppendResult (interp, cmdInfoPtr->name, (char *) NULL);
    for (cmdInfoPtr++;
	 cmdInfoPtr->name != NULL;
	 cmdInfoPtr++) {
      Tcl_AppendResult (interp, " ", cmdInfoPtr->name, (char *) NULL);
    }

  } else if (isGetsum) {

    sprintf (interp->result, "%g", cv2d_get_flt_squared_mod_sum ());

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
      p = my_fftw_plan_dft_r2c_1d(n, gah, (FFTW_COMPLEX *) gah, FFTW_MEASURE);
      /*      cv2d_fft_r(gah, (complex *) gah, n);*/
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

    sprintf (interp->result, "The cv2d package blablabla.");

  }

  return TCL_OK;
}
