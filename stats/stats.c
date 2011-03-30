/*
 * stats.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: stats.c,v 1.15 1998/08/31 15:42:53 decoster Exp $
 */

#include "stats_int.h"
//#include "../compat/stdlib.h"
#include <stdlib.h>

/**************************************/
/* Compute the 'n' branches histogram */
/* of the signal Y's.                 */
/**************************************/
/*
 * in1    : the signal to make the histogram on 
 * nbox        : the number of branches of the histogram
 * xmin,xmax  : the yMin and yMax of the histogram (not used if min > max)
 * weight   : signal of weights (or NULL). It must have the same size as input 
 * flag_imaginary : histogramm of the imaginary part
 */

Signal*
stat_signal_histogram(Signal *in1, int nbox, real xmin, real xmax, Signal *weight, int flag_imaginary)

{
  Signal *result;
  int i,index;
  real theWeight; 
  int imin,imax;

  result = sig_new (REALY, 0, nbox-1);

  /*initialisation*/
  for (i = 0; i < result->n; i++){
    result->dataY[i] = 0.0;
  }

  if ((in1->type == REALXY) || (in1->type == REALY)) {
    if (xmin > xmax)  sig_get_extrema(in1, &xmin, &xmax, &imin, &imax);
    if (xmin == xmax) {
      xmin -= .5;
      xmax += .5;
    }
    
    for (i = 0; i < in1->n; i++) {
      index=(int)((in1->dataY[i]-xmin)*nbox/(xmax-xmin));
      if (index < 0 || index > nbox) continue;
      if (index == nbox) index = nbox-1;
      if (weight == NULL) theWeight = 1;
      else theWeight = weight->dataY[i];
      
      result->dataY[index] += theWeight;
    }
  }  

  else 
    if ((in1->type == FOUR_NR) || (in1->type == CPLX)) 
      /* ("pas encore implemente\n");*/
      ;
  
  
  result->x0 = xmin + (xmax-xmin)/(2*nbox);
  result->dx = (xmax-xmin)/nbox;

  return result;
  
}


/**************************************/
/* Compute the 'n' branches histogram */
/* of the image Y's.                 */
/**************************************/
/*
 * in1    : the image to make the histogram on 
 * nbox        : the number of branches of the histogram
 * xmin,xmax  : the yMin and yMax of the histogram (not used if min > max)
 * weight   : image of weights (or NULL). It must have the same size as input 
 * flag_imaginary : histogramm of the imaginary part
 * mask   : image to mask computation
 */


Signal*
stat_imimage_histogram(Image *in1, int nbox, real xmin, real xmax, Image *weight, int flag_imaginary, Image *mask)

{
  Signal *result;
  int i,index;
  real theWeight; 

  result = sig_new (REALY, 0, nbox-1);

  /*initialisation*/
  for (i = 0; i < result->n; i++){
    result->dataY[i] = 0.0;
  }
  

  if (in1->type == PHYSICAL) {
    if (xmin > xmax)  {
      im_get_extrema(in1, &xmin, &xmax);
      xmin=in1->min;
      xmax=in1->max;
    }
    if (xmin == xmax) {
      xmin -= .5;
      xmax += .5;
    }

    if (!mask) {
      for (i=0;i<in1->size;i++){
	index=(int)((in1->data[i]-xmin)*nbox/(xmax-xmin));
	if (index < 0 || index > nbox) continue;
	if (index == nbox) index = nbox-1;
	if (weight == NULL) theWeight = 1;
	else theWeight = weight->data[i];
	
	result->dataY[index] += theWeight;
      }
    } else {
      for (i=0;i<in1->size;i++){
        if (mask->data[i] <= 0.0) continue;
	
        index=(int)((in1->data[i]-xmin)*nbox/(xmax-xmin));
        if (index < 0 || index > nbox) continue;
        if (index == nbox) index--;
        if (weight == NULL) theWeight = 1;
        else theWeight = weight->data[i];
	
        result->dataY[index] += theWeight;
      } 
    }
  } else if (in1->type == FOURIER){
    /*("pas encore implemente\n");*/
    ;
  }
    
  result->x0 = xmin + (xmax-xmin)/(2*nbox);
  result->dx = (xmax-xmin)/nbox;
  return result;

}

/**************************************/
/* Compute the 'n' branches histogram */
/* of the image3D Y's.                */
/**************************************/
/*
 * in1       : the image3D to make the histogram on 
 * nbox      : the number of branches of the histogram
 * xmin,xmax : the yMin and yMax of the histogram (not used if min > max)
 */
/* created January, 30 th , 2003 */

Signal*
stat_imimage3D_histogram(Image3D *in1, int nbox, real xmin, real xmax)

{
  Signal *result;
  int i,index;
  real theWeight; 

  result = sig_new (REALY, 0, nbox-1);

  /*initialisation*/
  for (i = 0; i < result->n; i++){
    result->dataY[i] = 0.0;
  }
  

  if (xmin > xmax)  {
    im3D_get_extrema(in1, &xmin, &xmax);
    xmin=in1->min;
    xmax=in1->max;
  }
  if (xmin == xmax) {
    xmin -= .5;
    xmax += .5;
  }
  
  for (i=0;i<in1->size;i++){
    index=(int)((in1->data[i]-xmin)*nbox/(xmax-xmin));
    if (index < 0 || index > nbox) continue;
    if (index == nbox) index = nbox-1;
    theWeight = 1;
    
    result->dataY[index] += theWeight;
  }
  
  result->x0 = xmin + (xmax-xmin)/(2*nbox);
  result->dx = (xmax-xmin)/nbox;
  return result;

}


