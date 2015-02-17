#ifndef FIXT_TASK_H
#define FIXT_TASK_H

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

struct fixt_task
{
	int tk_c, tk_p, tk_d;
	int tk_r;

	void* (*tk_routine)(void*);

	int tk_poison_pipe[2];
	pthread_t tk_thread;
	sem_t tk_sem_continue;
};

struct fixt_task* fixt_task_new(int c, int p, int d, void* (*rt)(void*));
void fixt_task_del(struct fixt_task*);

sem_t fixt_task_run(struct fixt_task*);
void fixt_task_stop(struct fixt_task*);

int fixt_task_get_c(struct fixt_task*);
int fixt_task_get_p(struct fixt_task*);
int fixt_task_get_d(struct fixt_task*);
int fixt_task_get_r(struct fixt_task*);

#endif
