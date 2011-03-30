#include <stdlib.h>
#include "wt2d.h"
#include "line.h"
#include "extremum.h"
#include "skeleton.h"
#include "surface.h"
#include "list.h"
#include <assert.h>

#define TRUE 1
#define FALSE 0

static int _proxi_ (Line *, Line *);

typedef struct _linkage_block_
{
  Line    *line;
  Surface *surface;
  List    *candidate_line_lst;
  int     nb_of_candidates;
} _linkage_block_;

static _linkage_block_ *
init_linkage_block_ (Line    *line,
		     Surface *surface)
{
  _linkage_block_ *linkage_block;

  assert (line);
  assert (surface);

  linkage_block = (_linkage_block_ *) malloc (sizeof (_linkage_block_));
  if (!linkage_block)
    return 0;

  linkage_block -> candidate_line_lst = lst_create ();
  linkage_block -> line = line;
  linkage_block -> surface = surface;
  linkage_block -> nb_of_candidates = 0;

  return linkage_block;
}

static void
_destroy_linkage_block_ (_linkage_block_ *linkage_block)
{
  assert (linkage_block);

  lst_destroy (linkage_block -> candidate_line_lst);
  free (linkage_block);
}

static _linkage_block_ *
_add_line_to_linkage_block_ (Line *line_to_add,
			     Line *linkage_block_line,
			     List *linkage_block_lst)
{
  _linkage_block_ *linkage_block;

  assert (line_to_add);
  assert (linkage_block_line);
  assert (linkage_block_lst);
  assert (!lst_is_empty (linkage_block_lst));
  assert (line_to_add -> scale < linkage_block_line -> scale);

  for (linkage_block = lst_first (linkage_block_lst);
       linkage_block && linkage_block -> line != linkage_block_line;
       linkage_block = lst_next (linkage_block_lst));
  if (linkage_block && linkage_block -> line == linkage_block_line)
    {
      lst_add (line_to_add, linkage_block -> candidate_line_lst);
      linkage_block -> nb_of_candidates ++;
    }

  return linkage_block;
}

/* --------------------------- */
static int
_dist_ (pos1, pos2, lx)
     int pos1;
     int pos2;
     int lx;
{
  int dx = ( pos1 % lx ) - ( pos2 % lx );
  int dy = ( pos1 / lx ) - ( pos2 / lx );

  return dx * dx + dy * dy;
}

static int
_is_near_ (extremum1, line)
     Extremum *extremum1;
     Line     *line;
{
  Extremum *extremum2;

  assert (extremum1);
  assert (line);

  foreach (extremum2, line -> ext_lst)
    if (_dist_ (extremum1 ->pos, extremum2 -> pos, line -> lx) <= 2)
      return TRUE;
  return FALSE;
}

static int
_proxi_ (line1, line2)
     Line *line1;
     Line *line2;
{
  Extremum *extremum1;
  int      proxi = 0;

  assert (line1);
  assert (line2);

  foreach (extremum1, line1 -> ext_lst)
    if (_is_near_ (extremum1, line2))
      proxi ++;

  return proxi;
}

Line *
find_nearest_line (line, prev_line_lst)
     Line *line;
     List *prev_line_lst;
{
  Line *prev_line;
  Line *nearest_line = NULL;
  int  best_condition = 0;
  int  condition = 0;

  assert (line);
  assert (prev_line_lst);

  foreach (prev_line, prev_line_lst)
    {
      condition = _proxi_ (line, prev_line);
      if (condition > best_condition)
	{
	  best_condition = condition;
	  nearest_line = prev_line;
	}
    }

  return nearest_line;
}

/* 
 * This function construct the skeleton of a WT study. The scales are
 * scan from the highest to the smallest. Each scale _must_ have been
 * horizontaly chained.
 */
