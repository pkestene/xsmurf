#ifndef __VL_IMAGE_H__
#define __VL_IMAGE_H__

#include "Colormap.h"

/*----------------------------------------------------------------------------
  Definition d'une image
  --------------------------------------------------------------------------*/
typedef struct
{
  int lx,ly;
  unsigned long * data;
} ViewImage;

/*----------------------------------------------------------------------------
  Procedures publiques du module
  --------------------------------------------------------------------------*/
void        ViewImaDelete_        (ViewImage *);/* destructeur de ViewImage */
ViewImage * ViewImaCreate_        (int,int);
ViewImage * ViewImaCreateFromData_(int,int,unsigned long *);
void        ViewImaClear_         (ViewImage *);

#endif
