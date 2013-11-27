#include <sys/sched.h>
#include <sys/k_stdio.h>
#include <sys/string.h>
#include <sys/mm_vma.h>

#include <sys/elf.h>
#include <sys/error.h>

#define sched_error(fmt, ...)	\
	k_printf(1, "<ELF> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_SCHED
#define sched_db(fmt, ...)	\
	k_printf(1, "<ELF DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define sched_db(fmt, ...)
#endif

#define TEST_SCHED 0

struct {
	/* FIXME: maybe we need a lock to protect this */
	struct task_struct tasks[NPROC];
} task_table;

#if TEST_SCHED
uint8_t stack_a[1024];
uint8_t stack_b[1024];

struct context *pa = (struct context *)(stack_a+1024-sizeof(struct context));
struct context *pb = (struct context *)(stack_b+1024-sizeof(struct context));

static void a(void)
{
	for (;;) {
		k_printf(0, "Hello\n");
		swtch(&pa, pb);
	}
}

static void b(void)
{
	for (;;) {
		k_printf(0, "World\n");
		swtch(&pb, pa);
	}
}
#endif

static int alloc_pid(void)
{
	static int pid = 0;	/* This persisting variable records the pid allocated for last process */

	/* Here is a naive implementation of PID allocation */
	return ++pid;
}

int sched_init(void)
{
	int i;

	for (i = 0; i < NPROC; i++) {
		task_table.tasks[i].state = TASK_UNUSED;
	}

	/* Create the very first user space process */
	create_task("bin/test");

	return 0;
}

#if TEST_SCHED
void usermode()
{
	for ( ; ; )
		;
}

static void sched_test()
{
	volatile int d=1;
	while (d);
	_jump_to_usermode(usermode, (void *)USTACK_TOP);
}
#endif

/*
 * The main loop for SBUNIX
 * this function should never return
 */
void scheduler(void)
{
#if TEST_SCHED
	sched_test();
	/* FIXME: This is a swtch test, remove this */
	if (0) {
		pa->rip = (uint64_t)&a;
		pb->rip = (uint64_t)&b;
		pb->rax = 0;
		pb->rbx = 0;
		pb->rsi = 0;
		pb->rdi = 0;

		swtch_to(pa);
	}
#endif

	for ( ; ; ) {
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

/*
 * Create a process from scratch (ELF file)
 */
struct task_struct *create_task(const char *name)
{
	struct elf64_executable exe;
	struct task_struct *task;
	page_t *page;
	uint64_t *temp;
	int rval = 0;
	volatile int d = 1;

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
	load_elf(task, &exe);

	while (d);
	/* Allocate and map user stack 
	 * also initialize the stack content for first run */
	page = alloc_page(PG_USR | PGT_RW);
	map_page(task->mm->pgt, USTACK_TOP - __PAGE_SIZE, 0,
			get_pa_from_page(page), PG_USR | PGT_RW, 0, 0, 0, PGT_RW | PGT_USR);
	temp = (uint64_t *)(page->va + __PAGE_SIZE - 8);
	*temp-- = (uint64_t)exe.entry;
	*temp-- = 0x0;
	*temp-- = 0x0;
	*temp-- = 0x0;
	*temp-- = 0x0;
	*temp-- = 0x0;
	*temp-- = 0x0;
	task->context = (struct context *)(USTACK_TOP - 8 * 7);
	task->rip = (uint64_t)exe.entry;

	while (d);
	/* Allocate kernel stack for it */
	task->stack = (void *)alloc_page(PG_SUP)->va + __PAGE_SIZE - 8;

	_switch_to_usermode(task->cr3, task->context, (void *)task->rip);

	return task;

fail_task:
	free_task(task);
	return NULL;
}

/*
 * Duplicate a process into a new process (like fork)
 */
struct task_struct *duplicate_task(struct task_struct *parent)
{
	/* Allocate mm_struct */

	return NULL;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
