#include <sys/sched.h>
#include <sys/k_stdio.h>
#include <sys/string.h>
#include <sys/mm_vma.h>
#include <sys/fs.h>

#include <sys/elf.h>
#include <sys/error.h>
#include <sys/gdt.h>
#include <sys/syscall.h>
#include <sys/x86.h>

#define sched_error(fmt, ...)	\
	k_printf(1, "<ELF> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_SCHED
#define sched_db(fmt, ...)	\
	k_printf(1, "<ELF DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define sched_db(fmt, ...)
#endif

#define TEST_SCHED 0

/* The very first userspace executable */
#define USER_INIT	"/bin/test"

/* The idle proc, which spin forever and never sleep */
#define USER_IDLE	"/bin/idle"

struct {
	/* FIXME: maybe we need a lock to protect this */
	struct task_struct tasks[NPROC];
} task_table;

struct task_struct *current;

struct context *scheduler_ctx;

static int alloc_pid(void)
{
	static int pid = 0;	/* This persisting variable records the pid allocated for last process */

	/* Here is a naive implementation of PID allocation */
	return ++pid;
}

/*
 * The main loop for SBUNIX
 * this function should never return
 */
void scheduler(void)
{
	static int idx = 0;
	struct task_struct *task;

	for ( ; ; ) {
		/* lock the table if necessary */
		if ((task = &task_table.tasks[idx])->state == TASK_RUNNABLE) {
			current = task;
			task->state = TASK_RUNNING;
			tss_set_kernel_stack(task->stack);
			lcr3(task->cr3);
			swtch(&scheduler_ctx, task->context);
		}

		idx = (idx + 1) % NPROC;
	}

	k_printf(0, "Oops! Why are we here?!\n");
}

/*
 * Allocate one task_struct for a new process, and initialize required elements
 */
struct task_struct *alloc_task(void)
{
	int i;
	struct task_struct *task;

	/* Lock the task_table if it has lock */

	/* Find an unused task_struct */
	for (i = 0;
			i < NPROC && task_table.tasks[i].state != TASK_UNUSED;
			i++)
		/*Empty*/;
	if (i == NPROC)
		return NULL;
	task = &task_table.tasks[i];

	/* allocate & assign a pid */
	task->pid = alloc_pid();
	if (task->pid < 0) {
		panic("Cannot allocate new PID for new process!");
	}

	/* Set its state as newly created */
	task->state = TASK_EMBRYO;

	/* Unlock task_table if it has lock */

	/* Allocate mm_struct for this process?
	 * XXX: actually we don't know where this process is from,
	 * so do this in create_task or duplicate_task
	 */

	return task;
}

/*
 * Free a task_struct (Note that this doesn't care about the resource,
 * so don't call this if the task still have resources holding in its
 * hand.
 */
void free_task(struct task_struct *task)
{
}

void fork_ret(void);

static mm_struct_t *create_uvm(const char *name, uint64_t *entry)
{
	mm_struct_t *mm = NULL;
	struct elf64_executable exe;
	int rval;
	page_t *page;

	memset(&exe, 0, sizeof(exe));
	strlcpy(exe.name, name, ELF_NAME_MAX);

	rval = parse_elf_executable(&exe);
	if (rval != 0) {
		sched_error("Failed to load elf file \"%s\" (%d)\n",
				exe.name, rval);
		return NULL;
	}

	/* Allocate mm_struct */
	mm = mm_struct_new((addr_t)exe.code_start,
			(addr_t)exe.code_start + exe.code_size,
			(addr_t)exe.data_start,
			(addr_t)exe.data_start + exe.data_size,
			0, 0, 0,
			exe.bss_size);

	/* Allocate physical memory and load elf file into it */
	load_elf(mm, &exe);

	*entry = (uint64_t)exe.entry;

	/* Allocate and map user stack */
	page = alloc_page(PG_USR);
	map_page(mm->pgt, USTACK_TOP - __PAGE_SIZE, 0,
			get_pa_from_page(page), PG_USR, 0, 0, 0, PGT_RW | PGT_USR);

	return mm;
}


/*
 * Create a process from scratch (ELF file)
 */
static struct task_struct *create_task(const char *name)
{
	struct elf64_executable exe;
	struct task_struct *task;
	struct pt_regs *regs;
	page_t *page;
	int rval = 0;
	volatile int d = 1;
	void *k_stack;

	while (d);
	task = alloc_task();
	if (task == NULL) {
		sched_error("Failed to alloc task\n");
		return NULL;
	}

	/* TODO: Clean up task */

	memset(&exe, 0, sizeof(exe));
	strlcpy(exe.name, name, ELF_NAME_MAX);

	rval = parse_elf_executable(&exe);
	if (rval != 0) {
		sched_error("Failed to parse elf file (%d)\n", rval);
		goto fail_task;
	}

	/* Allocate mm_struct */
	task->mm = mm_struct_new((addr_t)exe.code_start,
			(addr_t)exe.code_start + exe.code_size,
			(addr_t)exe.data_start,
			(addr_t)exe.data_start + exe.data_size,
			0, 0, 0,
			exe.bss_size);
	task->cr3 = get_pa_from_va(task->mm->pgt);

	/* Allocate physical memory and load elf file into it */
	load_elf(task->mm, &exe);

	while (d);
	/* Allocate and map user stack */
	page = alloc_page(PG_USR);
	map_page(task->mm->pgt, USTACK_TOP - __PAGE_SIZE, 0,
			get_pa_from_page(page), PG_USR, 0, 0, 0, PGT_RW | PGT_USR);

	/* Allocate kernel stack for it
	 * also initialize the stack content for first run */
	task->stack = (void *)alloc_page(PG_SUP)->va + __PAGE_SIZE;
	k_stack = task->stack - sizeof(struct pt_regs);
	regs = k_stack;
	regs->ss = 0x23;
	regs->rsp = USTACK_TOP;
	regs->rflags = DEFAULT_USER_RFLAGS;
	regs->cs = 0x2B;
	regs->rip = (uint64_t)exe.entry;
	regs->rax = 0x0;
	regs->rbx = 0x0;
	regs->rcx = (uint64_t)exe.entry;
	regs->rdx = 0x0;
	regs->rdi = 0x0;
	regs->rsi = 0x0;
	regs->rbp = 0x0;
	regs->r8 = 0x0;
	regs->r9 = 0x0;
	regs->r10 = 0x0;
	regs->r11 = DEFAULT_USER_RFLAGS;
	regs->r12 = 0x0;
	regs->r13 = 0x0;
	regs->r14 = 0x0;
	regs->r15 = 0x0;
	task->tf = regs;

	k_stack -= 8;
	*(uint64_t *)k_stack = (uint64_t)_syscall_lstar_ret;

	k_stack -= sizeof(struct context);
	task->context = (struct context *)k_stack;
	memset(task->context, 0, sizeof(struct context));
	task->context->rip = (uint64_t)fork_ret;

	/* duplicate stdin/stdout/stderr */
	task->files[0] = file_dup(&files[0]);
	task->files[1] = file_dup(&files[1]);
	task->files[2] = file_dup(&files[2]);

	task->state = TASK_RUNNABLE;

#if TEST_SCHED
	current = task;
	task->state = TASK_RUNNING;
	tss_set_kernel_stack(task->stack);
	_switch_to_usermode(task->cr3, task->tf);
#endif

	return task;

fail_task:
	free_task(task);
	return NULL;
}

/*
 * Duplicate current into a new process (for fork)
 */
struct task_struct *duplicate_task(void)
{
	struct task_struct *task;
	void *k_stack;
	int i;

	task = alloc_task();

	/* Duplicate vm and copy page mappings */
	task->mm = mm_struct_dup();
	task->cr3 = get_pa_from_va(task->mm->pgt);

	/* Allocate kernel stack, and fill it with sensible data */
	task->stack = (void *)alloc_page(PG_SUP)->va + __PAGE_SIZE;
	k_stack = task->stack - sizeof(struct pt_regs);
	task->tf = (struct pt_regs *)k_stack;

	k_stack -= 8;
	/* build syscall return point */
	*(uint64_t *)k_stack = (uint64_t)_syscall_lstar_ret;

	k_stack -= sizeof(struct context);
	task->context = (struct context *)k_stack;
	memset(task->context, 0, sizeof(struct context));
	task->context->rip = (uint64_t)fork_ret;

	/* Dup opened files */
	for (i = 0; i < NFILE_PER_PROC; i++) {
		if (current->files[i] != NULL)
			task->files[i] = file_dup(current->files[i]);
	}

	return task;
}

void fork_ret(void)
{
	/* Fake return call for forked child and first process
	 * no function should directly call this
	 */
}

pid_t fork(void)
{
	struct task_struct *new;

	new = duplicate_task();

	/* copy trapframe from parent */
	*new->tf = *current->tf;

	/* Set child's return value to 0 */
	new->tf->rax = 0;
	current->tf->rax = new->pid;

	new->state = TASK_RUNNABLE;
	
	return new->pid;
}

int execve(const char *pathname, char *const argv[], char *const envp[])
{
	mm_struct_t *new_mm;
	uint64_t entry;

	/* create new mm_struct */
	new_mm = create_uvm(pathname, &entry);
	if (new_mm == NULL) {
		sched_error("execve failed\n");
		return -1;
	}

	/* free previous mm_struct and resource */
	mm_struct_free(current->mm);

	/* switch to new mm_struct */
	current->mm = new_mm;
	current->cr3 = get_pa_from_va(new_mm->pgt);
	lcr3(current->cr3);

	/* free all opened files, and reopen stdin/stdout/stderr */
	for (int i = 0; i < NFILE_PER_PROC; i++) {
		if (current->files[i] != NULL) {
			file_put(current->files[i]);
			current->files[i] = NULL;
		}
	}

	current->files[0] = file_dup(&files[0]);
	current->files[1] = file_dup(&files[1]);
	current->files[2] = file_dup(&files[2]);

	/* TODO: setup up user stack based on argv and envp */

	/* rebuild trapframe */
	current->tf->rip = entry;
	current->tf->rcx = entry;
	current->tf->rsp = USTACK_TOP;

	return 0;
}

void sched(void)
{
	if (current->state == TASK_RUNNING) {
		sched_error("Task[%d] running!\n", current->pid);
		panic("Unexpected scheduler behavior!\n");
	}
	swtch(&current->context, scheduler_ctx);
}

void yield(void)
{
	current->state = TASK_RUNNABLE;
	sched();
}

int kill(pid_t pid)
{
	return -1;
}

int sched_init(void)
{
	int i;

	for (i = 0; i < NPROC; i++) {
		task_table.tasks[i].state = TASK_UNUSED;
	}

	/* Create the very first user space process */
	create_task(USER_INIT);

	/* Create the idle proc */
	create_task(USER_IDLE);

	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
