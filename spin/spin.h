#ifndef SPIN_H_
#define SPIN_H_

#include <time.h>

#define SPIN_QUANTUM_WIDTH_MS 10

void spin_calibrate();
void spin_for(int quanta);

struct timespec* spin_abstime_in_quanta(int quanta);

#endif
