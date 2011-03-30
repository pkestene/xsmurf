/*
 * list.c --
 *
 *   Copyright 1999 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: list.c,v 1.3 1999/01/28 11:05:38 decoster Exp $
 */

#include "list.h"
#include <assert.h>
#include <stdlib.h>

typedef struct Element
{
  void           *value;
  struct Element *next;
  struct Element *prev;
} Element;

typedef struct _list_
{
  Element *first;
  Element *last;
  Element *current;
} _list_;

/*
 * _create_element_ -- PRIVATE
 *
 *   Allocate memory and initialize the fields of an Element structure (except
 * for the value field).
 *
 * Arguments:
 *   None.
 *
 * Return value:
 *   Pointer to the new Element. 0 if allocation fails.
 */

static Element *
_create_element_()
{
  Element *element;

  element = (Element *) malloc (sizeof (Element));
  if (!element) {
    return 0;
  }

  element->next = 0;
  element->prev = 0;

  return element;
}


/*
 * _init_element_ -- PRIVATE
 *
 *   Allocate memory and initialize the fields of an Element structure.
 *
 * Arguments:
 *   void * - The value of the Element. Must be non 0.
 *
 * Return value:
 *   Pointer to the new Element. 0 if allocation fails.
 */

static Element *
_init_element_ (void *value)
{
  Element *element;

  assert (value);

  element = _create_element_ ();
  if (!element) {
    return 0;
  }
  element->value = value;

  return element;
}


/*
 * _destroy_element_ -- PRIVATE
 *
 *   Free all aloccated memory of an Element.
 *
 * Arguments:
 *   Element * - The Element.
 *
 * Return value:
 *   None.
 */

static void
_destroy_element_ (Element *element)
{
  free (element);
}


/*-----------------------------------------------*/

/*
 * Allocation of the memory. Init as an empty list.
 */
List *
lst_create ()
{
  List *list;

  list = (List *) malloc (sizeof (List));
  if (!list) {
    return 0;
  }

  list->first = 0;
  list->last = 0;
  list->current = 0;

  return list;
}


/*
 * Free the memory of all the elements (but not the memory of its value.
 * Free the memory of the list.
 */
void
lst_destroy (List *list)
{
  Element *element, *new_element;;

  if (list) {
    element = list->first;
    while (element) {
      new_element = element->next;
      _destroy_element_ (element);
      element = new_element;
    }
  }
  free (list);
}


/*
 *  Create an element and add it to the list. Doesn't check if an 
 * element with the same value exists on the list.
 */
List *
lst_add (void *value,
	 List *list)
{
  Element *element;

  assert (value);
  assert (list);

  element = _init_element_ (value);
  if (!element) {
    return 0;
  }

  element->next = list->first;
  if (list->first) {
    list->first->prev = element;
  } else {
    list->last = element;
  }
  element->prev = 0;
  list->first = element;

  return list;
}


/*
 * lst_add_beg --
 *
 *  Create an element and add it at the beginning of the list. Doesn't
 * check if an element with the same value exists on the list.
 */

List *
lst_add_beg (void *value,
	     List *list)
{
  Element *element;

  assert (value);
  assert (list);

  element = _init_element_ (value);
  if (!element) {
    return 0;
  }

  element->next = list->first;
  if (list->first) {
    list->first->prev = element;
  } else {
    list->last = element;
  }
  element->prev = 0;
  list->first = element;

  return list;
}


/*
 * lst_add_end --
 *
 *  Create an element and add it at the end of the list. Doesn't
 * check if an element with the same value exists on the list.
 */

List *
lst_add_end (void *value,
	     List *list)
{
  Element *element;

  assert (value);
  assert (list);

  element = _init_element_ (value);
  if (!element) {
    return 0;
  }

  element->prev = list->last;
  if (list->last) {
    list->last->next = element;
  } else {
    list->first = element;
  }
  element->next = 0;
  list->last = element;

  return list;
}


/*
 * lst_content_value --
 *
 *   Check if there is an element with a given value in the list.
 */

void *
lst_content_value (List *list,
		   void *value)
{
  void    *tmp_value;

  assert (list);
  assert (value);

  if (lst_is_empty (list)) {
    return 0;
  }

  foreach (tmp_value, list) {
    if (tmp_value == value) {
      return value;
    }
  }

  return 0;
}


/*
 * lst_get --
 *
 *   Return the value of the first element of the list. Remove this element.
 */

