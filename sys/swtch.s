
/*
 * Context switch
 * void swtch(struct context **old, struct context *new);
 * save current context into "old" and switch to the context
 * provided in "new"
 */
.globl swtch
swtch:
	// Save old context
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	// Switch stack pointer
	movq %rsp, (%rdi)
	movq %rsi, %rsp

	// Restore new context
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rax

	// Return to new context
	ret

/* 1: CR3, 2: target RSP, 3: target RIP */
.globl _switch_to_usermode
_switch_to_usermode:
	/* FIXME: Now we disable IRQ first */
	cli
	movw $0x23, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	/* Switch CR3 */
	mov	%rdi, %cr3
	mov %rsi, %rsp

	/* restore other registers */
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rax
	popq %rcx		/* RIP */
	addq $8, %rsp	/* skip CS */
	popq %r11		/* RFLAGS */
	popq %rsp		/* user stack */
	
	sysretq

.globl _jump_to_usermode
_jump_to_usermode:
/* Now we disable IRQ first */
	cli
	movw $0x23, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	mov %rsi, %rsp
	push $0x23	/* %ss */
	push %rsi
	pushfq
	push $0x1B	/* %cs */
	push %rdi	/* The function pointer passed in */
	iretq

/* vim: set ts=4 sw=0 tw=0 noet : */
