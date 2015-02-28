/*
 * File: fixt_algo_impl_edf.h
 * Author: Steven Kroh
 * Date: 28 Feb 2015
 * Description: Implementation of fixt_algo for Earliest Deadline First
 */

#ifndef FIXT_ALGO_IMPL_EDF_H_
#define FIXT_ALGO_IMPL_EDF_H_

#include "fixt/fixt_algo.h"
#include "fixt/fixt_hook.h"

#define EDF_PERIOD 1 /* Number of quanta between scheduler preemptions */
#define EDF_JITTER 2000000 /* 2ms grace period before preempting tasks */

void fixt_algo_impl_edf_init(struct fixt_algo*);
void fixt_algo_impl_edf_schedule(struct fixt_algo*);
void fixt_algo_impl_edf_block(struct fixt_algo*);

/*
 * Create an Earliest Deadline First scheduling algorithm
 */
struct fixt_algo* fixt_algo_impl_edf_new();

#endif
