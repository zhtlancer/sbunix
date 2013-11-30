#ifndef _SCHED_H
#define _SCHED_H

#include <defs.h>
#include <sys/mm.h>

/* Enum values for Task states */
enum TASK_STATE {
	TASK_UNUSED = 0,	/* PCB not used */
	TASK_EMBRYO,		/* Process being initialized */
	TASK_SLEEPING,		/* Process sleeping */
	TASK_RUNNABLE,		/* Process ready to run */
	TASK_RUNNING,		/* Process running */
	TASK_ZOMBIE			/* Process died */
};

struct task_struct;

#define DEFAULT_USER_RFLAGS	0x200202

/* 
 * proc's context shared between IRQ and syscall(based on the layout of stack)
 * XXX: Be careful, changing this would affect the context switch procedure,
 * so corresponding changes in _switch_to_usermode needed.
 */
struct context {
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	/* we don't save rsp here */
	uint64_t rbp;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
};

/*
 * Structure for process (PCB)
 */
struct task_struct {
	/* Saved context for this process (userspace stack) */
	struct context *context;

	/* Kernel stack */
	void *stack;

	/* Process ID */
	volatile pid_t pid;

	/* Process state */
	enum TASK_STATE state;

	/* Parent process */
	struct task_struct *parent;

	mm_struct_t *mm;

	/* Saved CR3 register (PA to page table)*/
	addr_t cr3;

	/*
	 * TODO:
	 * # Filesystem related elements (current dir, opened files)
	 * # Other stuffs that required...
	 */
	struct file *files[NFILE_PER_PROC];

	struct inode *cwd;

	char name[16];
};

extern struct task_struct *current;

struct task_struct *create_task(const char *name);

struct task_struct *duplicate_task(void);

int sched_init(void);

void scheduler(void);

void swtch(struct context **old, struct context *new);

void swtch_to(struct context *new);

void _switch_to_usermode(uint64_t cr3, void *stack);

void _jump_to_usermode(void *func, void *stack);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
