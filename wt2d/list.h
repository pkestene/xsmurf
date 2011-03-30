/*
 * list.h --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: list.h,v 1.2 1999/01/27 19:14:51 decoster Exp $
 */

#ifndef _LIST_H_
#define _LIST_H_

typedef struct _list_ List;

#define foreach(value,list) for(value=lst_first(list);\
				value;\
				value=lst_next(list))
#define lst_is_empty2(list) (!list->first)

List * lst_create ();
void   lst_destroy (List *);
List * lst_add (void *, List *);
List * lst_add_beg (void *, List *);
List * lst_add_end (void *, List *);
void * lst_content_value (List *, void *);
void * lst_get (List *);
void * lst_remove (void *, List *);
int    lst_length (List *);
void * lst_move (void *, List *, List *);
int    lst_is_empty (List *);

void * lst_set_current (void *, List *);
void * lst_current (List *);
void * lst_get_next (List *);
void * lst_get_prev (List *);
void * lst_first (List *);
void * lst_get_first(List *);
void * lst_last (List *);
void * lst_next (List *);
void * lst_prev (List *);

#endif /*_LIST_H_*/

