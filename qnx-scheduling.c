#include <stdlib.h>
#include <stdio.h>
#include "fixt.h"

#include "debug.h"

/*
 * Start up the test fixture, run the tests, then tear everything down.
 */
int main(int argc, char *argv[])
{
	printf("Welcome to 'Experiments with Real-Time Scheduling Algorithms'\n");

	fixt_init();
	fixt_test();
	fixt_term();

	return EXIT_SUCCESS;
}