void *
lst_get (List *list)
{
  Element *element;
  void    *value;

  assert (list);

  if (!list->last) {
    return 0;
  }

  element = list->last;
  if (list->last->prev) {
    list->last->prev->next = 0;
  }
  list->last = list->last->prev;
  
  value = element->value;
  _destroy_element_ (element);

  if (!list->last) {
    list->first = 0;
  }

  return value;
}


/*
 * lst_remove --
 *
 *   Search the element that has a given value. If it exists, it is removed
 * and its value is returned. Otherwise 0 is return.
 */

void *
lst_remove (void *value,
	    List *list)
{
  Element *element;

  assert (value);
  assert (list);

  element = list->first;
  if (!element) {
    return 0;
  }
  while ((element->value != value) && element->next) {
    element = element->next;
  } 
  if (element->value != value) {
    return 0;
  }

  if (list ->last == element) {
    list->last = list->last->prev;
  }

  if (element->next) {
    element->next->prev = element->prev;
  }
  if (element->prev) {
    element->prev->next = element->next;
  } else {
    list->first = element->next;
  }
  element->next = 0;
  element->prev = 0;

  value = element->value;
  _destroy_element_ (element);

  if (!list->first) {
    list->last = 0;
  }

  return value;
}



/*
 * lst_length --
 *
 * Return List length.
 * 
 */

int
lst_length (List *list)
{
  Element *element;
  int i=0;

  assert (list);

  element = list->first;
  if (!element) {
    return 0;
  }

  while (element) {
    i++;
    element = element->next;
  } 

  return i;
}



/*
 * lst_move --
 */

void *
lst_move (void *value,
	  List *src_list,
	  List *res_list)
{
  Element *element;

  assert (value);
  assert (src_list);
  assert (res_list);

  element = src_list->first;
  while ((element->value != value) && element->next) {
    element = element->next;
  }
  if (element->value != value) {
    return 0;
  }

  if (src_list ->last == element) {
    src_list->last = src_list->last->prev;
  }

  if (element->next) {
    element->next->prev = element->prev;
  }
  if (element->prev) {
    element->prev->next = element->next;
  } else {
    src_list->first = element->next;
  }

  element->next = res_list->first;
  if (res_list->first){
    res_list->first->prev = element;
  }
  element->prev = 0;
  res_list->first = element;

  if (!src_list->first) {
    src_list->last = 0;
  }

  return value;
}


/*
 * lst_is_empty --
 */

int
lst_is_empty (List *list)
{
  assert (list);

  if (list->first) {
    return 0;
  } else {
    return 1;
  }
}


/*
 * lst_set_current --
 */

void *
lst_set_current (void *value,
		 List *list)
{
  Element *element;

  assert (list);
  assert (value);

  element = list->first;
  while ((element->value != value) && element->next) {
    element = element->next;
  }
  if (element->value != value) {
    return 0;
  }

  list->current = element;

  return value;
}


/*
 * lst_current --
 */

void *
lst_current (List *list)
{
  assert (list);

  return list->current->value;
}


/*
 * lst_get_next --
 */
void *
lst_get_next (List *list)
{
  assert (list);

  if (list->current && list->current->next) {
    return list->current->next->value;
  } else {
    return 0;
  }
}

/*
 * lst_get_prev --
 */
void *
lst_get_prev (List *list)
{
  assert (list);

  if (list->current && list->current->prev) {
    return list->current->prev->value;
  } else {
    return 0;
  }
}

/*
 * lst_first --
 */

void *
lst_first (List *list)
{
  assert (list);

  if (list->first) {
    list->current  = list->first;
    return list->current->value;
  } else {
    return 0;
  }
}

/*
 * lst_get_first --
 */
void *
lst_get_first (List *list)
{
  assert (list);

  if (list->first) {
    return list->first->value;
  } else {
    return 0;
  }

}

/*
 * lst_last --
 */

void *
lst_last (List *list)
{
  assert (list);

  list->current  = list->last;
  if (list->current) {
    return list->current->value;
  } else {
    return 0;
  }
}


/*
 * lst_next --
 */

void *
lst_next (List *list)
{
  assert (list);

  if (list->current && list->current->next) {
    list->current = list->current->next;
    return list->current->value;
  } else {
    return 0;
  }
}


/*
 * lst_prev --
 */

void *
lst_prev (List *list)
{
  assert (list);

  if (list->current && list->current->prev) {
    list->current = list->current->prev;
    return list->current->value;
  } else {
    return 0;
  }
}

