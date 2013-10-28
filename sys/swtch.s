
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

/* vim: set ts=4 sw=0 tw=0 noet : */
