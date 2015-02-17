#ifndef FIXT_ALGO_H
#define FIXT_ALGO_H

extern struct fixt_task;

struct fixt_algo
{
	void (*fp_init)(void);
	void (*fp_schedule)(void);
	void (*fp_run)(void);

	struct fixt_task * tasks;
	struct fixt_task * taskq;

};

#endif
