#include <unistd.h>
#include <time.h>
#include "spin.h"

void spin_calibrate()
{
	/* TODO: Implement real solution */
}

void spin_for(int quanta)
{
	/* TODO: Implement real solution */
	usleep(quanta * SPIN_QUANTUM_WIDTH_MS * 1000);
}

struct timespec* spin_abstime_in_quanta(int quanta)
{
	/* TODO: Implement solution */
	return NULL;
}
