#ifndef FIXT_ALGO_IMPL_H_
#define FIXT_ALGO_IMPL_H_

#include "../fixt_algo.h"
#include "../fixt_hook.h"

extern struct fixt_algo;

void fixt_algo_impl_rma_init(struct fixt_algo*);
void fixt_algo_impl_rma_schedule(struct fixt_algo*);
void fixt_algo_impl_rma_block(struct fixt_algo*);

struct fixt_algo* fixt_algo_impl_rma_new();

#endif
