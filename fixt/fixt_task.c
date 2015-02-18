/*
 * File: fixt_task.c
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: Tasking structure containing bookeeping logic and state
 */

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "spin/spin.h"
#include "fixt_task.h"

/*
 * Value sent over the poison pill pipe indicating a thread should end
 * as soon as possible
 */
static const int POISON_PILL = 1;

/*
 * The routine which is run as the task in a new thread
 */
static void* fixt_task_routine(void*);

struct fixt_task* fixt_task_new(int c, int p, int d)
{
	struct fixt_task* task = malloc(sizeof *task);
	task->tk_c = c;
	task->tk_p = p;
	task->tk_d = d;
	task->tk_r = 0;
	task->tk_routine = &fixt_task_routine;

	task->prev = NULL;
	task->next = NULL;

	return task;
}

sem_t fixt_task_run(struct fixt_task* task, int policy)
{
	pipe(task->tk_poison_pipe);
	/* Set to nonblocking. A thread join is used to sync threads instead */
	fcntl(task->tk_poison_pipe[0], F_SETFL, O_NONBLOCK);
	fcntl(task->tk_poison_pipe[1], F_SETFL, O_NONBLOCK);

	sem_t cont;
	sem_init(&cont, 0, 0); /* First sem_wait blocks */
	task->tk_sem_cont = cont;

	sem_t done;
	sem_init(&done, 0, 0); /* First sem_wait blocks */
	task->tk_sem_done = done;

	/*
	 * The thread should have the appropriate scheduling policy when it
	 * starts. Thread priority is managed by the scheduler
	 */
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, policy);

	pthread_t t;
	pthread_create(&t, &attr, task->tk_routine, (void*) task);
	task->tk_thread = t;

	return cont;
}

void fixt_task_del(struct fixt_task* task)
{
	free(task);
}

void fixt_task_stop(struct fixt_task* task)
{
	write(task->tk_poison_pipe[1], &POISON_PILL, sizeof(POISON_PILL));

	/* Force task to check for a poison pill (pthread_kill better than post) */
	/* TODO: integrate with spin_for()'s actual solution. */
	pthread_kill(task->tk_thread, SIGALRM);
	/* sem_post(task->tk_sem_continue); */

	pthread_join(task->tk_thread, NULL);
	close(task->tk_poison_pipe[0]);
	close(task->tk_poison_pipe[1]);

	sem_destroy(&task->tk_sem_cont);
	sem_destroy(&task->tk_sem_done);
}

static void* fixt_task_routine(void* arg)
{
	struct fixt_task* task = (struct fixt_task*) arg;

	int pill;
	while (true)
	{
		/* Wait for the scheduler to post */
		sem_wait(&task->tk_sem_cont);
		/* If the thread was told to quit while waiting, quit now! */
		read(task->tk_poison_pipe[0], &pill, sizeof(POISON_PILL));
		if (pill == POISON_PILL)
			break;

		spin_for(task->tk_c);

		/* Notify the scheduler that this task is done executing */
		sem_post(&task->tk_sem_done);
		/* If the thread was told to quit while spinning, quit now! */
		read(task->tk_poison_pipe[0], &pill, sizeof(POISON_PILL));
		if (pill == POISON_PILL)
			break;
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

sem_t fixt_task_get_sem_cont(struct fixt_task* task)
{
	return task->tk_sem_cont;
}

sem_t fixt_task_get_sem_done(struct fixt_task* task)
{
	return task->tk_sem_done;
}
