#ifndef FIXT_ALGO_H
#define FIXT_ALGO_H

#include "fixt_hook.h"

extern struct fixt_task;

struct fixt_algo
{
	AlgoHook ah_init;
	AlgoHook ah_schedule;
	AlgoHook ah_run;

	struct fixt_task** tasks;
	struct fixt_task** taskq;
};

struct fixt_algo* fixt_algo_new(AlgoHook i, AlgoHook s, AlgoHook r);
void fixt_algo_del(struct fixt_algo*);

void fixt_algo_tasks(fixt_task**);

void fixt_algo_init(struct fixt_algo*);
void fixt_algo_schedule(struct fixt_algo*);
void fixt_algo_run(struct fixt_algo*);

#endif
