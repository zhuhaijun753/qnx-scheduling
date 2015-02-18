/*
 * File: fixt_hook.h
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: Shorthand for writing fixt_algo function pointer boilerplate
 */

#ifndef FIXT_HOOK_
#define FIXT_HOOK_

struct fixt_algo;

typedef void (*AlgoHook)(struct fixt_algo*);

#endif
