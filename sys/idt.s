#
# idt.s
#

.text

.global _x86_64_asm_lidt
_x86_64_asm_lidt:

	lidt (%rdi)

	retq

/* Page-fault exception */
.global x86_64_asm_irq_14
x86_64_asm_irq_14:
	xchgq	(%rsp), %rax		/* error code */
	pushq	%rbx
	pushq	%rcx
    movq	$14, %rbx			/* interrupt number */
    movabsq $._x86_64_asm_irq_common, %rcx
    jmpq    *%rcx

/* PIT timer */
.global x86_64_asm_irq_32
x86_64_asm_irq_32:
	pushq	%rax
	pushq	%rbx
	pushq	%rcx
    movq	$32, %rbx			/* interrupt number */
	movq	$0, %rax			/* error code */
    movabsq $._x86_64_asm_irq_common, %rcx
    jmpq    *%rcx

/* KBD */
.global x86_64_asm_irq_33
x86_64_asm_irq_33:
	pushq	%rax
	pushq	%rbx
	pushq	%rcx
    movq	$33, %rbx			/* interrupt number */
	movq	$0, %rax			/* error code */
    movabsq $._x86_64_asm_irq_common, %rcx
    jmpq    *%rcx

._x86_64_asm_irq_common:

save_all_regs:
	// rax, rbx, rcx already pushed by individual isr
	pushq   %rdx
	pushq	%rdi
	pushq   %rsi
	pushq   %rbp
	pushq   %r8
	pushq   %r9
	pushq   %r10
	pushq   %r11
	pushq   %r12
	pushq   %r13
	pushq   %r14
	pushq   %r15

	movq	%rbx, %rdi	/* irq no. */
	movq	%rax, %rsi	/* error code */
	movq	%rsp, %rdx	/* pt_regs */

    call    isr_common

restore_all_regs:
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %r11
    popq    %r10
    popq    %r9
    popq    %r8
    popq    %rbp
    popq    %rsi
	popq	%rdi
    popq    %rdx
    popq    %rcx
    popq    %rbx
    popq    %rax

    iretq

/* vim: set ts=4 sw=0 tw=0 noet : */
