/*
 * File: fixt_algo.c
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: State and logic common to all scheduling algorithms
 */

#include <semaphore.h>
#include <limits.h>
#include <stdbool.h>
#include "utlist.h"
#include "fixt_task.h"
#include "fixt_hook.h"
#include "fixt_algo.h"
#include "spin/spin.h"

#include "debug.h"
#include "log/log.h"
#include "log/kernel_trace.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct fixt_algo* fixt_algo_new(AlgoHook i, AlgoHook s, AlgoHook b, AlgoHook r,
		int policy)
{
	struct fixt_algo* algo = malloc(sizeof(*algo));
	algo->al_init = i;
	algo->al_schedule = s;
	algo->al_block = b;
	algo->al_recalc = r;

	algo->al_preferred_policy = policy;

	algo->al_tasks_head = NULL;
	algo->al_queue_head = NULL;

	return algo;
}

void fixt_algo_del(struct fixt_algo* algo)
{
	struct fixt_task *elt, *tmp;
	DL_FOREACH_SAFE2(algo->al_tasks_head, elt, tmp, _at_next) {
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
	log_func(2, "fixt_algo_init");

	/* Change the main fixture thread's priority to the user max! */
	pthread_t self = pthread_self();
	pthread_setschedprio(self, FIXT_ALGO_BASE_PRIO);

	/* Change the main fixture thread's policy to fit the algo */
	algo->al_init(algo);

	/* Start up all component threads with the right policy choice */
	struct fixt_task* elt;
	DL_FOREACH2(algo->al_tasks_head, elt, _at_next) {
		fixt_task_run(elt, algo->al_preferred_policy, FIXT_ALGO_BASE_PRIO - 1);
	}

	log_fend(2, "fixt_algo_init");
}

void fixt_algo_schedule(struct fixt_algo* algo)
{
	log_func(2, "fixt_algo_schedule");

	/* Defer scheduling to implementation */
	k_log_s(2);
	algo->al_schedule(algo);

	/* See if our queue is schedulable */
	struct fixt_task* elt;
	bool schedulable = true;
	int avail_c;
	DL_FOREACH2(algo->al_queue_head, elt, _aq_next) {
		/* The time availalbe if elt was run next - based on deadline */
		avail_c = fixt_task_get_d(elt) + fixt_task_get_r(elt);
		if(elt == algo->al_queue_head) {
			/* If elt is the head, then we can Indiana Jones our tk_c */
			schedulable &= (fixt_task_completion_time(elt) <= avail_c);
		} else {
			/* Otherwise, tk_c < avail_c, because elt won't have avail_c next */
			schedulable &= (fixt_task_completion_time(elt) < avail_c);
		}
	}
	algo->al_schedulable = schedulable;

	k_log_e(2);
	log_fend(2, "fixt_algo_schedule");
}

/*
 * Release the head task by posting its semaphore. Then, block the
 * scheduling thread (this thread) so that the the head task may run.
 *
 * Return from al_block() at the time specified by algorithm specific
 * logic. The different al_run implementations generally support preemptive
 * or non-preemptive configurations.
 */
void fixt_algo_run(struct fixt_algo* algo)
{
	log_func(2, "fixt_algo_run");

	/* If no task needs to run, spin the scheduler until one is ready */
	if (!algo->al_queue_head) {
		log_msg(3, "[ Null Queue Head ]");
		spin_for(min_r(algo));
	} else {
		log_msg(3, "[ Non-Null Queue Head ]");

		/* Reprioritize all threads according to the queue ordering */
		struct fixt_task* elt;
		int prio = FIXT_ALGO_BASE_PRIO;
		DL_FOREACH2(algo->al_queue_head, elt, _aq_next) {
			fixt_task_set_prio(elt, --prio); /* In descending order */
		}

		/* Release the head task for execution - only if the task is waiting */
		sem_t* sem_cont = fixt_task_get_sem_cont(algo->al_queue_head);
		int sem_cont_value;
		sem_getvalue(sem_cont, &sem_cont_value);
		if(sem_cont_value == 0) { /* Post only if task is waiting! */
			sem_post(sem_cont);
		}

		algo->al_block(algo);
	}

	k_log_s(2);
	algo->al_recalc(algo);
	k_log_e(2);

	log_fend(2, "fixt_algo_run");
}

int min_r(struct fixt_algo* algo)
{
	int r = INT_MAX;
	struct fixt_task* elt;
	DL_FOREACH2(algo->al_tasks_head, elt, _at_next) {
		r = MIN(r, elt->tk_r);
	}
	return r;
}

void fixt_algo_halt(struct fixt_algo* algo)
{
	log_func(2, "fixt_algo_halt");

	struct fixt_task *elt, *tmp;
	DL_FOREACH_SAFE2(algo->al_tasks_head, elt, tmp, _at_next) {
		fixt_task_stop(elt);
		DL_DELETE2(algo->al_tasks_head, elt, _at_prev, _at_next);
	}

	log_fend(2, "fixt_algo_halt");
}
