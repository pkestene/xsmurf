/*
 * fast_convol --
 *
 *  $Id: fast_convol.c,v 1.3 1999/01/08 18:09:21 decoster Exp $
 */

/****************************************************************/
/*              (c) Copyright  1990, 1991, 1992                 */
/*                         by                                   */
/*             Stephane Mallat, Sifen Zhong                     */
/*    Author: Stephen Mallat, Sifen Zhong and Hwang, Wen-Liang  */
/*                 New York University                          */
/*                 All right reserved                           */
/****************************************************************/


#include "../signal/signal.h"
#include "../image/image.h"
#include "dyadique.h"
#include "../wt2d/wt2d.h"

extern void store_image (char  *name, Image *image_ptr);

/* #include "motion.h" */

#define mod(a,d) (((a) % (d) + (d)) % (d))
#define COMMENT '#'
#define EO_FILTER '$'

#define get_token \
do { \
  fgets (input, STRING_SIZE, fp); \
  token = strtok(input, " "); \
} while (!token || token[0] == COMMENT)

#define nrerror(msg) return(0)

/**************************************************************/
/**************************************************************/
float convbf[66000];
FILTER filterh1,filterg1,filterk1;
FILTER filterh2,filterg2,filterk2;

float magnitude(x,y)
  float x,y;
{
  return((float)sqrt((double)(x*x + y*y)));
}

float argument(x,y)
  float x,y;
{
  if(x == 0.0 && y == 0.0)
    return(0.0);
  else
    return((float)atan2((double)y,(double)x));
}

float horizontal(mag,arg)
  float mag,arg;
{
  return((float)(mag * cos((double)arg)));
}

float vertical(mag,arg)
  float mag,arg;
{
  return((float)(mag * sin((double)arg)));
}

/* Return a new filter structure */

FILTER new_filter(name)
     char *name;
{
  FILTER filter;

  if(!(filter = (FILTER) (malloc(sizeof(struct filter)))))
    nrerror("Mem. alloc for FILTER failed\n");
  strcpy(filter->name,name);
  return(filter);
}

/*******************************************************/
/* Read in a pair of filters from a file.              */
/* '#' is used for comments, '$' to end a filter       */
/* Blank lines are skipped, and so are leading blanks. */
/* Format:                                             */
/*                                                     */
/* H_size                                              */
/* H_shift                                             */
/* symmetry                                            */
/* coefficents of H                                    */
/* $                                                   */
/* G_size                                              */
/* G_shift                                             */
/* symmetry                                            */
/* coefficents of G                                    */
/* $                                                   */
/* K_size                                              */
/* K_shift                                             */
/* symmetry                                            */
/* coeff. of K                                         */
/* $                                                   */
/* factor ---- (H,K,G)->factor                         */
/* $                                                   */
/*******************************************************/

void filttri_read(filename, H, G, K)
  char    *filename;
  FILTER  H, G, K;
{
  FILE    *fp;
  char     input[STRING_SIZE], *token;
  int      j;

  if (!(fp = fopen(filename, "r"))) {
    printf("Error: file %s does not exist!\n", filename);
    return;
  }

  get_token;
  H->size = atoi(token);
  get_token;
  H->shift = atoi(token);
  get_token;
  H->symmetry = atof(token);
  for (j = 0; j < H->size; j++) {
    get_token;
    H->values[j] = atof(token);
  }

  /* Skip the extra coefficents. */
  for (; j < FILT_SIZE; j++) {
    get_token;
    if (token[0] == EO_FILTER)
      break;
  }

  get_token;
  G->size = atoi(token);
  get_token;
  G->shift = atoi(token);
  get_token;
  G->symmetry = atof(token); 
  for (j = 0; j < G->size; j++) {
    get_token;
    G->values[j] = atof(token);
  }

  /* Skip the extra coefficents. */
  for (; j < FILT_SIZE; j++) {
    get_token;
    if (token[0] == EO_FILTER)
      break;
  }

  get_token;
  K->size = atoi(token);
  get_token;
  K->shift = atoi(token);
  get_token;
  K->symmetry = atof(token); 
  for (j = 0; j < K->size; j++) {
    get_token;
    K->values[j] = atof(token);
  }

  /* Skip the extra coefficents. */
  for (; j < FILT_SIZE; j++) {
    get_token;
    if (token[0] == EO_FILTER)
      break;
  }
 
  /* Read factors */
  j = 0;
  get_token;
  while (token[0] != EO_FILTER && j < NFACT) 
    {
      H->factors[j] = atof(token);
      K->factors[j] = atof(token);
      G->factors[j++] = atof(token);
      get_token;
    }

  fclose(fp);
}

