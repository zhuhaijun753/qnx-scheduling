#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "utlist.h"
#include "spin/spin.h"
#include "fixt/fixt_hook.h"
#include "fixt/fixt_algo.h"
#include "fixt/fixt_task.h"
#include "fixt_algo_impl_rma.h"

#include "debug.h"

#define POLICY_RMA SCHED_FIFO /* No preemption under RMA */

/*
 * This comparator compares two tasks under RMA and generates an ordering
 * such that tasks with small periods have high priority.
 */
static int rma_comparator(void*, void*);

/**
 * RMA requires the fixture thread (self) to use a FIFO policy.
 *
 * From QNX: a thread selected to run continues executing until it:
 *  - voluntarily relinquishes control (e.g., it blocks)
 *  - is preempted by a higher-priority thread
 */
void fixt_algo_impl_rma_init(struct fixt_algo* algo)
{
	pthread_t self = pthread_self();
	pthread_setschedparam(self, POLICY_RMA, NULL);
}

void fixt_algo_impl_rma_schedule(struct fixt_algo* algo)
{
	dprintf("......_rma_schedule()\n");

	/* Reset the queue so we can reschedule the tasks */
	algo->al_queue_head = NULL;

	/* Only consider tasks that are ready (r <= 0) */
	struct fixt_task *elt;
	DL_FOREACH2 (algo->al_tasks_head, elt, _at_next)
	{
		if (fixt_task_get_r(elt) <= 0)
		{
			dprintf("........ rchk (%d, %d, %d): %d\n", elt->tk_c, elt->tk_p,
					elt->tk_d, elt->tk_r);
			DL_APPEND2(algo->al_queue_head, elt, _aq_prev, _aq_next);
		}
	}

	/* Pull the task with the shortest period to the head of the queue */
	DL_SORT2(algo->al_queue_head, (&rma_comparator), _aq_prev, _aq_next);

	dprintf("......_rma_schedule() end\n");
}

/**
 * In RMA, tasks run to completion, and the scheduler does not preempt tasks.
 * Therefore, we can wait without timeout on the scheduled task.
 */
void fixt_algo_impl_rma_block(struct fixt_algo* algo)
{
	dprintf("......_rma_block()\n");

	sem_t* sem_done = fixt_task_get_sem_done(algo->al_queue_head);
	sem_wait(sem_done);

	/* Recalculate r parameter */
	struct fixt_task* head = algo->al_queue_head;
	int head_c = head->tk_c;

	dprintf("........ head (%d, %d, %d) -> ", head->tk_c, head->tk_p, head->tk_r);
	/* Queue head chosen to run for head_c quanta: ri' = di - c0 + ri */
	head->tk_r = head->tk_d - head_c + head->tk_r;
	dprintf("(%d, %d, %d)\n", head->tk_c, head->tk_p, head->tk_r);

	/* Tasks chosen to idle for head_c quanta: ri' = ri - c0 */
	struct fixt_task* elt;
	DL_FOREACH2(algo->al_tasks_head, elt, _at_next)
	{
		if (elt != head)
		{ /* Don't do this calculation on the queue head! */
			dprintf("........ idle (%d, %d, %d) -> ", elt->tk_c, elt->tk_p,
					elt->tk_r);

			elt->tk_r = elt->tk_r - head_c;

			dprintf("(%d, %d, %d)\n", elt->tk_c, elt->tk_p, elt->tk_r);
		}
	}

	dprintf("......_rma_block() end\n");

	/*
	 * TODO: for a preemptive application:
	 *
	 *  struct timespec abs_timeout = spin_abstime_in_quanta(1);
	 *  sem_timedwait_monotonic(&sem_wait, abs_timeout);
	 */
}

struct fixt_algo* fixt_algo_impl_rma_new()
{
	AlgoHook al_init = &fixt_algo_impl_rma_init;
	AlgoHook al_schedule = &fixt_algo_impl_rma_schedule;
	AlgoHook al_block = &fixt_algo_impl_rma_block;

	return fixt_algo_new(al_init, al_schedule, al_block, POLICY_RMA);
}

static int rma_comparator(void* l, void* r)
{
	struct fixt_task* task_l = (struct fixt_task*) l;
	struct fixt_task* task_r = (struct fixt_task*) r;
	/*
	 * If the left task has a smaller period than the right task, then the
	 * function will return a negative number, etc.
	 */
	return task_l->tk_p - task_r->tk_p;
}
