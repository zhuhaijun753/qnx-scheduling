#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "fixt_task.h"

static const int poison_pill = 1;
static void* fixt_task_routine(void*);

struct fixt_task* fixt_task_new(int c, int p, int d, void* (*rt)(void*))
{
	struct fixt_task task = malloc(sizeof *task);
	task->tk_c = c;
	task->tk_p = p;
	task->tk_d = d;
	task->tk_r = 0;
	task->tk_routine = rt;
}

sem_t fixt_task_run(struct fixt_task* task)
{
	pipe(task->tk_poison_pipe);
	fcntl(task->tk_poison_pipe[0], F_SETFL, O_NONBLOCK);
	fcntl(task->tk_poison_pipe[1], F_SETFL, O_NONBLOCK);

	pthread_t* t = malloc(sizeof *t);
	pthread_create(t, NULL, task.tk_routine, NULL);
	task->tk_thread = t;

	sem_t* cont = malloc(sizeof *cont);
	sem_init(cont, 0, 0);
	task->tk_sem_continue = cont;

	return cont;
}

void fixt_task_del(struct fixt_task* task)
{
	free(task->tk_thread);
	free(task->tk_sem_continue);
	free(task);
}

void fixt_task_stop(struct fixt_task* task)
{
	write(task->tk_poison_pipe[1], &poison_pill, sizeof(poison_pill));
	sem_post(task->tk_sem_continue);

	pthread_join(task->tk_thread, NULL);
	close(task->tk_poison_pipe[0]);
	close(task->tk_poison_pipe[1]);

	sem_destroy(task->tk_sem_continue);
}

void* fixt_task_routine(void* arg)
{
	struct fixt_task* task = (struct fixt_task*) arg;
	int pill = 0;
	do
	{
		sem_wait(&task->tk_sem_continue);
		/* spin_for(task->tk_c) */
		sem_post(&task->tk_sem_continue);

		read(task->tk_poison_pipe[0], &pill, sizeof(poison_pill));
	} while (pill != poison_pill);
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