/*******************************************************************/
/* initialize the filter path by string cancating filter path with */
/* filter type                                                     */
/*******************************************************************/

int init_filters()
{
  char FILT1[STRING_SIZE];
  char FILT2[STRING_SIZE];
  char FltrDir[STRING_SIZE];

  strcpy(FltrDir,"/u/decoster/xsmurf/filter/");

  strcpy(FILT1,FltrDir);
  strcat(FILT1,"p3.1");

  filterh1 = new_filter(FILT1);  
  filterg1 = new_filter(FILT1);
  filterk1 = new_filter(FILT1);
  if (!filterh1 || !filterg1 || !filterk1) {
    return 0;
  }

  filttri_read(FILT1,filterh1,filterg1,filterk1);  /* decomposition */
 
  strcpy(FILT2,FltrDir);
  strcat(FILT2,"p3.2");

  filterh2 = new_filter(FILT2);
  filterg2 = new_filter(FILT2);
  filterk2 = new_filter(FILT2);
  if (!filterh2 || !filterg2 || !filterk2) {
    return 0;
  }

  filttri_read(FILT2,filterh2,filterg2,filterk2);  /* reconstruction */
  return 1;

}

void
symevn(n,v,u)
  int n;
  float v[], u[];
{
  float *vp, *up, *vn = v+n;
  for(vp = v, up = u; vp < vn; vp++, up++)
    *up = *vp;  
  for(vp = vn-1; vp >= v; vp--, up++)
    *up = *vp;
}

void
asyevn(n,v,u)
  int n;
  float v[], u[];
{
  float *vp, *up, *vn = v+n;
  for(vp = v, up = u; vp < vn; vp++, up++)
    *up = *vp;
  for(vp = vn-1; vp >= v; vp--, up++)
    *up = -(*vp);
}

void
symodd(n,v,u)
  int n;
  float v[], u[];
{
  float *vp, *up, *vn = v+n;
  for(vp = v, up = u; vp < vn; vp++, up++)
    *up = *vp;
  for(vp = vn-2; vp > v; vp--, up++)
    *up = *vp;
}

void
asyodd(n,v,u)
  int n;
  float v[], u[];
{
  float *vp, *up, *vn = v+n;
  for(vp = v, up = u; vp < vn; vp++, up++)
    *up = *vp;
  for(vp = vn-2; vp > v; vp--, up++)
    *up = -(*vp);
}

void
ref(border,n,v,u)
  int border;
  int n;
  float v[], u[];
{
  switch(border) {
    case SYMEVN:
      n-= n%2;
      symevn(n,v,u);
      break;
    case SYMODD:
      n+= (1-n%2);
      symodd(n,v,u);
      break;
    case ASYEVN:
      n-= n%2;
      asyevn(n,v,u);
      break;
    case ASYODD:
      n+= (1-n%2);
      asyodd(n,v,u);
      break;
  }
}

Image *
convme(input, filt, scale)
     Image *input;
     FILTER filt;
     int scale;
{
  Image * output;
  int ncol = input->lx,
      nrow = input->ly;
  int border= input->border_hor;
  float *image_input;
  float *image_output;
  int filtsize = filt->size;
  int filtshift = filt->shift;
  float filtsym = filt->symmetry;
  float *filter = filt->values;
  int i, I, Iout, j, k, left, right, l1, r1;
  float sum;
  int ncol2= 2*(ncol-ncol%2);
  int ncol_out= ncol;
  int border_output= border;

  if(scale == 1) {
    ncol_out+= (ncol_out%2 ? -1 : 1);
    switch(filtshift) {
    case 1:
      l1= 1;
      r1= 0;
      if(filtsym < 0.0){
        border_output= ASYODD;
      }
      else {
        border_output= SYMODD;
      }
      break;
    case -1:
      l1= 0;
      r1= 1;
      if(filtsym < 0.0) {
	/* need REWRITE Zhong */
        border_output= border + (border>1 ? -1-2*(border%2) : 3-2*(border%2));
      } else {
        border_output= SYMEVN;
      }
      break;
     case 0:
       l1= r1= 1;
       border_output= border;
       break;
    }
  } else {
    if(filtshift) {
      l1= r1= scale/2;
    } else {
      l1= r1= scale;
    }
    if(filtsym < 0.0) {
      border_output= border + (border>1 ? -2 : 2);
    } else {
      border_output= border;
    }
  }
  
  output = im_new(ncol_out,nrow,nrow*ncol_out,PHYSICAL);
  output->border_hor= border_output;
  output->border_ver= input->border_ver;
  image_input = (float *)input->data;
  image_output = (float *)output->data;

  for(i= 0, I= 0, Iout= 0; i < nrow; i++, I+= ncol, Iout+= ncol_out) {
    ref(border,ncol,image_input+I,convbf);
    for(j= 0; j < ncol_out; j++) {
        sum= filter[0] * convbf[j];
        for(k= 1, left= j-l1, right= j+r1;
            k < filtsize ;
            k++, right+= scale, left-= scale) {
          sum+= filter[k] * (filtsym*convbf[mod(left,ncol2)]
                                   + convbf[mod(right,ncol2)]);
        }
        image_output[Iout+j]= sum;
      }
    }

  return(output);
}

