#ifndef __SKELETON_H__
#define __SKELETON_H__

#include "wt2d.h"
#include "surface.h"

/*typedef struct _skeleton_ Skeleton;*/

typedef struct Skeleton {
  List *surface_lst;
  int  nb_of_surfaces;
  List *ext_image_lst;
  int  nb_of_ext_images;
} Skeleton;

Skeleton * create_skeleton ();
void       destroy_skeleton (Skeleton *);
Skeleton * add_surface_to_skeleton (Surface *, Skeleton *);
Skeleton * add_ext_image_to_skeleton (struct ExtImage *, Skeleton *);

void construct_skeleton (Skeleton *);
void skeleton_to_chains (Skeleton *);

void remove_surface_from_skeleton (Surface *, Skeleton *);

void       display_skeleton (Skeleton *);

#endif /*__SKELETON_H__*/
