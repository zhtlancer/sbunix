#include <defs.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/io.h>
#include <sys/pic.h>
#include <sys/pit.h>
#include <sys/k_stdio.h>
#include <sys/sched.h>
#include <sys/tarfs.h>

#include <sys/mm.h>

void start(uint32_t* modulep, void* physbase, void* physfree)
{
	/* set vgatext virtual address base */
	vgatext_vbase = ((addr_t)&kernofs)+VGATEXT_PBASE;

	mm_init(modulep, physbase, physfree);

	__asm__("sti");
	__asm__("sti");

	sched_init();

	tarfs_init();

	/* Now we are calling the main loop, and should never return */
	scheduler();

	k_printf(0, "Oops! Why are we here?!\n");

	// kernel starts here
	while(1);
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	register char *temp1, *temp2;

	__asm__(
			"movq %%rsp, %0;"
			"movq %1, %%rsp;"
			:"=g"(loader_stack)
			:"r"(&stack[INITIAL_STACK_SIZE])
		   );
	reload_gdt();
	setup_tss();


	reload_idt();

	pic_init();
	idt_setup();

	PIT_init( 1000 );


	//__asm__("sti");
	//__asm__("sti");
	start(
			(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
			&physbase,
			(void*)(uint64_t)loader_stack[4]
		 );
	for(
			temp1 = "!!!!! start() returned !!!!!", temp2 = (char*)((addr_t)&kernofs+0xB8000);
			*temp1;
			temp1 += 1, temp2 += 2
	   ) *temp2 = *temp1;
	while(1);
} /* boot() */

/* vim: set ts=4 sw=0 tw=0 noet : */
