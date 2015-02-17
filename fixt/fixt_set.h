#ifndef FIXT_SET_H_
#define FIXT_SET_H_

#include <stdarg.h>
#include "../utlist.h"
#include "fixt_task.h"

struct fixt_set
{
	int ts_id;
	struct fixt_task* ts_set_head;

	/* Used privately by utlist */
	struct fixt_set* prev;
	struct fixt_set* next;
};

struct fixt_set* fixt_set_new(int, int, ...);
void fixt_set_del(struct fixt_set*);

#endif
