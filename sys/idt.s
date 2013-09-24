
# load IDT into IDTR
# param 1: address if IDT
.global _x86_64_asm_lidt
.type _x86_64_asm_lidt, @function
_x86_64_asm_lidt:
	lidt (%rdi)
	sti
	retq

.global _irqentry_generic
.type _irqentry_generic, @function
_irqentry_generic:
	movq $0, %rdi
	call intr_handler
	iretq

.extern _jiffies
.global _irqentry_timer
.type _irqentry_timer, @function
_irqentry_timer:
	call update_jiffies
	mov $0x20, %al
	out %al, $0x20
	iretq

.global _irqentry_kbd
.type _irqentry_kbd, @function
_irqentry_kbd:
	movq $0, %rax
	in $0x60, %al
	movq %rax, %rdi
	call kbd_intr_handler
	mov $0x20, %al
	out %al, $0x20
	iretq

