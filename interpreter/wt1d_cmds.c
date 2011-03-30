/*
 * wt1d_cmds.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: wt1d_cmds.c,v 1.11 1998/11/24 15:06:56 decoster Exp $
 */

#include <tcl.h>
#include "../wt1d/wt1d.h"
#include "../signal/signal.h"
#include "arguments.h"
#include "hash_tables.h"
#include <math.h>
#include <stdlib.h>

/* a revoir... */
/*#include <defunc.h>*/
#include <matheval.h>

/*void store_signal_in_dictionary (char   *name, Signal *signal_ptr);*/
void store_wavelet_in_dictionary (char   *name, Wavelet *wavelet_ptr);

static Wavelet *current_wavelet;


/*
 */
int
create_wavelet_TclCmd_ (ClientData clientData,
			Tcl_Interp *interp,
			int        argc,
			char       **argv)
{
  char * options[] = { "ssff[f][f]",
		       "-rc", "[ssff]",
		       "-cr", "s[sff]",
		       "-cc", "s[ssff]",
		       "-rr", "[sff]",
		       NULL };

  char * help_msg =
  {(" Set the parameters of the current wavelet : name, f(x), minimum\n"
    "of x, maximum of x, ...\n"
    "  -rr The wavelet has a real direct form and a real fourier form\n"
    "  -rc The wavelet has a real direct form and a complex fourier form\n"
    "(the default).\n"
    "  -cr The wavelet has a complex direct form and a real fourier form.\n"
    "  -cc The wavelet has a complex direct form and a complex fourier form.")};

  int is_rr;
  int is_rc;
  int is_cr;
  int is_cc;

  char   *wavelet_name;
  char   *d_r_expr = NULL;
  char   *d_i_expr = NULL;
  char   *f_r_expr = NULL;
  char   *f_i_expr = NULL;
  /*double (*d_r_fct_ptr)() = NULL;
  double (*d_i_fct_ptr)() = NULL;
  double (*f_r_fct_ptr)() = NULL;
  double (*f_i_fct_ptr)() = NULL;*/
  void *d_r_fct_ptr, *d_i_fct_ptr, *f_r_fct_ptr, *f_i_fct_ptr;
  real   d_x_min;
  real   d_x_max;
  real   f_x_min = 0.0;
  real   f_x_max = 0.0;
  real   time_scale_mult = 1.0;
  real   freq_scale_mult = 1.0;
  int    type = WAVE_REAL_CPLX;

  Wavelet *wavelet;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &wavelet_name, &d_r_expr, &d_x_min, &d_x_max,
	       &time_scale_mult, &freq_scale_mult) == TCL_ERROR)
    return TCL_ERROR;

  is_rc = arg_present (1);
  is_cr = arg_present (2);
  is_cc = arg_present (3);
  is_rr = arg_present (4);

  /* Get the expressions of all the function pointers we need. */
  if (is_rc)
    {
      type = WAVE_REAL_CPLX;
      if (arg_get (1, &f_r_expr, &f_i_expr, &f_x_min, &f_x_max) == TCL_ERROR)
	return TCL_ERROR;
    }
  if (is_cr)
    {
      type = WAVE_CPLX_REAL;
      if (arg_get (2, &d_i_expr, &f_r_expr, &f_x_min, &f_x_max) == TCL_ERROR)
	return TCL_ERROR;
    }
  if (is_cc)
    {
      type = WAVE_CPLX_CPLX;
      if (arg_get (3, &d_i_expr, &f_r_expr, &f_i_expr,
		   &f_x_min, &f_x_max) == TCL_ERROR)
	return TCL_ERROR;
    }
  if (is_rr)
    {
      type = WAVE_REAL_REAL;
      if (arg_get (4, &f_r_expr, &f_x_min, &f_x_max) == TCL_ERROR)
	return TCL_ERROR;
    }

  if (d_x_min > d_x_max)
    {
      sprintf (interp->result,
	       "The min of the direct domain (%f) must be lesser than the\n"
	       "max (%f).",
	       d_x_min, d_x_max);
      return TCL_ERROR;
    }

  if (f_x_min > f_x_max)
    {
      sprintf (interp->result,
	       "The min of the fourier domain (%f) must be lesser than the\n"
	       "max (%f).",
	       f_x_min, f_x_max);
      return TCL_ERROR;
    }

  /* Get all the function pointers we need. */
  /*nameargu("x", "a");*/
  if (d_i_expr)
    {
      /*d_i_fct_ptr = dfopen (d_i_expr);*/
      d_i_fct_ptr = evaluator_create(d_i_expr);
      if (!d_i_fct_ptr)
	{
	  Tcl_AppendResult (interp, "libmatheval : error", " in expression ", d_i_expr);
	  return TCL_ERROR;
	}
    }
  if (f_r_expr)
    {
      /*f_r_fct_ptr = dfopen (f_r_expr);*/
      f_r_fct_ptr = evaluator_create(f_r_expr);
      if (!f_r_fct_ptr)
	{
	  Tcl_AppendResult (interp, "libmatheval : error", " in expression ", f_r_expr);
	  return TCL_ERROR;
	}
    }
  if (f_i_expr)
    {
      /*f_i_fct_ptr = dfopen (f_i_expr);*/
      f_i_fct_ptr = evaluator_create(f_i_expr);
      if (!f_i_fct_ptr)
	{
	  Tcl_AppendResult (interp, "libmatheval : error", " in expression ", f_i_expr);
	  return TCL_ERROR;
	}
    }

  /*d_r_fct_ptr = dfopen (d_r_expr);*/
  d_r_fct_ptr = evaluator_create(d_r_expr);
  if (!d_r_fct_ptr)
    {
      Tcl_AppendResult (interp, "libmatheval : error", " in expression ", d_r_expr);
      return TCL_ERROR;
    }
  /*nameargu("x", "y");*/

  wavelet = wt1d_new_wavelet (type,
			      d_r_fct_ptr,
			      d_i_fct_ptr,
			      f_r_fct_ptr,
			      f_i_fct_ptr,
			      d_x_min,
			      d_x_max,
			      f_x_min,
			      f_x_max,
			      time_scale_mult,
			      freq_scale_mult);

  store_wavelet_in_dictionary (wavelet_name, wavelet);

  evaluator_destroy(d_r_fct_ptr);
  evaluator_destroy(d_i_fct_ptr);
  evaluator_destroy(f_r_fct_ptr);
  evaluator_destroy(f_i_fct_ptr);

  sprintf (interp->result, "%s", wavelet_name);

  return TCL_OK;
}

