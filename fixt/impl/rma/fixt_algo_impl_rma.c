#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "utlist.h"
#include "spin/spin.h"
#include "fixt/fixt_hook.h"
#include "fixt/fixt_algo.h"
#include "fixt/fixt_task.h"
#include "fixt_algo_impl_rma.h"

#include "log/log.h"
#include "debug.h"

#define POLICY_RMA SCHED_FIFO /* No preemption under RMA */

/*
 * This comparator compares two tasks under RMA and generates an ordering
 * such that tasks with small periods have high priority.
 */
static int rma_comparator(void*, void*);

/*
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
	log_func(3, "rma_schedule");

	/* Reset the queue so we can reschedule the tasks */
	algo->al_queue_head = NULL;

	/* Only consider tasks that are ready (r <= 0) */
	struct fixt_task *elt;
	DL_FOREACH2 (algo->al_tasks_head, elt, _at_next) {
		// TODO: error out if unschedulable
		if (fixt_task_get_r(elt) <= 0) {
			log_rchk(4, elt);
			DL_APPEND2(algo->al_queue_head, elt, _aq_prev, _aq_next);
		}
	}

	/* Pull the task with the shortest period to the head of the queue */
	DL_SORT2(algo->al_queue_head, (&rma_comparator), _aq_prev, _aq_next);

	log_fend(3, "rma_schedule");
}

/*
 * In RMA, tasks run to completion, and the scheduler does not preempt tasks.
 * Therefore, we can wait without timeout on the scheduled task.
 */
void fixt_algo_impl_rma_block(struct fixt_algo* algo)
{
	log_func(3, "rma_block");

	sem_t* sem_done = fixt_task_get_sem_done(algo->al_queue_head);
	sem_wait(sem_done);

	log_fend(3, "rma_block");
}

/*
 * Recalculate the r parameter across all tasks.
 *
 * If a task actually ran this iteration, then the head of the queue will
 * be that task. Adjust the r paramter by the general case. Calculate time
 * until ready for next period.
 *
 * If no task ran, then all tasks must have their r parameter normalized to
 * zero based upon the smallest r parameter in the current task pool.
 */
void fixt_algo_impl_rma_recalc(struct fixt_algo* algo)
{
	log_func(3, "rma_recalc");
	struct fixt_task* head = algo->al_queue_head;

	int delta;
	if (head) {
		/* Queue head chosen to run: Δ = c0,  ri' = pi - Δ + ri */
		log_hbef(4, head);

		delta = head->tk_c;
		head->tk_r = head->tk_p - delta + head->tk_r;

		log_haft(4, head);
	} else {
		/* Normalize all r parameters: Δ = min(ri) */
		delta = min_r(algo);
	}

	/* Tasks chosen to idle: ri' = ri - Δ */
	struct fixt_task* elt;
	DL_FOREACH2(algo->al_tasks_head, elt, _at_next) {
		/* Don't do this calculation on the queue head! */
		if (elt != head) {
			log_ibef(4, elt);

			elt->tk_r = elt->tk_r - delta;

			log_iaft(4, elt);
		}
	}
	log_fend(3, "rma_recalc");
}

struct fixt_algo* fixt_algo_impl_rma_new()
{
	AlgoHook al_init = &fixt_algo_impl_rma_init;
	AlgoHook al_schedule = &fixt_algo_impl_rma_schedule;
	AlgoHook al_block = &fixt_algo_impl_rma_block;
	AlgoHook al_recalc = &fixt_algo_impl_rma_recalc;

	return fixt_algo_new(al_init, al_schedule, al_block, al_recalc, POLICY_RMA);
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
