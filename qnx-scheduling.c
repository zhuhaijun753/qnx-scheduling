#include <stdlib.h>
#include <stdio.h>
#include "fixt.h"

#include "debug.h"
#include "log/kernel_trace.h"

/*
 * Start up the test fixture, run the tests, then tear everything down.
 */
int main(int argc, char *argv[])
{
	printf("Welcome to 'Experiments with Real-Time Scheduling Algorithms'\n");

	fixt_init();
	fixt_test();
	fixt_term();

#if LOG_K_METHOD == 2
	k_log_csv_print();
#endif

	return EXIT_SUCCESS;
}