/*
 */
int
wavelet_to_signal_TclCmd_ (ClientData clientData,
			   Tcl_Interp *interp,
			   int        argc,
			   char       **argv)
{
  char * options[] = { "sd[ff]",
		       "-four", "",
		       "-tab", "",
		       NULL };

  char * help_msg =
  {("Fill a signal with the direct form of a wavelet. Default scale is 1.\n"
    "  -four with fourier form.\n"
    "  -tab  Tabulation of the wavelet.")};

  int     is_four;
  int     is_tab;

  Signal  *signal;
  char    *signal_name;
  int     nb_of_points;
  real    first_x, last_x;
  real    scale = 1.0;
  int     flag = WAVE_DIRECT;
  int     i;
  real    dx;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal_name, &nb_of_points, &scale, &dx) == TCL_ERROR)
    return TCL_ERROR;

  is_four = arg_present (1);
  is_tab  = arg_present (2);

  if (is_four)
    {
      flag = WAVE_FOURIER;
      switch (wt1d_wavelet_type (current_wavelet))
	{
	case WAVE_REAL_REAL:
	case WAVE_CPLX_REAL:
	  signal = sig_new (REALY, 0, nb_of_points - 1);
	  break;
	case WAVE_REAL_CPLX:
	case WAVE_CPLX_CPLX:
	  signal = sig_new (CPLX, 0, nb_of_points - 1);
	  break;
	}
    }
  else
    switch (wt1d_wavelet_type (current_wavelet))
      {
      case WAVE_REAL_REAL:
      case WAVE_REAL_CPLX:
	signal = sig_new (REALY, 0, nb_of_points - 1);
	break;
      case WAVE_CPLX_REAL:
      case WAVE_CPLX_CPLX:
	signal = sig_new (CPLX, 0, nb_of_points - 1);
	break;
    }

  if (is_tab)
    {
      int size;
      real *tab_data;

      tab_data = wt1d_tab_wavelet (current_wavelet,
				   scale,
				   dx,
				   &size,
				   flag);
      /*      if (signal->type == CPLX) size = size*2;*/
      signal->dx = dx;
      for (i = 0; i < size && i < signal->size; i++)
	signal->dataY[i] = tab_data[i];
      for (; i < signal->size; i++)
	signal->dataY[i] = 0.0;
    }
  else
    {
      wt1d_wavelet_to_data (current_wavelet,
			    scale, nb_of_points, (void *) signal->dataY,
			    &first_x, &last_x, flag);
      signal->x0 = first_x;
      signal->dx = (last_x - first_x)/(nb_of_points-1);
    }
  
  store_signal_in_dictionary (signal_name, signal);

  return TCL_OK;
}