/**************************************/
/* Compute the 'n' branches histogram */
/* of the ExtImage Y's.               */
/**************************************/
/*
 * in1    : the extimage to make the histogram on 
 * nbox        : the number of branches of the histogram
 * xmin,xmax  : the yMin and yMax of the histogram (not used if min > max)
 * weight   : image of weights (or NULL). It must have the same size as input 
 * flag_imaginary : histogramm of the imaginary part
 */


Signal*
stat_extimage_histogram2(ExtImage *in1, int nbox, real xmin, real xmax, int flag_arg)

{
  Signal *result;
  int i,index;
  real theWeight; 

  result = sig_new (REALY, 0, nbox-1);

  /*initialisation*/
  for (i = 0; i < result->n; i++){
    result->dataY[i] = 0.0;
  }
  

  if (flag_arg == 0) {
    if (xmin > xmax)  {
      ExtImaMinMax(in1, &xmin, &xmax);
    }
    if (xmin == xmax) {
      xmin -= .5;
      xmax += .5;
    }

    for (i=0;i<in1->extrNb;i++){
      index=(int)((in1->extr[i].mod-xmin)*nbox/(xmax-xmin));
      if (index < 0 || index > nbox) continue;
      if (index == nbox) index = nbox-1;
      theWeight = 1;
      
      result->dataY[index] += theWeight;
    }
  }
  else {
    if (xmin > xmax)  {
      ExtImaMinMaxArg(in1, &xmin, &xmax);
    }
    if (xmin == xmax) {
      xmin -= .5;
      xmax += .5;
    }

    for (i=0;i<in1->extrNb;i++){
      index=(int)((in1->extr[i].arg-xmin)*nbox/(xmax-xmin));
      if (index < 0 || index > nbox) continue;
      if (index == nbox) index = nbox-1;
      theWeight = 1;
      
      result->dataY[index] += theWeight;
    }
  }
    
  result->x0 = xmin + (xmax-xmin)/(2*nbox);
  result->dx = (xmax-xmin)/nbox;
  return result;

}

/*
 * author : P. Kestener
 * increments histogram
 * direction : 0 -> X
 *             1 -> Y
 * mask : image to mask computation 
 */
Signal*
stat_imimage_increment_histogram(Image *in1, int nbox, real xmin, real xmax, int dist, int direction, Image *mask)

{
  Signal *result;
  int i,j, index, index1, index2;
  int lx,ly;
  real theWeight = 1.0; 
  
  result = sig_new (REALY, 0, nbox-1);
  
  lx = in1->lx;
  ly = in1->ly;

  /*initialisation*/
  for (i = 0; i < result->n; i++){
    result->dataY[i] = 0.0;
  }
  
  if (xmin > xmax) {

    im_get_extrema_increment (in1, &xmin, &xmax, direction, dist, mask);

  }
  if (xmin == xmax) {
    xmin -= .5;
    xmax += .5;
  }

  if (!mask) {
    if (direction) {
      
      for (i=0;i<lx;i++){
	for (j=0;j<ly-dist;j++){
	  
	  index1 = i + j*lx;
	  index2 = i + (j+dist)*lx;
	  
	  index=(int)((in1->data[index1] - in1->data[index2] - xmin)*nbox/(xmax-xmin));
	  if (index < 0 || index > nbox) continue;
	  if (index == nbox) index = nbox-1;
	  
	  result->dataY[index] += theWeight;
	  
	}
      }
      
    } else {
      
      for (i=0;i<lx-dist;i++){
	for (j=0;j<ly;j++){
	  
	  index1 = i + j*lx;
	  index2 = i+dist + j*lx;
	  
	  index=(int)((in1->data[index1] - in1->data[index2] - xmin)*nbox/(xmax-xmin));
	  if (index < 0 || index > nbox) continue;
	  if (index == nbox) index = nbox-1;
	  
	  result->dataY[index] += theWeight;
	  
	}
      }    

    }

  } else {

    if (direction) {
      
      for (i=0;i<lx;i++){
	for (j=0;j<ly-dist;j++){
	  
	  index1 = i + j*lx;
	  index2 = i + (j+dist)*lx;
	  
	  if (mask->data[index1] <= 0.0 || mask->data[index2] <= 0.0)
	    continue;

	  index=(int)((in1->data[index1] - in1->data[index2] - xmin)*nbox/(xmax-xmin));
	  if (index < 0 || index > nbox) continue;
	  if (index == nbox) index = nbox-1;
	  
	  result->dataY[index] += theWeight;
	  
	}
      }
      
    } else {
      
      for (i=0;i<lx-dist;i++){
	for (j=0;j<ly;j++){
	  
	  index1 = i + j*lx;
	  index2 = i+dist + j*lx;
	  
	  if (mask->data[index1] <= 0.0 || mask->data[index2] <= 0.0)
	    continue;
	  
	  index=(int)((in1->data[index1] - in1->data[index2] - xmin)*nbox/(xmax-xmin));
	  if (index < 0 || index > nbox) continue;
	  if (index == nbox) index = nbox-1;
	  
	  result->dataY[index] += theWeight;
	  
	}
      }    
      
    }  

  }
  
  
    
  result->x0 = xmin + (xmax-xmin)/(2*nbox);
  result->dx = (xmax-xmin)/nbox;
  return result;

}

