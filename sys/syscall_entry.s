# This file contains the entries for syscalls

.globl _syscall_lstar
_syscall_lstar:
	/* we build the context structure for future use (in user stack) */
	pushq %rcx	/* user rip, as after syscall instr, user rip is saved in rcx */
	pushq %r11	/* user rflags, see above */
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

	movq %rsp, %rdi
	/* switch to kernel stack, and save user stack pointer */
	movq current, %rax
	movq 8(%rax), %rsp
	pushq %rdi
	callq syscall_common

	/* syscall finished, returning */
	/* restore user stack */
	popq %rsp

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
	addq $8, %rsp	/* don't pop rax here, as it holds return value now*/
	popq %r11	/* rflags, see above */
	popq %rcx	/* rip, see above */
	sysretq


/* Calling from compatible mode, actually we don't expect this */
.globl _syscall_cstar
_syscall_cstar:
	jmp .

/* vim: set ts=4 sw=0 tw=0 noet : */
