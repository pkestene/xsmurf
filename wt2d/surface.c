#include "wt2d_int.h"
#include "extremum.h"
#include "line.h"
#include "list.h"

#include "surface.h"

#ifndef update_mean
#define update_mean(mean, val) (mean=(mean*surface->nb_of_extrema+(val))/(surface->nb_of_extrema+line->size))
#endif

Surface *
create_surface ()
{
  Surface *surface;

  surface = (Surface *) malloc (sizeof (Surface));
  if (!surface)
    return NULL;
  surface -> line_lst = lst_create ();
  surface -> greater_scale = NO_SCALE;
  surface -> smaller_scale = NO_SCALE;
  surface -> nb_of_lines = 0;
  surface -> nb_of_extrema = 0;
  surface -> mass = 0.0;
  surface -> mean_arg = 0.0;

  return surface;
}

void
destroy_surface (Surface *surface)
{
  assert (surface);

  lst_destroy (surface -> line_lst);
  free (surface);
}


Surface *
add_line_to_surface (Line    *line,
		     Surface *surface)
{
  assert (line);
  assert (surface);
  assert (line -> scale < surface -> smaller_scale
	  || surface -> greater_scale == NO_SCALE);

  if (surface -> greater_scale == NO_SCALE)
      surface -> greater_scale = line -> scale;

  if (!lst_add (line, surface -> line_lst))
    return 0;
  surface -> smaller_scale = line -> scale;
  surface -> nb_of_lines ++;
  surface -> mass += line ->mass;
  update_mean (surface -> mean_arg, line -> mean_arg*line -> size);
  surface -> nb_of_extrema += line -> size;

  assert (surface -> greater_scale != NO_SCALE);
  assert (surface -> smaller_scale == line -> scale);
  assert (surface -> nb_of_lines >= 1);

  return surface;
}

Line *
smaller_scale_line (Surface *surface)
{
  assert (surface);

  return (Line *) lst_first (surface -> line_lst);
}
