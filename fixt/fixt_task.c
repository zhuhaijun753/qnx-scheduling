#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "../spin/spin.h"
#include "fixt_task.h"

static const int POISON_PILL = 1;
static void* fixt_task_routine(void*);

struct fixt_task* fixt_task_new(int c, int p, int d)
{
	struct fixt_task task = malloc(sizeof *task);
	task->tk_c = c;
	task->tk_p = p;
	task->tk_d = d;
	task->tk_r = 0;
	task->tk_routine = &fixt_task_routine;

	task->prev = NULL;
	task->next = NULL;
}

sem_t fixt_task_run(struct fixt_task* task, int policy)
{
	pipe(task->tk_poison_pipe);
	fcntl(task->tk_poison_pipe[0], F_SETFL, O_NONBLOCK);
	fcntl(task->tk_poison_pipe[1], F_SETFL, O_NONBLOCK);

	sem_t cont;
	sem_init(&cont, 0, 0);
	task->tk_sem_cont = cont;

	sem_t done;
	sem_init(&done, 0, 0);
	task->tk_sem_done = done;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(policy);

	pthread_t* t = malloc(sizeof *t);
	pthread_create(t, &attr, task.tk_routine, (void*) task);
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

	// Force thread to check for a poison pill (pthread_kill better than post)
	pthread_kill(task->tk_thread, SIGALRM);
	// sem_post(task->tk_sem_continue);

	pthread_join(task->tk_thread, NULL);
	close(task->tk_poison_pipe[0]);
	close(task->tk_poison_pipe[1]);

	sem_destroy(task->tk_sem_cont);
	sem_destroy(task->tk_sem_done);
	free(task->tk_thread);
}

void* fixt_task_routine(void* arg)
{
	struct fixt_task* task = (struct fixt_task*) arg;

	int pill;
	while (true)
	{
		// Wait for the scheduler to post
		sem_wait(&task->tk_sem_cont);
		// If the thread was told to quit while waiting, quit now!
		read(task->tk_poison_pipe[0], &pill, sizeof(POISON_PILL));
		if (pill == POISON_PILL)
			break;

		spin_for(task->tk_c);

		// Notify the scheduler that this task is done executing
		sem_post(&task->tk_sem_done);
		// If the thread was told to quit while spinning, quit now!
		read(task->tk_poison_pipe[0], &pill, sizeof(POISON_PILL));
		if (pill == POISON_PILL)
			break;
	}
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
