#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "utlist.h"
#include "spin/spin.h"
#include "fixt/fixt_hook.h"
#include "fixt/fixt_algo.h"
#include "fixt/fixt_task.h"
#include "fixt_algo_impl_rma.h"

#define POLICY_RMA SCHED_FIFO

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
	/* Reset the queue so we can reschedule the tasks */
	algo->al_queue_head = NULL;

	/* Only consider tasks that are ready (r <= 0) */
	struct fixt_task *elt;
	DL_FOREACH(algo->al_tasks_head, elt)
	{
		if (fixt_task_get_r(elt) <= 0)
		{
			DL_APPEND(algo->al_queue_head, elt);
		}
	}

	/* Pull the task with the shortest period to the head of the queue */
	DL_SORT(algo->al_queue_head, (&rma_comparator));
}

/**
 * In RMA, tasks run to completion, and the scheduler does not preempt tasks.
 * Therefore, we can wait without timeout on the scheduled task.
 */
void fixt_algo_impl_rma_block(struct fixt_algo* algo)
{
	sem_t sem_task_wait = fixt_task_get_sem_cont(algo->al_queue_head);
	sem_wait(&sem_task_wait);

	/* Recalculate r parameter */
	struct fixt_task* head = algo->al_queue_head;
	int head_c = head->tk_c;

	/* Task chosen to run for head_c quanta: ri' = di - c0 + ri */
	head->tk_r = head->tk_d - head_c + head->tk_r;

	/* Tasks chosen to idle for head_c quanta: ri' = ri - c0 */
	struct fixt_task* elt;
	if (head->next)
	{
		DL_FOREACH(head->next, elt)
		{
			elt->tk_r = elt->tk_r - head_c;
		}
	}

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

	return task_l->tk_p - task_r->tk_p;
}
