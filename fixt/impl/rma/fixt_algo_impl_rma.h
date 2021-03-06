/*
 * File: fixt_algo_impl_rma.c
 * Author: Steven Kroh
 * Date: 28 Feb 2015
 * Description: Implementation of fixt_algo for Rate Monotonic Analysis
 */

#ifndef FIXT_ALGO_IMPL_RMA_H_
#define FIXT_ALGO_IMPL_RMA_H_

#include "fixt/fixt_algo.h"
#include "fixt/fixt_hook.h"

void fixt_algo_impl_rma_init(struct fixt_algo*);
void fixt_algo_impl_rma_schedule(struct fixt_algo*);
void fixt_algo_impl_rma_block(struct fixt_algo*);

/*
 * Create a Rate Monotonic Analysis--based scheduling algorithm
 */
struct fixt_algo* fixt_algo_impl_rma_new();

#endif
