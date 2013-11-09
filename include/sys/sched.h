#ifndef _SCHED_H
#define _SCHED_H

#include <defs.h>

/* Enum values for Task states */
enum TASK_STATE {
	TASK_UNUSED,	/* PCB not used */
	TASK_EMBRYO,	/* Process being initialized */
	TASK_SLEEPING,	/* Process sleeping */
	TASK_RUNNABLE,	/* Process ready to run */
	TASK_RUNNING,	/* Process running */
	TASK_ZOMBIE		/* Process died */
};

struct task_struct;

struct context {
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbx;
	uint64_t rax;
	uint64_t rip;
};

/*
 * Structure for process (PCB)
 */
struct task_struct {
	/* Process ID */
	volatile int pid;

	/* Process state */
	enum TASK_STATE state;

	/* Parent process */
	struct task_struct *parent;

	/* Saved CR3 register */
	void *cr3;

	/* Saved context for this process (userspace stack) */
	struct context *context;
};

int sched_init(void);

void scheduler(void);

void swtch(struct context **old, struct context *new);

void swtch_to(struct context *new);

#endif

/* vim: set ts=4 sw=0 tw=0 noet : */
