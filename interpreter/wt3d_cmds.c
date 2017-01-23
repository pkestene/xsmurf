/*
 * wt3d_cmds.c --
 *
 *   Copyright 2002-2003 Laboratoire de Physique, ENS Lyon, France
 *   Copyright 2003-20XX DSM/IRFU/SEDI, CEA, Saclay, France
 *  Written by Pierre Kestener (inspired from Nicolas Decoster wt2d_cmds.c).
 *
 *  The author may be reached (Email) at the address
 *      pierre.kestener@cea.fr
 *
 */

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
/*#include <defunc.h>*/
#include <matheval.h>
#include "../wt2d/wt2d.h"
#include "../wt3d/wt3d.h"
#include "../stats/stats.h"
#include "../wt2d/skeleton.h"
#include "../image/image.h"
#include "../image/image3D.h"

#include "../edge/extrema.h"

#include <assert.h>

#include "smPkgInt.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef PI
#define PI 3.14159265358979323846
#endif


int isBigEndian()
{
   short word = 0x4321;
   if((*(char *)& word) != 0x21 )
     return 1;
   else 
     return 0;
}

/************************************************
 *   fonction pour echanger les octets
 *  Big-->LitEndian et inversement
 *  repris dans Lastwave 1.7
 ************************************************/
void BigLittleValues(void *array, int n, size_t sizeval);

/*
 * pos_incr is an array that is usefull to describe the
 * 26 neighbors (3x3x3-1) of a given extrema.
 * recall that 3d position is given by:
 * index = i + lx*j + lx*ly*k
 */

static int pos_incr[27];

static void
init_pos_incr(int lx, int ly)
{
  pos_incr[0]  = -1 - lx -lx*ly;
  pos_incr[1]  =    - lx -lx*ly;
  pos_incr[2]  =  1 - lx -lx*ly;
  pos_incr[3]  =  1      -lx*ly;
  pos_incr[4]  =  1 + lx -lx*ly;
  pos_incr[5]  =      lx -lx*ly;
  pos_incr[6]  = -1 + lx -lx*ly;
  pos_incr[7]  =  -1     -lx*ly;
  pos_incr[8]  =         -lx*ly;
  pos_incr[9]  = -1 - lx;
  pos_incr[10] =    - lx;
  pos_incr[11] =  1 - lx;
  pos_incr[12] =  1     ;
  pos_incr[13] =  1 + lx;
  pos_incr[14] =      lx;
  pos_incr[15] = -1 + lx;
  pos_incr[16] =  -1    ;
  pos_incr[17]  = -1 - lx +lx*ly;
  pos_incr[18]  =    - lx +lx*ly;
  pos_incr[19]  =  1 - lx +lx*ly;
  pos_incr[20]  =  1      +lx*ly;
  pos_incr[21]  =  1 + lx +lx*ly;
  pos_incr[22]  =      lx +lx*ly;
  pos_incr[23]  = -1 + lx +lx*ly;
  pos_incr[24]  =  -1     +lx*ly;
  pos_incr[25]  =          lx*ly;
}

/*
 * test if given position is valid, i.e. inside domain
 */
static int
_is_in_image_(int lx,
	      int ly,
	      int lz,
	      int pos)
{
  int x,y,tmp,z;
  x   = pos%lx;
  tmp = pos/lx;
  y   = tmp%ly;
  z   = tmp/ly;
  return ((x >= 0) && (x < lx) && (y >= 0) && (y < ly) && (z >= 0) && (z < lz));
}

static int
_is_max_ (float  *buf,
	  unsigned char *bufpos,
	  int     pos,
	  int     lx,
	  int     ly,
	  int     lz)
{
  int i;
  int flag = 0;
  int pos_neighbor;

  /*
   * Is given extrema isolated or on an edge ??
   * if so, flag > 25
   */
  for (i = 0; i < 26; i++) {
    pos_neighbor = pos + pos_incr[i];
    if (!_is_in_image_(lx, ly, lz, pos_neighbor) || (bufpos[pos_neighbor]==0))
      {
	flag ++;
      }
  }

  /* if isolated, we reject given extrema */
  if (flag > 20) {
    return 0;
  }

  for (i = 0; i < 26; i++) {
    pos_neighbor = pos + pos_incr[i];
    if (_is_in_image_(lx, ly, lz, pos_neighbor)
	&& (bufpos[pos_neighbor]==1)
	&& buf[pos] < buf[pos_neighbor]) {
      return 0;
    }
  }
  return 1;
}

static int
_is_min_ (float  *buf,
	  unsigned char *bufpos,
	  int     pos,
	  int     lx,
	  int     ly,
	  int     lz)
{
  int i;
  int flag = 0;
  int pos_neighbor;
  
  /*
   * Is given extrema isolated or on an edge ??
   * if so, flag > 25
   */
  for (i = 0; i < 26; i++) {
    pos_neighbor = pos + pos_incr[i];
    if (!_is_in_image_(lx, ly, lz, pos_neighbor) || (bufpos[pos_neighbor]==0))
      {
	flag ++;
      }
  }

  /* if isolated, we reject given extrema */
  if (flag > 20) {
    return 0;
  }

  for (i = 0; i < 26; i++) {
    pos_neighbor = pos + pos_incr[i];
    if (_is_in_image_(lx, ly, lz, pos_neighbor)
	&& (bufpos[pos_neighbor]==1)
	&& buf[pos] > buf[pos_neighbor]) {
      return 0;
    }
  }
  return 1;
}


/*------------------------------------------------------------------------
  _SaveExtImage_3D_small_
  ----------------------------------------------------------------------*/
static int
_SaveExtImage_3D_small_(Tcl_Interp * interp,
			ExtImage3Dsmall * extImage,
			char * extImageFilename,
			int    isVc)
{
  FILE            * fileOut;
  int               lx,ly,lz,i,size;
  real              scale;
  Extremum3Dsmall * extr;
  int               vcN=0;
  
  if (!(fileOut = fopen(extImageFilename, "w")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for writing.", NULL);
  lx    = extImage->lx;
  ly    = extImage->ly;
  lz    = extImage->lz;
  scale = extImage->scale;
  size  = extImage->extrNb;
  extr  = extImage->extr;

  if (isVc) {
    for (i = 0;i < size; i++) {
      if (extr[i].up || extr[i].down) {
	vcN++;
      }
    }
  }

  if (isVc) {
    fprintf(fileOut,
	    "Binary ExtImage3d %dx%dx%d %d %f"
	    "(%d byte reals, %d byte ints)\n",
	    lx, ly, lz, vcN, scale, (int) sizeof(real),
	    (int) sizeof(int));
    for (i = 0; i < size; i++) {
      if (extr[i].up || extr[i].down) {
	fwrite(&(extr[i].pos), sizeof(int),  1, fileOut);
	fwrite(&(extr[i].mod), sizeof(real), 1, fileOut);
      }
    }
  } else {
    fprintf(fileOut,
	    "Binary ExtImage3d %dx%dx%d %d %f"
	    "(%d byte reals, %d byte ints)\n",
	    lx, ly, lz, size, scale, (int) sizeof(real),
	    (int) sizeof(int));
    for (i = 0; i < size; i++) {
      fwrite(&(extr[i].pos), sizeof(int),  1, fileOut);
      fwrite(&(extr[i].mod), sizeof(real), 1, fileOut);
    }
  }
  
  fclose(fileOut);
  
  Tcl_AppendResult(interp, extImageFilename, NULL);
  return TCL_OK;
}

/************************************************************************
 *  Command name in xsmurf : esave3Dsmall
 *
 * ExtFileSaveCmd_3Dsmall_
 *
 * Sauvegarde d'une ExtImage3Dsmall sur disque
 *************************************************************************/
int
ExtFileSaveCmd_3Dsmall_(ClientData clientData,
			Tcl_Interp *interp,
			int argc,
			char **argv)      
{ 
  char * options[] = { "T[s]", 
		       "-vc", "",
		       NULL};
  
  char * help_msg =
  {
    ("  Save an ext image 3D small in a file.\n"
     "\n")
  };
  

  char            * extImageFilename = NULL;
  ExtImage3Dsmall * extImage;
  int               res;
  int               isVc;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &extImage, &extImageFilename)==TCL_ERROR)
    return TCL_ERROR;
  
  if (!extImageFilename)
    extImageFilename = argv[1];
  
  isVc = arg_present(1);

  res = _SaveExtImage_3D_small_(interp, extImage, extImageFilename, isVc);

  return TCL_OK;
}


/*------------------------------------------------------------------------
  _LoadExtImage_3D_small_
  ----------------------------------------------------------------------*/
