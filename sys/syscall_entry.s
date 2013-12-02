# This file contains the entries for syscalls

.globl _syscall_lstar
_syscall_lstar:
	/* switch to kernel stack, and build pt_regs structure */
	movq (current), %r10
	movq (%r10), %r10
	xchgq %rsp, %r10

	pushq $0x23	/* user SS */
	pushq %r10	/* user stack */
	pushq %r11	/* user rflags */
	pushq $0x2B /* user CS */
	pushq %rcx	/* user rip */

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

	movq %rsp, %rdi		/* pointer of pt_regs strucure */
	callq syscall_common

	/* syscall finished, restore & returning */
.globl _syscall_lstar_ret
_syscall_lstar_ret:
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
	popq %rcx		/* rip, see above */
	addq $8, %rsp	/* Skip CS */
	popq %r11		/* rflags, see above */
	popq %rsp		/* rsp */
	sysretq

/* Calling from compatible mode, actually we don't expect this */
.globl _syscall_cstar
_syscall_cstar:
	jmp .

/* vim: set ts=4 sw=0 tw=0 noet : */