Image *
conv_hv(input,h_filt,h_scale,v_filt,v_scale)
     Image * input;
     FILTER h_filt,v_filt;
     int h_scale,v_scale;
{
  Image * wrk_image, * result;
  int i, j, I, J;
  int nrow, ncol;
  

  nrow = input->lx;
  ncol = input->ly;
  
  wrk_image=convme(input,h_filt,h_scale);

  result=im_new(wrk_image->ly,wrk_image->lx,wrk_image->size,PHYSICAL);
  for (i= 0, I= 0; i < result->lx ; i++)
    for (j= 0 , J= 0; j < result->ly; j++ , J+= result->lx, I++) 
      result->data[J+i] = wrk_image->data[I];
  result->border_hor= wrk_image->border_ver;
  result->border_ver= wrk_image->border_hor;

  im_free(wrk_image);
  wrk_image=convme(result,v_filt,v_scale);
  im_free(result);
  result=im_new(wrk_image->ly,wrk_image->lx,wrk_image->size,PHYSICAL);
  for (i= 0, I= 0; i < result->lx ; i++)
    for (j= 0 , J= 0; j < result->ly; j++ , J+= result->lx, I++)
      result->data[J+i] = wrk_image->data[I];
  result->border_hor= wrk_image->border_ver;
  result->border_ver= wrk_image->border_hor;
  return(result);
}

int
dyadic_decomposition(image,name,bgn_level,num_level)
  int bgn_level,num_level;
  char * name;
  Image * image;
{
  Image *image_hor,*image_ver, *image_app, *image_tmp;
  Image *image_mod,*image_arg;
  int l,i;
  int scale;
  char dst_name[100];

  if (!init_filters()){
    return 0;
  }
  
  
  image->border_hor=0;
  image->border_ver=0;

  image_tmp=im_duplicate(image);
  for(l= 0,scale= 1<<bgn_level; l < num_level; l++,scale+= scale) {

    if (l > 0) {
      im_free(image_tmp);
      image_tmp = im_duplicate(image_app);
    }
    image_app=conv_hv(image_tmp,filterh1,scale,filterh1,scale);
    sprintf(dst_name,"%sapp_%.2d",name, l);
    store_image (dst_name, image_app);

    image_hor=conv_hv(image_tmp,filterg1,scale,filterk1,scale);    
    sprintf(dst_name,"%shor_%.2d",name, l);
    store_image (dst_name, image_hor);

    image_ver=conv_hv(image_tmp,filterk1,scale,filterg1,scale);
    sprintf(dst_name,"%sver_%.2d",name, l);
    store_image (dst_name, image_ver);
    
    image_mod=im_new(image_hor->lx,image_hor->ly,image_hor->size,PHYSICAL);
    image_arg=im_new(image_hor->lx,image_hor->ly,image_hor->size,PHYSICAL);
    for(i=0;i<image_hor->size;i++) {
      image_mod->data[i]=magnitude(image_hor->data[i],image_ver->data[i]);
      image_arg->data[i]=argument(image_hor->data[i],image_ver->data[i]);
    }
    sprintf(dst_name,"%smod_%.2d",name, l);
    store_image (dst_name, image_mod);
    sprintf(dst_name,"%sarg_%.2d",name, l);
    store_image (dst_name, image_arg);
  }
  return 1;
}

/* RECONSTRUCTION FUNCTIONS */

extern Image    * get_image  (char *);

