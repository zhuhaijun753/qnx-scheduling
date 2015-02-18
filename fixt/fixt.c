/*
 * File: fixt.c
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: High-level global interface to the scheduling test fixture
 */

#include <stdbool.h>
#include <signal.h>
#include "utlist.h"
#include "fixt/impl/rma/fixt_algo_impl_rma.h"
#include "fixt_set.h"
#include "fixt_algo.h"
#include "fixt_task.h"
#include "fixt.h"

/*
 * A global doubly linked list (DL*) of task sets.
 */
static struct fixt_set* set_list = NULL;

/**
 * A global doubly linked list (DL*) of scheduling algorithms
 */
static struct fixt_algo* algo_list = NULL;

static void register_tasks(); /* User task sets go here */
static void register_algos(); /* User pulls in algorithms here */

static void clean_tasks(); /* Clean up resources */
static void clean_algos();

/*
 * Prime an algorithm with a particular task set. This will start up each
 * task thread and initialize relevant algorithm data.
 */
static void prime_algo(struct fixt_algo*, struct fixt_set*);

/*
 * Run a test on the primed task set for a limited run. The run is
 * limited by FIXT_SECONDS_PER_TEST. Otherwise, algo simulations are infinite.
 */
static void run_test_on(struct fixt_algo*);

void fixt_init()
{
	/* TODO: put whole process into a high priority */

	register_tasks();
	register_algos();
}

void fixt_test()
{
	/* Run each combination of algorithm and task set */
	struct fixt_algo* algo;
	DL_FOREACH(algo_list, algo)
	{
		struct fixt_set* set;
		DL_FOREACH(set_list, set)
		{
			prime_algo(algo, set);
			run_test_on(algo);
		}
	}
}

void fixt_term()
{
	clean_tasks();
	clean_algos();
}

/*
 * User: place task set definitions here
 */
static void register_tasks()
{
	//@formatter:off

	/* Task set #1 */
	DL_APPEND(set_list, fixt_set_new(1, 5,
					1, 7, 7,
					2, 5, 5,
					1, 8, 8,
					1, 10, 10,
					2, 16, 16));

	/* Task set #2 */
	DL_APPEND(set_list, fixt_set_new(2, 3,
					1, 3, 3,
					2, 5, 5,
					1, 10, 10));

	/* Task set #3 */
	DL_APPEND(set_list, fixt_set_new(3, 4,
					1, 4, 4,
					2, 5, 5,
					1, 8, 8,
					1, 10, 10));

	//@formatter:on
}

static void clean_tasks()
{
	struct fixt_set *elt, *tmp;
	DL_FOREACH_SAFE(set_list, elt, tmp)
	{
		DL_DELETE(set_list, elt);
		fixt_set_del(elt);
	}
}

/*
 * User: register the algorithms you want to use here. You do this by
 * using a method of the form fixt_algo_impl_{name}_new(). You can find
 * this methods in fixt/impl
 */
static void register_algos()
{
	/* Sched algorithm #1 - RMA */
	/* TODO: Figure this out */
	DL_APPEND(algo_list, fixt_algo_impl_rma_new());
}

static void clean_algos()
{
	struct fixt_algo *elt, *tmp;
	DL_FOREACH_SAFE(algo_list, elt, tmp)
	{
		DL_DELETE(algo_list, elt);
		fixt_algo_del(elt);
	}
}

static void prime_algo(struct fixt_algo* algo, struct fixt_set* set)
{
	/* Copy the task set over to the algorithms internal data */
	fixt_algo_copy_all(algo, set->ts_set_head);
	fixt_algo_init(algo);
}

/*
 * When true, the test runner should stop testing the current configuration
 */
static volatile sig_atomic_t move_on;
static void move_on_handler()
{
	move_on = true;
}
static void run_test_on(struct fixt_algo* algo)
{
	move_on = false;

	struct sigaction act;
	act.sa_handler = &move_on_handler;
	sigaction(SIGALRM, &act, NULL);
	/*
	 * Before the alarm, alternate between scheduling tasks and running them.
	 * This could go on forever, but we only need a limited stream of data
	 * for analysis. The halt method will kill all task threads.
	 */
	alarm(FIXT_SECONDS_PER_TEST); /* Posix alarm will trigger handler */
	while (!move_on)
	{
		fixt_algo_schedule(algo);
		fixt_algo_run(algo);
	}
	fixt_algo_halt(algo);
}