#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dm/evlist.h>
#include <dm/safe.h>

/* returns last node in linked list */
static struct dm_evlist* _evlist_get_last(struct dm_evlist *list)
{
  struct dm_evlist     *item;

  /* if caller passed us a NULL, return now */
  if(!list)
    return NULL;

  /* loop through to find the last item */
  item = list;
  while(item->next) {
    item = item->next;
  }
  return item;
}

struct dm_evlist* dm_evlist_append(struct dm_evlist *list, const char *event, void *data)
{
  struct dm_evlist     *last;
  struct dm_evlist     *new_item;

  new_item = malloc(sizeof(struct dm_evlist));
  if(new_item) {
    char *dupdata = strdup(event);
    if(dupdata && data) {
      new_item->next = NULL;
      new_item->event = dupdata;
	  new_item->data = data;
    }
    else {
      free(new_item);
      return NULL;
    }
  }
  else
    return NULL;

  if(list) {
    last = _evlist_get_last(list);
    last->next = new_item;
    return list;
  }

  /* if this is the first item, then new_item *is* the list */
  return new_item;
}

struct dm_evlist* dm_evlist_find(struct dm_evlist *list, const char *event)
{
	struct dm_evlist	*next;
	struct dm_evlist	*item;

	if (!list || !event)
		return NULL;

	item = list;
	do {
		if (strcmp(item->event, event) == 0)
			return item;
		next = item->next;
		item = next;
	} while(next);

	return NULL;
}

/*
struct dm_evlist* dm_evlist_free(struct dm_evlist *list)
{

}
*/

/* be nice and clean up resources */
void dm_evlist_free_all(struct dm_evlist *list)
{
  struct dm_evlist     *next;
  struct dm_evlist     *item;

  if(!list)
    return;

  item = list;
  do {
    next = item->next;
	safefree(item->event);
	item->data = NULL;
    free(item);
    item = next;
  } while(next);
}
