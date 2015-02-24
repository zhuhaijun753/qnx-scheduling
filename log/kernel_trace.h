#ifndef LOG_K_
#define LOG_K_

#include <sys/trace.h>

void k_log_start(int c);
void k_log_end(int c);

#if 0
#define k_log_s(t) k_log_start(t)
#define k_log_e(t) k_log_end(t)
#else
#define k_log_s(t)
#define k_log_e(t)
#endif

#endif
