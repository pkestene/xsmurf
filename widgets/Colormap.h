#ifndef __COLORMAP2_H__
#define __COLORMAP2_H__

#include "ViewerLib.h"

#ifndef _REAL_
#define _REAL_

typedef float real;

#endif

#define MAX_COL_NUMBER 256
#define MIN_COL_NUMBER   2

enum {
  WHITE,
  BLACK,
  RED,
  BLUE,
  YELLOW,
  GREEN,
  GREY1,
  GREY2,
  GREY3
  };
/*enum {
  BLACK,
  WHITE,
  RED,
  BLUE,
  YELLOW,
  GREEN,
  GREY1,
  GREY2,
  GREY3
  };*/

/*int  ViewColGetPixel_            (int);*/
int  ViewColBwGetPixel_          (int);
int  ViewColNoBwGetPixel_        (int);
int  ViewColGetPixelByName_      (char *);
int  get_number_of_colors_       ();
int  get_number_of_grey_scales_  ();
int  get_depth_                  ();
int  ViewColGetNumberOfColors_   ();
int  ViewColBwGetNumberOfColors_ ();
/*int  ViewColGetForegroundPixel_  ();*/
int  ViewColGetBackgroundPixel_  ();
int  set_colormap_TclCmd_         (ClientData,Tcl_Interp *,int,char**);
void init_colormap_                (Tk_Window);

int  ViewColConvertToPixelInit_  (real,real);
unsigned long  ViewColLogConvertToPixel_   (real);
unsigned long  ViewColLinConvertToPixel_   (real);
unsigned long  ViewColDisConvertToPixel_   (real);
void ViewColConvertHighlight_    (real);

unsigned long get_grey_intensity_ (int grey_number);
unsigned long get_color_intensity_ (int color_number);

unsigned long get_color_by_name_ (Tk_Window, char *name);

#endif
