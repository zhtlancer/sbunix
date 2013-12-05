#ifndef _PS_H
#define _PS_H

#include <defs.h>

/* Enum values for Task states */
enum TASK_STATE {
	TASK_UNUSED = 0,	/* PCB not used */
	TASK_EMBRYO,		/* Process being initialized */
	TASK_SLEEPING,		/* Process sleeping */
	TASK_RUNNABLE,		/* Process ready to run */
	TASK_RUNNING,		/* Process running */
	TASK_ZOMBIE			/* Process died */
};

struct ps_ent {
	pid_t pid;
	enum TASK_STATE state;
};

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
