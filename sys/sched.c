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

#include <ps.h>

#define sched_error(fmt, ...)	\
	k_printf(1, "<SCHED> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_SCHED
#define sched_db(fmt, ...)	\
	k_printf(1, "<SCHED DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define sched_db(fmt, ...)
#endif

#define TEST_SCHED 0

/* The idle proc, which spin forever and run very first */
#define USER_IDLE	"/bin/idle"

#define MAXARGS	10
#define MAXARGV_FIELD	(MAXARGS + 2)
#define MAXARGV_LEN	20

#define MAXENVS	10
#define MAXENVS_FIELD	(MAXENVS + 2)
#define MAXENV_LEN	20

struct {
	/* FIXME: maybe we need a lock to protect this */
	struct task_struct tasks[NPROC];
} task_table;

struct task_struct *current;
struct task_struct *idle_task;

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
	/* TODO */

	/* Free task's kernel stack */
	free_page(task->stack - __PAGE_SIZE);
	task->stack = NULL;

	/* free all vma mapped into userspace */
	mm_struct_free(task->mm);
	task->mm = NULL;

	task->pid = 0;
	task->context = NULL;
	task->tf = NULL;
	task->state = TASK_UNUSED;
	task->parent = NULL;
	task->cr3 = NULL;
	/* files and cwd already cleaned at exit */
	task->wait = NULL;
	task->waitpid = 0;
}

void fork_ret(void);

static mm_struct_t *create_uvm(const char *name, uint64_t *entry)
{
	mm_struct_t *mm = NULL;
	struct elf64_executable exe;
	int rval;

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
	/* We moved this to execve, because we need to operate on stack there */

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
	void *k_stack;
#if TEST_SCHED
	volatile int d = 1;
#endif

#if TEST_SCHED
	while (d);
#endif
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

	/* Set cwd to root */
	task->cwd = get_inode(rootfs);

	task->state = TASK_RUNNABLE;

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

	/* Dup cwd */
	task->cwd = get_inode(current->cwd);

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

	new->parent = current;

	new->state = TASK_RUNNABLE;
	
	return new->pid;
}

int execve(const char *pathname, char *const argv[], char *const envp[])
{
	mm_struct_t *new_mm;
	uint64_t entry;
	char *tmp[MAXARGV_FIELD];
	void *stack;
	uint64_t ustack_top;
	size_t len;
	page_t *page;
	int i;

	/* create new mm_struct */
	new_mm = create_uvm(pathname, &entry);
	if (new_mm == NULL) {
		sched_error("execve failed\n");
		return -1;
	}

	page = alloc_page(PG_USR);
	map_page(new_mm->pgt, USTACK_TOP - __PAGE_SIZE, 0,
			get_pa_from_page(page), PG_USR, 0, 0, 0, PGT_RW | PGT_USR);

	/* TODO: setup up user stack based on argv and envp
	 * XXX: we need to do this before the previous uvm is freed */
	stack = get_va_from_page(page) + __PAGE_SIZE;
	ustack_top = USTACK_TOP;
	if (envp != NULL) {
		for (i = 0; i < MAXENVS && envp[i] != NULL; i++) {
			len = strnlen(envp[i], MAXENV_LEN) + 1;
			stack -= len;
			ustack_top -= len;
			strlcpy(stack, envp[i], MAXENV_LEN);
			tmp[i] = (char *)ustack_top;
		}
		tmp[i++] = NULL;
		len = sizeof(char *) * i;
		stack -= len;
		ustack_top -=len;
		memcpy(stack, tmp, sizeof(char *) * i);

		current->tf->rdx = (uint64_t) ustack_top;
	} else {
		current->tf->rdx = 0;
	}
	if (argv != NULL) {
		for (i = 0; i < MAXARGS && argv[i] != NULL; i++) {
			len = strnlen(argv[i], MAXARGV_LEN) + 1;
			stack -= len;
			ustack_top -= len;
			strlcpy(stack, argv[i], MAXARGV_LEN);
			tmp[i] = (char *)ustack_top;
		}
		tmp[i++] = NULL;
		len = sizeof(char *) * i;
		stack -= len;
		ustack_top -= len;
		memcpy(stack, tmp, sizeof(char *) * i);

		current->tf->rsi = (uint64_t) ustack_top;
		current->tf->rdi = i - 1;
	}

	/* free previous mm_struct and resource */
	mm_struct_free_self(current->mm);

	/* switch to new mm_struct */
	current->mm = new_mm;
	current->cr3 = get_pa_from_va(new_mm->pgt);
	lcr3(current->cr3);

	/* free all opened files, and reopen stdin/stdout/stderr */
	for (int i = 0; i < NFILE_PER_PROC; i++) {
		if (current->files[i] != NULL) {
			file_close(current->files[i]);
			current->files[i] = NULL;
		}
	}

	current->files[0] = file_dup(&files[0]);
	current->files[1] = file_dup(&files[1]);
	current->files[2] = file_dup(&files[2]);

	/* rebuild trapframe */
	current->tf->rip = entry;
	current->tf->rcx = entry;
	current->tf->rsp = ustack_top;

	return 0;
}

/* option values for waitpid */
#define WNOHANG 0x01

/* wait for all child */
static pid_t wait_all(int *status, int options)
{
	int have_kid = 0;
	pid_t pid;
	int i;
	struct task_struct *task;

	for ( ; ; ) {
		for (i = 0, task = task_table.tasks; i < NPROC; i++, task++) {
			if (task->parent != current)
				continue;
			have_kid = 1;
			if (task->state == TASK_ZOMBIE) {
				/* Get child's status, see exit */
				*status = task->tf->rax;
				pid = task->pid;

				/* Now free child task and release resource */
				free_task(task);

				return pid;
			}
		}

		if (!have_kid)
			return -1;

		/* return immediately if WNOHANG set */
		if (options & WNOHANG)
			return 0;

		sleep(current);
	}

	sched_error("Should not reach here! pid=%d\n", current->pid);

	return -1;
}

pid_t waitpid(pid_t pid, int *status, int options)
{
	struct task_struct *task;
	int i;
	/* wait for all children */
	if (pid == -1)
		return wait_all(status, options);

	for (i = 0, task = task_table.tasks; i < NPROC; i++, task++)
		if (task->pid == pid)
			break;

	if (i == NPROC) {
		sched_db("Child %d not found\n", pid);
		return -1;
	}

	if (task->state == TASK_ZOMBIE) {
		/* Get child's status, see exit */
		*status = task->tf->rax;
		pid = task->pid;

		/* Now free child task and release resource */
		free_task(task);

		return pid;
	}

	if (options & WNOHANG)
		return 0;

	if (task->waitpid != 0) {
		sched_error("Child(%d)'s waitpid is not zero, already waited?\n", task->pid);
		panic("Unexpected waitpid behavior!\n");
	}
	task->waitpid = 1;
	sleep(&task->waitpid);

	if (task->state == TASK_ZOMBIE) {
		/* Get child's status, see exit */
		*status = task->tf->rax;
		pid = task->pid;

		/* Now free child task and release resource */
		free_task(task);
	} else {
		sched_error("Child not exit? Why I'm waken up? (%d)\n", task->state);
	}

	return pid;
}

pid_t wait(int *status)
{
	return wait_all(status, 0);
}

void exit(int status)
{
	struct task_struct *task;
	int i;

	/* Close all opened files */
	for (i = 0; i < NFILE_PER_PROC; i++) {
		if (current->files[i] != NULL) {
			file_close(current->files[i]);
			current->files[i] = NULL;
		}
	}

	/* free process' cwd */
	put_inode(current->cwd);

	/* wakeup parent if they are wating */
	/* first check waitpid */
	if (current->waitpid) {
		wakeup_task_obj(current->parent, &current->waitpid);
	} else {
		wakeup_obj(current->parent);
	}

	/* Pass child to idle process */
	for (i = 0, task = task_table.tasks; i < NPROC; i++, task++) {
		if (task->parent == current)
			task->parent = idle_task;
	}

	/* store process' exit status */
	current->tf->rax = status;

	current->state = TASK_ZOMBIE;
	sched();
	panic("Returning to a ZOMBIE!!!\n");
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

/* Put current proc into sleep */
void sleep(void *wait_obj)
{
	if (wait_obj == NULL) {
		sched_error("Why we want proc(%d) sleep on NULL?\n", current->pid);
		panic("Unexpected sleep behavior!\n");
	}

	if (current->state != TASK_RUNNING) {
		sched_error("Why a non-running proc(%d) wants to sleep?\n", current->pid);
		panic("Unexpected sleep behavior!\n");
	}

	if (current->wait != NULL) {
		sched_error("Why this proc(%d) have non-null wait(%p)?\n", current->pid, current->wait);
		panic("Unexpected sleep behavior!\n");
	}

	current->wait = wait_obj;
	current->state = TASK_SLEEPING;
	sched();
}

/* Find an wakeup any tasks sleep on wait_obj */
void wakeup_obj(void *wait_obj)
{
	int i;
	struct task_struct *tasks = task_table.tasks;

	for (i = 0; i < NPROC; i++) {
		if (tasks[i].state == TASK_SLEEPING && tasks[i].wait == wait_obj) {
			tasks[i].wait = NULL;
			tasks[i].state = TASK_RUNNABLE;
		}
	}
}

/* wakeup task, who is waiting on wait_obj */
void wakeup_task_obj(struct task_struct *task, void *wait_obj)
{
	if (task->state != TASK_SLEEPING) {
		sched_error("Why we need to wakeup a non-sleeping proc(%d)?\n", task->pid);
		panic("Unexpected wakeup behavior!\n");
	}

	if (task->wait != wait_obj) {
		sched_error("Proc(%d) is waiting on %p instead of %p?\n", task->pid, task->wait, wait_obj);
		panic("Unexpected wakeup behavior!\n");
	}

	task->wait = NULL;
	task->state = TASK_RUNNABLE;
}

int sched_init(void)
{
	int i;

	for (i = 0; i < NPROC; i++) {
		task_table.tasks[i].state = TASK_UNUSED;
	}

	/* Create the idle proc */
	idle_task = create_task(USER_IDLE);
	if (idle_task == NULL) {
		panic("Failed to create idle process\n");
	}

	return 0;
}

int ps(struct ps_ent *ps_buf, int count)
{
	int i, j = 0;

	for (i = 0; i < NPROC && j < count; i++)
		if (task_table.tasks[i].state != TASK_UNUSED) {
			ps_buf[j].pid = task_table.tasks[i].pid;
			ps_buf[j].state = task_table.tasks[i].state;
			j++;
		}

	return j;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
