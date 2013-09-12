#include <defs.h>

#include <console.h>
#include <printf.h>

void start(void* modulep, void* physbase, void* physfree)
{
	int rval = 0;

	// kernel starts here
	rval = init_console();
	if (rval != 0) {
		/* FIXME:
		 * put error handler here
		 */
	}

	rval = printf("Test\n");
	printf("rval = %d\n", rval);
	rval = printf("%%c: %c\n", 'j');
	printf("rval = %d\n", rval);
	rval = printf("%%s: %s\n", "deadbeef");
	printf("rval = %d\n", rval);
	rval = printf("%%d: %d\n", 1234567890);
	printf("rval = %d\n", rval);
	rval = printf("%%p: %p\n", 0);
	printf("rval = %d\n", rval);
	rval = printf("%%x: %x\n", 0xdeadbeef);
	printf("rval = %d\n", rval);

	while (1) /* We are not expected to return */
		;
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	volatile register char *rsp asm ("rsp");
	volatile register char *temp1, *temp2;
	loader_stack = (uint32_t*)rsp;
	rsp = &stack[INITIAL_STACK_SIZE];
	start(
		(char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase,
		&physbase,
		(void*)(uint64_t)loader_stack[4]
	);
	for(
		temp1 = "!!!!! start() returned !!!!!", temp2 = (char*)0xb8000;
		*temp1;
		temp1 += 1, temp2 += 2
	) *temp2 = *temp1;
	while(1);
}