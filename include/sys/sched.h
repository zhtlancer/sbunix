#ifndef _SCHED_H
#define _SCHED_H

#include <defs.h>

/* Enum values for Task states */
enum TASK_STATE {
	TASK_UNUSED,
	TASK_EMBRYO,
	TASK_SLEEPING,
	TASK_RUNNABLE,
	TASK_RUNNING,
	TASK_ZOMBIE
};

struct task_struct;

struct context {
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbx;
	uint64_t rax;
	uint64_t rip;
};

struct task_struct {
	struct context context;
};

int sched_init(void);

void swtch(struct context **old, struct context *new);

void swtch_to(struct context *new);

#endif

/* vim: set ts=4 sw=0 tw=0 noet : */
