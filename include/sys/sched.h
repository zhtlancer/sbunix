#ifndef _SCHED_H
#define _SCHED_H

#include <defs.h>
#include <sys/mm.h>
#include <ps.h>

/* Enum values for Task states */
//enum TASK_STATE {
	//TASK_UNUSED = 0,	[> PCB not used <]
	//TASK_EMBRYO,		[> Process being initialized <]
	//TASK_SLEEPING,		[> Process sleeping <]
	//TASK_RUNNABLE,		[> Process ready to run <]
	//TASK_RUNNING,		[> Process running <]
	//TASK_ZOMBIE			[> Process died <]
//};

struct task_struct;

#define DEFAULT_USER_RFLAGS	0x200202

/* 
 * proc's context shared between IRQ and syscall(based on the layout of stack)
 * XXX: Be careful, changing this would affect the context switch procedure,
 * so corresponding changes in _switch_to_usermode needed.
 */
struct pt_regs {
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
};

/*
 * Structure for process (PCB)
 */
struct task_struct {
	/* Kernel stack, it's important that this is stored at very first, this
	 * can facilitate us accessing the kernel stack in asm
	 */
	void *stack;

	/* Saved context for this process (on kernel stack) */
	struct context *context;

	/* Saved trampframe for syscall/irq (on kernel stack) */
	struct pt_regs *tf;

	/* Process ID */
	volatile pid_t pid;

	/* Process state */
	enum TASK_STATE state;

	/* Parent process */
	struct task_struct *parent;

	mm_struct_t *mm;

	/* Saved CR3 register (PA to page table)*/
	addr_t cr3;

	/* indicate if this proc has been killed */

	/*
	 * TODO:
	 * # Filesystem related elements (current dir, opened files)
	 * # Other stuffs that required...
	 */
	struct file *files[NFILE_PER_PROC];

	struct inode *cwd;

	void *wait;		/* if non-null, sleeping on chan */

	int waitpid;	/* special wait object for waitpid call, 0: none, 1: waited */

	char name[16];
};

extern struct task_struct *current;

struct task_struct *duplicate_task(void);

int sched_init(void);

void scheduler(void);

void swtch(struct context **old, struct context *new);

void _switch_to_usermode(uint64_t cr3, void *stack);

pid_t fork(void);

int execve(const char *pathname, char *const argv[], char *const envp[]);

pid_t wait(int *status);

pid_t waitpid(pid_t pid, int *status, int options);

void exit(int status);

void sched(void);

void yield(void);

int kill(pid_t pid);

void sleep(void *wait_obj);

void wakeup_obj(void *wait_obj);

void wakeup_task_obj(struct task_struct *task, void *wait_obj);

int ps(struct ps_ent *ps_buf, int count);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
