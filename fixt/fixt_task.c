/*
 * File: fixt_task.c
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: Tasking structure containing bookeeping logic and state
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "spin/spin.h"
#include "fixt_task.h"

#include "log/log.h"
#include "log/kernel_trace.h"

/*
 * Value sent over the poison pill pipe indicating a thread should end
 * as soon as possible
 */
static const int POISON_PILL = 1;

/*
 * The routine which is run as the task in a new thread
 */
static void* fixt_task_routine(void*);

struct fixt_task* fixt_task_new(int id, int c, int p, int d)
{
	struct fixt_task* task = malloc(sizeof *task);
	task->tk_id = id;

	task->tk_a = 0; /* To start, a task has run for 0 quanta */
	task->tk_c = c;
	task->tk_p = p;
	task->tk_d = d;
	task->tk_r = 0; /* To start, all tasks are ready */
	task->tk_routine = &fixt_task_routine;

	/* OOPS - we should find a better way to do lists */
	task->_ts_prev = NULL;
	task->_ts_next = NULL;
	task->_at_prev = NULL;
	task->_at_next = NULL;
	task->_aq_prev = NULL;
	task->_aq_next = NULL;

	return task;
}

sem_t* fixt_task_run(struct fixt_task* task, int policy, int prio)
{
	/* Every time a task is run from scratch, re-init tk_a and tk_r */
	task->tk_a = 0;
	task->tk_r = 0;

	pipe(task->tk_poison_pipe);
	/* Set to nonblocking. A thread join is used to sync threads instead */
	fcntl(task->tk_poison_pipe[0], F_SETFL, O_NONBLOCK);
	fcntl(task->tk_poison_pipe[1], F_SETFL, O_NONBLOCK);

	task->tk_sem_cont = malloc(sizeof(*task->tk_sem_cont));
	sem_init(task->tk_sem_cont, 0, 0); /* First sem_wait blocks */

	task->tk_sem_done = malloc(sizeof(*task->tk_sem_done));
	sem_init(task->tk_sem_done, 0, 0); /* First sem_wait blocks */

	/*
	 * The thread should have the appropriate scheduling policy when it
	 * starts. Thread priority is managed by the scheduler
	 */
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, policy);
	struct sched_param sched;
	sched.sched_priority = prio;
	pthread_attr_setschedparam(&attr, &sched);

	pthread_t t;
	pthread_create(&t, &attr, task->tk_routine, (void*) task);

	char buf[20];
	int tk_c = fixt_task_get_c(task);
	int tk_p = fixt_task_get_p(task);
	int tk_d = fixt_task_get_d(task);
	sprintf(buf, "(%d:%d:%d:%d)", task->tk_id, tk_c, tk_p, tk_d);
	pthread_setname_np(t, buf);

	task->tk_thread = t;

	return task->tk_sem_cont;
}

void fixt_task_del(struct fixt_task* task)
{
	free(task->tk_sem_cont);
	free(task->tk_sem_done);
	free(task);
}

void fixt_task_stop(struct fixt_task* task)
{
	log_func(4, "fixt_task_stop");

	write(task->tk_poison_pipe[1], &POISON_PILL, sizeof(POISON_PILL));
	sem_post(task->tk_sem_cont);

	pthread_join(task->tk_thread, NULL);
	close(task->tk_poison_pipe[0]);
	close(task->tk_poison_pipe[1]);

	sem_destroy(task->tk_sem_cont);
	sem_destroy(task->tk_sem_done);

	log_fend(4, "fixt_task_stop");
}

static void* fixt_task_routine(void* arg)
{
	struct fixt_task* task = (struct fixt_task*) arg;

	int pill;
	while (true) {
		/* Wait for the scheduler to post */
		sem_wait(task->tk_sem_cont);

		/* If the thread was told to quit while waiting, quit now! */
		read(task->tk_poison_pipe[0], &pill, sizeof(POISON_PILL));
		if (pill == POISON_PILL) break;

		/* Preemption handles splitting execution across quanta! */
		k_log_s(task->tk_id);
		spin_for(task->tk_c);
		k_log_e(task->tk_id);

		log_msg(5, " [ SPIN DONE ]");

		/*
		 * Notify the scheduler that this task is done executing. As soon as
		 * this goes through, we immediately loose control. We don't get to
		 * sem_wait above until this thread is unblocked.
		 */
		sem_post(task->tk_sem_done);
	}

	return NULL;
}

void fixt_task_set_prio(struct fixt_task* task, int prio)
{
	pthread_setschedprio(task->tk_thread, prio);
}

void fixt_task_set_param(struct fixt_task* task, int param)
{
	pthread_setschedparam(task->tk_thread, param, NULL);
}

int fixt_task_get_a(struct fixt_task* task)
{
	return task->tk_a;
}

int fixt_task_get_c(struct fixt_task* task)
{
	return task->tk_c;
}

int fixt_task_get_p(struct fixt_task* task)
{
	return task->tk_p;
}

int fixt_task_get_d(struct fixt_task* task)
{
	return task->tk_d;
}

int fixt_task_get_r(struct fixt_task* task)
{
	return task->tk_r;
}

bool fixt_task_already_executing(struct fixt_task* task)
{
	return task->tk_a > 0;
}

int fixt_task_completion_time(struct fixt_task* task)
{
	return task->tk_c - task->tk_a;
}

int fixt_task_remaining_time(struct fixt_task* task)
{
	return task->tk_d + task->tk_r;
}

sem_t* fixt_task_get_sem_cont(struct fixt_task* task)
{
	return task->tk_sem_cont;
}

sem_t* fixt_task_get_sem_done(struct fixt_task* task)
{
	return task->tk_sem_done;
}
