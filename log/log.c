#include <stdio.h>
#include <stdbool.h>
#include "fixt/fixt_task.h"
#include "log.h"

static char* dots[] =
{ "", "..", "....", "......", "........" };

void log_func_f(int indent, char* fun)
{
	printf("%s %s()\n", dots[indent], fun);
}

void log_fend_f(int indent, char* fun)
{
	printf("%s %s() end\n", dots[indent], fun);
}

void log_rchk_f(int indent, struct fixt_task* task)
{
	printf("%s rchk (%d, %d, %d: %d)\n", dots[indent], task->tk_c, task->tk_p,
			task->tk_d, task->tk_r);
}

void log_hbef_f(int indent, struct fixt_task* task)
{
	printf("%s head (%d, %d, %d: %d) -> ", dots[indent], task->tk_c, task->tk_p,
			task->tk_d, task->tk_r);
}

void log_haft_f(int indent, struct fixt_task* task)
{
	printf("(%d, %d, %d: %d)\n", task->tk_c, task->tk_p, task->tk_d, task->tk_r);
}

void log_ibef_f(int indent, struct fixt_task* task)
{
	printf("%s idle (%d, %d, %d: %d) -> ", dots[indent], task->tk_c, task->tk_p,
			task->tk_d, task->tk_r);
}

void log_iaft_f(int indent, struct fixt_task* task)
{
	printf("(%d, %d, %d: %d)\n", task->tk_c, task->tk_p, task->tk_d, task->tk_r);
}
