/*
 * File: fixt_algo.h
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: State and logic common to all scheduling algorithms
 */

#ifndef FIXT_ALGO_H
#define FIXT_ALGO_H

#include <stdarg.h>
#include <stdbool.h>
#include "fixt_hook.h"

#define FIXT_ALGO_BASE_PRIO 10 /* qconn port=8000 qconn_prio=10 */
#define FIXT_ALGO_MIN_PRIO 7
struct fixt_task;

struct fixt_algo
{
	AlgoHook al_init; /* Hook run to set the scheduler thread's policy */
	AlgoHook al_schedule; /* Hook run to organize the queue */
	AlgoHook al_block; /* Hook which blocks until the scheduler should resume */
	AlgoHook al_recalc; /* Hooks which updates bookeeping after a run */

	int al_preferred_policy; /* Scheduling policy for all new task threads */

	bool al_schedulable; /* Updated after fixt_algo_schedule() is run */

	struct fixt_task* al_tasks_head; /* List of tasks managed by this algo */
	struct fixt_task* al_queue_head; /* Ordering of tasks at any given time */

	/* For private use by utlist.h */
	struct fixt_algo* prev;
	struct fixt_algo* next;
};

/*
 * Create a new scheduling algorithm given the four implementation-specific
 * hooks and a preferred scheduling policy for all task threads.
 */
struct fixt_algo* fixt_algo_new(AlgoHook, AlgoHook, AlgoHook, AlgoHook, int policy);
void fixt_algo_del(struct fixt_algo*);

/*
 * Add a single task to the algorithms' accounting.
 */
void fixt_algo_add_task(struct fixt_algo*, struct fixt_task*);

/**
 * BAD - DO NOT USE!
 * Concat is only meant to concatenate two disjoint lists within the
 * same "list namespace"
 */
void fixt_algo_copy_all(struct fixt_algo*, struct fixt_task*);

/*
 * Initialize the scheduler and start component task threads.
 */
void fixt_algo_init(struct fixt_algo*);

/*
 * Determine which task should run next by sorting the queue.
 */
void fixt_algo_schedule(struct fixt_algo*);

/*
 * Reprioritize all threads given the queue order and release the top
 * task for execution. Also, recompute internal bookeeping logic.
 */
void fixt_algo_run(struct fixt_algo*);

/*
 * Stop all component threads.
 */
void fixt_algo_halt(struct fixt_algo*);

/*
 * Determines the minimum time until the next task becomes ready
 */
int fixt_algo_min_r(struct fixt_algo*);

#endif