/**************************************/
/* Compute the 'n' branches histogram */
/* of the image Y's with an extimage  */
/* mask                               */
/**************************************/
/*
 * in1        : the image to make the histogram on
 * mask       : extimage mask
 * nbox       : the number of branches of the histogram
 * xmin,xmax  : the yMin and yMax of the histogram (not used if min > max)
 */


Signal*
stat_imimage_masklines_histogram(Image *in1, ExtImage *mask, int nbox, real xmin, real xmax)

{
  Signal *result;
  int i,j,index;
  real theWeight; 

  result = sig_new (REALY, 0, nbox-1);

  /*initialisation*/
  for (i = 0; i < result->n; i++){
    result->dataY[i] = 0.0;
  }
  

  if (in1->type == PHYSICAL) {
    if (xmin > xmax)  {
      im_get_extrema(in1, &xmin, &xmax);
      xmin=in1->min;
      xmax=in1->max;
    }
    if (xmin == xmax) {
      xmin -= .5;
      xmax += .5;
    }

    for (j=0;j<mask->extrNb;j++){

      /* get extremum position index */
      i = mask->extr[j].pos;

      /* compute histogram index */
      index=(int)((in1->data[i]-xmin)*nbox/(xmax-xmin));
      if (index < 0 || index > nbox) continue;
      if (index == nbox) index = nbox-1;
      theWeight = 1;
      
      /* increment histogram value at index */
      result->dataY[index] += theWeight;
    }
  }
  else if (in1->type == FOURIER){
    /*("pas encore implemente\n");*/
    ;
  }
    
  result->x0 = xmin + (xmax-xmin)/(2*nbox);
  result->dx = (xmax-xmin)/nbox;
  return result;

}


/**************************************/
/* Compute the 'n' branches histogram */
/* of the extrema of a line.          */
/**************************************/
/*
 * line    : the line to make the histogram on 
 * nbox        : the number of branches of the histogram
 * xmin,xmax  : the yMin and yMax of the histogram (min must be < max)
 * flag_arg : histogramm of the argument.
 */


Signal *
stat_line_histogram (Line   *line,
		     int    nbox,
		     real   xmin,
		     real   xmax,
		     int    flag_arg,
		     int    max_flag,
		     Signal *result)
{
  int      i,index;
  real     the_weight; 
  Extremum *ext;
  List     *ext_lst_to_treat;

  assert (xmin < xmax);
  assert (result);
  assert (result->type == REALY);

  for (i = 0; i < result->n; i++)
    result->dataY[i] = 0.0;

  ext_lst_to_treat = line->ext_lst;

  if (flag_arg == MOD_STAT)
    {
      foreach (ext, ext_lst_to_treat)
	{
	  if (max_flag == LINE_MAX_STAT
	      || (max_flag == VERT_CHAINED_MAX_STAT
		  && (ext->up || ext ->down)))
	    {
	      index = (int)((ext->mod - xmin)*nbox/(xmax - xmin));
	      if (index < 0 || index > nbox) continue;
	      if (index == nbox) index = nbox-1;
	      the_weight = 1;
      
	      result->dataY[index] += the_weight;
	    }
	}
    }
  else
    {
      foreach (ext, ext_lst_to_treat)
	{
	  if (max_flag == LINE_MAX_STAT
	      || (max_flag == VERT_CHAINED_MAX_STAT
		  && (ext->up || ext ->down)))
	    {
	      index = (int)((ext->arg - xmin)*nbox/(xmax - xmin));
	      if (index < 0 || index > nbox) continue;
	      if (index == nbox) index = nbox-1;
	      the_weight = 1;
      
	      result->dataY[index] += the_weight;
	    }
	}
    }
    
  result->x0 = xmin + (xmax - xmin)/(2*nbox);
  result->dx = (xmax - xmin)/nbox;

  return result;
}

/**************************************/
/* Compute the 'n' branches histogram */
/* of the ExtImage Y's.               */
/**************************************/
/*
 * in1    : the extimage to make the histogram on 
 * nbox        : the number of branches of the histogram
 * xmin,xmax  : the yMin and yMax of the histogram (not used if min > max)
 * weight   : image of weights (or NULL). It must have the same size as input 
 * flag_imaginary : histogramm of the imaginary part
 */


Signal *
stat_extimage_histogram3(ExtImage *ext_image,
			int      nbox,
			real     xmin,
			real     xmax,
			int      flag_arg,
			int     max_flag)
{
  Signal *result;
  Signal *line_result;
  int i;
  Line *line;

  result = sig_new (REALY, 0, nbox-1);
  line_result = sig_new (REALY, 0, nbox-1);

  for (i = 0; i < result->n; i++)
    result->dataY[i] = 0.0;

  if (flag_arg == 0)
    {
      if (xmin > xmax)
	ExtImaMinMax(ext_image, &xmin, &xmax);
    if (xmin == xmax)
      {
	xmin -= .5;
	xmax += .5;
      }
    }
  else
    {
      if (xmin > xmax)
	ExtImaMinMaxArg(ext_image, &xmin, &xmax);
      if (xmin == xmax)
	{
	  xmin -= .5;
	  xmax += .5;
	}
    }

  foreach (line, ext_image->line_lst)
    {
      stat_line_histogram (line, nbox, xmin, xmax, flag_arg, max_flag, line_result);
      for (i = 0; i < nbox; i++)
	result->dataY[i] += line_result->dataY[i];
    }
    
  result->x0 = xmin + (xmax - xmin)/(2*nbox);
  result->dx = (xmax - xmin)/nbox;

  return result;
}


