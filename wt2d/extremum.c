#include "wt2d_int.h"

Extremum *
create_extremum ()
{
  Extremum *extremum;

  extremum = (Extremum *) malloc (sizeof (Extremum));
  extremum -> prev = NULL;
  extremum -> next = NULL;
  extremum -> up   = NULL;
  extremum -> down = NULL;

  return extremum;
}

void
destroy_extremum (extremum)
     Extremum *extremum;
{
  free (extremum);
}

void
copy_extremum (Extremum *extremum_in, Extremum *extremum_out)
{
  /* we assume that extremun_out has already been created by
     create_extremum or just simply with malloc ... */
  extremum_out->mod  = extremum_in->mod;
  extremum_out->arg  = extremum_in->arg;
  extremum_out->pos  = extremum_in->pos;
  extremum_out->line = extremum_in->line;
}
