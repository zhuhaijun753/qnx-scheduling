#ifndef LOG_H_
#define LOG_H_

#if 1
#define log_func(i, f) log_func_f(i, f)
#define log_fend(i, f) log_fend_f(i, f)
#define log_rchk(i, t) log_rchk_f(i, t)
#define log_hbef(i, t) log_hbef_f(i, t)
#define log_haft(i, t) log_haft_f(i, t)
#define log_ibef(i, t) log_ibef_f(i, t)
#define log_iaft(i, t) log_iaft_f(i, t)
#define log_msg(i, m) log_msg_f(i, m)

void log_func_f(int indent, char* fun);
void log_fend_f(int indent, char* fun);
void log_rchk_f(int indent, struct fixt_task* task);
void log_hbef_f(int indent, struct fixt_task* task);
void log_haft_f(int indent, struct fixt_task* task);
void log_ibef_f(int indent, struct fixt_task* task);
void log_iaft_f(int indent, struct fixt_task* task);
void log_msg_f(int indent, char* msg);

#else
#define log_func(i, f)
#define log_fend(i, f)
#define log_rchk(i, t)
#define log_hbef(i, t)
#define log_haft(i, t)
#define log_ibef(i, t)
#define log_iaft(i, t)
#define log_msg(i, m)
#endif

#endif
