#ifndef _IDT_H
#define _IDT_H

#include <defs.h>

#define T_DIVIDE	0 /* Divide error */
#define T_DEBUG		1 /* Debug trap */
#define T_NMI		2 /* Non-Markable Interrupt */

#define IRQ_OFFSET	32 /* Vector offset for first IRQ */

#define IRQ_TIMER	0
#define IRQ_KBD		1

struct idt_desc {
	uint16_t offset_1;
	uint16_t selector;
	uint8_t	reserved_1;
	uint8_t type_attr;
	uint16_t offset_2;
	uint32_t offset_3;
	uint32_t reserved_2;
}__attribute__((packed));

void setup_idt(void);

#endif
