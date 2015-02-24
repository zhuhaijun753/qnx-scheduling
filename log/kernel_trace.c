#include "fixt/fixt_task.h"
#include "kernel_trace.h"
#include <stdio.h>

void k_log_s(int c) {
	TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, c, "start");
}

void k_log_e(int c) {
	TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, c, "end");
}
