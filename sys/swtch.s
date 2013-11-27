
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
	pushq %rsi
	pushq %rdi

	// Switch stack pointer
	movq %rsp, (%rdi)
	movq %rsi, %rsp

	// Restore new context
	popq %rdi
	popq %rsi
	popq %rbx
	popq %rax

	// Return to new context
	ret

.globl swtch_to
swtch_to:
	movq %rdi, %rsp
	popq %rdi
	popq %rsi
	popq %rbx
	popq %rax
	ret

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
