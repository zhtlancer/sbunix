
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
	call irq_handler
	iretq

.global _irqentry_timer
.type _irqentry_timer, @function
_irqentry_timer:
	call irq_handler
	iretq

.global _irqentry_kbd
.type _irqentry_kbd, @function
_irqentry_kbd:
	call irq_handler
	iretq