/* this function is called by StatExtHistoCmd_ defined in
   interpreter/stats_cmds.c (ehisto in xsmurf)             */

Signal *
stat_extimage_histogram (ExtImage *ext_image,
			 int      nbox,
			 real     xmin,
			 real     xmax,
			 int      flag_arg,
			 int      max_flag,
			 Image   *mask)
{
  /* flag_arg can be MOD_STAT (histogram of modulus) or
     ARG_STAT (histo of argument) or
     MX_STAT or MY_STAT
     see file stats/stats.h */
  
  /* max_flag can be   LINE_MAX_STAT or
     VERT_CHAINED_MAX_STAT */
  
  Signal   *result;
  int      i, index;
  Extremum *ext;
  real     the_weight;

  result = sig_new (REALY, 0, nbox-1);

  for (i = 0; i < result->n; i++)
    result->dataY[i] = 0.0;

  if (flag_arg == MOD_STAT)
    {
      if (xmin > xmax)
	ExtImaMinMax(ext_image, &xmin, &xmax);
    if (xmin == xmax)
      {
	xmin -= .5;
	xmax += .5;
      }
    }
  else if (flag_arg == ARG_STAT)
    {
      if (xmin > xmax)
	/* ExtImaMinMaxArg is defined in wt2d/wt2d.c !! */
	ExtImaMinMaxArg(ext_image, &xmin, &xmax);
      if (xmin == xmax)
	{
	  xmin -= .5;
	  xmax += .5;
	}
    }
  else if (flag_arg == MX_STAT)
    {
      if (xmin > xmax)
	/* ExtImaMinMaxMx is defined in wt2d/wt2d.c !! */
	ExtImaMinMaxMx(ext_image, &xmin, &xmax);
      if (xmin == xmax)
	{
	  xmin -= .5;
	  xmax += .5;
	}
    }
  else if (flag_arg == MY_STAT)
    {
      if (xmin > xmax)
	/* ExtImaMinMaxMy is defined in wt2d/wt2d.c !! */
	ExtImaMinMaxMy(ext_image, &xmin, &xmax);
      if (xmin == xmax)
	{
	  xmin -= .5;
	  xmax += .5;
	}
    }

  for (i = 0; i < ext_image->extrNb; i++)
    {
      ext = &ext_image->extr[i];
      if (flag_arg == MOD_STAT) {
	if (!mask) {

	  if (max_flag == LINE_MAX_STAT
	      || (max_flag == VERT_CHAINED_MAX_STAT
		  && (ext->up || ext ->down))) {
	    index = (int)((ext->mod - xmin)*nbox/(xmax - xmin));
	    if (index < 0 || index > nbox) continue;
	    if (index == nbox) index = nbox-1;
	    the_weight = 1;
	    
	    result->dataY[index] += the_weight;
	  }

	} else {

	 if (mask->data[ext->pos] <= 0.0) continue;

	 if (max_flag == LINE_MAX_STAT
	     || (max_flag == VERT_CHAINED_MAX_STAT
		 && (ext->up || ext ->down))) {
	   index = (int)((ext->mod - xmin)*nbox/(xmax - xmin));
	   if (index < 0 || index > nbox) continue;
	   if (index == nbox) index = nbox-1;
	   the_weight = 1;
	   
	   result->dataY[index] += the_weight;
	 }

	}
	
      } else if (flag_arg == ARG_STAT) /* on fait la stat sur l'argument de la TO */ {
	if (max_flag == LINE_MAX_STAT
	    || (max_flag == VERT_CHAINED_MAX_STAT
		&& (ext->up || ext ->down)))
	  {
	    index = (int)((ext->arg - xmin)*nbox/(xmax - xmin));
	    if (index < 0 || index > nbox) continue;
	    if (index == nbox) index = nbox-1;
	    the_weight = 1;
	    
	    result->dataY[index] += the_weight;
	  }
      } else if (flag_arg == MX_STAT) /* on fait la stat sur module(TO)*cos(argument) */ {
	if (max_flag == LINE_MAX_STAT
	    || (max_flag == VERT_CHAINED_MAX_STAT
		&& (ext->up || ext ->down)))
	  {
	    index = (int)((ext->mod * cos(ext->arg) - xmin)*nbox/(xmax - xmin));
	    if (index < 0 || index > nbox) continue;
	    if (index == nbox) index = nbox-1;
	    the_weight = 1;
	    
	    result->dataY[index] += the_weight;
	  }
      } else if (flag_arg == MY_STAT) /* on fait la stat sur module(TO)*sin(argument) */ {
	if (max_flag == LINE_MAX_STAT
	    || (max_flag == VERT_CHAINED_MAX_STAT
		&& (ext->up || ext ->down)))
	  {
	    index = (int)((ext->mod * sin(ext->arg) - xmin)*nbox/(xmax - xmin));
	    if (index < 0 || index > nbox) continue;
	    if (index == nbox) index = nbox-1;
	    the_weight = 1;
	    
	    result->dataY[index] += the_weight;
	  }
      }
    }
  /*  foreach (line, ext_image->line_lst)
    {
      stat_line_histogram (line, nbox, xmin, xmax, flag_arg, max_flag, line_result);
      for (i = 0; i < nbox; i++)
	result->dataY[i] += line_result->dataY[i];
    }*/
    
  result->x0 = xmin + (xmax - xmin)/(2*nbox);
  result->dx = (xmax - xmin)/nbox;
  
  return result;
}