static int
_LoadExtImage_3D_small_(Tcl_Interp * interp,
			char * extImageFilename,
			char * extImageName,
			int  isSwap)
{
  FILE     * fileIn;
  char       tempBuffer[100], saveFormat[100], type[100];
  int        lx, ly, lz, i, size, realSize, intSize;
  real      scale;
  ExtImage3Dsmall * extImage;
  Extremum3Dsmall * extr;

  if (!(fileIn = fopen(extImageFilename, "r")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for reading.", NULL);
  
  fgets(tempBuffer, 100, fileIn);
  sscanf(tempBuffer, "%s %s %dx%dx%d %d %f (%d byte reals, %d", 
	 saveFormat, type, &lx, &ly, &lz, &size, &scale, &realSize, &intSize);
    
  if (strcmp(type, "ExtImage3d")) {
    return GenErrorAppend(interp, "`", extImageFilename,
			  "' doesn't seem to be a 3D ExtImage.", NULL);
  }
  
  extImage = w3_ext_small_new(size, lx, ly, lz, scale);
  if (!extImage) {
    return GenErrorMemoryAlloc(interp);
  }
  
  extr = extImage->extr;
    
  if ((sizeof(real) != realSize) || (sizeof(int) != intSize)) {
    ExtIma3DsmallDelete_(extImage);
    return GenErrorAppend(interp, "real or Int size problem...", NULL);
  }
  
  for (i=0;i<size;i++) {
    fread(&(extr[i].pos), sizeof(int),   1, fileIn);
    fread(&(extr[i].mod), sizeof(float), 1, fileIn);
    extr[i].up = NULL;
    extr[i].down = NULL;
    if (isSwap) {
      BigLittleValues(&(extr[i].pos),1,sizeof(int));
      BigLittleValues(&(extr[i].mod),1,sizeof(float));
    }
    //extr[i].pos %= lx*ly;
  }
    
  fclose(fileIn);
  
  Ext3DsmallDicStore(extImageName, extImage);

  Tcl_AppendResult(interp, extImageFilename, NULL);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  Command name in xsmurf : eload3Dsmall

  ExtFileLoadCmd_3Dsmall_

  Lecture d'une extImage3Dsmall depuis le disque.
  ----------------------------------------------------------------------*/
int
ExtFileLoadCmd_3Dsmall_(ClientData clientData,
			Tcl_Interp *interp,
			int argc,
			char **argv)      
{ 
  /* Command line definition */
  char * options[] =
  {
    "s[s]",
    "-swap_bytes", "",
    NULL
  };

  char * help_msg =
  {
    ("  Load an ext image3Dsmall for computation.\n"
     "  (as opposed to eload3D_display).\n"
     "\n"
     "Parameters:\n"
     "  string   - Name of the file.\n"
     "  [string] - Name of the resulting ext images. Default is the name of\n"
     "             the file.\n"
     "\n"
     "Options:\n"
     "  -swap_bytes :\n"
     "\n"
     "Return value:\n"
     "  Name of the resulting ext image.")
  };

  /* Command's parameters */
  char     * extImageFilename = NULL;
  char     * extImageName     = NULL; 

  /* Options's presence */
  int isSwap = 0;

  /* Options's parameters */

  /* Other variables */
  int      result;

  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &extImageFilename, &extImageName)==TCL_ERROR)
    return TCL_ERROR;

  isSwap   = arg_present (1);

  /* Parameters validity and initialisation */

  /* Treatement */

  if (!extImageName) {
    extImageName = extImageFilename;
  }

  result = _LoadExtImage_3D_small_(interp, extImageFilename, extImageName, isSwap);

  return result;
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

/****************************************
 *  command name in xsmurf : efct3Dsmall
 ****************************************/
/* see apply_fct_to_e3D_TclCmd_, here we do
   not load the file into memory, because 
   files can be as LARGE as hundredth of
   MegaBytes for 512^3 data */
/* modified by pierre kestener : 06/07/2006 -> option -mask */

int
apply_fct_to_e3Dsmall_TclCmd_ (ClientData clientData,
			       Tcl_Interp *interp,
			       int        argc,
			       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ts[f]",
    "-vc", "",
    "-mask","J",    
    NULL
  };

  char * help_msg =
  {
    (" Applies a function to each modulus of an extimage3Dsmall\n"
     " and sums all the values. The function can have two parameters.\n"
     "\n"
     "Parameters:\n"
     "  Extimage3Dsmall  - name of the 3Dsmall ext image to treat.\n"
     "  string           - an expression designing the function\n"
     "                     (defunc syntax).\n"
     "  [real]           - facultative value of a the second parameter\n"
     "                     of the function.\n"
     "Options:\n"
     " -vc : apply function only to extremum that are on a vertical chain\n"
     "       TAKE CARE using this option after a \"vchain3Dsmall\" command\n"
     " -mask[J] : apply function onmy to extremum such mask value at the\n"
     "            corresponding position is non-zero\n"
     "\n"
     "command defined in interpreter/wt3d_cmds.c by fct apply_fct_to_e3Dsmall_TclCmd_ \n")
  };

  /* Command's parameters */
  char     *fct_expr;
  real     scd_value = 0;
  ExtImage3Dsmall * extImage;
  Extremum3Dsmall * extr;
  int        lx, ly, lz, size;
  real       scale;
  float      tmp_mod;
  int        tmp_pos;

  /* Options's presence */
  int isVc =0;
  int isMask=0;

  /* Options's parameters */
  int nb_vc;
  Image3D *mask;

  /* Other variables */
  /*double   (*fct)();*/
  void     *fct;
  double   result = 0.0;
  double   *values;
  int      i;
  int      nb_of_values;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extImage, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

  isVc   = arg_present (1);
  isMask = arg_present (2);
  if (isMask)
    if (arg_get (2, &mask) == TCL_ERROR)
      return TCL_ERROR;

  
 /* Parameters validity and initialisation */
  
  lx = extImage->lx;
  ly = extImage->ly;
  lz = extImage->lz;
  size = extImage->extrNb;
  scale = extImage->scale;
  extr = extImage->extr;

  /*fct = dfopen (fct_expr);*/
  fct = evaluator_create(fct_expr);
  if (!fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  
  /* Treatement */
  nb_vc = 0;
  if (isVc) {
    /* first loop for memory allocation */
    nb_vc = 0;
    for (i=0;i<extImage->extrNb;i++) {
      extr = &(extImage->extr[i]);
      if (!extr->up && !extr->down) {
	continue;
      }
      nb_vc++;
    }
    nb_of_values = nb_vc;
    values = (double *) malloc (nb_of_values*sizeof (double));
    nb_vc = 0;
    for (i=0;i<extImage->extrNb;i++) {
      extr = &(extImage->extr[i]);
      if (!extr->up && !extr->down) {
	continue;
      }
      tmp_mod = extr->mod;
      tmp_pos = extr->pos;
      if (isMask) {
	if ( mask->data[tmp_pos] > 0.0 || mask->data[tmp_pos] < 0.0 ) {
	  /*values[nb_vc] = fct (tmp_mod, scd_value);*/
	  values[nb_vc] = evaluator_evaluate_x_y(fct, tmp_mod, scd_value);
	} else {
	  values[nb_vc] = 0.0;
	}
      } else {
	/*values[nb_vc] = fct (tmp_mod, scd_value);*/
	values[nb_vc] = evaluator_evaluate_x_y(fct, tmp_mod, scd_value);
      }
      nb_vc++;
    }
    

  } else {
    nb_of_values = size;
    values = (double *) malloc (nb_of_values*sizeof (double));
    for (i = 0; i < nb_of_values; i++,extr++) {
      tmp_mod = extr->mod;
      tmp_pos = extr->pos;
      if (isMask) {
	if ( mask->data[tmp_pos] > 0.0 || mask->data[tmp_pos] < 0.0 ) {
	  /*values[i] = fct (tmp_mod, scd_value);*/
	  values[i] = evaluator_evaluate_x_y(fct, tmp_mod, scd_value);
	} else {
	  values[i] = 0.0;
	}
      } else {
	/*values[i] = fct (tmp_mod, scd_value);*/
	values[i] = evaluator_evaluate_x_y(fct, tmp_mod, scd_value);
      } 
    }
  }

  if (nb_of_values > 2)
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


/****************************************
 *  command name in xsmurf : efct3Dsmall2
 ****************************************/
/* see first efct3Dsmall
 * (see apply_fct_to_e3D_TclCmd_) 
 * this is just a copy of efct3Dsmall but handle longitudinal or transversal
 * WT modulus....(see what mean...)
 */
int
apply_fct_to_e3Dsmall2_TclCmd_ (ClientData clientData,
				Tcl_Interp *interp,
				int        argc,
				char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "TTs[f]",
    "-vc", "",
    "-special","s",
    NULL
  };

  char * help_msg =
  {
    (" Applies a function to each modulus of an extimage3Dsmall\n"
     " and sums all the values. The function can have two parameters.\n"
     "\n"
     "Parameters:\n"
     "  Extimage3Dsmall  - name of the 3Dsmall ext image to treat.\n"
     "  Extimage3Dsmall  - name of the second 3Dsmall ext image to treat.\n"
     "  string           - an expression designing the function\n"
     "                     (defunc syntax).\n"
     "  [real]           - facultative value of a the second parameter\n"
     "                     of the function.\n"
     "Options:\n"
     " -vc : apply function only to extremum that are on a vertical chain\n"
     "       TAKE CARE using this option after a \"vchain3Dsmall\" command\n"
     " -special [s] : apply another function to the second EXtimage3Dsmall\n "
     "\n"
     "command defined in interpreter/wt3d_cmds.c by fct apply_fct_to_e3Dsmall_TclCmd_ \n")
  };

  /* Command's parameters */
  char     *fct_expr;
  real     scd_value = 0;
  ExtImage3Dsmall * extImage;
  ExtImage3Dsmall * extImage2;
  Extremum3Dsmall * extr;
  Extremum3Dsmall * extr2;
  int        lx, ly, lz, size;
  real       scale;
  float      tmp_mod;
  float      tmp_mod2;

  /* Options's presence */
  int        isVc = 0;
  int        isSpecial = 0;

  /* Options's parameters */
  int        nb_vc;
  char      *fct_expr2;


  /* Other variables */
  /*double   (*fct)();
    double   (*fct2)();*/
  void    *fct, *fct2;
  double   result = 0.0;
  double  *values;
  int      i;
  int      nb_of_values;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extImage, &extImage2, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

  isVc   = arg_present (1);
  isSpecial = arg_present (2);
  if (isSpecial) {
    if (arg_get(2, &fct_expr2) == TCL_ERROR) {
      return TCL_ERROR;
    }
    /*fct2 = dfopen (fct_expr2);*/
    fct2 = evaluator_create(fct_expr2);
    if (!fct2) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr2, (char *) NULL);
      return TCL_ERROR;
    }
  }
  
 /* Parameters validity and initialisation */
  
  lx = extImage->lx;
  ly = extImage->ly;
  lz = extImage->lz;
  size = extImage->extrNb;
  scale = extImage->scale;
  extr = extImage->extr;

  if (lx != extImage2->lx || ly != extImage2->ly || lz != extImage2->lz) {
    Tcl_AppendResult (interp, "Error : the two extima3Dsmall's must have same size !!! \n",(char *) NULL);
    return TCL_ERROR;
  }
  if (extImage->extrNb != extImage2->extrNb) {
    Tcl_AppendResult (interp, "Error : the two extima3Dsmall's must have same extrum number !!! \n",(char *) NULL);
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
  
  /* Treatement */
  nb_vc = 0;
  if (isVc) {
    /* first loop for memory allocation */
    nb_vc = 0;
    for (i=0;i<extImage->extrNb;i++) {
      extr = &(extImage->extr[i]);
      if (!extr->up && !extr->down) {
	continue;
      }
      nb_vc++;
    }
    nb_of_values = nb_vc;
    values = (double *) malloc (nb_of_values*sizeof (double));
    nb_vc = 0;
    for (i=0;i<extImage->extrNb;i++) {
      extr = &(extImage->extr[i]);
      if (!extr->up && !extr->down) {
	continue;
      }
      extr2 = &(extImage2->extr[i]);
      tmp_mod = extr->mod;
      tmp_mod2 = extr2->mod;
      if (isSpecial) {
	/*values[nb_vc] = fct (tmp_mod, scd_value) * fct2 (tmp_mod2, scd_value);*/
	values[nb_vc] = evaluator_evaluate_x_y(fct, tmp_mod, scd_value) * evaluator_evaluate_x_y(fct2, tmp_mod2, scd_value);
      } else {
	/*values[nb_vc] = fct (tmp_mod2, scd_value);*/
	values[nb_vc] = evaluator_evaluate_x_y(fct, tmp_mod2, scd_value);
      }
      nb_vc++;
    }
    

  } else {
    extr = extImage->extr;
    extr2 = extImage2->extr;
    nb_of_values = size;
    values = (double *) malloc (nb_of_values*sizeof (double));
    
    for (i = 0; i < nb_of_values; i++,extr++,extr2++) {
      tmp_mod = extr->mod;
      tmp_mod2 = extr2->mod;      
      if (isSpecial) {
	/*values[i] = fct (tmp_mod, scd_value)* fct2 (tmp_mod2, scd_value);*/
	values[i] = evaluator_evaluate_x_y(fct, tmp_mod, scd_value)* evaluator_evaluate_x_y(fct2, tmp_mod2, scd_value);

      } else {
	/*values[i] = fct (tmp_mod2, scd_value);*/
	values[i] = evaluator_evaluate_x_y(fct, tmp_mod2, scd_value);
      }
    }
  }

  if (nb_of_values > 2)
    _QuickSort_ (values-1, nb_of_values);
  for (i = 0; i < nb_of_values; i++)
    result += values[i];


  /*dfclose (fct);*/
  evaluator_destroy(fct);
  if (isSpecial) {
    /*dfclose (fct2);*/
    evaluator_destroy(fct2);    
  }
  free (values);

  if (result == 0)
    sprintf (interp->result, "0");
  else
    sprintf (interp->result, "%.15e", result);
  return TCL_OK;
}



static void
_ComputeBox_3D_(int center,
		int boxsize,
		int lx,
		int ly,
		int lz,
		int * xMin,
		int * xMax,
		int * yMin,
		int * yMax,
		int * zMin,
		int * zMax) 
{
  int x,y,z,tmp;
  
  x   = center % lx;
  tmp = center / lx;
  y   = tmp    % ly;
  z   = tmp    / ly;
  
  *xMin = x-boxsize  ;  if(*xMin<0)  *xMin = 0 ;
  *xMax = x+boxsize+1;  if(*xMax>lx) *xMax = lx;
  *yMin = y-boxsize  ;  if(*yMin<0)  *yMin = 0 ;
  *yMax = y+boxsize+1;  if(*yMax>ly) *yMax = ly;
  *zMin = z-boxsize  ;  if(*zMin<0)  *zMin = 0 ;
  *zMax = z+boxsize+1;  if(*zMax>lz) *zMax = lz;
}


static int
ExtChnDistance_3D_Local(int lx,
			int ly,
			int p1,
			int p2)
{
  int tmp, dy, dz;
  int dx = ( p1 % lx ) - ( p2 % lx );

  tmp = ( p1 / lx ) - ( p2 / lx );
  dy = tmp % ly;
  dz = tmp / ly;


  return dx * dx + dy * dy + dz * dz;
}

/*************************************************
 *  command name in xsmurf : ei3Dpercent_identity
 *************************************************/
/* 
 * Created on may 23rd 2003.
 * usage : try to define a "percentage of identity" between
 * two given Extimage3dSmall.
 * Usefull for comparing "maxima surface" computed with
 * two different filtering technics such as fft and recursive filter...
 */
int
percent_of_identity_e3Dsmall_TclCmd_ (ClientData clientData,
				      Tcl_Interp *interp,
				      int        argc,
				      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "TT",
    "-mod", "f",
    "-boxsize", "d",
    NULL
  };

  char * help_msg =
  {
    (" Compute a percentage of identity betwee two given extimage3Dsmall.\n"
     " Basics : for each extremun of the first extimage3Dsmall, one finds\n"
     " in the second one, the nearest point, and compute a kind of distance.\n"
     " between those points. Final result is the root mean square value of\n"
     "such a distance.\n"
     "\n"
     "Parameters:\n"
     "  2 Extimage3Dsmall's  - name of the 3Dsmall ext images to treat.\n"
     "\n"
     "Options:\n"
     " -mod : do the same on the modulus of the nearest points\n"
     " -boxsize [d]\n"
     "\n"
     "command defined in interpreter/wt3d_cmds.c by routine \n"
     "percent_of_identity_e3Dsmall_TclCmd_ .\n")
  };

  /* Command's parameters */
  ExtImage3Dsmall * extIma1;
  ExtImage3Dsmall * extIma2;
  Extremum3Dsmall **array2;
  Extremum3Dsmall * ext1;
  Extremum3Dsmall * ext2;
  int        lx, ly, lz;
  int        x1, x, y1, y, z1, z;
  int        nbExt1;
  int        dmin;
  real       dist,zeres=0.0;
  int        reject=0;

  /* Options's presence */
  int isMod =0;
  int isBoxsize = 0;

  /* Options's parameters */
  real rescale = 1.0;
  real zeres_mod = 0.0;
  real zeres_sum = 0.0;
  real diffmod,diffmod2;
  int  boxsize = 5;

  /* Other variables */
  int      i,i1,i2;
  int      x_min, x_max, y_min, y_max, z_min, z_max;
  int      d, compteur;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extIma1, &extIma2) == TCL_ERROR)
    return TCL_ERROR;

  isMod   = arg_present (1);
  if (isMod) {
    if (arg_get(1, &rescale) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isBoxsize   = arg_present (2);
  if (isBoxsize) {
    if (arg_get(2, &boxsize) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
 /* Parameters validity and initialisation */
  
  lx = extIma1->lx;
  ly = extIma1->ly;
  lz = extIma1->lz;
  nbExt1 = extIma1->extrNb;

  array2 = (Extremum3Dsmall **) malloc (sizeof(Extremum3Dsmall *)*lx*ly*lz);
  for (i2 = 0; i2 < lx * ly * lz; i2++) {
    array2[i2] = NULL;
  }
  for (i2 = 0; i2 < extIma2 -> extrNb; i2++) {
    ext2 = &extIma2 -> extr[i2];
    array2[ext2 -> pos] = ext2;
  }

  /* Treatement */
  for (i1=0;i1<extIma1->extrNb;i1++) {
    ext1 = &(extIma1->extr[i1]);
    x1 = ext1->pos%lx;
    y1 = (ext1->pos/lx) % ly;
    z1 = (ext1->pos/lx) / ly;
    dmin = 2*lx;
    
    _ComputeBox_3D_ (ext1 -> pos, boxsize, lx, ly, lz,
		     &x_min, &x_max, &y_min, &y_max, &z_min, &z_max);

    compteur = 0;
    for (x = x_min; x < x_max ; x++) /* parcours des points de la boite */
      for ( y = y_min ; y < y_max; y++)
	for ( z = z_min ; z < z_max; z++)
	  {
	    d = x + lx*y + lx*ly*z;
	    ext2 = array2[d];
	    if (ext2) {
	      compteur++;
	      dist = ExtChnDistance_3D_Local (lx,ly, d, ext1 -> pos);
	      if(dist<dmin) {
		dmin = dist;   
		//diffmod = ((ext1->mod-rescale*ext2->mod)*(ext1->mod-rescale*ext2->mod)+(ext1->mod-1.0/rescale*ext2->mod)*(ext1->mod-1.0/rescale*ext2->mod))/ext1->mod/ext1->mod;
		//diffmod = ((ext1->mod-rescale*ext2->mod)*(ext1->mod-rescale*ext2->mod))/ext1->mod/ext1->mod;
		//diffmod = (ext1->mod-rescale*ext2->mod)*(ext1->mod-rescale*ext2->mod);
		diffmod = (rescale*ext1->mod-(1-rescale)*ext2->mod)*(rescale*ext1->mod-(1-rescale)*ext2->mod);
		diffmod2 = (rescale*rescale*ext1->mod*ext1->mod+(1-rescale)*(1-rescale)*ext2->mod*ext2->mod);
	      }
	    }
	  }
    if (compteur>0) {
      zeres += (float) dmin;
      zeres_mod += diffmod;
      zeres_sum += diffmod2;
    } else {
      reject ++;
    }
  }
  
  //zeres /= (nbExt1-reject);
  //zeres /= reject;
  //zeres_mod /= (nbExt1-reject);
  zeres_mod /= zeres_sum;
  
  free(array2);

  if (isMod)
    //sprintf (interp->result, "mean square error (module) %.15e\n", zeres_mod);
    sprintf (interp->result, "%.15e\n", zeres_mod);
  else 
    sprintf (interp->result, "mean square error (distance) %.15e\nmean square error (module) %.15e\npercentage reject %.15e", zeres, zeres_mod,(float) (reject) / (float) (nbExt1));
    

  return TCL_OK;
}

/******************************************
 * Command name in xsmurf : vchain3Dsmall
 ******************************************/
int
pt_to_pt_vert_chain_3Dsmall_TclCmd_ (ClientData clientData,
				     Tcl_Interp *interp,
				     int        argc,
				     char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "TT[df]",
    "-first", "",
    "-simil", "f",
    NULL
  };

  char * help_msg =
  {
    (" Chain maxima of two 3Dsmall ext images. The maxima must be \n"
     " isolated (i.e. no surflets).\n"
     "\n"
     "Parameters :\n"
     "  2 3D small ext images - ext images to treat.\n"
     "  [integer]             - size of the search box.\n"
     "Options :\n"
     "  -first :\n"
     "  -simil [f]\n")
  };

  /* Command's parameters */
  Extremum3Dsmall **up_array;
  ExtImage3Dsmall *ext_im_do;
  ExtImage3Dsmall *ext_im_up;
  int              box_size = 7;
  int              lx,ly,lz;
  int              nb_vc;
  float            arg_simil = 0.8;

  /* Options's presence */
  int is_first;
  int is_simil;

  /* Options's parameters */

  /* Other variables */

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_im_do, &ext_im_up, &box_size) == TCL_ERROR)
    return TCL_ERROR;

  is_first = arg_present (1);
  is_simil = arg_present (2);
  if(is_simil) {
    if (arg_get(2, &arg_simil) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */

  /* Treatement */

  lx = ext_im_do->lx;
  ly = ext_im_do->ly;
  lz = ext_im_do->lz;
  up_array = (Extremum3Dsmall **) malloc (sizeof(Extremum3Dsmall *)*lx*ly*lz);

  nb_vc = pt_to_pt_vert_chain_3Dsmall (up_array,
				       ext_im_do,
				       ext_im_up,
				       lx,
				       ly,
				       lz,
				       box_size,
				       arg_simil,
				       is_first);

  sprintf(interp->result,"nb_vc %d", nb_vc);
  free (up_array);

  return TCL_OK;
}

/**********************************************
 * command name in xsmurf : e3Dsmall_getextr
 **********************************************/
int
e3Dsmall_getextr_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "T",
    NULL
  };
  
  char * help_msg =
  {
    (" Get the modulus extrema of an ext image 3D small.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage3Dsmall - ext image to treat.\n")
  };

  /* Command's parameters */
  ExtImage3Dsmall *extImage;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  real min;
  real max;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extImage) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  ExtIma3DsmallMinMax(extImage, &min, &max);

  sprintf(interp->result,"%g %g", min, max);

  return TCL_OK;
}

