#ifndef FIXT_ALGO_H
#define FIXT_ALGO_H

#include "hook.h"

extern struct fixt_task;

struct fixt_algo
{
	VoidHook ag_init;
	VoidHook ag_schedule;
	VoidHook ag_run;

	struct fixt_task* tasks;
	struct fixt_task* taskq;
};

struct fixt_algo* fixt_algo_new(VoidHook i, VoidHook s, VoidHook r);
void fixt_algo_del(struct fixt_algo*);

void fixt_algo_init(struct fixt_algo*);
void fixt_algo_schedule(struct fixt_algo*);
void fixt_algo_run(struct fixt_algo*);

#endif