/* this function is called by StatExt3DsmallHistoCmd_ defined in
   interpreter/stats_cmds.c (ehisto3Dsmall in xsmurf)             */
// new command : october 3rd 2002

Signal *
stat_extimage3Dsmall_histogram (ExtImage3Dsmall *ext_image,
				int      nbox,
				real     xmin,
				real     xmax)
{
  Signal          *result;
  int              i, index;
  real             the_weight;
  Extremum3Dsmall *ext;

  result = sig_new (REALY, 0, nbox-1);

  for (i = 0; i < result->n; i++)
    result->dataY[i] = 0.0;
  
  if (xmin > xmax)
    ExtIma3DsmallMinMax(ext_image, &xmin, &xmax);
  if (xmin == xmax)
    {
      xmin -= .5;
      xmax += .5;
    }
  
  for (i = 0; i < ext_image->extrNb; i++)
    {
      ext = &ext_image->extr[i];
      index = (int)((ext->mod - xmin)*nbox/(xmax - xmin));
      if (index < 0 || index > nbox) continue;
      if (index == nbox) index = nbox-1;
      the_weight = 1;
      result->dataY[index] += the_weight; 
    }
  
  result->x0 = xmin + (xmax - xmin)/(2*nbox);
  result->dx = (xmax - xmin)/nbox;
  
  return result;
}

/* this function is called by Stat3DExtHistoCmd_ defined in
   interpreter/stats_cmds.c (ehisto3D in xsmurf)             */
//new command : september 16th 2002 */

Signal *
stat_extimage3D_histogram (Tcl_Interp * interp,
			   char    * extImageFilename,
			   int      nbox,
			   int      flag_arg,
			   int      max_flag)
{
  /* flag_arg can be MOD_STAT (histogram of modulus) or
     ARG_STAT (histo of argument) or
     MX_STAT or MY_STAT
     see file stats/stats.h */
  
  /* max_flag can be   LINE_MAX_STAT or
     VERT_CHAINED_MAX_STAT */
  
  Signal   *result;
  int      i, index;
  Extremum *ext;
  real     the_weight;
  FILE     * fileIn;
  char       tempBuffer[100], saveFormat[100], type[100];
  int        lx, ly, lz, size, realSize, intSize;
  real       scale;
  long int   pos_file;
  float      tmp_mod;
  int        tmp_pos;
  real       xmin, xmax;

  result = sig_new (REALY, 0, nbox-1);

  /* init to zero the signal "result" */
  for (i = 0; i < result->n; i++)
    result->dataY[i] = 0.0;

  /* open file */
  if (!(fileIn = fopen(extImageFilename, "r"))) {
    GenErrorAppend(interp, "Couldn't open `", extImageFilename,
		   "' for reading.", NULL);
    return (Signal *) NULL;
  }
  

  fgets(tempBuffer, 100, fileIn);
  sscanf(tempBuffer, "%s %s %dx%dx%d %d %f (%d byte reals, %d", 
	 saveFormat, type, &lx, &ly, &lz, &size, &scale, &realSize, &intSize);
  
  pos_file = ftell(fileIn);
  
  /* compute xmin and xmax */
  fread(&tmp_pos, sizeof(int),   1, fileIn);
  fread(&tmp_mod, sizeof(float), 1, fileIn);
  fseek(fileIn, -1*(sizeof(int)+sizeof(float)), SEEK_CUR);
  xmin = tmp_mod;
  xmax = tmp_mod;
  for (i=0;i<size;i++) {
    fread(&tmp_pos, sizeof(int),   1, fileIn);
    fread(&tmp_mod, sizeof(float), 1, fileIn);
    if (tmp_mod > xmax)
      xmax = tmp_mod;
    else
      if (tmp_mod < xmin)
	xmin = tmp_mod;
  }

  if (flag_arg == MOD_STAT) {
  } else if (flag_arg == ARG_STAT) {
    if (xmin == xmax)
      {
	xmin -= 3.15;
	xmax += 3.15;
      }
  } else if (flag_arg == MX_STAT) {
  } else if (flag_arg == MY_STAT) {
  }

  fseek(fileIn, pos_file, SEEK_SET);
  for (i = 0; i < size; i++) {
    fread(&tmp_pos, sizeof(int),   1, fileIn);
    fread(&tmp_mod, sizeof(float), 1, fileIn);

    if (flag_arg == MOD_STAT) {
      if (max_flag == LINE_MAX_STAT) {
	index = (int)((tmp_mod - xmin)*nbox/(xmax - xmin));
	if (index < 0 || index > nbox) continue;
	if (index == nbox) index = nbox-1;
	the_weight = 1;
	
	result->dataY[index] += the_weight;
      }
    } else if (flag_arg == ARG_STAT) {
    } else if (flag_arg == MX_STAT) {
    } else if (flag_arg == MY_STAT) {
    }
  }
  /*  foreach (line, ext_image->line_lst)
      {
      stat_line_histogram (line, nbox, xmin, xmax, flag_arg, max_flag, line_result);
      for (i = 0; i < nbox; i++)
      result->dataY[i] += line_result->dataY[i];
      }*/
  
  result->x0 = xmin + (xmax - xmin)/(2*nbox);
  result->dx = (xmax - xmin)/nbox;
  
  return result;
}


