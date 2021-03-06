/*
 * File: fixt_task.h
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: Tasking structure containing bookeeping logic and state
 */

#ifndef FIXT_TASK_H
#define FIXT_TASK_H

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

/*
 * See the architecture doc for more on this structure.
 */
struct fixt_task
{
	int tk_id; /* Task id */

	int tk_a; /* Task run time accumlated in a single scheduler period */
	int tk_c, tk_p, tk_d; /* Execution time, period, deadline */
	int tk_r; /* Nuber of quanta until ready to execute */

	void* (*tk_routine)(void*); /* The routine run in a new thread */

	int tk_poison_pipe[2]; /* Pipe to inform the thread when to stop */
	pthread_t tk_thread;

	sem_t* tk_sem_cont; /* Scheduler releases task via posting this */
	sem_t* tk_sem_done; /* Task completes execution by posting this */

	/* Private use by utlist.h - OOPS our lists were clobbering each other */
	struct fixt_task *_ts_prev, *_ts_next; /* Task set list */
	struct fixt_task *_at_prev, *_at_next; /* Algo task list */
	struct fixt_task *_aq_prev, *_aq_next; /* Algo queue list */
};

/*
 * Create a new task and initialize internal state. The fixture is responsible
 * for managing the lifecycle of tasks.
 */
struct fixt_task* fixt_task_new(int id, int c, int p, int d);
void fixt_task_del(struct fixt_task*);

/*
 * Start up the backing routine in a new thread and initialize semaphores
 */
sem_t* fixt_task_run(struct fixt_task*, int policy, int prio);

/*
 * Send a poison pill to the task, then join on that task. Also reset
 * runtime data structures for future runs.
 */
void fixt_task_stop(struct fixt_task*);

/*
 * Set the task priority
 */
void fixt_task_set_prio(struct fixt_task*, int prio);

/*
 * Set the task scheduling policy (deprecated)
 */
void fixt_task_set_param(struct fixt_task*, int param);

int fixt_task_get_a(struct fixt_task*);
int fixt_task_get_c(struct fixt_task*);
int fixt_task_get_p(struct fixt_task*);
int fixt_task_get_d(struct fixt_task*);
int fixt_task_get_r(struct fixt_task*);

/*
 * Return true if the task is already partway through its' execution time
 */
bool fixt_task_already_executing(struct fixt_task* task);

/*
 * Return the number of quanta this task will take to complete
 */
int fixt_task_completion_time(struct fixt_task*);

/*
 * Return the number of quanta remaining until this task's deadline
 */
int fixt_task_remaining_time(struct fixt_task* task);

/*
 * Scheduler posts sem_cont to release the task for execution.
 */
sem_t* fixt_task_get_sem_cont(struct fixt_task*);

/*
 * Task posts sem_done to unblock the scheduler
 */
sem_t* fixt_task_get_sem_done(struct fixt_task*);

#endif
