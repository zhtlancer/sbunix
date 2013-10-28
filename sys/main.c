#include <defs.h>
#include <sys/gdt.h>
#include <sys/k_stdio.h>
#include <sys/sched.h>

#include <sys/mm.h>

void start(uint32_t* modulep, void* physbase, void* physfree)
{
    struct smap_t {
	uint64_t base, length;
	uint32_t type;
    }__attribute__((packed)) *smap;

    uint64_t mem_length = 0;
    uint64_t page_struct_size = 0;
    uint64_t page_struct_page = 0;


    uint32_t kernel_page_num = 0;

    addr_t addr;

    /* set vgatext virtual address base */
    vgatext_vbase = ((addr_t)&kernofs)+VGATEXT_PBASE;

    while(modulep[0] != 0x9001) modulep += modulep[1]+2;
    k_printf( 0, "modulep : %x\n", modulep );
    k_printf( 0, "physbase: %x\n", physbase);
    k_printf( 0, "physfree: %x\n", physfree);
    page_index_begin = 0;
    for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
	if (smap->type == 1 /* memory */ && smap->length != 0) {
	    k_printf( 0, "Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
	    mem_length = smap->length;
	    page_num   = mem_length>>__PAGE_SIZE_SHIFT;
	    page_index_begin = (uint32_t)((smap->base)>>__PAGE_SIZE_SHIFT);
	    kernel_page_num = (uint32_t)((uint64_t)physfree>>__PAGE_SIZE_SHIFT)-page_index_begin;
	}
    }

    k_printf( 0, "memsize : %x\n", mem_length );
    k_printf( 0, "pagenum : %x\n", page_num   );

    k_printf( 0, "pg idx begin: %x\n", page_index_begin );
    k_printf( 0, "k  page num : %x\n", kernel_page_num  );

    page_struct_size = (mem_length>>__PAGE_SIZE_SHIFT)<<__PAGE_STRUCT_SIZE_SHIFT;
    page_struct_page = (page_struct_size>>__PAGE_SIZE_SHIFT)+( (page_struct_size&__PAGE_SIZE_MASK)?1:0);

    k_printf( 0, "pg stru size: %x\n", page_struct_size );
    k_printf( 0, "pg stru page: %x\n", (page_struct_size>>__PAGE_SIZE_SHIFT)+( (page_struct_size&__PAGE_SIZE_MASK)?1:0) );

    /* important */
    page_struct_begin = (page_t *)(((addr_t)&kernofs)+physfree);
    k_printf( 0, "pg stru begin: %p\n", page_struct_begin);

    page_begin_addr = (addr_t)page_struct_begin + (page_struct_page<<__PAGE_SIZE_SHIFT);
    kvma_end        = page_begin_addr; /* important */

    k_printf( 0, "pg begin addr: %p\n", page_begin_addr);

    page_begin      = page_begin_addr>>__PAGE_SIZE_SHIFT;
    k_printf( 0, "pg begin     : %p\n", page_begin     );

    def_pgt_paddr_lv1 = (addr_t)physbase  - __PAGE_SIZE ;
    def_pgt_paddr_lv2 = def_pgt_paddr_lv1 - __PAGE_SIZE;
    def_pgt_paddr_lv3 = def_pgt_paddr_lv2 - __PAGE_SIZE;

    k_printf( 0, "default lv1 page table phy addr: %x\n", def_pgt_paddr_lv1 );
    k_printf( 0, "default lv2 page table phy addr: %x\n", def_pgt_paddr_lv2 );
    k_printf( 0, "default lv3 page table phy addr: %x\n", def_pgt_paddr_lv3 );

    /* initialize  default page tables */
    int i;
    init_pgt( def_pgt_paddr_lv1 );
    init_pgt( def_pgt_paddr_lv2 );
    init_pgt( def_pgt_paddr_lv3 );
    for ( i=0; i<(0x100000/__PAGE_SIZE)-3; ++i ) {
	init_pgt( 0x100000+(i*__PAGE_SIZE) );
    }

    /* set lv1 page table entry: self-reference entry */ 
    addr = ( (addr_t)&kernofs | def_pgt_paddr_lv1 ) + (8*PGT_ENTRY_LV1_SELFREF);
    set_pgt_entry( addr, def_pgt_paddr_lv1, PGT_P, PGT_EXE,
	    0x0, 0x0, PGT_RW | PGT_SUP );

    /* set lv1 page table entry: kernel page */
    addr = ( (addr_t)&kernofs | def_pgt_paddr_lv1 ) + (8*PGT_ENTRY_LV1_KERNEL );
    set_pgt_entry( addr, def_pgt_paddr_lv2, PGT_P, PGT_EXE,
	    0x0, 0x0, PGT_RW | PGT_SUP );

    /* set lv2 page table entry: kernel page */
    addr = ( (addr_t)&kernofs | def_pgt_paddr_lv2 ) + (8*PGT_ENTRY_LV2_KERNEL );
    set_pgt_entry( addr, def_pgt_paddr_lv3, PGT_P, PGT_EXE,
	    0x0, 0x0, PGT_RW | PGT_SUP );

    /* set lv3 page table entry: kernel page:2M:0x00000-0x1FFFFF */
    addr = ( (addr_t)&kernofs | def_pgt_paddr_lv3 ) + (8*  0);
    set_pgt_entry( addr, 0x000000, PGT_P, PGT_EXE,
	    0x0, 0x0, PGT_RW | PGT_SUP | PGT_PS );

    /* set lv3 page table entry: kernel page:2M:0x20000-0x3FFFFF */
    addr = ( (addr_t)&kernofs | def_pgt_paddr_lv3 ) + (8*  1);
    set_pgt_entry( addr, 0x200000, PGT_P, PGT_EXE,
	    0x0, 0x0, PGT_RW | PGT_SUP | PGT_PS );

    /* set other lv3 page table entries: kernel page: 4K */ 
    int j = 0;
    for ( i=2; i<(0x100000/__PAGE_SIZE)-3; ++i, ++j ) {
	addr = ( (addr_t)&kernofs | def_pgt_paddr_lv3 ) + (8*i);
	set_pgt_entry( addr, 0x100000+(j*__PAGE_SIZE), PGT_P, PGT_EXE,
		0x0, 0x0, PGT_RW | PGT_SUP );
    }



    /* load default CR3 */
    asm volatile("movq %0, %%cr3":: "b"((void *)def_pgt_paddr_lv1));

    k_printf( 0, "After reload CR3\n" );

    /* init page structure */
    //init_page( kernel_page_num );
    init_page( 0x300 ); /* FIXME: First 4MB mapped */

    k_printf( 0, "find_free_page: %x\n", find_free_pages( 0x7bff  ) );
    k_printf( 0, "find_free_page: %x\n", find_free_pages( 0x7bfe  ) );

    kvma_end =(addr_t)&kernofs + 0x400000; /* FIXME: first 4MB mapped*/ 
    set_vma( &kvma_head, (addr_t)&kernofs, kvma_end,
	    NULL, 0, 0, 0, 0, 0, 0 );

    uint32_t reg_temp_lo;
    uint32_t reg_temp_hi;
    asm volatile("rdmsr" : "=a"(reg_temp_lo), "=d"(reg_temp_hi) : "c"(0xC0000080));
    k_printf ( 0, "efer_hi=%X", ((uint64_t)reg_temp_hi<<32)+reg_temp_lo );
    reg_temp_lo |= 0x800;
    asm volatile("wrmsr" : : "a"(reg_temp_lo), "d"(reg_temp_hi),  "c"(0xC0000080)); 
    asm volatile("rdmsr" : "=a"(reg_temp_lo), "=d"(reg_temp_hi) : "c"(0xC0000080));
    k_printf ( 0, "efer_hi=%X", ((uint64_t)reg_temp_hi<<32)+reg_temp_lo );

    page_t *page_tmp = alloc_page( PG_SUP );
    k_printf( 0, "alloc_page.index = %x\n", page_tmp->idx );
    k_printf( 0, "alloc_page.va    = %x\n", page_tmp->va  );



    uint64_t *temp = (uint64_t *)kvma_end-0x100;
    *temp = 0xDEADBEEF;
    k_printf ( 0, "*temp   =%x\n", *temp );
    k_printf ( 0, "kvma_end=%x\n", kvma_end );

    __free_pages( page_tmp, 0 );
    void *page_tmp_va = get_zeroed_page( PG_SUP );
    k_printf( 0, "get_zeroed_page.addr  = %x\n", page_tmp_va );


    page_t *page_tmp1= get_page_from_va( (void *)(page_tmp->va) );
    k_printf( 0, "get_page_from_va = %x\n", page_tmp1->idx );
    k_printf( 0, "get_page_from_va = %x\n", page_tmp1->va  );


    temp = (uint64_t *)kvma_end-0x100;
    *temp = 0xDEADBEEF;
    k_printf ( 0, "*temp   =%x\n", *temp );
    k_printf ( 0, "kvma_end=%x\n", kvma_end );

    set_pgt_entry_lv1( PGT_ENTRY_LV1_SELFREF, def_pgt_paddr_lv1, PGT_P, PGT_NX,
	    0x0, 0x0, PGT_RW | PGT_SUP );

    /* test objcache */

    /* create object caches */

    for ( i=0; i<7; ++i )     
	objcache_gen_head[i] = create_objcache( 0x0, 1<<(i+4) ); 
    objcache_n4k_head = create_objcache( 0x0, __PAGE_SIZE - OBJCACHE_HEADER_SIZE ); 


    objcache_vma_head = create_objcache( 0x0, sizeof(vma_t) ); 
    if ( objcache_vma_head != NULL ) {
	k_printf ( 0, "objcache vma_head addr: %p\n", objcache_vma_head );
	k_printf ( 0, "kvma_end=%x\n", kvma_end );
    }
    vma_t *vma_tmp1;
    for ( i=0; i<64; i++ ) {
	vma_tmp1 = (vma_t *)(get_object( objcache_vma_head ));
    }
    k_printf ( 0, "vma_tmp1 addr: %p\n", vma_tmp1 );
    set_vma( vma_tmp1, (addr_t)&kernofs, kvma_end,
	    NULL, 0, 0, 0, 0, 0, 0 );
    return_object( vma_tmp1 ); 
    vma_tmp1 = (vma_t *)(get_object( objcache_vma_head ));


    /* kmalloc test */
    uint64_t *u64array = kmalloc( 8*10, PG_SUP );
    for ( i=0; i<10; i++ )
	*(u64array+i) = i;
    for ( i=0; i<10; i++ )
	k_printf( 0, "%d", *(u64array+i) );
    k_printf( 0, "\n" );
    u64array = kmalloc( 4096, PG_SUP );
    for ( i=0; i<512; i++ )
	*(u64array+i) = i;
    for ( i=0; i<10; i++ )
	k_printf( 0, "%d", *(u64array+i) );
    k_printf( 0, "\n" );
    k_printf( 0, "%p\n", u64array );
    //kfree( u64array );
    for ( i=0; i<10; i++ )
	k_printf( 0, "%d", *(u64array+i) );



    __asm__("sti");
    __asm__("sti");

	sched_init();

    // kernel starts here
    while(1);
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;

uint64_t pit_cnt_m; /* millisecond counter */
uint64_t pit_cnt_s; /*      second counter */

uint64_t kbd_counter;
extern void x86_64_asm_irq_32();
extern void x86_64_asm_irq_33();

// below - IO inline functions

    static inline
void outb( unsigned short port, unsigned char val )
{
    asm volatile( "outb %0, %1"
	    : : "a"(val), "Nd"(port) );
}


    static inline
unsigned char inb( unsigned short port )
{
    unsigned char ret;
    asm volatile( "inb %1, %0"
	    : "=a"(ret) : "Nd"(port) );
    return ret;
}


    static inline
void io_wait( void )
{
    asm volatile( "jmp 1f\n\t"
	    "1:jmp 2f\n\t"
	    "2:" );
}


// above - IO inline funcitons


// below - PIC remap
#define PIC1            0x20        /* IO base address for master PIC */
#define PIC2            0xA0        /* IO base address for slave PIC */
#define PIC1_CMD        PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_CMD        PIC2
#define PIC2_DATA       (PIC2+1)

/* reinitialize the PIC controllers, giving them specified vector offsets
 *    rather than 8h and 70h, as configured by default */

#define ICW1_ICW4       0x01        /* ICW4 (not) needed */
#define ICW1_SINGLE     0x02        /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04        /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08        /* Level triggered (edge) mode */
#define ICW1_INIT       0x10        /* Initialization - required! */

#define ICW4_8086       0x01        /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02        /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08        /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C        /* Buffered mode/master */
#define ICW4_SFNM       0x10        /* Special fully nested (not) */

#define PIC_READ_IRR    0x0A        /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR    0x0B        /* OCW3 irq service next CMD read */
#define PIC_EOI         0x20        /* End-of-interrupt command code */

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

// above - PIC remap

// below - PIT

#define PIT_CH0     0x40  //PIT Channel 0's Data Register Port
#define PIT_CH1     0x41  //PIT Channels 1's Data Register Port, we wont be using this here
#define PIT_CH2     0x42  //PIT Channels 2's Data Register Port
#define PIT_CMD     0x43  //PIT Chip's Command Register Port
#define PIT_FREQ    1193182


void PIT_init(uint16_t freq)
{
    int divider = PIT_FREQ / freq;       
    outb(PIT_CMD, 0x36);             
    outb(PIT_CH0, (uint08_t)(divider & 0xFF) );   
    outb(PIT_CH0, (uint08_t)(divider >>   8) );  
}

// above - PIT


void isr_pit()
{
    register char *temp1, *temp2;
    char num_str[K_NUM_STR_LEN_MAX+1];
    char temp[K_NUM_STR_LEN_MAX+1];
    uint32_t len;
    uint32_t i, j;
    pit_cnt_m++;
    if ( pit_cnt_m == 1000 ) {
	pit_cnt_s++;
	pit_cnt_m = 0;
	len = k_ltocstr( (long)pit_cnt_s, num_str, 'u' );
	for ( i=0; i<K_NUM_STR_LEN_MAX-len; i++ )
	    temp[i] = ' '; 
	for ( j=0; j<len; j++ )
	    temp[i+j] = num_str[j];
	temp[i+j] = '\0';
	for(
		temp1 = temp,
		temp2 = (char*)((addr_t)&kernofs+0xB8F68);
		//temp2 = (char*)((addr_t)&kernofs+0xB8EC8);
		*temp1;
		temp1 += 1, temp2 += 2
	   ) *temp2 = *temp1;
    }
    PIC_eoi(0);
}

char scanCode2Ascii[128] = { /*0x00*/    0,  27, '1', '2',
    /*0x04*/  '3', '4', '5', '6',
    /*0x08*/  '7', '8', '9', '0',
    /*0x0C*/  '-', '=','\b','\t', 
    /*0x10*/  'q', 'w', 'e', 'r',
    /*0x14*/  't', 'y', 'u', 'i',
    /*0x18*/  'o', 'p', '[', ']',
    /*0x1C*/  '\n',  0, 'a', 's',
    /*0x20*/  'd', 'f', 'g', 'h',
    /*0x24*/  'j', 'k', 'l', ';',
    /*0x28*/  '\'','`',   0,'\\',
    /*0x2C*/  'z', 'x', 'c', 'v',
    /*0x30*/  'b', 'n', 'm', ',',
    /*0x34*/  '.', '/',   0, '*',
    /*0x38*/    0, ' ',   0,   0,
    /*0x3C*/    0,   0,   0,   0,
    /*0x40*/    0,   0,   0,   0,
    /*0x44*/    0,   0,   0,   0,
    /*0x48*/    0,   0, '-',   0,
    /*0x4C*/    0,   0, '+',   0,
    /*0x50*/    0,   0,   0,   0,
    /*0x54*/    0,   0,   0,   0,
    /*0x58*/    0,   0            }; 

char scanCode2AsciiShift[128] = {
    /*0x00*/    0,  27, '!', '@',
    /*0x04*/  '#', '$', '%', '^',
    /*0x08*/  '&', '*', '(', ')',
    /*0x0C*/  '_', '+','\b','\t', 
    /*0x10*/  'Q', 'W', 'E', 'R',
    /*0x14*/  'T', 'Y', 'U', 'I',
    /*0x18*/  'O', 'P', '{', '}',
    /*0x1C*/  '\n',  0, 'A', 'S',
    /*0x20*/  'D', 'F', 'G', 'H',
    /*0x24*/  'J', 'K', 'L', ':',
    /*0x28*/  '\"','~',   0, '|',
    /*0x2C*/  'Z', 'X', 'C', 'V',
    /*0x30*/  'B', 'N', 'M', '<',
    /*0x34*/  '>', '?',   0, '*',
    /*0x38*/    0, ' ',   0,   0,
    /*0x3C*/    0,   0,   0,   0,
    /*0x40*/    0,   0,   0,   0,
    /*0x44*/    0,   0,   0,   0,
    /*0x48*/    0,   0, '-',   0,
    /*0x4C*/    0,   0, '+',   0,
    /*0x50*/    0,   0,   0,   0,
    /*0x54*/    0,   0,   0,   0,
    /*0x58*/    0,   0            }; 

#define KBD_SC1_LSHIFT_P 0x2A 
#define KBD_SC1_RSHIFT_P 0x36 

#define KBD_SC1_LSHIFT_R 0xAA 
#define KBD_SC1_RSHIFT_R 0xB6 

uint08_t kbd_shift; 

void isr_kbd()
{
    register char *temp1;
    //register char *temp2;
    uint08_t scanCode[6];
    uint08_t release;

    scanCode[0] = inb( 0x60 );

    release = (scanCode[0]>>7) ? 1 : 0;

    if      ( ( scanCode[0]==KBD_SC1_LSHIFT_P ||
		scanCode[0]==KBD_SC1_RSHIFT_P    ) &&
	    kbd_shift==0                              
	    ) 
	kbd_shift = 1;
    else if ( ( scanCode[0]==KBD_SC1_LSHIFT_R ||
		scanCode[0]==KBD_SC1_RSHIFT_R    ) &&
	    kbd_shift==1
	    ) 
	kbd_shift = 0;

    if ( scanCode2Ascii[scanCode[0]]!=0 && !release ) {
	temp1  = (char*)((addr_t)&kernofs+0xB8F9A);
	if ( kbd_shift )
	    *temp1 = scanCode2AsciiShift[scanCode[0]];
	else
	    *temp1 = scanCode2Ascii[scanCode[0]];

    }


    kbd_counter++;

    PIC_eoi(1);
}


uint64_t current_irq;
void isr_common(uint64_t irq)
{
    current_irq = irq;
    if (irq == 32)
	isr_pit();
    if (irq == 33)
	isr_kbd();
} 


void boot(void)
{
    // note: function changes rsp, local stack variables can't be practically used
    register char *temp1, *temp2;
    //uint64_t i, j, k;
    uint64_t isr_addr;

    __asm__(
	    "movq %%rsp, %0;"
	    "movq %1, %%rsp;"
	    :"=g"(loader_stack)
	    :"r"(&stack[INITIAL_STACK_SIZE])
	   );
    reload_gdt();
    setup_tss();


    reload_idt();
    PIC_remap( 0x20, 0x28 );

    int_gate_t *pit_int_gate;

    pit_int_gate = (void *)(&idt[2*32]);
    isr_addr = (uint64_t)(&x86_64_asm_irq_32);
    pit_int_gate->offsetLo  = (uint16_t)(isr_addr&0xFFFF); 
    pit_int_gate->segSel    = (uint16_t)0x8; 
    pit_int_gate->attr      = (uint16_t)(TYPE_IG64|DESC_P|DESC_DPL0); 
    pit_int_gate->offsetMi  = (uint16_t)((isr_addr>>16)&0xFFFF); 
    pit_int_gate->offsetHi  = (uint32_t)((isr_addr>>32)&0xFFFFFFFF); 
    pit_int_gate->resZero   = (uint32_t)0;

    pit_int_gate = (void *)(&idt[2*33]);
    isr_addr = (uint64_t)(&x86_64_asm_irq_33);
    pit_int_gate->offsetLo  = (uint16_t)(isr_addr&0xFFFF); 
    pit_int_gate->segSel    = (uint16_t)0x8; 
    pit_int_gate->attr      = (uint16_t)(TYPE_IG64|DESC_P|DESC_DPL0); 
    pit_int_gate->offsetMi  = (uint16_t)((isr_addr>>16)&0xFFFF); 
    pit_int_gate->offsetHi  = (uint32_t)((isr_addr>>32)&0xFFFFFFFF); 
    pit_int_gate->resZero   = (uint32_t)0;

    PIT_init( 1000 );


    //__asm__("sti");
    //__asm__("sti");
    start(
	    (uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
	    &physbase,
	    (void*)(uint64_t)loader_stack[4]
	 );
    for(
	    temp1 = "!!!!! start() returned !!!!!", temp2 = (char*)((addr_t)&kernofs+0xB8000);
	    *temp1;
	    temp1 += 1, temp2 += 2
       ) *temp2 = *temp1;
    while(1);
} /* boot() */

/* vim: set ts=4 sw=0 tw=0 noet : */
