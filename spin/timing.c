/*
 * File: fixt_set.h
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: This file contains the implementation of the puhlic timing
 * helper interface. It provides functions for working with the
 * obtuse timespec structures.
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/neutrino.h>

#include "timing.h"

/*
 * The following function is a modified version of the code provided in GNU's
 * documentation online:
 *
 * http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
 *
 * This function performs Result = X - Y on the provided timespecs. NOTE: the
 * input Y to this function is mutated as a result of the computation.
 *
 * Params: result - The difference
 *         x      - The minuend
 *         y      - The subtrahend (MODIFIED!)
 * Return: 1      - If difference is negative
 *         0      - If difference is positive
 */
int timing_timespec_sub(result, x, y)
	struct timespec *result, *x, *y; {
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_nsec < y->tv_nsec) {
		int nums = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
		y->tv_nsec -= 1000000000 * nums;
		y->tv_sec += nums;
	}
	if (x->tv_nsec - y->tv_nsec > 1000000000) {
		int nums = (x->tv_nsec - y->tv_nsec) / 1000000000;
		y->tv_nsec += 1000000000 * nums;
		y->tv_sec -= nums;
	}

	/* Compute the time remaining to wait.
	 tv_usec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_nsec = x->tv_nsec - y->tv_nsec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}

/*
 * This function records the current time in the provided timespec and then
 * pushes that timespec the provided number of nanoseconds into the future.
 *
 * Params: future   - the absolute timespec in which to record a future time
 *         my_nanos - the number of nanoseconds to go into the future
 */
void timing_future_nanos(struct timespec *future, long my_nanos) {
	clock_gettime(CLOCK_REALTIME, future);
	long rt_nanos = future->tv_nsec;

	if (rt_nanos + my_nanos > 1000000000) {
		future->tv_nsec = (rt_nanos + my_nanos) - 1000000000;
		future->tv_sec++;
	} else {
		future->tv_nsec += my_nanos;
	}
}

/*
 * Initializes a context timespec, serving as t0 in a [t0, t] interval.
 *
 * Params: init - A pointer to the timespec to initialize
 * Return: void
 */
void timing_heartbeat_init(struct timespec *init)
{
	clock_gettime(CLOCK_REALTIME, init);
}

/*
 * Waits the difference between the context timespec (initialized via
 * timing_heartbeat_init) and the number of nanoseconds in nanos. Of course,
 * this function will only sleep if a positive number of nanoseconds remains.
 *
 * Params: init  - A pointer to the context timespec
 *         nanos - The heartbeat in nanoseconds to perform
 * Return: void
 */
void timing_heartbeat_wait(struct timespec *init, long nanos)
{
	//puts("timing_heartbeat_wait");

	struct timespec post, elap, durt;
	clock_gettime(CLOCK_REALTIME, &post);
	durt.tv_sec = (time_t)0;
	durt.tv_nsec = nanos;

	if(timing_timespec_sub(&elap, &post, init) == 0) {
		//printf("timing_elap: %d, %ld\r\n", elap.tv_sec, elap.tv_nsec);
		if(timing_timespec_sub(&elap, &durt, &elap) == 0) {
			//printf("clock_nanosleep: %ld\r\n", elap.tv_nsec);
			clock_nanosleep(CLOCK_REALTIME, NULL, &elap, NULL);
		}
	} else {
		//printf("else: %d, %ld\r\n", elap.tv_sec, elap.tv_nsec);
	}
}