/*
 */
int
wt1d_single_TclCmd_ (ClientData clientData,
		     Tcl_Interp *interp,
		     int        argc,
		     char       **argv)
{
  char * options[] = { "Ssf",
		       "-normexpo", "f",
		       NULL };

  char * help_msg =
  {("Wavelet transform of a signal.")};

  Signal  *signal;
  Signal  *result;
  char    *result_name;
  real    scale;

  real    normExpo = 0.0;
  int isNormexpo;

  int  first_exact;
  int  last_exact;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0,&signal,  &result_name, &scale) == TCL_ERROR)
    return TCL_ERROR;

  isNormexpo = arg_present(1);
  if (isNormexpo) {
    if (arg_get(1, &normExpo) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  if (signal->type != REALY && signal->type != CPLX)
    {
      sprintf (interp->result,
	       "Sorry, but the signal must be a REALY or a CPLX signal.");
      return TCL_ERROR;
    }

  switch (wt1d_wavelet_type (current_wavelet))
    {
    case WAVE_REAL_REAL:
    case WAVE_REAL_CPLX:
      if (signal->type == REALY)
	result = sig_new (REALY, 0, signal->size - 1);
      else if (signal->type == CPLX)
	result = sig_new (CPLX, 0, signal->size/2 - 1);
      break;
    case WAVE_CPLX_REAL:
    case WAVE_CPLX_CPLX:
      result = sig_new (CPLX, 0, signal->size/2 - 1);
      break;
    }
  if (signal->type == REALY) {
    result-> dataY = wt1d_real_single (signal->dataY,
				       signal->size,
				       current_wavelet,
				       scale,
				       normExpo,
				       (real *) result->dataY,
				       CV1D_PERIODIC,
				       &first_exact,
				       &last_exact);
  } else if (signal->type == CPLX) {
    result-> dataY = (real *) wt1d_cplx_single ((complex *) signal->dataY,
						signal->size / 2,
						current_wavelet,
						scale,
						normExpo,
						result->dataY,
						CV1D_PERIODIC,
						&first_exact,
						&last_exact);
  }

  store_signal_in_dictionary (result_name, result);

  return TCL_OK;
}

/*
 */
int
wt1d_all_TclCmd_ (ClientData clientData,
		  Tcl_Interp *interp,
		  int        argc,
		  char       **argv)
{
  char * options[] = { "Ssfdd",
		       "-normexpo", "f",
		       "-border", "d",
		       NULL };

  char * help_msg =
  {("Wavelet transform of a signal.")};

  Signal  *signal;
  Signal  **result;
  char    **result_name;
  char    *base_name;
  int     lenght;
  real    first_scale;
  int     nb_of_octave;
  int     nb_of_vox;
  int     octave;
  int     vox;
  int     i = 0;
  real    **res_data;
  real    **c_data;
  real    min_scale;
  real    max_scale;
  real    last_scale;
  real    normExpo = 0.0;

  int    borderEffect = CV1D_PERIODIC;

  int  first_exact;
  int  last_exact;

  int isNormexpo;
  int isBorder;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &signal, &base_name, &first_scale,
	       &nb_of_octave, &nb_of_vox) == TCL_ERROR)
    return TCL_ERROR;

  isNormexpo = arg_present(1);
  if (isNormexpo) {
    if (arg_get(1, &normExpo) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }
  isBorder = arg_present(2);
  if (isBorder) {
    if (arg_get(2, &borderEffect) == TCL_ERROR) {
      return TCL_ERROR;
    }
  }

  if (borderEffect != CV1D_PERIODIC
      && borderEffect != CV1D_MIRROR
      && borderEffect != CV1D_PADDING
      && borderEffect != CV1D_0_PADDING) {
      sprintf (interp->result,
	       "bad value for option \"-border\"");
      return TCL_ERROR;
  }

  if (signal->type != REALY && signal->type != CPLX)
    {
      sprintf (interp->result,
	       "Sorry, but the signal must be a REALY or a CPLX signal.");
      return TCL_ERROR;
    }

  wt1d_wavelet_scale_limits (current_wavelet,
			     signal->n,
			     /*			     (signal->last - signal->first+1)*signal->dx,*/
			     /*			     signal->dx,*/
			     &min_scale,
			     &max_scale);
  /* ("%g %g\n", min_scale, max_scale);*/
  if (first_scale < min_scale)
    {
      sprintf (interp->result,
	       "The first scale (%g) must be greater than %.15g.",
	       first_scale, min_scale);
      return TCL_ERROR;
    }

  last_scale =
    first_scale*pow(2, nb_of_octave-1+(double)(nb_of_vox-1)/nb_of_vox);
  if (last_scale > max_scale)
    {
      sprintf (interp->result,
	       "The last scale (%g) must be lesser than %.15g.",
	       last_scale, max_scale);
      return TCL_ERROR;
    }

  lenght = strlen (base_name) + 3;

  result = (Signal **) malloc (nb_of_octave*nb_of_vox*sizeof (Signal *));
  result_name = (char **) malloc (nb_of_octave*nb_of_vox*sizeof (char *));

  switch (signal->type)
    {
    case REALY:
      res_data = (real **) malloc (nb_of_octave*nb_of_vox*sizeof (real *));

      for (octave = 0; octave < nb_of_octave; octave++)
	for (vox = 0; vox < nb_of_vox; vox++)
	  {
	    switch (wt1d_wavelet_type (current_wavelet))
	      {
	      case WAVE_REAL_REAL:
	      case WAVE_REAL_CPLX:
		result[i] = sig_new (REALY, 0, signal->size-1);
		break;
	      case WAVE_CPLX_REAL:
	      case WAVE_CPLX_IMAG:
	      case WAVE_CPLX_CPLX:
		result[i] = sig_new (CPLX, 0, signal->size-1);
		break;
	      default:
		result[i] = sig_new (REALY, 0, signal->size-1);
		break;
	      }

	    result[i]->dx = signal->dx;
	    result[i]->x0 = signal->x0;

	    result_name[i] = (char *) malloc (lenght*sizeof (char));
	    sprintf (result_name[i], "%s%.3d", base_name, i);
	
	    res_data[i] = result[i]->dataY;
	    i++;
	  }
      res_data = wt1d_real_all (signal->dataY,
				signal->n,
				current_wavelet,
				first_scale,
				nb_of_octave,
				nb_of_vox,
				normExpo,
				res_data,
				borderEffect,
				&first_exact,
				&last_exact);
      break;
    case CPLX:
      c_data = (real **) malloc (nb_of_octave*nb_of_vox*sizeof (complex *));

      for (octave = 0; octave < nb_of_octave; octave++)
	for (vox = 0; vox < nb_of_vox; vox++)
	  {
	    result[i] = sig_new (CPLX, 0, signal->n - 1);

	    result[i]->dx = signal->dx;
	    result[i]->x0 = signal->x0;

	    result_name[i] = (char *) malloc (lenght*sizeof (char));
	    sprintf (result_name[i], "%s%.3d", base_name, i);
	
	    c_data[i] = result[i]->dataY;
	    i++;
	  }
      c_data = wt1d_cplx_all ((complex *) signal->dataY,
			      signal->n,
			      current_wavelet,
			      first_scale,
			      nb_of_octave,
			      nb_of_vox,
			      normExpo,
			      c_data,
			      borderEffect,
			      &first_exact,
			      &last_exact);
      break;
    }

  i = 0;
  for (octave = 0; octave < nb_of_octave; octave++) {
    for (vox = 0; vox < nb_of_vox; vox++) {
      store_signal_in_dictionary (result_name[i], result[i]);
      i++;
    }
  }

  return TCL_OK;
}

