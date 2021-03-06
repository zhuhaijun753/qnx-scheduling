#ifndef LOG_K_
#define LOG_K_

#define LOG_K_ALGO 20 /* Higher than the number of tasks in a given set */
#define LOG_K_FIXT 21

void k_log_start(int c);
void k_log_end(int c);

void k_log_csv_start(int c);
void k_log_csv_end(int c);
void k_log_csv_print(void);

#define LOG_K_METHOD 2

#if LOG_K_METHOD == 1
#define k_log_s(t) k_log_start(t)
#define k_log_e(t) k_log_end(t)
#elif LOG_K_METHOD == 2
#define k_log_s(t) k_log_csv_start(t)
#define k_log_e(t) k_log_csv_end(t)
#else
#define k_log_s(t)
#define k_log_e(t)
#endif

#endif