/*
 * Create an image that represents th histogram of the gradient in a plane
 * for all the extrema from the line.
 */
Image *
stat_line_gradient_histogram (Line   *line,
			      int    nbox,
			      real   xmin,
			      real   xmax,
			      Image  *result,
			      int    max_flag)
{
  int      i;
  real     the_weight = 1;
  Extremum *ext;
  int      x, y;
  real     ymin, ymax;

  ymax = xmax;
  ymin = xmin;

  assert (xmin < xmax);
  assert (result);
  assert (result->type == PHYSICAL);
  assert (result->lx == nbox);
  assert (result->ly == nbox);

  for (i = 0; i < result->size; i++)
    result->data[i] = 0.0;
  
  foreach (ext, line->ext_lst)
    {
      if (max_flag == LINE_MAX_STAT
	  || (max_flag == VERT_CHAINED_MAX_STAT
	      && (ext->up || ext ->down)))
	{
	  if (ext->mod > 0.0*xmax && ext->arg != NO_ARG)
	    {
	      x = (int) ((cos (ext->arg) * ext->mod - xmin)*nbox/(xmax - xmin));
	      if (x < 0 || x > nbox) continue;
	      if (x == nbox) x = nbox-1;
	      y = (int) ((sin (ext->arg) * ext->mod - ymin)*nbox/(ymax - ymin));
	      if (y < 0 || y > nbox) continue;
	      if (y == nbox) y = nbox-1;
	      the_weight = 1;
      
	      result->data [x + y*result->lx] += the_weight;
	    }
	}
    }
    
  return result;
}

/*
 */
Image *
stat_extimage_gradient_histogram3(ExtImage *ext_image,
				 int    nbox,
				 real   *xmin,
				 real   *xmax,
				 int    max_flag)
{
  Image *result;
  Image *line_result;
  int   i;
  Line  *line;

  result = im_new (nbox, nbox, nbox*nbox, PHYSICAL);
  line_result = im_new (nbox, nbox, nbox*nbox, PHYSICAL);

  for (i = 0; i < result->size; i++)
    result->data[i] = 0.0;

  if (*xmin > *xmax)
    {
      ExtImaMinMax(ext_image, xmin, xmax);
      *xmax = *xmax*1.1;
      *xmin = - *xmax;
    }
    
  if (*xmin == *xmax)
    {
      *xmin -= .5;
      *xmax += .5;
    }

  foreach (line, ext_image->line_lst)
    {
      stat_line_gradient_histogram (line, nbox, *xmin, *xmax,
				    line_result, max_flag);
      for (i = 0; i < result->size; i++)
	result->data[i] += line_result->data[i];
    }
    
  return result;
}

/* 
 * this function is called by StatExtHistoCmd_ defined in
 * interpreter/stats_cmds.c (ehisto with option -grad in 
 * xsmurf)
 */

Image *
stat_extimage_gradient_histogram (ExtImage *ext_image,
				  int    nbox,
				  real   *xmin,
				  real   *xmax,
				  int    max_flag)
{
  Image    *result;
  int      i, x, y;
  Extremum *ext;
  real     the_weight;
  real     ymin, ymax;

  result = im_new (nbox, nbox, nbox*nbox, PHYSICAL);

  for (i = 0; i < result->size; i++)
    result->data[i] = 0.0;

  if (*xmin > *xmax) /* we re-calculate xmin and xmax from ExtImaMinMax */
    {
      ExtImaMinMax(ext_image, xmin, xmax);
      *xmax = *xmax*1.1;
      *xmin = - *xmax;
    }
    
  if (*xmin == *xmax)
    {
      *xmin -= .5;
      *xmax += .5;
    }

  ymax = *xmax;
  ymin = *xmin;

  for (i = 0; i < ext_image->extrNb; i++)
    {
      ext = &ext_image->extr[i];
      if (max_flag == LINE_MAX_STAT
	  || (max_flag == VERT_CHAINED_MAX_STAT
	      && (ext->up || ext ->down)))
	{
	  if (ext->mod > 0.0**xmax && ext->arg != NO_ARG)
	    {
	      x = (int) ((cos (ext->arg) * ext->mod - *xmin)*nbox/(*xmax - *xmin));
	      if (x < 0 || x > nbox) continue;
	      if (x == nbox) x = nbox-1;
	      y = (int) ((sin (ext->arg) * ext->mod - ymin)*nbox/(ymax - ymin));
	      if (y < 0 || y > nbox) continue;
	      if (y == nbox) y = nbox-1;
	      the_weight = 1;
      
	      result->data [x + y*result->lx] += the_weight;
	    }
	}
    }

  return result;
}



/*
 * Create an image that represents the histogram of the gradient in a plane
 * for an image.
 */
