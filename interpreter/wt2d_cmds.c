/*
 * wt2d_cmds.c --
 *
 *   Copyright (c) 1999 Nicolas Decoster
 *   Copyright (c) 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *
 *   Copyright (c) 1999-2007 Pierre Kestener.
 *   Copyright (c) 1999-2002 Centre de Recherche Paul Pascal, Bordeaux, France.
 *   Copyright (c) 2002-2003 Ecole Normale Superieure de Lyon, Lyon, France.
 *   Copyright (c) 2003-2007 CEA DSM/DAPNIA/SEDI, centre Saclay, France.
 *
 */

/* dec 2005 : add option to save ext-image in JPEG image file format */

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <matheval.h>
#include "../wt2d/wt2d.h"
#include "../stats/stats.h"
#include "../wt2d/skeleton.h"
#include "../image/image.h"
#include "../image/image3D.h"

#include "../edge/extrema.h"

#include <assert.h>

#include <jpeglib.h>

#include "smPkgInt.h"

/* ce fichier est a ranger, car c'est un sacre bordel ! */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef PI
#define PI 3.14159265358979323846
#endif


/*
 * !!!!!!!!!!!!!!!!!!
 * position a revoir
 * !!!!!!!!!!!!!!!!!!
 */
void store_line_in_dictionary      (char *name, Line *line_ptr); 

/************************************************
 *   fonction pour echanger les octets
 *  Big-->LitEndian et inversement
 *  repris dans Lastwave 1.7
 ************************************************/
void BigLittleValues(void *array, int n, size_t sizeval);

/*----------------------------------------------------------------------
  ExtChnChainCmd_
  
  Commande Tcl lancant le chainage...

  nom de la commande dans xsmurf : chain2
  --------------------------------------------------------------------*/
int
ExtChnChainCmd_(ClientData clientData,
		Tcl_Interp *interp,
		int        argc,
		char       **argv) 
{
  char     * options[] = { "L",
			   "-r", "",
			     NULL };
  char     ** expr;
  ExtImage ** extImageList;
  Extremum ** down;
  int      n, lx, ly, i, stamp;

  if (arg_init(interp, argc, argv, options, NULL))
    return TCL_OK;

  if (arg_get(0, &expr) == TCL_ERROR)
    return TCL_ERROR;
  
  if (ExtDicImageListGet(interp, expr, &extImageList) == TCL_ERROR)
    return TCL_ERROR;
  
  if (!extImageList)
    return GenErrorAppend(interp, "No matching Extimage.", NULL);
  
  /* on verifie que toutes les images ont les memes dimensions */
  lx=extImageList[0]->lx;
  ly=extImageList[0]->ly; 
  for (i=1;extImageList[i];i++)
    if ((extImageList[i]->lx != lx) || (extImageList[i]->ly != ly))
      {
	GenErrorAppend(interp,"Specified ExtImages don't have the same size",
		       NULL);
	free(extImageList);
	return TCL_ERROR;
      }
  
  /* on trie la liste par ordre croissant d'echelle */
  ExtMisSortImageList_(extImageList); 
  
  /* destruction d'un eventuel ancien chainage */
  for (n=0;extImageList[n];n++)
    ExtImaUnlink_(extImageList[n]);
  
  /* nouveau stamp pour le chainage que l'on cree */
  stamp = _GetNewStamp_();
  for (n=0;extImageList[n];n++)
    extImageList[n]->stamp = stamp;

  /* buffer temporaire pour stocker des references aux extrema*/
  down = (Extremum **) malloc (sizeof(Extremum*) * lx * ly);
  if(!down)
    exit(0);

  _CreateExtArray_ (extImageList[0], down);
  _HorizontalChain_(down, extImageList[0], lx, ly);
  
  for (n=1;extImageList[n];n++)
    {
      _CreateExtArray_(extImageList[n], down);

      /* on a fixe boxsize et argSimil */
      for (i=0;i<3;i++)
	VerticalChain(down, extImageList[n-1], lx, ly, 16, 0.5);
      
      fflush(stdout);
      _HorizontalChain_(down, extImageList[n], lx, ly);
    }
  fflush(stdout);
  free(down);
  /* on enleve les extrema isoles */
  if(arg_present(1))
    for (n=0;extImageList[n];n++)
      _RemoveShortChains_(extImageList[n]);

  return TCL_OK;
}


/*----------------------------------------------------------------------
  ExtChnThreshCmd_
  
  La commande Tcl permettant le seuillage sur le chainage:

  Seuillage sur la longueur des chaines : enlever les chaines de longueur
  inferieur a n.
  Seuillage sur les echelles : enlever les chaines qui n'atteignent
  pas l'echelle e.
  Seuillage sur la premiere echelle : enlever les chaines ne commencant
  pas au dessous de l'echelle donnee
  --------------------------------------------------------------------*/
/*
  command name in xsmurf : ethresh
 */
int
ExtChnThreshCmd_(ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv) 
{
  char     * options[] = { "L",
			   "-vlength","d",
			   "-hlength","d",
			   "-firstscale","f",
			   NULL };

  char * help_msg =
  {
    ("  Threshold apply to ExtImage before chain.\n"
     "\n")
  };
  
  char     ** expr;
  ExtImage ** extImageList, * extImage;
  Extremum *firstExt, *tmpExt;
  int      nb, i, j, stamp, vL=0, hL=0, length;
  real    firstScale; 

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &expr) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &vL) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(2, &hL) == TCL_ERROR)
    return TCL_ERROR;
  
  if (ExtDicImageListGet(interp, expr, &extImageList) == TCL_ERROR)
    return TCL_ERROR;
  
  if (!extImageList)
    return GenErrorAppend(interp, "No matching Extimage.", NULL);
  
  /* on verifie que toutes les images ont les memes dimensions */
  stamp = extImageList[0]->stamp;
  for (i=1,nb=0;extImageList[i];nb++,i++)
    if (extImageList[i]->stamp != stamp)
      {
	GenErrorAppend(interp, "Specified ExtImages are not linked together.",
		       NULL);
	free(extImageList);
	return TCL_ERROR;
      }
  
  /* on trie la liste par ordre croissant d'echelle */
  ExtMisSortImageList_(extImageList); 
  
  /* seuillage sur les chaines a une echelle donnee. On enleve dans le
     tableau chain[] les references aux chaines de longueur inferieure a hL */ 
  
  for (i=0; extImageList[i]; i++){
    extImage = extImageList[i];
    for (j=0; j< extImage->chainNb; j++)
      {
	length   = 0;
	firstExt = extImage->chain[j];
	
	/* calcul de la longueur de la chaine pointee par firstExt */
	for (tmpExt=firstExt;
	     tmpExt && (tmpExt->next != firstExt);
	     tmpExt = tmpExt->next)
	  length++;

	if (length < hL)	/* la chaine doit etre retiree */
	  {
	    /* decremente le nb total de chaines a cette echelle */
	    extImage->chainNb --;
	    
	    /* on place la derniere chaine a la place de celle-ci */
	    extImage->chain[j] = extImage->chain[extImage->chainNb];
	    
	    /* on decremente j pour tester la longueur de la chaine
	       ainsi remplacee... */
	    j--;
	  }
      }
  }
  
  /* seuillage le long des echelles. On enleve les liens des chaines
     de longueur inferieure a vH */
  
  for (i=0; extImageList[i]; i++) {
    extImage = extImageList[i];
    for (j=0; j< extImage->chainNb; j++)  
      {
	length   = 0;
	firstExt = extImage->chain[j];
	
	if (firstExt->down)	/* la chaine a deja ete testee a l'echelle */
	  continue;		/* precedente */

	/* calcul de la longueur de la chaine pointee par firstExt */
	for (tmpExt=firstExt;tmpExt->up;tmpExt = tmpExt->up)
	  length++;
	
	/* la chaine est trop courte, on enleve ses liens */
	if (length < vL)
	  {
	    tmpExt = firstExt;
	    while (tmpExt)
	      {
		firstExt     = tmpExt ->next;
		tmpExt->down = NULL;
		tmpExt->up   = NULL;
		tmpExt = firstExt;
	      }
	  }
      }
  }

  /* seuillage sur la position du debut des chaines... On enleve les
     chaines qui ne commencent pas au dessous de l'echelle firstScale */
  if (arg_present(3))
    {
      if (arg_get(3, &firstScale) == TCL_ERROR)
	return TCL_ERROR;
      
      for (i=0; extImageList[i]; i++){
	extImage = extImageList[i];
	if (extImage->scale > firstScale)
	  for (j=0; j< extImage->chainNb; j++)  
	    {
	      firstExt = extImage->chain[j];
	      
	      if (!firstExt->down) {/* la chaine commence a cette echelle,
				      il faut l'eliminer */
		tmpExt = firstExt;
		while (tmpExt)
		  {
		    firstExt     = tmpExt ->next;
		    tmpExt->down = NULL;
		    tmpExt->up   = NULL;
		    tmpExt = firstExt;
		  }
	      }
	    }
      }
    }
  
  return TCL_OK;
}



/*typedef struct point{
  int x;
  int y;
  real z;
} point;
*/
/*------------------------------------------------------------------------
  _SaveExtImage_
  ----------------------------------------------------------------------*/
static int
_SaveExtImage_(Tcl_Interp * interp,
	       ExtImage * extImage,
	       char * extImageFilename,
	       int ascii,
	       int isVc)
{
  FILE     * fileOut;
  int        lx,ly,i,size;
  real      scale;
  Extremum * extr;
  int vcN = 0;
  
  if (!(fileOut = fopen(extImageFilename, "w")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for writing.", NULL);
  lx    = extImage->lx;
  ly    = extImage->ly;
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

  if (arg_present(1)) {
    if (isVc) {
      fprintf(fileOut,
	      "Ascii  ExtImage %dx%d %d %f\n",
	      lx, ly, vcN, scale);
      for (i = 0; i < size; i++) {
	if (extr[i].up || extr[i].down) {
	  fprintf(fileOut,
		  "%d %.15g %g\n",
		  extr[i].pos, extr[i].mod, extr[i].arg);
	}
      }
    } else {
      fprintf(fileOut,
	      "Ascii  ExtImage %dx%d %d %f\n",
	      lx, ly, size, scale);
      for (i = 0; i < size; i++) {
	fprintf(fileOut,
		"%d %.15g %g\n",
		extr[i].pos, extr[i].mod, extr[i].arg);
      }
    }
  } else {
    if (isVc) {
      fprintf(fileOut,
	      "Binary ExtImage %dx%d %d %f"
	      "(%d byte reals, %d byte ints)\n",
	      lx, ly, vcN, scale, (int) sizeof(real),
	      (int) sizeof(int));
      for (i = 0; i < size; i++) {
	if (extr[i].up || extr[i].down) {
	  fwrite(&(extr[i].pos), sizeof(int),   1, fileOut);
	  fwrite(&(extr[i].mod), sizeof(real), 1, fileOut);
	  fwrite(&(extr[i].arg), sizeof(real), 1, fileOut);
	}
      }
    } else {
      fprintf(fileOut,
	      "Binary ExtImage %dx%d %d %f"
	      "(%d byte reals, %d byte ints)\n",
	      lx, ly, size, scale, (int) sizeof(real),
	      (int) sizeof(int));
      for (i = 0; i < size; i++) {
	fwrite(&(extr[i].pos), sizeof(int),  1, fileOut);
	fwrite(&(extr[i].mod), sizeof(real), 1, fileOut);
	fwrite(&(extr[i].arg), sizeof(real), 1, fileOut);
      }
    }
  }
  
  fclose(fileOut);

  Tcl_AppendResult(interp, extImageFilename, NULL);
  return TCL_OK;
}


/*------------------------------------------------------------------------
  _SaveExtImage_Pos_        nouveaute du 13/01/2000.
  voir description plus bas
  utilisation : image de Francois (trajectoire de bille de latex)
  Pierre Kestener
  ----------------------------------------------------------------------*/
static int
_SaveExtImage_Pos_(Tcl_Interp * interp,
		   ExtImage * extImage,
		   char * extImageFilename,
		   int ascii,
		   int isVc)
{
  FILE     * fileOut;
  int        lx,ly,i,size,posx,posy;
  real      scale;
  Extremum * extr;
  int vcN = 0;
  
  if (!(fileOut = fopen(extImageFilename, "w")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for writing.", NULL);
  lx    = extImage->lx;
  ly    = extImage->ly;
  scale = extImage->scale;
  size  = extImage->extrNb;
  extr  = extImage->extr;
  posx  = 0;
  posy  = 0;


  if (isVc) {
    for (i = 0;i < size; i++) {
      if (extr[i].up || extr[i].down) {
	vcN++;
      }
    }
  }

  if (arg_present(1)) {
    if (isVc) {
      /* fprintf(fileOut,
	      "Ascii  ExtImage %dx%d %d %f\n",
	      lx, ly, vcN, scale); */
      for (i = 0; i < size; i++) {
	if (extr[i].up || extr[i].down) {
	  posx = extr[i].pos % lx;
	  posy = (int) (extr[i].pos - posx)/lx;
	  fprintf(fileOut,
		  "%d %d\n",
		  posx, posy);
	}
      }
    } else {
      /* fprintf(fileOut,
	      "Ascii  ExtImage %dx%d %d %f\n",
	      lx, ly, size, scale); */
      for (i = 0; i < size; i++) {
	posx = extr[i].pos % lx;
	posy = (int) (extr[i].pos - posx)/lx;
	fprintf(fileOut,
		"%d %d\n",
		posx, posy);
      }
    }
  } else {
    if (isVc) {
      fprintf(fileOut,
	      "Binary ExtImage %dx%d %d %f"
	      "(%d byte reals, %d byte ints)\n",
	      lx, ly, vcN, scale, (int) sizeof(real),
	      (int) sizeof(int));
      for (i = 0; i < size; i++) {
	if (extr[i].up || extr[i].down) {
	  fwrite(&(extr[i].pos), sizeof(int),   1, fileOut);
	  fwrite(&(extr[i].mod), sizeof(real), 1, fileOut);
	  fwrite(&(extr[i].arg), sizeof(real), 1, fileOut);
	}
      }
    } else {
      fprintf(fileOut,
	      "Binary ExtImage %dx%d %d %f"
	      "(%d byte reals, %d byte ints)\n",
	      lx, ly, size, scale, (int) sizeof(real),
	      (int) sizeof(int));
      for (i = 0; i < size; i++) {
	fwrite(&(extr[i].pos), sizeof(int),   1, fileOut);
	fwrite(&(extr[i].mod), sizeof(real), 1, fileOut);
	fwrite(&(extr[i].arg), sizeof(real), 1, fileOut);
      }
    }
  }
  
  fclose(fileOut);

  Tcl_AppendResult(interp, extImageFilename, NULL);
  return TCL_OK;
}

/*------------------------------------------------------------------------
  _SaveExtImage_Jpeg_ (very simple, only grayscaled image)
  created on Dec 21 2005 for visualizing very large extima
  as obtained when studying interstellar medium simulations (5000x5000)
  ----------------------------------------------------------------------*/
static int
_SaveExtImage_Jpeg_(Tcl_Interp * interp,
		    ExtImage * extImage,
		    char * extImageFilename,
		    int jpegQuality,
		    Image *bgImage,
		    int  bgTh,
		    real thValue)
{
  FILE     * fileOut;
  int        lx,ly,i,size;
  real       scale;
  Extremum * extr;
  real       min,max,*bgData;

  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JDIMENSION num_scanlines;
  unsigned char *buf; 
  unsigned int row_stride;

  
  if (!(fileOut = fopen(extImageFilename, "w")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for writing.", NULL);
  lx    = extImage->lx;
  ly    = extImage->ly;
  scale = extImage->scale;
  size  = extImage->extrNb;
  extr  = extImage->extr;

  /* initiate jpeg conversion */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  cinfo.image_width = lx;
  cinfo.image_height = ly;
  
  jpeg_stdio_dest(&cinfo, fileOut);

  if (bgTh == 0) {
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
  } else {
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
  }

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo,jpegQuality<100?jpegQuality:100,TRUE);
  jpeg_start_compress(&cinfo,TRUE);

 
  /* memory allocation */
  if (bgTh == 0) {
    buf = (unsigned char *) calloc (lx * ly, sizeof(unsigned char));
  } else {
    buf = (unsigned char *) calloc (3 * lx * ly, sizeof(unsigned char));
  }

  if (!buf) {
    return GenErrorAppend(interp, "Memory allocation failed", NULL);
  }

  /* fill buffer with background image data (if given) */
  if (bgImage) {
    im_get_extrema (bgImage,&min,&max);
    bgData = bgImage->data;
    if (bgTh == 0) {
      for (i = 0; i < lx*ly; i++) {
	buf[i] = (unsigned char) ((bgData[i]-min)/(max-min)*255.0);
      }
    } else {
      unsigned char value;
      for (i = 0; i < lx*ly; i++) {
	value = (unsigned char) ((bgData[i]-min)/(max-min)*255.0);
	buf[3*i]   = value;
	buf[3*i+1] = value;
	buf[3*i+2] = value;
      }
    }
  }

  /* fill buf with extrema's locations */
  if (bgTh == 0) {
    for (i = 0; i < size; i++) {
      buf[extr[i].pos] = 255;
    }
  } else {
    int pos;
    for (i = 0; i < size; i++) {
      pos = extr[i].pos;
      if (bgData[pos] < thValue) { /* blue */
	buf[3 * extr[i].pos    ] = 0;
	buf[3 * extr[i].pos + 1] = 0;
	buf[3 * extr[i].pos + 2] = 255;
      } else { /* red */
	buf[3 * extr[i].pos    ] = 255;
	buf[3 * extr[i].pos + 1] = 0;
	buf[3 * extr[i].pos + 2] = 0;
      }
    }    
  }
  

  /* start jpeg conversion */
  {
    JSAMPROW row_pointer[1];
    unsigned int row_stride = lx;
    if (bgTh) {
      row_stride = 3 * lx;
    } 
    while (cinfo.next_scanline < cinfo.image_height) {
      row_pointer[0] = &buf[cinfo.next_scanline*row_stride];
      jpeg_write_scanlines(&cinfo,row_pointer,1);
    }
  }
  
  /* end jpeg conversion */
  jpeg_finish_compress(&cinfo);
  
  /* free memory */
  free (buf);

  /* close file */
  fclose(fileOut);
  
  /* purge jpeg structure */
  jpeg_destroy_compress(&cinfo);

  Tcl_AppendResult(interp, extImageFilename, NULL);
  return TCL_OK;
}


/*
 */
static int
_save_ext_with_links_ (Tcl_Interp * interp,
		       ExtImage * extImage,
		       char * extImageFilename,
		       int ascii,
		       int isTag,
		       int tag)
{
  FILE     * fileOut;
  int        lx,ly,size;
  real      scale;
  Extremum * extr;
  int      nb_of_lines;
  List     *line_lst;
  Line     *line;
  
  if (!(fileOut = fopen(extImageFilename, "w")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for writing.", NULL);

  lx    = extImage->lx;
  ly    = extImage->ly;
  scale = extImage->scale;
  size  = extImage->extrNb;
  extr  = extImage->extr;
  nb_of_lines = extImage->nb_of_lines;
  line_lst = extImage->line_lst;

  if (!line_lst)
    return TCL_ERROR;
  
  if (isTag) {
    size = 0;
    nb_of_lines = 0;
    foreach (line, line_lst){
      if (line->tag == tag) {
	nb_of_lines++;
	foreach (extr, line->ext_lst) {
	  size++;
	}
      }
    }
  }
  fprintf(fileOut,
	  "Binary LExtImage %dx%d %d %f"
	  "(%d byte reals, %d byte ints)\n",
	  lx, ly, size, scale,
	  (int) sizeof(real), (int) sizeof(int));
  fwrite(&(nb_of_lines), sizeof(int),   1, fileOut);
  foreach (line, line_lst){
    if (!isTag || line->tag == tag) {
      fwrite(&(line->size), sizeof(int),   1, fileOut);
      foreach (extr, line->ext_lst) {
	fwrite(&(extr->pos), sizeof(int),   1, fileOut);
	fwrite(&(extr->mod), sizeof(real), 1, fileOut);
	fwrite(&(extr->arg), sizeof(real), 1, fileOut);
      }
    }
  }

  fclose(fileOut);

  Tcl_AppendResult(interp, extImageFilename, NULL);
  return TCL_OK;
}

/************************************************************************
 *  Command name in xsmurf : esave
 *
 * ExtFileSaveCmd_
 *
 * Sauvegarde d'une ExtImage sur disque
 * . l'option ascii sauvegarde dans un format ascii
 * . l'option group sauvegarde toutes les ExtImages liees entre elles
 * . l'option pos sauvegarde uniquement les positions (images de Francois)
 *   sous forme d'un fichier avec deux colonnes : x et y (entiers dans l'
 *   intervalle 0..image->lx-1 !!!) voila, c'est comme ca!
 *
 * option "bg" can only be used with "jpeg" (background image)
 * option "bgTh" : use background image to threshold maxima chains to save
 * ************************************************************************/
int
ExtFileSaveCmd_(ClientData clientData,
		Tcl_Interp *interp,
		int argc,
		char **argv)      
{ 
  char * options[] = { "E[s]", 
		       "-ascii","",
		       "-group","",
		       "-link","",
		       "-tag", "d",
		       "-vc", "",
		       "-pos","",
		       "-jpeg", "d",
		       "-bg", "I",
		       "-bgth", "f",
		       NULL};
  
  char * help_msg =
  {
    ("  Save an ext image in a file.\n"
     "Options:\n"
     "  \"-ascii\"[] :\n"
     "  \"-group\"[] :\n"
     "  \"-link\"[] :\n"
     "  \"-tag\"[d] :\n"
     "  \"-vc\"[] :\n"
     "  \"-pos\"[] :\n"
     "  \"-jpeg\"[d] : save using JPEG file format.\n"
     "  \"-bg\"[I]   : (use with jpeg option) add a background image.\n"
     "  \"-bgth\"[f] :(use the background image to threshold maxima chains)\n"
     "         - the parameter is the threshold value (expressed in \n"
     "           background image pixel value).\n"
     "\n"
     "\n")
  };
  

  char     * extImageFilename = NULL;
  ExtImage * extImage;
  Image    * bgImage = NULL;
  int      ascii, group, is_link;
  int res;

  int isTag;
  int tag = -1;

  int isVc;
  int isPos;
  int isJpeg, jpegQuality;
  int isBg;
  int isBgTh;

  real *bgImageData;
  real  thValue;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &extImage, &extImageFilename)==TCL_ERROR)
    return TCL_ERROR;
  
  if (!extImageFilename)
    extImageFilename = argv[1];
  
  ascii=arg_present(1);
  group=arg_present(2);
  is_link = arg_present(3);
  isTag = arg_present(4);
  if (isTag) {
    if (arg_get(4, &tag) == TCL_ERROR) {
      return TCL_ERROR;
    }
    is_link = isTag;
  }
  isVc   = arg_present(5);
  isPos  = arg_present(6);
  isJpeg = arg_present(7);
  if (isJpeg) {
    if (arg_get(7, &jpegQuality) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isBg   = arg_present(8); 
  if (isBg) {
    if (arg_get(8, &bgImage) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isBgTh = arg_present(9);
  if (isBgTh) {
    if (arg_get(9, &thValue) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  if (is_link) {
    
    res = _save_ext_with_links_ (interp, extImage, extImageFilename, ascii, isTag, tag);
    
  } else if(isJpeg) {
    
    res = _SaveExtImage_Jpeg_(interp, extImage, extImageFilename, jpegQuality, bgImage, isBgTh, thValue);
    
  } else if (isPos) {
    
    res = _SaveExtImage_Pos_(interp, extImage, extImageFilename, ascii, isVc);
    
  } else {
    
    res = _SaveExtImage_(interp, extImage, extImageFilename, ascii, isVc);
    
  }

  return TCL_OK;
}


/*------------------------------------------------------------------------
  _LoadExtImage_
  ----------------------------------------------------------------------*/
static int
_LoadExtImage_(Tcl_Interp * interp,
	       char * extImageFilename,
	       char * extImageName,
	       int  isGrhack,
	       int  isSwap)
{
  FILE     * fileIn;
  char       tempBuffer[100], saveFormat[100], type[100];
  int        lx, ly, i, size, realSize, intSize;
  real      scale;
  ExtImage * extImage;
  Extremum * extr;
  int k;

  if (!(fileIn = fopen(extImageFilename, "r")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for reading.", NULL);
  
  fgets(tempBuffer, 100, fileIn);
  sscanf(tempBuffer, "%s %s %dx%d %d %f (%d byte reals, %d", 
	 saveFormat, type, &lx, &ly, &size, &scale, &realSize, &intSize);
  
  if (!strcmp(type, "LExtImage")) {
    Extremum *extr2;
    int nb_of_lines;
    int i, j;
    int line_size;
    Line *line;
    ExtImage *ext_image;
    int is_ascii;

    is_ascii = !strcmp(saveFormat, "Ascii");

    ext_image = w2_ext_new(size, lx, ly, scale);
    if (!ext_image) {
      return GenErrorMemoryAlloc(interp);
    }
    extImage = ext_image;
    extr2 = extImage->extr;
    k = 0;

    if (is_ascii) {
      fscanf (fileIn, "%d", &nb_of_lines);
    } else {
      fread(&(nb_of_lines), sizeof(int),   1, fileIn);
      if (isSwap) {
	BigLittleValues(&(nb_of_lines),1,sizeof(int));
      }
    }

    for (i = 0; i < nb_of_lines; i++)	{
      line = create_line (ext_image->lx, ext_image, ext_image->scale);
      if (is_ascii) {
	fscanf (fileIn, "%d", &line_size);
      } else {
	fread(&(line_size), sizeof(int),   1, fileIn);
      }
      lst_add (line, ext_image->line_lst);
      ext_image->nb_of_lines ++;
      for (j = 0; j < line_size; j++) {
	if (is_ascii) {
	  fscanf (fileIn, "%d", &(extr2[k].pos));
	  fscanf (fileIn, "%f", &(extr2[i].mod));
	  fscanf (fileIn, "%f", &(extr2[i].arg));
	} else {	      
	  fread(&(extr2[k].pos), sizeof(int),  1, fileIn);
	  fread(&(extr2[k].mod), sizeof(real), 1, fileIn);
	  fread(&(extr2[k].arg), sizeof(real), 1, fileIn);
	}
	add_extremum_to_line (&extr2[k], line, line->lx, LINE_BEG);
	k++;
      }
    }
  } else {
    Line *line;
    if (strcmp(type, "ExtImage")) {
      return GenErrorAppend(interp, "`", extImageFilename,
			    "' doesn't seem to be an ExtImage.", NULL);
    }
  
    extImage = w2_ext_new(size, lx, ly, scale);
    if (!extImage) {
      return GenErrorMemoryAlloc(interp);
    }
  
    extr = extImage->extr;
    if (isGrhack) {
      line = create_line (extImage->lx, extImage, extImage->scale);
      lst_add (line, extImage->line_lst);
      extImage->nb_of_lines ++;
    }
  
    if (!strcmp(saveFormat, "Ascii"))	{
      if (isGrhack) {
	for (i = 0; i < size; i++) {
	  fscanf(fileIn, "%d %f %f",
		 &(extr[i].pos), &(extr[i].mod), &(extr[i].arg));
	  add_extremum_to_line  (&extr[i], line, line->lx, LINE_BEG);
	  lst_add (&extr[i], line->gr_lst);
	  line->nb_of_gr++;
	}
      } else {
	for (i = 0; i < size; i++) {
	  fscanf(fileIn, "%d %f %f",
		 &(extr[i].pos), &(extr[i].mod), &(extr[i].arg));
	}
      }
    } else {
      if ((sizeof(real) != realSize) || (sizeof(int) != intSize)) {
	ExtImaDelete_(extImage);
	return GenErrorAppend(interp, "real or Int size problem...", NULL);
      }
      
      if (isGrhack) {
	for (i=0;i<size;i++) {
	  fread(&(extr[i].pos), sizeof(int),   1, fileIn);
	  fread(&(extr[i].mod), sizeof(real), 1, fileIn);
	  fread(&(extr[i].arg), sizeof(real), 1, fileIn);
	  if (isSwap) {
	    BigLittleValues(&(extr[i].pos),1,sizeof(int));
	    BigLittleValues(&(extr[i].mod),1,sizeof(real));
	    BigLittleValues(&(extr[i].arg),1,sizeof(real));
	  }
	  add_extremum_to_line (&extr[i], line, line->lx, LINE_BEG);
	  lst_add (&extr[i], line->gr_lst);
	  line->nb_of_gr++;
	}
      } else {
	for (i=0;i<size;i++) {
	  fread(&(extr[i].pos), sizeof(int),  1, fileIn);
	  fread(&(extr[i].mod), sizeof(real), 1, fileIn);
	  fread(&(extr[i].arg), sizeof(real), 1, fileIn);
	  if (isSwap) {
	    BigLittleValues(&(extr[i].pos),1,sizeof(int));
	    BigLittleValues(&(extr[i].mod),1,sizeof(real));
	    BigLittleValues(&(extr[i].arg),1,sizeof(real));
	  }
	}
      }
    }
  }
  fclose(fileIn);
  
  ExtDicStore(extImageName, extImage);

  Tcl_AppendResult(interp, extImageFilename, NULL);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  _LoadExtImage_cut_
  ----------------------------------------------------------------------*/
static int
_LoadExtImage_cut_(Tcl_Interp * interp,
		   char * extImageFilename,
		   char * extImageName,
		   int  isSwap,
		   int  isCut,
		   int  x1c,
		   int  x2c,
		   int  y1c,
		   int  y2c)
{
  FILE     * fileIn;
  char       tempBuffer[100], saveFormat[100], type[100];
  int        lx, ly, lxc, lyc, i, ic, j, size, sizec, realSize, intSize, tmp;
  int        x,y,x1,x2,y1,y2,pos;
  real       scale;
  ExtImage * extImage, *extImageCut;
  Extremum * extr, *extrCut;
  int k;

  x1 = x1c;
  x2 = x2c;
  y1 = y1c;
  y2 = y2c;

  if (!(fileIn = fopen(extImageFilename, "r")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for reading.", NULL);
  
  fgets(tempBuffer, 100, fileIn);
  sscanf(tempBuffer, "%s %s %dx%d %d %f (%d byte reals, %d", 
	 saveFormat, type, &lx, &ly, &size, &scale, &realSize, &intSize);
  
  if (strcmp(type, "ExtImage")) {
    return GenErrorAppend(interp, "`", extImageFilename,
			  "' doesn't seem to be an ExtImage.", NULL);
  }

  /* parameters check */
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
  if ((x2 < 0) || (y2 < 0) || (x1 >= lx) || (y1 >= ly)) {
    sprintf (interp->result, "bad cut domain");
    fclose(fileIn);
    return TCL_ERROR;
  }
  if (x1 < 0) {
    x1 = 0;
  }
  if (y1 < 0) {
    y1 = 0;
  }
  if (x2 >= lx) {
    x2 = lx-1;
  }
  if (y2 >= ly) {
    y2 = ly-1;
  }

  lxc = x2-x1;
  lyc = y2-y1;
  
  extImage = w2_ext_new(size, lx, ly, scale);
  if (!extImage) {
    return GenErrorMemoryAlloc(interp);
  }

  extr    = extImage->extr;

  if (!strcmp(saveFormat, "Ascii"))	{
    
    for (i = 0; i < size; i++) {
      fscanf(fileIn, "%d %f %f",
	     &(extr[i].pos), &(extr[i].mod), &(extr[i].arg));
    }
    
  } else {
    if ((sizeof(real) != realSize) || (sizeof(int) != intSize)) {
      ExtImaDelete_(extImage);
      return GenErrorAppend(interp, "real or Int size problem...", NULL);
    }
    
    for (i=0;i<size;i++) {
      fread(&(extr[i].pos), sizeof(int),  1, fileIn);
      fread(&(extr[i].mod), sizeof(real), 1, fileIn);
      fread(&(extr[i].arg), sizeof(real), 1, fileIn);
      if (isSwap) {
	BigLittleValues(&(extr[i].pos),1,sizeof(int));
	BigLittleValues(&(extr[i].mod),1,sizeof(real));
	BigLittleValues(&(extr[i].arg),1,sizeof(real));
      }
    }
  }

  fclose(fileIn);

  /* compute sizec */
  sizec = 0;
  for (i = 0; i < extImage->extrNb; i++) {
    pos = extImage->extr[i].pos;
    x = pos % extImage->lx;
    y = pos / extImage->lx;
    if ((x > x1) && (x < x2) && (y > y1) && (y < y2)) {
      sizec++;
    }
  }

  /* memory allocation for new extImageCut */
  extImageCut = w2_ext_new(sizec, lxc, lyc, scale);
  if (!extImageCut) {
    ExtImaDelete_(extImage);
    fclose(fileIn);
    return GenErrorMemoryAlloc(interp);
  }
  extrCut = extImageCut->extr;

  /* copy usefull data */
  for (i = 0, j=0; i < extImage->extrNb; i++) {
    pos = extImage->extr[i].pos;
    x = pos % extImage->lx;
    y = pos / extImage->lx;
    if ((x > x1) && (x < x2) && (y > y1) && (y < y2)) {
      extImageCut->extr[j].pos = (x-x1) + (y-y1)*lxc;
      extImageCut->extr[j].mod = extImage->extr[i].mod;
      extImageCut->extr[j].arg = extImage->extr[i].arg;
      j++;
    }
  }

  /* delete extImage */
  ExtImaDelete_ (extImage);
  
  /* store extImageCut */
  ExtDicStore(extImageName, extImageCut);

  Tcl_AppendResult(interp, extImageFilename, NULL);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  Command name in xsmurf : eload

  ExtFileLoadCmd_

  Lecture d'une extImage depuis le disque.
  ----------------------------------------------------------------------*/
int
ExtFileLoadCmd_(ClientData clientData,
		Tcl_Interp *interp,
		int argc,
		char **argv)      
{ 
  /* Command line definition */
  char * options[] =
  {
    "s[s]",
    "-grhack", "",
    "-swap_bytes", "",
    "-ecut", "dddd",
    NULL
  };

  char * help_msg =
  {
    ("  Load an ext image .\n"
     "\n"
     "Parameters:\n"
     "  string   - Name of the file.\n"
     "  [string] - Name of the resulting ext images. Default is the name of\n"
     "             the file.\n"
     "\n"
     "Options:\n"
     "  -grhack: Special hack that gather all extrema in the greater-list of\n"
     "     a line. Use it only with ext image with isolated extrema, i.e. an\n"
     "     ext image that only contains the maxima along the contour lines.\n"
     "     In that case you use all the commands that have a \"-vc\" option\n"
     "     (like \"efct\", \"ehisto\", ...)\n"
     "  -swap_bytes :\n"
     "  -ecut (4 integers to specify and rectangle to store, see \"ecut\"\n"
     "         command)\n"
     "\n"
     "Return value:\n"
     "  Name of the resulting ext image.")
  };

  /* Command's parameters */
  char     * extImageFilename = NULL;
  char     * extImageName     = NULL; 
  int      x1, x2, y1, y2;

  /* Options's presence */
  int isGrhack;
  int isSwap = 0;
  int isEcut = 0;

  /* Options's parameters */

  /* Other variables */
  int      result;
  int      tmp;

  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &extImageFilename, &extImageName)==TCL_ERROR)
    return TCL_ERROR;

  isGrhack = arg_present (1);
  isSwap   = arg_present (2);
  isEcut   = arg_present (3);
  if (isEcut) { /*  */
    if (arg_get(3, &x1, &y1, &x2, &y2) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */

  /* Treatement */

  if (!extImageName) {
    extImageName = extImageFilename;
  }

  if (isEcut) {

    result = _LoadExtImage_cut_ (interp, extImageFilename, extImageName, isSwap, isEcut, x1, x2, y1, y2);


  } else {

    result = _LoadExtImage_(interp, extImageFilename, extImageName, isGrhack, isSwap);
  }

  return result;
}


/*------------------------------------------------------------------------
  _LoadExtImage3D_4_Display_
  ----------------------------------------------------------------------*/
static int
_LoadExtImage3D_4_Display_(Tcl_Interp * interp,
			   char * extImageFilename,
			   char * extImageName,
			   int    seek_index,
			   int    isSwap)
{
  FILE     * fileIn;
  char       tempBuffer[100], saveFormat[100], type[100];
  int        lx, ly, lz, i, total_size, realSize, intSize;
  real       scale;
  ExtImage * extImage;
  Extremum * extr;
  int        nb_extr;
  long int   beginning_file, end_file, mean_intervalle, tmp_file, tmp_file2;
  int        tmp_pos;
  float      tmp_mod;

  if (!(fileIn = fopen(extImageFilename, "r")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename, "' for reading.", NULL);
  
  fgets(tempBuffer, 100, fileIn);
  sscanf(tempBuffer, "%s %s %dx%dx%d %d %f (%d byte reals, %d", 
	 saveFormat, type, &lx, &ly, &lz, &total_size, &scale, &realSize, &intSize);
  
  if (strcmp(type, "ExtImage3d")) {
    return GenErrorAppend(interp, "`", extImageFilename,
			  "' doesn't seem to be a 3D ExtImage.", NULL);
  }
  
  
  if (!strcmp(saveFormat, "Ascii"))	{
    return GenErrorAppend(interp, "3D ExtImage ASCII save Format is not curently available. To be implemented.", NULL);
  } else {
    if ((sizeof(real) != realSize) || (sizeof(int) != intSize)) {
      ExtImaDelete_(extImage);
      return GenErrorAppend(interp, "real or Int size problem...", NULL);
    }
  }
  
  if (seek_index < 0 || seek_index > lz-1 ) {
    return GenErrorAppend(interp, "bad value for parameter seek_index, see help", NULL);
  }


  /* 
   * we have now to seek for the beginning of the data to read
   * and to count how much extrema there are in the considered plane
   */

  beginning_file = ftell (fileIn);
  fseek(fileIn, 0, SEEK_END);
  end_file       = ftell (fileIn); 
  fseek(fileIn, beginning_file, SEEK_SET);

  /* compute mean number of extrema by plane */
  mean_intervalle = (end_file-beginning_file)/(sizeof(int)+sizeof(float))/lz;

  /* go arround the beginning of wanted plane */
  fseek(fileIn, seek_index*mean_intervalle*(sizeof(int)+sizeof(float)), SEEK_CUR);


  if (seek_index > 0) { 
    /* seek for the genuine beginning of the plane */
    fread(&tmp_pos, sizeof(int), 1, fileIn);

    if (tmp_pos<seek_index*lx*ly) {
      while (tmp_pos<seek_index*lx*ly) {
	fseek(fileIn, 1*sizeof(float), SEEK_CUR);
	fread(&tmp_pos, sizeof(int), 1, fileIn);
      }
      fseek(fileIn, -1*sizeof(int), SEEK_CUR);
    } else {
      while (tmp_pos>=seek_index*lx*ly) {
	fseek(fileIn, -1*sizeof(float)-2*sizeof(int), SEEK_CUR);
	fread(&tmp_pos, sizeof(int), 1, fileIn);
      }
      fseek(fileIn, 1*sizeof(float), SEEK_CUR);
    }
    
  }
  
  tmp_file = ftell (fileIn);
  tmp_file2 = tmp_file;
  fread(&tmp_pos, sizeof(int),   1, fileIn);
  fread(&tmp_mod, sizeof(float), 1, fileIn);
  nb_extr = 1;
  while (tmp_pos<(seek_index+1)*lx*ly && tmp_file2 <= end_file) {
    fread(&tmp_pos, sizeof(int),   1, fileIn);
    fread(&tmp_mod, sizeof(float), 1, fileIn);
    nb_extr++;
    tmp_file2 += (sizeof(int)+sizeof(float));
  }


  /* memory allocation */
  extImage = w2_ext_new(nb_extr, lx, ly, scale);
  if (!extImage) {
    return GenErrorMemoryAlloc(interp);
  }
  
  /* re-read to fill extimage structure data array */
  extr = extImage->extr;
  fseek(fileIn, tmp_file, SEEK_SET);
  
  for (i=0;i<nb_extr;i++) {
    fread(&(extr[i].pos), sizeof(int),   1, fileIn);
    fread(&(extr[i].mod), sizeof(float), 1, fileIn);
    extr[i].arg = 0.0;
    if (isSwap) {
      BigLittleValues(&(extr[i].pos),1,sizeof(int));
      BigLittleValues(&(extr[i].mod),1,sizeof(float));
    }
    extr[i].pos %= lx*ly;
  }
  
  fclose(fileIn);

  //theExtIma = extImage;
  ExtDicStore(extImageName, extImage);
  Tcl_AppendResult(interp, extImageFilename, NULL);


  return TCL_OK;
}

/*------------------------------------------------------------------------
  Command name in xsmurf : eload3D_display

  ExtFileLoad3DDisplayCmd_

  Lecture d'une extImage3D depuis le disque, dans un but d'affichage,
  c.a.d. on cree une extima prise comme un plan dans le volume 3D de
  donnees.
  
  (rappel: seul le module est specifie, a 3D il faudrait 2 angles
  pour decrire le gradient en coordonnees spheriques par exemple, mais
  j'ai laisser ca de cote pour commencer, sachant que seul le module
  va nous interesser pour les fonctions de partition).

  Les extImage3d sont generees par les routines de Gregoire Malandain
  (INRIA, on peut telechargees ses routines a l'adresse :
  http://www.inria.fr/epidaure/personnel/malandain/)
  ----------------------------------------------------------------------*/
int
ExtFileLoad3DDisplayCmd_(ClientData clientData,
			 Tcl_Interp *interp,
			 int argc,
			 char **argv)      
{ 
  /* Command line definition */
  char * options[] =
  {
    "sd[s]",
    "-swap_bytes", "",
    NULL
  };

  char * help_msg =
  {
    ("  Load a 3D ext image in a classical ext image for display purpose.\n"
     "  Recall that 3D extima are generated from computations made with\n"
     "  Gregoire Malandain's software available at: \n"
     "  http://www.inria.fr/epidaure/personnel/malandain/ \n"
     "\n"
     "Parameters:\n"
     "  string   - Name of the file.\n"
     "  integer  - Index of 2D plane to extract. Default is zero.\n"
     "  [string] - Name of the resulting ext image. Default is the name of\n"
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
  //ExtImage   the_extImage;
  int        seek_index       = 0;
  int        res;
  
  /* Options's presence */
  int        isSwap = 0;

  /* Options's parameters */

  /* Other variables */

  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &extImageFilename, &seek_index, &extImageName)==TCL_ERROR)
    return TCL_ERROR;

  isSwap = arg_present (1);

  /* Parameters validity and initialisation */

  /* Treatement */

  if (!extImageName) {
    extImageName = extImageFilename;
  }

  res = _LoadExtImage3D_4_Display_(interp, extImageFilename, extImageName, seek_index, isSwap);
  /*if (!&the_extImage)
    return GenErrorAppend(interp, "Error in processing `", extImageFilename, "'.", NULL);
  */
  
  //ExtDicStore(extImageName, &the_extImage);
  //Tcl_AppendResult(interp, extImageFilename, NULL);
  
  return TCL_OK;
}


/*
 * command name in xsmurf : ext3ddecimation
 */
/*
 * added on friday, september 20th 2002 
 */
int
ExtFile3DDecimationCmd_(ClientData clientData,
			Tcl_Interp *interp,
			int argc,
			char **argv)      
{ 
  /* Command line definition */
  char * options[] =
  {
    "ss",
    "-swap_bytes", "",
    NULL
  };

  char * help_msg =
  {
    ("  Kind of extraction of local modulus maxima in 3D ext image.\n"
     "  First load it plane by plane, convert it into extima structure,\n"
     "  Then apply hsearch, and save local modulus maxima that survive\n"
     "  this decimation procedure in a file (3d extima)\n"
     "  Recall that 3D extima are generated from computations made with\n"
     "  Gregoire Malandain's software available at: \n"
     "  http://www.inria.fr/epidaure/personnel/malandain/ \n"
     "\n"
     "Parameters:\n"
     "  string   - Name of input  file.\n"
     "  string   - Name of output file.\n"
     "\n"
     "Options:\n"
     "  -swap_bytes :\n"
     "\n"
     "Return value:\n"
     "  Name of the resulting ext image.")
  };

  /* Command's parameters */
  char     * extImageFilenameIN  = NULL;
  char     * extImageFilenameOUT = NULL;
  FILE     * fileIn, * fileOut;
  int        seek_index, lx, ly, lz;
  float      scale;
  ExtImage * extImage;
  int        size_out=0;
  Line     * line;
  Extremum * ext;
  float      tmp_mod;
  int        tmp_pos;

  /* Options's presence */
  int        isSwap = 0;

  /* Options's parameters */

  /* Other variables */

  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &extImageFilenameIN, &extImageFilenameOUT)==TCL_ERROR)
    return TCL_ERROR;

  isSwap = arg_present (1);

  /* Parameters validity and initialisation */

  /* Treatement */

  /* seek for lz */
  {
    char       tempBuffer[100], saveFormat[100], type[100];
    int        total_sizeIN, realSize, intSize;
    
    if (!(fileIn = fopen(extImageFilenameIN, "r")))
      return GenErrorAppend(interp, "Couldn't open `", extImageFilenameIN, "' for reading.", NULL);
    
    fgets(tempBuffer, 100, fileIn);
    sscanf(tempBuffer, "%s %s %dx%dx%d %d %f (%d byte reals, %d", 
	   saveFormat, type, &lx, &ly, &lz, &total_sizeIN, &scale, &realSize, &intSize);
    
    if (strcmp(type, "ExtImage3d")) {
      return GenErrorAppend(interp, "`", extImageFilenameIN,
			    "' doesn't seem to be a 3D ExtImage.", NULL);
    }
    
    if (!strcmp(saveFormat, "Ascii"))	{
      return GenErrorAppend(interp, "3D ExtImage ASCII save Format is not curently available. To be implemented.", NULL);
    } else {
      if ((sizeof(real) != realSize) || (sizeof(int) != intSize)) {
	ExtImaDelete_(extImage);
	return GenErrorAppend(interp, "real or Int size problem...", NULL);
      }
    } 
  }

  /* main loop on seek index */
  for (seek_index=0; seek_index<lz; seek_index++) {
    /*
     * A REVOIR !!!
     * en particulier _LoadExtImage3D_4_Display_
     */


    /*_LoadExtImage3D_4_Display_(interp, extImageFilenameIN, seek_index, isSwap,extImage);
    search_lines (extImage);
    search_single_max (extImage);
    foreach (line, extImage -> line_lst) {
      foreach (ext, line->gr_lst) {
	size_out ++;
      }
    }
    ExtImaDelete_(extImage);*/
  }

    /* second loop on seek index */
  if (!(fileOut = fopen(extImageFilenameOUT, "w")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilenameOUT, "' for writing.", NULL);
  
  fprintf(fileOut,
	  "Binary ExtImage3d %dx%dx%d %d %f"
	  "(%d byte reals, %d byte ints)\n",
	  lx, ly, lz, size_out, scale, (int) sizeof(float),
	  (int) sizeof(int));
  
  for (seek_index=0; seek_index<lz; seek_index++) {
    /*
    _LoadExtImage3D_4_Display_(interp, extImageFilenameIN, seek_index, isSwap,extImage);
    search_lines (extImage);
    search_single_max (extImage);
    foreach (line, extImage -> line_lst) {
      foreach (ext, line->gr_lst) {
	tmp_pos = ext->pos + seek_index*lx*ly;
	tmp_mod = ext->mod;
	fwrite(&tmp_pos,  sizeof(int),  1, fileOut);
	fwrite(&tmp_mod, sizeof(float), 1, fileOut);
      }
    }
    ExtImaDelete_(extImage);*/
  }
  
  fclose(fileOut);
 
  return TCL_OK;
}




point
_addSommet_(int pos, int lx, real z)
{
  point sommet;

  sommet.x = pos % lx;
  sommet.y = pos / lx;
  sommet.z = z;

  return sommet;
}

/*------------------------------------------------------------------------
  _SaveChainToVertexFile_
  ----------------------------------------------------------------------*/
int 
_SaveChainToVertexFile_(Tcl_Interp * interp,
			ExtImage * extImage,
			char * extImageFilename)
{
  FILE     * fileOut;
  int      i, j;
  int      lx, size, step = 6;
  real    scale;
  Extremum *extr;

  if (!(fileOut = fopen(extImageFilename, "a")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for writing.", NULL);

  lx    = extImage->lx;
  scale = extImage->scale;
  size  = extImage->chainNb;

  for (i = 0;i < size; i++) {
    extr = extImage->chain[i];
    do
      if(extr->down){
	Extremum *tmp = extr->next;
	point    sommet[100];
	int      nb = 0;

	sommet[nb] = _addSommet_(extr->pos, lx, scale);
	nb++;
	while(tmp &&
	      (tmp != extImage->chain[i]) &&
	      !tmp->down) {
	  sommet[nb] = _addSommet_(tmp->pos, lx, scale);
	  nb++;
	  tmp = tmp->next;
	} 
	if(tmp && tmp->down){
	  Extremum *tmpdown = tmp->down;

	  sommet[nb] = _addSommet_(tmp->pos, lx, scale);
	  nb++;
	  do {
	    sommet[nb] = _addSommet_(tmpdown->pos, lx, scale-step);
	    nb++;
	    tmpdown = tmpdown->prev;
	  } while ((nb<100) &&
		   (tmpdown) &&
		   (tmpdown != extr->down) &&
		   (tmpdown != tmp->down)     );
	  if (tmpdown && (tmpdown == extr->down)){
	    sommet[nb] = _addSommet_(tmpdown->pos, lx, scale-step);
	    nb++;
	    for(j=0;j<nb;j++) 
	      fprintf(fileOut,"v %d %d %f\n",
		      sommet[j].x, sommet[j].y, sommet[j].z);
	    fprintf(fileOut, "p\n");
	  }
	}
	extr = tmp;
      }
      else 
	extr = extr->next;
    while(extr && (extr != extImage->chain[i]));
  }

  fclose(fileOut);

  return TCL_OK;
}

/*------------------------------------------------------------------------
  _SaveChainFile_

  Sauvegarde du chainage sous la forme d'un 'squelette'.
  ----------------------------------------------------------------------*/
int 
_SaveChainFile_ (Tcl_Interp * interp,ExtImage * extImage,
			 char * extImageFilename)
{
  FILE     * fileOut, * ftmp;
  int      i, j ;
  int      lx, size, step = 6;
  real    scale;
  Extremum *extr;
  int      NPolylines = extImage->chainNb;
  int      NVertices = 0;
  int      *Nv, *Nc;
  char     c;

  if (!(fileOut = fopen(extImageFilename, "a")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for writing.", NULL);
  if (!(ftmp = fopen(".tmp", "w")))
    return GenErrorAppend(interp, "Couldn't open `", ".tmp",
			  "' for writing.", NULL);

  lx    = extImage->lx;
  scale = extImage->scale;
  size  = extImage->chainNb;

  for (i = 0;i < size; i++) {
    extr = extImage->chain[i];
    do {
      if(extr->down){
	NPolylines++;
	NVertices += 2;
      }
      extr = extr->next;
    }
    while(extr && (extr != extImage->chain[i]));
  }

  Nv = (int *) malloc (sizeof(int)*NPolylines);
  Nc = (int *) malloc (sizeof(int)*NPolylines);

  for(i=0;i<NPolylines;i++)
    Nc[i] = 0;

  for (i = 0;i < size; i++) {
    extr = extImage->chain[i];
    Nv[i] = 0;
    do {
      fprintf(ftmp, "%d %d %f\n", extr->pos%lx, extr->pos/lx, scale);
      extr = extr->next;
      Nv[i]++;
      NVertices++;
    }
    while(extr && (extr != extImage->chain[i]));
  }

  for (j = 0;j < size; j++) {
    extr = extImage->chain[j];
    do {
      if(extr->down) {
	fprintf(ftmp, "%d %d %f\n"
		        "%d %d %f\n",
		extr->pos%lx, extr->pos/lx, scale,
		extr->down->pos%lx, extr->down->pos/lx, scale-step);
	Nv[i] = 2;
	i++;
      }
      extr = extr->next;
    }
    while(extr && (extr != extImage->chain[j]));
  }
  fprintf(fileOut, "VECT\n");
  fprintf(fileOut, "%d %d 0\n",  NPolylines, NVertices);
  for(i=0;i<NPolylines;i++)
    fprintf(fileOut, "%d ", Nv[i]);
  fprintf(fileOut, "\n");
  for(i=0;i<NPolylines;i++)
    fprintf(fileOut, "%d ", Nc[i]);
  fprintf(fileOut, "\n");
  fclose(ftmp);
  if (!(ftmp = fopen(".tmp", "r")))
    return GenErrorAppend(interp, "Couldn't open `", ".tmp",
			  "' for writing.", NULL);
  while(fscanf(ftmp,"%c", &c) != EOF)
    fprintf(fileOut, "%c", c);

  fclose(fileOut);
  fclose(ftmp);
  system("rm -f .tmp");

  free(Nv);
  free(Nc);
  return TCL_OK;
}

/************************************************************************
 *  Command name in xsmurf : save3d
 *
 * ExtFileChainSaveCmd_
 *
 * Sauvegarde du chainage sur disque sous forme de surface a facettes.
 ************************************************************************/
int
ExtFileChainSaveCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{
  char * options[] = { "E[s]",
			 "-s", "",
			 NULL};
  
  char  * extImageFilename = NULL;
  ExtImage * extImage;
  
  if (arg_init(interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get(0, &extImage, &extImageFilename)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_present(1))
    return _SaveChainFile_(interp,  extImage,  extImageFilename);
  
  if (!extImageFilename)
    extImageFilename = argv[1];
  
  return _SaveChainToVertexFile_(interp, extImage, extImageFilename);
}

/************************************ 
 * command name in xsmurf : hsearch
 ************************************/
int
search_lines_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{
  char * options[] = { "E",
		       NULL };

  char * help_msg = 
  {("Horizontal search in extimage.\n"
    "see search_lines_TclCmd_ in interpreter/wt2d_cmds.c\n"
    "or more preciselly in search_lines defined in wt2d/chain.c\n"
    "\n"
    "This command is used in chain defined in tcl_library/eiChain.tcl\n")};

  ExtImage * ext_image;
  Extremum *ext, *extNext;
  Line *l;
  real meanradius;
  real meansquareradius;
  real xcenter, ycenter;
  real radius;
  real zex, zey;
  real bend;
  real area = 0.0;
  real diam = 0.0;
  int  lx,ly;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &ext_image) == TCL_ERROR)
    return TCL_ERROR;

  lx = ext_image->lx;
  ly = ext_image->ly;

  smSetBeginTime();
  /* search_lines is defined in wt2d/chain.c
     in particular, this computes
     ext_image->line_lst and ext_image->nb_of_lines*/
  search_lines (ext_image);

  /* on remplit le champ line de tous les extrema se
     trouvant sur une ligne donnee */
  foreach (l, ext_image->line_lst) {
    foreach (ext, l->ext_lst) {
      ext->line = (void *) l;
    }
  }

  /*
   * added on may 2002 
   * fill line->meanradius, line->meansquareradius fields 
   *  and line->bend !!!
   */
  foreach (l, ext_image->line_lst) {
    meanradius = 0.0;
    meansquareradius = 0.0;
    xcenter = l->gravity_center_x;
    ycenter = l->gravity_center_y;

    foreach (ext, l->ext_lst) {
      zex = ext->pos%lx;
      zey = ext->pos/lx;
      radius = (float) sqrt((xcenter-zex)*(xcenter-zex)+
			    (ycenter-zey)*(ycenter-zey));
      meanradius += radius;
      meansquareradius += radius*radius;
      bend = ext->mod*cos(ext->arg)*(xcenter-zex)+
	ext->mod*sin(ext->arg)*(ycenter-zey);
      bend /= ext->mod;
      bend /= sqrt((xcenter-zex)*(xcenter-zex)+(ycenter-zey)*(ycenter-zey));
      ext->bend = bend;
    }
  
    /* 
     * compute area for closed lines
     * dec 2 2005
     * TODO : add code for computing diameter
     */
    area = 0.0;
    if (l->type == LINE_CLOSED) {
      real Dtemp;

      foreach (ext, l->ext_lst) {

	/* get next extremum */
	extNext = lst_get_next(l->ext_lst);
	/* if NULL that means that next extremum is first (closed loop) */
	if (extNext == NULL)
	  extNext = lst_get_first(l->ext_lst);
	/* update area */
	area += ext->pos%lx * extNext->pos/lx;      //x[i]*y[i+1];
	area -= extNext->pos%lx * ext->pos/lx;      //x[i+1]*y[i];
	
	//printf("%d %d %d %d\n", ext->pos%lx,  ext->pos/lx, extNext->pos%lx, extNext->pos/lx);
	
      } /* end foreach loop */
      
      area = fabs(area)/2.0;

    }/* end if (l->type == LINE_CLOSED) */
      
    meanradius /= l->size;
    meansquareradius /= l->size;
    l->meanradius = meanradius;
    l->meansquareradius = meansquareradius;
    l->area = area;
    
  /*l = lst_first(ext_image->line_lst);
    ext = lst_first(l->ext_lst);
    l = ext->line;*/

  } /* foreach (l, ext_image->line_lst) */
  
  
  smSetEndTime();

  //printf("\n\n\n\n\n");
  
  sprintf(interp->result, "%f", smGetEllapseTime());

  return TCL_OK;
}

/************************************ 
 * command name in xsmurf : hsearch_with_diam
 ************************************/
int
search_lines_with_diam_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{
    char * options[] = { "E",
			 NULL };

    char * help_msg = 
	{("Horizontal search in extimage.\n"
	  "see search_lines_TclCmd_ in interpreter/wt2d_cmds.c\n"
	  "or more preciselly in search_lines defined in wt2d/chain.c\n"
	  "\n"
	  "This command is used in chain defined in tcl_library/eiChain.tcl\n")};

    ExtImage * ext_image;
    Extremum *ext, *extNext;
    Line *l;
    real meanradius;
    real meansquareradius;
    real xcenter, ycenter;
    real radius;
    real zex, zey;
    real bend;
    real area = 0.0;
    real diameter = 0.0;
    int perimeter = 0;
    int  lx,ly;
    real Dtemp;
    int x[50000], y[50000];
    int i, j=0;

    if (arg_init(interp, argc, argv, options, help_msg))
	return TCL_OK;

    if (arg_get(0, &ext_image) == TCL_ERROR)
	return TCL_ERROR;

    lx = ext_image->lx;
    ly = ext_image->ly;

    smSetBeginTime();
    /* search_lines is defined in wt2d/chain.c
       in particular, this computes
       ext_image->line_lst and ext_image->nb_of_lines*/
    search_lines (ext_image);

    /* on remplit le champ line de tous les extrema se
       trouvant sur une ligne donnee */
    foreach (l, ext_image->line_lst) {
	foreach (ext, l->ext_lst) {
	    ext->line = (void *) l;
	}
    }

    /*
     * added on may 2002 
     * fill line->meanradius, line->meansquareradius fields 
     *  and line->bend !!!
     */
    foreach (l, ext_image->line_lst) {
	meanradius = 0.0;
	meansquareradius = 0.0;
	xcenter = l->gravity_center_x;
	ycenter = l->gravity_center_y;

	foreach (ext, l->ext_lst) {
	    zex = ext->pos%lx;
	    zey = ext->pos/lx;
	    radius = (float) sqrt((xcenter-zex)*(xcenter-zex)+
				  (ycenter-zey)*(ycenter-zey));
	    meanradius += radius;
	    meansquareradius += radius*radius;
	    bend = ext->mod*cos(ext->arg)*(xcenter-zex)+
		ext->mod*sin(ext->arg)*(ycenter-zey);
	    bend /= ext->mod;
	    bend /= sqrt((xcenter-zex)*(xcenter-zex)+(ycenter-zey)*(ycenter-zey));
	    ext->bend = bend;

	}
  
	/* 
	 * compute area for closed lines
	 * dec 2 2005
	 *
       	 * compute diameter and perimeter for closed lines
	 * 2007 08 10
	 *
	 * A. Khalil
	 *
	 * Need to check that for closed lines, line->size and
	 * line->perimeter is the same. (PK 2008 09 16).
	 */
	if (l->type == LINE_CLOSED) {

	    area = 0.0;
	    diameter = 0.0;
	    perimeter = 0;
	    j=0;
	    
	    foreach (ext, l->ext_lst) {
		
		/* get next extremum */
		extNext = lst_get_next(l->ext_lst);
		/* if NULL that means that next extremum is first (closed loop) */
		if (extNext == NULL)
		    extNext = lst_get_first(l->ext_lst);
		
		x[j] = ext->pos%lx;
		y[j] = ext->pos/lx;
		j++;
		perimeter++;

		/* update area */
		area += ext->pos%lx * extNext->pos/lx;      //x[i]*y[i+1];
		area -= extNext->pos%lx * ext->pos/lx;      //x[i+1]*y[i];	
	    
		//printf("%d %d %d %d\n", ext->pos%lx,  ext->pos/lx, x[j], y[j]);
	    
		for(i=0;i<perimeter;i++){
		    //area = area + x[i]*y[i+1];
		    //area = area - x[i+1]*y[i];
		    //cx = cx + (x[i] + x[i+1])*(x[i]*y[i+1] - x[i+1]*y[i]);
		    //cy = cy + (y[i] + y[i+1])*(x[i]*y[i+1] - x[i+1]*y[i]);
		    Dtemp = 0;
		    for(j=i;j<perimeter;j++){
			Dtemp = sqrt((y[j]-y[i])*(y[j]-y[i]) + (x[j]-x[i])*(x[j]-x[i]));
			if (Dtemp > diameter){
			    diameter = Dtemp;
			}
		    }
		}		
		
	    } /* end foreach loop */
	    
	    area = fabs(area)/2.0;
	    
	}/* end if (l->type == LINE_CLOSED) */
	
	
	meanradius /= l->size;
	meansquareradius /= l->size;
	l->meanradius = meanradius;
	l->meansquareradius = meansquareradius;
	l->area = area;
	l->diameter = diameter;
 	l->perimeter = perimeter;
   
	/*l = lst_first(ext_image->line_lst);
	  ext = lst_first(l->ext_lst);
	  l = ext->line;*/
    
    } /* foreach (l, ext_image->line_lst) */
  
  
    smSetEndTime();
  
    sprintf(interp->result, "%f", smGetEllapseTime());
  
    return TCL_OK;
}


/************************************* 
 * Command name in xsmurf : eibending
 *************************************/
int
compute_bending_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{
  char * options[] = { "E",
		       NULL };
  
  char * help_msg = 
  {("Compute parameter of discrete tangent\n"
    "\n"
    "Note that you have to use this after hsearch command\n"
    "(Extrema must have been chained!!)\n"
    "\n")};
  
  ExtImage * ext_image;

  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &ext_image) == TCL_ERROR)
    return TCL_ERROR;

  smSetBeginTime();
  /* compute_discrete_tangent is defined in wt2d/chain.c
     in particular, this computes
     extremun->a
     extremun->b
     extremun->mu
     extremun->octant
  */


  /*foreach (l, ext_image->line_lst) {
    if (l->size < 3) { 
      foreach (ext, l->ext_lst) {
	ext->a =0;
	ext->b =0;
	ext->xe =0.0;
	ext->ye =0.0;
	ext->octant = -1;
      }
    } else {  
      compute_discrete_tangent (ext_image);
    }
  }
  */
  compute_discrete_tangent (ext_image);

  //compute_discrete_bending (ext_image);
  
  smSetEndTime();
  
  sprintf(interp->result, "%f", smGetEllapseTime());
  
  return TCL_OK;
}


/****************************************
 * command name in xsmurf : l2c
 *
 * an old function ?!?
 ****************************************/
int
lines_to_chains_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{
  char * options[] = { "E",
			 NULL };
  ExtImage * ext_image;

  if (arg_init(interp, argc, argv, options, NULL))
    return TCL_OK;

  if (arg_get(0, &ext_image) == TCL_ERROR)
    return TCL_ERROR;

  lines_to_chains (ext_image);

  return TCL_OK;
}

/**************************************
 *  Command name in xsmurf : vchainold
 **************************************/
int
vertical_chain_TclCmd_(clientData, interp, argc, argv)      
     ClientData clientData;
     Tcl_Interp *interp;
     int        argc;
     char       **argv;
{
  char * options[] = { "L",
			 NULL };
  char     ** expr;
  ExtImage **ext_image_tab;
  Skeleton *skeleton;
  int      lx, ly, i;
  int      nb_of_ext_image;

  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;

  if (arg_get (0, &expr) == TCL_ERROR)
    return TCL_ERROR;
  
  if (ExtDicImageListGet (interp, expr, &ext_image_tab) == TCL_ERROR)
    return TCL_ERROR;
  
  if (!ext_image_tab)
    return GenErrorAppend(interp, "No matching Extimage.", NULL);
  
  /* on verifie que toutes les images ont les memes dimensions */
  lx = ext_image_tab[0]->lx;
  ly = ext_image_tab[0]->ly; 
  for (i = 1; ext_image_tab[i]; i++)
    if ((ext_image_tab[i]->lx != lx) || (ext_image_tab[i]->ly != ly))
      {
	GenErrorAppend (interp, "Specified ExtImages don't have the same size",
			NULL);
	free (ext_image_tab);
	return TCL_ERROR;
      }
  
  /* on trie la liste par ordre croissant d'echelle */
  ExtMisSortImageList_ (ext_image_tab); 
  
  skeleton = create_skeleton ();
  for (i = 0; ext_image_tab[i]; i++);
  nb_of_ext_image = i;
  for (i = nb_of_ext_image - 1; i >= 0; i--)
    add_ext_image_to_skeleton (ext_image_tab[i], skeleton);

  construct_skeleton (skeleton);
  /*  display_skeleton (skeleton);*/
  skeleton_to_chains (skeleton);

  return TCL_OK;
}

/* encore une des fonctions les plus crades de l'histoire de la
 *programmation en c */

enum {CARAC, DIV, EDGE};

/*----------------------------------------------------------------------
  ExtExtComputeCmd_  
  
  Cree une structure ExtImage a partir de deux Image representant
  le module et l'argument du gradient d'une image...
  --------------------------------------------------------------------*/
int ExtExtComputeCmd_(ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)      
{
  char * options[] = { "sIIf",
		       "-thresh", "f",
		       "-rthresh","f",
		       "-first",  "",
		       "-eps", "f",
		       "-kapap", "IIf",
		       "-niv", "ff",
		       "-min", "",
		       "-kapa", "If",
		       "-kmax", "IIf",
		       "-kmin", "IIf",
		       "-contour", "IId",
		       NULL };
  
  char * help_msg =
  {("Extract some points from gradient field.\n"
    "(result name, gradient modulus image, gradient argument image, scale)\n"
    "   -thresh  :"
    "   -rthresh :"
    "   -first   :"
    "   -eps     :"
    "   -kapap   :"
    "   -niv"
    "   -min"
    "   -kapa"
    "   -kmax"
    "   -kmin\n"
    "   -contour : (kapa image, kapa prime image, flag)\n"
    "      Keep points where kapa is zero and where kapap is the opposite\n"
    "      sign of flag. So positive flag is for maxima of the gradient, and\n"
    "      negative flag for minima.")};
  
  Image    * modulImage, * argImage;
  Image    * kapaImage, * kapapImage;
  ExtImage * extImage;
  Extremum * extremum;
  char     * extImageName;
  real      scale, thresh, rthresh;
  real      min, max;
  int        fLevel, lx, ly, k;
  real      eps=0.0;
  int       is_kapap = 0, is_niv = 0, is_min = 0, is_kapa = 0;
  int       is_kmin = 0, is_kmax = 0, is_contour = 0;
  real kapa_epsilon, kapap_epsilon;
  real mult, niv;
  int       min_or_max;
  int       _maximaNumber_;

  if (arg_init(interp,argc,argv,options, help_msg))
    return TCL_OK;
  
  if (arg_get(0,&extImageName,&modulImage,&argImage,&scale)==TCL_ERROR)
    return TCL_ERROR;
  
  /* fLevel : pour une operation normalement appliquee aux petites echelles */
  fLevel = arg_present(3);

  is_kapap = arg_present(5);
  if (arg_get(5, &kapaImage, &kapapImage, &mult)==TCL_ERROR)
    return TCL_ERROR;

  is_niv = arg_present(6);
  if (arg_get(6,&niv, &eps)==TCL_ERROR)
    return TCL_ERROR;
  
  is_min = arg_present(7);
  
  is_kapa = arg_present(8);
  if (arg_get(8, &kapaImage, &mult)==TCL_ERROR)
    return TCL_ERROR;

  is_kmin = arg_present(9);
  if (arg_get(9, &kapaImage, &kapapImage, &mult)==TCL_ERROR)
    return TCL_ERROR;

  is_kmax = arg_present(10);
  if (arg_get(5, &kapaImage, &kapapImage, &mult)==TCL_ERROR)
    return TCL_ERROR;

  is_contour = arg_present(11);
  if (arg_get(11, &kapaImage, &kapapImage, &min_or_max)==TCL_ERROR)
    return TCL_ERROR;

  lx = modulImage->lx;
  ly = modulImage->ly;
  
  /* verifier que les dimensions des 2 images sont les memes */
  if ((argImage->lx!=lx) || (argImage->ly!=ly))
    return GenErrorAppend(interp,"Specified Images don't have the same size.",
			  NULL);

  /* par defaut valeurs negatives pour thresh et rthresh : pas de seuil */
  thresh  = rthresh= -1;

  /* recuperation des valeurs des seuils */
  if (arg_get(1,&thresh)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(4,&eps)==TCL_ERROR)
    return TCL_ERROR;


  if (arg_present(2))
    {
      if (arg_get(2,&rthresh)==TCL_ERROR)
	return TCL_ERROR;
      
      if ((rthresh<0.0) || (rthresh>100.0))
	return GenErrorAppend(interp,"Bad value for rthresh: 0<rthresh<100",
			      NULL);
    }
  
  /* calcul des maxima */
  im_get_extrema (modulImage, &min, &max);
  _InitMaximaTable_(lx*ly);
  if (is_niv)
    {
      im_get_extrema (modulImage, &min, &max);
      _maximaNumber_ = _FindNiv_(modulImage->data,
			lx,ly, min, max, eps,niv);
    }
  else if (is_min)
    {
      _maximaNumber_ = _FindExtrema2_(modulImage->data,argImage->data,
		    lx,ly,fLevel,thresh,rthresh, min, max, eps);
    }
  else if(is_kapap)
    {
      im_get_extrema (kapapImage, &min, &max);
      kapap_epsilon = (max-min)*eps;
      im_get_extrema (kapaImage, &min, &max);
      _maximaNumber_ = _FindMaximaKapap_(kapaImage->data, kapapImage->data, modulImage->data,
			lx,ly, min, max, kapap_epsilon,mult);
    }
  else if(is_kapa)
    {
      im_get_extrema (kapaImage, &min, &max);
      kapa_epsilon = (max-min)*eps;
      _maximaNumber_ = _FindExtremaKapa_(kapaImage->data, modulImage->data,
			lx,ly, min, max, kapa_epsilon,mult);
    }
  else if(is_kmin)
    {
      im_get_extrema (kapaImage, &min, &max);
      kapa_epsilon = (max-min)*eps;
      _maximaNumber_ = _FindMinimaKapap_(kapaImage->data, kapapImage->data, modulImage->data,
			lx,ly, min, max, kapap_epsilon,mult);
    }
  else if(is_kmax)
    {
      im_get_extrema (kapaImage, &min, &max);
      kapa_epsilon = (max-min)*eps;
      _maximaNumber_ = _FindMaximaKapap_(kapaImage->data, kapapImage->data, modulImage->data,
			lx,ly, min, max, kapap_epsilon,mult);
    }
  else if(is_contour)
    {
      im_get_extrema (kapaImage, &min, &max);
      _maximaNumber_ = _FindContourLine_(kapaImage->data, kapapImage->data, lx, ly, min_or_max);
    }
  else
    _maximaNumber_ = _FindExtrema_(modulImage->data,argImage->data,
		  lx,ly,fLevel,thresh,rthresh, min, max, eps);
/*  _FindExtremaII_(modulImage->data,argImage->data,lx,ly,fLevel,thresh,rthresh);*/
  
  /* creation de la ExtImage (la aussi, verifier les allocations, etc...) */
  extImage = w2_ext_new (_maximaNumber_,lx,ly,scale);
  if (!extImage)
    return GenErrorMemoryAlloc(interp);
  
  /* mise a jour de la structure ExtImage */
  extremum = extImage->extr;
  for (k=0;k<lx*ly;k++)
    if (_IsMaximum_(k))
      {
	extremum->mod = modulImage-> data[k];
	extremum->arg = argImage ->data[k];
	extremum->pos = k;
	extremum++;
      }  
  /* ajout de la structure ExtImage au dictionnaire adequat */
  ExtDicStore(extImageName,extImage);
  
  _DeleteMaximaTable_();
  
  return TCL_OK;
}

/*************************************
 *  Command name in xsmurf : locmax
 *************************************/
int
w2_local_maxima_TclCmd_(ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)      
{
  char * options[] =
  {
    "Is[f]",
    "-ext", "",
    "-thresh", "f",
    NULL
  };
  
  char * help_msg =
  {("Extract the local maxima from an image.\n"
    "Options :\n"
    "    -ext    :\n"
    "    -thresh : extract only maxima that are above a thresold\n"
    "pas encore implante!!\n")};
  
  Image    *image;
  ExtImage *extImage;
  Extremum *extremum;
  char     *extImageName;
  int      lx, ly, k;
  int      nb_of_maxima;
  real     scale = -1;
  real     thresh;

  int is_ext;
  int is_thresh;

  if (arg_init(interp,argc,argv,options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &image, &extImageName, &scale)==TCL_ERROR)
    return TCL_ERROR;

  is_ext = arg_present(1);
  is_thresh = arg_present(2);
  if (is_thresh) {
    if (arg_get(2, &thresh) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  
  lx = image->lx;
  ly = image->ly;
  
  _InitMaximaTable_(lx*ly);
  if (is_ext) {
    nb_of_maxima = w2_local_extrema (image->data, lx, ly);
  }
  else {
    nb_of_maxima = w2_local_maxima (image->data, lx, ly);
  }
  
  /* creation de la ExtImage (la aussi, verifier les allocations, etc...) */
  extImage = w2_ext_new(nb_of_maxima,lx,ly,scale);
  if (!extImage)
    return GenErrorMemoryAlloc(interp);
  
  /* mise a jour de la structure ExtImage */
  extremum = extImage->extr;
  for (k=0;k<lx*ly;k++)
    if (_IsMaximum_(k))
      {
	extremum->mod = image-> data[k];
	extremum->arg = 666.0;
	extremum->pos = k;
	extremum++;
      }  
  /* ajout de la structure ExtImage au dictionnaire adequat */
  ExtDicStore(extImageName,extImage);
  
  _DeleteMaximaTable_();
  
  return TCL_OK;
}


/*************************************
 *  Command name in xsmurf : sslocmax
 *************************************/
int
w2_local_space_scale_maxima_TclCmd_(ClientData clientData,
				    Tcl_Interp *interp,
				    int        argc,
				    char       **argv)
{
  /* Command line definition */
  char * options[] =
  {
    "IIIsff",
    "-box", "dddd",
    NULL
  };
  
  char * help_msg =
  {
    ("Extract the space-scale local maxima of one scale.\n"
     "Parameters :\n"
     "  image  - corresponding to the scale.\n"
     "  image  - corresponding to the previous scale.\n"
     "  image  - corresponding to the next scale.\n"
     "  string - the name of the resulting ext_image.\n"
     "  real   - value of the scale.\n"
     "  real   - ratio; search only the values over <<ratio * max_value>>.\n"
     "Options :\n"
     "  -box : search only value inside a box.\n"
     "    4 integers - box coordinates.")
  };

  /* Command's parameters */
  Image *image;
  Image *prev_image;
  Image *next_image;
  char  *name;
  real  scale;
  real  ratio;

  /* Options's presence */
  int is_box;

  /* Options's parameters */
  int x_min, x_max;
  int y_min, y_max;

  /* Other variables */
  ExtImage *ext_image;
  Extremum *extremum;
  int      lx, ly, k;
  int      nb_of_maxima;
  real     min_value;
  real     min, max;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &image, &prev_image, &next_image,
	       &name, &scale, &ratio)==TCL_ERROR)
    return TCL_ERROR;

  is_box = arg_present(1);
  if (is_box)
    if (arg_get (1, &x_min, &y_min, &x_max, &y_max)==TCL_ERROR)
      return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (ratio < 0 || ratio > 1)
    {
      sprintf (interp->result,
	       "The ratio (%f) must be between 0 and 1.", ratio);
      return TCL_ERROR;
    }

  lx = image->lx;
  ly = image->ly;

  if (is_box)
    {
      if (x_min < 1)      x_min = 1;
      if (x_max > lx - 2) x_max = lx - 2;
      if (y_min < 1)      y_min = 1;
      if (y_max < lx - 2) y_max = ly - 2;
    }
  else
    {
      x_min = 1;
      x_max = lx - 2;
      y_min = 1;
      y_max = ly - 2;
    }

  /* Treatement */
  im_get_extrema (image, &min, &max);

  min_value = ratio*max;

  _InitMaximaTable_(lx*ly);
  nb_of_maxima = w2_local_space_scale_maxima (image->data,
					      prev_image->data,
					      next_image->data,
					      min_value,
					      lx, ly,
					      x_min, y_min,
					      x_max, y_max);
  
  ext_image = w2_ext_new (nb_of_maxima, lx, ly, scale);
  if (!ext_image)
    return GenErrorMemoryAlloc (interp);
  
  extremum = ext_image->extr;
  for (k = 0; k < lx * ly; k++)
    if (_IsMaximum_(k))
      {
	/* pour l'instant on utilise le champs mod pour scale et le */
	/* champs arg pour la valeur de l'image.                    */
	/*  A REVOIR ! */
	extremum->mod = scale;
	extremum->arg = image->data[k];
	extremum->pos = k;
	extremum++;
      }  

  ExtDicStore (name, ext_image);
  
  _DeleteMaximaTable_ ();
  
  return TCL_OK;
}


/*----------------------------------------------------------------------
  ExtExtCaracCmd_  
  
  Cree une structure ExtImage a partir de plein d'Images representant
  les convolutions d'une image avec les derivees successives de la
  gaussienne.
  --------------------------------------------------------------------*/
int
ExtExtCaracCmd_(ClientData clientData,
		Tcl_Interp *interp,
		int        argc,
		char       **argv)      
{
  char * options[] = { "sIIIIIIf",
			 "-thresh", "f",
			 "-rthresh","f",
			 "-first",  "",
			 "-div",  "",
			 "-edge",  "",
			 NULL };
  
  Image    * caracImage;
  Image    * ddxImage, * ddyImage;
  Image    * d2dxdxImage, * d2dydyImage, * d2dxdyImage;
  ExtImage * extImage;
  Extremum * extremum;
  int      line = CARAC;
  char     * extImageName;
  real      scale, thresh=0.01, rthresh=0.01;
  int        fLevel, lx, ly, k;
  int       _maximaNumber_;
  
  if (arg_init(interp,argc,argv,options,NULL))
    return TCL_OK;
  
  if (arg_get(0, &extImageName, &caracImage,
		 &ddxImage, &ddyImage,
		 &d2dxdxImage, &d2dydyImage, &d2dxdyImage,
		 &scale)==TCL_ERROR)
    return TCL_ERROR;
  
  /* fLevel : pour une operation normalement appliquee aux petites echelles */
  fLevel = arg_present(3);
  
  lx = ddxImage->lx;
  ly = ddxImage->ly;
  
  /* A CHANGER !!!!!!!! */
  /* !!!!!!!!!!!!!!!!!!!*/
  /* verifier que les dimensions des 2 images sont les memes */
/*  if ((argImage->lx!=lx) || (argImage->ly!=ly))
    return GenErrorAppend(interp,"Specified Images don't have the same size.",
			  NULL);
*/
  /* par defaut valeurs negatives pour thresh et rthresh : pas de seuil */
  thresh  = rthresh= -1;

  /* recuperation des valeurs des seuils */
  if (arg_get(1,&thresh)==TCL_ERROR)
    return TCL_ERROR;

  if (arg_present(4))
    line = DIV;

  if (arg_present(5))
    line = EDGE;

  if (arg_present(2))
    {
      if (arg_get(2,&rthresh)==TCL_ERROR)
	return TCL_ERROR;
      
/*      if ((rthresh<0.0) || (rthresh>100.0))
	return GenErrorAppend(interp,"Bad value for rthresh: 0<rthresh<100",
			      NULL);*/
    }
  
/*  caracImage = ImaImaNew_(lx,ly,ddxImage->size,ddxImage->type);*/

  /* calcul des maxima */
 
  _InitMaximaTable_(lx*ly);
  _maximaNumber_ = _FindCarac_(ddxImage->data, ddyImage->data,
	      d2dxdxImage->data, d2dydyImage->data, d2dxdyImage->data,
	      caracImage->data, line,
	      lx, ly, fLevel, thresh, rthresh);
/*  _FindExtremaII_(modulImage->data,argImage->data,lx,ly,fLevel,thresh,rthresh);*/
  
  /* creation de la ExtImage (la aussi, verifier les allocations, etc...) */
  extImage = w2_ext_new(_maximaNumber_,lx,ly,scale);
  if (!extImage)
    return GenErrorMemoryAlloc(interp);
  
  /* mise a jour de la structure ExtImage */
  extremum = extImage->extr;
  for (k=0;k<lx*ly;k++)
    if (_IsMaximum_(k))
      {
	extremum->mod = 1; /*modulImage-> data[k];*/
	extremum->arg = 0; /*argImage ->data[k];*/
	extremum->pos = k;
	extremum++;
      }  
  /* ajout de la structure ExtImage au dictionnaire adequat */
  ExtDicStore(argv[1],extImage);
  
  _DeleteMaximaTable_();
  
  return TCL_OK;
}

/* un peu crade aussi.... */

/****************************************************************************/
/*                                                                          */
/*    fqaqtq.c    compute the f(alpha)                                      */
/*                                                                          */
/****************************************************************************/


#define NO 0
#define YES 1

#define NQ 300   /**** maximum number of qs ****/
#define mylog(x)   ((x) <= 0 ? -999999 : (log(x)))
#define mypow(x,alp)   (exp(alp*mylog(x)))

#define Q_FILE_NAME "/user3/decoster/Work/dev/xsmurf/travail/qval"

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

/*----------------------------------------------------------------------
  _QuickSort_Extr_
  
  Fonction qui classe un tableau d'Extremum dans l'ordre croissant (suivant
  le champs pos (entier) de cette structure extremum)
  le tableau est de taille n.
  --------------------------------------------------------------------*/
/*static void 
_QuickSort_Extr_(Extremum *ext,
		 int    n)
{
  int    l, j, ir, i;
  Extremum extt;
  Extremum extj, extj1;

  l = (n>>1) + 1;
  ir = n;

  for(;;) {
    if(l > 1)
      extt = ext[--l];
    else {
      extt = ext[ir];
      ext[ir] = ext[1];
      if(--ir == 1) {
	ext[1] = extt;
	return;
      }
    }
    i = l;
     j = l<<1;
    while(j <= ir) {
      extj  = ext[j];
      extj1 = ext[j+1];
      if(j < ir && extj.pos < extj1.pos)
	++j;
      extj = ext[j];
      if(extt.pos < extj.pos) {
	ext[i] = ext[j];
	j += (i = j);
      }
      else j = ir + 1;
    }
    ext[i] = extt;
  }
}
*/


#define _add_(a) ((a)>0?(pos=pos+(a)):(neg=neg+(a)))


/*************************************
 *  Command name in xsmurf : gkapa
 *************************************/
int
gkapa_TclCmd_ (clientData, interp, argc, argv)      
     ClientData clientData;
     Tcl_Interp *interp;
     int        argc;
     char       **argv;
{ 
  char * options[] = { "sIIIII",
			 NULL};
  int   lx,ly,i;
  char  *imageName;
  real  *result, *dx, *dxx, *dy, *dyy, *dxy;
  Image *result_im, *dx_im, *dxx_im, *dy_im, *dyy_im, *dxy_im;
  double pos = 0, neg = 0;
  
  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &dx_im, &dxx_im, &dy_im, &dyy_im, &dxy_im) == TCL_ERROR)
    return TCL_ERROR;

  /* To do : control the images size. */

/*  if ((imageDdx->lx!=imageDdy->lx) || (imageDdx->ly!=imageDdy->ly))
    return GenErrorAppend(interp, "Error: `",argv[1],"' and `",argv[2],
			  "' must have the same size.",NULL);*/
  
  lx = dx_im->lx;
  ly = dx_im->ly;
  
  result_im = im_new (lx, ly, lx*ly, PHYSICAL);

  if (!result_im)
    return GenErrorMemoryAlloc (interp);

  dx  = dx_im->data;
  dxx = dxx_im->data;
  dy  = dy_im->data;
  dyy = dyy_im->data;
  dxy = dxy_im->data;

  result = result_im->data;

  for (i = 0; i < lx*ly; i++)
    {
      pos = 0;
      neg = 0;

      _add_ (2*dx[i]*dx[i]*dxx[i]);
      _add_ (4*dxy[i]*dx[i]*dy[i]);
      _add_ (2*dy[i]*dy[i]*dyy[i]);

      /*      result[i] = 0.5*(pos + neg)/(dx[i]*dx[i]+dy[i]*dy[i]);*/
      result[i] = pos + neg;
      /*      if (result[i] == 0 && dx[i] != 0)
	printf ("(%d,%d), x %g, xx %g, y %g, yy %g, xy %g,\n", i%lx, i/lx, dx[i], dxx[i], dy[i], dyy[i], dxy[i]);
	*/
      /*
      result[i] = (2*dx[i]*dx[i]*dxx[i] 
		   + 4*dxy[i]*dx[i]*dy[i] 
		   + 2*dy[i]*dy[i]*dyy[i]);
		   */
    }
  
  store_image (imageName, result_im);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}


/*************************************
 *  Command name in xsmurf : gkapap
 *************************************/
int
gkapap_TclCmd_ (ClientData  clientData,
		Tcl_Interp *interp, 
		int         argc,
		char      **argv)
{ 
  char * options[] = { "sIIIIIIIII",
			 NULL};
  int   lx,ly,i;
  char  *imageName;
  real  *result, *dx, *dxx, *dy, *dyy, *dxy, *dxxx, *dxxy, *dxyy, *dyyy;
  Image *result_im, *dx_im, *dxx_im, *dy_im, *dyy_im, *dxy_im;
  Image *dxxx_im, *dxxy_im, *dxyy_im, *dyyy_im;
  double pos = 0, neg = 0;
  
  if (arg_init (interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get(0, &imageName, &dx_im, &dxx_im, &dy_im, &dyy_im, &dxy_im,
	      &dxxx_im, &dxxy_im, &dxyy_im, &dyyy_im) == TCL_ERROR)
    return TCL_ERROR;

  /* To do : control the images size. */

/*  if ((imageDdx->lx!=imageDdy->lx) || (imageDdx->ly!=imageDdy->ly))
    return GenErrorAppend(interp, "Error: `",argv[1],"' and `",argv[2],
			  "' must have the same size.",NULL);*/
  
  lx = dx_im->lx;
  ly = dx_im->ly;
  
  result_im = im_new (lx, ly, lx*ly, PHYSICAL);

  if (!result_im)
    return GenErrorMemoryAlloc (interp);

  dx   = dx_im->data;
  dxx  = dxx_im->data;
  dy   = dy_im->data;
  dyy  = dyy_im->data;
  dxy  = dxy_im->data;
  dxxx = dxxx_im->data;
  dxxy = dxxy_im->data;
  dxyy = dxyy_im->data;
  dyyy = dyyy_im->data;

  result = result_im->data;

  for (i = 0; i < lx*ly; i++)
    {
      pos = 0;
      neg = 0;

      _add_ (dx[i]*dx[i]*(4*(dxx[i]*dxx[i] + dxy[i]*dxy[i])
			  + 2*dx[i]*dxxx[i]
			  + 6*dy[i]*dxxy[i]));
      _add_ (dy[i]*dy[i]*(4*(dyy[i]*dyy[i] + dxy[i]*dxy[i])
				  + 2*dy[i]*dyyy[i]
				  + 6*dx[i]*dxyy[i]));
      _add_ (8*dx[i]*dy[i]*dxy[i]*(dxx[i] + dyy[i]));

      result[i] = pos + neg;
      /*
      result[i] = (  dx[i]*dx[i]*(4*(dxx[i]*dxx[i] + dxy[i]*dxy[i])
				  + 2*dx[i]*dxxx[i]
				  + 6*dy[i]*dxxy[i])
		   + dy[i]*dy[i]*(4*(dyy[i]*dyy[i] + dxy[i]*dxy[i])
				  + 2*dy[i]*dyyy[i]
				  + 6*dx[i]*dxyy[i])
		   + 8*dx[i]*dy[i]*dxy[i]*(dxx[i] + dyy[i]));
		   */
      /*      result[i] = (  dx[i]*dx[i]*dx[i]*dxxx[i]
		     + dy[i]*dy[i]*dy[i]*dyyy[i]
		     + 3*dx[i]*dx[i]*dy[i]*dxxy[i]
		     + 3*dx[i]*dy[i]*dy[i]*dxyy[i]);*/
    }
  
  store_image (imageName, result_im);
  Tcl_AppendResult (interp, imageName, NULL);

  return TCL_OK;
}

/*
 * WARNING : This command doesn't verify the existance of the line.
 * To be modified...
 */
/*************************************
 * command name in xsmurf : linestats
 *************************************/
int
get_line_stats_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  char * options[] = { "l",
		       NULL };
  Line *line_ptr;
  char * help_msg =
  {("Get the value of the stats of a line which the memory"
    "adress is given as an hexadecimal number.")};

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &line_ptr) == TCL_ERROR)
    return TCL_ERROR;

  /*  display_stats (line_ptr);*/
  sprintf (interp->result,
	   "size        %d\n"
	   "mass     %f\n"
	   "mean mod  %f\n"
	   "gravity center\n"
	   "( %f ,\n"
	   "  %f )\n"
	   "m radius  %f\n"
	   "sigma rad  %f\n"
	   "nb of max\n"
	   "%d",
	   line_ptr->size,
	   line_ptr->mass,
	   line_ptr->mass /  line_ptr->size,
	   line_ptr->gravity_center_x,
	   line_ptr->gravity_center_y,
	   line_ptr->meanradius,
	   (float) sqrt(line_ptr->meansquareradius-line_ptr->meanradius*line_ptr->meanradius),
	   line_ptr->nb_of_gr);

  return TCL_OK;
}

/*************************************
 *  Command name in xsmurf : rm_ext
 *************************************/
int
w2_remove_extrema_TclCmd_ (ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)
{
  char * options[] = { "Esdddd",
		       NULL };
  char * help_msg =
  {("Remove extrema that are outside a box. Create a new image.")};

  ExtImage *ext_image, *result;
  char     *name;
  int x_min;
  int x_max;
  int y_min;
  int y_max;


  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &ext_image, &name,
	      &x_min, &x_max, &y_min, &y_max) == TCL_ERROR)
    return TCL_ERROR;

  result = w2_remove_extrema (ext_image, x_min, x_max, y_min, y_max);

  ExtDicStore (name, result);
  
  return TCL_OK;
}

/******************************************
 *  Command name in xsmurf : ekeep_isolated
 ******************************************/
int
w2_keep_isolated_TclCmd_ (ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)
{
  char * options[] = { "Esd",
		       NULL };
  char * help_msg =
  {("Keep extrema that are \"isolated\" (extrema that are alone\n"
    "in a box of size \"sizebox\"). Create a new ext-image.\n")};

  ExtImage *ext_image, *result;
  char     *name;
  int sizebox;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &ext_image, &name,
	      &sizebox) == TCL_ERROR)
    return TCL_ERROR;

  result = w2_keep_isolated (ext_image, sizebox);

  ExtDicStore (name, result);
  
  return TCL_OK;
}

/******************************************
 * Command name in xsmurf : ekeep_circle
 ******************************************/
int
w2_keep_circle_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  char * options[] = { "Esdd",
		       NULL };
  char * help_msg =
  {("Keep extrema that form closed loops whose mean radius is\n"
    "above a min threshold and below a max threshold.\n"
    "Create a new ext-image.\n"
    "\n"
    "Extima must be \"hsearch'ed\" before doing this !!\n")};
  
  ExtImage *ext_image, *result;
  char     *name;
  int radius_min, radius_max;
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &ext_image, &name, &radius_min, &radius_max) == TCL_ERROR)
    return TCL_ERROR;
  
  result = w2_keep_circle (ext_image, radius_min, radius_max);

  ExtDicStore (name, result);
  
  return TCL_OK;
}

/******************************************
 * Command name in xsmurf : ekeep_circle_simple
 ******************************************/
int
w2_keep_circle_simple_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
    char * options[] = { "Esdd",
			 NULL };
    char * help_msg =
	{("Same as ekeep_circle, but without the diameter, fil index, etc.\n"
	  "\n"
	  "Extima must be \"hsearch'ed\" before doing this !!\n")};
  
    ExtImage *ext_image, *result;
    char     *name;
    int radius_min, radius_max;
  
    if (arg_init (interp, argc, argv, options, help_msg))
	return TCL_OK;
  
    if (arg_get(0, &ext_image, &name, &radius_min, &radius_max) == TCL_ERROR)
	return TCL_ERROR;
  
    result = w2_keep_circle_simple (ext_image, radius_min, radius_max);

    ExtDicStore (name, result);
  
    return TCL_OK;
}

/******************************************
 * Command name in xsmurf : elog
 ******************************************/
int
w2_log_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  char * options[] = { "Esf",
		       NULL };
  char * help_msg =
  {("Make a new Ext-Image by computing logarithm of modulus\n"
    "for each extremum\n")};
  
  ExtImage *ext_image, *result;
  char     *name;
  float    baselog = exp(1);
  int      lx,ly, i;
  
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &ext_image, &name,
	      &baselog) == TCL_ERROR)
    return TCL_ERROR;
  
  if (baselog == 1.0)
    return GenErrorAppend(interp,"Base of logarithm can't be 1!!!\n",
			  NULL);
  if (baselog < 0.0)
    return GenErrorAppend(interp,"Base of logarithm can't be negative!!!\n",
			  NULL);
  
  lx = ext_image -> lx;
  ly = ext_image -> ly;
  /* memory allocation for the new extimage */
  result = w2_ext_new (ext_image -> extrNb, lx, ly, ext_image -> scale);

  for (i = 0; i < ext_image -> extrNb; i++) {
    result->extr[i].pos = ext_image->extr[i].pos;
    result->extr[i].mod = log(ext_image->extr[i].mod)/log(baselog);
    result->extr[i].arg = ext_image->extr[i].arg;
  }
    ExtDicStore (name, result);
  
  return TCL_OK;
}


/*************************************
  Command name in xsmurf : rm_1line
  ************************************/
int
w2_remove_line_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  char * options[] = { "l",
		       NULL };
  char * help_msg =
  {("Remove a line from an ext image.")};

  Line *line;
  Line *result_line;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &line) == TCL_ERROR)
    return TCL_ERROR;

  result_line = w2_remove_line (line->ext_image, line);
  if (!result_line)
    {
      sprintf (interp->result,
	       "This line is not in an ext_image.");
      return TCL_ERROR;
    }
  else
    {
      unstore_line_by_value (line);
      destroy_line (line);
    }
  
  return TCL_OK;
}

/*************************************
  Command name in xsmurf : rm_by_size
  ************************************/
/* modified 24/01/2001 by pierre kestener
   option -new                       */
int
w2_remove_lines_by_size_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  char * options[] = { "E",
		       "-min", "d",
		       "-max", "d",
		       "-new", "s",
		       NULL };

  char * help_msg =
  {("Remove lines from an ext image depending on the size.\n"
    "keep lines which sizes are between min and max\n"
    "\n"
    "Command defined in interpreter/wt2d_cmds.c by function :\n"
    "w2_remove_lines_by_size_TclCmd_\n"
    "and calls routine : w2_remove_lines_by_size\n"
    "defined in wt2d/wt2d.c\n"
    "\n"
    "Options :\n"
    "   -min [int]    : low value for testing line size\n"
    "   -max [int]    : up  value for testing line size\n"
    "   -new [string] : name of the new ext-ima.\n"
    "\n"
    "Take care that hsearch is performed before doing anything\n"
    "if not no lines would be found in your ext-image!!\n"
    "\n"
    "You can give to the new ext image name of the input\n"
    "ext image\n")};

  ExtImage *ext_image;
  int      size_min = -1;
  int      size_max = -1;

  /* variables pour l'option new */
  ExtImage *new_ext_image;
  char     *new_ext_name;
  int isNew;
  int nbext = 0;
  Extremum *ext;
  Extremum *newext;
  Line *l;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &ext_image) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (1, &size_min) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (2, &size_max) == TCL_ERROR)
    return TCL_ERROR;
  
  isNew = arg_present(3);
  if (isNew) {
    if (arg_get (3, &new_ext_name) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* on applique "hsearch" */
  search_lines (ext_image);

  /* on commence par enlever de la liste des lignes celles qui n'ont pas
     la bonne taille */
  w2_remove_lines_by_size (ext_image, size_min, size_max);
  
  /* si on souhaite creer une nouvelle ext-ima : */
  if(isNew) {
    /* on calcule le nombre d'extremun de la nouvelle image pour
       faire correctement l'allocation */
    foreach (l, ext_image->line_lst) {
      foreach (ext, l->ext_lst) {
	nbext++;
      }
    }
    /* allocation !!*/
    new_ext_image = w2_ext_new(nbext,ext_image->lx,ext_image->ly,ext_image->scale);
    if (!new_ext_image)
      return GenErrorMemoryAlloc(interp);
    
    /* on place le pointeur newext au bon endroit alloue 
       et on remplit la nouvelle extima*/
    newext = new_ext_image->extr;
    
    foreach (l, ext_image->line_lst) {
      foreach (ext, l->ext_lst) {
	newext->mod = ext->mod;
	newext->arg = ext->arg;
	newext->pos = ext->pos;
	newext++;
      }
    }
    
    search_lines (new_ext_image);
    sprintf (interp->result, "%d", nbext);

    /* ajout de la structure ExtImage au dictionnaire adequat */
    ExtDicStore(new_ext_name,new_ext_image);
  }
  
  return TCL_OK;
}

/*************************************
 * Command name in xsmurf : rm_by_mod
 *************************************/
int
w2_remove_lines_by_mean_mod_TclCmd_ (ClientData clientData,
				     Tcl_Interp *interp,
				     int        argc,
				     char       **argv)
{
  char * options[] = { "E",
		       "-min", "f",
		       "-max", "f",
		       NULL };
  char * help_msg =
  {("Remove lines from an ext image depending on the mod.\n"
    "we keep lines which mean modulus is between min and max\n"
    "Do not create a new ext-image\n")};

  ExtImage *ext_image;
  real     mod_min = -1;
  real     mod_max = -1;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &ext_image) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (1, &mod_min) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get (2, &mod_max) == TCL_ERROR)
    return TCL_ERROR;

  w2_remove_lines_by_mean_mod (ext_image, mod_min, mod_max);

  return TCL_OK;
}

/*************************************
  Command name in xsmurf : rm_by_arg
  ************************************/
int
w2_remove_lines_by_arg_TclCmd_ (ClientData clientData,
				Tcl_Interp *interp,
				int        argc,
				char       **argv)
{
  char * options[] = { "E",
		       NULL };
  char * help_msg =
  {("Remove lines from an ext image depending on the mod.")};

  ExtImage *ext_image;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &ext_image) == TCL_ERROR)
    return TCL_ERROR;

  w2_remove_lines_by_arg (ext_image);

  return TCL_OK;
}

/***********************************
  Command name in xsmurf : rm_lines 
  **********************************/
int
w2_remove_lines_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  char * options[] = { "E",
		       "-smin", "d",
		       "-smax", "d",
		       "-mmin", "f",
		       "-mmax", "f",
		       "-arg", "",
		       "-circle", "f",
		       NULL };
  char * help_msg =
  {("Remove from an ext image the lines which :\n"
    "  -smin size if lesser than a value.\n"
    "  -smax size if greater than a value.\n"
    "  -mmin mean modulus if lesser than a value.\n"
    "  -mmax mean modulus if greater than a value.\n"
    "  -arg  all the gradient points at the oposite of the gavity center.\n"
    "  -circle argument distribution is not uniform (i.e. keeps only circles).")};

  int      is_smin;
  int      is_smax;
  int      is_mmin;
  int      is_mmax;
  int      is_arg;
  int      is_circle;

  ExtImage *ext_image;
  int      size_min;
  int      size_max;
  real     mod_min;
  real     mod_max;
  Line     *line;
  List     *rm_lst;
  Extremum *ext;
  int      rm_flag;
  int      x, y;
  real     ratio;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get (0, &ext_image) == TCL_ERROR)
    return TCL_ERROR;

  is_smin = arg_present (1);
  is_smax = arg_present (2);
  if (arg_get (1, &size_min) == TCL_ERROR)
    return TCL_ERROR;
  if (arg_get (2, &size_max) == TCL_ERROR)
    return TCL_ERROR;

  is_mmin = arg_present (3);
  is_mmax = arg_present (4);
  if (arg_get (3, &mod_min) == TCL_ERROR)
    return TCL_ERROR;
  if (arg_get (4, &mod_max) == TCL_ERROR)
    return TCL_ERROR;

  is_arg = arg_present (5);

  is_circle = arg_present (6);
  if (arg_get (6, &ratio) == TCL_ERROR)
    return TCL_ERROR;

  rm_lst = lst_create ();
  foreach (line, ext_image->line_lst)
    {
      rm_flag = NO;
      if (is_smin && line->size < size_min)
	rm_flag = YES;
      if (is_smax && line->size > size_max)
	rm_flag = YES;
      if (is_mmin && (line->mass / line->size) < mod_min)
	rm_flag = YES;
      if (is_mmax && (line->mass / line->size) > mod_max)
	rm_flag = YES;
      if (is_arg)
	{
	  foreach (ext, line->ext_lst)
	    {
	      x = line->gravity_center_x - (real) (ext->pos % line->lx);
	      y = line->gravity_center_y - (real) (ext->pos / line->lx);
	      if ((x*cos (ext->arg) + y*sin (ext->arg)) < 0)
		rm_flag = YES;
	    }
	}
      if (is_circle)
	{
	  Signal *arg_histo;
	  int    nb_of_0 = 0;
	  int    i;

	  arg_histo = sig_new (REALY, 0, line->size -1);
	  arg_histo = stat_line_histogram (line, line->size, -M_PI, M_PI,
					   ARG_STAT, LINE_MAX_STAT, arg_histo);
	  for (i = 0; i < line->size; i++)
	    if (arg_histo->dataY[i] == 0)
	      nb_of_0 ++;
	  if (nb_of_0 > (ratio*line->size))
	    rm_flag = YES;
	}
      if (rm_flag == YES)
	lst_add (line, rm_lst);
    }
  foreach (line, rm_lst)
    {
      line = w2_remove_line (ext_image, line);
      if (line)
	destroy_line (line);
    }

  return TCL_OK;
}


/************************************
  Commande name in xsmurf : eupdate
  ***********************************/
int
update_ext_image_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  char * options[] = { "Es", 
			 NULL};
  
  ExtImage *ext_image, *new_ext_image;
  char     *name;
  int      nb_of_ext = 0;
  Line     *line, *new_line;
  Extremum *ext;
  int      i;

  if (arg_init(interp, argc, argv, options, NULL))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &name) == TCL_ERROR)
    return TCL_ERROR;

  foreach (line, ext_image->line_lst)
    {
      foreach (ext, line->ext_lst)
	nb_of_ext++;
    }
  
  new_ext_image = w2_ext_new(nb_of_ext,
			     ext_image->lx, ext_image ->ly,
			     ext_image->scale);
  if (!new_ext_image)
    return GenErrorMemoryAlloc(interp);

  i = 0;
  foreach (line, ext_image->line_lst)
    {
      new_line = create_line (line->lx, new_ext_image, line->scale);
      foreach (ext, line->ext_lst)
	{
	  new_ext_image->extr[i].pos = ext->pos;
	  new_ext_image->extr[i].mod = ext->mod;
	  new_ext_image->extr[i].arg = ext->arg;
	  add_extremum_to_line (&(new_ext_image->extr[i]),
				new_line, new_line->lx, LINE_BEG);
	  i++;
	}
      lst_add (new_line, new_ext_image->line_lst);
      new_ext_image->nb_of_lines ++;
    }

  ExtDicStore (name, new_ext_image);

  return TCL_OK;
}

/**********************************
  Command name in xsmurf : emerge
  *********************************/
int
merge_ext_image_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "EEs",
    NULL
  };

  char * help_msg =
  {
    ("Merge two ext_images into a third one.\n"
     "Parameters :\n"
     "  2 ext_images - the ones to merge.\n"
     "  string       - the name of the third.")
  };

  /* Command's parameters */
  ExtImage *ext_image1, *ext_image2;
  char     *name;

  /* Other variables */
  ExtImage *new_ext_image;
  int      i;
  int      k = 0;

  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image1, &ext_image2, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if ((ext_image1->lx != ext_image2->lx)
      || (ext_image1->ly != ext_image2->ly))
    {
      sprintf (interp->result,
	       "The 2 ext_images must have the same dimensions.");
      return TCL_ERROR;
    }

  /* Treatement */
  new_ext_image = w2_ext_new(ext_image1->extrNb + ext_image2->extrNb,
			     ext_image1->lx, ext_image1 ->ly,
			     ext_image1->scale);
  if (!new_ext_image)
    return GenErrorMemoryAlloc(interp);

  for (i = 0; i < ext_image1->extrNb; i++)
    {
      new_ext_image->extr[k] = ext_image1->extr[i];
      k++;
    }
  for (i = 0; i < ext_image2->extrNb; i++)
    {
      new_ext_image->extr[k] = ext_image2->extr[i];
      k++;
    }

  ExtDicStore (name, new_ext_image);

  return TCL_OK;
}


/* *********************************
 *  Command name in xsmurf : egather
 * *********************************/
int
gather_ext_image_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "EEs",
    "-vert", "",
    "-null1", "",
    "-null2", "",
    NULL
  };

  char * help_msg =
  {
    ("Gather two ext_images into a third one.\n"
     "Default is put second extima on the right of first one\n"
     "Parameters :\n"
     "  2 ext_images - the ones to merge.\n"
     "  string       - the name of the third.\n"
     "\n"
     "Options:\n"
     "  -vert  : put second extima below first one\n"
     "  -null1 : first extima is empty!\n"
     "  -null2 : second extima is empty!\n")
  };

  /* Command's parameters */
  ExtImage *ext_image1, *ext_image2;
  char     *name;

  /* Other variables */
  ExtImage *new_ext_image;
  int      i;
  int      k = 0;
  int      lx1, lx2, ly1, ly2, new_lx, new_ly;
  int      new_x, new_y;

  int isVert;
  int isNull1,isNull2;

  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image1, &ext_image2, &name) == TCL_ERROR)
    return TCL_ERROR;

  isVert = arg_present(1);
  isNull1 = arg_present(2);
  isNull2 = arg_present(3);

  if (isNull1 && isNull2) {
    sprintf (interp->result,
	     "You can't use options -null1 and -null2 together!");
      return TCL_ERROR;
  }

  lx1 = ext_image1->lx;
  lx2 = ext_image2->lx;
  ly1 = ext_image1->ly;
  ly2 = ext_image2->ly;
  
  /* Parameters validity and initialisation */
  if (isVert) {
    if (lx1 != lx2) {
      sprintf (interp->result,
	       "The 2 ext_images must have the same x-dimensions.");
      return TCL_ERROR;
    } else {
      new_lx = lx1;
      new_ly = ly1+ly2;
    }
  } else {
    if (ly1 != ly2) {
      sprintf (interp->result,
	       "The 2 ext_images must have the same y-dimensions.");
      return TCL_ERROR;
    } else {
      new_lx = lx1+lx2;
      new_ly = ly1;
    }
  }


  /* Treatement */
  
  if (isNull1) {
    new_ext_image = w2_ext_new(ext_image2->extrNb,
			       new_lx, new_ly,
			       ext_image2->scale);
  } else if (isNull2) {
    new_ext_image = w2_ext_new(ext_image1->extrNb,
			       new_lx, new_ly,
			       ext_image1->scale);
  }else {
    new_ext_image = w2_ext_new(ext_image1->extrNb + ext_image2->extrNb,
			       new_lx, new_ly,
			       ext_image1->scale);
  }
  
  if (!new_ext_image)
    return GenErrorMemoryAlloc(interp);
  
  if (isVert) {
    if (isNull1) {
    } else {
      for (i = 0; i < ext_image1->extrNb; i++) {
	new_x = ext_image1->extr[i].pos%lx1;
	new_y = ext_image1->extr[i].pos/lx1;
	new_ext_image->extr[k].pos = new_x + new_lx * new_y;
	new_ext_image->extr[k].mod = ext_image1->extr[i].mod;
	new_ext_image->extr[k].arg = ext_image1->extr[i].arg;
	k++;
      }
    }
    if (isNull2) {
    } else {
      for (i = 0; i < ext_image2->extrNb; i++) {
	new_x = ext_image2->extr[i].pos%lx2;
	new_y = ext_image2->extr[i].pos/lx2 + ly1;
	new_ext_image->extr[k].pos = new_x + new_lx * new_y;
	new_ext_image->extr[k].mod = ext_image2->extr[i].mod;
	new_ext_image->extr[k].arg = ext_image2->extr[i].arg;
	k++;
      }
    }
  } else {
    if (isNull1) {
    } else {
      for (i = 0; i < ext_image1->extrNb; i++) {
	new_x = ext_image1->extr[i].pos%lx1;
	new_y = ext_image1->extr[i].pos/lx1;
	new_ext_image->extr[k].pos = new_x + new_lx * new_y;
	new_ext_image->extr[k].mod = ext_image1->extr[i].mod;
	new_ext_image->extr[k].arg = ext_image1->extr[i].arg;
	k++;
      }
    }
    if (isNull2) {
    } else {
      for (i = 0; i < ext_image2->extrNb; i++) {
	new_x = ext_image2->extr[i].pos%lx2+ lx1;
	new_y = ext_image2->extr[i].pos/lx2;
	new_ext_image->extr[k].pos = new_x + new_lx * new_y;
	new_ext_image->extr[k].mod = ext_image2->extr[i].mod;
	new_ext_image->extr[k].arg = ext_image2->extr[i].arg;
	k++;
      }
    }
  }

  ExtDicStore (name, new_ext_image);
  sprintf(interp->result, "%d %d", new_lx, new_ly);
  return TCL_OK;
}


/*********************************
 * Command name in xsmurf : emask
 *********************************/
int
mask_ext_image_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ess[dd]",
    NULL
  };

  char * help_msg =
  {
    ("Create ane ext image by removing all the maxima from a zone given by a\n"
     "mask.\n"
     "Warning : the ext image must not be chained.\n"
     "\n"
     "Parameters :\n"
     "  ext image    - ext image to treat.\n"
     "  string       - name of the new ext image.\n"
     "  string       - file name where the mask is defined (format : B/W bpm).\n"
     "  [2 integers] - coordinates to place the mask.")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  char     *ext_image_name;
  char     *mask_name;
  int      x_0 = 0, y_0 = 0;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  ExtImage *new_ext_image;
  FILE *mask_file;
  int  i, j;
  char buffer[100];
  char format[100];
  int  mask_lx;
  int  mask_ly;
  unsigned char *mask_data;
  int  nb_of_maxima;
  Extremum *extremum;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &ext_image_name, &mask_name,
	       &x_0, &y_0) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  mask_file = fopen (mask_name, "r");
  if (!mask_file)
    return GenErrorAppend(interp, "Couldn't open `", mask_name,
			  "' for reading.", NULL);

  /* Read file header */
  fgets(format, 100, mask_file);
  while (format[0] == '#')
    fgets(format, 100, mask_file);
  if (strncmp (format, "P4", 2))
    {
      sprintf (interp->result,
	       "Wrong file format for %s.\n",
	       mask_name);
      return TCL_ERROR;
    }

  fgets(buffer, 100, mask_file);
  while (buffer[0] == '#')
    fgets(buffer, 100, mask_file);
  sscanf(buffer, "%d %d", &mask_lx, &mask_ly);
  if (mask_lx % 8 != 0)
    mask_lx = (mask_lx/8+1)*8;
  mask_data = (unsigned char *) malloc (sizeof(unsigned char)*(mask_lx*mask_ly/8));
  fread (mask_data, sizeof (char), (mask_lx*mask_ly/8), mask_file);

  _InitMaximaTable_(ext_image->lx*ext_image->ly);
  nb_of_maxima = w2_mask_ext_image (ext_image,
				    mask_data,
				    mask_lx,
				    mask_ly,
				    x_0,
				    y_0);

  new_ext_image = w2_ext_new (nb_of_maxima,
			      ext_image->lx,
			      ext_image->ly,
			      ext_image->scale);
  if (!new_ext_image)
    return GenErrorMemoryAlloc(interp);

  extremum = new_ext_image->extr;
  j = 0;
  for (i = 0; i < ext_image->extrNb; i++)
    if (_IsMaximum_(ext_image->extr[i].pos))
      {
	extremum[j] = ext_image->extr[i];
	j++;
      }

  ExtDicStore (ext_image_name, new_ext_image);
  fclose(mask_file);

  return TCL_OK;
}


/**********************************
 * Command name in xsmurf : emask2
 **********************************/
/* created on august 29 2003 by pierre kestener */
int
mask2_ext_image_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "EIfs",
    NULL
  };

  char * help_msg =
  {
    ("Create an ext image by removing all the maxima according to the mask\n"
     "as given by an image and a threshold.\n"
     "Warning : the ext image must not be chained.\n"
     "\n"
     "Parameters :\n"
     "  ext image    - ext image to treat.\n"
     "  image        - the mask (same size as the ext-image to treat)\n"
     "  float        - threshold value : if mask value is under threshold\n"
     "                 value, the corresponding extremun is removed.\n"
     "  string       - name of the new ext image.\n")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  Image    *imagemask;
  real      threshold;
  char     *res_name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  ExtImage *new_ext_image;
  int       i, j, pos;
  int       nb_of_maxima;
  Extremum *extremum;
  real     *maskdata;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &imagemask, &threshold, &res_name) == TCL_ERROR)
    return TCL_ERROR;
  
  /* Parameters validity and initialisation */
  if (ext_image->lx != imagemask->lx || ext_image->ly != imagemask->ly) {
    sprintf (interp->result, "ext-image and mask-image must have the same dimensions !\n");
    return TCL_ERROR;
  }

  /* Treatement */
  maskdata = imagemask->data;
  nb_of_maxima = 0;
  for (i = 0; i < ext_image->extrNb; i++) {
    pos = ext_image->extr[i].pos;
    if (maskdata[pos]>threshold)
      nb_of_maxima++;
  }

  new_ext_image = w2_ext_new (nb_of_maxima,
			      ext_image->lx,
			      ext_image->ly,
			      ext_image->scale);
  if (!new_ext_image)
    return GenErrorMemoryAlloc(interp);

  extremum = new_ext_image->extr;
  j = 0;
  for (i = 0; i < ext_image->extrNb; i++) {
    pos = ext_image->extr[i].pos;
    if (maskdata[pos]>threshold)
      {
	extremum[j].pos = ext_image->extr[i].pos;
	extremum[j].mod = ext_image->extr[i].mod;
	extremum[j].arg = ext_image->extr[i].arg;
	j++;
      }
  }

  ExtDicStore (res_name, new_ext_image);

  return TCL_OK;
}


/****************************************
 Command name in xsmurf : ekeep 
*****************************************/
int
w2_keep_by_value_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  char * options[] = { "Esf",
		       "-arg", "",
		       "-keeplow", "",
		       NULL };
  char * help_msg =
  {("Remove extrema which modulus is lesser than a value. Create a new image.\n"
    "Options :\n"
    "\n"
    "  -arg : the same but using arg of wavelet transform.\n"
    "  -keeplow : remove extrema which modulus is GREATER than value !!\n")};

  ExtImage *ext_image, *new_ext_image;
  char     *name;
  real     value;
  int      nb_of_maxima;
  int      i, j;
  Extremum *extremum;
  int      flag;
  int iskeeplow;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

  if (arg_get(0, &ext_image, &name,&value) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_present (1))
    flag = W2_ARG;
  else
    flag = W2_MOD;

  iskeeplow = arg_present (2);

  _InitMaximaTable_(ext_image->lx*ext_image->ly);

  if (iskeeplow) {
    nb_of_maxima = w2_keep_by_value_low (ext_image, value, flag);
  } else {
    nb_of_maxima = w2_keep_by_value (ext_image, value, flag);
  }
  
  new_ext_image = w2_ext_new (nb_of_maxima,
			      ext_image->lx,
			      ext_image->ly,
			      ext_image->scale);

  if (!new_ext_image)
    return GenErrorMemoryAlloc(interp);

  extremum = new_ext_image->extr;
  j = 0;
  for (i = 0; i < ext_image->extrNb; i++)
    if (_IsMaximum_(ext_image->extr[i].pos))
      {
	extremum[j] = ext_image->extr[i];
	j++;
      }

  ExtDicStore (name, new_ext_image);
  
  return TCL_OK;
}


/*
 * _get_m_ --
 *
 *   Find the maximum of function between two points with the value of the
 * points and the values of their neighbor. The interpolation is done
 * with a 3rd order polynome.
 */

static real
_get_m_ (real y_1,
	 real y0,
	 real y1,
	 real y2)
{
  /*
   *  f(x) = k3 x^3 + k2 x^2 + k1 x + k0
   *
   *  f(-1) = y_1
   *  f(0)  = y0
   *  f(1)  = y1
   *  f(2)  = y2
   */

  double k0;
  double k1;
  double k2;
  double k3;

  /*
   *  f'(x) = a x^2 + b x + c
   */

  double a; /* = 3 k3 */
  double b; /* = 2 k2 */
  double c; /* = k1 */

  /*
   *  Solutions of : a x^2 + b x + c = 0
   */

  double s;
  double s1;
  double s2;

  /*
   *  val = f(s) (it's the return value)
   */
  double val;

  /*
   * Computation of function f.
   */

  k0 = y0;
  k1 = (-2*y_1 + 6*y1 - 3*y0 - y2)/6;
  k2 = (y_1 + y1 - 2*y0)/2;
  k3 = - k1 + k2 - y_1 + k0;

  /*
   * Looking for solutions of f'(x)=0
   */

  a = 3*k3;
  if (a == 0) {
    if (b == 0) {
      return -1;
    }
    s = c/b;
  } else {
    b = 2*k2/a;
    c = k1/a;
    if ((b*b-4*c) < 0) {
      return -2;
    }

    s1 = (-b+sqrt(b*b-4*c))/2;
    s2 = (-b-sqrt(b*b-4*c))/2;

    /*
     * We only need the solution that is in [0,1], but solutions in [-1,0[ or in
     * ]1,2] are acceptable.
     */

    if (s1 >= 0 && s1 <= 1) {
      s = s1;
    } else if (s2 >= 0 && s2 <= 1) {
      s = s2;
    } else if (s1 >= -1 && s1 <= 2) {
      return -3;
      s = s1;
    } else if (s2 >= -1 && s2 <= 2) {
      return -3;
      s = s2;
    } else {
      return -3;
    }
  }

  val = (k3*s*s*s+k2*s*s+k1*s+k0);

  if (val < 0) {
    return -4;
  }

  return (real) val;
}


/*
 * The following macros record value of 16 directions.
 *
 *        13 12 11 10 09
 *        14 ++ ++ ++ 08
 *        15 ++ ** ++ 07
 *        00 ++ ++ ++ 06
 *        01 02 03 04 05
 */

#define THETA0 -2.677945044588987
#define THETA1 -2.3561944901923448
#define THETA2 -2.0344439357957027
#define THETA3 -1.5707963267948966
#define THETA4 -1.1071487177940904
#define THETA5 -0.78539816339744828
#define THETA6 -0.46364760900080615
#define THETA7 0.0
#define THETA8 0.46364760900080615
#define THETA9 0.78539816339744828
#define THETA10 1.1071487177940904
#define THETA11 1.5707963267948966
#define THETA12 2.0344439357957027
#define THETA13 2.3561944901923448
#define THETA14 2.677945044588987
#define THETA15 3.1415926535897931

/*
 * Tcl script used to compute the previous macros.

set pi 3.14159265358979323846
set j 0
for {set i -3} {$i < 4} {incr i 2} {
    puts "\#define THETA$j [expr { ($i-1)*$pi/4.0+atan(0.5) }]"
    incr j
    puts "\#define THETA$j [expr { $i*$pi/4.0 }]"
    incr j
    puts "\#define THETA$j [expr { ($i+1)*$pi/4.0-atan(0.5) }]"
    incr j
    puts "\#define THETA$j [expr { ($i+1)*$pi/4.0 }]"
    incr j
}
*/

/*
 * Macro that returns a new position from a postion, a delta x and a delta y.
 * This macro needs the value of lx.
 */ 

#define _getPos_(pos,x,y) (pos+x+y*lx)

/* debug var.... */
static int zetheta;

enum {
  ARG_DIRECTION,
  OPP_DIRECTION
};

/*
 * _get_mods_ --
 *
 *   This function gets 4 interpolated value of modulus in a direction (an
 * argument).
 *   real *m   - Array containing the modulus grid.
 *   real arg  - The argument.
 *   int  lx   - Width of the grid
 *   int  pos  - Starting position in the grid.
 *   real *m_1 - Pointer to the previous value of the modulus in the direction
 *               of a[k]. Point -1.
 *   real *m0  - Pointer to the value of the modulus in pos k.
 *   real *m1  - Pointer to the next value of the modulus in the direction
 *               of a[k]. Point 1.
 *   real *m2  - Pointer to the next next value of the modulus in the direction
 *               of a[k]. Point 2.
 *   int direction_flag - Indicates the direction to "loolk". ARG_DIRECTION: it
 *               is the direction given by arg. OPP_DIRECTION: it is the
 *               opposite direction.
 */

void
_get_mods_(real *m,
	   real arg,
	   int  lx,
	   int  pos,
	   real *m_1,
	   real *m0,
	   real *m1,
	   real *m2,
	   int  direction_flag)
{
  /*
   *   For each interpolation we take the convention that : the neighbors of the
   * point are called a and b (clockwise); and its position between a and b is
   * given by the value of p, which is in [0,1]. 
   */

  /*
   * Values used to interpolate m_1.
   */
  real ma_1; /* Modulus of the a neighbor of point -1 */
  real mb_1; /* Modulus of the b neighbor of point -1 */
  real p_1;  /* Position between a and b. */

  /*
   * Values used to interpolate m1.
   */
  real ma1; /* Modulus of the a neighbor of point 1 */
  real mb1; /* Modulus of the a neighbor of point 1 */
  real p1;  /* Position between a and b. */

  /*
   * Values used to interpolate m2.
   */
  real ma2; /* Modulus of the a neighbor of point 2 */
  real mb2; /* Modulus of the a neighbor of point 2 */
  real p2;  /* Position between a and b. */

  /*
   * Used to compute the position of a point between its "a and b neighbors".
   * Depend on the value of arg.
   */
  real factor;

  if (direction_flag == OPP_DIRECTION) {
    if (arg > 0) {
      arg -= M_PI;
    } else {
      arg += M_PI;
    }
  }


  if (arg < THETA0) {
    zetheta = 0;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *      2 1  -1 
     *      + + + + +
     *      + + + b +
     *      a a o a +
     *      b b + + +
     *      + + + + +
     */

    factor = tan(arg);

    p_1 = factor;
    ma_1 = m[_getPos_(pos, 1, 0)];
    mb_1 = m[_getPos_(pos, 1, 1)];

    p1  = factor;
    ma1 = m[_getPos_(pos, -1, 0)];
    mb1 = m[_getPos_(pos, -1, -1)];

    p2  = 2*factor;
    ma2 = m[_getPos_(pos, -2, 0)];
    mb2 = m[_getPos_(pos, -2, -1)];

  } else if (arg < THETA1) {
    zetheta = 1;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *      2 1  -1 
     *      + + + + +
     *      + + + b +
     *      + a o a +
     *      a b + + +
     *      b + + + +
     */

    factor = tan(arg);

    p_1 = factor;
    ma_1 = m[_getPos_(pos, 1, 0)];
    mb_1 = m[_getPos_(pos, 1, 1)];

    p1  = factor;
    ma1 = m[_getPos_(pos, -1, 0)];
    mb1 = m[_getPos_(pos, -1, -1)];

    p2  = 2*factor - 1;
    ma2 = m[_getPos_(pos, -2, -1)];
    mb2 = m[_getPos_(pos, -2, -2)];

  } else if (arg < THETA2) {
    zetheta = 2;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *      + + + + +
     *   -1 + + b a +
     *      + + o + +
     *    1 + a b + +
     *    2 a b + + +
     */

    factor = 1/tan(arg);

    p_1 = 1 - factor;
    ma_1 = m[_getPos_(pos, 1, 1)];
    mb_1 = m[_getPos_(pos, 0, 1)];

    p1  = 1 - factor;
    ma1 = m[_getPos_(pos, -1, -1)];
    mb1 = m[_getPos_(pos, 0, -1)];

    p2  = 2 - 2*factor;
    ma2 = m[_getPos_(pos, -2, -2)];
    mb2 = m[_getPos_(pos, -1, -2)];

  } else if (arg < THETA3) {
    zetheta = 3;
    /*
     *
     *      + + + + +
     *   -1 + + b a +
     *      + + o + +
     *    1 + a b + +
     *    2 + a b + +
     */

    factor = 1/tan(arg);

    p_1 = 1 - factor;
    ma_1 = m[_getPos_(pos, 1, 1)];
    mb_1 = m[_getPos_(pos, 0, 1)];

    p1  = 1 - factor;
    ma1 = m[_getPos_(pos, -1, -1)];
    mb1 = m[_getPos_(pos, 0, -1)];

    p2  = 1 - 2*factor;
    ma2 = m[_getPos_(pos, -1, -2)];
    mb2 = m[_getPos_(pos, 0, -2)];

  } else if (arg < THETA4) {
    zetheta = 4;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *      + + + + +
     *   -1 + b a + +
     *      + + o + +
     *    1 + + a b +
     *    2 + + a b +
     */

    factor = 1/tan(arg);

    p_1 = - factor;
    ma_1 = m[_getPos_(pos, 0, 1)];
    mb_1 = m[_getPos_(pos, -1, 1)];

    p1  = - factor;
    ma1 = m[_getPos_(pos, 0, -1)];
    mb1 = m[_getPos_(pos, 1, -1)];

    p2  = - 2*factor;
    ma2 = m[_getPos_(pos, 0, -2)];
    mb2 = m[_getPos_(pos, 1, -2)];

  } else if (arg < THETA5) {
    zetheta = 5;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *      + + + + +
     *   -1 + b a + +
     *      + + o + +
     *    1 + + a b +
     *    2 + + + a b
     */

    factor = 1/tan(arg);

    p_1 = - factor;
    ma_1 = m[_getPos_(pos, 0, 1)];
    mb_1 = m[_getPos_(pos, -1, 1)];

    p1  = - factor;
    ma1 = m[_getPos_(pos, 0, -1)];
    mb1 = m[_getPos_(pos, 1, -1)];

    p2  = - 2*factor - 1;
    ma2 = m[_getPos_(pos, 1, -2)];
    mb2 = m[_getPos_(pos, 2, -2)];

  } else if (arg < THETA6) {
    zetheta = 6;
    /*
     *       -1   1 2
     *      + + + + +
     *      + a + + +
     *      + b o b +
     *      + + + a b
     *      + + + + a
     */

    factor = tan(arg);

    p_1 = 1 + factor;
    ma_1 = m[_getPos_(pos, -1, 1)];
    mb_1 = m[_getPos_(pos, -1, 0)];

    p1  = 1 + factor;
    ma1 = m[_getPos_(pos, 1, -1)];
    mb1 = m[_getPos_(pos, 1, 0)];

    p2  = 2 + 2*factor;
    ma2 = m[_getPos_(pos, 2, -2)];
    mb2 = m[_getPos_(pos, 2, -1)];

  } else if (arg < THETA7) {
    zetheta = 8;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *       -1   1 2
     *      + + + + +
     *      + a + + +
     *      + b o b b
     *      + + + a a
     *      + + + + +
     */

    factor = tan(arg);

    p_1 = 1 + factor;
    ma_1 = m[_getPos_(pos, -1, 1)];
    mb_1 = m[_getPos_(pos, -1, 0)];

    p1  = 1 + factor;
    ma1 = m[_getPos_(pos, 1, -1)];
    mb1 = m[_getPos_(pos, 1, 0)];

    p2  = 1 + 2*factor;
    ma2 = m[_getPos_(pos, 2, -1)];
    mb2 = m[_getPos_(pos, 2, 0)];

  } else if (arg < THETA8) {
    /*
     *
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *       -1   1 2
     *      + + + + +
     *      + + + b b
     *      + a o a a
     *      + b + + +
     *      + + + + +
     */

    factor = tan(arg);

    p_1 = factor;
    ma_1 = m[_getPos_(pos, -1, 0)];
    mb_1 = m[_getPos_(pos, -1, -1)];

    p1  = factor;
    ma1 = m[_getPos_(pos, 1, 0)];
    mb1 = m[_getPos_(pos, 1, 1)];

    p2  = 2*factor;
    ma2 = m[_getPos_(pos, 2, 0)];
    mb2 = m[_getPos_(pos, 2, 1)];

  } else if (arg < THETA9) {
    zetheta = 9;
    /*
     *
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *       -1   1 2
     *      + + + + b
     *      + + + b a
     *      + a o a +
     *      + b + + +
     *      + + + + +
     */

    factor = tan(arg);

    p_1 = factor;
    ma_1 = m[_getPos_(pos, -1, 0)];
    mb_1 = m[_getPos_(pos, -1, -1)];

    p1  = factor;
    ma1 = m[_getPos_(pos, 1, 0)];
    mb1 = m[_getPos_(pos, 1, 1)];

    p2  = 2*factor - 1;
    ma2 = m[_getPos_(pos, 2, 1)];
    mb2 = m[_getPos_(pos, 2, 2)];

  } else if (arg < THETA10) {
    zetheta = 10;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *    2 + + + b a
     *    1 + + b a +
     *      + + o + +
     *   -1 + a b + +
     *      + + + + +
     */

    factor = 1/tan(arg);

    p_1 = 1 - factor;
    ma_1 = m[_getPos_(pos, -1, -1)];
    mb_1 = m[_getPos_(pos, 0, -1)];

    p1  = 1 - factor;
    ma1 = m[_getPos_(pos, 1, 1)];
    mb1 = m[_getPos_(pos, 0, 1)];

    p2  = 2 - 2*factor;
    ma2 = m[_getPos_(pos, 2, 2)];
    mb2 = m[_getPos_(pos, 1, 2)];

  } else if (arg < THETA11) {
    zetheta = 11;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *    2 + + b a +
     *    1 + + b a +
     *      + + o + +
     *   -1 + a b + +
     *      + + + + +
     */

    factor = 1/tan(arg);

    p_1 = 1 - factor;
    ma_1 = m[_getPos_(pos, -1, -1)];
    mb_1 = m[_getPos_(pos, 0, -1)];

    p1  = 1 - factor;
    ma1 = m[_getPos_(pos, 1, 1)];
    mb1 = m[_getPos_(pos, 0, 1)];

    p2  = 1 - 2*factor;
    ma2 = m[_getPos_(pos, 1, 2)];
    mb2 = m[_getPos_(pos, 0, 2)];

  } else if (arg < THETA12) {
    zetheta = 12;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *    2 + b a + +
     *    1 + b a + +
     *      + + o + +
     *   -1 + + a b +
     *      + + + + +
     */

    factor = 1/tan(arg);

    p_1 = - factor;
    ma_1 = m[_getPos_(pos, 0, -1)];
    mb_1 = m[_getPos_(pos, 1, -1)];

    p1  = - factor;
    ma1 = m[_getPos_(pos, 0, 1)];
    mb1 = m[_getPos_(pos, -1, 1)];

    p2  = - 2*factor;
    ma2 = m[_getPos_(pos, 0, 2)];
    mb2 = m[_getPos_(pos, -1, 2)];

  } else if (arg < THETA13) {
    zetheta = 13;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *    2 b a + + +
     *    1 + b a + +
     *      + + o + +
     *   -1 + + a b +
     *      + + + + +
     */

    factor = 1/tan(arg);

    p_1 = - factor;
    ma_1 = m[_getPos_(pos, 0, -1)];
    mb_1 = m[_getPos_(pos, 1, -1)];

    p1  = - factor;
    ma1 = m[_getPos_(pos, 0, 1)];
    mb1 = m[_getPos_(pos, -1, 1)];

    p2  = - 2*factor - 1;
    ma2 = m[_getPos_(pos, -1, 2)];
    mb2 = m[_getPos_(pos, -2, 2)];

  } else if (arg < THETA14) {
    zetheta = 14;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *      2 1  -1 
     *      a + + + +
     *      b a + + +
     *      + b o b +
     *      + + + a +
     *      + + + + +
     */

    factor = tan(arg);

    p_1 = 1 + factor;
    ma_1 = m[_getPos_(pos, 1, -1)];
    mb_1 = m[_getPos_(pos, 1, 0)];

    p1  = 1 + factor;
    ma1 = m[_getPos_(pos, -1, 1)];
    mb1 = m[_getPos_(pos, -1, 0)];

    p2  = 2 + 2*factor;
    ma2 = m[_getPos_(pos, -2, 2)];
    mb2 = m[_getPos_(pos, -2, 1)];

  } else if (arg < THETA15) {
    zetheta = 15;
    /*
     * o : central point (pos).
     * a, b : all the neighbors, by pairs.
     *
     *      2 1  -1 
     *      + + + + +
     *      a a + + +
     *      b b o b +
     *      + + + a +
     *      + + + + +
     */

    factor = tan(arg);

    p_1 = 1 + factor;
    ma_1 = m[_getPos_(pos, 1, -1)];
    mb_1 = m[_getPos_(pos, 1, 0)];

    p1  = 1 + factor;
    ma1 = m[_getPos_(pos, -1, 1)];
    mb1 = m[_getPos_(pos, -1, 0)];

    p2  = 1 + 2*factor;
    ma2 = m[_getPos_(pos, -2, 1)];
    mb2 = m[_getPos_(pos, -2, 0)];

  }

  assert(p_1 >= 0 || p_1 <= 1);
  assert(p1  >= 0 || p1  <= 1);
  assert(p2  >= 0 || p2  <= 1);

  *m_1 = p_1* mb_1 + (1 - p_1) * ma_1;
  *m1  = p1 * mb1  + (1 - p1)  * ma1;
  *m2  = p2 * mb2  + (1 - p2)  * ma2;

  *m0 = m[pos];

  return;
}


static real incr_x[10] = {-1,  0,  1,  1,  1,  0, -1, -1, -1,  0};
static real incr_y[10] = {-1, -1, -1,  0,  1,  1,  1,  0, -1,  0};

static int pos_incr[10];
static void
_init_pos_incr_(int lx)
{
  pos_incr[0] = -1 - lx;
  pos_incr[1] =    - lx;
  pos_incr[2] =  1 - lx;
  pos_incr[3] =  1;
  pos_incr[4] =  1 + lx;
  pos_incr[5] =      lx;
  pos_incr[6] = -1 + lx;
  pos_incr[7] =  -1;
  pos_incr[8] = -1 - lx;
  pos_incr[9] =  0;
}

static int
_get_near_pos_ (real *kapa,
		   int  lx,
		   int  pos)
{
  int i;
  int n_pos;
  int ret_val = 0;
  real tmp = 0;

  for (i = 1; i < 9; i = i+2) {
    n_pos = pos + pos_incr[i];
    if ((kapa[pos]*kapa[n_pos]) < 0
	&& fabs (kapa[pos]) < fabs (kapa[n_pos])) {
      if (ret_val == 0 || fabs(kapa[n_pos]) > tmp) {
	ret_val = i;
	tmp = fabs(kapa[n_pos]);
      }
    }
  }

  return ret_val;
}

real
_get_interpolated_modulus2_ (real *mod,
			    real *kapa,
			    int  lx,
			    int  ly,
			    int  pos,
			    real arg)
{
  int x;
  int y;

  real scalar_product;

  int i;

  int dir_flag;

  real m_1;
  real m0;
  real m1;
  real m2;

  real result;

  x = pos%lx;
  y = pos/lx;

  if (x < 2 || x >= lx-2 || y < 2 || y >= ly-2) {
    /*
     * The point is to close to the border -> we can't interpolate.
     */

    return mod[pos];
  }

  /*
   * First we check if we must look in the arg direction or in the
   * opposite direction. This is said by scalar_product: it is the scalar
   * product between arg and the discrete direction of the nearest kapa=0 line.
   * This discrete direction is north, north-east, etc. and is given by
   * _get_near_pos_.
   */

  i = _get_near_pos_(kapa, lx, pos);
  scalar_product = incr_x[i]*cos(arg)+incr_y[i]*sin(arg);
  if (scalar_product < 0) {
    dir_flag = OPP_DIRECTION;
  } else {
    dir_flag = ARG_DIRECTION;
  }

  _get_mods_ (mod, arg, lx, pos, &m_1, &m0, &m1, &m2, dir_flag);
  result = _get_m_ (m_1, m0, m1, m2);

  /*  if (m0 > 8.3 && m0 < 8.4 && x > 940 && x < 1100 && y > 940 && y < 1100) {
    printf("hoho\n");
    printf("x %d y %d\narg %f mod %f\nres1 %f theta %d\n-1 %f 0 %f 1 %f 2 %f\n", x, y, arg, m0, result, zetheta, m_1, m0, m1, m2);
  }*/

  if (result < 0 || result > mod[pos]*2) {
    /* _get_m_ error */

    /*
     * Perhaps it will work if we change the direction.
     */

    /*    if (x > 940 && x < 1100 && y > 940 && y < 1100) {
      printf("\n");
      printf("x %d y %d\narg %f mod %f\nres1 %f theta %d\n-1 %f 0 %f 1 %f 2 %f\n", x, y, arg, m0, result, zetheta, m_1, m0, m1, m2);
    }*/

    if (dir_flag == ARG_DIRECTION) {
      dir_flag = OPP_DIRECTION;
    } else {
      dir_flag = ARG_DIRECTION;
    }

    _get_mods_ (mod, arg, lx, pos, &m_1, &m0, &m1, &m2, dir_flag);
    result = _get_m_ (m_1, m0, m1, m2);

    if (result < 0 || result > mod[pos]*2) {
      /* _get_m_ error */
	
      /*      if (x > 940 && x < 1100 && y > 940 && y < 1100) {
	printf("res2 %f theta %d\n-1 %f 0 %f 1 %f 2 %f\n", result, zetheta, m_1, m0, m1, m2);
      }*/

      /*
       * It still doesn't work.
       */
      return mod[pos];
    } else {
      return result;
    }
  } else {
    return result;
  }
}


/*
 * _get_m2_ --
 *
 *   Find the maximum of function between two points with the value of the
 * points and the values of their neighbor. The interpolation is done
 * with a 3rd order polynome.
 */

static real
_get_m2_ (real y_1,
	  real y0,
	  real y1,
	  real y2,
	  real s)
{
  /*
   *  f(x) = k3 x^3 + k2 x^2 + k1 x + k0
   *
   *  f(-1) = y_1
   *  f(0)  = y0
   *  f(1)  = y1
   *  f(2)  = y2
   */

  double k0;
  double k1;
  double k2;
  double k3;

  /*
   *  f'(x) = a x^2 + b x + c
   */

  //double a;
  /* = 3 k3 */
  //double b;
  /* = 2 k2 */
  //double c;
  /* = k1 */


  /*
   *  val = f(s) (it's the return value)
   */
  double val;

  assert(s >= 0 && s <= 1);

  /*
   * Computation of function f.
   */

  k0 = y0;
  k1 = (-2*y_1 + 6*y1 - 3*y0 - y2)/6;
  k2 = (y_1 + y1 - 2*y0)/2;
  k3 = - k1 + k2 - y_1 + k0;

  val = (k3*s*s*s+k2*s*s+k1*s+k0);

  if (val < 0) {
    return -4;
  }

  return (real) val;
}


real
_get_interpolated_modulus_ (real *mod,
			    real *kapa,
			    int  lx,
			    int  ly,
			    int  pos,
			    real arg)
{
  int x;
  int y;

  //real scalar_product;

  int i;

  //int dir_flag;

  /*real m_1;
    real m0;
    real m1;
    real m2;*/
    
  real result;

  int pos_2;
  int pos_1;
  int pos0;
  int pos1;
  int pos2;

  real ratio;

  x = pos%lx;
  y = pos/lx;

  if (x < 2 || x >= lx-2 || y < 2 || y >= ly-2) {
    /*
     * The point is to close to the border -> we can't interpolate.
     */

    return mod[pos];
  }

  /*
   * First we check if we must look in the arg direction or in the
   * opposite direction. This is said by scalar_product: it is the scalar
   * product between arg and the discrete direction of the nearest kapa=0 line.
   * This discrete direction is north, north-east, etc. and is given by
   * _get_near_pos_.
   */

  i = _get_near_pos_(kapa, lx, pos);
  pos_2 = pos - 2*pos_incr[i];
  pos_1 = pos - pos_incr[i];
  pos0  = pos;
  pos1  = pos + pos_incr[i];
  pos2  = pos + 2*pos_incr[i];
  ratio = fabs(kapa[pos]/(kapa[pos]-kapa[pos1]));
  result = _get_m2_ (mod[pos_1], mod[pos0], mod[pos1], mod[pos2], ratio);

  if (result < 0 || result > 2*mod[pos]) {
    result = mod[pos];
  }

  return result;
}


/*********************************
 * Command name in xsmurf : follow 
 *********************************/
int
w2_follow_contour_TclCmd_(ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)      
{
  /* Command line definition */
  char * options[] =
  {
    "IIIIs[f]",
    "-old", "",
    "-v2", "",
    NULL
  };
  
  char * help_msg =
  {
    ("  Extract the contour lines from an image using its successive "
     "gradients. Needs 4 images to compute the lines:\n"
     "\n"
     "    modulus - which the modulus of the gradient of the original image;\n"
     "    argument - which is the argument of the gradient of the original "
     "image\n"
     "    kapa - which is the modulus of the gradient of the modulus of the "
     "gradient of the original image;\n"
     "    kapap - which is the modulus of the gradient of the modulus of the "
     "gradient of the modulus of the gradient of the original image (gasp!).\n"
     "\n"
     "  For each point on a contour line the value of the gradient is kept. "
     "This value is interpolated using the neighbors values of the pixel.\n"
     "\n"
     "Parameters:\n"
     "  4 images - Kapa, kapap, modulus and argument (in that order).\n"
     "  string   - Name of the resulting ext image.\n"
     "  [float]  - Scale of the ext image.\n"
     "\n"
     "Options:\n"
     "  -old: Uses the old algorithm that does not interpolate the value of "
     "the modulus.\n"
     "\n"
     "Return value:\n"
     "  None.\n"
     "NB : used in WtmmgCurrentScale routine (see tcl_library/imStudy.tcl)\n")
  };
  
  /* Command's parameters */
  Image    *kapa;
  Image    *kapap;
  Image    *mod;
  Image    *arg;
  char     *extImageName;
  real     scale = -1;

  /* Options's presence */
  int isOld;
  int isV2;

  /* Options's parameters */

  /* Other variables */
  ExtImage *extImage;
  Extremum *extremum;

  int lx;
  int ly;
  int pos;
  int nb_of_maxima;

  real *m;
  real *a;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg)) {
    return TCL_OK;
  }
  
  if (arg_get (0, &kapa, &kapap, &mod, &arg,
	      &extImageName, &scale)==TCL_ERROR) {
    return TCL_ERROR;
  }

  isOld = arg_present (1);
  isV2 = arg_present (2);

  /* Parameters validity and initialisation */

  lx = mod->lx;
  ly = mod->ly;

  m = mod->data;
  a = arg->data;

  _InitMaximaTable_(lx*ly);

  /* Treatement */

  smSetBeginTime();

  nb_of_maxima = w2_folow_contour (kapa->data, kapap->data, lx, ly, 1);

  smSetEndTime();
  
  extImage = w2_ext_new (nb_of_maxima, lx, ly, scale);
  if (!extImage) {
    return GenErrorMemoryAlloc(interp);
  }
  
  /*
   * Update ExtImage structure.
   */

  _init_pos_incr_(lx);
  extremum = extImage->extr;
  
  for (pos = 0; pos < lx*ly; pos++) {
    if (_IsMaximum_(pos)) {
      if (isOld) {
	extremum->mod = mod->data[pos];
	extremum->arg = arg->data[pos];
	extremum->pos = pos;
	extremum++;
      } else {
	if (isV2) {
	  extremum->mod = _get_interpolated_modulus2_(m, kapa->data, lx, ly, pos,
						     a[pos]);
	} else {
	  extremum->mod = _get_interpolated_modulus_(m, kapa->data, lx, ly, pos,
						     a[pos]);
	}
	extremum->arg = a[pos];
	extremum->pos = pos;
	extremum++;
      }
    }  
  }

  ExtDicStore(extImageName,extImage);
  
  _DeleteMaximaTable_();

  sprintf(interp->result, "%f", smGetEllapseTime());
  
  return TCL_OK;
}


/***********************************
  Command name in xsmurf : follow2
  **********************************/
int
w2_follow2_contour_TclCmd_(ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)      
{
  char * options[] = { "IIs[f]",
		       NULL };
  
  char * help_msg =
  {("Extract the pseudo contour lines. Set mod and arg fields of each ext to 0.\n")};
  
  Image    *kapa;
  Image    *kapap;

  ExtImage *extImage;
  Extremum *extremum;
  char     *extImageName;
  int      lx, ly, k;
  int      nb_of_maxima;
  real     scale = -1;

  if (arg_init(interp,argc,argv,options, help_msg))
    return TCL_OK;
  
  if (arg_get(0, &kapa, &kapap,
	      &extImageName, &scale)==TCL_ERROR)
    return TCL_ERROR;
  
  lx = kapa->lx;
  ly = kapa->ly;
  
  _InitMaximaTable_(lx*ly);
  nb_of_maxima = w2_folow_contour (kapa->data, kapap->data, lx, ly, 1);
  
  /* creation de la ExtImage (la aussi, verifier les allocations, etc...) */
  extImage = w2_ext_new(nb_of_maxima,lx,ly,scale);
  if (!extImage)
    return GenErrorMemoryAlloc(interp);
  
  /* mise a jour de la structure ExtImage */
  extremum = extImage->extr;
  for (k=0;k<lx*ly;k++)
    if (_IsMaximum_(k))
      {
	extremum->mod = 0;
	extremum->arg = 0;
	extremum->pos = k;
	extremum++;
      }  
  /* ajout de la structure ExtImage au dictionnaire adequat */
  ExtDicStore(extImageName,extImage);
  
  _DeleteMaximaTable_();
  
  return TCL_OK;
}


/*********************************
 * Command name in xsmurf : wtmm2d 
 *********************************/
int
w2_wtmm2d_TclCmd_(ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)      
{
  /* Command line definition */
  char * options[] =
  {
    "IIsfss",
    "-vector", "II",
    "-svdtype", "d",
    "-svd_LT", "ss",
    "-svd_LT_max", "ss",
    NULL
  };
  
  char * help_msg =
  {
    ("  Extract the contour lines from two images (gradX, gradY).\n"
     "\n"
     "\n"
     "Parameters:\n"
     "  2 images - gradx, grady.\n"
     "  string   - Name of the resulting ext image.\n"
     "  float    - Scale of the ext image.\n"
     "  string   - Name of modulus image.\n"
     "  string   - Name of argument image.\n"
     "\n"
     "Options:\n"
     "\n"
     "  -vector [II]: extract contour lines of a 2D -> 2D vector field.\n"
     "                user must provide (gradX, gradY) image of the second\n"
     "                component of the vector field.\n"
     "  -svdtype [d]: argument must be 0, 1.\n"
     "                SVD_TYPE_MAX(=0) means get max value + associated direction\n"
     "                SVD_TYPE_MIN(=1) means get min value + associated direction\n"
     "  -svd_LT [ss]: also output the longitudinal/transversal information (2 strings required\n"
     "                for the names of the additional ext-images\n"
     "  -svd_LT_max [ss]: if this option is present, we also output maxima chains, with modL/modT values\n"
     "\n"
     "Return value:\n"
     "  None.\n"
     "NB : used in WtmmgCurrentScale routine (see tcl_library/imStudy.tcl)\n")
  };
  
  /* Command's parameters */
  Image    *gradx, *grady;
  char     *extImageName;
  char     *modName, *argName;
  real     scale = -1;

  /* Options's presence */
  int isVector;
  int isSvdType;
  int isSvd_LT;
  int isSvd_LT_max;
    
  /* Options's parameters */
  Image    *gradx2, *grady2;
  int       svdtype = SVD_TYPE_MAX;
  char     *modName_L, *modName_T;
  char     *modName_ext_L, *modName_ext_T;
  
  /* Other variables */
  ExtImage *extImage;
  Extremum *extremum;

  int lx;
  int ly;
  int pos;
  int nb_of_maxima=0;

  // Regular Wavelet Transform vector
  Image *Mod, *Arg;
  float *mod,*arg,*max;

  // longitudinal / transversal information
  Image *ModL, *ModT;
  float *modL, *modT;
  ExtImage *extImage_modL;
  ExtImage *extImage_modT;
  
  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg)) {
    return TCL_OK;
  }
  
  if (arg_get (0, &gradx, &grady, &extImageName, &scale, &modName, &argName)==TCL_ERROR) {
    return TCL_ERROR;
  }

  isVector = arg_present(1);
  if (isVector) {
    if (arg_get(1, &gradx2, &grady2) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isSvdType = arg_present(2);
  if (isSvdType) {
    if (arg_get(2, &svdtype) == TCL_ERROR) {
      return TCL_ERROR;
    }

    if (svdtype != SVD_TYPE_MAX && svdtype != SVD_TYPE_MIN)
      return GenErrorAppend(interp,"Bad value for option svdtype (0 or 1 accepted)",
			    NULL);
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
  if ((gradx->lx != grady->lx) || (gradx->ly != grady->ly)) {
    Tcl_AppendResult(interp, "Inputs must have the same sizes!!!.", NULL);
    return TCL_ERROR;
  }
  if ((gradx->type != PHYSICAL) || (grady->type != PHYSICAL)) {
    Tcl_AppendResult(interp, "Inputs must have a PHYSICAL type!!!.", NULL);
    return TCL_ERROR;
  }

  if (isVector) {
    if ((gradx2->lx != grady2->lx) || (gradx2->ly != grady2->ly)) {
      Tcl_AppendResult(interp, "Inputs2 must have the same sizes!!!.", NULL);
      return TCL_ERROR;
    }
    if ((gradx->lx != gradx2->lx) || (grady->ly != grady2->ly)) {
      Tcl_AppendResult(interp, "All image inputs must have the same sizes!!!.", NULL);
      return TCL_ERROR;
    }
    if ((gradx2->type != PHYSICAL) || (grady2->type != PHYSICAL)) {
      Tcl_AppendResult(interp, "Inputs2 must have a PHYSICAL type!!!.", NULL);
      return TCL_ERROR;
    }
  
  }


  /* here we go */
  lx = gradx->lx;
  ly = gradx->ly;

  Mod = im_new (gradx->lx, gradx->ly, gradx->size, gradx->type);
  Arg = im_new (gradx->lx, gradx->ly, gradx->size, gradx->type);
  if ((!Mod) || (!Arg))
    return TCL_ERROR;
  mod = Mod->data;
  arg = Arg->data;
  max = (float *) malloc (gradx->size * sizeof(float));

  if (isSvd_LT) {
    ModL = im_new (gradx->lx, gradx->ly, gradx->size, gradx->type);
    ModT = im_new (gradx->lx, gradx->ly, gradx->size, gradx->type);
    if ((!ModL) || (!ModT))
      return TCL_ERROR;
    modL = ModL->data;
    modT = ModT->data;
  }
  
  /* Treatement */
  smSetBeginTime();

  if (isVector) {
    
    // Extract Longitudinal / Transversal information
    // gradx,grady are not modified by this
    // modL / modT are output
    if (isSvd_LT) { 
      Extract_Gradient_Maxima_2D_vectorfield_LT( gradx, grady, gradx2, grady2, modL, modT);
    }

    // gradx,grady will be modified after this function call
    Extract_Gradient_Maxima_2D_vectorfield( gradx, grady, gradx2, grady2, mod, arg, max, scale, svdtype);

  } else { // the regular scalar WT

    Extract_Gradient_Maxima_2D( gradx, grady, mod, arg, max, scale);

  }
  
  // compute the number of maxima (used right after for ext-image memory allocation)
  {
    float *tmpMax = (float *) max;
    int i;
    for (i = 0; i < lx*ly; i++, tmpMax++) {
      if (*tmpMax>0) 
	nb_of_maxima++;
    }
  }

  /*printf("nb of max: %d\n",nb_of_maxima);*/
  
  extImage = w2_ext_new (nb_of_maxima, lx, ly, scale);
  if (!extImage) {
    return GenErrorMemoryAlloc(interp);
  }

  // if isSvd_LT_max is true, we also save ext-image containing Longitudinal / Transversal wavelet values
  if (isSvd_LT_max) {
    
    extImage_modL = w2_ext_new (nb_of_maxima, lx, ly, scale);
    extImage_modT = w2_ext_new (nb_of_maxima, lx, ly, scale);

    if (!extImage_modL) {
      return GenErrorMemoryAlloc(interp);
    }
    if (!extImage_modT) {
      return GenErrorMemoryAlloc(interp);
    }

  }
  
  /*
   * Update ExtImage structure.
   */
  extremum = extImage->extr;
  {
    float *tmpMod = (float *) mod;
    float *tmpArg = (float *) arg;
    float *tmpMax = (float *) max;
    int i;
    for (i = 0; i < lx*ly; i++, tmpMod++, tmpArg++, tmpMax++) {
      if (*tmpMax>0) {
	extremum->mod = *tmpMod;
	extremum->arg = *tmpArg;
	extremum->pos = i;
	extremum++;
	/*printf("%d %d\n",i%lx,i/lx);*/
      }
    }
  }

  // update Longitudinal / Transversal extImage
  if (isSvd_LT_max) {

    Extremum *extremumL, *extremumT;

    extremumL = extImage_modL->extr;
    extremumT = extImage_modT->extr;
    
    float *tmpModL = (float *) modL;
    float *tmpModT = (float *) modT;
    float *tmpArg = (float *) arg;
    float *tmpMax = (float *) max;
    int i;
    for (i = 0; i < lx*ly; i++, tmpModL++, tmpModT++, tmpArg++, tmpMax++) {
      if (*tmpMax>0) {
	extremumL->mod = *tmpModL;
	extremumT->mod = *tmpModT;
	
	extremumL->arg = *tmpArg;
	extremumT->arg = *tmpArg;

	extremumL->pos = i;
	extremumT->pos = i;
	
	extremumL++;
	extremumT++;

      }
    }
    
  } // end of isSvd_LT_max

  smSetEndTime();

  ExtDicStore(extImageName,extImage);
  store_image(modName,Mod);
  store_image(argName,Arg);

  if (isSvd_LT) {
    store_image(modName_L, ModL);
    store_image(modName_T, ModT);
  }

  if (isSvd_LT_max) {
    ExtDicStore(modName_ext_L, extImage_modL);
    ExtDicStore(modName_ext_T, extImage_modT);
  }
  
  sprintf(interp->result, "%f", smGetEllapseTime());
  
  free(max);

  return TCL_OK;
  
} // w2_wtmm2d_TclCmd_

/*********************************
 * Command name in xsmurf : eiset
 *********************************/
int
w2_set_with_images_TclCmd_(ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)      
{
  /* Command line definition */
  char * options[] =
  {
    "E[II]",
    "-mod", "I[I]",
    "-arg", "I",
    "-position", "dd",
    "-flag", "",
    NULL
  };
  
  char * help_msg =
  {
    ("  Set modulus and/or argument fields with values from images.\n"
     "\n"
     "Parameters:\n"
     "  ExtImage - Ext image to treat.\n"
     "  [Image]   - Image to use for the modulus (this parameter is used for\n"
     "              backward compatibility).\n"
     "  [Image]   - Image to use for the argument (this parameter is used for\n"
     "              backward compatibility).\n"
     "\n"
     "Options:\n"
     "  -mod: Set the modulus field.\n"
     "    Image   - Image to use for the modulus.\n"
     "    [Image] - If this optional parameter is given, the ext image gets\n"
     "              an interpolate value of the modulus. The interpolation is\n"
     "              done using the method of the \"follow\" command. So this\n"
     "              parameter is the image that contains the value of kapa.\n"
     "  -arg: Set the argument field.\n"
     "    Image   - Image to use for the argument.\n"
     "  -position: Position of the images in the ext image.\n"
     "    2 integers - The position.\n"
     "  -flag: This flag is used for the way to handle the 2-pixels border\n"
     "where interpolation is not possible. If the option is present, we do not\n"
     "modify the value of the modulus (in the border) if it is non zero. This \n"
     "option is useful when one set the modulus value by sub-part of the\n"
     "complete ext image.\n"
     "\n"
     "Return value:\n"
     "  None.")
  };
  
  /* Command's parameters */
  ExtImage *extImage;

  /* Options's presence */
  int isMod;
  int isArg;
  int isPosition;
  int isFlag;

  /* Options's parameters */
  Image *mod = 0;
  Image *kapa = 0;
  Image *arg = 0;
  int X = 0;
  int Y = 0;

  /* Other variables */
  Extremum *extremum;
  int       k;
  int       pos;
  int       nb_of_maxima;

  int lx;
  int ly;

  int aLx;
  int aLy;

  int mLx;
  int mLy;

  int argPos;
  int modPos;

  int ex;
  int ey;

  /* Command line analysis */
  if (arg_init(interp, argc, argv, options, help_msg)) {
    return TCL_OK;
  }
  
  if (arg_get(0, &extImage, &mod, &arg) == TCL_ERROR) {
    return TCL_ERROR;
  }
  
  isMod = arg_present(1);
  if (isMod) {
    if (arg_get(1, &mod, &kapa) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isArg = arg_present(2);
  if (isArg) {
    if (arg_get(2, &arg) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isPosition = arg_present(3);
  if (isPosition) {
    if (arg_get(3, &X, &Y) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isFlag = arg_present(4);

  /* Parameters validity and initialisation */

  /* Treatement */

  lx = extImage->lx;
  ly = extImage->ly;

  nb_of_maxima = extImage->extrNb;
  
  if (arg) {
    aLx = arg->lx;
    aLy = arg->ly;

    extremum = extImage->extr;
    for (k = 0; k < nb_of_maxima; k++) {
      ex = extremum[k].pos%lx;
      ey = extremum[k].pos/lx;
      if (ex >= X && ex < X+aLx
	  && ey >= Y && ey < Y+aLy ) {
	argPos = (ex - X) + (ey - Y)*aLx;
	extremum[k].arg = arg->data[argPos];
      }
    }
  }

  _init_pos_incr_(lx);

  if (mod) { 
    mLx = mod->lx;
    mLy = mod->ly;

    extremum = extImage->extr;
    for (k = 0; k < nb_of_maxima; k++) {
      pos = extremum[k].pos;

      ex = extremum[k].pos%lx;
      ey = extremum[k].pos/lx;
      if (ex >= X && ex < X+mLx
	  && ey >= Y && ey < Y+mLy ) {
	modPos = (ex - X) + (ey - Y)*mLx;
	if (!kapa) {
	  extremum[k].mod = mod->data[modPos];
	} else {
	  if (!isFlag || extremum[k].mod == 0) {
	    extremum[k].mod = _get_interpolated_modulus_(mod->data, kapa->data, mLx, mLy,
							 modPos, extremum[k].arg);
	  }
	}
      }
    }
  }

  return TCL_OK;
}


int
_arg_is_near_ (real ext_arg, real arg, real arg_simil)
{
  return (fabs((1. - fabs(ext_arg - arg)/M_PI)) > arg_simil);
}

/********************************
  command name in xsmurf : efct
  *******************************/
/* modified by pierre kestener : 09/02/2001 -> option -domain */
/* modified by pierre kestener : 21/04/2006 -> option -mask */

int
apply_fct_to_e_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es[f]",
    "-vc", "",
    "-vc_max", "",
    "-arg", "ff",
    "-dx","",
    "-dy","",
    "-tag","",
    "-notag","",
    "-onarg","",
    "-domain","dddd",
    "-mask","I",
    NULL
  };

  char * help_msg =
  {
    (" Applies a function to each modulus of an ext image and sums all the "
     "values. The function can have two parameters.\n"
     "\n"
     "Parameters:\n"
     "  ext image - ext image to treat.\n"
     "  string    - an expression designing the function (defunc syntax).\n"
     "  [real]    - facultative value of a the second parameter of the\n"
     "              function.\n"
     "Options:\n"
     "  -vc: Applies the function only on maxima that are on a vertical\n"
     "    chain.\n"
     "  -vc_max: Same as \"-vc\" but take the maxmima of the modulus down\n"
     "    the chain.\n"
     "  -arg: Applies the function only on maxima which gradient argument is\n"
     "    near a given value.\n"
     "      float - value to use.\n"
     "      float - blblblblbblblbl.\n"
     "  -dx: Applies the function on the absolute value of the x component of\n"
     "    the gradient (and not on its modulus).\n"
     "  -dy: Applies the function on the absolute value of the y component of\n"
     "    the gradient (and not on its modulus).\n"
     "  -tag: Applies the function only on maxima that are tagged.\n"
     "  -notag: Applies the function only on maxima that are not tagged.\n"
     "  -onarg: Applies the function on the value of the arg.\n"
     "  -domain [dddd]: Applies the function only on maxima that are on \n"
     "                  a vertical chain AND inside a domain\n"
     "                  domain specified as in ecut command\n"
     "  -mask [I]: Applies the function only on maxima that are on \n"
     "             a vertical chain AND such that the value of the is\n"
     "             non-zero at the corresponding pixel\n"
     "\n"
     "command defined in interpreter/wt2d_cmds.c by fct apply_fct_to_e_TclCmd_ \n")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  char     *fct_expr;
  real     scd_value = 0;

  /* Options's presence */
  int is_vc;
  int is_vc_max;
  int is_arg;
  int is_dx;
  int is_dy;
  int is_tag;
  int is_notag;
  int is_onarg;
  int is_domain;
  int is_mask;

  /* Options's parameters */
  real arg;
  real arg_simil;

  /* Other variables */
  /*double   (*fct)();*/
  void     *fct;
  double   result = 0.0;
  double   *values;
  int      i, j;
  int      nb_of_values;
  Line     *line_ptr;
  Extremum *ext_ptr;
  int      x1, y1, x2, y2, tmp;
  int      pos, posx, posy;
  Image   *mask;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

  is_vc = arg_present (1);
  is_vc_max = arg_present (2);
  is_arg = arg_present (3);
  if (is_arg)
    if (arg_get (3, &arg, &arg_simil) == TCL_ERROR)
      return TCL_ERROR;

  is_dx = arg_present (4);
  is_dy = arg_present (5);
  is_tag = arg_present (6);
  is_notag = arg_present (7);
  is_onarg = arg_present (8);
  is_domain = arg_present (9);
  if (is_domain)
    if (arg_get (9, &x1, &y1, &x2, &y2) == TCL_ERROR)
      return TCL_ERROR;
  is_mask = arg_present (10);
  if (is_mask)
    if (arg_get (10, &mask) == TCL_ERROR)
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
  if (is_arg && (arg_simil > 1.0 || arg_simil < 0.0))
    {
      sprintf (interp->result,
	       "Bad value for arg_simil (%f). Must be between 0.0 and 1.0", arg_simil);
      return TCL_ERROR;
    }

  /* Treatement */
  if (is_domain) {
    /* Is specified domain valid ?*/
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
    if ((x2 < 0) || (y2 < 0) || (x1 >= ext_image->lx) || (y1 >= ext_image->ly)) {
      sprintf (interp->result, "bad specified domain : x1 %d, y1 %d, x2 %d, y2 %d", x1, y1, x2, y2);
      return TCL_ERROR;
    }
    if (x1 < 0) {
      x1 = 0;
    }
    if (y1 < 0) {
      y1 = 0;
    }
    if (x2 >= ext_image->lx) {
      x2 = ext_image->lx-1;
    }
    if (y2 >= ext_image->ly) {
      y2 = ext_image->ly-1;
    }
    
    /* computes nb_of_values (of ext on vc inside the domain)
       for memory allocation*/
    if (is_vc || is_vc_max) {
      nb_of_values = 0;
      foreach (line_ptr, ext_image->line_lst) {
	foreach (ext_ptr, line_ptr->gr_lst) {
	  if (ext_ptr->down || ext_ptr->up) {
	    /* test if ext is inside the specified domain */
	    pos  = ext_ptr->pos;
	    posx = pos % ext_image->lx;
	    posy = pos / ext_image->lx;
	    if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) {
	      /* current extremun is inside our specified domain, cool! */
	      if (is_tag && !ext_ptr->next) {
		continue;
	      }
	      if (is_notag && ext_ptr->next) {
		continue;
	      }
	      if (!is_arg
		  || (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil))) {
		nb_of_values++;
	      }
	    }
	    
	  } /* end loop on ext->up ext->down               */
	}   /* end loop on ext that are on horizontal line */
      }     /* end loop on lines                           */

      values = (double *) malloc (nb_of_values*sizeof (double));
      i = 0;
      foreach (line_ptr, ext_image->line_lst) {
	foreach (ext_ptr, line_ptr->gr_lst) {
	  if (ext_ptr->down || ext_ptr->up) {
	    
	    /* test if ext is inside the specified domain */
	    pos  = ext_ptr->pos;
	    posx = pos % ext_image->lx;
	    posy = pos / ext_image->lx;
	    if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) {
	      if (is_tag && !ext_ptr->next) {
		continue;
	      }
	      if (is_notag && ext_ptr->next) {
		continue;
	      }
	      if (!is_arg
		  || (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil))) {
		if (is_dx) {
		  values[i] = fabs(ext_ptr->mod*cos(ext_ptr->arg));
		} else if (is_dy) {
		  values[i] = fabs(ext_ptr->mod*sin(ext_ptr->arg));
		} else if (is_onarg) {
		  values[i] = ext_ptr->arg;
		} else {
		  values[i] = ext_ptr->mod;
		}
		if (is_vc_max) {
		  while(ext_ptr->down) {
		    ext_ptr = ext_ptr->down;
		    if (ext_ptr->mod > values[i]) {
		      if (is_dx) {
			values[i] = fabs(ext_ptr->mod*cos(ext_ptr->arg));
		      } else if (is_dy) {
			values[i] = fabs(ext_ptr->mod*sin(ext_ptr->arg));
		      } else if (is_onarg) {
			values[i] = ext_ptr->arg;
		      } else {
			values[i] = ext_ptr->mod;
		      }
		    }
		  }
		}
		/*values[i] = fct (values[i], scd_value);*/
                values[i] = evaluator_evaluate_x_y(fct, values[i], scd_value);
		i++;
	      } /* matches if(!is_arg...      */
	    }   /* matches if ((x > x1) && ...*/
	  }     /* */
	}       /* foreach ext_ptr ...        */
      }         /* foreach line ...           */
      
    } else { /* if (is_vc || is_vc_max) */
      if (is_arg) {
	nb_of_values = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  posx = pos % ext_image->lx;
	  posy = pos / ext_image->lx;
	  if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) { 
	    if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil))
	      nb_of_values++;
	  }
	}
	values = (double *) malloc (nb_of_values*sizeof (double));
	j = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  posx = pos % ext_image->lx;
	  posy = pos / ext_image->lx;
	  if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) { 
	    if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil)) {
	      if (is_dx) {
                /*values[j] = fct (fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);*/
		values[j] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);
	      } else if (is_dy) {
		/*values[j] = fct (fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);*/
		values[j] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);
	      } else if (is_onarg) {
                /*values[j] = fct (ext_image->extr[i].arg, scd_value);*/
		values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].arg, scd_value);
	      } else {
		/*values[j] = fct (ext_image->extr[i].mod, scd_value);*/
		values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].mod, scd_value);
	      }
	      j++;
	    }
	  }
	}
      } else { /* if (is_arg) */
	nb_of_values = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  posx = pos % ext_image->lx;
	  posy = pos / ext_image->lx;
	  if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) { 
	    nb_of_values++;
	  }
	}
	/*nb_of_values = ext_image->extrNb;*/
	values = (double *) malloc (nb_of_values*sizeof (double));
	j=0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  posx = pos % ext_image->lx;
	  posy = pos / ext_image->lx;
	  if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) { 
	    if (is_dx) {
              /*values[j] = fct (fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);
	    } else if (is_dy) {
	      /*values[j] = fct (fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);
	    } else if (is_onarg) {
              /*values[j] = fct (ext_image->extr[i].arg, scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].arg, scd_value);
	    } else {
	      /*values[j] = fct (ext_image->extr[i].mod, scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].mod, scd_value);
	    }
	    j++;
	  }
	}
      }  
    }
  } else if (is_mask) {

    /* computes nb_of_values (of ext on vc such that mask is non-zero)
       for memory allocation*/
    if (is_vc || is_vc_max) {
      nb_of_values = 0;
      foreach (line_ptr, ext_image->line_lst) {
	foreach (ext_ptr, line_ptr->gr_lst) {
	  if (ext_ptr->down || ext_ptr->up) {
	    /* test if ext is such that mask is non-zero */
	    pos  = ext_ptr->pos;
	    if ( mask->data[pos] > 0.0 || mask->data[pos] < 0.0 ) {

	      if (is_tag && !ext_ptr->next) {
		continue;
	      }
	      if (is_notag && ext_ptr->next) {
		continue;
	      }
	      if (!is_arg
		  || (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil))) {
		nb_of_values++;
	      }

	    }
	    
	  } /* end loop on ext->up ext->down               */
	}   /* end loop on ext that are on horizontal line */
      }     /* end loop on lines                           */

      values = (double *) malloc (nb_of_values*sizeof (double));
      i = 0;
      foreach (line_ptr, ext_image->line_lst) {
	foreach (ext_ptr, line_ptr->gr_lst) {
	  if (ext_ptr->down || ext_ptr->up) {
	    
	    /* test if ext is inside the specified domain */
	    pos  = ext_ptr->pos;
	    posx = pos % ext_image->lx;
	    posy = pos / ext_image->lx;
	    if ( mask->data[pos] > 0.0 || mask->data[pos] < 0.0) {
	      if (is_tag && !ext_ptr->next) {
		continue;
	      }
	      if (is_notag && ext_ptr->next) {
		continue;
	      }
	      if (!is_arg
		  || (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil))) {
		if (is_dx) {
		  values[i] = fabs(ext_ptr->mod*cos(ext_ptr->arg));
		} else if (is_dy) {
		  values[i] = fabs(ext_ptr->mod*sin(ext_ptr->arg));
		} else if (is_onarg) {
		  values[i] = ext_ptr->arg;
		} else {
		  values[i] = ext_ptr->mod;
		}
		if (is_vc_max) {
		  while(ext_ptr->down) {
		    ext_ptr = ext_ptr->down;
		    if (ext_ptr->mod > values[i]) {
		      if (is_dx) {
			values[i] = fabs(ext_ptr->mod*cos(ext_ptr->arg));
		      } else if (is_dy) {
			values[i] = fabs(ext_ptr->mod*sin(ext_ptr->arg));
		      } else if (is_onarg) {
			values[i] = ext_ptr->arg;
		      } else {
			values[i] = ext_ptr->mod;
		      }
		    }
		  }
		}
                /*values[i] = fct (values[i], scd_value);*/
		values[i] = evaluator_evaluate_x_y(fct, values[i], scd_value);
		i++;
	      } /* matches if(!is_arg...      */
	    }   /* matches if ((x > x1) && ...*/
	  }     /* */
	}       /* foreach ext_ptr ...        */
      }         /* foreach line ...           */
      
    } else { /* if (is_vc || is_vc_max) */
      if (is_arg) {
	nb_of_values = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  
	  if ( mask->data[pos] > 0.0 || mask->data[pos] < 0.0 ) { 
	    if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil))
	      nb_of_values++;
	  }
	}
	values = (double *) malloc (nb_of_values*sizeof (double));
	j = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  posx = pos % ext_image->lx;
	  posy = pos / ext_image->lx;
	  if ( mask->data[pos] > 0.0 || mask->data[pos] < 0.0 ) { 
	    if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil)) {
	      if (is_dx) {
		/*values[j] = fct (fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);*/
		values[j] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);
	      } else if (is_dy) {
                /*values[j] = fct (fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);*/
		values[j] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);
	      } else if (is_onarg) {
                /*values[j] = fct (ext_image->extr[i].arg, scd_value);*/
		values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].arg, scd_value);
	      } else {
                /*values[j] = fct (ext_image->extr[i].mod, scd_value);*/
		values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].mod, scd_value);
	      }
	      j++;
	    }
	  }
	}
      } else { /* if (is_arg) */
	nb_of_values = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  if ( mask->data[pos] > 0.0 || mask->data[pos] < 0.0 ) { 
	    nb_of_values++;
	  }
	}
	/*nb_of_values = ext_image->extrNb;*/
	values = (double *) malloc (nb_of_values*sizeof (double));
	j=0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;

	  if ( mask->data[pos] > 0.0 || mask->data[pos] < 0.0 ) { 
	    if (is_dx) {
              /*values[j] = fct (fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);
	    } else if (is_dy) {
	      /*values[j] = fct (fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);
	    } else if (is_onarg) {
	      /*values[j] = fct (ext_image->extr[i].arg, scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].arg, scd_value);
	    } else {
	      /*values[j] = fct (ext_image->extr[i].mod, scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].mod, scd_value);
	    }
	    j++;
	  }
	}
      }  
    }

  } else { /* !is_domain and !is_mask*/
    if (is_vc || is_vc_max) {
      nb_of_values = 0;
      foreach (line_ptr, ext_image->line_lst) {
	foreach (ext_ptr, line_ptr->gr_lst) {
	  if (ext_ptr->down || ext_ptr->up) {
	    if (is_tag && !ext_ptr->next) {
	      continue;
	    }
	    if (is_notag && ext_ptr->next) {
	      continue;
	    }
	    if (!is_arg
		|| (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil))) {
	      nb_of_values++;
	    }
	  }
	}
      }
      values = (double *) malloc (nb_of_values*sizeof (double));
      i = 0;
      foreach (line_ptr, ext_image->line_lst) {
	foreach (ext_ptr, line_ptr->gr_lst) {
	  if (ext_ptr->down || ext_ptr->up) {
	    if (is_tag && !ext_ptr->next) {
	      continue;
	    }
	    if (is_notag && ext_ptr->next) {
	      continue;
	    }
	    if (!is_arg
		|| (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil))) {
	      if (is_dx) {
		values[i] = fabs(ext_ptr->mod*cos(ext_ptr->arg));
	      } else if (is_dy) {
		values[i] = fabs(ext_ptr->mod*sin(ext_ptr->arg));
	      } else if (is_onarg) {
		values[i] = ext_ptr->arg;
	      } else {
		values[i] = ext_ptr->mod;
	      }
	      if (is_vc_max) {
		while(ext_ptr->down) {
		  ext_ptr = ext_ptr->down;
		  if (ext_ptr->mod > values[i]) {
		    if (is_dx) {
		      values[i] = fabs(ext_ptr->mod*cos(ext_ptr->arg));
		    } else if (is_dy) {
		      values[i] = fabs(ext_ptr->mod*sin(ext_ptr->arg));
		    } else if (is_onarg) {
		      values[i] = ext_ptr->arg;
		    } else {
		      values[i] = ext_ptr->mod;
		    }
		  }
		}
	      }
	      /*values[i] = fct (values[i], scd_value);*/
	      values[i] = evaluator_evaluate_x_y(fct, values[i], scd_value);
	      i++;
	    }
	  }
	}
      }
    } else { /* if (is_vc || is_vc_max) */
      if (is_arg) {
	nb_of_values = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil))
	    nb_of_values++;
	}
	values = (double *) malloc (nb_of_values*sizeof (double));
	j = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil)) {
	    if (is_dx) {
              /*values[j] = fct (fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);
	    } else if (is_dy) {
              /*values[j] = fct (fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);
	    } else if (is_onarg) {
	      /*values[j] = fct (ext_image->extr[i].arg, scd_value);*/
 	      values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].arg, scd_value);
	    } else {
              /*values[j] = fct (ext_image->extr[i].mod, scd_value);*/
	      values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].mod, scd_value);
	    }
	    j++;
	  }
	}
      } else { /* if (is_arg) */
	nb_of_values = ext_image->extrNb;
	values = (double *) malloc (ext_image->extrNb*sizeof (double));
	for (i = 0; i < ext_image->extrNb; i++) {
	  if (is_dx) {
	    /*values[i] = fct (fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);*/
	    values[i] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg)), scd_value);
	  } else if (is_dy) {
	    /*values[i] = fct (fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);*/
	    values[i] = evaluator_evaluate_x_y(fct, fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg)), scd_value);
	  } else if (is_onarg) {
	    /*values[i] = fct (ext_image->extr[i].arg, scd_value);*/
	    values[i] = evaluator_evaluate_x_y(fct, ext_image->extr[i].arg, scd_value);
	  } else {
            /*values[i] = fct (ext_image->extr[i].mod, scd_value);*/
	    values[i] = evaluator_evaluate_x_y(fct, ext_image->extr[i].mod, scd_value);
	  }
	}
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

/***************************************
  command name in xsmurf : efct_tsallis
  **************************************/

/* created by pierre kestener : 20/03/2001 -> to handle tsallis stuffs */
/* on duplique le code de apply_fct_to_e_TclCmd_ et on apporte les
   modifications necessaires pour gerer les trucs de tsallis*/
int
apply_fct_to_e_tsallis_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Essff",
    "-vc", "",
    "-vc_max", "",
    "-arg", "ff",
    "-dx","",
    "-dy","",
    "-tag","",
    "-notag","",
    "-onarg","",
    "-domain","dddd",
    "-SK_logT","",
    NULL
  };

  char * help_msg =
  {
    (" Applies a (certain) function to each modulus of an ext image \n"
     "and sums all the values. The function can have two parameters.\n"
     "\n"
     "Parameters:\n"
     "  ext image - ext image to treat.\n"
     "  string    - an expression designing the function f (defunc syntax).\n"
     "  string    - an expression designing the function g (defunc syntax).\n"
     "  real      - parameter q     (temperature      )\n"
     "  real      - parameter qtsa  (tsallis parameter)\n"
     "\n"
     "Options:\n"
     "  -vc: Applies the function only on maxima that are on a vertical\n"
     "    chain.\n"
     "  -vc_max: Same as \"-vc\" but take the maxmima of the modulus down\n"
     "    the chain.\n"
     "  -arg: Applies the function only on maxima which gradient argument is\n"
     "    near a given value.\n"
     "      float - value to use.\n"
     "      float - blblblblbblblbl.\n"
     "  -dx: Applies the function on the absolute value of the x component of\n"
     "    the gradient (and not on its modulus).\n"
     "  -dy: Applies the function on the absolute value of the y component of\n"
     "    the gradient (and not on its modulus).\n"
     "  -tag: Applies the function only on maxima that are tagged.\n"
     "  -notag: Applies the function only on maxima that are not tagged.\n"
     "  -onarg: Applies the function on the value of the arg.\n"
     "  -domain [dddd]: Applies the function only on maxima that are on \n"
     "                  a vertical chain AND inside a domain\n"
     "                  domain specified as in ecut command\n"
     "  -SK_logT : usefull to compute SKqqtsa_log in the correct way\n"
     "\n"
     "command defined in interpreter/wt2d_cmds.c by fct \n"
     "apply_fct_to_e_tsallis_TclCmd_ \n")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  char     *fct_expr1;
  char     *fct_expr2;
  /*real     scd_value = 0; replace by temperature q*/
  real q = 0;
  real qtsa = 1;

  /* Options's presence */
  int is_vc;
  int is_vc_max;
  int is_arg;
  int is_dx;
  int is_dy;
  int is_tag;
  int is_notag;
  int is_onarg;
  int is_domain;
  int is_SK_logT;

  /* Options's parameters */
  real arg;
  real arg_simil;

  /* Other variables */
  /*double   (*fct1)();
    double   (*fct2)();*/
  void *fct1, *fct2;
  double   result = 0.0;
  double   *values;
  double   tmpvalue;
  int      i, j;
  int      nb_of_values;
  Line     *line_ptr;
  Extremum *ext_ptr;
  int      x1, y1, x2, y2, tmp;
  int      pos, posx, posy;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &fct_expr1, &fct_expr2, &q, &qtsa) == TCL_ERROR)
    return TCL_ERROR;

  is_vc = arg_present (1);
  is_vc_max = arg_present (2);
  is_arg = arg_present (3);
  if (is_arg)
    if (arg_get (3, &arg, &arg_simil) == TCL_ERROR)
      return TCL_ERROR;

  is_dx = arg_present (4);
  is_dy = arg_present (5);
  is_tag = arg_present (6);
  is_notag = arg_present (7);
  is_onarg = arg_present (8);
  is_domain = arg_present (9);
  if (is_domain)
    if (arg_get (9, &x1, &y1, &x2, &y2) == TCL_ERROR)
      return TCL_ERROR;
  is_SK_logT = arg_present(10);

  /* Parameters validity and initialisation */
  /*fct1 = dfopen (fct_expr1);
    fct2 = dfopen (fct_expr2);*/
  fct1 = evaluator_create(fct_expr1);
  fct2 = evaluator_create(fct_expr2);
  if (!fct1 || !fct2)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression 1 or 2 ",
			fct_expr1, (char *) NULL);
      return TCL_ERROR;
    }
  if (is_arg && (arg_simil > 1.0 || arg_simil < 0.0))
    {
      sprintf (interp->result,
	       "Bad value for arg_simil (%f). Must be between 0.0 and 1.0", arg_simil);
      return TCL_ERROR;
    }

  /* Treatement */
  if (is_domain) {
    /* Is specified domain valid ?*/
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
    if ((x2 < 0) || (y2 < 0) || (x1 >= ext_image->lx) || (y1 >= ext_image->ly)) {
      sprintf (interp->result, "bad specified domain : x1 %d, y1 %d, x2 %d, y2 %d", x1, y1, x2, y2);
      return TCL_ERROR;
    }
    if (x1 < 0) {
      x1 = 0;
    }
    if (y1 < 0) {
      y1 = 0;
    }
    if (x2 >= ext_image->lx) {
      x2 = ext_image->lx-1;
    }
    if (y2 >= ext_image->ly) {
      y2 = ext_image->ly-1;
    }
    
    /* computes nb_of_values (of ext on vc inside the domain)
       for memory allocation*/
    if (is_vc || is_vc_max) {
      nb_of_values = 0;
      foreach (line_ptr, ext_image->line_lst) {
	foreach (ext_ptr, line_ptr->gr_lst) {
	  if (ext_ptr->down || ext_ptr->up) {
	    /* test if ext is inside the specified domain */
	    pos  = ext_ptr->pos;
	    posx = pos % ext_image->lx;
	    posy = pos / ext_image->lx;
	    if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) {
	      /* current extremun is inside our specified domain, cool! */
	      if (is_tag && !ext_ptr->next) {
		continue;
	      }
	      if (is_notag && ext_ptr->next) {
		continue;
	      }
	      if (!is_arg
		  || (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil))) {
		nb_of_values++;
	      }
	    }
	    
	  } /* end loop on ext->up ext->down               */
	}   /* end loop on ext that are on horizontal line */
      }     /* end loop on lines                           */

      values = (double *) malloc (nb_of_values*sizeof (double));
      i = 0;
      foreach (line_ptr, ext_image->line_lst) {
	foreach (ext_ptr, line_ptr->gr_lst) {
	  if (ext_ptr->down || ext_ptr->up) {
	    
	    /* test if ext is inside the specified domain */
	    pos  = ext_ptr->pos;
	    posx = pos % ext_image->lx;
	    posy = pos / ext_image->lx;
	    if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) {
	      if (is_tag && !ext_ptr->next) {
		continue;
	      }
	      if (is_notag && ext_ptr->next) {
		continue;
	      }
	      if (!is_arg
		  || (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil))) {
		if (is_dx) {
		  values[i] = fabs(ext_ptr->mod*cos(ext_ptr->arg));
		} else if (is_dy) {
		  values[i] = fabs(ext_ptr->mod*sin(ext_ptr->arg));
		} else if (is_onarg) {
		  values[i] = ext_ptr->arg;
		} else {
		  values[i] = ext_ptr->mod;
		}
		if (is_vc_max) {
		  while(ext_ptr->down) {
		    ext_ptr = ext_ptr->down;
		    if (ext_ptr->mod > values[i]) {
		      if (is_dx) {
			values[i] = fabs(ext_ptr->mod*cos(ext_ptr->arg));
		      } else if (is_dy) {
			values[i] = fabs(ext_ptr->mod*sin(ext_ptr->arg));
		      } else if (is_onarg) {
			values[i] = ext_ptr->arg;
		      } else {
			values[i] = ext_ptr->mod;
		      }
		    }
		  }
		}
		/* old expression (see above apply_fct_to_e_TclCmd_ )
		   values[i] = fct (values[i], scd_value);*/
		if (is_SK_logT) {
		  /*tmpvalue = fct1(q,    values[i]);*/
		  tmpvalue = evaluator_evaluate_x_y(fct1, q,    values[i]);
		  /*values[i] = fct2(qtsa, tmpvalue)*log(fabs(values[i]));*/
		  values[i] = evaluator_evaluate_x_y(fct2, qtsa, tmpvalue)*log(fabs(values[i]));
		} else {
		  /*tmpvalue = fct1(q,    values[i]);*/
		  tmpvalue = evaluator_evaluate_x_y(fct1, q,    values[i]);
		  /*values[i] = fct2(qtsa, tmpvalue);*/
		  values[i] = evaluator_evaluate_x_y(fct2, qtsa, tmpvalue);
		}
		i++;
	      } /* matches if(!is_arg...      */
	    }   /* matches if ((x > x1) && ...*/
	  }     /* */
	}       /* foreach ext_ptr ...        */
      }         /* foreach line ...           */
      
    } else {
      if (is_arg) {
	nb_of_values = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  posx = pos % ext_image->lx;
	  posy = pos / ext_image->lx;
	  if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) { 
	    if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil))
	      nb_of_values++;
	  }
	}
	values = (double *) malloc (nb_of_values*sizeof (double));
	j = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  posx = pos % ext_image->lx;
	  posy = pos / ext_image->lx;
	  if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) { 
	    if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil)) {
	      if (is_dx) {
		values[j] = fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg));
	      } else if (is_dy) {
		values[j] = fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg));
	      } else if (is_onarg) {
		values[j] = ext_image->extr[i].arg;
	      } else {
		values[j] = ext_image->extr[i].mod;
	      }
	      if (is_SK_logT) {
		/*tmpvalue = fct1(q,    values[j]);
		  values[j] = fct2(qtsa, tmpvalue)*log(fabs(values[j]));*/
		tmpvalue = evaluator_evaluate_x_y(fct1, q,    values[j]);
		values[j] = evaluator_evaluate_x_y(fct2, qtsa, tmpvalue)*log(fabs(values[j]));
	      } else {
		/*tmpvalue = fct1(q,    values[j]);
		  values[j] = fct2(qtsa, tmpvalue);*/
		tmpvalue = evaluator_evaluate_x_y(fct1, q,    values[j]);
		values[j] = evaluator_evaluate_x_y(fct2, qtsa, tmpvalue);
	      }
	      j++;
	    }
	  }
	}
      } else {
	nb_of_values = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  posx = pos % ext_image->lx;
	  posy = pos / ext_image->lx;
	  if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) { 
	    nb_of_values++;
	  }
	}
	/*nb_of_values = ext_image->extrNb;*/
	values = (double *) malloc (nb_of_values*sizeof (double));
	j=0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  pos = ext_image->extr[i].pos;
	  posx = pos % ext_image->lx;
	  posy = pos / ext_image->lx;
	  if ((posx > x1) && (posx < x2) && (posy > y1) && (posy < y2)) { 
	    if (is_dx) {
	      values[j] = fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg));
	    } else if (is_dy) {
	      values[j] = fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg));
	    } else if (is_onarg) {
	      values[j] = ext_image->extr[i].arg;
	    } else {
	      values[j] = ext_image->extr[i].mod;
	    }
	    if (is_SK_logT) {
	      /*tmpvalue = fct1(q,    values[j]);
		values[j] = fct2(qtsa, tmpvalue)*log(fabs(values[j]));*/
	      tmpvalue = evaluator_evaluate_x_y(fct1,q,    values[j]);
	      values[j] = evaluator_evaluate_x_y(fct2,qtsa, tmpvalue)*log(fabs(values[j]));
	    } else {
	      /*tmpvalue = fct1(q,    values[j]);
		values[j] = fct2(qtsa, tmpvalue);*/
	      tmpvalue = evaluator_evaluate_x_y(fct1,q,    values[j]);
	      values[j] = evaluator_evaluate_x_y(fct2,qtsa, tmpvalue);
	    }
	    j++;
	  }
	}
      }  
    }
  } else { /* !is_domain */
    if (is_vc || is_vc_max) {
      nb_of_values = 0;
      foreach (line_ptr, ext_image->line_lst) {
	foreach (ext_ptr, line_ptr->gr_lst) {
	  if (ext_ptr->down || ext_ptr->up) {
	    if (is_tag && !ext_ptr->next) {
	      continue;
	    }
	    if (is_notag && ext_ptr->next) {
	      continue;
	    }
	    if (!is_arg
		|| (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil))) {
	      nb_of_values++;
	    }
	  }
	}
      }
      values = (double *) malloc (nb_of_values*sizeof (double));
      i = 0;
      foreach (line_ptr, ext_image->line_lst) {
	foreach (ext_ptr, line_ptr->gr_lst) {
	  if (ext_ptr->down || ext_ptr->up) {
	    if (is_tag && !ext_ptr->next) {
	      continue;
	    }
	    if (is_notag && ext_ptr->next) {
	      continue;
	    }
	    if (!is_arg
		|| (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil))) {
	      if (is_dx) {
		values[i] = fabs(ext_ptr->mod*cos(ext_ptr->arg));
	      } else if (is_dy) {
		values[i] = fabs(ext_ptr->mod*sin(ext_ptr->arg));
	      } else if (is_onarg) {
		values[i] = ext_ptr->arg;
	      } else {
		values[i] = ext_ptr->mod;
	      }
	      if (is_vc_max) {
		while(ext_ptr->down) {
		  ext_ptr = ext_ptr->down;
		  if (ext_ptr->mod > values[i]) {
		    if (is_dx) {
		      values[i] = fabs(ext_ptr->mod*cos(ext_ptr->arg));
		    } else if (is_dy) {
		      values[i] = fabs(ext_ptr->mod*sin(ext_ptr->arg));
		    } else if (is_onarg) {
		      values[i] = ext_ptr->arg;
		    } else {
		      values[i] = ext_ptr->mod;
		    }
		  }
		}
	      }
	      /* old expression 
		 values[i] = fct (values[i], scd_value);*/
	      if (is_SK_logT) {
		/*tmpvalue = fct1(q,    values[i]);
		  values[i] = fct2(qtsa, tmpvalue)*log(fabs(values[i]));*/
		tmpvalue = evaluator_evaluate_x_y(fct1,q,    values[i]);
		values[i] = evaluator_evaluate_x_y(fct2,qtsa, tmpvalue)*log(fabs(values[i]));
	      } else {
		/*tmpvalue = fct1(q,    values[i]);
		  values[i] = fct2(qtsa, tmpvalue);*/
		tmpvalue = evaluator_evaluate_x_y(fct1,q,    values[i]);
		values[i] = evaluator_evaluate_x_y(fct2,qtsa, tmpvalue);
	      }
	      i++;
	    }
	  }
	}
      }
    } else {
      if (is_arg) {
	nb_of_values = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil))
	    nb_of_values++;
	}
	values = (double *) malloc (nb_of_values*sizeof (double));
	j = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil)) {
	    if (is_dx) {
	      values[j] = fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg));
	    } else if (is_dy) {
	      values[j] = fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg));
	    } else if (is_onarg) {
	      values[j] = ext_image->extr[i].arg;
	    } else {
	      values[j] = ext_image->extr[i].mod;
	    }
	    if (is_SK_logT) {
	      /*tmpvalue = fct1(q,    values[j]);
		values[j] = fct2(qtsa, tmpvalue)*log(fabs(values[j]));*/
	      tmpvalue = evaluator_evaluate_x_y(fct1,q,    values[j]);
	      values[j] = evaluator_evaluate_x_y(fct2,qtsa, tmpvalue)*log(fabs(values[j]));
	    } else {
	      /*tmpvalue = fct1(q,    values[j]);
		values[j] = fct2(qtsa, tmpvalue);*/
	      tmpvalue = evaluator_evaluate_x_y(fct1,q,    values[j]);
	      values[j] = evaluator_evaluate_x_y(fct2,qtsa, tmpvalue);
	    }
	    j++;
	  }
	}
      } else {
	nb_of_values = ext_image->extrNb;
	values = (double *) malloc (ext_image->extrNb*sizeof (double));
	for (i = 0; i < ext_image->extrNb; i++) {
	  if (is_dx) {
	    values[i] = fabs(ext_image->extr[i].mod*cos(ext_image->extr[i].arg));
	  } else if (is_dy) {
	    values[i] = fabs(ext_image->extr[i].mod*sin(ext_image->extr[i].arg));
	  } else if (is_onarg) {
	    values[i] = ext_image->extr[i].arg;
	  } else {
	    values[i] = ext_image->extr[i].mod;
	  }
	  if (is_SK_logT) {
	    /*tmpvalue = fct1(q,    values[i]);
	      values[i] = fct2(qtsa, tmpvalue)*log(fabs(values[i]));*/
	    tmpvalue = evaluator_evaluate_x_y(fct1,q,    values[i]);
	    values[i] = evaluator_evaluate_x_y(fct2,qtsa, tmpvalue)*log(fabs(values[i]));
	  } else {
	    /*tmpvalue = fct1(q,    values[i]);
	      values[i] = fct2(qtsa, tmpvalue);*/
	    tmpvalue = evaluator_evaluate_x_y(fct1,q,    values[i]);
	    values[i] = evaluator_evaluate_x_y(fct2,qtsa, tmpvalue);
	  }
	}
      }
    }
  }
  if (nb_of_values > 2)
    _QuickSort_ (values-1, nb_of_values);
  for (i = 0; i < nb_of_values; i++)
    result += values[i];
  
  /*dfclose (fct1);
    dfclose (fct2);*/
  evaluator_destroy(fct1);
  evaluator_destroy(fct2);
  free (values);
  if (result == 0)
    sprintf (interp->result, "0");
  else
    sprintf (interp->result, "%.15e", result);
  return TCL_OK;
}


/***********************************
 *  command name in xsmurf : efct3D
 ***********************************/
/* created by pierre kestener : september 16th 2002 */
/* see apply_fct_to_e_TclCmd_, here we do
   not load the file into memory, because 
   files can be as LARGE as hundredth of
   MegaBytes for 512^3 data */
/* modified by pierre kestener : 06/07/2006 -> option -mask */

int
apply_fct_to_extima3D_TclCmd_ (ClientData clientData,
			       Tcl_Interp *interp,
			       int        argc,
			       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "ss[f]",
    "-mask","J",
    NULL
  };

  char * help_msg =
  {
    (" Applies a function to each modulus of an ext image 3D (specified\n"
     " by its filename) and sums all the "
     " values. The function can have two parameters.\n"
     "\n"
     "Parameters:\n"
     "  string    - filename of the 3D ext image to treat.\n"
     "  string    - an expression designing the function (defunc syntax).\n"
     "  [real]    - facultative value of a the second parameter of the\n"
     "              function.\n"
     "Options:\n"
     " -mask[J]: mask computation.\n"
     "\n"
     "command defined in interpreter/wt2d_cmds.c by fct apply_fct_to_extima3D_TclCmd_ \n")
  };

  /* Command's parameters */
  char     *fct_expr;
  real     scd_value = 0;
  char     * extImageFilename;
  FILE     * fileIn;
  char       tempBuffer[100], saveFormat[100], type[100];
  int        lx, ly, lz, size, realSize, intSize;
  real       scale;
  int        tmp_pos;
  float      tmp_mod;

  /* Options's presence */
  int is_mask=0;

  /* Options's parameters */
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
  
  if (arg_get (0, &extImageFilename, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

  is_mask = arg_present (1);
  if (is_mask)
    if (arg_get (1, &mask) == TCL_ERROR)
      return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (!(fileIn = fopen(extImageFilename, "r")))
    return GenErrorAppend(interp, "Couldn't open `", extImageFilename,
			  "' for reading.", NULL);
  
  fgets(tempBuffer, 100, fileIn);
  sscanf(tempBuffer, "%s %s %dx%dx%d %d %f (%d byte reals, %d", 
	 saveFormat, type, &lx, &ly, &lz, &size, &scale, &realSize, &intSize);
  
  /*fct = dfopen (fct_expr);*/
  fct = evaluator_create(fct_expr);  
  if (!fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }
  
  /* Treatement */
  nb_of_values = size;
  values = (double *) malloc (nb_of_values*sizeof (double));
  if (!is_mask) {
    for (i = 0; i < nb_of_values; i++) {
      fread(&tmp_pos, sizeof(int),   1, fileIn);
      fread(&tmp_mod, sizeof(float), 1, fileIn);
      /*values[i] = fct (tmp_mod, scd_value);*/
      values[i] = evaluator_evaluate_x_y(fct, tmp_mod, scd_value);
    }
  } else {
    for (i = 0; i < nb_of_values; i++) {
      fread(&tmp_pos, sizeof(int),   1, fileIn);
      fread(&tmp_mod, sizeof(float), 1, fileIn);
      if ( mask->data[tmp_pos] > 0.0 || mask->data[tmp_pos] < 0.0 ) { 
	/*values[i] = fct (tmp_mod, scd_value);*/
	values[i] = evaluator_evaluate_x_y(fct, tmp_mod, scd_value);
      } else {
	values[i] = 0.0;
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
  fclose(fileIn);

  if (result == 0)
    sprintf (interp->result, "0");
  else
    sprintf (interp->result, "%.15e", result);
  return TCL_OK;
}

/***********************************
 *  command name in xsmurf : ifct3D
 ***********************************/
/* created by pierre kestener : september 16th 2002 */
/* see apply_fct_to_extimma3D_TclCmd_*/
int
apply_fct_to_data3D_TclCmd_ (ClientData clientData,
			     Tcl_Interp *interp,
			     int        argc,
			     char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "ss[f]",
    "-coarse", "dd",
    "-normalize", "",
    "-subsample", "d",
    "-keep_up", "f",
    "-keep_low", "f",
    NULL
  };

  char * help_msg =
  {
    (" Applies a function to each values of 3D float data (specified\n"
     " by its filename) and sums all the "
     " values. The function can have two parameters.\n"
     "\n"
     "Parameters:\n"
     "  string    - filename of the 3D float data to treat.\n"
     "  string    - an expression designing the function (defunc syntax).\n"
     "  [real]    - facultative value of a the second parameter of the\n"
     "              function.\n"
     "Options:\n"
     "  -coarse [int cubesize depth] : apply function to a coarse grained \n"
     "                version of\n the 3D data (divided into 8^depth cells\n"
     "                of size 2^depth)\n"
     "                If your data are 512^3, cube_size must be 512\n"
     "  -normalize   : only active together with -coarse option\n"
     "                 normalize computations by cell_size\n"
     "  -subsample [dd]  : takes into account only points that are spaced\n"
     "                    by an integer given as first parameter.\n"
     "                    Size of the cube along x or y, or z direction is\n"
     "                    given as a second parameter.\n"
     "  -keep_up [f] : takes into account only points that are above\n"
     "                 the threshold given as a parameter\n"
     "  -keep_low [f] \n"
     "\n"
     "command defined in interpreter/wt2d_cmds.c by fct apply_fct_to_extima3D_TclCmd_ \n")
  };

  /* Command's parameters */
  char     *fct_expr;
  real     scd_value = 0;
  char     * dataFilename;
  FILE     * fileIn;
  //real       scale;
  float      tmp_data;

  /* Options's presence */
  int isCoarse;
  int isNormalize;
  int isSubsample;
  int isKeepup;
  int isKeeplow;
 
  /* Options's parameters */
  int cube_size, depth;

  /* Other variables */
  /*double   (*fct)();*/
  void     *fct;
  double   result = 0.0;
  double   *values;
  int      i;
  int      nb_of_values;
  int      sub_spaced, size_cube;
  float    thresh;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &dataFilename, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

  isCoarse = arg_present (1);
  if (isCoarse)
    if (arg_get (1, &cube_size, &depth) == TCL_ERROR)
      return TCL_ERROR;
  isNormalize = arg_present (2);
  isSubsample = arg_present (3);
  if (isSubsample)
    if (arg_get (3, &sub_spaced, &size_cube) == TCL_ERROR)
      return TCL_ERROR;
  isKeepup = arg_present (4);
  if (isKeepup)
    if (arg_get (4, &thresh) == TCL_ERROR)
      return TCL_ERROR;
  if (isKeeplow)
    if (arg_get (5, &thresh) == TCL_ERROR)
      return TCL_ERROR;
  
  /* Parameters validity and initialisation */
  if (!(fileIn = fopen(dataFilename, "r")))
    return GenErrorAppend(interp, "Couldn't open `", dataFilename,
			  "' for reading.", NULL);
  
  /*fct = dfopen (fct_expr);*/
  fct = evaluator_create(fct_expr);
  if (!fct)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ",
			fct_expr, (char *) NULL);
      return TCL_ERROR;
    }

  if (depth < 0) {
    return GenErrorAppend(interp, "Parameter \"depth\" can't be negative!.\n", NULL);
  }
  
  /* Treatement */
  nb_of_values = 0;
  while (fread(&tmp_data, sizeof(float), 1, fileIn)==1)
    nb_of_values++;
  fseek(fileIn,0,SEEK_SET);

  if (isCoarse) {
    int nb_cells;
    int x,y,z;
    int xx,yy,zz;
    int lx, ly, lz;
    int j;
    int cell_size = (int) pow(2.0,(double) depth);
    int cell_edge;

    lx = ly = lz = cube_size;
    
    nb_cells = (int) pow(8.0,(double) depth);
    cell_edge = cube_size / cell_size;

    if (nb_of_values > cube_size*cube_size*cube_size) 
      return GenErrorAppend(interp, "Parameter \"cube_size\" is false or your data corrupted!.\n", NULL);
    if (nb_cells > nb_of_values)
      return GenErrorAppend(interp, "Parameter \"depth\" is too high!.", NULL);
    
    /* everything is OK, let's hope */
    values = (double *) malloc (nb_cells*sizeof (double));
    for (j = 0; j < nb_cells; j++)
      values[j] = 0.0;
   
    if (isKeepup)
      for (i = 0; i < nb_of_values; i++) {
	fread(&tmp_data, sizeof(float), 1, fileIn);
	x  = i % lx;
	y  = (i / lx) % ly;
	z  = (i / lx) / ly;
	xx = x / cell_edge;
	yy = y / cell_edge;
	zz = z / cell_edge;
	j = xx + yy*cell_size + zz*cell_size*cell_size;
	if (tmp_data>thresh) {
	  values[j] += (double) tmp_data;
	} else {
	  values[j] += (double) 0.0;
	}
      }
    else if (isKeeplow)
      for (i = 0; i < nb_of_values; i++) {
	fread(&tmp_data, sizeof(float), 1, fileIn);
	x  = i % lx;
	y  = (i / lx) % ly;
	z  = (i / lx) / ly;
	xx = x / cell_edge;
	yy = y / cell_edge;
	zz = z / cell_edge;
	j = xx + yy*cell_size + zz*cell_size*cell_size;
	if (tmp_data<=thresh) {
	  values[j] += (double) tmp_data;
	} else {
	  values[j] += (double) 0.0;
	}
      }
    else
      for (i = 0; i < nb_of_values; i++) {
	fread(&tmp_data, sizeof(float), 1, fileIn);
	x  = i % lx;
	y  = (i / lx) % ly;
	z  = (i / lx) / ly;
	xx = x / cell_edge;
	yy = y / cell_edge;
	zz = z / cell_edge;
	j = xx + yy*cell_size + zz*cell_size*cell_size;
	values[j] += (double) tmp_data;
      }
    
    if (isNormalize) {
      for (j = 0; j < nb_cells; j++) {
	values[j] = values[j]/cell_edge/cell_edge/cell_edge;
	/*values[j] = fct (values[j], scd_value);*/
	values[j] = evaluator_evaluate_x_y(fct, values[j], scd_value);
      }
   } else {
     for (j = 0; j < nb_cells; j++) {
       /*values[j] = fct (values[j], scd_value);*/
       values[j] = evaluator_evaluate_x_y(fct, values[j], scd_value);
     }
   }
     
    if (nb_cells > 2)
      _QuickSort_ (values-1, nb_cells);

    for (j = 0; j < nb_cells; j++)
      result += values[j];

    free(values);

  } else {
    if (isSubsample) {
      /* a terminer !!!*/
      int sub_nb_of_values;
      int tmpx, tmpy, tmpz;
      sub_nb_of_values = size_cube / sub_spaced;
      sub_nb_of_values ++;
      sub_nb_of_values = sub_nb_of_values*sub_nb_of_values*sub_nb_of_values;
      values = (double *) malloc (sub_nb_of_values*sizeof (double));
      for (i = 0; i < nb_of_values; i++) {
	fread(&tmp_data, sizeof(float), 1, fileIn);
	tmpx = i  % size_cube; 
	/*values[i] = fct (tmp_data, scd_value);*/
 	values[i] = evaluator_evaluate_x_y(fct, tmp_data, scd_value);
     }
    } else {
      values = (double *) malloc (nb_of_values*sizeof (double));
      for (i = 0; i < nb_of_values; i++) {
	fread(&tmp_data, sizeof(float), 1, fileIn);
	/*values[i] = fct (tmp_data, scd_value);*/
	values[i] = evaluator_evaluate_x_y(fct, tmp_data, scd_value);
      }
      
      if (nb_of_values > 2)
	_QuickSort_ (values-1, nb_of_values);
      for (i = 0; i < nb_of_values; i++)
	result += values[i];
    }

    free(values);
  }
  
  
  /*dfclose (fct);*/
  evaluator_destroy(fct);
  //free (values);
  fclose(fileIn);
  
  if (result == 0)
    sprintf (interp->result, "0");
  else
    sprintf (interp->result, "%.15e", result);
  return TCL_OK;
}

/*
 */
int
old_apply_fct_to_e_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es[f]",
    "-vc", "",
    "-vc_max", "",
    "-arg", "ff",
    NULL
  };

  char * help_msg =
  {
    (" Applies a function to each modulus of an ext image and sums all the\n"
     "values. The function can have two parameters.\n"
     "\n"
     "Parameters :\n"
     "  ext image - ext image to treat.\n"
     "  string    - an expression designing the function (defunc syntax).\n"
     "  [real]    - facultative value of a the second parameter of the\n"
     "              function.\n"
     "Options :\n"
     "  -vc : Applies the function only on maxima that are on a vertical\n"
     "    chain.\n"
     "  -vc_max : Same as \"-vc\" but take the maxmima of the modulus down\n"
     "    the chain.\n"
     "  -arg : Applies the function only on maxima which gradient argument is\n"
     "    near a given value.\n"
     "      float - value to use.\n"
     "      float - blblblblbblblbl.")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  char     *fct_expr;
  real     scd_value = 0;

  /* Options's presence */
  int is_vc;
  int is_vc_max;
  int is_arg;

  /* Options's parameters */
  real arg;
  real arg_simil;

  /* Other variables */
  /*double   (*fct)();*/
  void     *fct;
  double   result = 0.0;
  double   *values;
  int      i, j;
  int      nb_of_values;
  Line     *line_ptr;
  Extremum *ext_ptr;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &fct_expr, &scd_value) == TCL_ERROR)
    return TCL_ERROR;

  is_vc = arg_present (1);
  is_vc_max = arg_present (2);
  is_arg = arg_present (3);
  if (is_arg)
    if (arg_get (3, &arg, &arg_simil) == TCL_ERROR)
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
  if (is_arg && (arg_simil > 1.0 || arg_simil < 0.0))
    {
      sprintf (interp->result,
	       "Bad value for arg_simil (%f). Must be between 0.0 and 1.0", arg_simil);
      return TCL_ERROR;
    }

  /* Treatement */
  if (is_vc || is_vc_max)
    {
      nb_of_values = 0;
      foreach (line_ptr, ext_image->line_lst)
	foreach (ext_ptr, line_ptr->gr_lst)
	{
	  if (ext_ptr->down || ext_ptr->up)
	    if (!is_arg
		|| (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil)))
	      nb_of_values++;
	}
      values = (double *) malloc (nb_of_values*sizeof (double));
      i = 0;
      foreach (line_ptr, ext_image->line_lst)
	foreach (ext_ptr, line_ptr->gr_lst)
	{
	  if (ext_ptr->down || ext_ptr->up)
	    if (!is_arg
		|| (is_arg && _arg_is_near_ (ext_ptr->arg, arg, arg_simil)))
	      {
		values[i] = ext_ptr->mod;
		if (is_vc_max)
		  {
		    while(ext_ptr->down)
		      {
			ext_ptr = ext_ptr->down;
			if (ext_ptr->mod > values[i])
			  values[i] = ext_ptr->mod;
		      }
		  }
		/*values[i] = fct (values[i], scd_value);*/
		values[i] = evaluator_evaluate_x_y(fct, values[i], scd_value);
		i++;
	      }
	}
    }
  else
    {
      if (is_arg) {
	nb_of_values = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil))
	      nb_of_values++;
	}
	values = (double *) malloc (nb_of_values*sizeof (double));
	j = 0;
	for (i = 0; i < ext_image->extrNb; i++) {
	  if (_arg_is_near_ (ext_image->extr[i].arg, arg, arg_simil)) {
	    /*values[j] = fct (ext_image->extr[i].mod, scd_value);*/
	    values[j] = evaluator_evaluate_x_y(fct, ext_image->extr[i].mod, scd_value);
	    j++;
	  }
	}
      } else {
	nb_of_values = ext_image->extrNb;
	values = (double *) malloc (ext_image->extrNb*sizeof (double));
	for (i = 0; i < ext_image->extrNb; i++) {
	  /*values[i] = fct (ext_image->extr[i].mod, scd_value);*/
	  values[i] = evaluator_evaluate_x_y(fct, ext_image->extr[i].mod, scd_value);
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


/********************************
  Command name in xsmurf : l2m
  *******************************/
int
line_2_max_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es",
    NULL
  };

  char * help_msg =
  {
    (" Create a new ext image with the local maxima of each line\n"
     " (that are on a vertical chain)\n"
     "\n"
     "Parameters :\n"
     "  ext image - ext image to treat.\n"
     "  string    - name of the resulting ext image.")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  char     *name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  ExtImage *new_ext_image;
  Line     *line_ptr;
  Extremum *ext;
  Extremum *gr_ext;
  int      i;
  int      nb_of_gr = 0;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  foreach (line_ptr, ext_image->line_lst)
    foreach (gr_ext, line_ptr->gr_lst)
    {
      if (gr_ext->up || gr_ext->down)
	nb_of_gr ++;
    }

  new_ext_image = w2_ext_new (nb_of_gr,
			      ext_image->lx,
			      ext_image->ly,
			      ext_image->scale);

  i = 0;
  foreach (line_ptr, ext_image->line_lst)
    {
      foreach (gr_ext, line_ptr->gr_lst)
	{
	  if (gr_ext->up || gr_ext->down)
	    {
	      ext = &new_ext_image->extr[i];
	      ext->mod = gr_ext->mod;
	      ext->arg = gr_ext->arg;
	      ext->pos = gr_ext->pos;
	      i++;
	    }
	}
    }

  ExtDicStore (name, new_ext_image);

  return TCL_OK;
}



/* A mettre ailleur... Utilise pour search_single_max2. */

int min_max_flag;
enum {
  _MAX_,
  _MIN_
};

/*********************************
  Command name in xsmurf : ssm
  ********************************/
int
search_single_max_TclCmd_ (ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "E",
    "-new", "s",
    "-eps", "f",
    "-middle", "",
    "-smooth", "",
    "-greatest", "",
    "-min", "",
    NULL
  };

  char * help_msg =
  {
    (" Search the local maxima of each line of an ext image.\n"
     "\n"
     "Parameters :\n"
     "  ext image - ext image to treat.\n"
     "Options :\n"
     "  -new [s] : create a new ext image with this maxima.\n"
     "    string - name of the new ext image.\n"
     "  -eps [f] : epsilon ???\n"
     "  -middle  : only with eps option\n"
     "  -smooth  : remove down pics\n"
     "  -greatest\n"
     "  -min : only with greatest option.\n")
  };

  /* Command's parameters */
  ExtImage *ext_image;

  /* Options's presence */
  int is_new;
  int isEps;
  int isMiddle;
  int isSmooth;
  int isGreatest;
  int isMin;

  /* Options's parameters */
  char *name;
  real zeEps = 0;

  /* Other variables */
  ExtImage *new_ext_image;
  int      i;
  Line     *line_ptr;
  Extremum *ext_ptr;
  Extremum *ext1_ptr;
  Extremum *ext2_ptr;
  Extremum *ext3_ptr;
  Extremum *ext4_ptr;
  Extremum *new_ext_ptr;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image) == TCL_ERROR)
    return TCL_ERROR;

  is_new = arg_present(1);
  if (is_new)
    if (arg_get (1, &name)==TCL_ERROR)
      return TCL_ERROR;

  isEps = arg_present(2);
  if (isEps)
    if (arg_get (2, &zeEps)==TCL_ERROR)
      return TCL_ERROR;

  isMiddle = arg_present(3);
  isSmooth = arg_present(4);
  isGreatest = arg_present(5);
  isMin = arg_present(6);

  /* Parameters validity and initialisation */

  if (isMin) {
    min_max_flag = _MIN_;
  } else {
    min_max_flag = _MAX_;
  }

  /* Treatement */

  if (isSmooth) {
    /*
     * We remove "down-pics".
     */
    real m1;
    real m2;
    real m3;
    real m4;

    foreach (line_ptr, ext_image->line_lst) {
      ext1_ptr = 0;
      ext2_ptr = 0;
      ext3_ptr = 0;
      foreach (ext4_ptr, line_ptr->ext_lst) {
	if (ext1_ptr && ext2_ptr && ext3_ptr) {
	  m1 = ext1_ptr->mod;
	  m2 = ext2_ptr->mod;
	  m3 = ext3_ptr->mod;
	  m4 = ext4_ptr->mod;
	  /*
	  if (m1 >= m2 && m2 < m3 && m3 > m4) {
	    ext2_ptr->mod = (m1+m3)/2;
	  } else if (m1 < m2 && m2 >m3 && m3 <= m4) {
	    ext3_ptr->mod = (m2+m4)/2;
	  }
	  */
	  if (m1 > m2 && m2 < m3) {
	    ext2_ptr->mod = (m1+m3)/2;
	  }
	}
	ext1_ptr = ext2_ptr;
	ext2_ptr = ext3_ptr;
	ext3_ptr = ext4_ptr;
      }

      /*
       * Update greatest and smaller extrema of the line.
       */

      line_ptr->greater_ext = 0;
      line_ptr->smaller_ext = 0;
      foreach (ext1_ptr, line_ptr->ext_lst) {
	if (!line_ptr->greater_ext
	    || (ext1_ptr->mod) > (line_ptr->greater_ext->mod)) {
	  line_ptr->greater_ext = ext1_ptr;
	}

	if (!line_ptr->smaller_ext
	    || (ext1_ptr->mod) < (line_ptr->smaller_ext->mod)) {
	  line_ptr->smaller_ext = ext1_ptr;
	}
      }      
    }
  }


  if (isGreatest) {
    if (!isMin) {
      foreach (line_ptr, ext_image->line_lst) {
	lst_add(line_ptr->greater_ext, line_ptr->gr_lst);
	line_ptr->nb_of_gr++;
      }
    } else {
      foreach (line_ptr, ext_image->line_lst) {
	lst_add(line_ptr->smaller_ext, line_ptr->gr_lst);
	line_ptr->nb_of_gr++;
      }
    }
  } else if (isEps) {
    search_single_max2 (ext_image, zeEps, isMiddle);
  } else {
    search_single_max (ext_image);
  }

  if (is_new) {
    i = 0;
    foreach (line_ptr, ext_image->line_lst) {
      foreach (ext_ptr, line_ptr->gr_lst) {
	i++;
      }
    }
    new_ext_image = w2_ext_new (i,
				ext_image->lx,
				ext_image->ly,
				ext_image->scale);

    i = 0;
    foreach (line_ptr, ext_image->line_lst) {
      foreach (ext_ptr, line_ptr->gr_lst) {
	new_ext_ptr = &new_ext_image->extr[i];
	new_ext_ptr->mod = ext_ptr->mod;
	new_ext_ptr->arg = ext_ptr->arg;
	new_ext_ptr->pos = ext_ptr->pos;
	i++;
      }
    }

    ExtDicStore (name, new_ext_image);
  }

  return TCL_OK;
}



/**************************************
 * Command name in xsmurf : vchain2
 **************************************/
int
pt_to_pt_vert_chain_TclCmd_ (ClientData clientData,
			     Tcl_Interp *interp,
			     int        argc,
			     char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "EE[df]",
    "-first", "",
    NULL
  };

  char * help_msg =
  {
    (" Chain maxima of two ext images. The maxima must be isolated (i.e. no\n"
     "line).\n"
     "\n"
     "Parameters :\n"
     "  2 ext images - ext images to treat.\n"
     "  [integer]    - size of the search box.\n"
     "  [float]      - \"argument similitude\".")
  };

  /* Command's parameters */
  ExtImage *ext_im_do;
  ExtImage *ext_im_up;
  int       box_size = 5;
  float     arg_sim = 0.0;

  /* Options's presence */
  int is_first;

  /* Options's parameters */

  /* Other variables */

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_im_do, &ext_im_up, &box_size, &arg_sim) == TCL_ERROR)
    return TCL_ERROR;

  is_first = arg_present (1);

  /* Parameters validity and initialisation */

  /* Treatement */
  /*  down = (Extremum **) malloc (sizeof(Extremum*) * ext_im_do->lx * ext_im_do->ly);
  if(!down)
    exit(0);

  for (i=0;i<ext_im_do->lx*ext_im_do->ly;i++)
    down[i] = NULL;
  foreach (line_ptr, ext_im_do->line_lst)
    foreach (ext_ptr, line_ptr->gr_lst)
    {
      down[ext_ptr->pos] = ext_ptr;
    }
    */
  pt_to_pt_vert_chain (ext_im_do,
		       ext_im_up,
		       ext_im_do->lx,
		       ext_im_do->ly,
		       box_size,
		       arg_sim,
		       is_first);

  return TCL_OK;
}


/****************************************
 command name in xsmurf : vchain
 ****************************************/

int
mlm_vert_chain_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "EE[df]",
    "-first", "",
    NULL
  };

  char * help_msg =
  {
    (" Chain maxima lines maxima of two ext images.\n"
     "\n"
     " NB : vchain -> mlm_vert_chain_TclCmd_ in interpreter/wt2d_cmds.c\n"
     "    : mlm_vert_chain_TclCmd_ -> vert_chain in wt2d/Chain.c\n"
     "\n"
     "Parameters :\n"
     "  2 ext images - ext images to treat.\n"
     "  [integer]    - size of the search box.\n"
     "  [float]      - \"argument similitude\".")
  };

  /* Command's parameters */
  ExtImage *ext_im_do;
  ExtImage *ext_im_up;
  int       box_size = 5;
  float     arg_sim = 0.0;

  /* Options's presence */
  int is_first;

  /* Options's parameters */

  /* Other variables */

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_im_do, &ext_im_up, &box_size, &arg_sim) == TCL_ERROR)
    return TCL_ERROR;

  is_first = arg_present (1);

  /* Parameters validity and initialisation */

  /* Treatement */
  /*  down = (Extremum **) malloc (sizeof(Extremum*) * ext_im_do->lx * ext_im_do->ly);
  if(!down)
    exit(0);

  for (i=0;i<ext_im_do->lx*ext_im_do->ly;i++)
    down[i] = NULL;
  foreach (line_ptr, ext_im_do->line_lst)
    foreach (ext_ptr, line_ptr->gr_lst)
    {
      down[ext_ptr->pos] = ext_ptr;
    }
    */
  vert_chain (ext_im_do,
	      ext_im_up,
	      ext_im_do->lx,
	      ext_im_do->ly,
	      box_size,
	      arg_sim,
	      is_first);

  return TCL_OK;
}


/*
 */
int
purge_chains_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ed",
    NULL
  };

  char * help_msg =
  {
    (" Remove maxima that are on a chain that doesn't go to the smallest scale.\n"
     "Small scale defined by an integer index\n"
     "Use it after chain command !\n"
     "\n"
     "Parameters :\n"
     "  ext image - ext image to treat.")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  int      ind_of_ext_image; 

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Line     *line_ptr;
  Extremum *ext_ptr;
  Extremum *down_ext;
  int      i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &ind_of_ext_image) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  foreach (line_ptr, ext_image->line_lst)
    foreach (ext_ptr, line_ptr->gr_lst)
    {
      down_ext = ext_ptr;
      i = 0;
      while (down_ext->down)
	{
	  down_ext = down_ext ->down;
	  i++;
	}
      if (i != ind_of_ext_image)
	{
	  if (ext_ptr->down)
	    {
	      ext_ptr->down->up = NULL;
	      ext_ptr->down = NULL;
	    }
	  if (ext_ptr->up)
	    {
	      ext_ptr->up ->down = NULL;
	      ext_ptr->up = NULL;
	    }
	}
    }

  return TCL_OK;
}


/****************************************** 
 * Command name in xsmurf : vc2s
 ******************************************/
int
vert_chain_to_s_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Esdd",
    "-arg", "s",
    "-xy", "s",
    "-meanmod", "s",
    NULL
  };

  char * help_msg =
  {
    (" Convert a vertical chain to a signal (with the modulus of the gradient).\n"
     "\n"
     "Parameters :\n"
     "  ext image  - ext image to treat.\n"
     "  string     - name of the resulting signal.\n"
     "  2 integers - coordinate of a point. The vertical chain to treat is\n"
     "               the nearest this point.\n"
     "\n"
     "Options :\n"
     "  -arg : Convert the argument of the gradient to a signal.\n"
     "     string - name of the resulting signal.\n"
     "  -xy : Create a signal with the position (x, y).\n"
     "     string - name of the resulting signal.\n"
     "  -meanmod : Create a signal with meanmod computed on extrema\n"
     "     that are on the current horizontal line.\n"
     "     string - name of the resulting signal.\n"
     "\n"
     "Return value :\n"
     "  The position (x, y) of the lower extremity (lower scale) of the "
     "vertical chain. If the vc is tagged the string \"tagged\" is append to the result"
     "\n"
     "NB : command defined in interpreter/wt2d_cmds.c by\n"
     "vert_chain_to_s_TclCmd_\n")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  char     *name;
  int      x, y;

  /* Options's presence */
  int isArg;
  int isXy;
  int isMeanmod;

  /* Options's parameters */
  char *argName;
  char *xyName;
  char *meanmodName;

  /* Other variables */
  Signal   *result;
  Signal   *argResult;
  Signal   *xyResult;
  Signal   *meanmodResult;
  int      size = 1;
  int      i,j;
  int      pos;
  Extremum *ext_ptr;
  Line     *line;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &name, &x, &y) == TCL_ERROR)
    return TCL_ERROR;

  isArg = arg_present(1);
  if (isArg) {
    if (arg_get(1, &argName) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isXy = arg_present(2);
  if (isXy) {
    if (arg_get(2, &xyName) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isMeanmod = arg_present(3);
  if (isMeanmod) {
    if (arg_get(3, &meanmodName) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */

  /* Treatement */
  pos = x + y*ext_image->lx;
  ext_ptr = w2_closest_vc_ext (ext_image, pos);
  if (!ext_ptr) {
    sprintf (interp->result, "No extremum near (%d,%d)", x, y);
    return TCL_ERROR;
  }
  while (ext_ptr->up && ext_image->up) {
    ext_ptr = ext_ptr->up;
    ext_image = ext_image->up;
  }
  while (ext_ptr->down && ext_image->down) {
    ext_ptr = ext_ptr ->down;
    ext_image = ext_image->down;
    size ++;
  }
  if (size == 1) {
    sprintf (interp->result, "The extremum near (%d,%d) is not on a vertical chain.", x, y);
    return TCL_ERROR;
  }

  if (!ext_ptr->next) {
    sprintf(interp->result, "%d %d",
	    ext_ptr->pos%ext_image->lx,
	    ext_ptr->pos/ext_image->lx);
  } else {
    sprintf(interp->result, "%d %d tagged",
	    ext_ptr->pos%ext_image->lx,
	    ext_ptr->pos/ext_image->lx);
  }

  result = sig_new (REALY, 0, size - 1);
  if (isArg) {
    argResult = sig_new (REALY, 0, size - 1);
  }
  if (isXy) {
    xyResult = sig_new (REALXY, 0, size - 1);
  }
  if (isMeanmod) {
    meanmodResult = sig_new (REALY, 0, size - 1);
  }

  for (i = 0; i < size; i++) {
    result->dataY[i] = ext_ptr->mod;
    if (isArg) {
      argResult->dataY[i] = ext_ptr->arg;
    }
    if (isXy) {
      xyResult->dataX[i] = ext_ptr->pos%ext_image->lx;
      xyResult->dataY[i] = ext_ptr->pos/ext_image->lx;
    }

    if (isMeanmod) {
      /* first we have to find on which line we are */
      
      /* vieux morceau de code : solution de bourrin -> je ne 
	 connaissais pas l'existence de  ExtMisClosestChain */
      /*foreach (line, ext_image->line_lst) {
        foreach (ext_ptr2, line->gr_lst) {
	  if (ext_ptr2->pos == ext_ptr->pos) {
	    meanmodResult->dataY[i] = line->mass / line->size;
	    break;
	  }
	}
      }*/
      
      ExtMisClosestChain(ext_image, ext_ptr->pos, &j, (void *) &line);
      if (!line) {
	sprintf (interp->result, "No line near (%d,%d)", ext_ptr->pos%ext_image->lx, ext_ptr->pos/ext_image->lx);
	return TCL_ERROR;
      }
      meanmodResult->dataY[i] = line->mass / line->size;

    }
    /* attention : pas tres catholique */
    /* LA LIGNE SUIVANTE NE SERT  QUE POUR L'OPTION MEANMOD
       TANDIS QUE LA SUIVANTE EST NECESSAIRE POUR TOUTES LES
       OPTIONS ! */
    ext_image = ext_image->up;
    ext_ptr = ext_ptr->up;
  }

  store_signal_in_dictionary (name, result);
  if (isArg) {
    store_signal_in_dictionary (argName, argResult);
  }
  if (isXy) {
    store_signal_in_dictionary (xyName, xyResult);
  }
  if (isMeanmod) {
    store_signal_in_dictionary (meanmodName, meanmodResult);
  }

  return TCL_OK;
}


real
_get_best_on_vc_ (Extremum* ext_ptr,
		  Extremum** best_ext_ptr,
		  real      scale0,
		  real      dscale)
{
  real best_mod = 0.0;
  real best_scale;
  real current_scale = scale0;

  *best_ext_ptr = NULL;

  while (ext_ptr->up) {
    if (ext_ptr->down
	&& ext_ptr->mod > ext_ptr->down->mod
	&& ext_ptr->mod > ext_ptr->up->mod
	&& ext_ptr->mod > best_mod) {
      *best_ext_ptr = ext_ptr;
      best_mod = (*best_ext_ptr)->mod;
      best_scale = current_scale;
    }
    ext_ptr = ext_ptr->up;
    current_scale += dscale;
  }

  return best_scale;
}

/************************************
 * Command name in xsmurf : egbm
 ************************************/
int
get_best_max_on_vc_TclCmd_ (ClientData clientData,
			    Tcl_Interp *interp,
			    int        argc,
			    char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es",
    NULL
  };

  char * help_msg =
  {
    (" Extima Get Best Max.\n"
     "\n"
     "Parameters :\n"
     "  ext image  - ext image to treat.\n"
     "  string     - name of the resulting signal.\n"
     "  2 integers - coordinate of a point. The vertical chain to treat is\n"
     "               the nearest this point.")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  char     *name;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  ExtImage *new_ext_image;
  int      i;
  real     scale;
  int      nb_of_ext = 0;
  Extremum *ext_array;
  Extremum *ext_ptr;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  ext_array = (Extremum *) malloc (sizeof(Extremum)*ext_image->extrNb);

  for (i = 0; i < ext_image->extrNb; i++) {
    scale = _get_best_on_vc_ (&ext_image->extr[i], &ext_ptr, ext_image->scale, 1);
    if (ext_ptr) {
      ext_array[nb_of_ext].mod = scale;
      ext_array[nb_of_ext].arg = ext_ptr->mod;
      ext_array[nb_of_ext].pos = ext_ptr->pos;
      nb_of_ext++;
    }
  }

  new_ext_image = w2_ext_new(nb_of_ext, ext_image->lx, ext_image->ly, -1);
  for (i = 0; i < nb_of_ext; i++) {
      new_ext_image->extr[i].mod = ext_array[i].mod;
      new_ext_image->extr[i].arg = ext_array[i].arg;
      new_ext_image->extr[i].pos = ext_array[i].pos;
  }

  ExtDicStore (name, new_ext_image);

  return TCL_OK;
}


/**********************************
 * command name in xsmurf : einfo
 **********************************/
int
ext_get_info_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "E",
    NULL
  };

  char * help_msg =
  {
    (" Get info about an ext  image as a list. The list has the following order "
     ": scale, lx, ly, extrNb, chainNb, nb_of_lines and stamp.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage - ext image to treat.")
  };

  /* Command's parameters */
  ExtImage *ext;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  char string[100];

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  sprintf(string, "%g", ext->scale);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%d", ext->lx);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%d", ext->ly);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%d", ext->extrNb);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%d", ext->chainNb);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%d", ext->nb_of_lines);
  Tcl_AppendElement (interp, string);
  sprintf(string, "%d", ext->stamp);
  Tcl_AppendElement (interp, string);

  return TCL_OK;
}


/* *********************************************
 * Command name in xsmurf : foreache and eiloop
 * *********************************************/
int
foreache_TclCmd_ (ClientData clientData,
		      Tcl_Interp *interp,
		      int        argc,
		      char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es",
    NULL
  };

  char * help_msg =
  {
    (" Execute a script foreach point of an ext image. In this script the variables x, y, mod and arg refer to the coresponding fields of the point.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage - ext image to treat.\n"
     "  string   - script to execute.")
  };

  /* Command's parameters */
  ExtImage *extImage;
  char     *scriptStr;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int result;
  Extremum *currentExt;
  int extInd;
  Tcl_Obj *modStrObj, *modObj;
  Tcl_Obj *argStrObj, *argObj;
  Tcl_Obj *xStrObj, *xObj;
  Tcl_Obj *yStrObj, *yObj;
  Tcl_Obj *typeStrObj, *typeObj;
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
  argStrObj = Tcl_NewStringObj("arg", 3);
  argObj = Tcl_NewDoubleObj(0.0);
  xStrObj = Tcl_NewStringObj("x", 1);
  xObj = Tcl_NewLongObj(0);
  yStrObj = Tcl_NewStringObj("y", 1);
  yObj = Tcl_NewLongObj(0);
  typeStrObj = Tcl_NewStringObj("type", 4);
  typeObj = Tcl_NewStringObj("undefined", 9);

  while (1) {
    if (extInd == extImage->extrNb) {
      return TCL_OK;
    }
    currentExt = &extImage->extr[extInd];

    Tcl_SetDoubleObj(modObj, currentExt->mod);
    Tcl_ObjSetVar2(interp, modStrObj, NULL, modObj, 0);

    Tcl_SetDoubleObj(argObj, currentExt->arg);
    Tcl_ObjSetVar2(interp, argStrObj, NULL, argObj, 0);

    Tcl_SetLongObj(xObj, currentExt->pos%extImage->lx);
    Tcl_ObjSetVar2(interp, xStrObj, NULL, xObj, 0);

    Tcl_SetLongObj(yObj, currentExt->pos/extImage->lx);
    Tcl_ObjSetVar2(interp, yStrObj, NULL, yObj, 0);

    if (currentExt->up || currentExt->down) {
      Tcl_SetStringObj(typeObj, "vc", 2);
    } else {
      Tcl_SetStringObj(typeObj, "undefined", 9);
    }
    Tcl_ObjSetVar2(interp, typeStrObj, NULL, typeObj, 0);

    result = Tcl_Eval(interp, scriptStr);
    if ((result != TCL_OK) && (result != TCL_CONTINUE)) {
      if (result == TCL_ERROR) {
	char msg[60];
	sprintf(msg, "\n    (\"foreache\" body line %d)",interp->errorLine);
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


/*************************************
 * Command name in xsmurf : eigrloop
 *************************************/
int
gr_loop_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es",
    NULL
  };

  char * help_msg =
  {
    (" Execute a script foreach point of an ext image that is a local maxima "
     "along a contour line. In this script the variables x, y, mod and arg "
     "refer to the coresponding fields of the point.\n"
     "You can also use the following fields (only if the ext image\n"
     "is part of a skeleton (set of chained ext images)).\n"
     " - vclength\n"
     " - lmod (module at lowest scale)\n"
     " - umod (at uppest scale)\n"
     " - type : \"tag vc\" or \"vc\" or \"undefined\"\n"
     " - linemeanmod : mean module computed on the horizontal line\n"
     "             containing the extremun\n"
     " - locallinemeanmod : mean module computed on a neighbourhood of\n"
     "                  that line\n"
     " - bend : this values 1 when WT vector is locally convergent\n"
     "          and -1 if it is divergent\n"
     " - meanradius\n"
     " - meansquareradius\n"
     "\n"
     "Parameters :\n"
     "  ExtImage - ext image to treat.\n"
     "  string   - script to execute.")
  };

  /* Command's parameters */
  ExtImage *extImage;
  char     *scriptStr;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int      result;
  Extremum *currentExt,  *mobileExt;
  Line     *line;

  Tcl_Obj *modStrObj,              *modObj;
  Tcl_Obj *argStrObj,              *argObj;
  Tcl_Obj *xStrObj,                *xObj;
  Tcl_Obj *yStrObj,                *yObj;
  Tcl_Obj *vclengthStrObj,         *vclengthObj;
  Tcl_Obj *lmodStrObj,             *lmodObj;
  Tcl_Obj *umodStrObj,             *umodObj;  
  Tcl_Obj *typeStrObj,             *typeObj;
  Tcl_Obj *linemeanmodStrObj,      *linemeanmodObj;
  Tcl_Obj *locallinemeanmodStrObj, *locallinemeanmodObj;
  Tcl_Obj *bendStrObj,             *bendObj;
  Tcl_Obj *closedStrObj,           *closedObj;
  Tcl_Obj *meanradiusStrObj,       *meanradiusObj;
  Tcl_Obj *meansquareradiusStrObj, *meansquareradiusObj;

  int length = 0;
  double locallinemeanmod;
  int i;

  int x,y, x1,y1 ,x2,y2, xmid,ymid;
  //real xcenter,ycenter;
  double scalar_product;

  /* allocation for mobileExt */
  mobileExt = (Extremum *) malloc(1*sizeof(Extremum));


  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extImage, &scriptStr) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  modStrObj                  = Tcl_NewStringObj("mod", 3);
  modObj                       = Tcl_NewDoubleObj(0.0);
  argStrObj                  = Tcl_NewStringObj("arg", 3);
  argObj                       = Tcl_NewDoubleObj(0.0);
  xStrObj                    = Tcl_NewStringObj("xx", 2);
  xObj                         = Tcl_NewLongObj(0);
  yStrObj                    = Tcl_NewStringObj("yy", 2);
  yObj                         = Tcl_NewLongObj(0);
  vclengthStrObj             = Tcl_NewStringObj("vclength", 8);
  vclengthObj                  = Tcl_NewLongObj(0);
  lmodStrObj                 = Tcl_NewStringObj("lmod", 4);
  lmodObj                      = Tcl_NewDoubleObj(0.0);
  umodStrObj                 = Tcl_NewStringObj("umod", 4);
  umodObj                      = Tcl_NewDoubleObj(0.0);
  typeStrObj                 = Tcl_NewStringObj("type", 4);
  typeObj                      = Tcl_NewStringObj("undefined", 9);
  linemeanmodStrObj          = Tcl_NewStringObj("linemeanmod", 11);
  linemeanmodObj               = Tcl_NewDoubleObj(0.0);
  locallinemeanmodStrObj     = Tcl_NewStringObj("locallinemeanmod", 16);
  locallinemeanmodObj          = Tcl_NewDoubleObj(0.0);
  bendStrObj                 = Tcl_NewStringObj("bend", 4);
  bendObj                      = Tcl_NewDoubleObj(1.0);
  closedStrObj               = Tcl_NewStringObj("isallmostclosed", 15 );
  closedObj                    = Tcl_NewLongObj(0);
  meanradiusStrObj           = Tcl_NewStringObj("meanradius", 10 );
  meanradiusObj                = Tcl_NewDoubleObj(0.0);
  meansquareradiusStrObj     = Tcl_NewStringObj("meansquareradius", 16 );
  meansquareradiusObj          = Tcl_NewDoubleObj(0.0);


  foreach (line, extImage->line_lst) {
    /***********************
     * computing LINEMEANMOD 
     * MEANRADIUS
     * MEANSQUARERADIUS
     ***********************/
    Tcl_SetDoubleObj(linemeanmodObj, line->mass / line->size );
    Tcl_ObjSetVar2(interp, linemeanmodStrObj, NULL, linemeanmodObj, 0);
    
    Tcl_SetDoubleObj(meanradiusObj, line->meanradius );
    Tcl_ObjSetVar2(interp, meanradiusStrObj, NULL, meanradiusObj, 0);
    Tcl_SetDoubleObj(meansquareradiusObj, line->meansquareradius );
    Tcl_ObjSetVar2(interp, meansquareradiusStrObj, NULL, meansquareradiusObj, 0);
    
    foreach (currentExt, line->gr_lst) {
      Tcl_SetDoubleObj(modObj, currentExt->mod);
      Tcl_ObjSetVar2(interp, modStrObj, NULL, modObj, 0);

      Tcl_SetDoubleObj(argObj, currentExt->arg);
      Tcl_ObjSetVar2(interp, argStrObj, NULL, argObj, 0);

      Tcl_SetLongObj(xObj, currentExt->pos%extImage->lx);
      Tcl_ObjSetVar2(interp, xStrObj, NULL, xObj, 0);

      Tcl_SetLongObj(yObj, currentExt->pos/extImage->lx);
      Tcl_ObjSetVar2(interp, yStrObj, NULL, yObj, 0);

      /* ****************************************************/
      /* computing LOCALLINEMEANMOD and BEND using neighbour*/
      /* locations on the line                              */
      /* ****************************************************/
      locallinemeanmod = 0.0;
      
      if (0==1) { /* dummy test 
		   * we don't want to do the following lines !!!
		   */
	x = currentExt->pos%extImage->lx;
	y = currentExt->pos/extImage->lx;
	
	/* we set list->current to "currentExt" !!*/
	lst_set_current (currentExt, line->ext_lst);
	
	/*mobileExt = currentExt;
	  copy_extremum(currentExt,mobileExt);*/
	
	for (i=0; i<3; i++) {
	  mobileExt = lst_prev(line->ext_lst);
	}
	
	if (mobileExt) {
	  x1 = mobileExt->pos%extImage->lx;
	  y1 = mobileExt->pos/extImage->lx;
	} else {
	  scalar_product = 0.0;
	}
	
	for (i=-3; i<=3; i++) {
	  if (mobileExt) {
	    locallinemeanmod += mobileExt->mod;
	    mobileExt  =lst_next(line->ext_lst);
	  } else {
	    locallinemeanmod = 0.0;
	    break;
	  }
	}
	
	if (mobileExt) {
	  x2 = mobileExt->pos%extImage->lx;
	  y2 = mobileExt->pos/extImage->lx;
	} else {
	  scalar_product = 0.0;
	}
	
	xmid = (x1+x2)/2.0;
	ymid = (y1+y2)/2.0;
	
	scalar_product = (currentExt->mod * cos(currentExt->arg))*(xmid-x)
	  + (currentExt->mod*sin(currentExt->arg))*(ymid-y);
	
	if (scalar_product>0) {
	  Tcl_SetDoubleObj(bendObj, 1.0);
	} else if (scalar_product<0) {
	  Tcl_SetDoubleObj(bendObj, -1.0);
	} else {
	  Tcl_SetDoubleObj(bendObj, 0.0);
	}
	Tcl_ObjSetVar2(interp, bendStrObj, NULL, bendObj, 0);

	locallinemeanmod /= 7.0;
	Tcl_SetDoubleObj(locallinemeanmodObj, locallinemeanmod);
	Tcl_ObjSetVar2(interp, locallinemeanmodStrObj, NULL, locallinemeanmodObj, 0);
      } /* end of dummy loop */
      
      locallinemeanmod = 0.0;
      Tcl_SetDoubleObj(locallinemeanmodObj, locallinemeanmod);
      Tcl_ObjSetVar2(interp, locallinemeanmodStrObj, NULL, locallinemeanmodObj, 0);
      
      /*xcenter = (double) line->gravity_center_x;
	ycenter = (double) line->gravity_center_y;
	scalar_product = (double) (currentExt->mod * cos(currentExt->arg)*(xcenter-x) + currentExt->mod*sin(currentExt->arg)*(ycenter-y));

	scalar_product /= currentExt->mod;
	scalar_product /= sqrt((xcenter-x)*(xcenter-x)+(ycenter-y)*(ycenter-y));

      if (scalar_product>0.0) {
	Tcl_SetDoubleObj(bendObj, scalar_product);
      } else if (scalar_product<0.0) {
	Tcl_SetDoubleObj(bendObj, scalar_product);
      } else {
	Tcl_SetDoubleObj(bendObj, 0.0);
      }
      Tcl_ObjSetVar2(interp, bendStrObj, NULL, bendObj, 0);

      */
      Tcl_SetDoubleObj(bendObj, currentExt->bend);
      Tcl_ObjSetVar2(interp, bendStrObj, NULL, bendObj, 0);
      
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

      /* computing LMOD and UMOD */
      while (currentExt->down) {
	currentExt = currentExt->down;
      }
      Tcl_SetDoubleObj(lmodObj, currentExt->mod);
      Tcl_ObjSetVar2(interp, lmodStrObj, NULL, lmodObj, 0);
      while (currentExt->up) {
	currentExt = currentExt->up;
      }
      Tcl_SetDoubleObj(umodObj, currentExt->mod);
      Tcl_ObjSetVar2(interp, umodStrObj, NULL, umodObj, 0);

      /* computing TYPE */
      if (currentExt->up || currentExt->down) {
	if (currentExt->next) {
	  Tcl_SetStringObj(typeObj, "tag vc", 6);
	} else {
	  Tcl_SetStringObj(typeObj, "vc", 2);
	}
      } else {
	Tcl_SetStringObj(typeObj, "undefined", 9);
      }
      Tcl_ObjSetVar2(interp, typeStrObj, NULL, typeObj, 0);

      result = Tcl_Eval(interp, scriptStr);
      if ((result != TCL_OK) && (result != TCL_CONTINUE)) {
	if (result == TCL_ERROR) {
	  char msg[60];
	  sprintf(msg, "\n    (\"eigrloop\" body line %d)",interp->errorLine);
	  Tcl_AddErrorInfo(interp, msg);
	}
	break;
      }
    }
  }
  if (result == TCL_BREAK) {
    result = TCL_OK;
  }
  if (result == TCL_OK) {
    Tcl_ResetResult(interp);
  }
  return result;
}


/* *****************************************
 * Command name in xsmurf : eilineloop
 * *****************************************/
int
line_loop_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es",
    NULL
  };

  char * help_msg =
  {
    (" Execute a script foreach line of an ext image. In this script the "
     "variables size refer to the coresponding fields of the line.\n"
     "Fields are : size, nbgr, linemeanmod, xcenter, ycenter, posfirst.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage - ext image to treat.\n"
     "  string   - script to execute.\n")
  };

  /* Command's parameters */
  ExtImage *extImage;
  char     *scriptStr;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int      result;
  Line     *line;
  Tcl_Obj  *sizeStrObj,        *sizeObj;
  Tcl_Obj  *nbgrStrObj,        *nbgrObj;
  Tcl_Obj  *linemeanmodStrObj, *linemeanmodObj;
  Tcl_Obj  *xcenterStrObj, *xcenterObj;
  Tcl_Obj  *ycenterStrObj, *ycenterObj;
  Tcl_Obj  *areaStrObj, *areaObj;
  Tcl_Obj  *diameterStrObj, *diameterObj;
  Tcl_Obj  *perimeterStrObj, *perimeterObj;
  //Tcl_Obj  *xFirstExtStrObj, *xFirstextObj;
  //Tcl_Obj  *yFirstExtStrObj, *yFirstextObj;
  Tcl_Obj  *posFirstExtStrObj, *posFirstExtObj;
  Tcl_Obj *closedStrObj, *closedObj;
  Extremum *Extr;


  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extImage, &scriptStr) == TCL_ERROR)
    return TCL_ERROR;
  
  /* Parameters validity and initialisation */
  
  /* Treatement */
  sizeStrObj              = Tcl_NewStringObj("size", 4);
  sizeObj                 = Tcl_NewLongObj(0);
  nbgrStrObj              = Tcl_NewStringObj("nbgr", 4);
  nbgrObj                 = Tcl_NewLongObj(0);
  linemeanmodStrObj       = Tcl_NewStringObj("linemeanmod", 11);
  linemeanmodObj          = Tcl_NewDoubleObj(0.0);
  xcenterStrObj           = Tcl_NewStringObj("xcenter", 7);
  xcenterObj              = Tcl_NewDoubleObj(0.0);
  ycenterStrObj           = Tcl_NewStringObj("ycenter", 7);
  ycenterObj              = Tcl_NewDoubleObj(0.0);
  areaStrObj              = Tcl_NewStringObj("area", 4);
  areaObj                 = Tcl_NewDoubleObj(0.0);
  diameterStrObj          = Tcl_NewStringObj("diameter", 8);
  diameterObj             = Tcl_NewDoubleObj(0.0);
  perimeterStrObj         = Tcl_NewStringObj("perimeter", 9);
  perimeterObj            = Tcl_NewDoubleObj(0.0);
  posFirstExtStrObj       = Tcl_NewStringObj("posfirst", 8);
  posFirstExtObj          = Tcl_NewLongObj(0);
  closedStrObj = Tcl_NewStringObj("closed",6);
  closedObj = Tcl_NewLongObj(1);

  foreach (line, extImage->line_lst) {
    Tcl_SetLongObj(sizeObj, line->size);
    Tcl_ObjSetVar2(interp, sizeStrObj, NULL, sizeObj, 0);

    Tcl_SetLongObj(nbgrObj, line->nb_of_gr);
    Tcl_ObjSetVar2(interp, nbgrStrObj, NULL, nbgrObj, 0);

    Tcl_SetDoubleObj(linemeanmodObj, line->mass / line->size );
    Tcl_ObjSetVar2(interp, linemeanmodStrObj, NULL, linemeanmodObj, 0);

    Tcl_SetDoubleObj(areaObj, line->area );
    Tcl_ObjSetVar2(interp, areaStrObj, NULL, areaObj, 0);

    Tcl_SetDoubleObj(diameterObj, line->diameter );
    Tcl_ObjSetVar2(interp, diameterStrObj, NULL, diameterObj, 0);
    
    Tcl_SetDoubleObj(perimeterObj, line->perimeter );
    Tcl_ObjSetVar2(interp, perimeterStrObj, NULL, perimeterObj, 0);

    Tcl_SetDoubleObj(xcenterObj, line->gravity_center_x );
    Tcl_ObjSetVar2(interp, xcenterStrObj, NULL, xcenterObj, 0);

    Tcl_SetDoubleObj(ycenterObj, line->gravity_center_y );
    Tcl_ObjSetVar2(interp, ycenterStrObj, NULL, ycenterObj, 0);

    Tcl_SetLongObj(closedObj,1-line->type);
    Tcl_ObjSetVar2(interp,closedStrObj,NULL,closedObj,0);

    Extr = lst_first(line->ext_lst);

    Tcl_SetLongObj(posFirstExtObj, Extr->pos );
    Tcl_ObjSetVar2(interp, posFirstExtStrObj, NULL, posFirstExtObj, 0);

    result = Tcl_Eval(interp, scriptStr);
    if ((result != TCL_OK) && (result != TCL_CONTINUE)) {
      if (result == TCL_ERROR) {
	char msg[60];
	sprintf(msg, "\n    (\"lineloop\" body line %d)",interp->errorLine);
	Tcl_AddErrorInfo(interp, msg);
      }
      break;
    }
  }
  if (result == TCL_BREAK) {
    result = TCL_OK;
  }
  if (result == TCL_OK) {
    Tcl_ResetResult(interp);
  }
  return result;
}


/************************************
  Command name in xsmurf : l2s
  ***********************************/
int
line_to_s_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Esdd",
    "-arg", "s",
    "-xy", "s",
    "-octant", "s",
    "-theta", "s",
    NULL
  };

  char * help_msg =
  {
    (" Convert a line into a signal (with the modulus of the gradient).\n"
     "\n"
     "Parameters :\n"
     "  extima     - extima to treat.\n"
     "  string     - name of the resulting signal.\n"
     "  2 integers - coordinate of a point. The vertical chain to treat is\n"
     "               the nearest this point.\n"
     "\n"
     "Options :\n"
     "  -arg : Convert the argument of the gradient to a signal.\n"
     "     string - name of the resulting signal.\n"
     "  -xy : Convert the position of the gradient to a signal.\n"
     "     string - name of the resulting signal.\n"
     "  -theta : Convert the field \"theta\" to a signal.\n"
     "     string - name of the resulting signal.\n"
     "See source code in interpreter/wt2d_cmds.c.\n")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  char *name;
  int  x, y;

  /* Options's presence */
  int isArg;
  int isXy;
  int isOctant;
  int isTheta;

  /* Options's parameters */
  char *argName;
  char *xyName;
  char *octantName;
  char *thetaName;

  /* Other variables */
  Signal   *result;
  Signal   *argResult;
  Signal   *xyResult;
  Signal   *octantResult;
  Signal   *thetaResult;
  int      size = 1;
  int      i;
  int      pos;
  Extremum *ext_ptr;
  Line     *line;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &name, &x, &y) == TCL_ERROR)
    return TCL_ERROR;
  isArg = arg_present(1);
  if (isArg) {
    if (arg_get(1, &argName) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isXy = arg_present(2);
  if (isXy) {
    if (arg_get(2, &xyName) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isOctant = arg_present(3);
  if (isOctant) {
    if (arg_get(3, &octantName) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isTheta = arg_present(4);
  if (isTheta) {
    if (arg_get(4, &thetaName) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */

  /* Treatement */
  pos = x + y*ext_image->lx;
  ExtMisClosestChain(ext_image, pos, &i, (void *) &line);
  if (!line) {
      sprintf (interp->result, "No line near (%d,%d)", x, y);
      return TCL_ERROR;
    }
  size = line->size;

  result = sig_new (REALY, 0, size - 1);
  if (isArg) {
    argResult = sig_new (REALY, 0, size - 1);
  }
  if (isXy) {
    xyResult = sig_new (REALXY, 0, size - 1);
  }
  if (isOctant) {
    octantResult = sig_new (REALY, 0, size - 1);
  }
  if (isTheta) {
    thetaResult = sig_new (REALY, 0, size - 1);
  }

  i = 0;
  foreach(ext_ptr, line->ext_lst) {
    result->dataY[i] = ext_ptr->mod;
    if (isArg) {
      argResult->dataY[i] = ext_ptr->arg;
    }
    if (isXy) {
      xyResult->dataX[i] = ext_ptr->pos%ext_image->lx;
      xyResult->dataY[i] = ext_ptr->pos/ext_image->lx;
    }
    if (isOctant) {
      octantResult->dataY[i] = ext_ptr->octant;
    }
    if (isTheta) {
      thetaResult->dataY[i] = ext_ptr->theta;
    }
    i++;
  }
  store_signal_in_dictionary (name, result);
  if (isArg) {
    store_signal_in_dictionary (argName, argResult);
  }
  if (isXy) {
    store_signal_in_dictionary (xyName, xyResult);
  }
  if (isOctant) {
    store_signal_in_dictionary (octantName, octantResult);
  }
  if (isTheta) {
    store_signal_in_dictionary (thetaName, thetaResult);
  }

  return TCL_OK;
}


/*********************************
 * Command name in xsmurf : ecut
 *********************************/
int
e_cut_TclCmd_ (ClientData clientData,
	       Tcl_Interp *interp,
	       int        argc,
	       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Esdddd",
    NULL
  };

  char * help_msg =
  {
    (" Cut an ext image.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage   - ext image to cut.\n"
     "  string     - name of the result.\n"
     "  4 integers - coordinates of a rectangle that correspond to the new ext "
     "image.\n"
     "\n"
     "Return value :\n"
     "  None.")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  char     *name;
  int      x1, x2, y1, y2;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  ExtImage *result;
  int      nExt = 0;
  int      lx;
  int      ly;
  int      i;
  int      j;
  int      x;
  int      y;
  int      tmp;
  int      pos;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &name, &x1, &y1, &x2, &y2) == TCL_ERROR)
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
  if ((x2 < 0) || (y2 < 0) || (x1 >= ext_image->lx) || (y1 >= ext_image->ly)) {
    sprintf (interp->result, "bad cut domain");
    return TCL_ERROR;
  }
  if (x1 < 0) {
    x1 = 0;
  }
  if (y1 < 0) {
    y1 = 0;
  }
  if (x2 >= ext_image->lx) {
    x2 = ext_image->lx-1;
  }
  if (y2 >= ext_image->ly) {
    y2 = ext_image->ly-1;
  }

  /* Treatement */

  lx = x2 - x1;
  ly = y2 - y1;

  /* Count the number of extrema to put in the new image. */

  for (i = 0; i < ext_image->extrNb; i++) {
    pos = ext_image->extr[i].pos;
    x = pos % ext_image->lx;
    y = pos / ext_image->lx;
    if ((x > x1) && (x < x2) && (y > y1) && (y < y2)) {
      nExt++;
    }
  }
  result = w2_ext_new(nExt, lx, ly, ext_image->scale);

  /* Put the extrema in the new image. */

  j = 0;
  for (i = 0; i < ext_image->extrNb; i++) {
    pos = ext_image->extr[i].pos;
    x = pos % ext_image->lx;
    y = pos / ext_image->lx;
    if ((x > x1) && (x < x2) && (y > y1) && (y < y2)) {
      result->extr[j].pos = (x-x1) + (y-y1)*lx;
      result->extr[j].mod = ext_image->extr[i].mod;
      result->extr[j].arg = ext_image->extr[i].arg;
      j++;
    }
  }

  ExtDicStore(name, result);
  Tcl_AppendResult(interp, name, NULL);

  return TCL_OK;
}


/**********************************
  Command name in xsmurf : l2name
  *********************************/
int
name_line_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)

{
  char * options[] =
  {
    "Esdd",
    NULL
  };

  char * help_msg =
  {
    (" Give a name to a line. Be carreful using line names, because the smurf "
     "object that is created with this command is not destroyed when the "
     "associated structure is freed. Keep this in mind ! Ok ? This can lead to "
     "very unexpected results. You've been warned.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage   - ExtImage selectionned.\n"
     "  string     - Name of the line.\n"
     "  2 integers - coordinate of a point. (x,y)\n" 
     "      The vertical chain to treat is the nearest from this point.\n"
     "\n"
     "Return value :\n"
     "  Warning message."     )
  };
  
  /* Command's parameters */
  ExtImage *ext_image;
  int      x, y;
  char     *line_name;

  /* Other variables */
  int      i;
  int      pos;
  Line     *line;
  
  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &line_name, &x, &y) == TCL_ERROR)
    return TCL_ERROR;
    
  /* Treatement */
  pos = x + y*ext_image->lx;
  ExtMisClosestChain(ext_image, pos, &i, (void *) &line);
  if (!line) {
    sprintf (interp->result, "no line near (%d,%d)", x, y);
    return TCL_ERROR;
  } else {
    store_line_in_dictionary (line_name, line);
    sprintf (interp->result,
	     "referencing to lines with this command is very dangerous (see "
	     "help for this command)");
    return TCL_OK;
  }
}


/************************************************
  command name in xsmurf : s2ei
  ********************************************* */
int
xysig_2_ei_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ssdf",
    NULL
  };

  char * help_msg =
  {
    (" Create an ext image with a REALXY signal.\n"
     "\n"
     "Parameters :\n"
     "  Signal  - The signal.\n"
     "  string  - Name of the ext image.\n"
     "  integer - lx of the resulting ext image.\n"
     "  float   - Scale.")
  };

  /* Command's parameters */
  Signal *sig;
  char   *name;
  int    lx;
  real   scale;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  ExtImage *ext_image;
  int      i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &sig, &name, &lx, &scale) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (sig->type != REALXY) {
    sprintf (interp->result, "bad signal type");
    return TCL_ERROR;
  }

  /* Treatement */
  ext_image = w2_ext_new (sig->n, lx, 1, scale);
  if (!ext_image) {
    return GenErrorMemoryAlloc(interp);
  }

  for (i = 0; i < sig->n; i++) {
    ext_image->extr[i].pos = (int)sig->dataX[i];
    ext_image->extr[i].mod = sig->dataY[i];
    ext_image->extr[i].arg = 0.0;
  }

  ExtDicStore(name, ext_image);

  return TCL_OK;
}


/************************************************
  command name in xsmurf : mys2ei
  ********************************************* */
int
my_xysig_2_ei_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ssddf",
    NULL
  };

  char * help_msg =
  {
    (" Create an ext image with a REALXY signal.\n"
     "\n"
     "Parameters :\n"
     "  Signal  - The signal.\n"
     "  string  - Name of the ext image.\n"
     "  integer - lx of the resulting ext image.\n"
     "  integer - ly of the resulting ext image.\n"
     "  float   - Scale.\n"
     "Be careful that sig->dataX array must contain\n"
     " positions of the extrema : pos = x + lx * y  . \n")
  };

  /* Command's parameters */
  Signal *sig;
  char   *name;
  int    lx, ly;
  real   scale;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  ExtImage *ext_image;
  int      i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &sig, &name, &lx, &ly, &scale) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (sig->type != REALXY) {
    sprintf (interp->result, "bad signal type");
    return TCL_ERROR;
  }

  /* Treatement */
  ext_image = w2_ext_new (sig->n, lx, ly, scale);
  if (!ext_image) {
    return GenErrorMemoryAlloc(interp);
  }

  for (i = 0; i < sig->n; i++) {
    ext_image->extr[i].pos = (int)sig->dataX[i];
    ext_image->extr[i].mod = sig->dataY[i];
    ext_image->extr[i].arg = 0.0;
  }

  ExtDicStore(name, ext_image);

  return TCL_OK;
}


/************************************************
  command name in xsmurf : eisavelines
*************************************************/
/* created  the 26th july 2001*/
int
ei_save_lines_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "ESs",
    "-reverse", "s",
    NULL
  };

  char * help_msg =
  {
    (" Save some chains of an ext image. Chains are refered to by the\n" 
     "position of an extremun belonging to it.\n"
     "\n"
     "Parameters :\n"
     "  Extima  - The ext image to treat.\n"
     "  Signal  - The signal.\n"
     "  string  - Name of the ext image.\n"
     "Be careful that sig->dataX array must contain\n"
     " positions of the extrema : pos = x + lx * y  . \n"
     "\n"
     "Options:\n"
     "  -reverse: one removes the specified lines from the whole set\n"
     "            of vertical lines.\n")
  };

  /* Command's parameters */
  
  ExtImage *extImage_in;
  ExtImage *extImage_out;
  Extremum *ext, extonline;
  Line     *theline, *line;
  List     *theline_lst;
  Signal   *sig;
  char     *name;
  int      lx, ly;
  real     scale;
  int      thepos;
  Extremum *extr_selection;
  int      found, length;
 
  /* Options's presence */
  int      isReverse;
  /* Options's parameters */
  char     *name_reverse;
  int      length_reverse;
  List     *theline_lst_reverse;
  ExtImage *extImage_out_reverse;
  /* Other variables */
  int      i,j;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extImage_in, &sig, &name) == TCL_ERROR)
    return TCL_ERROR;

  isReverse = arg_present(1);
  if (arg_get (1, &name_reverse) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (sig->type != REALXY) {
    sprintf (interp->result, "bad signal type, must be REALXY");
    return TCL_ERROR;
  }

  extr_selection = (Extremum *) malloc(sig->n*sizeof(Extremum));

  /* Treatement */
  lx    = extImage_in->lx;
  ly    = extImage_in->ly;
  scale = extImage_in->scale;

  /* on va creer une liste de lines, deux a deux differentes
     pour eviter qu'il y ait plusieurs extrema pointant sur
     la meme line */
  theline_lst         = lst_create ();
  theline_lst_reverse = lst_create ();
  /* on initialise theline_lst_reverse avec les chaines de extImage_in */
  foreach (line, extImage_in->line_lst) {
    lst_add(line, theline_lst_reverse);
  }
  length = 0;

  if (sig->n <=0) {
    sprintf(interp->result,"ext-ima vide !!");    
    return TCL_OK;
  }

  /* *************************************************/
  /* premiere passe pour determiner a la fin combien */
  /*   d'extrema contiendra l'ext image de sortie    */
  /* *************************************************/
  for (i = 0; i < sig->n; i++) {
    /* on initialise extonline (pointeur d'extremun)
       avec l'extremun correspondant sur l'ext image 
       de depart */
    thepos = (int)(sig->dataX[i]);
    
    /* il faut trouver dans le tableau extImage_in->extr, le bon extremun,
       c'est dire celui dont la position est thepos !!!*/
    found = 0;
    for (j = 0; j < extImage_in->extrNb; j++) {
      extonline = extImage_in->extr[j];
      if (extonline.pos == thepos) {
	found = 1;
	break;
      }
    }
    if (found) {
      extr_selection[i] = extonline;
      theline = (Line *)(extonline.line);
    } else {
      sprintf(interp->result,"error : specified extremun not found in the extimage\n");    
      return TCL_ERROR;
    }
    
    /* si theline n'est pas dans la liste, on l'y met ! */
    if (!lst_content_value (theline_lst, theline)) {
      lst_add (theline, theline_lst);
      length += theline->size;
    }
    /* if option Reverse is on, one removes the line from the list*/
    if (isReverse) {
      lst_remove (theline, theline_lst_reverse);
    }
  }

  length_reverse = extImage_in->extrNb - length;

  /* *******************************************/
  /* a present on peut allouer la memoire pour */
  /*   l'ext image de sortie                   */
  /* *******************************************/

  if (length!=0) {
    extImage_out = w2_ext_new (length, lx, ly, scale);
  }
  if (!extImage_out)
    return GenErrorMemoryAlloc(interp);


  if (isReverse && length_reverse!=0) {
    extImage_out_reverse = w2_ext_new (length_reverse, lx, ly, scale);
    if (!extImage_out_reverse) {
      return GenErrorMemoryAlloc(interp);
    }
  }

  /* ****************************/
  /* deuxieme passe : ENFIN !!! */
  /* ****************************/
  j=0;
  foreach (line, theline_lst) {
    /*thepos = (int)sig->dataX[i];
    extonline = extr_selection[i];
    theline = (Line *)(extonline.line);*/

    /* on parcourt les extrema de la ligne sur laquelle
       se trouve extonline 
       et on remplit l'ext image de sortie*/
    foreach (ext, line->ext_lst) {
      extImage_out->extr[j].pos = ext->pos;
      extImage_out->extr[j].mod = ext->mod;
      extImage_out->extr[j].arg = ext->arg;
      j++;
    }
  }

  if (isReverse) {
    j=0;
    foreach (line, theline_lst_reverse) {
      /* on parcourt les extrema de la ligne sur laquelle
	 se trouve extonline 
	 et on remplit l'ext image de sortie*/
      foreach (ext, line->ext_lst) {
	extImage_out_reverse->extr[j].pos = ext->pos;
	extImage_out_reverse->extr[j].mod = ext->mod;
	extImage_out_reverse->extr[j].arg = ext->arg;
	j++;
      }
    }
  }
  
  if (isReverse) {
    ExtDicStore(name_reverse, extImage_out_reverse);
  }
    
  ExtDicStore(name, extImage_out);
  free(extr_selection);
  sprintf(interp->result,"%d %d",j,length);    
  return TCL_OK;
}


/************************************************
  command name in xsmurf : eisavelines2
*************************************************/
/* created  the 29th nov 2001*/
int
ei_save_lines_2_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "ESs",
    NULL
  };

  char * help_msg =
  {
    (" Save some pieces of chains of an ext image. Pieces of chains are\n"
     "refered to by the\n" 
     "position of an extremun belonging to it.\n"
     "\n"
     "Parameters :\n"
     "  Extima  - The ext image to treat.\n"
     "  Signal  - The signal.\n"
     "  string  - Name of the ext image.\n"
     "Be careful that sig->dataX array must contain\n"
     " positions of the extrema : pos = x + lx * y  . \n"
     "\n"
     "Options:\n"
     "\n")
  };

  /* Command's parameters */
  
  ExtImage *extImage_in;
  ExtImage *extImage_out;
  Extremum *extonline;
  Line     *theline;
  List     *theline_lst;
  Signal   *sig;
  char     *name;
  int      lx, ly;
  real     scale;
  int      thepos;
  
  Extremum *mobileExtremum;
  int      found, length;
 
  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int      i,j,k;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extImage_in, &sig, &name) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */
  if (sig->type != REALXY) {
    sprintf (interp->result, "bad signal type, must be REALXY");
    return TCL_ERROR;
  }

  mobileExtremum = (Extremum *) malloc(1     *sizeof(Extremum));

  /* Treatement */
  lx    = extImage_in->lx;
  ly    = extImage_in->ly;
  scale = extImage_in->scale;

  /* on va creer une liste de lines, deux a deux differentes
     pour eviter qu'il y ait plusieurs extrema pointant sur
     la meme line */
  theline_lst         = lst_create ();
  
  length = 0;

  if (sig->n <=0) {
    sprintf(interp->result,"ext-ima vide !!");    
    return TCL_OK;
  }

  /* *************************************************/
  /* premiere passe pour determiner a la fin combien */
  /* d'extrema contiendra l'ext image de sortie      */
  /* *************************************************/
  for (i = 0; i < sig->n; i++) {
    /* on initialise extonline (pointeur d'extremun)
       avec l'extremun correspondant sur l'ext image 
       de depart */
    thepos = (int)(sig->dataX[i]);
    
    /* il faut trouver dans le tableau extImage_in->extr, le bon extremun,
       c'est dire celui dont la position est thepos !!!*/
    found = 0;
    extonline = extImage_in->extr;
    j=0;
    while (extonline && (j < extImage_in->extrNb)) {
      if (extonline->pos == thepos) {
	found = 1;
	break;
      } else if (j == (extImage_in->extrNb -1)) {
	found = 0;
	break;
      } else {
	j++;
	extonline++;
      }
    }
    if (found) {
      theline = (Line *)(extonline->line);
    } else {
      sprintf(interp->result,"error : specified extremun not found in the extimage\n");    
      return TCL_ERROR;
    }
    
    /* si theline n'est pas dans la liste, on l'y met ! */
    /* puis on positionne  theline->ext_lst->current    */
    /* on parcourt le voisinage du point courant        */
    if (1) {
      int compt;
      lst_add (theline, theline_lst);
      length += 1;

      compt = 0;
      lst_set_current (extonline, theline->ext_lst);     
      while((mobileExtremum = lst_next(theline->ext_lst))) {
	compt++;
	if (compt==3) break;
      }
      length += compt;
      
      compt = 0;
      lst_set_current (extonline, theline->ext_lst);     
      while((mobileExtremum = lst_prev(theline->ext_lst))) {
	compt++;
	if (compt==3) break;
      }
      length += compt;
    }
    
  }
  
  
  /* *******************************************/
  /* a present on peut allouer la memoire pour */
  /* l'ext image de sortie                     */
  /* *******************************************/

  if (length!=0) {
    extImage_out = w2_ext_new (length, lx, ly, scale);
  }
  if (!extImage_out) {
    return GenErrorMemoryAlloc(interp);
  }


  /* ****************************/
  /* deuxieme passe : ENFIN !!! */
  /* ****************************/
  k=0;

  for (i = 0; i < sig->n; i++) {
    /* on initialise extonline (pointeur d'extremun)
       avec l'extremun correspondant sur l'ext image 
       de depart */
    thepos = (int)(sig->dataX[i]);
    
    /* il faut trouver dans le tableau extImage_in->extr, le bon extremun,
       c'est dire celui dont la position est thepos !!!*/
    found = 0;
    extonline = extImage_in->extr;
    j=0;
    while (extonline && (j < extImage_in->extrNb)) {
      if (extonline->pos == thepos) {
	found = 1;
	break;
      } else if (j == (extImage_in->extrNb -1)) {
	found = 0;
      } else {
	j++;
	extonline++;
      }
    }
    if (found) {
      theline = (Line *)(extonline->line);
    }
     
    /* si theline n'est pas dans la liste, on l'y met ! */
    /* puis on positionne  theline->ext_lst->current    */
    /* on parcourt le voisinage du point courant        */
    if (1) {
      int compt;
      
      extImage_out->extr[k].pos = extonline->pos;
      extImage_out->extr[k].mod = extonline->mod;
      extImage_out->extr[k].arg = extonline->arg;
      k++;

      compt = 0;
      lst_set_current (extonline, theline->ext_lst);     
      while((mobileExtremum = lst_next(theline->ext_lst))) {
	compt++;
	extImage_out->extr[k].pos = mobileExtremum->pos;
	extImage_out->extr[k].mod = mobileExtremum->mod;
	extImage_out->extr[k].arg = mobileExtremum->arg;
	k++;
	if (compt==3) break;
      }
      
      compt = 0;
      lst_set_current (extonline, theline->ext_lst);     
      while((mobileExtremum = lst_prev(theline->ext_lst))) {
	compt++;
	extImage_out->extr[k].pos = mobileExtremum->pos;
	extImage_out->extr[k].mod = mobileExtremum->mod;
	extImage_out->extr[k].arg = mobileExtremum->arg;
	k++;
	if (compt==3) break;
      }
    }
  }
  
  /*
  foreach (line, theline_lst) {
    foreach (ext, line->ext_lst) {
      extImage_out->extr[j].pos = ext->pos;
      extImage_out->extr[j].mod = ext->mod;
      extImage_out->extr[j].arg = ext->arg;
      j++;
    }
  }
  */
  
  ExtDicStore(name, extImage_out);
  sprintf(interp->result,"%d %d",j,length);    
  return TCL_OK;
}



/************************************************
  command name in xsmurf : eisave_skel4vtk
  ********************************************* */
/* created the 22nd october 2001
modified in feb 2008 (option tag) */
int
ei_save_skel_vtk_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es",
    "-between", "dd",
    "-color", "",
    "-slope", "",
    "-localslope", "",
    "-tag", "",
    NULL
  };

  char * help_msg =
  {
    (" Save some vertical lines of a skeleton in a file\n" 
     "using VTK file format.\n"
     "See : http://public.kitware.com/FileFormats.pdf\n"
     "\n"
     "Parameters :\n"
     "  Extima  - The ext image to treat.\n"
     "  string  - Name of output file.\n"
     "            example: \"toto.vtk\"\n"
     "Be careful that the given extima must be\n"
     "chained (result of a \"chain\" operation). \n"
     "\n"
     "Options:\n"
     "  -between [integer integer]: see eivcgerbe command\n"
     "  -color   : add scalar field to put some color\n"
     "             default scalar is WT module\n"
     "  -slope   : colorize lines according to mean slope\n"
     "  -localslope : same with local slope\n"
     "  -tag : use only tagged vertical lines\n"
     "\n")
  };
  
  /* Command's parameters */
  ExtImage *extima;
  int total_length, total_length_between, total_length_between_tag;
  int length, thelength, thesum;
  int pos, x, y;
  int nb_vc, nb_vc_between, nb_vc_between_tag;
  int nb_points, nb_points_between, nb_points_between_tag;
  int count, count_between;
  
  /* Options's presence */

  int isBetween;
  int isColor;
  int isSlope;
  int isLocalSlope;
  int isTag;

  /* Options's parameters */
  int scale_up = 0;
  int scale_low = 0;
  char color_fieldname[10] = "mod"; 
  real     *slope_array,  *slope_array_between;
  real     *lslope_array, *lslope_array_between;
  int      i;
  real     slope,t, st2;

  /* Other variables */
  Line     *l;
  Extremum *ext;
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
  isColor = arg_present(2);
  /*if(isColor) {
    if (arg_get(2, &color_fieldname) == TCL_ERROR) {
      return TCL_ERROR;
    }
    }*/
  isSlope = arg_present(3);
  if(isSlope) {
    strcpy(color_fieldname, "meanh");
  }

  isLocalSlope = arg_present(4);
  isTag = arg_present(5);

  /* fill header */
  fileOut = fopen(filename, "w");
  fprintf(fileOut, "# vtk DataFile Version 1.0 \n");
  fprintf(fileOut, "data generated by xsmurf.\n");
  fprintf(fileOut, "ASCII\n");
  fprintf(fileOut, "\n");
  fprintf(fileOut, "DATASET POLYDATA\n");
  

  total_length         = 0;
  total_length_between = 0;
  total_length_between_tag = 0;

  nb_vc     = 0;
  nb_points = 0;
  nb_vc_between     = 0;
  nb_points_between = 0;
  nb_vc_between_tag     = 0;
  nb_points_between_tag = 0;



  /* first compute nb_vc and nb_vc_between */
  foreach (l, extima->line_lst) {
    foreach (ext, l->gr_lst) {
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
	if (isTag && ext->next) {
	  nb_vc_between_tag++;
	  total_length_between_tag += length;
	}
	nb_vc_between++;
	total_length_between += length;
      }

    } /* end of foreach(ext, l->gr_lst ) loop  */
  }   /* end of foreach(l, ei->line_lst) loop  */
  
  /* now we fill POINTS field */
  if (isBetween && isTag) {
    fprintf(fileOut, "POINTS %d integer\n", total_length_between_tag);
  } else if (isBetween && !isTag) {
    fprintf(fileOut, "POINTS %d integer\n", total_length_between);    
  } else {
    fprintf(fileOut, "POINTS %d integer\n", total_length);
  }

  /* some stuff for Option Color meanH
     when color_fieldname = "meanh" */
  /* we use nb_vc to allocate memory for array that will
     store meanh values of each lines of skeleton */
  if (isBetween && isTag) {
      slope_array_between  = (float *) malloc(nb_vc_between_tag*sizeof(float));
      lslope_array_between = (float *) malloc(total_length_between_tag*sizeof(float));
  } else if (isBetween && !isTag) {
    slope_array_between  = (float *) malloc(nb_vc_between*sizeof(float));
    lslope_array_between = (float *) malloc(total_length_between*sizeof(float));
  } else {
    slope_array          = (float *) malloc(nb_vc*sizeof(float));
    lslope_array         = (float *) malloc(total_length*sizeof(float));
  }
 
  i=0;
  foreach (l, extima->line_lst) {
    foreach (ext, l->gr_lst) {
      if (!ext->up && !ext->down) {
	continue;
      }
      /* Follow the chain down to up */
      thelength = 0;
      thesum    = 0;
      while (ext->up) {
	ext = ext->up;
	thelength++;
	thesum += thelength;
      }

      while (ext->down) {
	ext = ext->down;
      }

      length = 0;
      t = length - thesum/(thelength+1);
      st2 = t * t;
      slope  = t * log(ext->mod)/log(2.0);
      while (ext->up) {
	ext = ext->up;
	length++;
	t = length - thesum/(thelength+1);
	st2 += t * t;
	slope += t * log(ext->mod)/log(2.0);
      }
      slope = slope/st2*10;

      if ((isBetween && length >= scale_low && length < scale_up) && (isTag && ext->next)) {
	slope_array_between[i]=slope;
	i++;
      } else if ((isBetween && length >= scale_low && length < scale_up) && !isTag) {
	slope_array_between[i]=slope;
	i++;	
      } else if (isBetween==0) {
	slope_array[i]=slope;
	i++;
      }

    } /* end of foreach(ext, l->gr_lst ) loop  */
  }   /* end of foreach(l, ei->line_lst) loop  */


  /***************************
   * we fill arrays
   * lslope_array
   *and lslope_array_between
   ***************************/
  i = 0;
  foreach (l, extima->line_lst) {
    foreach (ext, l->gr_lst) {
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

      if ((isBetween && thelength >= scale_low && thelength < scale_up) && (isTag && ext->next)) {
	while (ext->up) {
	  lslope_array_between[i]=log(ext->up->mod / ext->mod)/log(2);
	  ext = ext->up;
	  i++;
	}
	lslope_array_between[i] = lslope_array_between[i-1];
      } else if ((isBetween && thelength >= scale_low && thelength < scale_up) && !isTag) {
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

    } /* end of foreach(ext, l->gr_lst ) loop  */
  }   /* end of foreach(l, ei->line_lst) loop  */

  /***********************************/
  /* now we enter POINTS coordinates */
  /***********************************/
  if (isBetween && isTag) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
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
	
	if (length >= scale_low && length < scale_up && ext->next) {
	  while (ext->down) {
	    ext = ext->down;
	  }
	  length = 1;
	  pos = ext->pos;
	  x = pos % extima->lx;
	  y = pos / extima->lx;
	  fprintf(fileOut, "%d %d %d\n", x, (int) (extima->lx*(length-1)/50), y);
	  while (ext->up) {
	    ext = ext->up;
	    length++;
	    pos = ext->pos;
	    x = pos % extima->lx;
	    y = pos / extima->lx;
	    fprintf(fileOut, "%d %d %d\n", x, (int) (extima->lx*(length-1)/50), y);
	  }
	}
      }
    }
  } else if (isBetween && !isTag) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
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
	  y = pos / extima->lx;
	  fprintf(fileOut, "%d %d %d\n", x, (int) (extima->lx*(length-1)/50), y);
	  while (ext->up) {
	    ext = ext->up;
	    length++;
	    pos = ext->pos;
	    x = pos % extima->lx;
	    y = pos / extima->lx;
	    fprintf(fileOut, "%d %d %d\n", x, (int) (extima->lx*(length-1)/50), y);
	  }
	}
      }
    }    
  } else {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* Follow the chain down to up */
	length = 1;
	pos = ext->pos;
	x = pos % extima->lx;
	y = pos / extima->lx;
	fprintf(fileOut, "%d %d %d\n", x, (int) (extima->lx*(length-1)/50), y);
		
	while (ext->up) {
	  ext = ext->up;
	  length++;
	  pos = ext->pos;
	  x = pos % extima->lx;
	  y = pos / extima->lx;
	  fprintf(fileOut, "%d %d %d\n", x, (int) (extima->lx*(length-1)/50), y);
	}
      } /* end of foreach(ext, l->gr_lst ) loop  */
    }   /* end of foreach(l, ei->line_lst) loop  */
  }
  
  
  /***********************
   * now we fill LINES field
   ***********************/
  if (isBetween && isTag) {
    fprintf(fileOut, "LINES %d %d\n",nb_vc_between_tag, nb_vc_between_tag+total_length_between_tag);
  } else if (isBetween && !isTag) {
    fprintf(fileOut, "LINES %d %d\n",nb_vc_between, nb_vc_between+total_length_between);
  } else {
    fprintf(fileOut, "LINES %d %d\n",nb_vc, nb_vc+total_length);
  }

  count         = 0;
  count_between = 0;

  if (isBetween && isTag) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
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

	if (length >= scale_low && length < scale_up && ext->next) {
	  fprintf(fileOut, "%d ",length);
	  fprintf(fileOut, "%d ",count_between++);
	  while (ext->up) {
	    ext = ext->up;
	    fprintf(fileOut, "%d ",count_between++);
	  }
	  fprintf(fileOut,"\n");
	}
      }
    }
  } else if (isBetween && !isTag) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
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
    }
  } else {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
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
      } /* end of foreach(ext, l->gr_lst ) loop  */
    }   /* end of foreach(l, ei->line_lst) loop  */
  }

/*
 *
 * put some colors
 *
 */
i=0;
if (isColor) {
  if (isBetween) {
    if (isTag)
      fprintf(fileOut,"POINT_DATA %d\n", total_length_between_tag);
    else
      fprintf(fileOut,"POINT_DATA %d\n", total_length_between);

    fprintf(fileOut,"SCALARS my_scalars float\n");
      fprintf(fileOut,"LOOKUP_TABLE default\n");
      foreach (l, extima->line_lst) {
	foreach (ext, l->gr_lst) {
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
	  
	  if (length >= scale_low && length < scale_up && (!isTag || (isTag && ext->next))) {
	    while (ext->down) {
	      ext = ext->down;
	    }
	    if (strcmp(color_fieldname,"meanh")==0) {
	      fprintf(fileOut, "%f\n", slope_array_between[i]);
	      while (ext->up) {
		ext = ext->up;
		fprintf(fileOut, "%f\n", slope_array_between[i]);
	      }
	      i++;
	    } else {
	      fprintf(fileOut, "%f\n", ext->mod);
	      while (ext->up) {
		ext = ext->up;
		fprintf(fileOut, "%f\n", ext->mod);
	      }
	    }
	  }
	}
      }
  } else {
      fprintf(fileOut,"POINT_DATA %d\n", total_length);
      fprintf(fileOut,"SCALARS my_scalars float\n");
      fprintf(fileOut,"LOOKUP_TABLE default\n");
      foreach (l, extima->line_lst) {
	foreach (ext, l->gr_lst) {
	  if (!ext->up && !ext->down) {
	    continue;
	  }
	  /* Follow the chain down to up */
	  if (strcmp(color_fieldname,"meanh")==0) {
	      fprintf(fileOut, "%f\n", slope_array[i]);
	      while (ext->up) {
		ext = ext->up;
		fprintf(fileOut, "%f\n", slope_array[i]);
	      }
	      i++;
	  } else {
	    fprintf(fileOut, "%f\n", ext->mod);  
	    while (ext->up) {
	      ext = ext->up;
	      fprintf(fileOut, "%f\n", ext->mod);
	    }
	  }
	} /* end of foreach(ext, l->gr_lst ) loop  */
      }   /* end of foreach(l, ei->line_lst) loop  */
    }    
  } else if (isSlope) {
    if (isBetween) {
      fprintf(fileOut,"POINT_DATA %d\n", total_length_between);
      fprintf(fileOut,"SCALARS my_scalars float\n");
      fprintf(fileOut,"LOOKUP_TABLE default\n");
      foreach (l, extima->line_lst) {
	foreach (ext, l->gr_lst) {
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
	    if (strcmp(color_fieldname,"meanh")==0) {
	      fprintf(fileOut, "%f\n", slope_array_between[i]);
	      while (ext->up) {
		ext = ext->up;
		fprintf(fileOut, "%f\n", slope_array_between[i]);
	      }
	      i++;
	    } else {
	      /*fprintf(fileOut, "%f\n", ext->mod);
	      while (ext->up) {
		ext = ext->up;
		fprintf(fileOut, "%f\n", ext->mod);
		}*/
	      fprintf(fileOut, "%f\n", slope_array_between[i]);
	      while (ext->up) {
		ext = ext->up;
		fprintf(fileOut, "%f\n", slope_array_between[i]);
	      }
	      i++;
	    }
	  }
	}
      }
    } else {
      fprintf(fileOut,"POINT_DATA %d\n", total_length);
      fprintf(fileOut,"SCALARS my_scalars float\n");
      fprintf(fileOut,"LOOKUP_TABLE default\n");
      foreach (l, extima->line_lst) {
	foreach (ext, l->gr_lst) {
	  if (!ext->up && !ext->down) {
	    continue;
	  }
	  /* Follow the chain down to up */
	    fprintf(fileOut, "%f\n",  slope_array[i]);  
	    while (ext->up) {
	      ext = ext->up;
	      fprintf(fileOut, "%f\n",  slope_array[i]);
	    }
	    i++;

	} /* end of foreach(ext, l->gr_lst ) loop  */
      }   /* end of foreach(l, ei->line_lst) loop  */
    }
  } else if (isLocalSlope) {
    if (isBetween) {
      fprintf(fileOut,"POINT_DATA %d\n", total_length_between);
      fprintf(fileOut,"SCALARS my_scalars float\n");
      fprintf(fileOut,"LOOKUP_TABLE default\n");
      foreach (l, extima->line_lst) {
	foreach (ext, l->gr_lst) {
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
	    
	    if (100*lslope_array_between[i]< -5.0) {
	      fprintf(fileOut, "%f\n", -5.0);
	      i++;
	    } else if (100*lslope_array_between[i]> 5.0) {
	      fprintf(fileOut, "%f\n", 5.0);
	      i++;
	    } else {
	      fprintf(fileOut, "%f\n", 100*lslope_array_between[i++]);
	    }	      
	    while (ext->up) {
	      ext = ext->up;
	      if (100*lslope_array_between[i]< -5.0) {
		fprintf(fileOut, "%f\n", -5.0);
		i++;
	      } else if (100*lslope_array_between[i]> 5.0) {
		fprintf(fileOut, "%f\n", 5.0);
		i++;
	      } else {
		fprintf(fileOut, "%f\n", 100*lslope_array_between[i++]);
	      }
	      /*fprintf(fileOut, "%f\n", mymax(100*lslope_array_between[i++],-10));*/
	    }
	    
	  }
	}
      }
    } else {
      fprintf(fileOut,"POINT_DATA %d\n", total_length);
      fprintf(fileOut,"SCALARS my_scalars float\n");
      fprintf(fileOut,"LOOKUP_TABLE default\n");
      foreach (l, extima->line_lst) {
	foreach (ext, l->gr_lst) {
	  if (!ext->up && !ext->down) {
	    continue;
	  }
	  /* Follow the chain down to up */

	  fprintf(fileOut, "%f\n", lslope_array[i++]);  
	  while (ext->up) {
	    ext = ext->up;
	    fprintf(fileOut, "%f\n", lslope_array[i++]);
	  }
	  
	} /* end of foreach(ext, l->gr_lst ) loop  */
      }   /* end of foreach(l, ei->line_lst) loop  */
    }
  }
  fclose(fileOut);
  
  return TCL_OK;
}



/************************************************
  command name in xsmurf : eisave_skel4jvx
  ********************************************* */
/* created  on march 4th 2002*/
int
ei_save_skel_jvx_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{
  /* Command line definition */
  char * options[] =
  {
    "Es",
    "-between", "dd",
    "-color", "",
    NULL
  };

  char * help_msg =
  {
    (" Save some vertical lines of a skeleton in a file\n"
     "using JVX file format (kind of XML).\n"
     "See : http://\n"
     "\n"
     "Parameters :\n"
     "  Extima  - The ext image to treat.\n"
     "  string  - Name of output file.\n"
     "            example: \"toto.jvx\"\n"
     "Be careful that the given extima must be\n"
     "chained (result of a \"chain\" operation). \n"
     "\n"
     "Options:\n"
     "  -between [integer integer]: see eivcgerbe command\n"
     "  -color   : add scalar field to put some color\n"
     "             default scalar is WT module\n"
     "\n")
  };

  /* Command's parameters */
  ExtImage *extima;
  int total_length, total_length_between;
  int length;
  int pos, x, y;
  int lx, ly;
  int nb_vc, nb_vc_between;
  int nb_points, nb_points_between;
  int count, count_between;
  int scale_up = 0;
  int scale_low = 0;

  /* see in manual 3 : strftime */
  float maxmod=0.0;
  float mmod;
  int index_red, index_green, index_blue;
  int pos_scale;
  float colorth;

  /* Options's presence */

  int isBetween;
  int isColor;

  /* Options's parameters */

  /* Other variables */
  Line     *l;
  Extremum *ext;
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
  isColor = arg_present(2);


  lx = extima->lx;
  ly = extima->ly;
  colorth = 46.0;


  /* fill header */
  /*strftime (mystring, 20, "%D", mytm);*/
  /*mystring = ctime (mytm);*/

  fileOut = fopen(filename, "w");
  fprintf(fileOut, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"yes\"?>\n");
  fprintf(fileOut, "<!DOCTYPE jvx-model SYSTEM \"http://www.javaview.de/rsrc/jvx.dtd\">");
  fprintf(fileOut, "<jvx-model>\n");
  fprintf(fileOut, "   <meta generator=\"Xsmurf\"/>\n");
  fprintf(fileOut, "   <meta date=\"Thu Jan 18 00:10:21 GMT+01:00 2001\"/>\n");
  fprintf(fileOut, "   <version type=\"final\">2.00</version>\n");
  fprintf(fileOut, "   <title>WTMM Skeleton</title>\n");
  fprintf(fileOut, "   <description>\n");
  fprintf(fileOut, "      <abstract>WTMM Skeleton.</abstract>\n");
  fprintf(fileOut, "   </description>\n");  
  fprintf(fileOut, "   <geometries>\n");
  fprintf(fileOut, "      <geometry name=\"skeleton's points\">\n");
  fprintf(fileOut, "         <pointSet dim=\"3\" point=\"hide\">\n");
  fprintf(fileOut, "            <points>\n");

  total_length         = 0;
  total_length_between = 0;

  nb_vc     = 0;
  nb_points = 0;
  nb_vc_between     = 0;
  nb_points_between = 0;

  /* first */
  foreach (l, extima->line_lst) {
    foreach (ext, l->gr_lst) {
      if (!ext->up && !ext->down) {
	continue;
      }
      nb_vc++;

      if (maxmod<ext->mod) maxmod = ext->mod;

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

    } /* end of foreach(ext, l->gr_lst ) loop  */
  }   /* end of foreach(l, ei->line_lst) loop  */
  
  /* now we fill POINTS field */
  /*if (isBetween) {
    fprintf(fileOut, "POINTS %d integer\n", total_length_between);
  } else {
    fprintf(fileOut, "POINTS %d integer\n", total_length);
    }*/


  /* now we enter POINTS coordinates */
  if (isBetween) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
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
	  y = pos / extima->lx;
	  fprintf(fileOut, "               <p>%d %d %d</p>\n", x, (int) (extima->lx*(length-1)/50.0), y);
	  while (ext->up) {
	    ext = ext->up;
	    length++;
	    pos = ext->pos;
	    x = pos % extima->lx;
	    y = pos / extima->lx;
	    fprintf(fileOut, "               <p>%d %d %d</p>\n", x, (int) (extima->lx*(length-1)/50.0), y);
	  }
	}
      }
    }
  } else {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* Follow the chain down to up */
	length = 1;
	pos = ext->pos;
	x = pos % extima->lx;
	y = pos / extima->lx;
	fprintf(fileOut, "               <p>%d %d %d</p>\n", x, (int) (extima->lx*(length-1)/50.0), y);
		
	while (ext->up) {
	  ext = ext->up;
	  length++;
	  pos = ext->pos;
	  x = pos % extima->lx;
	  y = pos / extima->lx;
	  fprintf(fileOut, "               <p>%d %d %d</p>\n", x, (int) (extima->lx*(length-1)/50.0), y);
	}
      }
    }
  }

  fprintf(fileOut, "            </points>\n");
  fprintf(fileOut, "         </pointSet>\n");

  /* now we fill LINES field */
  fprintf(fileOut, "         <lineSet line=\"show\" color=\"show\">\n");
  fprintf(fileOut, "            <lines>\n");
  
  count         = 0;
  count_between = 0;

  if (isBetween) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}

	/* Follow the chain down to up */
	length = 1;
	pos_scale = 1;
	while (ext->up) {
	  ext = ext->up;
	  length++;
	}
	while (ext->down) {
	  ext = ext->down;
	}

	if (length >= scale_low && length < scale_up) {
	  fprintf(fileOut, "               <l>%d %d</l>\n", count_between, count_between+1);
	  count_between++;
	  while (pos_scale<length-1) {
	    pos_scale++;
	    fprintf(fileOut, "               <l>%d %d</l>\n", count_between, count_between+1);
	     count_between++;
	  }
	  count_between++;
	}
      }
    }
  } else {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}

	/* Follow the chain down to up */
	length = 1;
	pos_scale = 1;
	while (ext->up) {
	  ext = ext->up;
	  length++;
	}
	while (ext->down) {
	  ext = ext->down;
	}

	fprintf(fileOut, "               <l>%d %d</l>\n", count, count+1);
	count++;
	while (pos_scale<length-1) {
	  pos_scale++;
	  fprintf(fileOut, "               <l>%d %d</l>\n", count, count+1);
	  count++;
	}
	count++;
      }
    }
  }

  fprintf(fileOut, "            </lines>\n");

  /* now colors !!! */
  fprintf(fileOut, "            <colors>\n");

  if (isBetween) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}

	/* Follow the chain down to up */
	length = 1;
	pos_scale = 1;
	while (ext->up) {
	  ext = ext->up;
	  length++;
	}
	while (ext->down) {
	  ext = ext->down;
	}

	if (length >= scale_low && length < scale_up) {
	  mmod = 100*ext->mod/maxmod - 50;
	  index_red   = (int) (255*(atan(0.5*(mmod-colorth))/3.141592+0.5));
	  index_green = (int) (255*(atan(0.5*(mmod-0 ))/3.141592+0.5));
	  index_blue  = (int) (255*(atan(0.5*(mmod+colorth))/3.141592+0.5));
	  fprintf(fileOut, "               <c>%d %d %d</c>\n", index_red, index_green, index_blue);
	  while (pos_scale<length) {
	    pos_scale++;
	    ext = ext->up;
	    mmod = 100*ext->mod/maxmod - 50;
	    index_red   = (int) (255*(atan(0.5*(mmod-colorth))/3.141592+0.5));
	    index_green = (int) (255*(atan(0.5*(mmod-0 ))/3.141592+0.5));
	    index_blue  = (int) (255*(atan(0.5*(mmod+colorth))/3.141592+0.5));
	    fprintf(fileOut, "               <c>%d %d %d</c>\n", index_red, index_green, index_blue );
	  }
	}
      }
    }
  } else {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}

	/* Follow the chain down to up */
	length = 1;
	pos_scale = 1;
	while (ext->up) {
	  ext = ext->up;
	  length++;
	}
	while (ext->down) {
	  ext = ext->down;
	}
	
	mmod = 100*ext->mod/maxmod - 50;
	index_red   = (int) (255*(atan(0.5*(mmod-colorth))/3.141592+0.5));
	index_green = (int) (255*(atan(0.5*(mmod-0 ))/3.141592+0.5));
	index_blue  = (int) (255*(atan(0.5*(mmod+colorth))/3.141592+0.5));
	fprintf(fileOut, "               <c>%d %d %d</c>\n", index_red, index_green, index_blue);
	
	while (pos_scale<length) {
	  pos_scale++;
	  ext=ext->up;
	  mmod = 100*ext->mod/maxmod - 50;
	  index_red   = (int) (255*(atan(0.5*(mmod-colorth))/3.141592+0.5));
	  index_green = (int) (255*(atan(0.5*(mmod-0 ))/3.141592+0.5));
	  index_blue  = (int) (255*(atan(0.5*(mmod+colorth))/3.141592+0.5));
	  fprintf(fileOut, "               <c>%d %d %d</c>\n", index_red, index_green, index_blue);
	}

      }
    }
  }

  fprintf(fileOut, "            </colors>\n");
  fprintf(fileOut, "         </lineSet>\n");
  fprintf(fileOut, "      </geometry>\n");
  fprintf(fileOut, "   </geometries>\n");
  fprintf(fileOut, "</jvx-model>\n");


  fclose(fileOut);

  return TCL_OK;
}


/*******************************************************
  command name in xsmurf : eisave_skel4m3d (JView3d)
  ****************************************************** */
/* created  on march 5th 2002*/
int
ei_save_skel_m3d_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{
  /* Command line definition */
  char * options[] =
  {
    "Es",
    "-between", "dd",
    "-texture", "s",
    NULL
  };

  char * help_msg =
  {
    (" Save some vertical lines of a skeleton in a file\n"
     "using m3d file format (similar to OBJ of Wavefront).\n"
     "See : http://\n"
     "\n"
     "Parameters :\n"
     "  Extima  - The ext image to treat.\n"
     "  string  - Name of output file.\n"
     "            example: \"toto.m3d\"\n"
     "Be careful that the given extima must be\n"
     "chained (result of a \"chain\" operation). \n"
     "\n"
     "Options:\n"
     "  -between [integer integer]: see eivcgerbe command\n"
     "  -texture [string] : add a texture on the floor.\n"
     "           argument should be the path of a GIF or JPG image.\n"
     "\n")
  };

  /* Command's parameters */
  ExtImage *extima;
  int total_length, total_length_between;
  int length;
  int pos, x, y;
  int lx, ly;
  int nb_vc, nb_vc_between;
  int nb_points, nb_points_between;
  int count, count_between;

  float maxmod=0.0;
  float minmod=100000.0;
  float mmod;
  int index_red, index_green, index_blue;
  int pos_scale;
  float colorth;

  /* Options's presence */

  int isBetween;
  int isTexture;

  /* Options's parameters */
  int scale_up = 0;
  int scale_low = 0;
  char *texturename = NULL;
  
  /* Other variables */
  Line     *l;
  Extremum *ext;
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
  isTexture = arg_present(2);
  if(isTexture) {
    if (arg_get(2, &texturename) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  lx = extima->lx;
  ly = extima->ly;
  colorth = 25.0;
  
  
  /* fill header */
  fileOut = fopen(filename, "w");
  fprintf(fileOut, "NEW:\n");
  fprintf(fileOut, "NAME:  \"WTMM Skeleton\"\n");
  
  
  total_length         = 0;
  total_length_between = 0;
  
  nb_vc     = 0;
  nb_points = 0;
  nb_vc_between     = 0;
  nb_points_between = 0;

  /* first */
  foreach (l, extima->line_lst) {
    foreach (ext, l->gr_lst) {
      if (!ext->up && !ext->down) {
	continue;
      }
      nb_vc++;
      
      if (maxmod<ext->mod) maxmod = ext->mod;
      if (minmod>ext->mod) minmod = ext->mod;

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
    }
  }
  
  /* now we enter POINTS coordinates */
  fprintf(fileOut, "POINTS:\n");
  
  if (isBetween) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
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
	  y = pos / extima->lx;
	  fprintf(fileOut, "%d,%d,%d,\n", x-lx/2, (int) (extima->lx*(length-1)/50.0), y-ly/2);
	  while (ext->up) {
	    ext = ext->up;
	    length++;
	    pos = ext->pos;
	    x = pos % extima->lx;
	    y = pos / extima->lx;
	    fprintf(fileOut, "%d,%d,%d,\n", x-lx/2, (int) (extima->lx*(length-1)/50.0), y-ly/2);
	  }
	}
      }
    }
  } else {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* Follow the chain down to up */
	length = 1;
	pos = ext->pos;
	x = pos % extima->lx;
	y = pos / extima->lx;
	fprintf(fileOut, "%d,%d,%d,\n", x-lx/2, (int) (extima->lx*(length-1)/50.0), y-ly/2);

	while (ext->up) {
	  ext = ext->up;
	  length++;
	  pos = ext->pos;
	  x = pos % extima->lx;
	  y = pos / extima->lx;
	  fprintf(fileOut, "%d,%d,%d,\n", x-lx/2, (int) (extima->lx*(length-1)/50.0), y-ly/2);
	}
      }
    }
  }
  
  /* if isTexture we add points on the ground floor for the frame */
  if (isTexture) {
    fprintf(fileOut, "%d,%d,%d,\n", lx/2, 0, ly/2);
    fprintf(fileOut, "%d,%d,%d,\n", lx/2, 0, -ly/2 );
    fprintf(fileOut, "%d,%d,%d,\n", -lx/2 , 0, -ly/2 );
    fprintf(fileOut, "%d,%d,%d,\n", -lx/2 , 0, ly/2);
  }
  


  /* now we fill LINES field */
  fprintf(fileOut, "LINES:\n");

  count         = 0;
  count_between = 0;

  if (isBetween) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}

	/* Follow the chain down to up */
	length = 1;
	pos_scale = 1;
	while (ext->up) {
	  ext = ext->up;
	  length++;
	}
	while (ext->down) {
	  ext = ext->down;
	}

	if (length >= scale_low && length < scale_up) {
	  fprintf(fileOut, "%d,%d,  ", count_between, count_between+1);
	  count_between++;

	  mmod = 100*(log(ext->mod)-log(minmod))/(log(maxmod)-log(minmod)) - 50;
	  //mmod = 100*ext->mod/maxmod - 50;
	  //index_red   = (int) (255*(atan(0.2*(mmod-colorth))/3.141592+0.5));
	  //index_green = (int) (255*(atan(0.2*(mmod-0 ))/3.141592+0.5));
	  //index_blue  = (int) (255*(atan(0.2*(mmod+colorth))/3.141592+0.5));
	  index_red   = (int) (255*exp(-(mmod-colorth)*(mmod-colorth)/2/35/35));
	  index_green = (int) (255*exp(-(mmod-0.0    )*(mmod-0.0    )/2/25/25));
	  index_blue  = (int) (255*exp(-(mmod+colorth)*(mmod+colorth)/2/25/25));
	  fprintf(fileOut, "RGB(%d,%d,%d)\n", index_red, index_green, index_blue);

	  while (pos_scale<length-1) {
	    pos_scale++;
	    fprintf(fileOut, "%d,%d,  ", count_between, count_between+1);
	    count_between++;

	    ext = ext->up;
	    mmod = 100*(log(ext->mod)-log(minmod))/(log(maxmod)-log(minmod)) - 50;
	    //mmod = 100*ext->mod/maxmod - 50;
	    //index_red   = (int) (255*(atan(0.2*(mmod-colorth))/3.141592+0.5));
	    //index_green = (int) (255*(atan(0.2*(mmod-0 ))/3.141592+0.5));
	    //index_blue  = (int) (255*(atan(0.2*(mmod+colorth))/3.141592+0.5));
	    index_red   = (int) (255*exp(-(mmod-colorth)*(mmod-colorth)/2/35/35));
	    index_green = (int) (255*exp(-(mmod-0.0    )*(mmod-0.0    )/2/25/25));
	    index_blue  = (int) (255*exp(-(mmod+colorth)*(mmod+colorth)/2/25/25));
	    fprintf(fileOut, "RGB(%d,%d,%d)\n", index_red, index_green, index_blue );
	  }
	  count_between++;
	}
      }
    }
  } else {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* Follow the chain down to up */
	length = 1;
	pos_scale = 1;
	while (ext->up) {
	  ext = ext->up;
	  length++;
	}
	while (ext->down) {
	  ext = ext->down;
	}

	fprintf(fileOut, "%d,%d,  ", count, count+1);
	count++;
	
	mmod = 100*(log(ext->mod)-log(minmod))/(log(maxmod)-log(minmod)) - 50;
	//mmod = 100*ext->mod/maxmod - 50;
	//index_red   = (int) (255*(atan(1.7*(mmod-colorth))/3.141592+0.5));
	//index_green = (int) (255*(atan(1.7*(mmod-0 ))/3.141592+0.5));
	//index_blue  = (int) (255*(atan(1.7*(mmod+colorth))/3.141592+0.5));
	index_red   = (int) (255*exp(-(mmod-colorth)*(mmod-colorth)/2/35/35));
	index_green = (int) (255*exp(-(mmod-0.0    )*(mmod-0.0    )/2/25/25));
	index_blue  = (int) (255*exp(-(mmod+colorth)*(mmod+colorth)/2/25/25));
	fprintf(fileOut, "RGB(%d,%d,%d)\n", index_red, index_green, index_blue);
	
	while (pos_scale<length-1) {
	  pos_scale++;
	  fprintf(fileOut, "%d,%d,  ", count, count+1);
	  count++;

	  ext = ext->up;
	  mmod = 100*(log(ext->mod)-log(minmod))/(log(maxmod)-log(minmod)) - 50;
	  //mmod = 100*ext->mod/maxmod - 50;
	  //index_red   = (int) (255*(atan(1.7*(mmod-colorth))/3.141592+0.5));
	  //index_green = (int) (255*(atan(1.7*(mmod-0 ))/3.141592+0.5));
	  //index_blue  = (int) (255*(atan(1.7*(mmod+colorth))/3.141592+0.5));
	  index_red   = (int) (255*exp(-(mmod-colorth)*(mmod-colorth)/2/35/35));
	  index_green = (int) (255*exp(-(mmod-0.0    )*(mmod-0.0    )/2/25/25));
	  index_blue  = (int) (255*exp(-(mmod+colorth)*(mmod+colorth)/2/25/25));
	  fprintf(fileOut, "RGB(%d,%d,%d)\n", index_red, index_green, index_blue );
	}
	count++;
      }
    }
  }

  if (isTexture) {
    fprintf(fileOut, "%d,%d,  RGB(255,255,255)\n", count, count+1);
    fprintf(fileOut, "%d,%d,  RGB(255,255,255)\n", count+1, count+2);
    fprintf(fileOut, "%d,%d,  RGB(255,255,255)\n", count+2, count+3);
    fprintf(fileOut, "%d,%d,  RGB(255,255,255)\n", count+3, count);
    fprintf(fileOut, "\n");
    fprintf(fileOut, "FACES:\n");
    fprintf(fileOut, " %d,%d,%d,%d,  0,MAPPED(\"%s\")\n", count+2, count+3, count, count+1, texturename);
  }

  fclose(fileOut);

  return TCL_OK;
}


/************************************************
 * command name in xsmurf : eisave_skelchain4vtk
 ************************************************/
/* created the 24nd october 2001 (P. Kestener) */
/* modified in March 2008: option tag (P. Kestener) */
int
ei_save_skel_chain_vtk_TclCmd_ (ClientData clientData,
				Tcl_Interp *interp,
				int        argc,
				char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es[f]",
    "-between", "dd",
    "-color",   "",
    "-tag","",
    NULL
  };

  char * help_msg =
  {
    (" Save some horizontal chains of a skeleton in a file\n" 
     "using VTK file format.\n"
     "See : http://public.kitware.com/FileFormats.pdf\n"
     "\n"
     "Parameters :\n"
     "  Extima  - The ext image to treat.\n"
     "  string  - Name of output file.\n"
     "            example: \"toto.vtk\"\n"
     "  [real] - float value used to scale spacing in the z-direction\n"
     "Be careful that the given extima must be\n"
     "chained (result of a \"chain\" operation). \n"
     "\n"
     "Options:\n"
     "  -between [integer integer]: see eivcgerbe command\n"
     "  -color   [] : add scalar field to put some color\n"
     "             default scalar is WT module\n"
     "  -tag []: save chain which contain at least one tagged WTMMM\n"
     "\n")
  };
  
  /* Command's parameters */
  ExtImage *extima;
  real spacing=1.0;
  int total_length, total_length_between,total_length_between_tag;
  int pos, x, y;
  int scale;
  int nb_chains, nb_chains_between,nb_chains_between_tag;
  int nb_points, nb_points_between,nb_points_between_tag;
  int count, count_between, count_between_tag;
  int scale_up = 0;
  int scale_low = 0;

  /* Options's presence */

  int isBetween;
  int isColor;
  int isTag;

  /* Options's parameters */

  /* Other variables */
  Line     *l;
  Extremum *ext;
  char     *filename = NULL;
  FILE     *fileOut;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extima, &filename, &spacing) == TCL_ERROR)
    return TCL_ERROR;

  isBetween = arg_present(1);
  if(isBetween) {
    if (arg_get(1, &scale_low, &scale_up) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isColor = arg_present(2);
  isTag = arg_present(3);

  /* fill header */
  fileOut = fopen(filename, "w");
  fprintf(fileOut, "# vtk DataFile Version 1.0 \n");
  fprintf(fileOut, "data generated by xsmurf.\n");
  fprintf(fileOut, "ASCII\n");
  fprintf(fileOut, "\n");
  fprintf(fileOut, "DATASET POLYDATA\n");
  

  total_length         = 0;
  total_length_between = 0;
  total_length_between_tag = 0;

  nb_chains = 0;
  nb_points = 0;
  nb_chains_between = 0;
  nb_points_between = 0;
  nb_chains_between_tag = 0;
  nb_points_between_tag = 0;

  /* first */
  scale = 0;
  if (isTag) {
    if (scale >= scale_low && scale < scale_up) {
      foreach (l, extima->line_lst) {
	if (l->tag ==1) {
	  nb_chains_between_tag++;
	  total_length_between_tag +=l->size;
	}
      } /* end foreach */
    }
    while (extima->up) {
      extima = extima->up;
      scale++;
      if (scale >= scale_low && scale < scale_up) {
	foreach (l, extima->line_lst) {
	  if (l->tag ==1) {
	    nb_chains_between_tag++;
	    total_length_between_tag +=l->size;
	  }
	} /* end foreach */
      }
    }
  } else { /* if (isTag) */
    nb_chains    += extima->nb_of_lines;
    total_length += extima->extrNb;
    if (scale >= scale_low && scale < scale_up) {
      nb_chains_between    += extima->nb_of_lines;
      total_length_between += extima->extrNb;
    }
    while (extima->up) {
      extima = extima->up;
      scale++;
      nb_chains    += extima->nb_of_lines;
      total_length += extima->extrNb;
      if (scale >= scale_low && scale < scale_up) {
	nb_chains_between    += extima->nb_of_lines;
	total_length_between += extima->extrNb;
      }
    }
  } /* end if(isTag) */

  /* go down */
  while (extima->down) {
    extima = extima->down;
  }


  /* now we fill POINTS field */
  if (isTag && isBetween) {
    fprintf(fileOut, "POINTS %d integer\n", total_length_between_tag);
  } else if (isBetween) {
    fprintf(fileOut, "POINTS %d integer\n", total_length_between);
  } else {
    fprintf(fileOut, "POINTS %d integer\n", total_length);
  }

  scale = 0;

  /* now we enter POINTS coordinates */
 if (isBetween) {
   
   if (scale >= scale_low && scale < scale_up) {
      foreach (l, extima->line_lst) {
	if ((isTag && l->tag ==1) || !isTag) {
	  foreach (ext, l->ext_lst) {
	    pos = ext->pos;
	    x = pos % extima->lx;
	    y = pos / extima->lx;
	    fprintf(fileOut, "%d %d %d\n", x, (int) (extima->lx*(scale)/50*spacing), y);
	  }
	}
      }
   }
   while (extima->up) {
     extima = extima->up;
     scale++;
     if (scale >= scale_low && scale < scale_up) {
       foreach (l, extima->line_lst) {
	 if ((isTag && l->tag ==1) || !isTag) {
	   foreach (ext, l->ext_lst) {
	     pos = ext->pos;
	     x = pos % extima->lx;
	     y = pos / extima->lx;
	     fprintf(fileOut, "%d %d %d\n", x, (int) (extima->lx*(scale)/50*spacing), y);
	   }
	 }
       }
     }
   }
   
 } else {
   
   foreach (l, extima->line_lst) {
     foreach (ext, l->ext_lst) {
       pos = ext->pos;
       x = pos % extima->lx;
       y = pos / extima->lx;
       fprintf(fileOut, "%d %d %d\n", x, (int) (extima->lx*(scale)/50*spacing), y);
     }
   }	
   while (extima->up) {
     extima = extima->up;
     scale++;
     foreach (l, extima->line_lst) {
       foreach (ext, l->ext_lst) {
	 pos = ext->pos;
	 x = pos % extima->lx;
	 y = pos / extima->lx;
	 fprintf(fileOut, "%d %d %d\n", x, (int) (extima->lx*(scale)/50*spacing), y);
       }
     }
   }
 }
 
 /* now we fill LINES field */
 if (isTag && isBetween) {
   fprintf(fileOut, "LINES %d %d\n",nb_chains_between_tag, nb_chains_between_tag+total_length_between_tag);
 } else if (isBetween) {
   fprintf(fileOut, "LINES %d %d\n",nb_chains_between, nb_chains_between+total_length_between);
 } else {
   fprintf(fileOut, "LINES %d %d\n",nb_chains, nb_chains+total_length);
 }
 
 
 /* go down */
 while (extima->down) {
   extima = extima->down;
 }
 
 
 count         = 0;
 count_between = 0;
 
 scale = 0;
 
 if (isBetween) {
   if (scale >= scale_low && scale < scale_up) {
     foreach (l, extima->line_lst) {
       if ((isTag && l->tag ==1) || !isTag) {
	 fprintf(fileOut, "%d ",l->size);
	 foreach (ext, l->ext_lst) {
	   fprintf(fileOut, "%d ", count++);
	 }
	 fprintf(fileOut,"\n");
       }
     }
   }
   while (extima->up) {
     extima = extima->up;
     scale++;
     if (scale >= scale_low && scale < scale_up) {
       foreach (l, extima->line_lst) {
	 if ((isTag && l->tag ==1) || !isTag) {	
	   fprintf(fileOut, "%d ",l->size);
	   foreach (ext, l->ext_lst) {
	     fprintf(fileOut, "%d ", count++);
	   }
	   fprintf(fileOut,"\n");
	 }
       }
     }
   }
 } else {
   foreach (l, extima->line_lst) {
     fprintf(fileOut, "%d ",l->size);
     foreach (ext, l->ext_lst) {
       fprintf(fileOut, "%d ", count++);
     }
     fprintf(fileOut,"\n");
   }
   while (extima->up) {
     extima = extima->up;
     scale++;
     foreach (l, extima->line_lst) {
       fprintf(fileOut, "%d ",l->size);
       foreach (ext, l->ext_lst) {
	 fprintf(fileOut, "%d ", count++);
       }
       fprintf(fileOut,"\n");
     }
   }
 }
 
 /* go down */
 while (extima->down) {
   extima = extima->down;
 }
 scale = 0;
 
 if (isColor) {
   if (isBetween) {
     if (isTag)
       fprintf(fileOut,"POINT_DATA %d\n", total_length_between_tag);
     else
      fprintf(fileOut,"POINT_DATA %d\n", total_length_between);

     fprintf(fileOut,"SCALARS my_scalars float\n");
     fprintf(fileOut,"LOOKUP_TABLE default\n");
     if (scale >= scale_low && scale < scale_up) {
       foreach (l, extima->line_lst) {
	 if ((isTag && l->tag ==1) || !isTag) {
	   foreach (ext, l->ext_lst) {
	     fprintf(fileOut, "%f ", ext->mod);
	   }
	   fprintf(fileOut,"\n");
	 }
       }
     }
     while (extima->up) {
       extima = extima->up;
       scale++;
       if (scale >= scale_low && scale < scale_up) {
	 foreach (l, extima->line_lst) {
	   if ((isTag && l->tag ==1) || !isTag) {	   
	     foreach (ext, l->ext_lst) {
	       fprintf(fileOut, "%f ", ext->mod);
	     }
	     fprintf(fileOut,"\n");
	   }
	 }
       }
     }
   } else { /* if (isBetween) */
     fprintf(fileOut,"POINT_DATA %d\n", total_length);
     fprintf(fileOut,"SCALARS my_scalars float\n");
     fprintf(fileOut,"LOOKUP_TABLE default\n");
     foreach (l, extima->line_lst) {
       foreach (ext, l->ext_lst) {
	 fprintf(fileOut, "%f ", ext->mod);
       }
       fprintf(fileOut,"\n");
     }
     while (extima->up) {
       extima = extima->up;
       foreach (l, extima->line_lst) {
	 foreach (ext, l->ext_lst) {
	   fprintf(fileOut, "%f ", ext->mod);
	 }
	 fprintf(fileOut,"\n");
       }
     }
   }    
 }
 
 
 fclose(fileOut);
 
 return TCL_OK;
}



/************************************************
 * command name in xsmurf : e2vtk
 ************************************************/
/* created july the 8th 2003 by pierre kestener */
int
ei_save_extimage_4_vtk_TclCmd_ (ClientData clientData,
				Tcl_Interp *interp,
				int        argc,
				char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es",
    "-function",   "s",
    "-gradient",   "s",
    "-normalize", "",
    NULL
  };

  char * help_msg =
  {
    (" Save an extimage in a file using VTK file format.\n"
     "Data type is POLYDATA.\n"
     "See : http://public.kitware.com/FileFormats.pdf\n"
     "Note that the extimage must have chained (see \"hsearch\" command.\n"
     "\n"
     "Parameters :\n"
     "  Extima  - The ext image to treat.\n"
     "  string  - Name of output file.\n"
     "            example: \"toto.vtk\"\n"
     "Be careful that the given extima must be\n"
     "chained (result of a \"chain\" operation). \n"
     "\n"
     "Options:\n"
     "   -function [s] : fuction to rescale data (modulus)\n"
     "   -gradient [s] : write gradient arrows (after chain command).\n"
     "                   using STRUCTURED_POINTS Datatype in file\n"
     "                   \"filename\" given as parameter.\n"
     "   -normalize    : use this option to tell -gradient option\n"
     "                   to normalize gradient arrows.\n"
     "\n")
  };
  
  /* Command's parameters */
  ExtImage *extima;
  int total_length;
  int pos, x, y;
  int scale;
  int nb_chains;
  int nb_points;
  int count;
  int scale_up = 0;
  int scale_low = 0;

  /* Options's presence */
  int isFunction;
  int isGradient;
  int isNormalize;

  /* Options's parameters */

  /* Other variables */
  Line     *l;
  Extremum *ext;
  char     *filename = NULL;
  char     *filename2 = NULL;
  FILE     *fileOut;
  char     *fct_expr;
  /*double   (*fct)();*/
  void     *fct;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extima, &filename) == TCL_ERROR)
    return TCL_ERROR;

  isFunction = arg_present(1);
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
  isGradient = arg_present(2);
  if (isGradient) {
    if (arg_get(2, &filename2) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isNormalize = arg_present(3);

  /* fill header */
  fileOut = fopen(filename, "w");
  fprintf(fileOut, "# vtk DataFile Version 2.0 \n");
  fprintf(fileOut, "data generated by xsmurf.\n");
  fprintf(fileOut, "ASCII\n");
  fprintf(fileOut, "\n");
  fprintf(fileOut, "DATASET POLYDATA\n");
  
  total_length  = 0;

  /* first */
  nb_chains    = extima->nb_of_lines;
  total_length = extima->extrNb;
    
  /* now we fill POINTS field */
  fprintf(fileOut, "POINTS %d integer\n", total_length);
  
  /* now we enter POINTS coordinates */
  foreach (l, extima->line_lst) {
    foreach (ext, l->ext_lst) {
      pos = ext->pos;
      x = pos % extima->lx;
      y = pos / extima->lx;
      fprintf(fileOut, "%d %d %d\n", y, x, 0);
    }
  }	


  /* now we fill LINES field */
  fprintf(fileOut, "LINES %d %d\n",nb_chains, nb_chains+total_length);

  count = 0;

  foreach (l, extima->line_lst) {
    fprintf(fileOut, "%d ",l->size);
    foreach (ext, l->ext_lst) {
      fprintf(fileOut, "%d ", count++);
    }
    fprintf(fileOut,"\n");
  }
    
  
  fprintf(fileOut,"POINT_DATA %d\n", total_length);
  fprintf(fileOut,"SCALARS my_scalars float\n");
  fprintf(fileOut,"LOOKUP_TABLE default\n");
  if (isFunction) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->ext_lst) {
	/*fprintf(fileOut, "%f ", (float) fct(ext->mod,0.0));*/
	fprintf(fileOut, "%f ", (float) evaluator_evaluate_x_y(fct, ext->mod,0.0));
      }
      fprintf(fileOut,"\n");
    }   
  } else {
    foreach (l, extima->line_lst) {
      foreach (ext, l->ext_lst) {
	fprintf(fileOut, "%f ", ext->mod);
      }
      fprintf(fileOut,"\n");
    }
  }
  fclose(fileOut);

  if (isGradient) {
    int reduce_param=1, nb_points;
    int i,j,lx,ly;
    float *data1, *data2, tmp;

    lx = extima->lx;
    ly = extima->ly;

    /* init data */
    data1 = (float *) malloc (lx*ly*sizeof (float));
    data2 = (float *) malloc (lx*ly*sizeof (float));
    for (i=0;i<lx;i++)
      for (j=0;j<ly;j++) {
	data1[i+j*lx] = 0.0;
	data2[i+j*lx] = 0.0;
      }	
    
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	pos = ext->pos;
	//i = pos % lx;
	//j = pos / lx;
	//data1[i+j*lx]=ext->mod*cos(ext->arg);
	//data2[i+j*lx]=ext->mod*sin(ext->arg);
	if (isNormalize) {
	  data1[pos]=sin(ext->arg);
	  data2[pos]=cos(ext->arg);
	} else {
	  data1[pos]=ext->mod*sin(ext->arg);
	  data2[pos]=ext->mod*cos(ext->arg);
	}
      }
    }	
    /* end init data */

    nb_points = 0;
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	if ((i%reduce_param)==0 && (j%reduce_param)==0)
	  nb_points++;
      }
    }
    /* fill header */
    fileOut = fopen(filename2, "w");
    fprintf(fileOut, "# vtk DataFile Version 2.0 \n");
    fprintf(fileOut, "data generated by xsmurf.\n");
    fprintf(fileOut, "ASCII\n");
    fprintf(fileOut, "\n");
    fprintf(fileOut,"DATASET STRUCTURED_POINTS\n");
    fprintf(fileOut,"DIMENSIONS %d %d %d\n ", lx/reduce_param, ly/reduce_param, 1);
    fprintf(fileOut,"ORIGIN %f %f %f\n", 0.0, 0.0, 0.0);
    fprintf(fileOut,"SPACING %f %f %f\n", (float) reduce_param, (float) reduce_param, 1.0);
    fprintf(fileOut," \n");
    
    // first scalar field : gradient modulus
    fprintf(fileOut,"POINT_DATA %d\n", nb_points);
    fprintf(fileOut,"SCALARS scalars float\n");
    fprintf(fileOut,"LOOKUP_TABLE default\n");
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	if ((i%reduce_param)==0 && (j%reduce_param)==0) { 
	  tmp = sqrt(data1[i+j*lx]*data1[i+j*lx]+data2[i+j*lx]*data2[i+j*lx]);
	  fprintf(fileOut, "%f ", tmp);
	}
      }
    }
    // next vector field !!!
    fprintf(fileOut,"\nVECTORS vectors float\n"); 
    for (i=0;i<lx;i++) {
      for (j=0;j<ly;j++) {
	if ((i%reduce_param)==0 && (j%reduce_param)==0) 
	  fprintf(fileOut, "%f %f %f\n", data1[i+j*lx], data2[i+j*lx], 0.0);
      }
    }
    
    free(data1);
    free(data2);
    fclose(fileOut);
  }
  
  if (isFunction) {
    evaluator_destroy(fct);
  }

  return TCL_OK;
}

/*************************************
  command name in xsmurf : egetextr
  ************************************/
int
egetextr_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "E",
    NULL
  };

  char * help_msg =
  {
    (" Get the modulus extrema of an ext image.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage - ext image to treat.")
  };

  /* Command's parameters */
  ExtImage *extImage;

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
  ExtImaMinMax(extImage, &min, &max);

  sprintf(interp->result,"%g %g", min, max);

  return TCL_OK;
}


/*************************************
 * command name in xsmurf : egetmin
 *************************************/
int
egetmin_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{ 
    /* Command line definition */
    char * options[] =
	{
	    "E",
	    NULL
	};

    char * help_msg =
	{
	    (" Get the modulus minimum of an ext image.\n"
	     "\n"
	     "Parameters :\n"
	     "  ExtImage - ext image to treat.")
	};

    /* Command's parameters */
    ExtImage *extImage;

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
    ExtImaMinMax(extImage, &min, &max);

    sprintf(interp->result,"%g", min);

    return TCL_OK;
}

/*************************************
 * command name in xsmurf : egetmax
 *************************************/
int
egetmax_TclCmd_ (ClientData clientData,
		 Tcl_Interp *interp,
		 int        argc,
		 char       **argv)
{ 
    /* Command line definition */
    char * options[] =
	{
	    "E",
	    NULL
	};

    char * help_msg =
	{
	    (" Get the modulus maximum of an ext image.\n"
	     "\n"
	     "Parameters :\n"
	     "  ExtImage - ext image to treat.")
	};

    /* Command's parameters */
    ExtImage *extImage;

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
    ExtImaMinMax(extImage, &min, &max);

    sprintf(interp->result,"%g", max);

    return TCL_OK;
}

/*
 */
int
e_mult_by_i_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "EI",
    NULL
  };

  char * help_msg =
  {
    (" Multiply an ext image using an image.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage - ext image to treat.\n"
     "  Image    - image to use.")
  };

  /* Command's parameters */
  ExtImage *extImage;
  Image    *im;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  int i;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extImage, &im) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  if (extImage->lx != im->lx || extImage->ly != im->ly) {
    sprintf (interp->result, "images must have the same lx and ly");
    return TCL_ERROR;
  }

  /* Treatement */

  for (i = 0; i < extImage->extrNb; i++) {
    extImage->extr[i].mod *= im->data[extImage->extr[i].pos];
  }

  return TCL_OK;
}


/*
 Command name in xsmurf : eidrawcv
 an old function ?!?
 What's the difference with eaff ??
 A regarder un autre jour !
*/
int
e_draw_in_canvas_TclCmd_ (ClientData clientData,
			  Tcl_Interp *interp,
			  int        argc,
			  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Esddddddf",
    "-grey", "",
    "-color", "s",
    "-tags", "s",
    NULL
  };

  char * help_msg =
  {
    (" Draw \"some contents\" of an ext image in a canvas.\n"
     "\n"
     "Parameters :\n"
     "  ExtImage - ext image to treat.\n"
     "  string   - path of the canvas.\n"
     "  integer  - width of the area in the canvas where the ext image must\n"
     "             be drawn.\n"
     "  integer  - height of the area in the canvas where the ext image must\n"
     "             be drawn.\n"
     "  integer  - x position of the area in the canvas.\n"
     "  integer  - y position of the area in the canvas.\n"
     "  integer  - horizontal offset in the ext image.\n"
     "  integer  - vertical offset in the ext image.\n"
     "  integer  - zoom.\n"
     "\n"
     "Options :\n"
     "  -grey : Draw a square for each max with a grey level depending on its\n"
     "          modulus value.\n"
     "  -color : Draw a square of a given color for each max.\n"
     "    string - name of the color.\n"
     "  -tags : tags of the objects in the canvas.\n"
     "    list - list of the tags.")
  };

  /* Command's parameters */
  ExtImage *extImage;
  char     *cvName;
  int      wiWidth;
  int      wiHeight;
  int      eiHoffset;
  int      eiVoffset;
  int      xOrigPos;
  int      yOrigPos;
  real     zoom;

  /* Options's presence */
  int isGrey;
  int isColor;
  int isTags;

  /* Options's parameters */
  char *colorName;
  char *tagsList;

  /* Other variables */
  int i;
  int x;
  int y;
  real x1;
  real y1;
  real x2;
  real y2;
  int  rectSize;
  real min, max;
  int greyLevel;
  int result;
  char scriptStr[200];
  char theColorName[200];

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extImage, &cvName, &wiWidth, &wiHeight,
	       &xOrigPos, &yOrigPos, &eiHoffset, &eiVoffset,
	       &zoom) == TCL_ERROR)
    return TCL_ERROR;

  isGrey = arg_present(1);
  isColor = arg_present(2);
  if (isColor) {
    if (arg_get(2, &colorName) == TCL_ERROR) {
      return TCL_ERROR;
    }
    sprintf(theColorName, "%s", colorName);
  }
  isTags = arg_present(3);
  if (isTags) {
    if (arg_get(3, &tagsList) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */

  /* Treatement */
  rectSize = zoom;
  ExtImaMinMax(extImage, &min, &max);

  for (i = 0; i < extImage->extrNb; i++) {
    x = extImage->extr[i].pos%extImage->lx;
    y = extImage->extr[i].pos/extImage->lx;

    x1 = xOrigPos + (x-eiHoffset)*zoom-1;
    y1 = yOrigPos + (y-eiVoffset)*zoom-1;
    x2 = x1+rectSize;
    y2 = y1+rectSize;
    if (isGrey) {
      greyLevel = ceil((extImage->extr[i].mod-min)*20/(max-min))*4+5;
      sprintf(theColorName, "grey%d", greyLevel);
    }

    if (x2 <= (xOrigPos+wiWidth) && y2 <= (yOrigPos+wiHeight) && x1 >= (xOrigPos-1) && y1 >= (yOrigPos-1)) {
      if (isTags) {
	sprintf(scriptStr, "%s create rectangle %f %f %f %f -fill %s -outline {} -tags {%s}", cvName, x1, y1, x2, y2, theColorName, tagsList);
      } else {
	sprintf(scriptStr, "%s create rectangle %f %f %f %f -fill %s -outline {}", cvName, x1, y1, x2, y2, theColorName);
      }
      result = Tcl_Eval(interp, scriptStr);
      if (result == TCL_ERROR) {
	return TCL_ERROR;
      }
    }
  }
  return result;
}


/*************************************
  Command name in xsmurf : eilinetag
  ************************************/
int
special_tag_lines_TclCmd_ (ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Eddd",
    NULL
  };

  char * help_msg =
  {
    ("  Tag all lines that contain one of the vertical chains that are on a "
     "given line at a given scale.\n"
     "\n"
     "Parameters :\n"
     "  ext image  - Ext image of the scale where the base line is.\n"
     "  2 integers - Coordinate of a point. The base line is the nearest to "
     "               this point.\n"
     "  integer    - Tag to apply on the lines.\n"
     "\n"
     "Options :\n"
     "\n"
     "Return value :\n"
     "  Number of lines that change of tag.")
  };

  /* Command's parameters */
  ExtImage *ext_image;
  int      x, y;
  int      tag;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  Line     *line;
  Line     *line2;
  int      nbOfTaggedLines = 0;
  int      pos;
  Extremum *ext_ptr;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ext_image, &x, &y, &tag) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */
  pos = x + y*ext_image->lx;
  ext_ptr = w2_closest_vc_ext (ext_image, pos);
  if (!ext_ptr) {
    sprintf (interp->result, "No extremum near (%d,%d)", x, y);
    return TCL_ERROR;
  }

  sprintf(interp->result, "%d %d",
	  ext_ptr->pos%ext_image->lx,
	  ext_ptr->pos/ext_image->lx);

  /* Beurk ! */
  line = (Line *) ext_ptr->next;

  foreach (ext_ptr, line->gr_lst) {
    while (ext_ptr->up) {
      ext_ptr = ext_ptr->up;
    }
    while (ext_ptr->down){
      line2 = (Line*) ext_ptr->next;
      if (line2->tag != tag) {
	line2->tag = tag;
	nbOfTaggedLines++;
      }
      ext_ptr = ext_ptr->down;
    }
    if (ext_ptr->up) {
      if (line2->tag != tag) {
	line2->tag = tag;
	nbOfTaggedLines++;
      }
    }
  }

  sprintf (interp->result, "%d", nbOfTaggedLines);

  return TCL_OK;
}


/***********************************
 * Command name in xsmurf : eicorr
 ***********************************/
int
e_correlation_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "EEs",
    "-mean1", "f",
    "-mean2", "f",
    "-max", "",
    "-gradx", "",
    "-grady", "",
    NULL
  };

  char * help_msg =
  {
    ("  Compute the correlation between 2 ext images. The points involved are "
     "the maxima along the contour lines (WT3M).\n"
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
  ExtImage *ei1;
  ExtImage *ei2;
  char     *sName;

  /* Options's presence */
  int isMean1;
  int isMean2;
  int isMax;
  int isGradx;
  int isGrady;

  /* Options's parameters */
  real mean1 = 0;
  real mean2 = 0;

  /* Other variables */
  Signal   *sig;
  Line     *l1, *l2;
  int      x, y, x1, y1;
  Extremum *ext1;
  Extremum *ext2;
  int      lx, ly, n, i;
  int      *nb;
  real     dist;

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
  isGradx = arg_present(4);
  isGrady = arg_present(5);

  /* Parameters validity and initialisation */

  /* Treatement */

  lx = ei1->lx;
  ly = ei1->ly;
  n = sqrt(lx*lx+ly*ly);
  sig = sig_new (REALY, 0, n - 1);
  sig->x0 = 0.0;
  sig->dx = 1;

  nb = (int *) malloc(sizeof(int)*n);
  for (i = 0; i < n; i++) {
    nb[i] = 0;
    sig->dataY[i] = 0.0;
  }

  if (isMax) {
    foreach (l1, ei1->line_lst) {
      foreach (ext1, l1->ext_lst) {
	
	x1 = ext1->pos%lx;
	y1 = ext1->pos/lx;
	
	foreach (l2, ei2->line_lst) {
	  foreach (ext2, l2->ext_lst) {
	    
	    x = x1 - ext2->pos%lx;
	    y = y1 - ext2->pos/lx;
	    
	    dist = sqrt(x*x + y*y);
	    i = (int) (dist+0.5);
	    if (isGradx) {
	      sig->dataY[i] += (log(fabs(ext1->mod*cos(ext1->arg)))-mean1)\
		*(log(fabs(ext2->mod*cos(ext2->arg)))-mean2);
	    } else if (isGrady) {
	      sig->dataY[i] += (log(fabs(ext1->mod*sin(ext1->arg)))-mean1)\
		*(log(fabs(ext2->mod*sin(ext2->arg)))-mean2);
	    } else {
	      sig->dataY[i] += (log(ext1->mod)-mean1)*(log(ext2->mod)-mean2);
	    }
	    nb[i]++;
	    
	  } /* Loop end for gr on l2 */
	} /* Loop end for l2 on e2 */
      } /* Loop end for gr on l1 */
    } /* Loop end for l2 on e2 */
  } else {
    foreach (l1, ei1->line_lst) {
      foreach (ext1, l1->gr_lst) {
	if (!ext1->up && !ext1->down) {
	  continue;
	}
	
	x1 = ext1->pos%lx;
	y1 = ext1->pos/lx;
	
	foreach (l2, ei2->line_lst) {
	  foreach (ext2, l2->gr_lst) {
	    if (!ext2->up && !ext2->down) {
	      continue;
	    }
	    
	    x = x1 - ext2->pos%lx;
	    y = y1 - ext2->pos/lx;
	    
	    dist = sqrt(x*x + y*y);
	    i = (int) (dist+0.5);
	    sig->dataY[i] += (log(ext1->mod)-mean1)*(log(ext2->mod)-mean2);
	    nb[i]++;
	    
	  } /* Loop end for gr on l2 */
	} /* Loop end for l2 on e2 */
      } /* Loop end for gr on l1 */
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


/*************************************
 * Command name in xsmurf : eicorr2
 *************************************/
int
e_correlation2_TclCmd_ (ClientData clientData,
		       Tcl_Interp *interp,
		       int        argc,
		       char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "EEs",
    "-mean1", "f",
    "-mean2", "f",
    "-max", "",
    "-gradx", "",
    "-grady", "",
    NULL
  };

  char * help_msg =
  {
    ("  Compute the correlation between 2 ext images. The points involved are "
     "the maxima along the contour lines (WT3M).\n"
     "TAKE CARE : eicorr2 does the same thing that eicorr but it takes\n"
     "into account periodic effects, that's to say the largest distance\n"
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
  ExtImage *ei1;
  ExtImage *ei2;
  char     *sName;

  /* Options's presence */
  int isMean1;
  int isMean2;
  int isMax;
  int isGradx;
  int isGrady;

  /* Options's parameters */
  real mean1 = 0;
  real mean2 = 0;

  /* Other variables */
  Signal   *sig;
  Line     *l1, *l2;
  int      x, y, x1, y1, x2, y2, x22, y22;
  Extremum *ext1;
  Extremum *ext2;
  int      lx, ly, n, i;
  int      *nb;
  real     dist, distcurrent;

  int      pos1;
  //int      pos2;
  /* these variables are between 1 and 4
     and indicate in what quarter the corresponding 
     extremum is.*/

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
  isGradx = arg_present(4);
  isGrady = arg_present(5);

  /* Parameters validity and initialisation */

  /* Treatement */

  lx = ei1->lx;
  ly = ei1->ly;
  n = sqrt(lx*lx+ly*ly);
  sig = sig_new (REALY, 0, n - 1);
  sig->x0 = 0.0;
  sig->dx = 1;

  nb = (int *) malloc(sizeof(int)*n);
  for (i = 0; i < n; i++) {
    nb[i] = 0;
    sig->dataY[i] = 0.0;
  }

  if (isMax) {
    foreach (l1, ei1->line_lst) {
      foreach (ext1, l1->ext_lst) {
	
	x1 = ext1->pos%lx;
	y1 = ext1->pos/lx;
	
	foreach (l2, ei2->line_lst) {
	  foreach (ext2, l2->ext_lst) {
	    
	    x = x1 - ext2->pos%lx;
	    y = y1 - ext2->pos/lx;
	    
	    dist = sqrt(x*x + y*y);
	    i = (int) (dist+0.5);
	    if (isGradx) {
	      sig->dataY[i] += (log(fabs(ext1->mod*cos(ext1->arg)))-mean1)\
		*(log(fabs(ext2->mod*cos(ext2->arg)))-mean2);
	    } else if (isGrady) {
	      sig->dataY[i] += (log(fabs(ext1->mod*sin(ext1->arg)))-mean1)\
		*(log(fabs(ext2->mod*sin(ext2->arg)))-mean2);
	    } else {
	      sig->dataY[i] += (log(ext1->mod)-mean1)*(log(ext2->mod)-mean2);
	    }
	    nb[i]++;
	    
	  } /* Loop end for gr on l2 */
	} /* Loop end for l2 on e2 */
      } /* Loop end for gr on l1 */
    } /* Loop end for l2 on e2 */
  } else {
    foreach (l1, ei1->line_lst) {
      foreach (ext1, l1->gr_lst) {
	if (!ext1->up && !ext1->down) {
	  continue;
	}
	
	x1 = ext1->pos%lx;
	y1 = ext1->pos/lx;
	/* determine in which quarter is ext1 */
	if (x1<lx/2) {
	  if (y1<ly/2) {
	    pos1 = 1;
	  } else {
	    pos1 = 3;
	  }
	} else {
	  if (y1<ly/2) {
	    pos1 = 2;
	  } else {
	    pos1 = 4;
	  }
	}
	
	foreach (l2, ei2->line_lst) {
	  foreach (ext2, l2->gr_lst) {
	    if (!ext2->up && !ext2->down) {
	      continue;
	    }
	    x2 = ext2->pos%lx;
	    y2 = ext2->pos/lx;
	    /* determine in which quarter is ext1 */
	    if (x2<lx/2) {
	      if (y2<ly/2) {
		pos1 = 1;
	      } else {
		pos1 = 3;
	      }
	    } else {
	      if (y2<ly/2) {
		pos1 = 2;
	      } else {
		pos1 = 4;
	      }
	    }
	
	    x = x1 - x2;
	    y = y1 - y2;
	    dist = sqrt(x*x + y*y);

	    /*if (x > lx/2 || y > ly/2) {
	      if (pos1 == 1 && pos2 == 4) {
		x2 -= lx;
		y2 -= ly;
	      } else if (pos1 == 4 && pos2 == 1) {
		x1 -= lx;
		y1 -= ly;
	      } else if (pos1 == 2 && pos2 == 3) {
		x2 += lx;
		y2 -= ly;
	      } else if (pos1 == 3 && pos2 == 2) {
		x1 += lx;
		y1 -= ly;
	      } else if (pos1 == 1 && pos2 == 2) {
		x2 -= lx;
	      } else if (pos1 == 2 && pos2 == 1) {
		x1 -= lx;
	      } else if (pos1 == 3 && pos2 == 4) {
		x2 -= lx;
	      } else if (pos1 == 4 && pos2 == 3) {
		x1 -= lx;
	      } else if (pos1 == 1 && pos2 == 3) {
		y2 -= ly;
	      } else if (pos1 == 3 && pos2 == 1) {
		y1 -= ly;
	      } else if (pos1 == 2 && pos2 == 4) {
		y2 -= ly;
	      } else if (pos1 == 4 && pos2 == 2) {
		y1 -= ly;
	      }
	      }*/

	    /* move position of ext2 in the eight periodic images of current
	       data, and find the distance minimum */
	    if (dist > lx/2) {
	      int incr_x[] = {1,  1,  0, -1, -1, -1, 0, 1};
	      int incr_y[] = {0, -1, -1, -1,  0,  1, 1, 1};
	      int i;

	      for (i=0;i<8;i++) {
		x22 = x2 + lx*incr_x[i];
		y22 = y2 + ly*incr_y[i];
		distcurrent = sqrt((x1-x22)*(x1-x22)+(y1-y22)*(y1-y22));
		if (distcurrent<dist) dist = distcurrent;
	      }
	    }


	    i = (int) (dist+0.5);
	    sig->dataY[i] += (log(ext1->mod)-mean1)*(log(ext2->mod)-mean2);
	    nb[i]++;
	    
	  } /* Loop end for gr on l2 */
	} /* Loop end for l2 on e2 */
      } /* Loop end for gr on l1 */
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


/*
 * the following routine is used inside e_tag_vc_TclCmd_
 */
void tag_extremum (Extremum *extr) {
  Extremum *ext;
  Line *l;

  /* copy extr */
  ext = extr;
  
  /* tag ext, i.e. fill field next */
  ext->next = ext;

  /* tag the linee that contain the extremum */
  l = (Line *) ext->line;
  l->tag = 1;

  /* repeat that downside a vertical line */
  while (ext->down) {
    ext = ext->down;
    ext->next = ext;
    l = (Line *) ext->line;
    l->tag = 1;
  }

}
  
/****************************************
 * Command name in xsmurf : eitagvc
 ****************************************/
int
e_tag_vc_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "E",
    "-border",           "",
    "-notborder",        "",
    "-border2",          "",
    "-decrease",         "d",
    "-decrease2",        "d",
    "-trigger_low",      "f",
    "-trigger_up",       "f",
    "-in",               "ff",
    "-scale",            "d",
    "-meanmod",          "fs",
    "-decrease3",        "d",
    "-increase3",        "dds",
    "-nullholder",       "df",
    "-nullholder2",      "df",
    "-prout",            "df",
    "-prout2",           "df",
    "-microcal",         "fdf",
    "-index",            "ddd",
    "-specified_chains", "S",
    "-dust",             "dff",
    "-dust2",            "ff",
    "-dust3",            "dff",
    "-user",             "s",
    "-arg",              "ff", 
    "-dust4",            "df",
    "-mask",             "I",
    "-solar",            "ff", 
    NULL
  };
  
  char * help_msg =
  {
    ("  Tag vertical chain of that are on an ext image according to a given "
     "criteria.\n"
     "use this command after chain command.\n"
     "\n"
     "Parameters:\n"
     "  ext image  - Ext image to treat.\n"
     "\n"
     "Options:\n"
     "  -border: Tag vc which upper modulus is lesser than the lower "
     "     modulus.\n"
     "  -notborder: Inverse of previous option.\n"
     "  -border2:\n"
     "  -decrease:\n"
     "  -decrease2:\n"
     "  -trigger_low : Tag vc if the modulus at the smallest scale \n"
     "                 is under some level : \n"
     "        - float - level \n"
     "  -trigger_up : Tag vc if the modulus at the smallest scale \n"
     "                 is above the level : float\n"
     "  -in         : Tag vc if the modulus at the smallest scale is inside\n"
     "                 some range\n"
     "  -scale      : Tag vc which still exist above a certain scale.\n"
     "                TAKE CARE GIVING A VALID SCALE!!!\n"
     "                that is an integer between zero (lower scale)\n"
     "                and noct*nvoice-1 \n"
     "  -meanmod    : Tag vc when at lower scale meanmod is above a certain\n"
     "                threshold\n"
     "      - float  - threshold\n"
     "      - string -\n"
     "                it also creates an ext_ima with horizontal lines (from\n"
     "                lower scale) that contains tagged MMMTO \n"
     "  -decrease3      : Tag vc if\n"
     "                modulus is decreasing between scale0 and scale\n"
     "                scaledec\n" 
     "  -increase3      : same as above with increasin\n"
     "  -nullholder     : tag a vertical chain if holder exposent estimated\n"
     "                    is near zero\n"
     "  -nullholder2     : tag a vertical chain if holder exposent estimated\n"
     "                     is near zero\n"
     "  -prout   : old thing, forget it \n"
     "  -prout2  : old thing, forget it \n"
     "  -microcal : to the segmentation of vc corresponding to microcal\n"
     "      - real    - threshold for lmod     \n"
     "      - integer - threshold for vc-length\n"
     "      - real    - threshold for (lmod - umod)/umod\n"
     "  -index\n"
     "    3 integers : index_th, incr_plus and incr_moins\n"
     "  -specified_chains : tag only chains specified by the position at\n"
     "                      lowest scale, in a REALY signal\n"
     "                      recall that pos = x + lx * y\n"
     "  -dust         : tag vertical chains such that length is below\n"
     "                  length_thresh and estimated holder is above\n"
     "                  holder_thresh\n"
     "                  created in june 2002 for interstellar clouds study\n"
     "      - integer - length_thresh\n"
     "      - float   - holder_thresh\n"
     "      - float   - holder_thresh2\n"     
     "  -dust2        : tag vertical chains that their trajectory go through\n"
     "                  a straight line which equation is given by its\n"
     "                  slope and intercept\n"
     "      - float - slope\n"
     "      - float - intercept\n"
     "  -dust3        : tag vertical chains such that length is below\n"
     "                  length_thresh and estimated holder is below\n"
     "                  holder_thresh\n"
     "                  created in june 2002 for interstellar clouds study\n"
     "      - integer - length_thresh\n"
     "      - float   - holder_thresh\n"
     "      - float   - holder_thresh2\n"     
     "  -user         : tag vertical chain defined by user\n"
     "      - string - a Tcl list of integers (position of extremun at \n"
     "                 lowest scale.\n"
     "  -arg          : tag a vertical chain...\n"
     "  -dust4        : tag vertical chains such that length is above\n"
     "                  length_thresh and estimated holder is above\n"
     "                  holder_thresh\n"
     "                  created in february 2006 for interstellar turbulence study\n"
     "      - integer - length_thresh\n"
     "      - float   - holder_thresh\n"
     "  -mask[I]      : tag a vertical chain according to an image mask used\n"
     "                  to make segregation at lowest scale\n"
     "  -solar[ff]    : tag a vertial chain if wtmm at highest scale satisfies :\n"
     "                  log_2 M >= \alpha log_2 a + \beta.\n"
     "                  \alpha and \beta are the given parameters, describing a\n"
     "                  line in the plane (log_2 a, log_2 M)\n"
     "\n"
     "Return value:\n"
     "  Name of the resulting signal.")
  };
  
  /* Command's parameters */
  ExtImage *ei;
  
  /* Options's presence */
  int isBorder;
  int isNotborder;
  int isBorder2;
  int isDecrease;
  int isDecrease2;
  int isTriggerlow;
  int isTriggerup;
  int isIn;
  int isScale;
  int isMeanMod;
  int isDecrease3;
  int isIncrease3;
  int isNullHolder;   /* 30 janv 2001 */
  int isNullHolder2;
  int isProut;
  int isProut2;
  int isMicro = 0; /* april 24  2001 */
  int isIndex = 0; /* april 25  2001 */
  //int isspecif= 0; /* july 28th 2001 */
  int isDust = 0; /* june 5th 2002*/
  int isDust2 = 0;
  int isDust3 = 0; /* june 12th 2002 */
  int isUser = 0;  /* june 18th 2002 */
  int isDust4 = 0; /* february 13th 2006 */
  int isMask = 0; /* may 2006 */
  int isSolar = 0; /* September 29th, 2009 */


  /* Options's parameters */
  int decLim;
  int scaledecrease;
  int decreasing = 0;
  int increasing = 0;
  int highscale;
  int count;
  int hsize;                /* used with increase3 option */
  int vc_length;            /* used with nullholder option */
  real nullholder_thresh;  /* used with nullholder option */
  real nullholder_rate;    /* used with nullholder option */
  real sigmamod;           /* used with nullholder option */
  real prodmod;            /* used with nullholder option */
  Image *mask;

  /* Other variables */
  Line     *l, *ll;
  List     *tagged_lines_list; /* used with isMeanMod option */
  Extremum *ext;
  ExtImage *extimatag;
  ExtImage *extimanotag;
  ExtImage *extimatag_highscale;
  Extremum *extremumtag;
  Extremum *extremumnotag;
  Extremum *extremumtag_highscale;
  char     *extimaNametag;
  char     *extimaNamenotag;
  char     *extimaNametag_highscale;
  
  real     uMod, lMod;
  real     maxMod, minMod, someMod;
  real     thresh;
  real     threshlow;
  real     threshup;
  int      scale;
  int      scalenb;
  int      nb = 0;
  int      decCount;
  int      maxPos, minPos, extPos;
  int      curPos;
  real     mmod;
  real     current_mmod;
  real     lMod_th, ratio_th;
  int      vc_length_th = 0;
  int      decreasing_mod = 1; /* used to know if the module the chain is decreasing*/
  int      incr_plus  = 1;
  int      incr_moins = 1;
  int      index, index_th;

  real     oldMod;
  int      total_size = 0; /* used with isMeanMod option */
  int      nbtot = 0;

  real     holder,holder2,holder_th,holder_th2;
  real     intercept, slope;
  int      dust2_test;

  char *position_lst_str;
  char **position_elt;
  int  code, position_lst_size;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ei) == TCL_ERROR)
    return TCL_ERROR;

  isBorder = arg_present(1);
  isNotborder = arg_present(2);
  isBorder2 = arg_present(3);
  
  isDecrease = arg_present(4);
  if (isDecrease) {
    if (arg_get(4, &decLim) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isDecrease2 = arg_present(5);
  if (isDecrease2) {
    if (arg_get(5, &decLim) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isTriggerlow = arg_present(6);
  if (isTriggerlow) {
    if (arg_get(6, &thresh) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isTriggerup = arg_present(7);
  if (isTriggerup) {
    if (arg_get(7, &thresh) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isIn = arg_present(8);
  if (isIn) {
    if (arg_get(8, &threshlow, &threshup) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isScale = arg_present(9);
  if(isScale) {
    if (arg_get(9, &scale) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  isMeanMod = arg_present(10);
  if(isMeanMod) {
    if (arg_get(10, &mmod, &extimaNametag) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isDecrease3 = arg_present(11);
  if(isDecrease3) {
    if (arg_get(11, &scaledecrease, &extimaNametag, &extimaNamenotag) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isIncrease3 = arg_present(12);
  if(isIncrease3) {
    if (arg_get(12, &scaledecrease, &highscale, &extimaNametag_highscale) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isNullHolder = arg_present(13);
  if(isNullHolder) {
    if (arg_get(13, &vc_length, &nullholder_thresh) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isNullHolder2 = arg_present(14);
  if(isNullHolder2) {
    if (arg_get(14, &vc_length, &nullholder_thresh) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isProut = arg_present(15);
  if(isProut) {
    if (arg_get(15, &vc_length, &thresh) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isProut2 = arg_present(16);
  if(isProut2) {
    if (arg_get(16, &vc_length, &thresh) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isMicro = arg_present(17);
  if(isMicro) {
    if (arg_get(17, &lMod_th, &vc_length_th, &ratio_th) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isIndex = arg_present(18);
  if(isIndex) {
    if (arg_get(18, &index_th, &incr_plus, &incr_moins) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isDust = arg_present(20);
  if(isDust) {
    if (arg_get(20, &vc_length_th, &holder_th, &holder_th2) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isDust2 = arg_present(21);
  if(isDust2) {
    if (arg_get(21, &slope, &intercept) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isDust3 = arg_present(22);
  if(isDust3) {
    if (arg_get(22, &vc_length_th, &holder_th, &holder_th2) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isUser = arg_present(23);
  if(isUser) {
    if (arg_get(23, &position_lst_str) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isDust4 = arg_present(25);
  if(isDust4) {
    if (arg_get(25, &vc_length_th, &holder_th) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isMask = arg_present(26);
  if(isMask) {
    if (arg_get(26, &mask) == TCL_ERROR) {
      return TCL_ERROR;
    }
  } 

  isSolar = arg_present(27);
  if(isSolar) {
    if (arg_get(27, &slope, &intercept) == TCL_ERROR) {
      return TCL_ERROR;
    }  
  }

  /* Parameters validity and initialisation */
  if ( isTriggerlow && isTriggerup) {
    sprintf (interp -> result,"You can't use options -triggerup and -triggerlow together.\n");
    return TCL_ERROR;
  }

  if ( isTriggerlow && isIn) {
    sprintf (interp -> result,"You can't use options -triggerlow and -in together.\n");
    return TCL_ERROR;
  }

  /* Treatement */
  if (isUser) {
    code = Tcl_SplitList(interp, position_lst_str, &position_lst_size, (CONST char ***)&position_elt);
    if (code == TCL_ERROR)
      return TCL_ERROR;
  }


  /* reset tag */
 foreach (l, ei->line_lst) {
   l->tag = 0;
   foreach (ext, l->gr_lst) {
     if (!ext->up && !ext->down) {
       continue;
     }
     ext->next = NULL;
   }
 }
 while (ei->up) { /* repeat reset for every extImage in skeleton */
   ei = ei->up;
   foreach (l, ei->line_lst) {
     l->tag = 0;
     foreach (ext, l->gr_lst) {
       if (!ext->up && !ext->down) {
	 continue;
       }
       ext->next = NULL;
     }
   }
 }
 /* go back downscale */
 while (ei->down) { /* repeat reset for every extImage in skeleton */
   ei = ei->down;
 }

 /* tag procedure */
 if (isMeanMod) {
    /* first init variable tagged_lines_list */
    tagged_lines_list = lst_create ();

    foreach (l, ei->line_lst) {
      current_mmod = l->mass / l->size;
      if (current_mmod >= mmod) {
	
      
	/* now we can suppose current_mmod >= mmod */
	/* first we increment total_size */
	total_size += l->size;
	/* then add l to tagged_lines_list */
	lst_add (l, tagged_lines_list);
	/* and now we are going to tag all the MMMTO of */
	/* these lines */
      }
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* Get the modulus of the lowest wtmmm */
	while (ext->down) {
	  ext = ext->down;
	}
	lMod = ext->mod;
	
	decCount = 0; /* indicateur de croissance */
	curPos = 0;
	
	/* Follow the chain down to up */
	ext->next = NULL;
	maxMod = ext->mod;
	maxPos = curPos;
	someMod = 0.0;
	scalenb = 0;
	while (ext->up) {
	  if (ext->mod > ext->up->mod) {
	    decCount++;
	  }
	  if (ext->mod > maxMod) {
	    maxMod = ext->mod;
	    maxPos = curPos;
	  }
	  if (isDust4 && scalenb == vc_length)
	    someMod = ext->mod;

	  /* go up to update */ 
	  ext = ext->up;
	  scalenb++;
	  ext->next = NULL;
	  curPos++;
	}
	/* modulus at the top of the vc */
	uMod = ext->mod;
	
	
	/* ca y est ! on tagge : on remplit le champ next des "ext MMMTO" */
	if (current_mmod >= mmod) {
	  tag_extremum(ext);
	  nb++;
	}
      }
    }
  } else {

    foreach (l, ei->line_lst) {
      current_mmod = l->mass / l->size;    
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* Get the modulus of the wtmmm at lowest scale */
	while (ext->down) {
	  ext = ext->down;
	}
	lMod = ext->mod;
	extPos = ext->pos;

	decCount = 0;  /* decrease count */
	curPos = 0;
	
	/* Follow the chain down to up */
	ext->next = NULL;
	maxMod = ext->mod;
	minMod = ext->mod;
	maxPos = curPos;
	minPos = curPos;
	scalenb = 0;
	sigmamod = 0.0;
	prodmod = 1.0;
	oldMod = ext->mod;
	index = 0;
	dust2_test = 0;

	while (ext->up) {
	  if (ext->mod > ext->up->mod) {
	    decCount++;
	  }
	  if (ext->mod > maxMod) {
	    maxMod = ext->mod;
	    maxPos = curPos;
	  }
	  if (ext->mod < minMod) {
	    minMod = ext->mod;
	    minPos = curPos;
	  }
	  /* compute sum of the vc_length first modulus in 
	     the vertical chain */
	  if (isNullHolder) {
	    if (scalenb < vc_length){
	      sigmamod += ext->mod;
	    }
	  } else if (isNullHolder2) {
	    if (scalenb < vc_length){
	      prodmod *= ext->mod;
	    }
	  }

	  if (isDecrease3) {
	    /*if (scalenb >= lengthdecrease) {
	      toshort = 0;
	    } */
	    if  (scalenb <= scaledecrease) {
	      if (ext->mod < lMod) {
		decreasing = 1;
	      } else {
		decreasing = 0;
	      }
	    }
	  }
	  if (isIncrease3) {
	    if  (scalenb <= scaledecrease) {
	      if (ext->mod > lMod) {
		increasing = 1;
	      } else {
		increasing = 0;
	      }
	    }
	  }
	  if (isDust2) {
	    if ((log(ext->mod)-slope*(scalenb/10)-intercept)>=0 && dust2_test==0) {
	      dust2_test=1;
	    }
	  }
	  
	  ext = ext->up;
	  scalenb++;
	  ext->next = NULL;
	  curPos++;
	  if (ext->mod > oldMod) {
	    decreasing_mod = 0;
	    index += incr_plus;
	  } else {
	    index -= incr_moins;
	  }
	  oldMod = ext->mod;
	} /* end while */
	uMod = ext->mod;

	nbtot++;
	
	if (isBorder) {
	  if (uMod < lMod) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isNotborder) {
	  if (uMod > lMod) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isBorder2) {
	  real zeMod = uMod;
	  if (uMod < lMod) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isDecrease) {
	  if (decCount >= decLim ) {
	    tag_extremum(ext);
	    nb++;
	  } else if ((scalenb < decLim - 1) & (decCount>=3)) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isDecrease2) {
	  if ((curPos-maxPos) >= decLim) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isTriggerlow) {
	  if (lMod <= thresh) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isTriggerup) {
	  if (lMod > thresh) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isIn) {
	  if (lMod >= threshlow && lMod < threshup) {
	    tag_extremum(ext);
	    nb++;
	  }
	}  else if (isScale) {
	  if (scalenb > scale) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isDecrease3) {
	  if (decreasing == 1 /*|| toshort == 1 */) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isIncrease3) {
	  if (increasing == 1) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isNullHolder) {
	  if (scalenb < vc_length) {
	    nullholder_rate = nullholder_thresh;
	  } else {
	    nullholder_rate = fabs(sigmamod / vc_length / lMod - 1);
	  }
	  if (nullholder_rate < nullholder_thresh) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isNullHolder2) {
	  if (scalenb < vc_length) {
	    nullholder_rate = nullholder_thresh;
	  } else {
	    nullholder_rate = fabs(sigmamod / pow(lMod, vc_length) - 1);
	  }
	  if (nullholder_rate < nullholder_thresh) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isProut) {
	  if (scalenb > vc_length && lMod > thresh) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isProut2) {
	  if (scalenb < vc_length && lMod < thresh) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isMicro && isIndex == 0) {
	  if (scalenb >= vc_length_th && current_mmod >= lMod_th && (lMod-uMod)/uMod >= ratio_th && lMod >= lMod_th  ) {
	    tag_extremum(ext);
	    nb++;
	  }
	}  else if (isMicro && isIndex == 1 && index <= index_th) {
	  if (scalenb >= vc_length_th && current_mmod >= lMod_th && (lMod-uMod)/uMod >= ratio_th && lMod >= lMod_th  ) {
	    tag_extremum(ext);
	    nb++;
	  }
	}  else if (isDust) {
	  holder  =10.0*log(uMod/lMod)/scalenb;
	  holder2 =10.0*log(maxMod/lMod)/scalenb;
	  if (scalenb <= vc_length_th && (holder >= holder_th || holder2 > holder_th2) ) {
	    tag_extremum(ext);
	    nb++;
	  }
	}  else if (isDust2) {
	  if (dust2_test>0) {
	    tag_extremum(ext);
	    nb++;
	  }
	}  else if (isDust3) {
	  holder  =10.0*log(minMod/lMod)/scalenb;
	  holder2 =10.0*log(uMod/minMod)/scalenb;
	  if (scalenb <= vc_length_th && holder < holder_th && holder2 >= holder_th2) {
	    tag_extremum(ext);
	    nb++;
	  }
	}  else if (isDust4) {
	  real holder1, holder2;

	  holder1  = 10.0*log(someMod/lMod)/scalenb;
	  holder2  = 10.0*log(uMod/lMod)/scalenb;

	  /*printf("holder %f, length %d\n",holder,scalenb);*/
	  if ((scalenb < vc_length_th && holder2 < holder_th) || (scalenb >= vc_length_th && holder2 < holder_th) ) {
	    tag_extremum(ext);
	    nb++;
	  }
	} else if (isMask) {
	  if (mask->data[extPos]>0.0 || mask->data[extPos]<0.0) {
	    tag_extremum(ext);
            nb++;
	  }
	} else if (isSolar) {
	  real tmp = slope * scalenb/10.0 + intercept;
	  if (log(uMod)/log(2) >= tmp) {
	    tag_extremum(ext);
	    nb++;
	  } 
	}
      } /* Loop end for gr on l */
    } /* Loop end for l on ei */
  }

  /* new loop to create an ExtImage which shows
     tagged points locations */
  if ( 0 == 1 ) { 
    /* create the ExtImage extima */
    extimatag   = w2_ext_new(nb,ei->lx,ei->ly,ei->scale);
    extimanotag = w2_ext_new(nbtot-nb,ei->lx,ei->ly,ei->scale);
    if (!extimatag)
      return GenErrorMemoryAlloc(interp);
    if (!extimanotag)
      return GenErrorMemoryAlloc(interp);


    /* memory allocation for extremumtag and extremumnotag */
    /*extremumtag   = (Extremum *) malloc (nb*sizeof(Extremum));
    extremumnotag = (Extremum *) malloc ((nbtot-nb)*sizeof(Extremum));*/
    
    extremumtag   = extimatag->extr;
    extremumnotag = extimanotag->extr;
    
    foreach (l, ei->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* then we go down allong the vert chain */
	while (ext->down) {
	  ext = ext->down;
	}
	/* test to know if ext is tagged */
	/* if so we fill fields mod arg and pos of extremum */
  	/*if (!ext->next) {
	  extremumtag->mod = ext->mod;
	  extremumtag->arg = ext->arg;
	  extremumtag->pos = ext->pos;
	  extremumtag++;
	  } else {
	  extremumnotag->mod = ext->mod;
	  extremumnotag->arg = ext->arg;
	  extremumnotag->pos = ext->pos;
	  extremumnotag++;
	}*/
	if (ext->next == NULL) {
	  extremumnotag->mod = ext->mod;
	  extremumnotag->arg = ext->arg;
	  extremumnotag->pos = ext->pos;
	  extremumnotag++;
	} else {
	  extremumtag->mod = ext->mod;
	  extremumtag->arg = ext->arg;
	  extremumtag->pos = ext->pos;
	  extremumtag++;
	}
      } /* end of foreach (ext, l->gr_lst) loop */
    } /* end of foreach (l, ei->line_lst) loop */

    /* ajout de la structure ExtImage au dictionnaire adequat */
    ExtDicStore(extimaNametag,extimatag);
    ExtDicStore(extimaNamenotag,extimanotag); 
    /*free(extremumtag);
      free(extremumnotag);*/
  } else if ( isMeanMod ) {
    extimatag   = w2_ext_new(total_size,ei->lx,ei->ly,ei->scale);
    if (!extimatag)
      return GenErrorMemoryAlloc(interp);
    extremumtag   = extimatag->extr;
    
    foreach (l, tagged_lines_list) {
      foreach (ext, l->ext_lst) {
	/*if (!ext->up && !ext->down) {
	  continue;
	}*/
	/* then we go down allong the vert chain */
	/*while (ext->down) {
	  ext = ext->down;
	}*/
	/* test to know if ext is tagged */
	/* if so we fill fields mod arg and pos of extremum */
	/*if (ext->next == NULL) {*/
	  /*extremumnotag->mod = ext->mod;
	  extremumnotag->arg = ext->arg;
	  extremumnotag->pos = ext->pos;
	  extremumnotag++;*/
	/*} else {*/
	  
	if (ext->mod > 0) {
	extremumtag->mod = ext->mod;
	extremumtag->arg = ext->arg;
	extremumtag->pos = ext->pos;
	extremumtag++;
	} else {
	}
	/*}*/
      } /* end of loop foreach (ext, l->gr_lst) */
    } /* end of loop foreach (l, tagged_lines_lst) */
    
    /* ajout de la structure ExtImage au dictionnaire adequat */
    ExtDicStore(extimaNametag,extimatag);
    /*ExtDicStore(extimaNamenotag,extimanotag);*/ 
    
    lst_destroy (tagged_lines_list);
  }

  if (isIncrease3) {
    /* what's extremumtag_highscale's size ? */
    hsize = 0; /* at the end of the two following loops
		  hsize will be extremumtag_highscale! */
     foreach (l, ei->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* then we go down allong the vert chain */
	while (ext->down) {
	  ext = ext->down;
	}
	
	/* then we go up to scale : "highscale" */
	count = 0;
	while (ext->up && count <= highscale) {
	  ext = ext->up;
	  count++;
	}
	if (count > highscale) {  /* it means we get out of last loop by the second condition */
	  hsize++;
	}
      } /* end of foreach (ext, l->gr_lst) loop */
    } /* end of foreach (l, ei->line_lst) loop */
  

   extimatag_highscale   = w2_ext_new(hsize,ei->lx,ei->ly,ei->scale);
    if (!extimatag_highscale)
      return GenErrorMemoryAlloc(interp);
    extremumtag_highscale   = extimatag_highscale->extr;
    /* now we can fill array extremumtag_highscale */
    foreach (l, ei->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* then we go down allong the vert chain */
	while (ext->down) {
	  ext = ext->down;
	}
	
	/* then we go up to scale : "highscale" */
	count = 0;
	while (ext->up && count <= highscale) {
	  ext = ext->up;
	  count++;
	}
	if (count > highscale) {  /* it means we get out of last loop by the second condition */
	  extremumtag_highscale->mod = ext->mod;
	  extremumtag_highscale->arg = ext->arg;
	  extremumtag_highscale->pos = ext->pos;
	  extremumtag_highscale++;
	}
      } /* end of foreach (ext, l->gr_lst) loop */
    } /* end of foreach (l, ei->line_lst) loop */
    
    /*_QuickSort_Extr_(extremumtag_highscale-1,hsize);*/

    ExtDicStore(extimaNametag_highscale,extimatag_highscale); 
  }
  
  sprintf (interp->result, "nbtag %d nbtot %d", nb, nbtot);
  
  return TCL_OK;
}



/****************************************
  command name in xsmurf : eivchisto
  ***************************************/
int
e_vc_histo_TclCmd_ (ClientData clientData,
		    Tcl_Interp *interp,
		    int        argc,
		    char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "E",
    "-length",  "s",
    "-tag",     "s",
    "-untag",   "s",
    "-meanh",   "s",
    NULL
  };

  char * help_msg =
  {
    ("  Make some stats on vertical chains (size, ...)\n"
     "\n"
     "use this command AFTER chain command and eventually\n"
     "after eitagvc command.\n"
     "\n"
     "Parameters:\n"
     "  ext image  - Ext image to treat.\n"
     "               this extimage must belong to a skeleton\n"
     "\n"
     "Options:\n"
     "  -length [s]  : create a signal with vertical chains'sizes \n"
     "  -tag [s]     : create a signal with tagged vc'sizes \n"
     "  -untag [s]   : same as above with untagged points. \n"
     "  -meanh [s]   : create a signal with vertical chains mean Holder \n"
     "\n"
     "Return value:\n"
     "  None.")
  };

  /* Command's parameters */
  ExtImage *extima;
  
  /* Options's presence */
  int isLength;
  int isTag;
  int isUntag;
  int isMeanh;

  /* Options's parameters */
  int nb_vc;
  int nb_vc_tag;
  int nb_vc_untag;
  int length;
  int count;
  int counttag;
  int countuntag;

  /* Other variables */
  Line     *l;
  Extremum *ext;
  char     *histo_name;
  char     *histo_name_tag;
  char     *histo_name_untag;
  char     *histo_name_meanh;
  Signal   *histo;
  Signal   *histotag;
  Signal   *histountag;
  Signal   *histomeanh;

  int thelength, thesum;
  float t, st2, slope;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extima) == TCL_ERROR)
    return TCL_ERROR;

  isLength = arg_present(1);
  if(isLength) {
    if (arg_get(1, &histo_name) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isTag = arg_present(2);
  if(isTag) {
    if (arg_get(2, &histo_name_tag) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isUntag = arg_present(3);
  if(isUntag) {
    if (arg_get(3, &histo_name_untag) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isMeanh = arg_present(4);
  if(isMeanh) {
    if (arg_get(4, &histo_name_meanh) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* How many vertical chain's ?? */
  nb_vc = 0;
  nb_vc_tag = 0;
  nb_vc_untag = 0;

  foreach (l, extima->line_lst) {
    foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	
	nb_vc++;
	if (ext->next == NULL) {
	  nb_vc_untag++;
	} else {
	  nb_vc_tag++;
	}
    } /* end of foreach(ext, l->gr_lst ) loop  */
  }   /* end of foreach(l, ei->line_lst) loop  */

  sprintf (interp->result, "nb_vc_tag %d\n nb_vc_untag %d", nb_vc_tag, nb_vc_untag);
  /*return TCL_OK;*/

  /* on alloue de la memoire pour l'histogramme */
  histo      = sig_new (REALY, 0, nb_vc-1      );
  if ((nb_vc_tag > 0) && (nb_vc_untag > 0)) {
    histotag   = sig_new (REALY, 0, nb_vc_tag-1  );
    histountag = sig_new (REALY, 0, nb_vc_untag-1);
  }
  if (isMeanh) {
    histomeanh = sig_new (REALXY, 0, nb_vc-1      );
  }

  
  /* on remplit histo */
  if (isLength) {
    count = 0;
    counttag = 0;
    countuntag = 0;

    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	
	/* Follow the chain down to up */
	length = 0;
	while (ext->up) {
	  ext = ext->up;
	  length++;
	}
	histo->dataY[count] = length;
	count++;

	if ((nb_vc_tag > 0) && (nb_vc_untag > 0)) {

	  if (ext->next == NULL) {
	    histountag->dataY[countuntag] = length;
	    countuntag++;
	  } else {
	    histotag->dataY[counttag] = length;
	    counttag++;
	  }
	}
	
      } /* end of foreach(ext, l->gr_lst ) loop  */
    }   /* end of foreach(l, ei->line_lst) loop  */
  } else if (isMeanh) {
    count = 0;
    counttag = 0;
    countuntag = 0;
    
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	
	/* Follow the chain down to up */
	thelength = 0;
	thesum = 0;
	while (ext->up) {
	  ext = ext->up;
	  thelength++;
	  thesum += thelength;
	}
	while (ext->down) {
	  ext = ext->down;
	}
	
	length = 0;
	t = length - thesum/(thelength+1);
	st2 = t * t;
	slope  = t * log(ext->mod)/log(2.0);
	while (ext->up) {
	  ext = ext->up;
	  length++;
	  t = length - thesum/(thelength+1);
	  st2 += t * t;
	  slope += t * log(ext->mod)/log(2.0);
	}
	slope = slope/st2*10;

	histomeanh->dataY[count] = slope;
	histomeanh->dataX[count] = thelength;
	count++;
	
	if ((nb_vc_tag > 0) && (nb_vc_untag > 0)) {
	  
	  if (ext->next == NULL) {
	    histountag->dataY[countuntag] = slope;
	    countuntag++;
	  } else {
	    histotag->dataY[counttag] = slope;
	    counttag++;
	  }
	}
	
      } /* end of foreach(ext, l->gr_lst ) loop  */
    }   /* end of foreach(l, ei->line_lst) loop  */
  }

  if ((nb_vc_tag > 0) && (nb_vc_untag > 0)) {
    store_signal_in_dictionary(histo_name_tag, histotag);
    store_signal_in_dictionary(histo_name_untag, histountag);
  }
  
  if (isLength) {
    store_signal_in_dictionary(histo_name, histo);
  } else if (isMeanh) {
    store_signal_in_dictionary(histo_name_meanh, histomeanh);
  }

  return TCL_OK;

}

/****************************************
  command name in xsmurf : eivchisto2
  ***************************************/
int
e_vc_histo2_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "E",
    "-length",  "",
    "-tag",     "",
    "-untag",   "",
    "-mod",    "d",
    NULL
  };

  char * help_msg =
  {
    ("  Make some stats on vertical chains (size, ...)\n"
     "\n"
     "use this command AFTER chain command and eventually\n"
     "after eitagvc command.\n"
     "\n"
     "Parameters:\n"
     "  ext image  - Ext image to treat.\n"
     "               this extimage must belong to a skeleton\n"
     "\n"
     "Options:\n"
     "  -length    : return list of vertical chains'sizes \n"
     "  -mod [d]   : at specified length return module of\n"
     "               remaining vc chains\n"
     "  -tag       : return list of tagged points \n"
     "  -untag     : return list of untagged points \n"
     "\n"
     "Return value:\n"
     "  list of values.\n")
  };

  /* Command's parameters */
  ExtImage *extima;

  /* Options's presence */
  int isLength;
  int isTag;
  int isUntag;
  int isMod;

  /* Options's parameters */
  int nb_vc;
  int nb_vc_tag;
  int nb_vc_untag;
  int length;
  char val_str[30];
  int specif_length;
  int thelength;

  /* Other variables */
  Line     *l;
  Extremum *ext;


  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extima) == TCL_ERROR)
    return TCL_ERROR;

  isLength = arg_present(1);
  isTag    = arg_present(2);
  isUntag  = arg_present(3);
  isMod    = arg_present(4);
  if(isMod) {
    if (arg_get(4, &specif_length) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  
  /* How many vertical chain's ?? */
  nb_vc       = 0;
  nb_vc_tag   = 0;
  nb_vc_untag = 0;

  foreach (l, extima->line_lst) {
    foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	
	nb_vc++;
	if (ext->next == NULL) {
	  nb_vc_untag++;
	} else {
	  nb_vc_tag++;
	}
    } /* end of foreach(ext, l->gr_lst ) loop  */
  }   /* end of foreach(l, ei->line_lst) loop  */

  /*sprintf (interp->result, "nb_vc_tag %d\n nb_vc_untag %d", nb_vc_tag, nb_vc_untag);*/

  /* main */
  if (isLength) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	
	/* Follow the chain down to up */
	length = 0;
	while (ext->up) {
	  ext = ext->up;
	  length++;
	}

	if (isTag) {
	  if (ext->next == NULL) {
	  } else {
	    sprintf(val_str, "%d", length);
	    Tcl_AppendElement(interp, val_str);
	  }
	} else if (isUntag){
	  if (ext->next == NULL) {
	    sprintf(val_str, "%d", length);
	    Tcl_AppendElement(interp, val_str);
	  } else {
	  }
	} else {
	  sprintf(val_str, "%d", length);
	  Tcl_AppendElement(interp, val_str);
	}
	
	
      } /* end of foreach(ext, l->gr_lst ) loop  */
    }   /* end of foreach(l, ei->line_lst) loop  */
  } else if (isMod) {
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* Follow the chain down to up */
	thelength = 0;
	while (ext->up) {
	  ext = ext->up;
	  thelength++;
	}
	if (thelength < specif_length) {
	  continue;
	} else {
	  while (ext->down) {
	    ext = ext->down;
	  }
	  length = 0;
	  while (ext->up) {
	    ext = ext->up;
	    length++;
	    if (length == specif_length) {
	      if (isTag) {
		if (ext->next == NULL) {
		} else {
		  sprintf(val_str, "%d %f", thelength, ext->mod);
		  Tcl_AppendElement(interp, val_str);
		}
	      } else if (isUntag){
		if (ext->next == NULL) {
		  sprintf(val_str, "%d %f", thelength, ext->mod);
		  Tcl_AppendElement(interp, val_str);
		} else {
		}
	      } else {
		sprintf(val_str, "%d %f", thelength, ext->mod);
		Tcl_AppendElement(interp, val_str);
	      }
	      break;
	    }
	  }
	}


      } /* end of foreach(ext, l->gr_lst ) loop  */
    }   /* end of foreach(l, ei->line_lst) loop  */
  }
  
  return TCL_OK;
  
}


/****************************************
 * Command name in xsmurf : eivcgerbe
 ****************************************/
int
e_vc_gerbe_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Es",
    "-tag",   "ss",
    "-normalize", "f",
    "-top", "s",
    "-scalemin", "sd",
    "-between",  "sdd",
    "-decrease", "s",
    "-increase", "s",
    NULL
  };

  char * help_msg =
  {
    ("  Create a REALXY-signal (log(a), log(mod))\n"
     "  with all scale-space points that are on a vertical\n"
     "  chain\n"
     "use this command AFTER chain command!!\n"
     "\n"
     "Parameters:\n"
     "  ext image  - Ext image to treat.\n"
     "               this extimage must belong to a skeleton\n"
     "  string     - name of the resulting signal\n"
     "\n"
     "Options:\n"
     "   -tag [ss]       : creates two signals : one for tagged points\n"
     "                     and the other for untagged points\n"
     "   -normalize [f]  : substract 0.5*log(a) to log(mod)\n"
     "   -top [s]        : creates a signal (log(a), log(mod))\n"
     "                     only with points at top of vertical chains\n"
     "   -scalemin [sd]  : display chains that still exist above that integer\n"
     "   -between  [sdd] : same as above but keep chains whose top is between\n"
     "                     scale_low and scale_up\n"
     "   -decrease [s]\n"
     "   -increase [s]\n"
     "\n"
     "Return value:\n"
     "  None.\n")
  };

  /* Command's parameters */
  ExtImage *extima;
  int total_length, total_length_tag, total_length_untag, total_length_above_scalemin, total_length_between, total_length_decrease, total_length_increase;
  int length, length2;
  int count, count_tag, count_untag;
  int nb_vc, nb_vc_above_scalemin;
  int scalemin = 0;
  int scale_up = 0;
  int scale_low = 0;
  real lmod = 0;
  real umod = 0;

  /* Options's presence */
  int isTag;
  int isNormalize;
  int isTop;
  int isScalemin;
  int isBetween;
  int isDecrease;
  int isIncrease;

  /* Options's parameters */
  real slope_normalize;

  /* Other variables */
  Line     *l;
  Extremum *ext;
  char     *gerbe_name, *tag_name, *untag_name, *top_name, *scalemin_name, *between_name, *decrease_name, *increase_name;
  Signal   *gerbe, *gerbe_tag, *gerbe_untag, *top_sig, *scalemin_sig, *between_sig, *increase_sig, *decrease_sig;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &extima, &gerbe_name) == TCL_ERROR)
    return TCL_ERROR;

  isTag = arg_present(1);
  if(isTag) {
    if (arg_get(1, &tag_name, &untag_name) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isNormalize = arg_present(2);
  if(isNormalize) {
    if (arg_get(2, &slope_normalize) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isTop = arg_present(3);
  if(isTop) {
    if (arg_get(3, &top_name) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isScalemin = arg_present(4);
  if(isScalemin) {
    if (arg_get(4, &scalemin_name, &scalemin) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isBetween = arg_present(5);
  if(isBetween) {
    if (arg_get(5, &between_name, &scale_low, &scale_up) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isDecrease = arg_present(6);
  if(isDecrease) {
    if (arg_get(6, &decrease_name) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isIncrease = arg_present(7);
  if(isIncrease) {
    if (arg_get(7, &increase_name) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }


  /* compute total_length for memory allocation */
  total_length       = 0;
  total_length_tag   = 0;
  total_length_untag = 0;
  nb_vc = 0;
  nb_vc_above_scalemin = 0;
  total_length_above_scalemin = 0;
  total_length_between = 0;
  total_length_decrease = 0;
  total_length_increase = 0;

  foreach (l, extima->line_lst) {
    foreach (ext, l->gr_lst) {
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

	 if (ext->next == NULL) {
	   total_length_untag += length;
	 } else {
	   total_length_tag += length;
	 }
	total_length += length;

	if (length >= scalemin) {
	  total_length_above_scalemin += length;
	}
	if (length >= scale_low && length < scale_up) {
	  total_length_between += length;
	}
	if (umod <=lmod) {
	  total_length_decrease += length;
	} else {
	  total_length_increase += length;
	}	  
    } /* end of foreach(ext, l->gr_lst ) loop  */
  }   /* end of foreach(l, ei->line_lst) loop  */

  sprintf (interp->result, "nb_vc %d total_length %d total_length_tag %d total_length_untag %d\n", nb_vc, total_length, total_length_tag, total_length_untag);
  /*return TCL_OK;*/

  /* on alloue de la memoire pour l'histogramme */
  gerbe       = sig_new (REALXY, 0, 2*total_length-1         );
  if (isTop) {
    top_sig   = sig_new (REALXY, 0, nb_vc-1                  );
  }
  if (isScalemin) {
    scalemin_sig = sig_new (REALXY, 0, total_length_above_scalemin-1);
  }
  if (isBetween) {
    between_sig  = sig_new (REALXY, 0, 2*total_length_between-1);
  }
  if (isDecrease) {
    decrease_sig  = sig_new (REALXY, 0, total_length_decrease-1);
  }
  if (isIncrease) {
    increase_sig  = sig_new (REALXY, 0, total_length_increase-1);
  }


  /* on remplit gerbe */
  count = 0;
  nb_vc = 0;

  foreach (l, extima->line_lst) {
    foreach (ext, l->gr_lst) {
      if (!ext->up && !ext->down) {
	continue;
      }

      /* Follow the chain down to up */
      length = 0;	
      gerbe->dataX[count] = length;
      gerbe->dataY[count] = log(ext->mod)/log(2) - isNormalize*slope_normalize*length/10;
      count++;
      
      while (ext->up) {
	ext = ext->up;
	length++;
	gerbe->dataX[count] = length;
	gerbe->dataY[count] = log(ext->mod)/log(2) - isNormalize*slope_normalize*length/10 ;
	count++;
      }
      while (length>=0) {
	gerbe->dataX[count] = length;
	gerbe->dataY[count] = log(ext->mod)/log(2) - isNormalize*slope_normalize*length/10 ;
	ext = ext->down;
	length--;
	count++;
      }

      if (isTop) {
	top_sig->dataX[nb_vc] = length;
	top_sig->dataY[nb_vc] = log(ext->mod)/log(2) - isNormalize*slope_normalize*length/10;
	nb_vc++;
      }

    } /* end of foreach(ext, l->gr_lst ) loop  */
  }   /* end of foreach(l, ei->line_lst) loop  */

  /*sprintf (interp->result, "total_length %d count %d\n", total_length, count);
    return TCL_OK;*/

  if (isScalemin) {
    count = 0;
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	
	/* Follow the chain down to up */
	length = 0;	
	while (ext->up) {
	  ext = ext->up;
	  length++;
	}
	if (length >= scalemin) {
	  while (ext->down) {
	    ext = ext->down;
	  }
	  length2 = 0;
	  scalemin_sig->dataX[count] = length2;
	  scalemin_sig->dataY[count] = log(ext->mod)/log(2);
	  count++;
	  while (ext->up) {
	    ext = ext->up;
	    length2++;
	    scalemin_sig->dataX[count] = length2;
	    scalemin_sig->dataY[count] = log(ext->mod)/log(2) - isNormalize*slope_normalize*length2/10 ;
	    count++;
	  }
	}
	
      } /* end of foreach(ext, l->gr_lst ) loop  */
    }   /* end of foreach(l, ei->line_lst) loop  */ 
  }

  if (isBetween) {
    count = 0;
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
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
	    between_sig->dataY[count] = log(ext->mod)/log(2) - isNormalize*slope_normalize*length2/10 ;
	    count++;
	  }
	  while (length2>=0) {
	    between_sig->dataX[count] = length2;
	    between_sig->dataY[count] = log(ext->mod)/log(2) - isNormalize*slope_normalize*length2/10 ;
	    ext = ext->down;
	    length2--;
	    count++;
	  }
	}
	
      } /* end of foreach(ext, l->gr_lst ) loop  */
    }   /* end of foreach(l, ei->line_lst) loop  */ 
  }

  if (isDecrease || isIncrease) {
    count = 0;
    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	
	/* Follow the chain down to up */
	length = 0;
	lmod = ext->mod;
	while (ext->up) {
	  ext = ext->up;
	  length++;
	}
	umod = ext->mod;

	if (umod<=lmod && isDecrease) {
	  while (ext->down) {
	    ext = ext->down;
	  }
	  length2 = 0;
	  decrease_sig->dataX[count] = length2;
	  decrease_sig->dataY[count] = log(ext->mod)/log(2);
	  count++;
	  while (ext->up) {
	    ext = ext->up;
	    length2++;
	    decrease_sig->dataX[count] = length2;
	    decrease_sig->dataY[count] = log(ext->mod)/log(2) - isNormalize*slope_normalize*length2/10 ;
	    count++;
	  }
 	}
	if (umod>lmod && isIncrease) {
	  while (ext->down) {
	    ext = ext->down;
	  }
	  length2 = 0;
	  increase_sig->dataX[count] = length2;
	  increase_sig->dataY[count] = log(ext->mod)/log(2);
	  count++;
	  while (ext->up) {
	    ext = ext->up;
	    length2++;
	    increase_sig->dataX[count] = length2;
	    increase_sig->dataY[count] = log(ext->mod)/log(2) - isNormalize*slope_normalize*length2/10 ;
	    count++;
	  }
	}
	
      } /* end of foreach(ext, l->gr_lst ) loop  */
    }   /* end of foreach(l, ei->line_lst) loop  */ 
  }

  
  store_signal_in_dictionary(gerbe_name, gerbe);
  sprintf (interp->result, "count %d\n", count);
  if (isTop) {
    store_signal_in_dictionary(top_name, top_sig);
  }
  if (isScalemin) {
    store_signal_in_dictionary(scalemin_name, scalemin_sig);
  }
  if (isBetween) {
    store_signal_in_dictionary(between_name, between_sig);
  }
  if (isDecrease) {
    store_signal_in_dictionary(decrease_name, decrease_sig);
  }
  if (isIncrease) {
    store_signal_in_dictionary(increase_name, increase_sig);
  }
   

  if (isTag) {
    /* on alloue la memoire ki faut */
    gerbe_tag   = sig_new (REALXY, 0, 2*total_length_tag-1  );
    gerbe_untag = sig_new (REALXY, 0, 2*total_length_untag-1);
    count_tag = 0;
    count_untag = 0;

    foreach (l, extima->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	
	if (ext->next == NULL) { 
	  length = 0;	
	  gerbe_untag->dataX[count_untag] = length;
	  gerbe_untag->dataY[count_untag] = log(ext->mod)/log(2);
	  count_untag++;
	  
	  while (ext->up) {
	    ext = ext->up;
	    length++;
	    gerbe_untag->dataX[count_untag] = length;
	    gerbe_untag->dataY[count_untag] = log(ext->mod)/log(2)-isNormalize*slope_normalize*length/10;
	    count_untag++;
	  }
	  while (length>=0) {
	    gerbe_untag->dataX[count_untag] = length;
	    gerbe_untag->dataY[count_untag] = log(ext->mod)/log(2)-isNormalize*slope_normalize*length/10 ;
	    ext = ext->down;
	    length--;
	    count_untag++;
	  }
	  
	} else {
	  length = 0;	
	  gerbe_tag->dataX[count_tag] = length;
	  gerbe_tag->dataY[count_tag] = log(ext->mod)/log(2);
	  count_tag++;
	  
	  while (ext->up) {
	    ext = ext->up;
	    length++;
	    gerbe_tag->dataX[count_tag] = length;
	    gerbe_tag->dataY[count_tag] = log(ext->mod)/log(2)-isNormalize*slope_normalize*length/10;
	    count_tag++;
	  }
	   while (length>=0) {
	    gerbe_tag->dataX[count_tag] = length;
	    gerbe_tag->dataY[count_tag] = log(ext->mod)/log(2)-isNormalize*slope_normalize*length/10 ;
	    ext = ext->down;
	    length--;
	    count_tag++;
	  }
	}
      } /* end of foreach(ext, l->gr_lst ) loop  */
    }   /* end of foreach(l, ei->line_lst) loop  */
    
    store_signal_in_dictionary(tag_name  , gerbe_tag  );
    store_signal_in_dictionary(untag_name, gerbe_untag);
  } 
  
  return TCL_OK;
  
}


/**************************************************
shsdhsdjfjjf
***************************************************/

/*
 * Command name in xsmurf : eisavetag
 */
int
e_save_tagged_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Ess",
    "-line", "E",
    "-line2", "E",
    "-tagext", "s",
    NULL
  };
  
  char * help_msg =
  {
    ("Save the result (at lowest scale) of tag operation \n"
     "  (segmentation of vert chain) \n"
     "Take care using this after chain and eitagvc command !!!.\n"
     "\n"
     "Parameters:\n"
     "  ext image  - Ext image to treat (one of the skeleton)\n"
     "  string     - filename for the tagged extrema extima \n" 
     "  string     - filename for the nottagged extrema extima \n" 
     "\n"
     "Options:\n"
     "    -line  [E] : save horizontal lines of a certain scale identified\n"
     "                 the ext-ima of that scale (given parameter)\n"
     "                 that contains tagged extrema.\n"
     "    -line2 [E] : save horizontal lines of a certain scale identified\n"
     "                 the ext-ima of that scale (given parameter)\n"
     "                 that contains tagged extrema.\n"
     "                 keep only lines such that number of tagged-extrema\n"
     "                 is greater (or equal) than number of un-tagged extr.\n"
     "     -tagext [s] : creates a signal with tag_ext location. Use it\n"
     "                  with line2 option\n"
     "                  attention : pas implemente car c'est plus simple de\n"
     "                  passer par eigrloop\n "
     "\n"
     "Return value:\n")
  };
  
  /* Command's parameters */
  ExtImage *ei;
  ExtImage *ei_line;
  
  /* Options's presence */
  int isLine;
  int isLine2;
  int isTagExt;
  int total_size;

  /* Options's parameters */
 
  /* Other variables */
  Line     *l;
  List     *tagged_lines_list;
  Extremum *ext;
  ExtImage *extimatag;
  ExtImage *extimanotag;

  Extremum *extremumtag;
  Extremum *extremumnotag;

  char     *extimaNametag;
  char     *extimaNamenotag;
  char     *sigext_name;

  int exttagnb = 0;
  int extnotagnb = 0;

   int nb_ext_on_vc ;
   int nb_ext_on_vc_tag ;

  /*int scale_number = 0;*/

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ei, &extimaNametag, &extimaNamenotag) == TCL_ERROR)
    return TCL_ERROR;

  isLine = arg_present(1);
  if(isLine) {
    if (arg_get(1, &ei_line) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isLine2 = arg_present(2);
  if(isLine2) {
    if (arg_get(2, &ei_line) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  isTagExt = arg_present(3);
  if(isTagExt) {
    if (arg_get(3, &sigext_name) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  /* Parameters validity and initialisation */

  /* Treatement */
  
  if (isLine) {
    total_size = 0;
    tagged_lines_list = lst_create ();
    foreach (l, ei_line->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	/* on tient extremun sur une vc, mais est-il tagge? */
	if (ext->next == NULL) {
	} else {
	  lst_add (l, tagged_lines_list);
	  total_size += l->size;
	  break;
	}
      }
    }
    extimatag   = w2_ext_new(total_size,ei->lx,ei->ly,ei->scale);
    if (!extimatag)
      return GenErrorMemoryAlloc(interp);
    extremumtag   = extimatag->extr;
    
    foreach (l, tagged_lines_list) {
      foreach (ext, l->ext_lst) {
	if (ext->mod > 0) {
	extremumtag->mod = ext->mod;
	extremumtag->arg = ext->arg;
	extremumtag->pos = ext->pos;
	extremumtag++;
	} else {
	}
      }
    }
    ExtDicStore(extimaNametag,extimatag);
  } else if (isLine2 && isLine == 0) {
    total_size = 0;
    tagged_lines_list = lst_create ();
    foreach (l, ei_line->line_lst) {
      nb_ext_on_vc     = 0;
      nb_ext_on_vc_tag = 0;
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	nb_ext_on_vc++;
	/* on tient extremun sur une vc, mais est-il tagge? */
	if (ext->next == NULL) {
	} else { /* il est tagge !! Ah Ah */
	  nb_ext_on_vc_tag++;
	}
      } /* fin boucle sur les ext de la ligne */
      /* on peut a present tester la ligne */
      if (2*nb_ext_on_vc_tag >= nb_ext_on_vc && nb_ext_on_vc_tag>0) {
	lst_add (l, tagged_lines_list);
	total_size += l->size;
      }
    }
    extimatag   = w2_ext_new(total_size,ei->lx,ei->ly,ei->scale);
    if (!extimatag)
      return GenErrorMemoryAlloc(interp);
    extremumtag   = extimatag->extr;
    
    foreach (l, tagged_lines_list) {
      foreach (ext, l->ext_lst) {
	if (ext->mod > 0) {
	extremumtag->mod = ext->mod;
	extremumtag->arg = ext->arg;
	extremumtag->pos = ext->pos;
	extremumtag++;
	} else {
	}
      }
    }
    ExtDicStore(extimaNametag,extimatag);
  } else {
    /* loop to create an ExtImage which shows
       tagged points locations */
    exttagnb = 0; /* at the end of the two following loops
		     hsize will be an integer (nb of extr
		     in extimatag! */
    extnotagnb =0; /* same for extimanotag */
    
    foreach (l, ei->line_lst) {
      foreach (ext, l->gr_lst) {
	/* we just want to test MMMTO */
	if (!ext->up && !ext->down) {
	  continue;
	}
	
	if (ext->next == NULL) {
	  extnotagnb++;
	} else {
	  exttagnb++;
	}
      } /* end of foreach (ext, l->gr_lst) loop */
    } /* end of foreach (l, ei->line_lst) loop */
    
    /* create the ExtImage extima */
    extimatag   = w2_ext_new(exttagnb,ei->lx,ei->ly,ei->scale);
    extimanotag = w2_ext_new(extnotagnb,ei->lx,ei->ly,ei->scale);
    if (!extimatag)
      return GenErrorMemoryAlloc(interp);
    if (!extimanotag)
      return GenErrorMemoryAlloc(interp);
    
    extremumtag   = extimatag->extr;
    extremumnotag = extimanotag->extr;
    
    /* now allocation is done we can fill the blanks ! */
    foreach (l, ei->line_lst) {
      foreach (ext, l->gr_lst) {
	if (!ext->up && !ext->down) {
	  continue;
	}
	
	/* test to know if ext is tagged */
	/* if so we fill fields mod arg and pos of extremum */
	if (ext->next == NULL) {
	  extremumnotag->mod = ext->mod;
	  extremumnotag->arg = ext->arg;
	  extremumnotag->pos = ext->pos;
	  extremumnotag++;
	} else {
	  extremumtag->mod = ext->mod;
	  extremumtag->arg = ext->arg;
	  extremumtag->pos = ext->pos;
	  extremumtag++;
	}
      } /* end of foreach (ext, l->gr_lst) loop */
    } /* end of foreach (l, ei->line_lst) loop */
    
    /* ajout de la structure ExtImage au dictionnaire adequat */
    ExtDicStore(extimaNametag,extimatag);
    ExtDicStore(extimaNamenotag,extimanotag);
    sprintf (interp->result, "%d", exttagnb);

  }
  
  return TCL_OK;
}




/*
 * Command name in xsmurf : eicounttag
 */
int
e_count_tag_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Edddd",
    NULL
  };
  
  char * help_msg =
  {
    ("Count how many tagged points there are in a rectangular box\n"
     "specified by 4 integers (as in ecut command) \n"
     "Take care using this after chain and eitagvc command !!!.\n"
     "\n"
     "Parameters:\n"
     "  ext image  - Ext image to treat (one of the skeleton)\n"
     "  4 integers - to locate the rectangular box \n" 
     "\n"
     "Options:\n"
     "\n"
     "Return value:\n"
     "  two values : nb of tagged point and nb_total of points on a\n"
     "  vertical chain.\n")
  };
  
  /* Command's parameters */
  ExtImage *ei;
  
  int      x1, x2, y1, y2, tmp, pos;
  int      x, y;

  /* Options's presence */

  /* Options's parameters */
 
  /* Other variables */

  Extremum *ext;
  Line     *l;
  int nb_tag = 0;
  int nb_tot = 0;

  /*int scale_number = 0;*/

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;

   if (arg_get (0, &ei, &x1, &y1, &x2, &y2) == TCL_ERROR)
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
  if ((x2 < 0) || (y2 < 0) || (x1 >= ei->lx) || (y1 >= ei->ly)) {
    sprintf (interp->result, "bad specified domain");
    return TCL_ERROR;
  }
  if (x1 < 0) {
    x1 = 0;
  }
  if (y1 < 0) {
    y1 = 0;
  }
  if (x2 >= ei->lx) {
    x2 = ei->lx-1;
  }
  if (y2 >= ei->ly) {
    y2 = ei->ly-1;
  }

  /* Treatement */

  nb_tot=0;
  nb_tag=0;

  foreach (l, ei->line_lst) {
    foreach (ext, l->gr_lst) {
      if (!ext->up && !ext->down) {
	continue;
      }
      pos = ext->pos;
      x = pos % ei->lx;
      y = pos / ei->lx;
      if ((x > x1) && (x < x2) && (y > y1) && (y < y2)) {
	/* on tient extremun sur une vc, mais est-il tagge? */
	nb_tot++;
	if (ext->next == NULL) {
	} else {
	  nb_tag++;
	  break;
	}
      }

    }
  }
  
  sprintf(interp->result,"%d %d", nb_tag, nb_tot);
  return TCL_OK;
}




/*
 */
int
e_save_skeleton_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "Esfdd",
    NULL
  };

  char * help_msg =
  {
    ("  Save the skeleton which contains a given ext image.\n"
     "\n"
     "Parameters:\n"
     "  ext image  - An ext image from the skeleton.\n"
     "  string     - Name of the file.\n"
     "  real       - First scale of the skeleton (amin).\n"
     "  integer    - Number of octaves (noct).\n"
     "  integer    - Number of voices (nvoice).\n"
     "\n"
     "Options:\n"
     "  None.\n"
     "\n"
     "Return value:\n"
     "  Name of the file.")
  };

  /* Command's parameters */
  ExtImage *ei;
  char     *fileName;
  real     aMin;
  int      nO;
  int      nV;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  FILE     *fid;
  Line     *l;
  Extremum *ext;
  int      lx;
  int      ly;
  int      lineSize = -1;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &ei, &fileName, &aMin, &nO, &nV) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  fid = fopen(fileName, "w");
  if (!fid) {
    return GenErrorAppend(interp,
			  "Couldn't open `", fileName, "' for writing.", NULL);
  }

  lx = ei->lx;
  ly = ei->ly;

  fprintf(fid, "Skeleton %dx%d %f %d %d\n", lx, ly, aMin, nO, nV);

  foreach (l, ei->line_lst) {
    foreach (ext, l->gr_lst) {
      if (!ext->up && !ext->down) {
	continue;
      }

      while (ext->down) {
	ext = ext->down;
      }

      lineSize = 1;
      while (ext->up) {
	ext = ext->up;
	lineSize++;
      }

      fwrite(&(lineSize), sizeof(int), 1, fid);

      while (ext->down) {
	ext = ext->down;
      }

      while (ext) {
	fwrite(&(ext->pos), sizeof(int),  1, fid);
	fwrite(&(ext->mod), sizeof(real), 1, fid);
	fwrite(&(ext->arg), sizeof(real), 1, fid);
	ext = ext->up;
      }
    }
  }

  fclose (fid);

  sprintf (interp->result, "%s", fileName);

  return TCL_OK;
}




/* 
 * pas termine !!
 */
int
e_load_skeleton_TclCmd_ (ClientData clientData,
			 Tcl_Interp *interp,
			 int        argc,
			 char       **argv)
{ 
  /* Command line definition */
  char * options[] =
  {
    "ss",
    NULL
  };

  char * help_msg =
  {
    ("  Load a skeleton.\n"
     "\n"
     "Parameters:\n"
     "  string - Name of the file.\n"
     "  string - Base name of the resulting ext images.\n"
     "\n"
     "Options:\n"
     "  None.\n"
     "\n"
     "Return value:\n"
     "  First scale, number of octaves and number of voices of the skeleton.")
  };

  /* Command's parameters */
  char *fileName;
  char *baseName;

  /* Options's presence */

  /* Options's parameters */

  /* Other variables */
  //real     aMin;
  //int      nO;
  //int      nV;
  FILE     *fid;
  //Line     *l;
  //Extremum *ext;
  //int      lx;
  //int      ly;
  //int      lineSize = -1;

  /* Command line analysis */
  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &fileName, &baseName) == TCL_ERROR)
    return TCL_ERROR;

  /* Parameters validity and initialisation */

  /* Treatement */

  fid = fopen(fileName, "r");
  if (!fid) {
    return GenErrorAppend(interp,
			  "couldn't open `", fileName, "' for reading.", NULL);
  }
  fclose (fid);

  /* A faire.... */

  /*  sprintf (interp->result, "%s", fileName);*/

  return TCL_OK;
}