int 
border(basename,noct, flag_precons,preconslx,preconsly)
  char * basename;
  int noct, flag_precons,preconslx,preconsly;
{
  int l;
  Image *image_tmp;
  char dst_name[100];

  /* ATTENTION NE PAS OUBLIER POUR L'IMAGE FINALE 
  wtrans->images[0][0]->border_hor=
  wtrans->images[0][0]->border_ver= SYMEVN;
   */


  for(l=0;l<noct;l++) {
    sprintf(dst_name,"%sapp_%.2d",basename, l);
    if (flag_precons) {
      image_tmp=im_new(preconslx,preconsly,preconslx*preconsly,PHYSICAL);
      im_set_0(image_tmp);
      store_image (dst_name, image_tmp);      
    } else {
      image_tmp =get_image(dst_name);
    }
    if(filterh1->shift) {
      image_tmp->border_hor=SYMODD;
      image_tmp->border_ver=SYMODD;
    }
    else {
      if (l != noct){
	image_tmp->border_hor=SYMEVN;
	image_tmp->border_ver=SYMEVN;
      }
    }
  
  
    sprintf(dst_name,"%shor_%.2d",basename, l);
    if (flag_precons) {
      image_tmp=im_new(preconslx,preconsly,preconslx*preconsly,PHYSICAL);
      im_set_0(image_tmp);
      store_image (dst_name, image_tmp);      
    } else {
      image_tmp =get_image(dst_name);
    }
    if (l == 0) {
      image_tmp->border_hor=ASYODD; /* 3 */
      image_tmp->border_ver=SYMEVN; /* 0 */
    } else {
      if(filterh1->shift) {
	image_tmp->border_hor=ASYODD; /* 3 */ 
	image_tmp->border_ver=SYMODD; /* 1 */
      } else {
	if (l ==noct) {
	  image_tmp->border_hor=SYMEVN; /* 0 */
	} else {
	  image_tmp->border_hor=ASYEVN; /* 2 */
	  image_tmp->border_ver=SYMEVN; /* 0 */
	}
      }
    }    
    
    sprintf(dst_name,"%sver_%.2d",basename, l);
    if (flag_precons) {
      image_tmp=im_new(preconslx,preconsly,preconslx*preconsly,PHYSICAL);
      im_set_0(image_tmp);
      store_image (dst_name, image_tmp); 
    } else {
      image_tmp =get_image(dst_name);
    }
    if (l == 0) {
      image_tmp->border_hor=SYMEVN; /* 0 */
      image_tmp->border_ver=ASYODD; /* 3 */
    } else {
      if(filterh1->shift) {
	image_tmp->border_hor=SYMODD; /* 1 */
	image_tmp->border_ver=ASYODD; /* 3 */
      } else {
	if (l ==noct) {
	  image_tmp->border_ver=SYMEVN; /* 0 */
	} else {
	  image_tmp->border_hor=SYMEVN; /* 0 */
	  image_tmp->border_ver=ASYEVN; /* 2 */
	}
      }
    }
    
  }
  return 1;
}



Image *
dyadic_reconstruction(basename, end_level, num_level)
  char * basename;
  int end_level, num_level;
{

  Image * image_bsc, * image_result, * image_tmp;
  int l;
  int scale;
  char dst_name[100];
  int flag_precons=NO;
  int new_dim=0;

  if (!init_filters()){
    return 0;
  }

  border(basename,num_level,flag_precons,new_dim,new_dim);

  for(l = 0, scale = 1; l < end_level + num_level - 1; l++, scale += scale);
  
  for(; l >= end_level; l--, scale /= 2) {
    sprintf(dst_name,"%sapp_%.2d",basename, l);
    image_bsc = get_image(dst_name);
    image_result=conv_hv(image_bsc, 
	      filterh2, scale, filterh2, scale);


    sprintf(dst_name,"%shor_%.2d",basename, l);
    image_bsc = get_image(dst_name);
    image_tmp=conv_hv(image_bsc, 
	      filterg2, scale, filterk2, scale);
    image_result=im_add(image_result, image_tmp);
    im_free(image_tmp);

    sprintf(dst_name,"%sver_%.2d",basename, l);
    image_bsc = get_image(dst_name);
    image_tmp=conv_hv(image_bsc, 
	      filterk2, scale, filterg2, scale);
    image_result=im_add(image_result, image_tmp);
    im_free(image_tmp);

    if (l> 0) {
      sprintf(dst_name,"%sapp_%.2d",basename, l-1);
      store_image (dst_name, image_result);
    }

  }

  
  return(image_result);
}

/* PROC FOR POINT RECONSTRUCTION */

