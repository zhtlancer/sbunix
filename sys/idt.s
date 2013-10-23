#
# idt.s
#

.text

.global _x86_64_asm_lidt
_x86_64_asm_lidt:

	lidt (%rdi)

	retq



.global x86_64_asm_irq_32
x86_64_asm_irq_32:
    pushq   $32
    pushq   %rdi
    movabsq $._x86_64_asm_irq_common, %rdi   
    jmpq    *%rdi

.global x86_64_asm_irq_33
x86_64_asm_irq_33:
    pushq   $33
    pushq   %rdi
    movabsq $._x86_64_asm_irq_common, %rdi   
    jmpq    *%rdi

._x86_64_asm_irq_common:

    movq    8(%rsp), %rdi

save_all_regs:
#   pushq   %rdi // already pushed by individual isr
    pushq   %rax
    pushq   %rbx
    pushq   %rcx
    pushq   %rdx
    pushq   %rbp
    pushq   %rsi
    pushq   %r8
    pushq   %r9
    pushq   %r10
    pushq   %r11
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15

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
    popq    %rsi
    popq    %rbp
    popq    %rdx
    popq    %rcx
    popq    %rbx
    popq    %rax
    popq    %rdi
    addq    $8, %rsp

    iretq

