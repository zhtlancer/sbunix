#include <sys/sched.h>
#include <sys/k_stdio.h>

struct {
	/* FIXME: maybe we need a lock to protect this */
	struct task_struct tasks[NPROC];
} task_table;

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

int
sched_init(void)
{
	return 0;
}

/*
 * The main loop for SBUNIX
 * this function should never return
 */
void scheduler(void)
{
	/* FIXME: This is a swtch test, remove this */
	{
		pa->rip = (uint64_t)&a;

		pb->rip = (uint64_t)&b;
		pb->rax = 0;
		pb->rbx = 0;
		pb->rsi = 0;
		pb->rdi = 0;
		swtch_to(pa);
	}

	for ( ; ; ) {
	}

	k_printf(0, "Oops! Why are we here?!\n");
}

/* vim: set ts=4 sw=0 tw=0 noet : */
