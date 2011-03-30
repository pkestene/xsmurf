#include "stdlib.h"
#include "Image.h"
#include "Colormap.h"

/*----------------------------------------------------------------------
  ViewImageCreate_
  --------------------------------------------------------------------*/
ViewImage *
ViewImaCreate_(int lx,int ly)
{
  ViewImage * image;
  
  image = (ViewImage *) malloc (sizeof(ViewImage));
  image->lx = lx;
  image->ly = ly;
  //  image->data = (char *) malloc (lx*ly*sizeof(char));
  image->data = (unsigned long *) malloc (lx*ly*sizeof(unsigned long));

  return image;
}

/*----------------------------------------------------------------------
  ViewImageCreateFromData_
  --------------------------------------------------------------------*/
ViewImage *
ViewImaCreateFromData_(int lx,int ly,unsigned long * data)
{
  ViewImage * image;
  
  image = (ViewImage *) malloc (sizeof(ViewImage));
  image->lx = lx;
  image->ly = ly;
  image->data = data;

  return image;
}
		   

/*----------------------------------------------------------------------
  ViewImaDelete_
  --------------------------------------------------------------------*/
void
ViewImaDelete_(ViewImage * image)
{
  if (image)
    {
      if (image->data)
	free(image->data);
      free(image);
    }
}

/*----------------------------------------------------------------------
  ViewImaClear_

  Met le contenu de la viewImage a la couleur de fond 
  --------------------------------------------------------------------*/
void
ViewImaClear_(ViewImage * viewImagePtr)
{
  int i,bgPixel;
  
  bgPixel = get_color_intensity_ (BLACK);
  //bgPixel = get_color_intensity_ (WHITE);
  
  for (i=0;i<viewImagePtr->lx*viewImagePtr->ly;i++)
    viewImagePtr->data[i] = bgPixel;
}
