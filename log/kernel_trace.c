#include <stdio.h>
#include <time.h>
#include <sys/neutrino.h>
#include <sys/trace.h>
#include "fixt/fixt_task.h"
#include "kernel_trace.h"

#define LOG_K_LENGTH 1000
static struct timespec log_time[LOG_K_LENGTH];
static int log_event[LOG_K_LENGTH];
static int log_entry = 0;

#define LOG_K_BEG 0
#define LOG_K_END 1

void k_log_start(int c)
{
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, c, "beg");
}

void k_log_end(int c)
{
	TraceEvent(_NTO_TRACE_INSERTUSRSTREVENT, c, "end");
}

void k_log_csv_start(int c)
{
	if (log_entry < LOG_K_LENGTH) {
		log_event[log_entry] = c;
		clock_gettime(CLOCK_REALTIME, &log_time[log_entry]);

		log_entry++;
	}
}

void k_log_csv_end(int c)
{
	if (log_entry < LOG_K_LENGTH) {
		log_event[log_entry] = c;
		clock_gettime(CLOCK_REALTIME, &log_time[log_entry]);

		log_entry++;
	}
}

void k_log_csv_print()
{
	int i;
	for (i = 0; i < LOG_K_LENGTH; i++) {
		struct timespec ti = log_time[i];
		printf("%d, %d, %d, %ld\n", i, log_event[i], ti.tv_sec, ti.tv_nsec);
	}
}