Image *
stat_imimage_gradient_histogram(Image *imagemod,
				Image *imagearg,
				int    nbox,
				real   xmin,
				real   xmax)
{
  Image *result;
  int   i;
  int x,y;
  real     the_weight = 1;
  real     ymin, ymax;

  result = im_new (nbox, nbox, nbox*nbox, PHYSICAL);

  for (i = 0; i < result->size; i++)
    result->data[i] = 0.0;

  if (xmin > xmax)
    {
      im_get_extrema(imagemod, &xmin, &xmax);
      xmax = xmax*1.1;
      xmin = - xmax;
    }
    
  if (xmin == xmax)
    {
      xmin -= .5;
      xmax += .5;
    }

  ymax = xmax;
  ymin = xmin;


  for (i=0;i < imagemod->size;i++)
    {
      x = (int) ((cos (imagearg->data[i]) * imagemod->data[i] - xmin)*nbox/(xmax - xmin));
      if (x < 0 || x > nbox) continue;
      if (x == nbox) x = nbox-1;
      y = (int) ((sin (imagearg->data[i]) * imagemod->data[i] - ymin)*nbox/(ymax - ymin));
      if (y < 0 || y > nbox) continue;
      if (y == nbox) y = nbox-1;
      the_weight = 1;
      
      result->data [x + y*result->lx] += the_weight;
    }
  
  return result;

}

/*
 * Create an image that represents the 2D histogram of the 2 input images
 */
Image *
stat_imimage_histogram2D(Image *image1, real xmin, real xmax, int nboxx,
			 Image *image2, real ymin, real ymax, int nboxy) {

  Image *result;
  int i;
  int x,y;

  result = im_new (nboxx, nboxy, nboxx*nboxy, PHYSICAL);

  for (i = 0; i < result->size; i++)
    result->data[i] = 0.0;

  if (xmin > xmax) {
    im_get_extrema(image1, &xmin, &xmax);
    xmax = xmax*1.1;
    xmin = - xmax;
  }
  if (ymin > ymax) {
    im_get_extrema(image2, &ymin, &ymax);
    ymax = ymax*1.1;
    ymin = - ymax;
  }
  
    
  if (xmin == xmax) {
    xmin -= .5;
    xmax += .5;
  }
  if (ymin == ymax) {
    ymin -= .5;
    ymax += .5;
  }


  for (i=0; i < image1->size;i++) {
    x = (int) ( (image1->data[i] - xmin)*nboxx/(xmax - xmin) );
    if (x < 0 || x > nboxx) continue;
    if (x == nboxx) x = nboxx-1;
    y = (int) ( (image2->data[i] - ymin)*nboxy/(ymax - ymin) );
    if (y < 0 || y > nboxy) continue;
    if (y == nboxy) y = nboxy-1;
    
    result->data [x + y*result->lx] += 1.0;
  }
  
  return result;

}

/*
 * Create an image that represents the 2D histogram associated with the
 * two 3D images.
 */
Image *
stat_imimage3D_histogram2D(Image3D *image1,
			   Image3D *image2,
			   int  nboxx, int nboxy,
			   real xmin, real xmax,
			   real ymin, real ymax) {
  Image *result;
  int   i;
  int x,y;
  real     the_weight = 1.0;

  result = im_new (nboxx, nboxy, nboxx*nboxy, PHYSICAL);

  for (i = 0; i < result->size; i++)
    result->data[i] = 0.0;

  if (xmin > xmax) {
      im3D_get_extrema(image1, &xmin, &xmax);
      xmax = xmax*1.1;
      xmin = - xmax;
  }
  if (ymin > ymax) {
      im3D_get_extrema(image2, &ymin, &ymax);
      ymax = ymax*1.1;
      ymin = - ymax;
  }
    
  if (xmin == xmax) {
    xmin -= .5;
    xmax += .5;
  }    
  if (ymin == ymax) {
    ymin -= .5;
    ymax += .5;
  }


  for (i=0;i < image1->size;i++)
    {
      x = (int) ( (image1->data[i] - xmin)*nboxx/(xmax - xmin) );
      if (x < 0 || x > nboxx) continue;
      if (x == nboxx) x = nboxx-1;
      y = (int) ( (image2->data[i] - ymin)*nboxy/(ymax - ymin) );
      if (y < 0 || y > nboxy) continue;
      if (y == nboxy) y = nboxy-1;
      the_weight = 1.0;
      
      result->data [x + y*result->lx] += the_weight;
    }
  
  return result;
  
}

/*
 * Create an image that represents the histogram of the vector given by
 * two 3D images, histogram is constrained by an extimage3Dsmall which gives
 * the location of points used to do histogram.
 */
