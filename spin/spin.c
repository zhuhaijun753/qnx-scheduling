#include <unistd.h>
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
