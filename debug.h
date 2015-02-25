#ifndef DEBUG_H_
#define DEBUG_H_

#include "stdio.h"

#if 0
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

#endif
