/*
 * File: fixt_algo_impl_sct.h
 * Author: Steven Kroh
 * Date: 28 Feb 2015
 * Description: Implementation of fixt_algo for Shortest Completion Time
 */

#ifndef FIXT_ALGO_IMPL_SCT_H_
#define FIXT_ALGO_IMPL_SCT_H_

#include "fixt/fixt_algo.h"
#include "fixt/fixt_hook.h"

#define SCT_PERIOD 1 /* Number of quanta between scheduler preemptions */
#define SCT_JITTER 2000000 /* 2ms leeway before preempting tasks */

void fixt_algo_impl_sct_init(struct fixt_algo*);
void fixt_algo_impl_sct_schedule(struct fixt_algo*);
void fixt_algo_impl_sct_block(struct fixt_algo*);

/*
 * Create a Shortest Completion Time--based scheduling algorithm
 */
struct fixt_algo* fixt_algo_impl_sct_new();

#endif
