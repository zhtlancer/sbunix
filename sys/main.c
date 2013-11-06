#include <defs.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/io.h>
#include <sys/pic.h>
#include <sys/pit.h>
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

	while(modulep[0] != 0x9001)
		modulep += modulep[1]+2;

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

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	register char *temp1, *temp2;

	__asm__(
			"movq %%rsp, %0;"
			"movq %1, %%rsp;"
			:"=g"(loader_stack)
			:"r"(&stack[INITIAL_STACK_SIZE])
		   );
	reload_gdt();
	setup_tss();


	reload_idt();

	pic_init();
	idt_setup();

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
