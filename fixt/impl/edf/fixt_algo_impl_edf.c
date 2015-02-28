/*
 * File: fixt_algo_impl_edf.c
 * Author: Steven Kroh
 * Date: 28 Feb 2015
 * Description: Implementation of fixt_algo for Earliest Deadline First
 */

#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "utlist.h"
#include "spin/spin.h"
#include "fixt/fixt_hook.h"
#include "fixt/fixt_algo.h"
#include "fixt/fixt_task.h"
#include "fixt_algo_impl_edf.h"

#include "log/log.h"

#define POLICY_EDF SCHED_FIFO /* No preemption under EDF */

/*
 * This comparator compares two tasks under EDF and generates an ordering
 * such that tasks with early deadlines have high priority.
 */
static int edf_comparator(void*, void*);

/*
 * EDF requires the fixture thread (self) and all child task threads to use a
 * FIFO policy. Since the scheduler manipulates thread priorities to align with
 * the queueing order, we don't actually have any user threads within the
 * same priority. Therefore, RR does not apply to our solution.
 *
 * From QNX: a thread selected to run continues executing until it:
 *  - voluntarily relinquishes control (e.g., it blocks)
 *  - is preempted by a higher-priority thread
 */
void fixt_algo_impl_edf_init(struct fixt_algo* algo)
{
	pthread_t self = pthread_self();
	pthread_setschedparam(self, POLICY_EDF, NULL);
}

void fixt_algo_impl_edf_schedule(struct fixt_algo* algo)
{
	log_func(3, "edf_schedule");

	/* Reset the queue so we can reschedule the tasks */
	algo->al_queue_head = NULL;

	/* Only consider tasks that are ready (r <= 0) */
	struct fixt_task *elt;
	DL_FOREACH2 (algo->al_tasks_head, elt, _at_next) {
		if (fixt_task_get_r(elt) <= 0) {
			log_rchk(4, elt);
			DL_APPEND2(algo->al_queue_head, elt, _aq_prev, _aq_next);
		}
	}

	/* Pull the task with the earliest deadline to the head of the queue */
	DL_SORT2(algo->al_queue_head, (&edf_comparator), _aq_prev, _aq_next);

	log_fend(3, "edf_schedule");
}

/*
 * In EDF, tasks do not necessarily run to completion. The scheduler preempts
 * user tasks at a period of EDF_PERIOD * SPIN_QUANTUM_WIDTH_MS. This is
 * possible because the scheduler always maintains the highest priority out of
 * any (non-QNX) task in our system.
 *
 * This means user tasks may only run when the scheduler thread is blocked.
 * We block the scheduler for EDF_PERIOD quanta by using a timed wait.
 */
void fixt_algo_impl_edf_block(struct fixt_algo* algo)
{
	log_func(3, "edf_block");

	sem_t* sem_done = fixt_task_get_sem_done(algo->al_queue_head);
	struct timespec abs_next;
	abs_next = spin_abstime_in_quanta(EDF_PERIOD, EDF_JITTER);

	if(sem_timedwait(sem_done, &abs_next) == 0) {
		log_msg(4, "[ Scheduler Resume b/c Task Completed ]");
	} else if(errno == ETIMEDOUT) {
		log_msg(4, "[ Scheduler Preemption ]");
	}

	log_fend(3, "edf_block");
}

/*
 * Recalculate the r parameter across all tasks.
 *
 * If a task actually ran this iteration, then the head of the queue will
 * be that task. If the queue head has completed its execution, then calculate
 * time until ready for next period. Otherwise, all tasks were ready
 * EDF_PERIOD quanta ago.
 *
 * If no task ran, then all tasks must have their r parameter normalized to
 * zero based upon the smallest r parameter in the current task pool.
 */
void fixt_algo_impl_edf_recalc(struct fixt_algo* algo)
{
	log_func(3, "edf_recalc");
	struct fixt_task* head = algo->al_queue_head;

	int delta; /* The number of quanta elapsed since last run */
	if (head) {
		log_hbef(4, head);

		/* Queue head chosen to run: Δ = scheduler period */
		delta = EDF_PERIOD;
		head->tk_a += delta; /* Add one to the task's accumlated time */

		if(fixt_task_completion_time(head) > 0) {
			/* Still execution time left: task is still ready */
			head->tk_r = head->tk_r - delta;
		} else {
			/* No execution time left: calculate time til next period */
			head->tk_r = head->tk_p - delta + head->tk_r;
			head->tk_a = 0; /* Reset accumulated time */
		}

		log_haft(4, head);
	} else {
		/* Normalize all r parameters: Δ = min(ri) */
		delta = fixt_algo_min_r(algo);
	}

	/* Tasks chosen to idle: r' = r - Δ */
	struct fixt_task* elt;
	DL_FOREACH2(algo->al_tasks_head, elt, _at_next) {
		/* Don't do this calculation on the queue head! */
		if (elt != head) {
			log_ibef(4, elt);

			elt->tk_r = elt->tk_r - delta;

			log_iaft(4, elt);
		}
	}
	log_fend(3, "edf_recalc");
}

struct fixt_algo* fixt_algo_impl_edf_new()
{
	AlgoHook al_init = &fixt_algo_impl_edf_init;
	AlgoHook al_schedule = &fixt_algo_impl_edf_schedule;
	AlgoHook al_block = &fixt_algo_impl_edf_block;
	AlgoHook al_recalc = &fixt_algo_impl_edf_recalc;

	return fixt_algo_new(al_init, al_schedule, al_block, al_recalc, POLICY_EDF);
}

static int edf_comparator(void* l, void* r)
{
	struct fixt_task* task_l = (struct fixt_task*) l;
	struct fixt_task* task_r = (struct fixt_task*) r;
	/*
	 * If the left task has an earlier than the right task, then the
	 * function will return a negative number, etc.
	 */
	return fixt_task_remaining_time(task_l) - fixt_task_remaining_time(task_r);
}
