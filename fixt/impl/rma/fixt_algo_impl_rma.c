#include <pthread.h>

#include "../fixt_hook.h"
#include "../fixt_algo.h"
#include "fixt_algo_impl_rma.h"

/**
 * RMA requires the fixture thread (self) to use a FIFO policy.
 *
 * From QNX: a thread selected to run continues executing until it:
 *  - voluntarily relinquishes control (e.g., it blocks)
 *  - is preempted by a higher-priority thread
 */
void fixt_algo_impl_rma_init(struct fixt_algo* algo)
{
	pthread_t self = pthread_self();
	pthread_setschedparam(self, SCHED_FIFO, NULL);
}

void fixt_algo_impl_rma_schedule(struct fixt_algo* algo)
{

}

void fixt_algo_impl_rma_run(struct fixt_algo* algo)
{

}

struct fixt_algo* fixt_algo_impl_rma_new()
{
	AlgoHook ag_init = &fixt_algo_impl_rma_init();
	AlgoHook ag_schedule = &fixt_algo_impl_rma_schedule();
	AlgoHook ag_run = &fixt_algo_impl_rma_run();

	return fixt_algo_new(ag_init, ag_schedule, ag_run);
}
