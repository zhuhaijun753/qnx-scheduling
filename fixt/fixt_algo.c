/*
 * File: fixt_algo.c
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: State and logic common to all scheduling algorithms
 */

#include <semaphore.h>
#include "utlist.h"
#include "fixt_task.h"
#include "fixt_hook.h"
#include "fixt_algo.h"

#include "debug.h"

struct fixt_algo* fixt_algo_new(AlgoHook i, AlgoHook s, AlgoHook b, int policy)
{
	struct fixt_algo* algo = malloc(sizeof(*algo));
	algo->al_init = i;
	algo->al_schedule = s;
	algo->al_block = b;

	algo->al_preferred_policy = policy;

	algo->al_tasks_head = NULL;
	algo->al_queue_head = NULL;

	return algo;
}

void fixt_algo_del(struct fixt_algo* algo)
{
	struct fixt_task *elt, *tmp;
	DL_FOREACH_SAFE2(algo->al_tasks_head, elt, tmp, _at_next)
	{
		DL_DELETE2(algo->al_tasks_head, elt, _at_prev, _at_next);
		/* fixt should manage task lifetimes */
	}
	free(algo);
}

void fixt_algo_add_task(struct fixt_algo* algo, struct fixt_task* task)
{
	DL_APPEND2(algo->al_tasks_head, task, _at_prev, _at_next);
}

/**
 * BAD - DO NOT USE!
 * Concat is only meant to concatenate two disjoint lists within the
 * same "list namespace"
 */
void fixt_algo_copy_all(struct fixt_algo* algo, struct fixt_task* task)
{
	DL_CONCAT2(algo->al_tasks_head, task, _at_prev, _at_next);
}

void fixt_algo_init(struct fixt_algo* algo)
{
	dprintf("....fixt_algo_init\n");

	/* Change the main fixture thread's priority to the user max! */
	pthread_t self = pthread_self();
	pthread_setschedprio(self, FIXT_ALGO_BASE_PRIO);

	/* Change the main fixture thread's policy to fit the algo */
	algo->al_init(algo);

	/* Start up all component threads with the right policy choice */
	struct fixt_task* elt;
	DL_FOREACH2(algo->al_tasks_head, elt, _at_next)
	{
		fixt_task_run(elt, algo->al_preferred_policy, FIXT_ALGO_BASE_PRIO - 1);
	}

	dprintf("....fixt_algo_init (end)\n");
}

void fixt_algo_schedule(struct fixt_algo* algo)
{
	dprintf("....fixt_algo_schedule()\n");

	/* Defer scheduling to implementation */
	algo->al_schedule(algo);
}

void fixt_algo_run(struct fixt_algo* algo)
{
	dprintf("....fixt_algo_run()\n");

	/* Reprioritize all threads according to the queue ordering */
	struct fixt_task* elt;
	int prio = FIXT_ALGO_BASE_PRIO;
	DL_FOREACH2(algo->al_queue_head, elt, _aq_next)
	{
		fixt_task_set_prio(elt, --prio); /* In descending order */
	}

	/*
	 * Release the head task by posting its semaphore. Then, block the
	 * scheduling thread (this thread) so that the the head task may run.
	 * Return from al_block() at the time specified by algorithm specific
	 * logic. The different al_run implementations generally support preemptive
	 * or non-preemptive configurations.
	 */
	sem_t* sem_cont = fixt_task_get_sem_cont(algo->al_tasks_head);
	sem_post(sem_cont);
	algo->al_block(algo);

	dprintf("....fixt_algo_run() end\n");
}

void fixt_algo_halt(struct fixt_algo* algo)
{
	struct fixt_task* elt;
	DL_FOREACH2(algo->al_tasks_head, elt, _at_next)
	{
		fixt_task_stop(elt);
	}
}
