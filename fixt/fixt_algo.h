#ifndef FIXT_ALGO_H
#define FIXT_ALGO_H

#include <stdarg.h>
#include "fixt_hook.h"

#define FIXT_ALGO_BASE_PRIO 32 /* TODO determine max user prio */

extern struct fixt_task;

struct fixt_algo
{
	AlgoHook al_init;
	AlgoHook al_schedule;
	AlgoHook al_block;

	int al_preferred_policy;

	struct fixt_task* al_tasks_head;
	struct fixt_task* al_queue_head;

	/* For private use by utlist */
	struct fixt_algo* prev;
	struct fixt_algo* next;
};

struct fixt_algo* fixt_algo_new(AlgoHook i, AlgoHook s, AlgoHook r);
void fixt_algo_del(struct fixt_algo*);

void fixt_algo_add_task(fixt_task*);
void fixt_algo_add_all(fixt_task*);

void fixt_algo_init(struct fixt_algo*);
void fixt_algo_schedule(struct fixt_algo*);
void fixt_algo_run(struct fixt_algo*);
void fixt_algo_halt(struct fixt_algo*);

#endif
