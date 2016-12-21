#ifndef _DM_EVENT_LIST_H_
#define _DM_EVENT_LIST_H_

struct dm_evlist {
	char *event;
	void *data;
	struct dm_evlist *next;
};

struct dm_evlist* dm_evlist_append(struct dm_evlist *list, const char *event, void *data);
struct dm_evlist* dm_evlist_find(struct dm_evlist *list, const char *event);
void dm_evlist_free_all(struct dm_evlist *list);
#endif
