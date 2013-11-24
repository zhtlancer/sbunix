#include <defs.h>
#include <sys/idt.h>
#include <sys/pic.h>
#include <sys/io.h>

void PIC_eoi(unsigned char irq)
{
	if(irq >= 8)
		outb(PIC2_CMD,PIC_EOI);

	outb(PIC1_CMD,PIC_EOI);
} 

/* Helper func */
static uint16_t __pic_get_irq_reg(int ocw3)
{
	/* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
	 *      * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
	outb(PIC1_CMD, ocw3);
	outb(PIC2_CMD, ocw3);
	return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

/* Returns the combined value of the cascaded PICs irq request register */
uint16_t PIC_get_irr(void)
{
	return __pic_get_irq_reg(PIC_READ_IRR);
}

/* Returns the combined value of the cascaded PICs in-service register */
uint16_t PIC_get_isr(void)
{
	return __pic_get_irq_reg(PIC_READ_ISR);
} 

/*
 * arguments:
 *  offset1 - vector offset for master PIC
 *          vectors on the master become offset1..offset1+7
 *              offset2 - same for slave PIC: offset2..offset2+7
 *              */
void PIC_remap(int offset1, int offset2)
{
	unsigned char a1, a2;

	a1 = inb(PIC1_DATA);                        // save masks
	a2 = inb(PIC2_DATA);

	outb(PIC1_CMD, ICW1_INIT+ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_CMD, ICW1_INIT+ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, a1);   // restore saved masks.
	outb(PIC2_DATA, a2);
}

/* FIXME: maybe we can change to pass these entries by a registration function */
extern void isr_pit();
extern void isr_kbd();
extern void isr_page_fault();

uint64_t current_irq;

void isr_common(uint64_t irq)
{
	current_irq = irq;

	if (irq == 14) isr_page_fault();
	if (irq == 32) isr_pit();
	if (irq == 33) isr_kbd();
} 

extern void x86_64_asm_irq_14();
extern void x86_64_asm_irq_32();
extern void x86_64_asm_irq_33();


int idt_setup(void)
{
    int_gate_t *pit_int_gate;
	uint64_t isr_addr;

	/* Page Fault */
    pit_int_gate = (void *)(&idt[2*14]);
    isr_addr = (uint64_t)(&x86_64_asm_irq_14);
    pit_int_gate->offsetLo  = (uint16_t)(isr_addr&0xFFFF); 
    pit_int_gate->segSel    = (uint16_t)0x8; 
    pit_int_gate->attr      = (uint16_t)(TYPE_IG64|DESC_P|DESC_DPL0); 
    pit_int_gate->offsetMi  = (uint16_t)((isr_addr>>16)&0xFFFF); 
    pit_int_gate->offsetHi  = (uint32_t)((isr_addr>>32)&0xFFFFFFFF); 
    pit_int_gate->resZero   = (uint32_t)0;


	/* PIT */
    pit_int_gate = (void *)(&idt[2*32]);
    isr_addr = (uint64_t)(&x86_64_asm_irq_32);
    pit_int_gate->offsetLo  = (uint16_t)(isr_addr&0xFFFF); 
    pit_int_gate->segSel    = (uint16_t)0x8; 
    pit_int_gate->attr      = (uint16_t)(TYPE_IG64|DESC_P|DESC_DPL0); 
    pit_int_gate->offsetMi  = (uint16_t)((isr_addr>>16)&0xFFFF); 
    pit_int_gate->offsetHi  = (uint32_t)((isr_addr>>32)&0xFFFFFFFF); 
    pit_int_gate->resZero   = (uint32_t)0;

	/* keyboard */
    pit_int_gate = (void *)(&idt[2*33]);
    isr_addr = (uint64_t)(&x86_64_asm_irq_33);
    pit_int_gate->offsetLo  = (uint16_t)(isr_addr&0xFFFF); 
    pit_int_gate->segSel    = (uint16_t)0x8; 
    pit_int_gate->attr      = (uint16_t)(TYPE_IG64|DESC_P|DESC_DPL0); 
    pit_int_gate->offsetMi  = (uint16_t)((isr_addr>>16)&0xFFFF); 
    pit_int_gate->offsetHi  = (uint32_t)((isr_addr>>32)&0xFFFFFFFF); 
    pit_int_gate->resZero   = (uint32_t)0;

	return 0;
}

int pic_init(void)
{
    PIC_remap( 32, 40 ); /* 0x20, 0x28 */
	
	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
