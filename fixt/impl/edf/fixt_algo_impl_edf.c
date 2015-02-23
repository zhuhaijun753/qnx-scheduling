#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "utlist.h"
#include "spin/spin.h"
#include "fixt/fixt_hook.h"
#include "fixt/fixt_algo.h"
#include "fixt/fixt_task.h"
#include "fixt_algo_impl_edf.h"

#define POLICY_EDF SCHED_FIFO

/*
 * This comparator compares two tasks under EDF and generates an ordering
 * such that tasks with earliest deadlines have higher priority.
 */
static int edf_comparator(void*, void*);

void fixt_algo_impl_edf_init(struct fixt_algo* algo)
{
	pthread_t self = pthread_self();
	pthread_setschedparam(self, POLICY_EDF, NULL);
}

void fixt_algo_impl_edf_schedule(struct fixt_algo* algo)
{
	/* Reset the queue so we can reschedule the tasks */
	algo->al_queue_head = NULL;

	/* Only consider tasks that are ready (r <= 0) */
	struct fixt_task *elt;
	DL_FOREACH2 (algo->al_tasks_head, elt, _at_next)
	{
		if (fixt_task_get_r(elt) <= 0)
		{
			DL_APPEND2(algo->al_queue_head, elt, _aq_prev, _aq_next);
		}
	}

	/* Pull the task with the earliest deadline to the head of the queue */
	DL_SORT2(algo->al_queue_head, (&edf_comparator), _aq_prev, _aq_next);
}


void fixt_algo_impl_edf_block(struct fixt_algo* algo)
{
	sem_t* sem_task_wait = fixt_task_get_sem_cont(algo->al_queue_head);
	sem_wait(sem_task_wait);

	/* Recalculate r parameter */
	struct fixt_task* head = algo->al_queue_head;
	int head_c = head->tk_c;

	/* Task chosen to run for head_c quanta: ri' = di - c0 + ri */
	head->tk_r = head->tk_d - head_c + head->tk_r;

	/* Tasks chosen to idle for head_c quanta: ri' = ri - c0 */
	struct fixt_task* elt;
	if (head->_at_next)
	{
		DL_FOREACH2(head->_at_next, elt, _at_next)
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

struct fixt_algo* fixt_algo_impl_edf_new()
{
	AlgoHook al_init = &fixt_algo_impl_edf_init;
	AlgoHook al_schedule = &fixt_algo_impl_edf_schedule;
	AlgoHook al_block = &fixt_algo_impl_edf_block;

	return fixt_algo_new(al_init, al_schedule, al_block, POLICY_EDF);
}

static int edf_comparator(void* l, void* r)
{
	struct fixt_task* task_l = (struct fixt_task*) l;
	struct fixt_task* task_r = (struct fixt_task*) r;
	/*
	 * If the left task has an earlier deadline the right task, then the
	 * function will return a negative number, etc.
	 */
	return task_l->tk_d - task_r->tk_d;
}