void
interp(u0, un, n, u, r1)
     float u0, un;
     int n; 
     float u[], r1;
{
  double r_1, r_2, r2, rn_1, r2n_2, r2n, r2n_2i, rn_i, ri, r2i, a0, an;
  int i;

  rn_1 = r1;
  for(i=1; i < n-1 ; ++i)
    rn_1 *= r1;
  r2n_2 = rn_1 * rn_1;
  r_1 = 1. / r1;
  r_2 = r_1 * r_1;
  r2 = r1 * r1;
  r2n = r2n_2 * r2;

  a0 = u0  / (1. - r2n);
  an = un  / (1. - r2n);

  u[0] = u0;
  r2n_2i = r2n_2;
  r2i = r2;
  ri = r1;
  rn_i = rn_1;
  for (i = 1; i < n; i++) {
    u[i] = a0 * ri * (1 - r2n_2i) + an * rn_i * (1 - r2i) ;

    r2n_2i *= r_2;
    r2i *= r2;
    ri *= r1;
    rn_i *= r_1;
  }
  u[n] = un;
}

extern ExtImage * ExtDicGet  (char *);

void
pt_level_proj_1st(ext_image,pic_h,pic_v,level, is_vc)
     ExtImage * ext_image;
     Image * pic_h, * pic_v;
     int level, is_vc;
{
  int nrow = ext_image->lx;
  int ncol = ext_image->ly;
  real *image_h, *image_v;

  //int i,I,j;
  int t0,t1;
  int scale = 1<<level;
  float e0,e1, a, exponent;
  int k,n;
  real u[65536];
  Line     *line_ptr;
  Extremum *ext_ptr;

  image_h=pic_h->data;
  image_v=pic_v->data;


  exponent = 2. / (double)(scale);
  a = 1. / pow(5.8 , (double)exponent);

  printf("%g %d %d %d %d %d %d\n",ext_image->scale,ext_image->lx,ext_image->ly,ext_image->extrNb,ext_image->chainNb,ext_image->nb_of_lines,ext_image->stamp);

  if(is_vc)
    {
      foreach (line_ptr, ext_image -> line_lst)
	{
	  foreach (ext_ptr, line_ptr -> gr_lst)
	    {
	      t0=(ext_ptr->pos)/ncol*ncol;
	      t1=ext_ptr->pos;
	      e0=0.0-image_h[t0];
	      e1=ext_ptr->mod*cos(ext_ptr->arg)-image_h[t1];
	      n=t1-t0;
	      interp(e0, e1, n, u, a);
	      if (ext_image->scale == 16) printf("%d %d %f %f %d %f\n",t0,t1,e0,e1,n,a);

	      for(k= 0; k < n; k++)
		{
		image_h[t0+k]+= u[k];
		/*if (ext_image->scale == 4) 
		 		  printf("%d %d %f\n",t0,k,u[k]);*/
		}
	      image_h[t0+ncol-1]=0.0;
	      
	      t0=ext_ptr->pos%ncol;
	      t1=ext_ptr->pos;
	      e0=0.0-image_v[t0];
	      e1=ext_ptr->mod*sin(ext_ptr->arg)-image_h[t1];
	      n=(t1-t0)/ncol;
	      interp(e0, e1, n, u, a);
	      for(k= 0; k < n; k++)
		{
		  image_v[t0+k]+= u[k];
		  /*		  printf("%d %d %f\n",t0,k,u[k]);*/
		}
	      image_h[t0+(nrow-1)*ncol]=0.0;
	    }
	}
    }
}

