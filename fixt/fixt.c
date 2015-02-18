#include <stdbool.h>
#include <signal.h>
#include "utlist.h"
#include "fixt/impl/rma/fixt_algo_impl_rma.h"
#include "fixt_set.h"
#include "fixt_algo.h"
#include "fixt_task.h"
#include "fixt.h"

static struct fixt_set* set_list = NULL;
static struct fixt_algo* algo_list = NULL;

static void register_tasks();
static void register_algos();

static void clean_tasks();
static void clean_algos();

static void prime_algo(struct fixt_algo*, struct fixt_set*);
static void run_test_on(struct fixt_algo*);

void fixt_init()
{
	register_tasks();
	register_algos();
}

void fixt_test()
{
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

//@formatter:off
static void register_tasks()
{
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
}
//@formatter:on

static void clean_tasks()
{
	struct fixt_set *elt, *tmp;
	DL_FOREACH_SAFE(set_list, elt, tmp)
	{
		DL_DELETE(set_list, elt);
		fixt_set_del(elt);
	}
}

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
	fixt_algo_add_all(algo, set->ts_set_head);
	fixt_algo_init(algo);
}

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

	alarm(FIXT_SECONDS_PER_TEST);
	while (!move_on)
	{
		fixt_algo_schedule(algo);
		fixt_algo_run(algo);
	}
	fixt_algo_halt(algo);
}