/*
 * A FAIRE A FAIRE A FAIRE !!!!!!!!!!!!!
 */

/*******************************************************
 * Command name in xsmurf : ei3Dsmallsave_4_vtk 
 *******************************************************/
/* created  on october 10th 2002*/
int 
ei_3Dsmall_save_4_vtk_TclCmd_ (ClientData clientData,
			       Tcl_Interp *interp,
			       int        argc,
			       char       **argv)
{     
  /* Command line definition */
  char * options[] =
  {
    "Ts",
    "-color", "",
    NULL
  };

  char * help_msg =
  {
    (" Save some \"surflets\" of a 3Dsmall extima in a file\n" 
     "using VTK file format (STRUCTURED_POINTS).\n"
     "See : http://public.kitware.com/FileFormats.pdf\n"
     "\n"
     "Parameters :\n"
     "  Extima3Dsmall  - The ext image 3Dsmall to treat.\n"
     "  string         - Name of output file.\n"
     "                   example: \"toto.vtk\"\n"
     "\n"
     "Options:\n"
     "  -color   : add scalar field to put some color\n"
     "             default scalar is WT module\n"
     "\n")
  };

  /* Command's parameters */
  ExtImage3Dsmall *extima;
  char     *filename = NULL;
  FILE     *fileOut;
  int       i;
  Extremum3Dsmall **array;
  int       lx,ly,lz;

  int       isColor;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extima, &filename) == TCL_ERROR)
    return TCL_ERROR;
  
  isColor = arg_present(1);

  lx = extima->lx;
  ly = extima->ly;
  lz = extima->lz;

  array = (Extremum3Dsmall **) malloc (sizeof(Extremum3Dsmall *)*lx*ly*lz);

  for (i=0;i<extima->lx*extima->ly*extima->lz;i++)
    array[i] = NULL;
  for (i=0;i<extima->extrNb;i++)
    array[extima->extr[i].pos] = &(extima->extr[i]);

  /* fill header */
  fileOut = fopen(filename, "w");
  fprintf(fileOut, "# vtk DataFile Version 1.0 \n");
  fprintf(fileOut, "data generated by xsmurf.\n");
  fprintf(fileOut, "ASCII\n");
  //fprintf(fileOut, "BINARY\n");
  fprintf(fileOut, "\n");
  fprintf(fileOut,"DATASET STRUCTURED_POINTS\n");
  fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx, ly, lz);
  fprintf(fileOut,"ORIGIN %f %f %f\n", 0.0, 0.0, 0.0);
  fprintf(fileOut,"SPACING %f %f %f\n", 1.0, 1.0, 1.0);
  fprintf(fileOut," \n");
  fprintf(fileOut,"POINT_DATA %d\n", lx*ly*lz);
  fprintf(fileOut,"SCALARS scalars unsigned_char\n");
  fprintf(fileOut,"LOOKUP_TABLE default\n");

  for (i=0;i<extima->lx*extima->ly*extima->lz;i++) {
    if (array[i])
      fprintf(fileOut,"%d ",128);
    else
      fprintf(fileOut,"%d ",0);  
  }


  if (isColor) {
    real min, max;
    ExtIma3DsmallMinMax(extima, &min, &max);
    
    //fprintf(fileOut,"SCALARS colors unsigned_char\n");
    fprintf(fileOut,"SCALARS colors float\n");
    fprintf(fileOut,"LOOKUP_TABLE default\n");
    for (i=0;i<extima->lx*extima->ly*extima->lz;i++) {
      if (array[i])
	fprintf(fileOut,"%f ", (float) ((array[i]->mod-min)*12.0/(max-min)) );
      else
	fprintf(fileOut,"%f ",0.0);  
    }
  }

  fclose(fileOut);
  
  return TCL_OK;
  
}

/*******************************************************
 * Command name in xsmurf : buffer3D_2_vtk 
 *******************************************************/