/*
 */
int
tab_scales_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{
  char * options[] = { "fdd[f]",
		       NULL };

  char * help_msg =
  {("Tab a wavelet for several scales.")};

  real    first_scale;
  int     nb_of_octave;
  int     nb_of_vox;
  int     octave;
  int     vox;
  int     i = 0;
  real    scale;
  int     tab_size;
  real    dx = 1;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &first_scale, &nb_of_octave, &nb_of_vox, &dx) == TCL_ERROR)
    return TCL_ERROR;

  i = 0;
  for (octave = 0; octave < nb_of_octave; octave++)
    for (vox = 0; vox < nb_of_vox; vox++)
      {
	scale = first_scale*pow (2, octave+(double)vox/nb_of_vox);

	wt1d_tab_wavelet (current_wavelet,
			  scale,
			  dx,
			  &tab_size,
			  WAVE_DIRECT);
      }

  return TCL_OK;
}

/*
 */
int
get_scales_TclCmd_ (ClientData clientData,
		   Tcl_Interp *interp,
		   int        argc,
		   char       **argv)
{
  char * options[] = { "fdd",
		       NULL };

  char * help_msg =
  {(" Gives the corresponding scales from the first scale the number of\n"
    "octaves and the number of vox.")};

  real    first_scale;
  int     nb_of_octave;
  int     nb_of_vox;
  int     octave;
  int     vox;
  int     i = 0;
  real    scale;
  char    string[100];

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &first_scale, &nb_of_octave, &nb_of_vox) == TCL_ERROR)
    return TCL_ERROR;

  i = 0;
  for (octave = 0; octave < nb_of_octave; octave++)
    for (vox = 0; vox < nb_of_vox; vox++)
      {
	scale = first_scale*pow (2, octave+(double)vox/nb_of_vox);
	sprintf (string, "%f", scale);
	Tcl_AppendElement (interp, string);
      }

  return TCL_OK;
}

