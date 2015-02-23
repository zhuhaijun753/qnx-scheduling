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
#include "spin/spin.h"
#include "spin/timing.h"

#include "debug.h"
#include "log/log.h"

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

static void calibrate_spin(); /* Calibrates the tests to the processor speed */

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
	calibrate_spin();
	register_tasks();
	register_algos();
}

void calibrate_spin()
{
	dprintf("Calibrating to the host processor...\n");
	spin_calibrate();
	dprintf("Calibration successful!\n");

	// Verify calibration
	struct timespec t_init, t_post, t_elap;
	printf("Target 10ms\n");

	clock_gettime(CLOCK_REALTIME, &t_init);
	spin_for(1);
	clock_gettime(CLOCK_REALTIME, &t_post);

	timing_timespec_sub(&t_elap, &t_post, &t_init);
	printf("Actual: %ldms\n", t_elap.tv_nsec / 1000000);
	printf("Calibration successful!\n");
}

void fixt_test()
{
	log_func(0, "fixt_test");

	/* Run each combination of algorithm and task set */
	struct fixt_algo* algo;
	DL_FOREACH(algo_list, algo) {
		struct fixt_set* set;
		DL_FOREACH(set_list, set) {
			prime_algo(algo, set);
			run_test_on(algo);
		}
	}

	log_fend(0, "fixt_test");
}

void fixt_term()
{
	log_func(0, "fixt_term");

	clean_tasks();
	clean_algos();

	log_fend(0, "fixt_term");
}

/*
 * User: place task set definitions here
 */
static void register_tasks()
{
	/* @formatter:off */

	/* Task set #1 */
	DL_APPEND(set_list, fixt_set_new(1, 5*3,
					1, 7, 7,
					2, 5, 5,
					1, 8, 8,
					1, 10, 10,
					2, 16, 16));

	//	/* Task set #2 */
	//	DL_APPEND(set_list, fixt_set_new(2, 3*3,
	//					1, 3, 3,
	//					2, 5, 5,
	//					1, 10, 10));
	//
	//	/* Task set #3 */
	//	DL_APPEND(set_list, fixt_set_new(3, 4*3,
	//					1, 4, 4,
	//					2, 5, 5,
	//					1, 8, 8,
	//					1, 10, 10));

	/* @formatter:on */
}

static void clean_tasks()
{
	struct fixt_set *elt, *tmp;
	DL_FOREACH_SAFE(set_list, elt, tmp) {
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
	DL_APPEND(algo_list, fixt_algo_impl_rma_new());
}

static void clean_algos()
{
	struct fixt_algo *elt, *tmp;
	DL_FOREACH_SAFE(algo_list, elt, tmp) {
		DL_DELETE(algo_list, elt);
		fixt_algo_del(elt);
	}
}

static void prime_algo(struct fixt_algo* algo, struct fixt_set* set)
{
	log_func(1, "prime_algo");

	/* Copy the task set over to the algorithms internal data */
	struct fixt_task* elt;
	DL_FOREACH2(set->ts_set_head, elt, _ts_next) {
		fixt_algo_add_task(algo, elt);
	}
	fixt_algo_init(algo);

	log_fend(1, "prime_algo");
}

static void run_test_on(struct fixt_algo* algo)
{
	dprintf("..run_test_on()\n");

	/*
	 * Before the alarm, alternate between scheduling tasks and running them.
	 * This could go on forever, but we only need a limited stream of data
	 * for analysis. The halt method will kill all task threads.
	 */
	struct timespec init, post, elap;
	clock_gettime(CLOCK_REALTIME, &init);
	do {
		fixt_algo_schedule(algo);
		fixt_algo_run(algo);

		clock_gettime(CLOCK_REALTIME, &post);
		timing_timespec_sub(&elap, &post, &init);
	} while (elap.tv_sec < FIXT_SECONDS_PER_TEST);
	fixt_algo_halt(algo);
}