void
construct_skeleton (skeleton)
     Skeleton *skeleton;
{
  ExtImage        *ext_image;             /* Current scale to link to
					     the skeleton. */
  ExtImage        *prev_ext_image = NULL; /* Last scale linked. It's
					     the immediatly highest
					     scale than the current
					     scale. */
  Line            *line;
  Line            *nearest_line;
  Surface         *surface;
  List            *linkage_block_lst;
  _linkage_block_ *linkage_block;

  assert (skeleton);

  foreach (ext_image, skeleton -> ext_image_lst)
    {
      if (!prev_ext_image) /* highest scale. */
	{
	  foreach (line, ext_image -> line_lst)
	    {
	      surface = create_surface ();
	      add_line_to_surface (line, surface);
	      add_surface_to_skeleton (surface, skeleton);
	    }
	}
      else /* other scales. */
	{
	  /*
	   * Initialisation of the _linkage_block_ list with the end
	   * line of each surface of the skeleton. These lines belong
	   * to the previous scale.
	   */
	  linkage_block_lst = lst_create ();
	  foreach (surface, skeleton -> surface_lst)
	    {
	      line = smaller_scale_line (surface);
	      linkage_block = init_linkage_block_ (line, surface);
	      lst_add (linkage_block, linkage_block_lst);
	    }

	  /*
	   * Search of the upper nearest line for every line from the
	   * current scale.
	   */
	  foreach (line, ext_image -> line_lst)
	    {
	      nearest_line = find_nearest_line (line,
						prev_ext_image -> line_lst);
	      if (nearest_line)
		  linkage_block =
		    _add_line_to_linkage_block_ (line, nearest_line,
						 linkage_block_lst);

	      if (!nearest_line || (nearest_line && !linkage_block))
		{
		  surface = create_surface ();
		  add_line_to_surface (line, surface);
		  add_surface_to_skeleton (surface, skeleton);
		}
	    }

	  /*
	   * Choose 1 line from each linkage_block, and create the
	   * link. If a linkage_block is empty close the surface.
	   */
	  foreach (linkage_block, linkage_block_lst)
	    {
	      if (linkage_block -> nb_of_candidates == 0)
		remove_surface_from_skeleton (linkage_block -> surface,
					      skeleton);
	      else
		{
		  nearest_line = 
		    find_nearest_line (linkage_block -> line,
				       linkage_block -> candidate_line_lst);
		  add_line_to_surface (nearest_line,
				       linkage_block -> surface);
		}
	    }

	  /*
	   * Now we create a surface for each line that is not already linked
	   * to a surface.
	   */
	  foreach (line, ext_image -> line_lst)
	    {
	      int is_line_in_surface = 0;
	      foreach (surface, skeleton -> surface_lst)
		if (line == smaller_scale_line (surface))
		  {
		    is_line_in_surface = 1;
		    continue;
		  }
	      if (!is_line_in_surface)
		{
		  surface = create_surface ();
		  add_line_to_surface (line, surface);
		  add_surface_to_skeleton (surface, skeleton);
		}
	    }

	  /* Complete destruction of the list of _linkage_block */
	  foreach (linkage_block, linkage_block_lst)
	    _destroy_linkage_block_ (linkage_block);
	  lst_destroy (linkage_block_lst);
	}
      prev_ext_image = ext_image;
    }
}

void
skeleton_to_chains (skeleton)
     Skeleton *skeleton;
{
  ExtImage *ext_image;
  Line     *line;
  Surface  *surface;

  assert (skeleton);

  foreach (ext_image, skeleton -> ext_image_lst)
    {
      ext_image -> chainNb = 0;
      ext_image -> chain = (Extremum **) malloc (sizeof (Extremum *)
						 *ext_image -> nb_of_lines);
    }

  foreach (surface, skeleton -> surface_lst)
    {
      foreach (line, surface -> line_lst)
	{
	  Extremum *extremum;
	  Extremum *new_extremum;
	  
	  ext_image = line -> ext_image;
	  
	  extremum = lst_first (line -> ext_lst);;
	  ext_image -> chain[ext_image -> chainNb] = extremum;
	  line -> ext_chain = ext_image -> chain[ext_image -> chainNb];
	  ext_image -> chainNb++;
	  new_extremum = lst_next (line -> ext_lst);
	  for (;
	       new_extremum;
	       new_extremum = lst_next (line -> ext_lst))
	    {
	      new_extremum -> prev = extremum;
	      new_extremum -> next = NULL;
	      extremum -> next = new_extremum;
	      extremum = new_extremum;
	    }
	}
    }
}
