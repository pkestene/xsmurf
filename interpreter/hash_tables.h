#ifndef __HASH_TABLES_H__
#define __HASH_TABLES_H__

int     GenDicGetUniqueEntryList (Tcl_Interp *,Tcl_HashTable *,
				  char **,Tcl_HashEntry ***);


int     GenErrorAppend           (Tcl_Interp *,...);
int     GenErrorMemoryAlloc      (Tcl_Interp *);
char *  GenErrorRandomMessage    ();

void ExtDicUnlinkStamp_(int stamp);
void Ext3DDicUnlinkStamp_(int stamp);
#include "../signal/signal.h"
#include "../wt2d/wt2d.h"
#include "../wt3d/wt3d.h"
#include "../stats/stats.h"
#include "../wt2d/skeleton.h"
#include "../image/image.h"
#include "../image/image3D.h"

int ExtDicImageListGet(Tcl_Interp * interp,char ** expr, 
		       ExtImage *** extImageListPtr);
int Ext3DDicImageListGet(Tcl_Interp *interp,char **expr, 
			 ExtImage3D   ***extImage3DListPtr);

void ExtMisSortImageList_(ExtImage ** list);      /* files_cmds.c */
int _RemoveShortChains_(ExtImage * downExtImage); /* ../wt2d/Chain.c */

void ExtDicStore  (char * name, ExtImage * image_ptr);  /* hash_tables.c */
void Ext3DDicStore(char * name, ExtImage3D * image_ptr);
void Ext3DsmallDicStore(char * name, ExtImage3Dsmall * image_ptr);

void store_image   (char  *name, Image *image_ptr);
void store_image3D (char  *name, Image3D *image_ptr);
void unstore_line_by_value (Line *line);
void store_signal_in_dictionary (char   *name, Signal *signal_ptr);
extern Signal   * get_signal (char *);

#endif
