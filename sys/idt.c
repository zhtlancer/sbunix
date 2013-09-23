#include <sys/idt.h>
#include <printf.h>

#define MAX_IDT 255
#define NUM_IDT 256

struct idt_desc idt[NUM_IDT] = { { 0 } };

struct idtr_t {
	uint16_t size;
	uint64_t addr;
}__attribute__((packed));

static struct idtr_t idtr = {
	sizeof(idt),
	(uint64_t)idt,
};

void _x86_64_asm_lidt(struct idtr_t *idtr);
void _irqentry_generic(void);
void _irqentry_timer(void);
void _irqentry_kbd(void);

static void idt_init(void)
{
	int i;

	for (i = 0; i < NUM_IDT; i += 1) {
		idt[i].selector = 0x08;
		idt[i].type_attr = 0x8E;
		idt[i].offset_1 = (uint64_t)_irqentry_generic & 0xFFFF;
		idt[i].offset_2 = ((uint64_t)_irqentry_generic >> 16)  & 0xFFFF;
		idt[i].offset_3 = ((uint64_t)_irqentry_generic >> 32)  & 0xFFFFFFFF;
	}

	/* IRQ0 for timer */
	idt[IRQ_OFFSET + IRQ_TIMER].selector = 0x08;
	idt[IRQ_OFFSET + IRQ_TIMER].type_attr = 0x8E;
	idt[IRQ_OFFSET + IRQ_TIMER].offset_1 = (uint64_t)_irqentry_timer & 0xFFFF;
	idt[IRQ_OFFSET + IRQ_TIMER].offset_2 = ((uint64_t)_irqentry_timer >> 16)  & 0xFFFF;
	idt[IRQ_OFFSET + IRQ_TIMER].offset_3 = ((uint64_t)_irqentry_timer >> 32)  & 0xFFFFFFFF;

	/* IRQ0 for keyboard */
	idt[IRQ_OFFSET + IRQ_KBD].selector = 0x08;
	idt[IRQ_OFFSET + IRQ_KBD].type_attr = 0x8E;
	idt[IRQ_OFFSET + IRQ_KBD].offset_1 = (uint64_t)_irqentry_kbd & 0xFFFF;
	idt[IRQ_OFFSET + IRQ_KBD].offset_2 = ((uint64_t)_irqentry_kbd >> 16)  & 0xFFFF;
	idt[IRQ_OFFSET + IRQ_KBD].offset_3 = ((uint64_t)_irqentry_kbd >> 32)  & 0xFFFFFFFF;
}

void setup_idt(void)
{
	idt_init();

	_x86_64_asm_lidt(&idtr);
}

void intr_handler(unsigned long vector_num)
{
	printf("Handler called, vector = %x\n", vector_num);
}
