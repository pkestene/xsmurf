#include "Convert.h"
#include "../interpreter/arguments.h"
#include "../interpreter/hash_tables.h"
#include "Dictionary.h"
#include <string.h>

/*----------------------------------------------------------------------------
  ViewDrawPlotCmd_
  
  Trace un point dans une ViewImage
  --------------------------------------------------------------------------*/
int
ViewDrawPlotCmd_(ClientData clientData,
		 Tcl_Interp * interp,
		 int argc,
		 char ** argv)
{
  char * options[] = { "Vdd" ,
			 "-zoom", "d",
			 NULL };
  
  ViewImage       *viewImagePtr;
  int             i, j;
  unsigned long   posPixel;
  int             zoom = 1;
  int             xpos, ypos;
  int             position;
  
  if (arg_init(interp, argc, argv, options, NULL))
    return TCL_OK;

  if (arg_get(0, &viewImagePtr, &xpos, &ypos) == TCL_ERROR)
    return TCL_ERROR;

  if (arg_get(1, &zoom) == TCL_ERROR)
    return TCL_ERROR;

  position = zoom*(ypos*viewImagePtr->lx + xpos);
  posPixel = get_color_intensity_(GREEN);
  for (i=0; i<zoom; i++)
    for (j=0; j<zoom; j++)
      viewImagePtr->data[position+j+viewImagePtr->lx*i] = posPixel;
  
/*  ViewWidReconfigureall_(char * viewImageName)*/
  return TCL_OK;
}

