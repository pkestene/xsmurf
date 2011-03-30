#ifndef _LIST_H_
#define _LIST_H_

/*typedef struct _list_ List;*/
typedef struct Element
{
  void           *value;
  struct Element *next;
  struct Element *prev;
} Element;

typedef struct List
{
  Element *first;
  Element *last;
  Element *current;
} List;


#define foreach(value,list) for(value=lst_first(list);\
				value;\
				value=lst_next(list))

List * lst_create ();
void   lst_destroy (List *);
List * lst_add (void *, List *);
void * lst_get (List *);
void * lst_remove (void *, List *);
void * lst_move (void *, List *, List *);
int    lst_is_empty (List *);

void * lst_set_current (void *, List *);
void * lst_current (List *);
void * lst_current_next (List *);
void * lst_first (List *);
void * lst_last (List *);
void * lst_next (List *);
void * lst_prev (List *);

#endif /*_LIST_H_*/
