/*
 * nuage.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: nuage.c,v 1.6 1998/07/27 14:45:34 decoster Exp $
 */

#include "image_int.h"

/*
 * Deplacer ces fonctions vers le module de lecture d'images a partir
 * de fichiers.
 */

enum {
  INT_FRMT,
  REAL_FRMT,
  CHAR_FRMT
};

/*------------------------------------------------------------------------
  Cette fonction "charge" un nuage en memoire.
  ----------------------------------------------------------------------*/
/*void
_ImaNuaLoadFunction_(Image *image,
		     char  *nom,
		     int   taille,
		     int   nrow,
		     int   x0,
		     int   y0,
		     int   format,
		     int   pas,
		     int   zero)
{
  FILE          *fin;
  real         *values;
  int           i, j, x, y;
  unsigned char c;
  unsigned int i_data;
  real         f;
  int           nou_taille;
  
  nou_taille=taille/pas;
  
  values = (real *) image->data;
  for(i = 0; i < nou_taille; i++)
    for(j = 0; j < nou_taille; j++)
      values[i+j*nou_taille] = 0.0;
  
  
  fin = fopen(nom, "r");
  if(fin == NULL)
    return;
  else {
    switch (format) {
    case REAL_FRMT:
      for(y = 0; y < taille; y++) {
	fseek(fin, (x0+(y+y0)*nrow)*sizeof(real), SEEK_SET);
	for(x = 0; x < taille; x++) {
	  fread(&f, sizeof(real), 1, fin);
	  if(((x%pas) == 0) & ((y%pas) == 0))
	    values[x/pas+y*nou_taille/pas] = f;
	}
      }
      break;
    case CHAR_FRMT:      
      for(y = 0; y < taille; y++) {
	fseek(fin, (x0+(y+y0)*nrow)*sizeof(char), SEEK_SET);
	for(x = 0; x < taille; x++) {
	  fread(&c, sizeof(char), 1, fin);
	  if(((x%pas) == 0) & ((y%pas) == 0))
	    values[x/pas+y*nou_taille/pas] = (real)c;
	}
      }
      break;
    case INT_FRMT:      
      for(y = 0; y < taille; y++) {
	fseek(fin, (x0+(y+y0)*nrow)*sizeof(int), SEEK_SET);
	for(x = 0; x < taille; x++) {
	  fread(&i_data, sizeof(int), 1, fin);
	  if(((x%pas) == 0) & ((y%pas) == 0))
	    values[x/pas+y*nou_taille/pas] = (real)i_data;
	}
      }
      break;
    }
    fclose(fin);
  }
  if(zero==1)
    for(i = 0; i < nou_taille; i++)
      for(j = 0; j < nou_taille; j++)
	if(values[i+j*nou_taille] == 255.0)
	  values[i+j*nou_taille] = 0.; 
}
*/

void
_ImaNuaLoadFunction_(Image *image,
		     char  *nom,
		     int   taille,
		     int   nrow,
		     int   x0,
		     int   y0,
		     int   is_real,
		     int   pas,
		     int   zero,
		     int   is_new_format)
{
  FILE          *fin;
  real         *values;
  int           i, j, x, y;
  unsigned char c;
  real         f;
  int           nou_taille;

  int   k;
  short cc;

  nou_taille=taille/pas;
  
  values = (real *) image->data;
  for(i = 0; i < nou_taille; i++)
    for(j = 0; j < nou_taille; j++)
      values[i+j*nou_taille] = 0.0;
  
  
  fin = fopen(nom, "r");
  if(fin == NULL)
    return;
  else {
    if(is_real)
      for(y = 0; y < taille; y++) {
	fseek(fin, (x0+(y+y0)*nrow)*sizeof(real), SEEK_SET);
	for(x = 0; x < taille; x++) {
	  fread(&f, sizeof(real), 1, fin);
	  if(((x%pas) == 0) & ((y%pas) == 0))
	    values[x/pas+y*nou_taille/pas] = f;
	}
      }
    else
      if (is_new_format){
	for(k=0;k<49;k++) {
	  fread(&cc,sizeof(short),1,fin); 
	}
	for(y = 0; y < taille; y++) {
          for(k=0;k<2;k++) {
	    fread(&cc,sizeof(short),1,fin); 
	  }
	  /*	  fseek(fin, (x0+(y+y0)*nrow)*sizeof(int), SEEK_SET);*/
	  for(x = 0; x < taille; x++) {
	    fread(&cc, sizeof(short), 1, fin);
	    if(((x%pas) == 0) & ((y%pas) == 0))
	      values[x/pas+y*nou_taille/pas] = (real)cc;
	  }
          for(k=0;k<2;k++) {
	    fread(&cc,sizeof(short),1,fin); 
	  }
	}
      }
      else
	for(y = 0; y < taille; y++) {
	  fseek(fin, (x0+(y+y0)*nrow)*sizeof(char), SEEK_SET);
	  for(x = 0; x < taille; x++) {
	    fread(&c, sizeof(char), 1, fin);
	    if(((x%pas) == 0) & ((y%pas) == 0))
	      values[x/pas+y*nou_taille/pas] = (real)c;
	  }
	}
    fclose(fin);
  }
  if(zero==1)
    for(i = 0; i < nou_taille; i++)
      for(j = 0; j < nou_taille; j++)
	if(values[i+j*nou_taille] == 255.0)
	  values[i+j*nou_taille] = 0.; 
}

/*========================================================*/
/* lecture des nuages simules (ascii)                     */
/*========================================================*/
void
_ImaSimNuaLoadFunction_(Image *image,
			char  *nom,
			int   taille)
{
  FILE          *fin;
  real         *values;
  int           i, j;
  real         inten;

  values = (real *) image->data;
  for (i = 0; i < taille; i++)
    for (j = 0; j < taille; j++)
      values[i*taille+j] = 0.0;
  
  fin = fopen (nom,"r");
  if (fin == NULL)
    return;
  else {
    for (i=0;i<taille;i++)
      for (j=0;j<taille;j++) {
	fscanf(fin,"%f\n",&inten);
	values[i*taille+j]=(real)inten;
      }
    fclose (fin);
  }
}
