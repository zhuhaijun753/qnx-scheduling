#ifndef FIXT_ALGO_IMPL_EDF_H_
#define FIXT_ALGO_IMPL_EDF_H_

#include "fixt/fixt_algo.h"
#include "fixt/fixt_hook.h"

void fixt_algo_impl_edf_init(struct fixt_algo*);
void fixt_algo_impl_edf_schedule(struct fixt_algo*);
void fixt_algo_impl_edf_block(struct fixt_algo*);

/*
 * Create a Rate Monotonic Analysis--based scheduling algorithm
 */
struct fixt_algo* fixt_algo_impl_edf_new();

#endif
