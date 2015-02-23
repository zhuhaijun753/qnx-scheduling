#ifndef FIXT_ALGO_IMPL_SCT_H_
#define FIXT_ALGO_IMPL_SCT_H_

#include "fixt/fixt_algo.h"
#include "fixt/fixt_hook.h"

void fixt_algo_impl_sct_init(struct fixt_algo*);
void fixt_algo_impl_sct_schedule(struct fixt_algo*);
void fixt_algo_impl_sct_block(struct fixt_algo*);

/*
 * Create a Rate Monotonic Analysis--based scheduling algorithm
 */
struct fixt_algo* fixt_algo_impl_sct_new();

#endif
