/*
 * File: spin.h
 * Author: Steven Kroh
 * Date: 18 Feb 2015
 * Description: CPU-time consuming functionality + timespec manipulation
 */

#ifndef SPIN_H_
#define SPIN_H_

#include <time.h>

/*
 * The width of one quantum in milliseconds.
 */
#define SPIN_QUANTUM_WIDTH_MS 10

/*
 * Calibrate the spin module for the hardware in use.
 */
void spin_calibrate();

/*
 * Consume CPU time for a particular number of quanta.
 */
void spin_for(int quanta);

/*
 * Consume CPU time for a particular number of quanta, but for not more
 * than that specific number of quanta.
 */
void spin_for_nmt(int quanta);

/*
 * Generate an absolute timespec representing the time a particular
 * number of quanta from now.
 */
struct timespec spin_abstime_in_quanta(int quanta, long jitter_ns);

#endif