/*
 */
int
get_wavelet_from_collection_TclCmd_ (ClientData clientData,
				     Tcl_Interp *interp,
				     int        argc,
				     char       **argv)
{
  char * options[] = { "s",
		       NULL };

  char * help_msg =
  {(" Init current wavelet.")};

  char    *string;

  if (arg_init (interp, argc, argv, options, help_msg))
    return TCL_OK;
  
  if (arg_get (0, &string) == TCL_ERROR)
    return TCL_ERROR;

  if (!strcmp (string, "gauss"))
    current_wavelet = wt1d_gaussian_ptr;
  else if (!strcmp (string, "d1_gauss"))
    current_wavelet = wt1d_d1_gaussian_ptr;
  else if (!strcmp (string, "d2_gauss"))
    current_wavelet = wt1d_d2_gaussian_ptr;
  else if (!strcmp (string, "d3_gauss"))
    current_wavelet = wt1d_d3_gaussian_ptr;
  else if (!strcmp (string, "d4_gauss"))
    current_wavelet = wt1d_d4_gaussian_ptr;
  else if (!strcmp (string, "morlet"))
    current_wavelet = wt1d_morlet_ptr;
  else
    {
      sprintf (interp->result, "The wavelet \"%s\" doesn't exist in the"
	       " collection.", string);
      return TCL_ERROR;
    }
  return TCL_OK;
}