void
pt_level_proj_2nd(ext_image,pic_mag,pic_angle,level,is_vc)
     ExtImage * ext_image;
     Image * pic_mag, * pic_angle;
     int level, is_vc;
{

  //int nrow = ext_image->lx;
  int ncol = ext_image->ly;
  real *image_m, *image_a;
  
  int i,j;//I
  int t0,t1;
  int t_min;
  float m0,m1;
  float m_min;
  Line     *line_ptr;
  Extremum *ext_ptr;

  image_m=pic_mag->data;
  image_a=pic_angle->data;

  if(is_vc)
    {
      printf("coucou\n");
      foreach (line_ptr, ext_image -> line_lst)
	foreach (ext_ptr, line_ptr -> gr_lst)
	{
	  t0=(ext_ptr->pos)/ncol*ncol;
	  t1=ext_ptr->pos;
	  m0=image_m[t0];
	  m1=ext_ptr->mod;
	  
	  m_min = image_m[t_min=t0];
	  for(j = t0+1; j <= t1; j++)
	    if(image_m[j] < m_min)
	      m_min = image_m[t_min=j];
	  if(m0 > m_min)
	    for(j = t0+1; j < t_min; j++) {
	      if(fabs(fabs(image_a[j])/M_PI-0.5) >= 0.375) {
		if(image_m[j] > image_m[j-1])
		  image_m[j] = image_m[j-1];
	      } else {
		break;
	      }
	    }
	  if(m1 > m_min)
	    for(j = t1-1; j > t_min; j--) {
	      if(fabs(fabs(image_a[j])/M_PI-0.5) >= 0.375) {
		if(image_m[j] > image_m[j+1])
		  image_m[j] = image_m[j+1];
	      } else {
		break;
	      }
	    }

	  t0=ext_ptr->pos%ncol;
	  t1=ext_ptr->pos;
	  m0=image_m[t0];
	  m1=ext_ptr->mod;

	  m_min = image_m[t_min=t0];
	  for(i = t0+ncol; i <= t1; i += ncol)
	    if(image_m[i] < m_min)
	      m_min = image_m[t_min=i];
	  if(m0 > m_min)
	    for(i = t0+ncol; i < t_min; i += ncol) {
	      if(fabs(fabs(image_a[i])/M_PI-0.5) <= 0.125) {
		if(image_m[i] > image_m[i-ncol])
		  image_m[i] = image_m[i-ncol];
	      } else {
		break;
	      }
	    }
	  if(m1 > m_min)
	    for(i = t1-ncol; i > t_min; i -= ncol) {
	      if(fabs(fabs(image_a[i])/M_PI-0.5) <= 0.125) {
		if(image_m[i] > image_m[i+ncol])
		  image_m[i] = image_m[i+ncol];
	      } else {
		break;
	      }
	    }
	}    
    } 

}


void
point_repr_projection(basename,name2,noct,is_vc,clipping)
     char * basename, * name2;
     int noct,is_vc,clipping;
{
  int i;//j;
  Image * pic_h, * pic_v,* pic_mag;// *pic_angle;
  ExtImage * ext_image;
  char name[100];

  for (i=0;i<noct;i++) {
    sprintf(name,"%s%.2d",basename, i);
    ext_image=ExtDicGet(name);
    sprintf(name,"%sapp_%.2d",name2, i);
    pic_mag=get_image(name);
    sprintf(name,"%shor_%.2d",name2, i);
    pic_h=get_image(name);
    sprintf(name,"%sver_%.2d",name2, i);
    pic_v=get_image(name);


    pt_level_proj_1st(ext_image,pic_h,pic_v,i+1,is_vc);
    /*    if (clipping == YES) {      
      for (j=0;j<pic_mag->size; j++) {
	pic_mag->data[j]=magnitude(pic_h->data[j],pic_v->data[j]);
      }
      pic_angle=im_duplicate(pic_mag);
      for (j=0;j<pic_mag->size; j++) {
	pic_angle->data[j]=argument(pic_h->data[j],pic_v->data[j]);
      }

      pt_level_proj_2nd(ext_image,pic_mag,pic_angle,i+1,is_vc); 
      
      for (j=0;j<pic_mag->size; j++) {
	pic_h->data[j]=horizontal(pic_mag->data[j],pic_angle->data[j]);
	pic_v->data[j]=vertical(pic_mag->data[j],pic_angle->data[j]);
      } 
    }
    */
  }

}

Image *
point_reconstruction(basename, name2,noct,iteration,lx,ly,is_vc)
  char * basename, * name2;
  int noct,iteration,is_vc;
  int lx,ly;
{
  //int initial=YES;
  int clipping=YES;
  //int error_flag=NO;
  int flag_precons=YES;
  Image * image_tmp;
  //int i;

  if (!init_filters()){
    return 0;
  }
  
  border(name2,noct,flag_precons,lx,ly);

  point_repr_projection(basename,name2,noct,is_vc,clipping);
  
  image_tmp=dyadic_reconstruction(name2, 0, noct);
  /*
  printf("iteration =% d\n",iteration);
  for (i = 0; i < iteration; i++) {
    dyadic_decomposition(image_tmp, name2, 0, noct);


    point_repr_projection(basename,name2,noct,is_vc,clipping);


    image_tmp=dyadic_reconstruction(name2, 0, noct);
    
 

    printf("iteration = %d\n",iteration- i -1);
  } 
  */
  return(image_tmp);
}
