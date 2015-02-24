#include <stdio.h>
#include "fixt/fixt_task.h"
#include "kernel_trace.h"

void k_log_start(int c) {
	TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, c, "start");
}

void k_log_end(int c) {
	TraceEvent(_NTO_TRACE_INSERTSUSEREVENT, c, "end");
}