/* created  on november 21st 2002*/
int 
buffer3D_2_vtk_structured_points_TclCmd_ (ClientData clientData,
					  Tcl_Interp *interp,
					  int        argc,
					  char       **argv)
{     
  /* Command line definition */
  char * options[] =
  {
    "sds",
    "-function", "s",
    "-half_cube", "",
    "-border", "d",
    "-reduce", "d",
    "-shrink", "d",
    "-size", "dd",
    NULL
  };

  char * help_msg =
  {
    (" Convert 3D buffer (float, col major format) on disk in a file\n" 
     "using VTK file format (STRUCTURED_POINTS).\n"
     "So you can easily visualize isosurfaces with VTK\n"
     "See : http://public.kitware.com/FileFormats.pdf\n"
     "\n"
     "Parameters :\n"
     "  string         - Name of input file.\n"
     "  integer        - size of buffer (example: 64 means that\n"
     "                   your buffer contains 64^3 values\n"
     "  string         - Name of output file.\n"
     "                   example: \"data.vtk\"\n"
     "\n"
     "Options:\n"
     "   -function [s]: fuction to rescale data\n"
     "   -half_cube []: only saves one-half of the cube so that\n"
     "                  inner slice whose cartesian equation is\n"
     "                  \"x+y=1\" becomes visible. --> USELESS !!\n"
     "   -border [d] : resize buffer before saving it. Removes border of\n"
     "                 some thickness (given parameter).\n"
     "   -reduce [d] : save only one over several points...\n"
     "   -shrink [d] : same as reduce but in a coarse-grain way...\n"
     "   -size [dd]  : specify the ly and lz sizes (assuming col-major format)\n"
     "\n")
  };

  /* Command's parameters */
  char     *filenameIn = NULL;
  char     *filenameOut = NULL;
  FILE     *fileIn;
  FILE     *fileOut;
  float    *buffer;
  int       i, bufferSize, eltRead;
  int       lx,ly,lz;
  float     min, max;

  /* Option */
  int isFunction;
  int isHalfcube;
  int isBorder;
  int isReduce;
  int isShrink;
  int isSize;

  /* Other variables */
  char     *fct_expr;
  /*double  (*fct)();*/
  void     *fct;
  int       border;
  int       reduce_param=1;
  float    *bufferShrink;
  int       xx,yy,zz;
  int       ii, bufferShrinkSize;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &filenameIn, &lx, &filenameOut) == TCL_ERROR)
    return TCL_ERROR;

  
  isFunction  = arg_present (1);
  if (isFunction) {
    if (arg_get(1, &fct_expr) == TCL_ERROR) {
      return TCL_ERROR;
    }
    /*fct = dfopen (fct_expr);*/
    fct = evaluator_create(fct_expr);
    if (!fct) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  }
  
  isHalfcube  = arg_present (2);
  
  isBorder  = arg_present (3);
  if (isBorder) {
    if (arg_get(3, &border) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isReduce = arg_present (4);
  if (isReduce) {
    if (arg_get(4, &reduce_param) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isShrink = arg_present (5);
  if (isShrink) {
    if (arg_get(5, &reduce_param) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* set default values for ly and lz sizes */
  ly = lx;
  lz = lx;
  
  isSize = arg_present (6);
  if (isSize) {
    if (arg_get(6, &ly, &lz) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  bufferSize = lx*ly*lz;
  bufferShrinkSize = (lx/reduce_param+1)*(ly/reduce_param+1)*(lz/reduce_param+1);
  //bufferShrinkSize = (lx/reduce_param+0)*(ly/reduce_param+0)*(lz/reduce_param+0);
  /*printf("bufferShrinkSize : %d\n",bufferShrinkSize);
    printf("%d %d %d\n", (lx/reduce_param), (ly/reduce_param), (lz/reduce_param));*/

  buffer = (float *) malloc (bufferSize*sizeof(float));

  if (isShrink) {
    bufferShrink = (float *) malloc (bufferShrinkSize*sizeof(float));
    if (bufferShrink) {
      for (ii = 0; ii < bufferShrinkSize; ii++)
	bufferShrink[ii] = 0.0; 
    }
  }
  
  /* read buffer */
  fileIn = fopen(filenameIn, "r");
  eltRead = fread(buffer, sizeof(float), bufferSize, fileIn);
  if (eltRead != bufferSize) {
    printf("problem in reading input file : not enough data !!\n");
    printf("% data read instead of %d\n", eltRead, bufferSize);
  }  

  /* find min max */
  if (isFunction) {
    for (i=0;i<bufferSize;i++) {
      /*buffer[i] = fct(buffer[i],0.0);*/
      buffer[i] = evaluator_evaluate_x_y(fct, buffer[i],0.0);
    }
  }
  
  min = max = buffer[0];
  for (i=0;i<bufferSize;i++) {
    if (buffer[i]>max) {
      max = buffer[i];
    } else {
      if (buffer[i]<min)
	min = buffer[i];
    }
  }
  

  /* fill header */
  fileOut = fopen(filenameOut, "w");
  fprintf(fileOut, "# vtk DataFile Version 1.0 \n");
  fprintf(fileOut, "data generated by xsmurf.\n");
  fprintf(fileOut, "ASCII\n");
  //fprintf(fileOut, "BINARY\n");
  fprintf(fileOut, "\n");
  fprintf(fileOut,"DATASET STRUCTURED_POINTS\n");
  if (isBorder)
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx-2*border, ly-2*border, lz-2*border);
  else if (isShrink)
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx/reduce_param+1, ly/reduce_param+1, lz/reduce_param+1);
  else
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx/reduce_param, ly/reduce_param, lz/reduce_param);
  fprintf(fileOut,"ORIGIN %f %f %f\n", 0.0, 0.0, 0.0);
  fprintf(fileOut,"SPACING %f %f %f\n", 1.0, 1.0, 1.0);
  fprintf(fileOut," \n");
  if (isHalfcube)
    fprintf(fileOut,"POINT_DATA %d\n", ((lx+1)*ly)/2*lz);
  else if (isBorder)
    fprintf(fileOut,"POINT_DATA %d\n", (lx-2*border)*(ly-2*border)*(lz-2*border));
  else if (isShrink)
    fprintf(fileOut,"POINT_DATA %d\n", bufferShrinkSize);
  else
    fprintf(fileOut,"POINT_DATA %d\n", lx/reduce_param*ly/reduce_param*lz/reduce_param);
  fprintf(fileOut,"SCALARS scalars float\n");
  //fprintf(fileOut,"SCALARS scalars unsigned_char\n");
  fprintf(fileOut,"LOOKUP_TABLE default\n");


  if (isHalfcube) {
    float tmp;
    //unsigned char tmp;
    for (i=0;i<((lx+1)*ly)/2*lz;i++) {
      //tmp = 255 * ( ((buffer[i])-min)/(max-min)*0.8 + 0.2 );
      tmp = 255 * ( ((buffer[i])-min)/(max-min) );
      //fwrite(&tmp, sizeof(float), 1 ,fileOut);
      fprintf(fileOut, "%f ", tmp);
    }
  } else if (isBorder) {
    float tmp;
    int posx, posy, posz;
    //unsigned char tmp;
    for (i=0;i<lx*ly*lz;i++) {
      // i vaut posx + posy*lx + posz*lx*ly
      posx = i % lx;
      posy = (i / lx) % ly;
      posz = (i / lx) / ly;
      if (posx >= border && posx < lx-border && posy >= border && posy < ly-border && posz >= border && posz < lz-border) {
	tmp = 255 * ( ((buffer[i])-min)/(max-min) );
	fprintf(fileOut, "%f ", tmp);
      }
    }
  } else if (isReduce) {
    float tmp;
    int x, y, z;

    //unsigned char tmp;
    for (i=0;i<lx*ly*lz;i++) {
      /* col-major format */
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      if ((x%reduce_param)==0 && (y%reduce_param)==0 && (z%reduce_param)==0) {
	//tmp = 255 * ( ((buffer[i])-min)/(max-min)*0.8 + 0.2 );
	tmp = 255 * ( ((buffer[i])-min)/(max-min) );
	//fwrite(&tmp, sizeof(float), 1 ,fileOut);
	fprintf(fileOut, "%f ", tmp);
      }
    }
  } else if (isShrink) {
    float tmp;
    int x, y, z;

    //unsigned char tmp;
    for (i=0;i<bufferSize;i++) {
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      xx = x / reduce_param;
      yy = y / reduce_param;
      zz = z / reduce_param;
      ii = xx + yy*(lx/reduce_param+1) + zz*(lx/reduce_param+1)*(ly/reduce_param+1);
      tmp = 255 * ( ((buffer[i])-min)/(max-min) );
      //bufferShrink[ii] += tmp;
      if (ii >= bufferShrinkSize) {
	printf("we got a problem with ii : %d %d %d %d %d %d %d\n",ii, x, xx, y, yy, z, zz);
	//printf("ii xx yy zz : %d %d %d %d\n",ii, xx, yy, zz);
      } else {
	bufferShrink[ii] += tmp;
      }
      //fprintf(fileOut, "%f ", tmp);
    }
    for (ii=0;ii<bufferShrinkSize;ii++) {
      fprintf(fileOut, "%f ", bufferShrink[ii]);
    }
  } else {
    float tmp;
    int x, y, z;

    //unsigned char tmp;
    for (i=0;i<lx*ly*lz;i++) {
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      if ((x%reduce_param)==0 && (y%reduce_param)==0 && (z%reduce_param)==0) {
        //tmp = 255 * ( ((buffer[i])-min)/(max-min)*0.8 + 0.2 );
        tmp = 255 * ( ((buffer[i])-min)/(max-min) );
        //fwrite(&tmp, sizeof(float), 1 ,fileOut);
        fprintf(fileOut, "%f ", tmp);
      }
    }
  }

  free(buffer);
  if (isShrink)
    free(bufferShrink);
  
  if (isFunction) {
    /*dfclose(fct);*/
    evaluator_destroy(fct);
  }

  fclose(fileIn);
  fclose(fileOut);
  
  return TCL_OK;
  
}

/*******************************************************
 * Command name in xsmurf : i3D_2_vtk 
 *******************************************************/
/* created on june 1st 2007 */
/* modified on March 23rd 2008: add option binary */
int 
i3D_2_vtk_structured_points_TclCmd_ (ClientData clientData,
				     Tcl_Interp *interp,
				     int        argc,
				     char       **argv)
{     
  /* Command line definition */
  char * options[] =
  {
    "Js",
    "-function", "s",
    "-half_cube", "",
    "-border", "d",
    "-reduce", "d",
    "-shrink", "d",
    "-binary", "",
    NULL
  };

  char * help_msg =
  {
    (" Save an image3D 3D (float, column major format) into a file\n" 
     "using VTK file format (STRUCTURED_POINTS) with ASCII mode.\n"
     "So you can easily visualize isosurfaces with VTK\n"
     "See : http://public.kitware.com/FileFormats.pdf\n"
     "\n"
     "Parameters :\n"
     "  image3D        - Name of input.\n"
     "  string         - Name of output file.\n"
     "                   example: \"data.vtk\"\n"
     "\n"
     "Options:\n"
     "   -function [s]: fuction to rescale data\n"
     "   -half_cube []: only saves one-half of the cube so that\n"
     "                  inner slice whose cartesian equation is\n"
     "                  \"x+y=1\" becomes visible. --> USELESS !!\n"
     "   -border [d] : resize buffer before saving it. Removes border of\n"
     "                 some thickness (given parameter).\n"
     "   -reduce [d] : save only one over several points...\n"
     "   -shrink [d] : same as reduce but in a coarse-grain way...\n"
     "   -binary []  : use the BINARY mode of the VTK file format\n"
     "                 take care that data must be written using BIG ENDIAN\n"
     "                 format, whatever the platform is running this software.\n"
     "\n")
  };

  /* Command's parameters */
  Image3D  *image;
  char     *filenameOut = NULL;
  FILE     *fileOut;
  float    *buffer;
  int       i, bufferSize, eltRead;
  int       lx, ly, lz;
  float     min, max;

  /* Option */
  int isFunction;
  int isHalfcube;
  int isBorder;
  int isReduce;
  int isShrink;
  int isBinary;

  /* Other variables */
  char     *fct_expr;
  /*double  (*fct)();*/
  void     *fct;
  int       border;
  int       reduce_param=1;
  float    *bufferShrink;
  int       xx,yy,zz;
  int       bufferShrinkSize;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &filenameOut) == TCL_ERROR)
    return TCL_ERROR;

  if (image->type != PHYSICAL) {
    sprintf (interp->result, "Can only convert 3D image with type PHYSICAL.");
    return TCL_ERROR;
  }
  
  /* parse command line options */
  isFunction  = arg_present (1);
  if (isFunction) {
    if (arg_get(1, &fct_expr) == TCL_ERROR) {
      return TCL_ERROR;
    }
    /*fct = dfopen (fct_expr);*/
    fct = evaluator_create(fct_expr);
    if (!fct) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  }
  
  isHalfcube  = arg_present (2);
  
  isBorder  = arg_present (3);
  if (isBorder) {
    if (arg_get(3, &border) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isReduce = arg_present (4);
  if (isReduce) {
    if (arg_get(4, &reduce_param) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isShrink = arg_present (5);
  if (isShrink) {
    if (arg_get(5, &reduce_param) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isBinary  = arg_present (6);

  /* set values for lx, ly and lz sizes */
  /* remember that xsmurf uses column major format */
  lx = image->lx;
  ly = image->ly;
  lz = image->lz;
  
  bufferSize = lx*ly*lz;
  bufferShrinkSize = (lx/reduce_param+1)*(ly/reduce_param+1)*(lz/reduce_param+1);
  //bufferShrinkSize = (lx/reduce_param+0)*(ly/reduce_param+0)*(lz/reduce_param+0);
  /*printf("bufferShrinkSize : %d\n",bufferShrinkSize);
    printf("%d %d %d\n", (lx/reduce_param), (ly/reduce_param), (lz/reduce_param));*/

  /*buffer = (float *) malloc (bufferSize*sizeof(float));*/
  buffer = image->data;

  if (isShrink) {
    bufferShrink = (float *) malloc (bufferShrinkSize*sizeof(float));
    if (bufferShrink) {
      int ii;
      for (ii = 0; ii < bufferShrinkSize; ii++)
	bufferShrink[ii] = 0.0; 
    }
  }
  
  /* read buffer */
  /*fileIn = fopen(filenameIn, "r");
  eltRead = fread(buffer, sizeof(float), bufferSize, fileIn);
  if (eltRead != bufferSize) {
    printf("problem in reading input file : not enough data !!\n");
    printf("% data read instead of %d\n", eltRead, bufferSize);
    }*/ 

  /* find min max */
  if (isFunction) {
    for (i=0;i<bufferSize;i++) {
      /*buffer[i] = fct(buffer[i],0.0);*/
      buffer[i] = evaluator_evaluate_x_y(fct,buffer[i],0.0);
    }
  }
  
  min = max = buffer[0];
  for (i=0;i<bufferSize;i++) {
    if (buffer[i]>max) {
      max = buffer[i];
    } else {
      if (buffer[i]<min)
	min = buffer[i];
    }
  }
  

  /* fill header */
  fileOut = fopen(filenameOut, "w");
  fprintf(fileOut, "# vtk DataFile Version 1.0 \n");
  fprintf(fileOut, "data generated by xsmurf.\n");
  if (isBinary)
    fprintf(fileOut, "BINARY\n");   
  else
    fprintf(fileOut, "ASCII\n");
  fprintf(fileOut, "\n");
  fprintf(fileOut,"DATASET STRUCTURED_POINTS\n");
  if (isBorder)
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx-2*border, ly-2*border, lz-2*border);
  else if (isShrink)
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx/reduce_param+1, ly/reduce_param+1, lz/reduce_param+1);
  else
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx/reduce_param, ly/reduce_param, lz/reduce_param);
  fprintf(fileOut,"ORIGIN %f %f %f\n", 0.0, 0.0, 0.0);
  fprintf(fileOut,"SPACING %f %f %f\n", 1.0, 1.0, 1.0);
  fprintf(fileOut," \n");
  if (isHalfcube)
    fprintf(fileOut,"POINT_DATA %d\n", ((lx+1)*ly)/2*lz);
  else if (isBorder)
    fprintf(fileOut,"POINT_DATA %d\n", (lx-2*border)*(ly-2*border)*(lz-2*border));
  else if (isShrink)
    fprintf(fileOut,"POINT_DATA %d\n", bufferShrinkSize);
  else
    fprintf(fileOut,"POINT_DATA %d\n", lx/reduce_param*ly/reduce_param*lz/reduce_param);
  fprintf(fileOut,"SCALARS scalars float\n");
  //fprintf(fileOut,"SCALARS scalars unsigned_char\n");
  fprintf(fileOut,"LOOKUP_TABLE default\n");


  if (isHalfcube) {
    float tmp;
    //unsigned char tmp;
    for (i=0;i<((lx+1)*ly)/2*lz;i++) {
      //tmp = 255 * ( ((buffer[i])-min)/(max-min)*0.8 + 0.2 );
      tmp = 255 * ( ((buffer[i])-min)/(max-min) );
      if (!isBinary) {
	fprintf(fileOut, "%f ", tmp);
      } else {
	if (!isBigEndian()) // must byes-swap tmp before writing
	  BigLittleValues(&tmp, 1, sizeof(float));
	fwrite(&tmp, sizeof(float), 1 ,fileOut);
      }
    }
  } else if (isBorder) {
    float tmp;
    int posx, posy, posz;
    //unsigned char tmp;
    for (i=0;i<lx*ly*lz;i++) {
      // i vaut posx + posy*lx + posz*lx*ly
      /* this is column-major format */
      posx = i % lx;
      posy = (i / lx) % ly;
      posz = (i / lx) / ly;
      if (posx >= border && posx < lx-border && posy >= border && posy < ly-border && posz >= border && posz < lz-border) {
	tmp = 255 * ( ((buffer[i])-min)/(max-min) );
	if (!isBinary) {
	  fprintf(fileOut, "%f ", tmp);
	} else {
	  if (!isBigEndian()) // must byes-swap tmp before writing
	    BigLittleValues(&tmp, 1, sizeof(float));
	  fwrite(&tmp, sizeof(float), 1 ,fileOut);
	}
      }
    }
  } else if (isReduce) {
    float tmp;
    int x, y, z;

    //unsigned char tmp;
    for (i=0;i<lx*ly*lz;i++) {
      /* column-major format */
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      if ((x%reduce_param)==0 && (y%reduce_param)==0 && (z%reduce_param)==0) {
	//tmp = 255 * ( ((buffer[i])-min)/(max-min)*0.8 + 0.2 );
	tmp = 255 * ( ((buffer[i])-min)/(max-min) );
	//fwrite(&tmp, sizeof(float), 1 ,fileOut);
	if (!isBinary) {
	  fprintf(fileOut, "%f ", tmp);
	} else {
	  if (!isBigEndian()) // must byes-swap tmp before writing
	    BigLittleValues(&tmp, 1, sizeof(float));
	  fwrite(&tmp, sizeof(float), 1 ,fileOut);
	}
      }
    }
  } else if (isShrink) {
    float tmp;
    int x, y, z, ii;

    //unsigned char tmp;
    for (i=0;i<bufferSize;i++) {
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      xx = x / reduce_param;
      yy = y / reduce_param;
      zz = z / reduce_param;
      ii = xx + yy*(lx/reduce_param+1) + zz*(lx/reduce_param+1)*(ly/reduce_param+1);
      tmp = 255 * ( ((buffer[i])-min)/(max-min) );
      //bufferShrink[ii] += tmp;
      if (ii >= bufferShrinkSize) {
	printf("we got a problem with ii : %d %d %d %d %d %d %d\n",ii, x, xx, y, yy, z, zz);
	//printf("ii xx yy zz : %d %d %d %d\n",ii, xx, yy, zz);
      } else {
	bufferShrink[ii] += tmp;
      }
      //fprintf(fileOut, "%f ", tmp);
    }
    if (!isBinary) {
      for (ii=0;ii<bufferShrinkSize;ii++) {
	fprintf(fileOut, "%f ", bufferShrink[ii]);
      }  
    } else {
      if (!isBigEndian()) // must byte-swap tmp before writing
	BigLittleValues(bufferShrink, bufferShrinkSize, sizeof(float));
      fwrite(bufferShrink, sizeof(float),  bufferShrinkSize,fileOut);
    }
  } else {
    float tmp;
    int x, y, z;

    //unsigned char tmp;
    for (i=0;i<lx*ly*lz;i++) {
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      if ((x%reduce_param)==0 && (y%reduce_param)==0 && (z%reduce_param)==0) {
	//tmp = 255 * ( ((buffer[i])-min)/(max-min)*0.8 + 0.2 );
        tmp = 255 * ( ((buffer[i])-min)/(max-min) );
	if (!isBinary) {
	  fprintf(fileOut, "%f ", tmp);
	} else {
	  if (!isBigEndian()) // must byte-swap tmp before writing
	    BigLittleValues(&tmp, 1, sizeof(float));
	  fwrite(&tmp, sizeof(float), 1 ,fileOut);
	}
      }
    }
  }

  /*free(buffer);*/
  if (isShrink)
    free(bufferShrink);

  if (isFunction) {
    /*dfclose(fct);*/
    evaluator_destroy(fct);
  }
  
  /*fclose(fileIn);*/
  fclose(fileOut);
  
  return TCL_OK;
  
}

/*******************************************************
 * Command name in xsmurf : vector3D_2_vtk 
 *******************************************************/
/* created  on november 21st 2002*/
int 
vector3D_2_vtk_structured_points_TclCmd_ (ClientData clientData,
					  Tcl_Interp *interp,
					  int        argc,
					  char       **argv)
{     
  /* Command line definition */
  char * options[] =
  {
    "sssds",
    "-function", "s",
    "-border", "d",
    "-special", "df",
    "-reduce", "d",
    NULL
  };

  char * help_msg =
  {
    (" Convert three 3D buffers (float, same size) on disk in a file\n" 
     "using VTK file format (STRUCTURED_POINTS) with field VECTORS.\n"
     "See : http://public.kitware.com/FileFormats.pdf\n"
     "\n"
     "Parameters :\n"
     "  3 strings      - Names of the three input files.\n"
     "  integer        - size of buffer (example: 64 means that\n"
     "                   your buffer contains 64^3 values\n"
     "  string         - Name of output file.\n"
     "                   example: \"toto.vtk\"\n"
     "\n"
     "Options:\n"
     "   -function [s]: function to rescale data\n"
     "   -border   [d]: cut border of thickness the parameter\n"
     "   -special  [df]: set to special_value (second parameter) the field\n"
     "                   \"scalars\" when point is outside a sphere of\n"
     "                    some radius (given as first parameter).\n"
     "   -reduce [d] : save only one over several points...\n"
     "\n")
  };

  /* Command's parameters */
  char     *filenameIn1 = NULL;
  char     *filenameIn2 = NULL;
  char     *filenameIn3 = NULL;
  char     *filenameOut = NULL;
  FILE     *fileIn1,*fileIn2, *fileIn3, *fileOut;
  float    *buffer1, *buffer2, *buffer3;
  int       i;
  int       lx,ly,lz;

  /* Option */
  int isFunction;
  int isBorder;
  int isSpecial;
  int isReduce;

  /* Other variables */
  char     *fct_expr;
  /*double   (*fct)();*/
  void     *fct;
  int      thickness;
  int      nb_points;
  int      radius;
  float    special_value;
  int      reduce_param=1;
  int      x,y,z;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &filenameIn1, &filenameIn2, &filenameIn3, &lx, &filenameOut) == TCL_ERROR)
    return TCL_ERROR;
  
  
  isFunction  = arg_present (1);
  if (isFunction) {
    if (arg_get(1, &fct_expr) == TCL_ERROR) {
      return TCL_ERROR;
    }
    /*fct = dfopen (fct_expr);*/
    fct = evaluator_create(fct_expr);
    if (!fct) {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  }
  isBorder = arg_present (2);
  if (isBorder) {
    if (arg_get(2, &thickness) == TCL_ERROR) {
      return TCL_ERROR;
    }
    if (thickness<0 || thickness>lx/2-1) {
      Tcl_AppendResult (interp, "Error in parameter of option \"-border\" \n",(char *) NULL);
      return TCL_ERROR;
    }
  }

  isSpecial = arg_present (3);
  if (isSpecial) {
    if (arg_get(3, &radius, &special_value) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isReduce = arg_present (4);
  if (isReduce) {
    if (arg_get(4, &reduce_param) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  

  ly = lx;
  lz = lx;

  buffer1 = (float *) malloc (lx*ly*lz*sizeof(float));
  buffer2 = (float *) malloc (lx*ly*lz*sizeof(float));
  buffer3 = (float *) malloc (lx*ly*lz*sizeof(float));

  /* read buffers */
  fileIn1 = fopen(filenameIn1, "r");
  fread(buffer1, sizeof(float), lx*ly*lz, fileIn1);
  fileIn2 = fopen(filenameIn2, "r");
  fread(buffer2, sizeof(float), lx*ly*lz, fileIn2);
  fileIn3 = fopen(filenameIn3, "r");
  fread(buffer3, sizeof(float), lx*ly*lz, fileIn3);
  
  /* find min max */
  if (isFunction) {
    float rescale1, rescale2;
    for (i=0;i<lx*ly*lz;i++) {
      rescale1 = sqrt(buffer1[i]*buffer1[i] \
		      +buffer2[i]*buffer2[i] \
		      +buffer3[i]*buffer3[i]);
      /*rescale2 = fct(rescale1,0.0);*/
      rescale2 = evaluator_evaluate_x_y(fct,rescale1,0.0);
      buffer1[i] *= rescale2/rescale1;
      buffer2[i] *= rescale2/rescale1;
      buffer3[i] *= rescale2/rescale1;
      //buffer1[i] = fct(buffer1[i],0.0);
      //buffer2[i] = fct(buffer2[i],0.0);
      //buffer3[i] = fct(buffer3[i],0.0);
    }
  }
  
  /*
  min = max = buffer[0];
  for (i=0;i<lx*ly*lz;i++) {
    if (buffer[i]>max) {
      max = buffer[i];
    } else {
      if (buffer[i]<min)
	min = buffer[i];
    }
  }
  */
  
  if (isBorder) {
    nb_points = 0;
    for (i=0;i<lx*ly*lz;i++) {
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      if (x<thickness || y<thickness || z<thickness || x>lx-1-thickness || y>ly-1-thickness || z>lz-1-thickness) {
      } else {
	nb_points++;
      }
	
    }
  }


  /* fill header */
  fileOut = fopen(filenameOut, "w");
  fprintf(fileOut, "# vtk DataFile Version 1.0 \n");
  fprintf(fileOut, "data generated by xsmurf.\n");
  fprintf(fileOut, "ASCII\n");
  //fprintf(fileOut, "BINARY\n");
  fprintf(fileOut, "\n");
  fprintf(fileOut,"DATASET STRUCTURED_POINTS\n");
  if (isBorder) {
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx-2*thickness, ly-2*thickness, lz-2*thickness);
  } else {
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx/reduce_param, ly/reduce_param, lz/reduce_param);
  }
  fprintf(fileOut,"ORIGIN %f %f %f\n", 0.0, 0.0, 0.0);
  fprintf(fileOut,"SPACING %f %f %f\n", 1.0, 1.0, 1.0);
  fprintf(fileOut," \n");

  // first scalar field : gradient modulus
  if (isBorder) {
    fprintf(fileOut,"POINT_DATA %d\n", nb_points);
  } else {
    fprintf(fileOut,"POINT_DATA %d\n", (int) lx*ly*lz/reduce_param/reduce_param/reduce_param);
  }
  fprintf(fileOut,"SCALARS scalars float\n");
  fprintf(fileOut,"LOOKUP_TABLE default\n");


  if (isBorder && !isSpecial) {
    float tmp;
    
    for (i=0;i<lx*ly*lz;i++) {
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      if (x< thickness || y< thickness || z< thickness || x>lx-1-thickness || y>ly-1-thickness || z>lz-1-thickness) {
      } else {
	tmp = sqrt(buffer1[i]*buffer1[i]+buffer2[i]*buffer2[i]+buffer3[i]*buffer3[i]);
	fprintf(fileOut, "%f ", tmp);
      }
    }
  } else if (isBorder && isSpecial) {
    float tmp;
    int   rad2;
    for (i=0;i<lx*ly*lz;i++) {
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      rad2  = (x-lx/2)*(x-lx/2) + (y-ly/2)*(y-ly/2) + (z-lz/2)*(z-lz/2);
      if (x< thickness || y< thickness || z< thickness || x>lx-1-thickness || y>ly-1-thickness || z>lz-1-thickness) {
      } else {
	tmp = sqrt(buffer1[i]*buffer1[i]+buffer2[i]*buffer2[i]+
		   buffer3[i]*buffer3[i]);
	if (rad2>radius*radius) {
	  fprintf(fileOut, "%f ", special_value);
	} else {
	  fprintf(fileOut, "%f ", tmp);
	}
      }
    }
  } else {
    float tmp;
    for (i=0;i<lx*ly*lz;i++) {
      //tmp = 255 * ( ((buffer[i])-min)/(max-min) );
      tmp = sqrt(buffer1[i]*buffer1[i]+buffer2[i]*buffer2[i]+buffer3[i]*buffer3[i]);
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      if ((x%reduce_param)==0 && (y%reduce_param)==0 && (z%reduce_param)==0) 
	fprintf(fileOut, "%f ", tmp);
    }
  }
  
  // next vector field !!!
  fprintf(fileOut,"VECTORS vectors float\n"); 
  
 if (isBorder) {
    
    for (i=0;i<lx*ly*lz;i++) {
      x = i % lx;
      y = (i / lx) % ly;
      z = (i / lx) / ly;
      if (x< thickness || y< thickness || z< thickness || x>lx-1-thickness || y>ly-1-thickness || z>lz-1-thickness) {
      } else {
	fprintf(fileOut, "%f %f %f\n", buffer1[i], buffer2[i], buffer3[i]);
      }
    }
 } else {
   
   for (i=0;i<lx*ly*lz;i++) {
     x = i % lx;
     y = (i / lx) % ly;
     z = (i / lx) / ly;
     if ((x%reduce_param)==0 && (y%reduce_param)==0 && (z%reduce_param)==0) 
       fprintf(fileOut, "%f %f %f\n", buffer1[i], buffer2[i], buffer3[i]);
   }
 }


  free(buffer1);
  free(buffer2);
  free(buffer3);

  if (isFunction) {
    /*dfclose(fct);*/
    evaluator_destroy(fct);
  }

  fclose(fileIn1);
  fclose(fileIn2);
  fclose(fileIn3);
  fclose(fileOut);
  
  return TCL_OK;
  
}

/*******************************************************
 * Command name in xsmurf : ei3Dsmallsave_4_vtkpolydata 
 *******************************************************/
/* created  on october 31th 2002*/
int 
ei_3Dsmall_save_4_vtk_polydata_TclCmd_ (ClientData clientData,
					Tcl_Interp *interp,
					int        argc,
					char       **argv)
{     
  /* Command line definition */
  char * options[] =
  {
    "Ts",
    "-minmax", "ff",
    "-vc", "TTTT",
    NULL
  };

  char * help_msg =
  {
    (" Save some \"surflets\" of a 3Dsmall extima in a file\n" 
     "using VTK file format (POLYDATA).\n"
     "See : http://public.kitware.com/FileFormats.pdf\n"
     "\n"
     "Parameters :\n"
     "  Extima3Dsmall  - The ext image 3Dsmall to treat.\n"
     "  string         - Name of output file.\n"
     "                   example: \"toto.vtk\"\n"
     "\n"
     "Options:\n"
     "  -minmax [float float]  : specify scalar range to rescale\n"
     "                           WT module\n"
     "  -vc [TTTT]   : add VECTOR field (WT vector) for every WTMMM\n"
     "                very nice (satisfait ou rembourse !)\n"
     "                the first is the extima that has been chained...\n"
     "                the three Extima3Dsmall must have been created\n"
     "                by make_edge_compo (Save_local_maxima_compo)...\n"
     //"                Each of them must have the same number of extrema\n"
     "                they represent the three components of gradient\n"
     "                These extima3Dsmall must be of type eeext (See what\n"
     "                i mean...)\n"
     "\n")
  };

  /* Command's parameters */
  ExtImage3Dsmall *extima, *chained_extima, *Tx, *Ty, *Tz;
  char     *filename = NULL;
  FILE     *fileOut;
  int       i;
  //Extremum3Dsmall **array;
  int       lx,ly,lz;
  int       pos, posx, posy, posz, tmp;
  

  int       isMinMax, isVc;
  real      zemin, zemax;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extima, &filename) == TCL_ERROR)
    return TCL_ERROR;
  
  isMinMax = arg_present(1);
  if(isMinMax) {
    if (arg_get(1, &zemin, &zemax) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isVc = arg_present(2);
  if(isVc) {
    if (arg_get(2, &chained_extima, &Tx, &Ty, &Tz) == TCL_ERROR) {
      return TCL_ERROR;
    }
    /*if (Tx->extrNb != extima->extrNb || Ty->extrNb != extima->extrNb || Tz->extrNb != extima->extrNb)
      return GenErrorAppend(interp, "option -vc used with wrong extima3Dsmall...", NULL);*/
  }
  
  lx = extima->lx;
  ly = extima->ly;
  lz = extima->lz;

  //array = (Extremum3Dsmall **) malloc (sizeof(Extremum3Dsmall *)*lx*ly*lz);

  //for (i=0;i<extima->lx*extima->ly*extima->lz;i++)
  //  array[i] = NULL;
  //for (i=0;i<extima->extrNb;i++)
  //  array[extima->extr[i].pos] = &(extima->extr[i]);

  /* fill header */
  fileOut = fopen(filename, "w");
  fprintf(fileOut, "# vtk DataFile Version 1.0 \n");
  fprintf(fileOut, "data generated by xsmurf.\n");
  fprintf(fileOut, "ASCII\n");
  //fprintf(fileOut, "BINARY\n");
  fprintf(fileOut, "\n");
  fprintf(fileOut, "DATASET POLYDATA\n");
  fprintf(fileOut, "POINTS %d integer\n", extima->extrNb);
  // writing point coordinates
  for (i=0;i<extima->extrNb;i++) {
    pos = extima->extr[i].pos;
    posx = pos % lx;
    tmp  = pos / lx;
    posy = tmp % ly;
    posz = tmp / ly;
    fprintf(fileOut,"%d %d %d\n", posx, posy, posz);
  }

  /* writing colors ... */
  fprintf(fileOut,"\n");
  fprintf(fileOut,"POINT_DATA %d\n", extima->extrNb);
  fprintf(fileOut,"SCALARS scalars float\n");
  fprintf(fileOut,"LOOKUP_TABLE default\n");

  {
    real min, max;
    if (isMinMax) {
      min = zemin;
      max = zemax;
    } else {
      ExtIma3DsmallMinMax(extima, &min, &max);
    }

    for (i=0;i<extima->extrNb;i++) 
      //fprintf(fileOut,"%f ", (float) ((extima->extr[i].mod-min)*256.0/(max-min)) );
      fprintf(fileOut,"%f ", extima->extr[i].mod );
  }
  
  /* write WT vector cartesian components for vc points (WTMMM) */
  if (isVc) {
    Extremum3Dsmall *ext, *cext;
    int j;
    fprintf(fileOut,"\n");
    //fprintf(fileOut,"POINT_DATA %d\n",extima->extrNb);
    fprintf(fileOut,"VECTORS vectors float\n");
    for (i=0;i<extima->extrNb;i++) {
      ext = &(extima->extr[i]);
      for (j=0; j<chained_extima->extrNb;j++) {
	cext = &(chained_extima->extr[j]);
	if (cext->pos == ext->pos)
	  break;
      }	
      if (j==chained_extima->extrNb) {
	fprintf(fileOut,"%f %f %f\n", 0.0, 0.0, 0.0);
      } else {
	//cext = &(chained_extima->extr[j]);
	if ((cext->up && cext->up->pos) || (cext->down && cext->down->pos)) {
	  fprintf(fileOut,"%f %f %f\n", Tx->extr[j].mod, Ty->extr[j].mod, Tz->extr[j].mod);
	} else {
	  fprintf(fileOut,"%f %f %f\n", 0.0, 0.0, 0.0);
	}
      }
    }
  }

  fclose(fileOut);
  
  return TCL_OK;
  
}

/****************************************************
 *  command name in xsmurf : ei3Dsmallsave_skel4vtk
 ****************************************************/
/* created on november 6th 2002*/
int
ei3Dsmall_save_skel_vtk_TclCmd_ (ClientData clientData,
				 Tcl_Interp *interp,
				 int        argc,
				 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ts",
    "-between", "dd",
    "-xz", "",
    "-yz", "",
    "-localslope", "",
    "-xyz", "",
    "-log", "",
    NULL
  };

  char * help_msg =
  {
    (" Save some vertical lines of the skeleton of 3D data in a file\n" 
     "using VTK file format.\n"
     "See : http://public.kitware.com/FileFormats.pdf\n"
     "See : http://www.vtk.org/pdf/file-formats.pdf\n"
     "As these lines live in a four-dimensionnal space (x,y,z,scale)\n"
     "one has to project them in a three-dimensionnal space; for\n"
     "example (x,y,scale) which is the default or (x,z,scale)\n"
     " or (y,z,scale).\n"
     "\n"
     "Parameters :\n"
     "  Extima  - The ext 3D small image to treat.\n"
     "  string  - Name of output file.\n"
     "            example: \"toto.vtk\"\n"
     "Be careful that the given extima must be\n"
     "chained (result of multiple \"vchain3Dsmall\" operations). \n"
     "\n"
     "Options:\n"
     "  -between [integer integer]: see eivcgerbe3Dsmall command\n"
     "  -xz\n"
     "  -yz\n"
     "  -xyz\n"
     "  -localslope : use ratio of modulus of two consecutive extrema\n"
     "                on a vertical chain to color the vertical chain.\n"
     "  -log :\n"
     "\n")
  };
  
  /* Command's parameters */
  ExtImage3Dsmall *extima;
  int total_length, total_length_between;
  int length, thelength;
  int pos, x, y, z;
  int nb_vc, nb_vc_between;
  int nb_points, nb_points_between;
  int count, count_between;
  
  /* Options's presence */
  int isBetween;
  int isXz;
  int isYz;
  int isLocalSlope;
  int isXyz;
  int isLog;

  /* Options's parameters */
  int scale_up = 0;
  int scale_low = 0;
  real     *lslope_array, *lslope_array_between;
  int      i,i1;

  /* Other variables */
  Extremum3Dsmall *ext;
  char     *filename = NULL;
  FILE     *fileOut;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extima, &filename) == TCL_ERROR)
    return TCL_ERROR;

  isBetween = arg_present(1);
  if(isBetween) {
    if (arg_get(1, &scale_low, &scale_up) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isXz  = arg_present(2);
  isYz  = arg_present(3);
  isLocalSlope = arg_present(4);
  isXyz = arg_present(5);
  isLog = arg_present(6);

  /* fill header */
  fileOut = fopen(filename, "w");
  fprintf(fileOut, "# vtk DataFile Version 1.0 \n");
  fprintf(fileOut, "data generated by xsmurf.\n");
  fprintf(fileOut, "ASCII\n");
  fprintf(fileOut, "\n");
  fprintf(fileOut, "DATASET POLYDATA\n");
  

  total_length         = 0;
  total_length_between = 0;

  nb_vc     = 0;
  nb_points = 0;
  nb_vc_between     = 0;
  nb_points_between = 0;

  /* first compute total_length and total_length_between */
  for (i1=0;i1<extima->extrNb;i1++) {
    ext = &(extima->extr[i1]);
    if (!ext->up && !ext->down) {
      continue;
    }
    nb_vc++;
    /* Follow the chain down to up */
    length = 1;
    
    while (ext->up) {
      ext = ext->up;
      length++;
    }
    
    total_length += length;
    
    if (length >= scale_low && length < scale_up) {
      nb_vc_between++;
      total_length_between += length;
    }
  } /* end of the for loop  */
  
  /* now we fill POINTS field */
  if (isBetween) {
    fprintf(fileOut, "POINTS %d integer\n", total_length_between);
  } else {
    fprintf(fileOut, "POINTS %d integer\n", total_length);
  }
 
  /* allocation for signal used in localslope option */
  if (isBetween) {
    lslope_array_between = (float *) malloc(total_length_between*sizeof(float));
  } else {
    lslope_array         = (float *) malloc(total_length*sizeof(float));
  }

  /* ******************************
   * we fill lslope_array_between
   ********************************/
  i = 0;
  for (i1=0;i1<extima->extrNb;i1++) {
    ext = &(extima->extr[i1]);
    if (!ext->up && !ext->down) {
      continue;
    }
    /* Follow the chain down to up */
    thelength = 0;
    while (ext->up) {
      ext = ext->up;
      thelength++;
    }
    
    while (ext->down) {
      ext = ext->down;
    }
    
    if (isBetween && thelength >= scale_low && thelength < scale_up) {
      while (ext->up) {
	lslope_array_between[i]=log(ext->up->mod / ext->mod)/log(2);
	ext = ext->up;
	i++;
      }
      lslope_array_between[i] = lslope_array_between[i-1];
    } else if (isBetween==0) {
      while (ext->up) {
	lslope_array[i]=log(ext->up->mod / ext->mod)/log(2);
	ext = ext->up;
	i++;
      }
      lslope_array[i] = lslope_array[i-1];
    } 
  }


 
  /* *********************************/
  /* now we enter POINTS coordinates */
  /* *********************************/
  
  if (isBetween) {
    for (i1=0;i1<extima->extrNb;i1++) {
      ext = &(extima->extr[i1]);
      if (!ext->up && !ext->down) {
	continue;
      }
      /* go down */
      while (ext->down) {
	ext = ext->down;
      }
      length = 1;
      while (ext->up) {
	ext = ext->up;
	length++;
      }
      
      if (length >= scale_low && length < scale_up) {
	while (ext->down) {
	  ext = ext->down;
	}
	length = 1;
	pos = ext->pos;
	x = pos % extima->lx;
	y = (pos / extima->lx) % extima->ly;
	z = (pos / extima->lx) / extima->ly;
	if (isXz) {
	  fprintf(fileOut, "%d %d %d\n", x, (int) 2*(extima->lx*(length-1)/50), z);
	  while (ext->up) {
	    ext = ext->up;
	    length++;
	    pos = ext->pos;
	    x = pos % extima->lx;
	    y = (pos / extima->lx) % extima->ly;
	    z = (pos / extima->lx) / extima->ly;
	    fprintf(fileOut, "%d %d %d\n", x, (int) 2*(extima->lx*(length-1)/50), z);
	  }
	} else if (isYz) {
	  fprintf(fileOut, "%d %d %d\n", y, (int) 2*(extima->lx*(length-1)/50), z);
	  while (ext->up) {
	    ext = ext->up;
	    length++;
	    pos = ext->pos;
	    x = pos % extima->lx;
	    y = (pos / extima->lx) % extima->ly;
	    z = (pos / extima->lx) / extima->ly;
	    fprintf(fileOut, "%d %d %d\n", y, (int) 2*(extima->lx*(length-1)/50), z);
	  }
	} else {
	  fprintf(fileOut, "%d %d %d\n", x, (int) 2*(extima->lx*(length-1)/50), y);
	  while (ext->up) {
	    ext = ext->up;
	    length++;
	    pos = ext->pos;
	    x = pos % extima->lx;
	    y = (pos / extima->lx) % extima->ly;
	    z = (pos / extima->lx) / extima->ly;
	    fprintf(fileOut, "%d %d %d\n", x, (int) 2*(extima->lx*(length-1)/50), y);
	  }
	}
      }
    }
  } else {
    for (i1=0;i1<extima->extrNb;i1++) {
      ext = &(extima->extr[i1]);
      if (!ext->up && !ext->down) {
	continue;
      }
      /* Follow the chain down to up */
      length = 1;
      pos = ext->pos;
      x = pos % extima->lx;
      y = (pos / extima->lx) % extima->ly;
      z = (pos / extima->lx) / extima->ly;
      if (isXz) {
	fprintf(fileOut, "%d %d %d\n", x, (int) 2*(extima->lx*(length-1)/50), z);
	while (ext->up) {
	  ext = ext->up;
	  length++;
	  pos = ext->pos;
	  x = pos % extima->lx;
	  y = (pos / extima->lx) % extima->ly;
	  z = (pos / extima->lx) / extima->ly;
	  fprintf(fileOut, "%d %d %d\n", x, (int) 2*(extima->lx*(length-1)/50), z);
	}
      } else if (isYz) {
	fprintf(fileOut, "%d %d %d\n", y, (int) 2*(extima->lx*(length-1)/50), z);
	while (ext->up) {
	  ext = ext->up;
	  length++;
	  pos = ext->pos;
	  x = pos % extima->lx;
	  y = (pos / extima->lx) % extima->ly;
	  z = (pos / extima->lx) / extima->ly;
	  fprintf(fileOut, "%d %d %d\n", y, (int) 2*(extima->lx*(length-1)/50), z);
	}
      } else if (isXyz) {
	fprintf(fileOut, "%d %d %d\n", y, x, z);
	while (ext->up) {
	  ext = ext->up;
	  length++;
	  pos = ext->pos;
	  x = pos % extima->lx;
	  y = (pos / extima->lx) % extima->ly;
	  z = (pos / extima->lx) / extima->ly;
	  fprintf(fileOut, "%d %d %d\n", y, x, z);
	}
      } else {
	fprintf(fileOut, "%d %d %d\n", x, (int) 2*(extima->lx*(length-1)/50), y);
	while (ext->up) {
	  ext = ext->up;
	  length++;
	  pos = ext->pos;
	  x = pos % extima->lx;
	  y = (pos / extima->lx) % extima->ly;
	  z = (pos / extima->lx) / extima->ly;
	  fprintf(fileOut, "%d %d %d\n", x, (int) 2*(extima->lx*(length-1)/50), y);
	}
      }
    } /* end of the for loop  */
  }
  
  /* now we fill LINES field */
  
  if (isBetween) {
    fprintf(fileOut, "LINES %d %d\n",nb_vc_between, nb_vc_between+total_length_between);
  } else {
    fprintf(fileOut, "LINES %d %d\n",nb_vc, nb_vc+total_length);
  }
  
  count         = 0;
  count_between = 0;
  
  if (isBetween) {
    for (i1=0;i1<extima->extrNb;i1++) {
      ext = &(extima->extr[i1]);
      if (!ext->up && !ext->down) {
	continue;
      }
      
      /* Follow the chain down to up */
      length = 1;
      while (ext->up) {
	ext = ext->up;
	length++;
      }
      while (ext->down) {
	ext = ext->down;
      }
      
      if (length >= scale_low && length < scale_up) {
	fprintf(fileOut, "%d ",length);
	fprintf(fileOut, "%d ",count_between++);
	while (ext->up) {
	  ext = ext->up;
	  fprintf(fileOut, "%d ",count_between++);
	}
	fprintf(fileOut,"\n");
      }
    }
  } else {
    for (i1=0;i1<extima->extrNb;i1++) {
      ext = &(extima->extr[i1]);
      if (!ext->up && !ext->down) {
	continue;
      }
      
      /* Follow the chain down to up */
      length = 1;
      while (ext->up) {
	ext = ext->up;
	length++;
      }
      while (ext->down) {
	ext = ext->down;
      }
      fprintf(fileOut, "%d ",length);
      fprintf(fileOut, "%d ",count++);
      while (ext->up) {
	ext = ext->up;
	fprintf(fileOut, "%d ",count++);
      }
      fprintf(fileOut,"\n");
    } /* end of the for loop  */
  }
 
  /***************
   * now color
   ***************/
  i=0;
  if (isBetween) {
    fprintf(fileOut,"POINT_DATA %d\n", total_length_between);
    fprintf(fileOut,"SCALARS my_scalars float\n");
    fprintf(fileOut,"LOOKUP_TABLE default\n");
    for (i1=0;i1<extima->extrNb;i1++) {
      ext = &(extima->extr[i1]);
      if (!ext->up && !ext->down) {
	continue;
      }
      /* go down */
      while (ext->down) {
	ext = ext->down;
      }
      length = 1;
      while (ext->up) {
	ext = ext->up;
	length++;
      }
      
      if (length >= scale_low && length < scale_up) {
	while (ext->down) {
	  ext = ext->down;
	}

	if (isLocalSlope) {
	  fprintf(fileOut, "%f\n", lslope_array_between[i++]);
	  while (ext->up) {
	    ext = ext->up;
	    fprintf(fileOut, "%f\n", lslope_array_between[i++]);
	  }
	} else if (isLog) {
	  fprintf(fileOut, "%f\n", log(ext->mod));
	  while (ext->up) {
	    ext = ext->up;
	    fprintf(fileOut, "%f\n", log(ext->mod));
	  }
	} else {
	  fprintf(fileOut, "%f\n", ext->mod);
	  while (ext->up) {
	    ext = ext->up;
	    fprintf(fileOut, "%f\n", ext->mod);
	  }
	}
      }
    }
  } else {
    fprintf(fileOut,"POINT_DATA %d\n", total_length);
    fprintf(fileOut,"SCALARS my_scalars float\n");
    fprintf(fileOut,"LOOKUP_TABLE default\n");
    for (i1=0;i1<extima->extrNb;i1++) {
      ext = &(extima->extr[i1]);
      if (!ext->up && !ext->down) {
	continue;
      }
      /* Follow the chain down to up */
      if (isLocalSlope) {
	fprintf(fileOut, "%f\n", lslope_array[i++]);
	while (ext->up) {
	  ext = ext->up;
	  fprintf(fileOut, "%f\n", lslope_array[i++]);
	}
      } else if (isLog) {
	fprintf(fileOut, "%f\n", log(ext->mod));
	while (ext->up) {
	  ext = ext->up;
	  fprintf(fileOut, "%f\n", log(ext->mod));
	}
      } else {
	fprintf(fileOut, "%f\n", ext->mod);  
	while (ext->up) {
	  ext = ext->up;
	  fprintf(fileOut, "%f\n", ext->mod);
	}
      }
    } /* end of the for loop  */
  }
  
  fclose(fileOut);
  
  return TCL_OK;
} 
 
/***************************************
 * Command name in xsmurf : ecut3Dsmall
 ***************************************/
int
e_cut_3Dsmall_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Tsdddddd",
    NULL
  };

  char * help_msg =
  {
    (" Cut an ext image 3D small.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage3Dsmall   - ext image to cut.\n"
     "  string            - name of the result.\n"
     "  6 integers        - coordinates of a cube that correspond to\n" 
     "                      the new ext image 3Dsmall.\n"
     "\n"
     "Return value :\n"
     "  None.")
  };

  /* Command's parameters */
  ExtImage3Dsmall *ext_image;
  Extremum3Dsmall *extr;  
  char            *name;
  int              x1, x2, y1, y2, z1, z2;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  ExtImage3Dsmall *result;
  int              nExt = 0;
  int      lx, ly, lz;
  int      i, j;
  int      x, y, z;
  int      tmp;
  int      pos;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &name, &x1, &y1, &z1, &x2, &y2, &z2) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (x1 > x2) {
    tmp = x1;
    x1 = x2;
    x2 = tmp;
  }
  if (y1 > y2) {
    tmp = y1;
    y1 = y2;
    y2 = tmp;
  }
  if (z1 > z2) {
    tmp = z1;
    z1 = z2;
    z2 = tmp;
  }
  if ((x2 < 0) || (y2 < 0) || (z2 < 0) || (x1 >= ext_image->lx) 
      || (y1 >= ext_image->ly) || (z1>= ext_image->lz)) {
    sprintf (interp->result, "bad cut domain");
    return TCL_ERROR;
  }
  if (x1 < 0) {
    x1 = 0;
  }
  if (y1 < 0) {
    y1 = 0;
  }
  if (z1 < 0) {
    z1 = 0;
  }
  if (x2 >= ext_image->lx) {
    x2 = ext_image->lx-1;
  }
  if (y2 >= ext_image->ly) {
    y2 = ext_image->ly-1;
  }
  if (z2 >= ext_image->lz) {
    z2 = ext_image->lz-1;
  }

  /* Treatement */

  lx = x2 - x1;
  ly = y2 - y1;
  lz = z2 - z1;

  /* Count the number of extrema to put in the new image. */

  for (i = 0; i < ext_image->extrNb; i++) {
    pos = ext_image->extr[i].pos;
    x   = pos % ext_image->lx;
    tmp = pos / ext_image->lx;
    y   = tmp % ext_image->ly;
    z   = tmp / ext_image->ly;
    if ((x > x1) && (x < x2) && (y > y1) && (y < y2) && (z > z1) && (z < z2)) {
      nExt++;
    }
  }

  result = w3_ext_small_new(nExt, lx, ly, lz, ext_image->scale);
  if (!result) {
    return GenErrorMemoryAlloc(interp);
  }
  extr = result->extr;

  /* Put the extrema in the new image. */
  j = 0;
  for (i = 0; i < ext_image->extrNb; i++) {
    pos = ext_image->extr[i].pos;
    x   = pos % ext_image->lx;
    tmp = pos / ext_image->lx;
    y   = tmp % ext_image->ly;
    z   = tmp / ext_image->ly;
    if ((x > x1) && (x < x2) && (y > y1) && (y < y2) && (z > z1) && (z < z2)) {
      extr[j].pos = (x-x1) + (y-y1)*lx + (z-z1)*lx*ly;
      extr[j].mod = ext_image->extr[i].mod;
      extr[j].up = NULL;
      extr[j].down = NULL;
      j++;
    }
  }

  Ext3DsmallDicStore(name, result);
  Tcl_AppendResult(interp, name, NULL);

  return TCL_OK;
}


/****************************************** 
 * Command name in xsmurf : vc2s3Dsmall
 ******************************************/
int
vert_chain_3Dsmall_to_s_TclCmd_ (ClientData clientData,
				 Tcl_Interp *interp,
				 int        argc,
				 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Tsddd",
    "-xy", "s",
    NULL
  };
  
  char * help_msg =
  {
    (" Convert a vertical chain (from 3Dsmall data) to a signal \n"
     " (with the modulus of the gradient).\n"
     "\n"
     "Parameters :\n"
     "  ext image 3Dsmall  - ext image to treat.\n"
     "  string             - name of the resulting signal.\n"
     "  3 integers         - coordinate of a point.\n"
     "                       The vertical chain to treat is\n"
     "                       the nearest this point.\n"
     "\n"
     "Options :\n"
     "  -xy : Create a signal with the position (x, y).\n"
     "     string - name of the resulting signal.\n"
     "\n"
     "Return value :\n"
     "  The position (x, y, z) of the lower extremity (lower scale) of the "
     "vertical chain.\n"
     "\n"
     "NB : command defined in interpreter/wt3d_cmds.c by\n"
     "vert_chain_3Dsmall_to_s_TclCmd_\n")
  };

  /* Command's parameters */
  ExtImage3Dsmall *ext_image;
  char            *name;
  int             x, y, z;

  /* Options's presence */
  int isXy;

  /* Options's parameters */
  char *xyName;

  /* Other variables */
  Signal   *result;
  Signal   *xyResult;
  int      size = 1;
  int      i;
  int      pos;
  Extremum3Dsmall *ext_ptr;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &name, &x, &y, &z) == TCL_ERROR)
    return TCL_ERROR;

  isXy = arg_present(1);
  if (isXy) {
    if (arg_get(1, &xyName) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */

  /* Treatement */
  pos = x + y*ext_image->lx + z*ext_image->lx*ext_image->ly;

  ext_ptr = w3_closest_vc_ext3Dsmall (ext_image, pos);
  if (!ext_ptr) {
    sprintf (interp->result, "No extremum near (%d,%d,%d)", x, y, z);
    return TCL_ERROR;
  }
  while (ext_ptr->up) {
    ext_ptr = ext_ptr->up;
    //ext_image = ext_image->up;
  }
  while (ext_ptr->down) {
    ext_ptr = ext_ptr ->down;
    //ext_image = ext_image->down;
    size ++;
  }
  if (size == 1) {
    sprintf (interp->result, "The extremum near (%d,%d,%d) is not on a vertical chain.", x, y, z);
    return TCL_ERROR;
  }

  sprintf(interp->result, "%d %d %d",
	  ext_ptr->pos%ext_image->lx,
	  (ext_ptr->pos/ext_image->lx)%ext_image->ly,
	  (ext_ptr->pos/ext_image->lx)/ext_image->ly);
  
  result = sig_new (REALY, 0, size - 1);
  if (isXy) {
    xyResult = sig_new (REALXY, 0, size - 1);
  }

  for (i = 0; i < size; i++) {
    result->dataY[i] = ext_ptr->mod;
    if (isXy) {
      xyResult->dataX[i] =  ext_ptr->pos%ext_image->lx;
      xyResult->dataY[i] = (ext_ptr->pos/ext_image->lx)%ext_image->ly;
    }
    //ext_image = ext_image->up;
    ext_ptr = ext_ptr->up;
  }
  
  store_signal_in_dictionary (name, result);

  if (isXy) {
    store_signal_in_dictionary (xyName, xyResult);
  }

  return TCL_OK;
}

/*********************************************
 * Command name in xsmurf : eivcgerbe3Dsmall
 *********************************************/
int
e_vc_gerbe_3Dsmall_TclCmd_ (ClientData clientData,
			    Tcl_Interp *interp,
			    int        argc,
			    char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ts",
    "-between",  "sdd",
    "-tag",      "ss",
    NULL
  };

  char * help_msg =
  {
    ("  Create a REALXY-signal (log(a), log(mod))\n"
     "  with all scale-space points that are on a vertical\n"
     "  chain (from 3D data). See Also command \"vc2s\".\n"
     "use this command AFTER having chained the 3Dsmall extimages!!\n"
     "IMPORTANT : we assume that the 3Dsmall images are made with\n"
     "isolated extrema!!!\n"
     "\n"
     "Parameters:\n"
     "  ext image 3Dsmall - Ext image to treat.\n"
     "                      this extimage must belong to a skeleton\n"
     "  string            - name of the resulting signal\n"
     "\n"
     "Options:\n"
     "   -between  [sdd] : same as above but keep chains whose top is\n"
     "                     between scale_low and scale_up.\n"
     "   tag [ss]\n"
     "\n"
     "Return value:\n"
     "  None.\n")
  };

  /* Command's parameters */
  ExtImage3Dsmall *extima;
  int total_length, total_length_between;
  int total_length_tag, total_length_untag;
  int length, length2;
  int count;
  int count_tag,count_untag;
  int nb_vc;
  int scale_up = 0;
  int scale_low = 0;
  real lmod = 0;
  real umod = 0;
  int i;

  /* Options's presence */
  int isBetween;
  int isTag;

  /* Options's parameters */

  /* Other variables */
  Extremum3Dsmall *ext;
  char     *gerbe_name, *between_name;
  char     *tag_name, *untag_name;
  Signal   *gerbe, *between_sig;
  Signal   *gerbe_tag, *gerbe_untag;

  //int       nbvclines=0;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extima, &gerbe_name) == TCL_ERROR)
    return TCL_ERROR;

  isBetween = arg_present(1);
  if(isBetween) {
    if (arg_get(1, &between_name, &scale_low, &scale_up) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isTag = arg_present(2);
  if(isTag) {
    if (arg_get(2, &tag_name, &untag_name) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* compute total_length for memory allocation */
  total_length       = 0;
  nb_vc = 0;
  total_length_between = 0;
  total_length_tag = 0;
  total_length_untag = 0;

  for (i=0;i<extima->extrNb;i++) {
    ext = &(extima->extr[i]);
    if (!ext->up && !ext->down) {
      continue;
    }
    nb_vc++;
    /* Follow the chain down to up */
    length = 1;
    lmod = ext->mod;
    
    while (ext->up) {
      ext = ext->up;
      length++;
    }
    umod = ext->mod;
    
    total_length += length;

    if (ext->tag == 1) {
      total_length_tag += length;
    } else {
      total_length_untag += length;
    }
    
    if (length >= scale_low && length < scale_up) {
      total_length_between += length;
    }
  } /* end of for (i=0;i<extima->extrNb;i++) loop  */
  
  sprintf (interp->result, "nb_vc %d total_length %d total_length_tag %d total_length_untag %d\n\n", nb_vc, total_length, total_length_tag, total_length_untag);
  /*return TCL_OK;*/
  
  /* on alloue de la memoire pour l'histogramme */
  gerbe       = sig_new (REALXY, 0, 2*total_length-1         );
  
  if (isBetween) {
    between_sig  = sig_new (REALXY, 0, 2*total_length_between-1);
  }
  
  if (isTag) {
    gerbe_tag    = sig_new (REALXY, 0, 2*total_length_tag-1);
    gerbe_untag  = sig_new (REALXY, 0, 2*total_length_untag-1);
  }
  
 

  /* on remplit gerbe */
  count = 0;
  nb_vc = 0;

  for (i=0;i<extima->extrNb;i++) {
    ext = &(extima->extr[i]);
    if (!ext->up && !ext->down) {
      continue;
    }
    
    /* Follow the chain down to up */
    length = 0;	
    gerbe->dataX[count] = length;
    gerbe->dataY[count] = log(ext->mod)/log(2);
    count++;
    
    while (ext->up) {
      ext = ext->up;
      length++;
      gerbe->dataX[count] = length;
      gerbe->dataY[count] = log(ext->mod)/log(2);
      count++;
    }
    while (length>=0) {
      gerbe->dataX[count] = length;
      gerbe->dataY[count] = log(ext->mod)/log(2);
      ext = ext->down;
      length--;
      count++;
    }
  } /* end of for (i=0;i<extima->extrNb;i++) loop  */
  
  
  /*sprintf (interp->result, "total_length %d count %d\n", total_length, count);
    return TCL_OK;*/
  
  if (isBetween) {
    count = 0;
    for (i=0;i<extima->extrNb;i++) {
      ext = &(extima->extr[i]);
      if (!ext->up && !ext->down) {
	continue;
      }
      
      /* Follow the chain down to up */
      length = 1;	
      while (ext->up) {
	ext = ext->up;
	length++;
      }
      if (length >= scale_low && length < scale_up) {
	while (ext->down) {
	  ext = ext->down;
	}
	length2 = 0;
	between_sig->dataX[count] = length2;
	between_sig->dataY[count] = log(ext->mod)/log(2);
	count++;
	while (ext->up) {
	  ext = ext->up;
	  length2++;
	  between_sig->dataX[count] = length2;
	  between_sig->dataY[count] = log(ext->mod)/log(2);
	    count++;
	  }
	  while (length2>=0) {
	    between_sig->dataX[count] = length2;
	    between_sig->dataY[count] = log(ext->mod)/log(2);
	    ext = ext->down;
	    length2--;
	    count++;
	  }
      } /* end if */
    } /* end of for (i=0;i<extima->extrNb;i++) loop  */
  } /* end isBetween */
  
  if (isTag) {
    count_tag = 0;
    count_untag = 0;
    for (i=0;i<extima->extrNb;i++) {
      ext = &(extima->extr[i]);
      if (!ext->up && !ext->down) {
	continue;
      }
      
      /* Follow the chain down to up */
      length = 0;	
      if (ext->tag == 1) {
	gerbe_tag->dataX[count_tag] = length;
	gerbe_tag->dataY[count_tag] = log(ext->mod)/log(2);
	count_tag++;
	
	while (ext->up) {
	  ext = ext->up;
	  length++;
	  gerbe_tag->dataX[count_tag] = length;
	  gerbe_tag->dataY[count_tag] = log(ext->mod)/log(2);
	  count_tag++;
	}
	while (length>=0) {
	  gerbe_tag->dataX[count_tag] = length;
	  gerbe_tag->dataY[count_tag] = log(ext->mod)/log(2);
	  ext = ext->down;
	  length--;
	  count_tag++;
	}
      } else {
	gerbe_untag->dataX[count_untag] = length;
	gerbe_untag->dataY[count_untag] = log(ext->mod)/log(2);
	count_untag++;
	
	while (ext->up) {
	  ext = ext->up;
	  length++;
	  gerbe_untag->dataX[count_untag] = length;
	  gerbe_untag->dataY[count_untag] = log(ext->mod)/log(2);
	  count_untag++;
	}
	while (length>=0) {
	  gerbe_untag->dataX[count_untag] = length;
	  gerbe_untag->dataY[count_untag] = log(ext->mod)/log(2);
	  ext = ext->down;
	  length--;
	  count_untag++;
	}
      }
    } /* end of for (i=0;i<extima->extrNb;i++) loop  */
  }
  
  store_signal_in_dictionary(gerbe_name, gerbe);
  sprintf (interp->result, "count %d\n", count);
  if (isBetween) {
    store_signal_in_dictionary(between_name, between_sig);
  }
  if (isTag) {
    store_signal_in_dictionary(tag_name, gerbe_tag);
    store_signal_in_dictionary(untag_name, gerbe_untag);
  }
  
  return TCL_OK;
}

/******************************************
 * Command name in xsmurf : ei3Dsmallcorr2
 ******************************************/
int
ei3Dsmall_correlation2_TclCmd_ (ClientData clientData,
				Tcl_Interp *interp,
				int        argc,
				char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "TTs",
    "-mean1", "f",
    "-mean2", "f",
    "-max", "",
    NULL
  };

  char * help_msg =
  {
    ("  Compute the correlation between 2 ext images 3Dsmall. The points involved are "
     "the maxima along the contour surfaces (WT3M).\n"
     "TAKE CARE : ei3Dsmallcorr2 is \"analog\" to eicorr2\n"
     "it takes into account periodic effects, that's to say the largest distance\n"
     "we can use to compute correlation is L/2.\n"
     "\n"
     "Note that the two ext-ima must have the same sizes.\n"
     "Parameters :\n"
     "  2 ext images  - Ext images to treat.\n"
     "  string        - Name of the resulting signal.\n"
     "\n"
     "Options :\n"
     "  -max : points involved are WTMM\n"
     "\n"
     "Return value :\n"
     "  Name of the resulting signal.")
  };

  /* Command's parameters */
  ExtImage3Dsmall *ei1;
  ExtImage3Dsmall *ei2;
  char            *sName;

  /* Options's presence */
  int isMean1;
  int isMean2;
  int isMax;

  /* Options's parameters */
  real mean1 = 0;
  real mean2 = 0;

  /* Other variables */
  Signal          * sig;
  int               x, y, z, x1, y1, z1, x2, y2, z2, x22, y22, z22;
  int               i1, i2;
  Extremum3Dsmall * ext1;
  Extremum3Dsmall * ext2;
  int               lx, ly, lz, n, i;
  int             * nb;
  real              dist, distcurrent;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ei1, &ei2, &sName) == TCL_ERROR)
    return TCL_ERROR;

  isMean1 = arg_present(1);
  if (isMean1) {
    if (arg_get(1, &mean1) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isMean2 = arg_present(2);
  if (isMean2) {
    if (arg_get(2, &mean2) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isMax   = arg_present(3);

  /* Parameters validity and initialisation */

  /* Treatement */

  lx = ei1->lx;
  ly = ei1->ly;
  lz = ei1->lz;
  n = sqrt(lx*lx+ly*ly+lz*lz);
  sig = sig_new (REALY, 0, n - 1);
  sig->x0 = 0.0;
  sig->dx = 1;

  nb = (int *) malloc(sizeof(int)*n);
  for (i = 0; i < n; i++) {
    nb[i] = 0;
    sig->dataY[i] = 0.0;
  }

  if (isMax) {
    for (i1=0;i1<ei1->extrNb;i1++) {
      ext1 = &(ei1->extr[i1]);
      x1 = ext1->pos%lx;
      y1 = (ext1->pos/lx) % ly;
      z1 = (ext1->pos/lx) / ly;
      
      for (i2=0;i2<ei2->extrNb;i2++) {
	ext2 = &(ei2->extr[i2]);
	
	x = x1 - ext2->pos%lx;
	y = y1 - ((ext2->pos/lx)%ly);
	z = z1 - ((ext2->pos/lx)/ly);
	
	dist = sqrt(x*x + y*y + z*z);
	i = (int) (dist+0.5);
	sig->dataY[i] += (log(ext1->mod)-mean1)*(log(ext2->mod)-mean2);
	nb[i]++;
	
      }
    }
  } else {
    for (i1=0;i1<ei1->extrNb;i1++) {
      ext1 = &(ei1->extr[i1]);
      if (!ext1->up && !ext1->down) {
	continue;
      }
      
      x1 = ext1->pos%lx;
      y1 = (ext1->pos/lx) % ly;
      z1 = (ext1->pos/lx) / ly;
      
      for (i2=0;i2<ei2->extrNb;i2++) {
	ext2 = &(ei2->extr[i2]);	
	if (!ext2->up && !ext2->down) {
	  continue;
	}
	
	x2 = ext2->pos%lx;
	y2 = (ext2->pos/lx) % ly;
	z2 = (ext2->pos/lx) / ly;
	
	x = x1 - x2;
	y = y1 - y2;
	z = z1 - z2;
	dist = sqrt(x*x + y*y + z*z);
	
	/* move position of ext2 in the 26 periodic images of current
	   data, and find the distance minimum */
	if (dist > lx/2) {
	  int incr_x[] = { 1,  1,  0, -1, -1, -1,  0,  1,  0,  1,  1,  0, -1, -1, -1,  0,  1,  1,  1,  0, -1, -1, -1,  0,  1,  0};
	  int incr_y[] = { 0, -1, -1, -1,  0,  1,  1,  1,  0,  0, -1, -1, -1,  0,  1,  1,  1,  0, -1, -1, -1,  0,  1,  1,  1,  0};
	  int incr_z[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1};
	  int i;
	  
	  for (i=0;i<26;i++) {
	    x22 = x2 + lx*incr_x[i];
	    y22 = y2 + ly*incr_y[i];
	    z22 = z2 + lz*incr_z[i];
	    distcurrent = sqrt((x1-x22)*(x1-x22)+(y1-y22)*(y1-y22)+(z1-z22)*(z1-z22));
	    if (distcurrent<dist) dist = distcurrent;
	  }
	}
	
	
	i = (int) (dist+0.5);
	sig->dataY[i] += (log(ext1->mod)-mean1)*(log(ext2->mod)-mean2);
	nb[i]++;
	
      }
    }
  }

  for (i = 0; i < n; i++) {
    if (nb[i] != 0) {
      sig->dataY[i] /= nb[i];
    }
  }

  store_signal_in_dictionary (sName, sig);

  Tcl_AppendResult(interp, sName, NULL);

  free (nb);

  return TCL_OK;
}

/**********************************************
 * Command name in xsmurf : ei3Dsmallloop
 **********************************************/
int
ei3Dsmall_loop_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ts",
    NULL
  };

  char * help_msg =
  {
    (" Execute a script foreach point of a 3Dsmall ext image. In this script\n"
     " the variables x, y, mod and arg refer to the coresponding fields of\n"
     " the point.\n"
     "\n"
     "Parameters :\n"
     "  3Dsmall ExtImage - ext image to treat.\n"
     "  string           - script to execute.")
  };

  /* Command's parameters */
  ExtImage3Dsmall *extImage;
  char            *scriptStr;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int result;
  Extremum3Dsmall *currentExt;
  int length;
  
  int extInd;
  Tcl_Obj *modStrObj, *modObj;
  Tcl_Obj *xStrObj, *xObj;
  Tcl_Obj *yStrObj, *yObj;
  Tcl_Obj *zStrObj, *zObj;
  Tcl_Obj *typeStrObj, *typeObj;
  Tcl_Obj *vclengthStrObj,         *vclengthObj;
  //Tcl_Obj *typeStrTag, *typeTag;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extImage, &scriptStr) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  extInd = 0;
  modStrObj = Tcl_NewStringObj("mod", 3);
  modObj = Tcl_NewDoubleObj(0.0);
  xStrObj = Tcl_NewStringObj("x", 1);
  xObj = Tcl_NewLongObj(0);
  yStrObj = Tcl_NewStringObj("y", 1);
  yObj = Tcl_NewLongObj(0);
  zStrObj = Tcl_NewStringObj("z", 1);
  zObj = Tcl_NewLongObj(0);
  typeStrObj = Tcl_NewStringObj("type", 4);
  typeObj = Tcl_NewStringObj("undefined", 9);
  vclengthStrObj     = Tcl_NewStringObj("vclength", 8);
  vclengthObj        = Tcl_NewLongObj(0);

  while (1) {
    int x,y,z;
    if (extInd == extImage->extrNb) {
      return TCL_OK;
    }
    currentExt = &extImage->extr[extInd];

    Tcl_SetDoubleObj(modObj, currentExt->mod);
    Tcl_ObjSetVar2(interp, modStrObj, NULL, modObj, 0);
    
    x = currentExt->pos%extImage->lx;
    y = (currentExt->pos / extImage->lx) % extImage->ly;
    z = (currentExt->pos / extImage->lx) / extImage->ly;

    Tcl_SetLongObj(xObj, x);
    Tcl_ObjSetVar2(interp, xStrObj, NULL, xObj, 0);

    Tcl_SetLongObj(yObj, y);
    Tcl_ObjSetVar2(interp, yStrObj, NULL, yObj, 0);

    Tcl_SetLongObj(zObj, z);
    Tcl_ObjSetVar2(interp, zStrObj, NULL, zObj, 0);

    if (currentExt->up || currentExt->down) {
      Tcl_SetStringObj(typeObj, "vc", 2);
    } else {
      Tcl_SetStringObj(typeObj, "undefined", 9);
    }
    Tcl_ObjSetVar2(interp, typeStrObj, NULL, typeObj, 0);

    /* *********************/
    /* computing VC_LENGTH */
    /* *********************/
    /* first go down       */
    while (currentExt->down) {
      currentExt = currentExt->down;
    }
    length = 0;
    while (currentExt->up) {
      currentExt = currentExt->up;
      length++;
    }
    /* if ext image is not part of a skeleton, length = 0 */ 
    Tcl_SetLongObj(vclengthObj, length);
    Tcl_ObjSetVar2(interp, vclengthStrObj, NULL, vclengthObj, 0);
    
    /************************/

    result = Tcl_Eval(interp, scriptStr);
    if ((result != TCL_OK) && (result != TCL_CONTINUE)) {
      if (result == TCL_ERROR) {
	char msg[60];
	sprintf(msg, "\n    (\"ei3Dsmallloop\" body line %d)",interp->errorLine);
	Tcl_AddErrorInfo(interp, msg);
      }
      break;
    }
    extInd++;
  }
  if (result == TCL_BREAK) {
    result = TCL_OK;
  }
  if (result == TCL_OK) {
    Tcl_ResetResult(interp);
  }
  return result;
}

/*******************************************
 * Command name in xsmurf : eitagvc3Dsmall
 *******************************************/
/* created on august 12th 2003 by pierre kestener */
int
e_tag_vc_3Dsmall_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "T",
    "-smallscale",   "df",
    NULL
  };
  
  char * help_msg =
  {
    ("  Tag vertical chain of that are on a extimage3Dsmall according to\n"
     "a given criteria.\n"
     "Tag an extremun3Dsmall consists in setting its \"tag\" field to 1.\n"
     "Use this command after vchain3Dsmall command.\n"
     "\n"
     "Parameters:\n"
     "  extimage3dsmall  - Extimage3Dsmall to treat.\n"
     "\n"
     "Options:\n"
     " -smallscale [df] : tag a vertical chain if the slope between scale 0\n"
     "                    and scale s [d] is larger than a certain slope [f]\n"
     "\n"
     "Return value:\n"
     "  Name of the resulting signal.")
  };
  
  /* Command's parameters */
  ExtImage3Dsmall *ei;
  
  /* Options's presence */
  int isSmallscale;

  /* Options's parameters */
  int count;
  int vc_length;            /* used with nullholder option */

  /* Other variables */
  Extremum3Dsmall *ext;
  
  real     uMod;
  real     lMod;
  real     maxMod, minMod;
  real     thresh;
  real     threshlow;
  real     threshup;
  int      scale;
  int      scalenb;
  int      nb = 0;
  int      decCount, length;
  int      maxPos, minPos;
  int      curPos;
  real     mmod;
  real     current_mmod;
  real     lMod_th, ratio_th;
  int      vc_length_th;
  int      decreasing_mod = 1; /* used to know if the module the chain is decreasing*/
  int      index, index_th;

  real     oldMod;
  int      total_size = 0; /* used with isMeanMod option */
  int      nbtot = 0;

  int      i;

  real     thMod, slopeTh, slopeEstim;
  int      scaleTh; 

  char *position_lst_str;
  char **position_elt;
  int  code, position_lst_size;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ei) == TCL_ERROR)
    return TCL_ERROR;

  isSmallscale = arg_present(1);
  if (isSmallscale) {
    if (arg_get(1, &scaleTh, &slopeTh) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
 
  /* Parameters validity and initialisation */
  for (i=0;i<ei->extrNb;i++) {
    ext = &(ei->extr[i]);
    if (!ext->up && !ext->down) {
      continue;
    }
    /* Get the modulus of the wtmmm at lowest scale */
    while (ext->down) {
      ext = ext->down;
    }
    lMod = ext->mod;
    
    decCount = 0;  /* decrease count */
    curPos = 0;
    
    /* Follow the chain down to up */
    //ext->tag = 0;
    maxMod = ext->mod;
    minMod = ext->mod;
    thMod  = ext->mod;
    maxPos = curPos;
    minPos = curPos;
    scalenb = 0;
    length = 0;

    while (ext->up) {
      thMod = ext->mod;
      if (scalenb>=scaleTh)
	break;
      ext = ext->up;
      scalenb++;
      length++;
      curPos++;  
    }    
    //uMod = ext->mod;
    
    while (ext->up) {
      ext = ext->up;
      length++;
    }
    uMod = ext->mod;
    
    nbtot++;
    
    if (isSmallscale) {
      slopeEstim = 10.0*log(thMod/lMod)/log(2)/scalenb;
      //if ( slopeEstim > slopeTh && scalenb >5) {}
      if ( slopeEstim > slopeTh && length <11) {
	ext->tag = 1;
	while (ext->down) {
	  ext = ext->down;
	  ext->tag = 1;
	}
	nb++;
      }
    }
  } /* end for loop */

  
  sprintf (interp->result, "nbtag %d nbtot %d", nb, nbtot);
  
  return TCL_OK;
}

/**********************************
 * Command name in xsmurf : wtmm3d 
 **********************************/
int
w3_wtmm3d_TclCmd_(ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)      
{
  /* Command line definition */
  char * options[] =
  {
    "JJJsfss",
    "-vector", "JJJJJJ",
    "-svdtype", "d",
    "-svd_LT", "ss",
    "-svd_LT_max", "ss",
    NULL
  };
  
  char * help_msg =
  {
    ("  Extract the contour surfaces from three 3D images (gradX, gradY,\n"
     "gradZ).\n"
     "\n"
     "\n"
     "Parameters:\n"
     "  3 images - gradx, grady, gradz.\n"
     "  string   - Name of the resulting extima3d image (WTMM edges).\n"
     "  float    - Scale of the ext image.\n"
     "  string   - Name of modulus image.\n"
     "  string   - Name of the output extima3d image (local modulus maxima,\n"
     "             kind of WTMMM)\n"
     "\n"
     "Options:\n"
     "\n"
     "  -vector [JJJJJJ]: extract contour surfaces of a 3D -> 3D vector\n"
     "                    field. User must provide (gradX2, gradY2, gradZ2)\n"
     "                    and (gradX3, gradY3, gradZ3) images for the second\n"
     "                    and thrid components of the vector field.\n"
     "  -svdtype [d]: argument must be 0 or 1. Allow to select max/min svd\n"
     "                value\n"
     "                SVD_TYPE_MAX(=0) means get max value + associated direction\n"
     "                SVD_TYPE_MIN(=1) means get min value + associated direction\n"
     "  -svd_LT [ss]: also output the longitudinal/transversal information (2 strings required\n"
     "                for the names of the additional images\n"
     "  -svd_LT_max [ss]: if this option is present, we also output maxima chains, with modL/modT values\n"
     "\n"
     "Return value:\n"
     "  None.\n"
     "NB : used in WtmmgCurrentScale routine (see tcl_library/imStudy.tcl)\n")
  };
  
  /* Command's parameters */
  Image3D  *gradx, *grady, *gradz;
  char     *extImageName, *extImageName2;
  char     *modName;
  real     scale = -1;

  /* Options's presence */
  int isVector;
  int isSvdType;
  int isSvd_LT;
  int isSvd_LT_max;

  /* Options's parameters */
  Image3D  *gradx2, *grady2, *gradz2, *gradx3, *grady3, *gradz3;
  int       svdtype = SVD_TYPE_MAX;
  char     *modName_L, *modName_T;
  char     *modName_ext_L, *modName_ext_T;
  
  /* Other variables */
  ExtImage3Dsmall *extImage, *extImage2;
  Extremum3Dsmall *extremum, *extremum2;

  int lx, ly, lz, size;
  int pos;
  int nb_of_maxima=0;
  int nb_of_local_maxima=0;

  // Regular Wavelet Transform vector
  Image3D *Mod;
  float *mod,*max;

  // longitudinal / transversal information
  Image3D *ModL, *ModT;
  float   *modL, *modT;
  ExtImage3Dsmall *extImage_modL;
  ExtImage3Dsmall *extImage_modT;

  unsigned char *bufferPos;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg)) {
    return TCL_OK;
  }
  
  if (arg_get (0, &gradx, &grady, &gradz,
	       &extImageName, &scale, &modName, &extImageName2)==TCL_ERROR) {
    return TCL_ERROR;
  }

  isVector = arg_present(1);
  if (isVector) {
    if (arg_get(1, &gradx2, &grady2, &gradz2, &gradx3, &grady3, &gradz3) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isSvdType = arg_present(2);
  if (isSvdType) {
    if (arg_get(2, &svdtype) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isSvd_LT = arg_present(3);
  if (isSvd_LT) {
    if (arg_get(3, &modName_L, &modName_T) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isSvd_LT_max = arg_present(4);
  if (isSvd_LT_max) {
    if (arg_get(4, &modName_ext_L, &modName_ext_T) == TCL_ERROR) {
      return TCL_ERROR;
    }
    
    if (!isSvd_LT) {
      return GenErrorAppend(interp,"Option svd_LT_max cannot be used if svd_LT is not acitvated (longitudinal/tranversal information must have been computed)",
			    NULL);
    }
  }
  
  /* Parameters validity and initialisation */
  if ((gradx->lx != grady->lx) ||
      (gradx->ly != grady->ly) ||
      (gradx->lz != grady->lz)) {
    Tcl_AppendResult(interp, "Inputs must have the same sizes!!!.", NULL);
    return TCL_ERROR;
  }
  if ((gradx->lx != gradz->lx) ||
      (gradx->ly != gradz->ly) ||
      (gradx->lz != gradz->lz)) {
    Tcl_AppendResult(interp, "Inputs must have the same sizes!!!.", NULL);
    return TCL_ERROR;
  }
  if ((gradx->type != PHYSICAL) ||
      (grady->type != PHYSICAL) ||
      (gradz->type != PHYSICAL)) {
    Tcl_AppendResult(interp, "Inputs must have a PHYSICAL type!!!.", NULL);
    return TCL_ERROR;
  }

  if (isVector) {
    if ((gradx2->lx != grady2->lx) ||
	(gradx2->ly != grady2->ly) ||
	(gradx2->lz != grady2->lz)) {
      Tcl_AppendResult(interp, "Inputs2 must have the same sizes!!!.", NULL);
      return TCL_ERROR;
    }
    if ((gradx->lx != gradx2->lx) ||
	(grady->ly != grady2->ly)||
	(gradz->ly != gradz2->ly)) {
      Tcl_AppendResult(interp, "All image inputs must have the same sizes!!!.", NULL);
      return TCL_ERROR;
    }
    if ((gradx2->type != PHYSICAL) ||
	(grady2->type != PHYSICAL)) {
      Tcl_AppendResult(interp, "Inputs2 must have a PHYSICAL type!!!.", NULL);
      return TCL_ERROR;
    }
  
  }

  /* here we go */
  lx = gradx->lx;
  ly = gradx->ly;
  lz = gradx->lz;
  size = gradx->size;

  /* memory allocation for modulus image */
  Mod = im3D_new (gradx->lx, gradx->ly, gradx->lz, gradx->size, gradx->type);

  if (!Mod)
    return TCL_ERROR;
  mod = Mod->data;
  max = (float *) malloc (gradx->size * sizeof(float));

  bufferPos = (unsigned char *) calloc (gradx->size, sizeof(unsigned char));

  /* fill pos_incr array with neighbour positions */
  init_pos_incr (lx,ly);

  if (isSvd_LT) {
    ModL = im3D_new (gradx->lx, gradx->ly, gradx->lz, gradx->size, gradx->type);
    ModT = im3D_new (gradx->lx, gradx->ly, gradx->lz, gradx->size, gradx->type);
    if ((!ModL) || (!ModT))
      return TCL_ERROR;
    modL = ModL->data;
    modT = ModT->data;
  }
  
  /* Treatement */
  smSetBeginTime();

  if (isVector) {

    // Extract Longitudinal / Transversal information
    // gradx,grady,gradz are not modified by this
    // modL / modT are output
    if (isSvd_LT) { 
      Extract_Gradient_Maxima_3D_vectorfield_LT( gradx,  grady,  gradz,
						 gradx2, grady2, gradz2,
						 gradx3, grady3, gradz3,
						 modL, modT);
    }

    // gradx,grady, gradz will be modified after this function call
    Extract_Gradient_Maxima_3D_vectorfield( gradx,  grady,  gradz,
					    gradx2, grady2, gradz2,
					    gradx3, grady3, gradz3,
					    mod, max, scale, svdtype);
  } else {
    
    Extract_Gradient_Maxima_3D( gradx, grady, gradz, mod, max, scale);
    
  }
  
  /* compute nb of maxima (used for memory allocation) */
  /* initialize bufferPos which is a mask for the location of edges points */
  {
    float *tmpMax = (float *) max;
    unsigned char *tmpPos = (unsigned char *) bufferPos;
    int i;
    for (i = 0; i < size; i++, tmpMax++, tmpPos++) {
      if (*tmpMax>0) {
	nb_of_maxima++;
	(*tmpPos) = 1;
      }
    }
  }

  /* compute number of local maxima */
  {
    float *tmpMax  = (float *) max;
    float *tmpMax2 = (float *) max;
    unsigned char *tmpPos = (unsigned char *) bufferPos;
    int i;
    for (i = 0; i < size; i++,tmpMax++) {
      if (*tmpMax>0) {  /* there's an extrema at location "i" ! */
	if (_is_max_(tmpMax2,tmpPos,i,lx,ly,lz)) /* this extrema is a maxima */
	  nb_of_local_maxima++;
      }
    }
  }
  
  /* extima memory allocation for wtmm edges */
  extImage = w3_ext_small_new (nb_of_maxima, lx, ly, lz, scale);
  if (!extImage) {
    return GenErrorMemoryAlloc(interp);
  }

  /* extima memory allocation for "wtmmm" local maxima */
  extImage2 = w3_ext_small_new (nb_of_local_maxima, lx, ly, lz, scale);
  if (!extImage2) {
    return GenErrorMemoryAlloc(interp);
  }

  // if isSvd_LT_max is true, we also save ext-image containing
  // Longitudinal / Transversal wavelet values
  if (isSvd_LT_max) {
    
    extImage_modL = w3_ext_small_new (nb_of_local_maxima, lx, ly, lz, scale);
    extImage_modT = w3_ext_small_new (nb_of_local_maxima, lx, ly, lz, scale);

    if (!extImage_modL) {
      return GenErrorMemoryAlloc(interp);
    }
    if (!extImage_modT) {
      return GenErrorMemoryAlloc(interp);
    }

  }
  
  /*
   * Update ExtImage and Extimage2 structures.
   */
  extremum  = extImage->extr;
  extremum2 = extImage2->extr;
  {
    float *tmpMod  = (float *) mod;
    float *tmpMax  = (float *) max;
    float *tmpMax2 = (float *) max;
    unsigned char *tmpPos = (unsigned char *) bufferPos;
    int i;
    for (i = 0; i < lx*ly*lz; i++, tmpMod++, tmpMax++) {
      if (*tmpMax>0) {
	extremum->mod = *tmpMod;
	extremum->pos = i;
	extremum++;
	/*printf("%d %d\n",i%lx,i/lx);*/
	if (_is_max_(tmpMax2,tmpPos,i,lx,ly,lz)) /* this extrema is a maxima */
	  {
	    extremum2->mod = *tmpMod;
	    extremum2->pos = i;
	    extremum2++;
	  }
	
      }
    }
  }

  // update Longitudinal / Transversal extImage
  if (isSvd_LT_max) {

    Extremum3Dsmall *extremumL, *extremumT;

    extremumL = extImage_modL->extr;
    extremumT = extImage_modT->extr;
    
    float *tmpModL = (float *) modL;
    float *tmpModT = (float *) modT;
    float *tmpMax  = (float *) max;
    float *tmpMax2 = (float *) max;
    unsigned char *tmpPos = (unsigned char *) bufferPos;
    int i;

    for (i = 0; i < lx*ly*lz; i++, tmpModL++, tmpModT++, tmpMax++) {
      if (*tmpMax>0) {
	if (_is_max_(tmpMax2,tmpPos,i,lx,ly,lz)) /* this extrema is a maxima */
	  {
	    extremumL->mod = *tmpModL;
	    extremumT->mod = *tmpModT;

	    extremumL->pos = i;
	    extremumT->pos = i;

	    extremumL++;
	    extremumT++;
	  }
      }
    }
    
  } // end of isSvd_LT_max
  
  smSetEndTime();

  Ext3DsmallDicStore(extImageName, extImage);
  Ext3DsmallDicStore(extImageName2, extImage2);
  store_image3D(modName,Mod);

  if (isSvd_LT) {
    store_image3D(modName_L, ModL);
    store_image3D(modName_T, ModT);
  }

  if (isSvd_LT_max) {
    Ext3DsmallDicStore(modName_ext_L, extImage_modL);
    Ext3DsmallDicStore(modName_ext_T, extImage_modT);
  }

  sprintf(interp->result, "%f", smGetEllapseTime());
  
  free(max);
  free(bufferPos);

  return TCL_OK;
}

