#include <sys/idt.h>
#include <printf.h>

#define MAX_IDT 255

struct idt_desc idt[MAX_IDT+1] = { { 0 } };

void _x86_64_asm_lidt(struct idt_desc *idt);
void _irqentry_timer(void);
void _irqentry_kbd(void);

static void idt_init(void)
{
	/* IRQ0 for timer */
	idt[IRQ_OFFSET + IRQ_TIMER].selector = 0x08;
	idt[IRQ_OFFSET + IRQ_TIMER].type_attr = 0xE;
	idt[IRQ_OFFSET + IRQ_TIMER].offset_1 = (uint64_t)_irqentry_timer & 0xFFFF;
	idt[IRQ_OFFSET + IRQ_TIMER].offset_2 = ((uint64_t)_irqentry_timer >> 16)  & 0xFFFF;
	idt[IRQ_OFFSET + IRQ_TIMER].offset_3 = ((uint64_t)_irqentry_timer >> 32)  & 0xFFFFFFFF;

	/* IRQ0 for keyboard */
	idt[IRQ_OFFSET + IRQ_KBD].selector = 0x08;
	idt[IRQ_OFFSET + IRQ_KBD].type_attr = 0xE;
	idt[IRQ_OFFSET + IRQ_KBD].offset_1 = (uint64_t)_irqentry_kbd & 0xFFFF;
	idt[IRQ_OFFSET + IRQ_KBD].offset_2 = ((uint64_t)_irqentry_kbd >> 16)  & 0xFFFF;
	idt[IRQ_OFFSET + IRQ_KBD].offset_3 = ((uint64_t)_irqentry_kbd >> 32)  & 0xFFFFFFFF;
}

void setup_idt(void)
{
	idt_init();

	_x86_64_asm_lidt(idt);
}

void irq_handler(void)
{
	printf("Handler called\n");
}
