#ifndef _SCHED_H
#define _SCHED_H

#include <defs.h>
#include <sys/mm.h>

/* Top of user address space(excluding) */
#define UVMA_TOP	(0x0000800000000000UL)
/* Top of user stack */
#define USTACK_TOP	(UVMA_TOP - __PAGE_SIZE)

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

	/* Kernel stack */
	void *stack;

	mm_struct_t *mm;

	/* Saved CR3 register (PA to page table)*/
	addr_t cr3;

	/* Saved context for this process (userspace stack) */
	struct context *context;

	/*
	 * TODO:
	 * # Filesystem related elements (current dir, opened files)
	 * # Other stuffs that required...
	 */
};

struct task_struct *create_task(const char *name);

struct task_struct *duplicate_task(struct task_struct *parent);

int sched_init(void);

void scheduler(void);

void swtch(struct context **old, struct context *new);

void swtch_to(struct context *new);

void _jump_to_usermode(void *func, void *stack);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
