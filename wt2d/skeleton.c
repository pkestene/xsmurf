#include "extremum.h"
#include "line.h"
#include "list.h"
#include "surface.h"
#include "wt2d_int.h"

#include "skeleton.h"

Skeleton *
create_skeleton ()
{
  Skeleton *skeleton;

  skeleton = (Skeleton *) malloc (sizeof (Skeleton));
  if (!skeleton)
    return NULL;
  skeleton -> surface_lst = lst_create ();
  skeleton -> nb_of_surfaces = 0;
  skeleton -> ext_image_lst = lst_create ();
  skeleton -> nb_of_ext_images = 0;

  return skeleton;
}

void
destroy_skeleton (skeleton)
     Skeleton *skeleton;
{
  assert (skeleton);

  lst_destroy (skeleton -> surface_lst);
  lst_destroy (skeleton -> ext_image_lst);
  free (skeleton);
}


Skeleton *
add_surface_to_skeleton (surface, skeleton)
     Surface  *surface;
     Skeleton *skeleton;
{
  assert (surface);
  assert (skeleton);

  lst_add (surface, skeleton -> surface_lst);
  skeleton -> nb_of_surfaces ++;

  return skeleton;
}

void
remove_surface_from_skeleton (surface, skeleton)
     Surface  *surface;
     Skeleton *skeleton;
{
  ExtImage *ext_image;
  Line     *line;

  assert (surface);
  assert (skeleton);
  assert (lst_content_value (skeleton -> surface_lst, surface));

  /* a ameliorer. Tres tres crade */
  foreach (line, surface -> line_lst)
    {
      ext_image = line -> ext_image;

      lst_remove (line, ext_image -> line_lst);
      line -> ext_image -> nb_of_lines --;
    }

  lst_remove (surface, skeleton -> surface_lst);
  destroy_surface (surface);
  skeleton -> nb_of_surfaces --;
}

Skeleton *
add_ext_image_to_skeleton (ext_image, skeleton)
     ExtImage *ext_image;
     Skeleton *skeleton;
{
  assert (ext_image);
  assert (skeleton);

  lst_add (ext_image, skeleton -> ext_image_lst);
  skeleton -> nb_of_ext_images ++;

  return skeleton;
}

