
#include <defs.h>
#include <sys/mm.h>
#include <sys/k_stdio.h>


/* --------------------------------------------------------
 * Global Variable
 * --------------------------------------------------------
 */

/* kernal space vma */
vma_t kvma_head;
uint64_t kvma_end;

/* object cache for user space */
objcache_t *objcache_vma_head;
objcache_t *objcache_mm_struct_head;


/* --------------------------------------------------------
 * Function
 * --------------------------------------------------------
 */

int
vma_set
(
    vma_t       *vma_p      ,
    addr_t      vm_start    ,
    addr_t      vm_end      ,
    struct vma* next        ,
    struct vma* prev        ,
    addr_t      anon_vma    ,
    addr_t      file        ,
    addr_t      ofs         ,
    uint64_t    rsv_1       ,
    uint16_t    flag
)
{
    vma_p->vm_start = vm_start  ;
    vma_p->vm_end   = vm_end    ;
    vma_p->next     = next      ;
    vma_p->prev     = prev      ;
    vma_p->anon_vma = anon_vma  ;
    vma_p->file     = file      ;
    vma_p->ofs      = ofs       ;
    vma_p->rsv_1    = rsv_1     ;
    vma_p->flag     = flag      ;

    return 0;
}/* vma_set */


mm_struct_t *
mm_struct_new (
    addr_t      code_start      ,
    addr_t      code_end        ,
    addr_t      data_start      ,
    addr_t      data_end        ,
    addr_t      file            ,
    addr_t      code_ofs        ,
    addr_t      data_ofs        ,
    uint64_t    bss_size   
)
{
    addr_t  pgt_pa  = 0;
	addr_t  addr    = 0;

    mm_struct_t *mm_s   = (mm_struct_t *)(get_object( objcache_mm_struct_head ));

    mm_s->code_start    = code_start;
    mm_s->code_end      = code_end  ;
    mm_s->data_start    = data_start;
    mm_s->data_end      = data_end  ;

    mm_s->mmap          = (vma_t *)get_object( objcache_vma_head );
	vma_set( mm_s->mmap      , code_start, code_end, NULL            , NULL,
             0, file, code_ofs, 0, 0 );

    mm_s->mmap->next    = (vma_t *)get_object( objcache_vma_head );  
	vma_set( mm_s->mmap->next, data_start, data_end, mm_s->mmap->next, NULL,
             0, file, data_ofs, 0, 0 );

    /* setup page table */

    mm_s->pgt   = get_zeroed_page( PG_SUP );
    pgt_pa      = get_pa_from_va( mm_s->pgt );
    init_pgt( (addr_t)(mm_s->pgt) );

	/* set lv1 page table entry: self-reference entry */
	addr = ((addr_t)(mm_s->pgt)) + (8*PGT_ENTRY_LV1_SELFREF);
	set_pgt_entry( addr, pgt_pa          , PGT_P, PGT_EXE,
			0x0, 0x0, PGT_RW | PGT_SUP );

	/* set lv1 page table entry: kernel page */
	addr = ((addr_t)(mm_s->pgt)) + (8*PGT_ENTRY_LV1_KERNEL );
	set_pgt_entry( addr, def_pgt_paddr_lv2, PGT_P, PGT_EXE,
			0x0, 0x0, PGT_RW | PGT_SUP );

    return mm_s;
}

