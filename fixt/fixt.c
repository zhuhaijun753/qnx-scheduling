/*
 * File: fixt.c
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: High-level global interface to the scheduling test fixture
 */

#include <stdbool.h>
#include <signal.h>
#include <stdio.h>
#include "utlist.h"
#include "fixt/impl/rma/fixt_algo_impl_rma.h"
#include "fixt/impl/edf/fixt_algo_impl_edf.h"
#include "fixt/impl/sct/fixt_algo_impl_sct.h"
#include "fixt_set.h"
#include "fixt_algo.h"
#include "fixt_task.h"
#include "fixt.h"
#include "spin/spin.h"
#include "spin/timing.h"

#include "debug.h"
#include "log/log.h"
#include "log/kernel_trace.h"

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
	k_log_s(0);
	spin_calibrate();
	register_tasks();
	register_algos();
	k_log_e(0);
}

void fixt_test()
{
	log_func(0, "fixt_test");

	/* Run each combination of algorithm and task set */
	struct fixt_algo* algo; int a = 0;
	DL_FOREACH(algo_list, algo) {
		struct fixt_set* set; int s = 0;
		DL_FOREACH(set_list, set) {
			prime_algo(algo, set);
			run_test_on(algo); /* Returns if algo becomes unschedulable */

			if(algo->al_schedulable) {
				printf(" [ ALGO %d TEST SET %d PASS ]\n", a, s);
			} else {
				printf(" [ ALGO %d TEST SET %d FAIL ]\n", a, s);
			}
			s++;
		}
		a++;
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
 * User: place task set definitions here. Be aware of macro double evaluation.
 */
static void register_tasks()
{
	/* @formatter:off */
	set_list = NULL;

	/* Task set #1 */
	struct fixt_set* set1 = fixt_set_new(1, 5*3,
			1, 7, 7,
			2, 5, 5,
			1, 8, 8,
			1, 10, 10,
			2, 16, 16);

	/* Task set #2 */
	struct fixt_set* set2 = fixt_set_new(2, 3*3,
			1, 3, 3,
			2, 5, 5,
			1, 10, 10);

	/* Task set #3 */
	struct fixt_set* set3 = fixt_set_new(3, 4*3,
			1, 4, 4,
			2, 5, 5,
			1, 8, 8,
			1, 10, 10);

	DL_APPEND(set_list, set1);
	//DL_APPEND(set_list, set2);
	//DL_APPEND(set_list, set3);
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
 * these methods in fixt/impl.
 *
 * Be aware of macro double evaluation.
 */
static void register_algos()
{
	algo_list = NULL;

	struct fixt_algo* rma = fixt_algo_impl_rma_new();
	struct fixt_algo* edf = fixt_algo_impl_edf_new();
	struct fixt_algo* sct = fixt_algo_impl_sct_new();

	DL_APPEND(algo_list, rma);
	//DL_APPEND(algo_list, edf);
	//DL_APPEND(algo_list, sct);
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
	log_func(1, "run_test_on");
    /*
	 * Before the alarm, alternate between scheduling tasks and running them.
	 * This could go on forever, but we only need a limited stream of data
	 * for analysis. The halt method will kill all task threads.
	 */
	struct timespec init, post, elap;
	clock_gettime(CLOCK_REALTIME, &init);
	do {
		fixt_algo_schedule(algo);
		if(algo->al_schedulable) {
			fixt_algo_run(algo);
		} else {
			/* Algo is no longer schedulable. End test and halt threads */
			break;
		}
		clock_gettime(CLOCK_REALTIME, &post);
		timing_timespec_sub(&elap, &post, &init);
	} while (elap.tv_sec < FIXT_SECONDS_PER_TEST);
	fixt_algo_halt(algo);

	log_fend(1, "run_test_on");
}
