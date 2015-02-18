/*
 * File: fixt_set.c
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: A collection of tasks to run as a unit
 */

#include <stdarg.h>
#include <assert.h>
#include "utlist.h"
#include "fixt_task.h"
#include "fixt_set.h"

struct fixt_set* fixt_set_new(int id, int size, ...)
{
	assert(size % 3 == 0); /* We need groups of three to create tasks */

	struct fixt_set* set = malloc(sizeof(*set));
	set->ts_id = id;
	set->ts_set_head = NULL;

	va_list tuples;
	int tuple;
	struct fixt_task* task;

	va_start(tuples, size);
	for (tuple = 0; tuple < size; tuple += 3) /* Advance index by 3 */
	{
		int c = va_arg(tuples, int);
		int p = va_arg(tuples, int);
		int d = va_arg(tuples, int);

		task = fixt_task_new(c, p, d);
		DL_APPEND2(set->ts_set_head, task, _ts_prev, _ts_next);
	}
	va_end(tuples);

	return set;
}

void fixt_set_del(struct fixt_set* set)
{
	struct fixt_task *elt, *tmp;
	DL_FOREACH_SAFE2(set->ts_set_head, elt, tmp, _ts_next)
	{
		DL_DELETE2(set->ts_set_head, elt, _ts_prev, _ts_next);
		fixt_task_del(elt);
	}
	free(set);
}