Image *
stat_imimage3D_gradient_histogram_mask(Image3D *image1,
				       Image3D *image2,
				       ExtImage3Dsmall *extima_mask,
				       int    nbox,
				       real   xmin,
				       real   xmax)
{
  Image *result;
  int   i;
  int x,y;
  int lx,ly,lz;
  real     the_weight = 1;
  real     ymin, ymax;
  float   *mask;

  result = im_new (nbox, nbox, nbox*nbox, PHYSICAL);

  for (i = 0; i < result->size; i++)
    result->data[i] = 0.0;

  if (xmin > xmax)
    {
      im3D_get_extrema(image1, &xmin, &xmax);
      xmax = xmax*1.1;
      xmin = - xmax;
    }
    
  if (xmin == xmax)
    {
      xmin -= .5;
      xmax += .5;
    }

  ymax = xmax;
  ymin = xmin;

  
  lx = ly = lz = image1->lx;
  mask = (float *) malloc(sizeof(float)*lx*ly*lz);
  for (i=0; i<lx*ly*lz;i++)
    mask[i]=0.0;
  for (i=0; i<extima_mask->extrNb;i++)
    mask[extima_mask->extr[i].pos]=1.0;
  
  
  
  for (i=0;i < image1->size;i++)
    {
      if (mask[i]>0.0) {
	x = (int) ((image1->data[i] - xmin)*nbox/(xmax - xmin));
	if (x < 0 || x > nbox) continue;
	if (x == nbox) x = nbox-1;
	y = (int) ((image2->data[i] - ymin)*nbox/(ymax - ymin));
	if (y < 0 || y > nbox) continue;
	if (y == nbox) y = nbox-1;
	the_weight = 1;
	
	result->data [x + y*result->lx] += the_weight;
      }
    }
  
  free(mask);
  
  return result;
  
}

/*
 * This function compute the histogram of an array.
 */
real *
stat_array_histo(real *data,           /* Array to compute. */
		 int  size,            /* Size of the array. */
		 real min,             /* Minimum value (of array) to */
		                       /* compute. */
		 real max,             /* Maximum value (of array) to */
		                       /* compute. We must have max > min. */
		 real *histo,          /* The array for the result. */
		 int  h_size,          /* Size of this array. */
		 real *x0,             /* Value for the abciss of histo[0]. */
		 real *dx,             /* Abciss step between to successive */
		                       /* points of histo */
		 /*double (* fct_ptr)(),*/ /* Function to apply to each value of */
		                       /* array before insert it in histo. */
		 void *fct_ptr,
		 real *weight)         /* Array with the same size than histo */
                                       /* that gives a weight for each value. */
{
  int  i;
  int  index;
  real value;

  assert(data);
  assert(size >= 0);
  assert(min < max);
  assert(histo);
  assert(h_size > 0);

  /*return h_size;*/

  for (i = 0; i < h_size; i++) {
    histo[i] = 0.0;
  }
    
  for (i = 0; i < size; i++) {
    if (fct_ptr) {
      /*value = fct_ptr(data[i]);*/
      value = evaluator_evaluate_x(fct_ptr,data[i]);
    } else {
      value = data[i];
    }
    index = (int)((value - min)*h_size/(max - min));
    if (index < 0 || index > h_size) {
      continue;
    }
    /* I don't understand the following 2 lines... */
    if (index == h_size) {
      index = h_size-1;
    }
    if (weight) {
      histo[index] += weight[i];
    } else {
      histo[index]++;
    }
  }
  
  *x0 = min + (max - min)/(2*h_size);
  *dx = (max - min)/h_size;

  assert(dx > 0);

  return histo;
}


/*
 * This function compute the histogram of an array with a rectangular mask.
 * it takes four more arguments : 
 * - two integer for the coordinates of upper-left corner
 * - two integer for width and height of rectangle
 */
real *
stat_array_histo_mask(real *data,           /* Array to compute. */
		      int  size,            /* Size of the array. */
		      real min,             /* Minimum value (of array) to */
		                            /* compute. */
		      real max,             /* Maximum value (of array) to */
		                            /* compute. We must have max > min. */
		      real *histo,          /* The array for the result. */
		      int  h_size,          /* Size of this array. */
		      real *x0,             /* Value for the abciss of histo[0]. */
		      real *dx,             /* Abciss step between to successive */
		                            /* points of histo */
		      /*double (* fct_ptr)(),*/ /* Function to apply to each value of */
		                            /* array before insert it in histo. */
		      void *fct_ptr,
		      real *weight,         /* Array with the same size than histo */
		                            /* that gives a weight for each value. */
		      int image_lx,         /* width of image */
		      int posX,
		      int posY,
		      int width,
		      int height)           /* position of the mask */
  /* you must check validity of posX, posY, width and height before calling this routine */
{
  int  i, j;
  int i_x, i_y;
  int  index;
  real value;

  assert(data);
  assert(size >= 0);
  assert(min < max);
  assert(histo);
  assert(h_size > 0);
  
  /*if (h_size > 1000) {
    return 5;
  }*/
  
  for (j = 0; j < h_size; j++) {
    histo[j] = 0.0;
  }

  /*return h_size;*/    
  for (i = 0; i < size; i++) {
    i_y = i / image_lx;
    i_x = i % image_lx;

    if (i_x >= posX && i_x < posX+width && i_y >= posY && i_y < posY+height) {
	value = (float) (2 * max);
    } else {
      /*return "tamere";*/
      if (fct_ptr) {
	/*value = fct_ptr(data[i]);*/
	value = evaluator_evaluate_x(fct_ptr,data[i]);
      } else {
	value = data[i];
      }
    }
    index = (int)((value - min)*h_size/(max - min));
    if (index < 0 || index > h_size) {
      continue;
    }
    /* I don't understand the following 2 lines... */
    if (index == h_size) {
      index = h_size-1;
    }
    if (weight) {
      histo[index] += weight[i];
    } else {
      histo[index]++;
    }
  }
  
  *x0 = min + (max - min)/(2*h_size);
  *dx = (max - min)/h_size;

  assert(dx > 0);

  return histo;
}



