/*
 * File: spin.c
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: CPU-time consuming functionality + timespec manipulation
 */

#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <time.h>
#include <inttypes.h>
#include <pthread.h>
#include "timing.h"
#include "fixt/fixt_algo.h"
#include "spin.h"

static int FUDGE_FACTOR;

#pragma GCC push_options
#pragma GCC optimize ("O0")

void spin_calibrate()
{
	/* Set thread to highest user priority so we reduce jitter */
	pthread_t self = pthread_self();
	pthread_setschedprio(self, FIXT_ALGO_BASE_PRIO);

	/* Set the clock period */
	ThreadCtl(_NTO_TCTL_IO, NULL);

	struct _clockperiod clk;
	clk.fract = 0;
	clk.nsec = 100000; /* .1 ms */
	ClockPeriod(CLOCK_REALTIME, &clk, NULL, 0);

	/* Time measuring variables */
	int64_t std_elap = 0, unit_elap = 0;
	struct timespec init_a, post_a, elap_a;

	/* Measure the average accuracy of usleep */
	int i;
	for (i = 0; i < 10; i++)
	{
		/* Measure the actual time passage in a 10ms usleep */
		clock_gettime(CLOCK_REALTIME, &init_a);
		usleep(SPIN_QUANTUM_WIDTH_MS * 1000);
		clock_gettime(CLOCK_REALTIME, &post_a);

		/* Calculate diff */
		timing_timespec_sub(&elap_a, &post_a, &init_a);
		std_elap += elap_a.tv_nsec;
	}
	/* Compute usleep average */
	std_elap = (int) (std_elap / 10.0);
	int unit = 10000;

	int j;
	for (i = 0; i < 11; i++)
	{
		int x = 1;
		clock_gettime(CLOCK_REALTIME, &init_a);

		for (j = 0; j < unit; j++)
		{
			x = x + 1;
		}

		clock_gettime(CLOCK_REALTIME, &post_a);

		if (i == 0)
			continue;

		timing_timespec_sub(&elap_a, &post_a, &init_a);
		unit_elap += elap_a.tv_nsec;
	}

	unit_elap = (int) (unit_elap / 10.0);

	/* printf("std_elap: %lld, unit_elap: %lld\n", std_elap, unit_elap); */
	double fudge = std_elap / (double) unit_elap;
	int fudged_unit = (int) ((double) unit * fudge);
	int x = 1;
	struct timespec t_init, t_post, t_elap;
	usleep(10000);
	long act = 0;

	/* Fine tune calibration */
	while (abs(10000000 - act) > 5000000)
	{
		fudged_unit *= 1.01;
		act = 0;
		for (j = 0; j < 10; j++)
		{
			clock_gettime(CLOCK_REALTIME, &t_init);
			for (i = 0; i < fudged_unit; i++)
			{
				x = x + 1;
			}
			clock_gettime(CLOCK_REALTIME, &t_post);
			timing_timespec_sub(&t_elap, &t_post, &t_init);
			act += t_elap.tv_nsec;
		}
		act /= 10;
	}

	usleep(1000000);

	for (j = 0; j < 10; j++)
	{
		clock_gettime(CLOCK_REALTIME, &t_init);
		for (i = 0; i < fudged_unit; i++)
		{
			x = x + 1;
		}
		clock_gettime(CLOCK_REALTIME, &t_post);
		timing_timespec_sub(&t_elap, &t_post, &t_init);
		/* printf("result: %ld\n", t_elap.tv_nsec); */
	}

	FUDGE_FACTOR = fudged_unit;
}

void spin_for(int quanta)
{
	int i, x = 0;
	for (i = 0; i < quanta * FUDGE_FACTOR; i++)
	{
		x = x + 1;
	}
}

struct timespec* spin_abstime_in_quanta(int quanta)
{
	/* TODO: Implement solution */
	return NULL;
}
