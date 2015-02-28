/*
 * File: fixt_set.h
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: A collection of tasks to run as a unit
 */

#ifndef FIXT_SET_H_
#define FIXT_SET_H_

#include <stdarg.h>
#include "utlist.h"
#include "fixt_task.h"

struct fixt_set
{
	int ts_id; /* Id number of the set for logging purposes */
	struct fixt_task* ts_set_head; /* The head of the doubly linked list */

	/* Used privately by utlist */
	struct fixt_set* prev;
	struct fixt_set* next;
};

/*
 * Create a new task set comprised of multiple tasks. The first parameter is
 * the task set id. The second parameter is the number of following params.
 * The remaining parameters, in groups of three, represent task tuples
 * of the form (c, p, d). The size parameter must be a multiple of three.
 */
struct fixt_set* fixt_set_new(int, int, ...);
void fixt_set_del(struct fixt_set*);

#endif
