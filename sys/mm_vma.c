
#include <defs.h>
#include <sys/mm.h>
#include <sys/k_stdio.h>
#include <sys/sched.h>

#define mm_error(fmt, ...)	\
	k_printf(1, "<MM> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_MM_VMA
#define mm_db(fmt, ...)	\
	k_printf(1, "<MM DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define mm_db(fmt, ...)
#endif


/*-------------------------------------------------------------------------
 * Global Variable
 *-------------------------------------------------------------------------
 */

/* kernal space vma */
vma_t kvma_head;
uint64_t kvma_end;

/* object cache for user space */
objcache_t *objcache_vma_head;
objcache_t *objcache_mm_struct_head;


/*-------------------------------------------------------------------------
 * Function
 *-------------------------------------------------------------------------
 */

/* setup a given vma */
int
vma_set
(
    vma_t       *vma_p      ,
    addr_t      vm_start    ,
    addr_t      vm_end      ,
    vma_t       *next       ,
    vma_t       *prev       ,
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
}/* vma_set() */


/* check if a virtual address within given vma list */
/* FIXME: not tested yet */
vma_t *
vma_find
(
    vma_t       *vma_head       ,
    void        *va
)
{
    vma_t       *vma_tmp    = NULL;
    addr_t      vaddr       = (addr_t)(va);

    for ( vma_tmp = vma_head; vma_tmp != vma_head; vma_tmp = vma_tmp->next )
        if ( vaddr >= vma_tmp->vm_start && vaddr < vma_tmp->vm_start )
            return vma_tmp;

    return NULL;

} /* vma_find() */

vma_t *
vma_duplicate (
	vma_t *vma_src
)
{
	vma_t *vma_iter = NULL;
	vma_t *vma_new, *vma_tmp1, *vma_tmp2;

	vma_new = (vma_t *)get_object(objcache_vma_head);
	vma_new->flag = vma_src->flag;
	vma_new->vm_start = vma_src->vm_start;
	vma_new->vm_end = vma_src->vm_end;
	vma_new->anon_vma = vma_src->anon_vma;
	vma_new->file = vma_src->anon_vma;
	vma_new->ofs = vma_src->ofs;
	vma_new->prev = vma_new;
	vma_new->next = vma_new;

	vma_tmp1 = vma_new;

	for (vma_iter = vma_src; vma_iter != vma_src; vma_iter = vma_iter->next) {
		vma_tmp2 = (vma_t *)get_object(objcache_vma_head);
		vma_tmp2->flag = vma_iter->flag;
		vma_tmp2->vm_start = vma_iter->vm_start;
		vma_tmp2->vm_end = vma_iter->vm_end;
		vma_tmp2->anon_vma = vma_iter->anon_vma;
		vma_tmp2->file = vma_iter->anon_vma;
		vma_tmp2->ofs = vma_iter->ofs;

		vma_tmp2->prev = vma_tmp1;
		vma_tmp1->next = vma_tmp2;
		vma_tmp1 = vma_tmp2;
	}

	vma_tmp1->next = vma_tmp1;
	vma_new->prev = vma_tmp1;

	return vma_new;
}

/* create a new mm_struct for a process */
/* FIXME: not tested yet */
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
    addr_t  pgt_pa      = 0;
	addr_t  addr        = 0;
    vma_t   *vma_tmp    = NULL;
    vma_t   *vma_current= NULL;

    mm_struct_t *mm_s   = (mm_struct_t *)(get_object( objcache_mm_struct_head ));

    mm_s->code_start    = code_start;
    mm_s->code_end      = code_end  ;
    mm_s->data_start    = data_start;
    mm_s->data_end      = data_end  ;
	mm_s->stack_start	= USTACK_TOP - __PAGE_SIZE;
	mm_s->brk_start		= data_end;
	mm_s->brk_end		= data_end;

    /* setup vma for code */
    vma_tmp             = (vma_t *)get_object( objcache_vma_head );
	vma_set( vma_tmp, code_start, code_end         , NULL      , NULL,
             0, file, code_ofs, 0, 0 );
    mm_s->mmap          = vma_tmp;
    vma_current         = vma_tmp;

    /* setup vma for data */
	if (data_start < data_end) {
		vma_tmp             = (vma_t *)get_object( objcache_vma_head );
		vma_set( vma_tmp, data_start, data_end         , mm_s->mmap, vma_current,
				0, file, data_ofs, 0, 0 );
		vma_current->next   = vma_tmp;
		mm_s->mmap->prev    = vma_tmp;
	}

    /* setup vma for bss  */
	if (bss_size > 0) {
		vma_tmp             = (vma_t *)get_object( objcache_vma_head );
		vma_set( vma_tmp, data_end  , data_end+bss_size, mm_s->mmap, vma_current,
				0, file, data_ofs, 0, 0 );
		vma_current->next   = vma_tmp;
		mm_s->mmap->prev    = vma_tmp;
	}

	/* XXX: we always provide user mode stack */
	vma_tmp = (vma_t *)get_object(objcache_vma_head);
	vma_set(vma_tmp, mm_s->stack_start, USTACK_TOP, mm_s->mmap, vma_current,
			0, file, data_ofs, 0, 0);
	vma_current->next = vma_tmp;
	mm_s->mmap->prev = vma_tmp;

    /* setup page table */

    mm_s->pgt   = get_zeroed_page( PG_PGT | PG_SUP | PG_OCP );
    pgt_pa      = get_pa_from_va( mm_s->pgt );
    init_pgt( (mm_s->pgt) );

	/* set lv1 page table entry: self-reference entry */
	addr = ((addr_t)(mm_s->pgt)) + (8*PGT_ENTRY_LV1_SELFREF);
	set_pgt_entry( addr, pgt_pa          , PGT_P, PGT_EXE,
			0x0, 0x0, PGT_RW | PGT_SUP );

	/* set lv1 page table entry: kernel page */
	addr = ((addr_t)(mm_s->pgt)) + (8*PGT_ENTRY_LV1_KERNEL );
	set_pgt_entry( addr, def_pgt_paddr_lv2, PGT_P, PGT_EXE,
			0x0, 0x0, PGT_RW | PGT_SUP );

    return mm_s;
} /* mm_struct_new() */

/*
 * Create a duplicated process address space from current proc's mm_struct.
 * Also, we copy all needed page mapping into the new process too
 */
mm_struct_t *
mm_struct_dup(void)
{
	addr_t pgt_pa = 0;
	addr_t addr = 0;
	mm_struct_t *mm_p = current->mm;

	mm_struct_t *mm_new = (mm_struct_t *)(get_object( objcache_mm_struct_head ));

	mm_new->code_start = mm_p->code_start;
	mm_new->code_end = mm_p->code_end;
	mm_new->data_start = mm_p->data_start;
	mm_new->data_end = mm_p->data_end;
	mm_new->brk_start = mm_p->brk_start;
	mm_new->brk_end = mm_p->brk_end;
	mm_new->stack_start = mm_p->stack_start;

	/* Duplicate VMAs */
	mm_new->mmap = vma_duplicate(mm_p->mmap);

    /* setup page table */
    mm_new->pgt   = get_zeroed_page( PG_PGT | PG_SUP | PG_OCP );
    pgt_pa      = get_pa_from_va( mm_new->pgt );
    init_pgt( (mm_new->pgt) );

	/* set lv1 page table entry: self-reference entry */
	addr = ((addr_t)(mm_new->pgt)) + (8*PGT_ENTRY_LV1_SELFREF);
	set_pgt_entry( addr, pgt_pa          , PGT_P, PGT_EXE,
			0x0, 0x0, PGT_RW | PGT_SUP );

	/* set lv1 page table entry: kernel page */
	addr = ((addr_t)(mm_new->pgt)) + (8*PGT_ENTRY_LV1_KERNEL );
	set_pgt_entry( addr, def_pgt_paddr_lv2, PGT_P, PGT_EXE,
			0x0, 0x0, PGT_RW | PGT_SUP );

	/* Duplicate user address space (including user stack) */
	dup_upgt_self(mm_new->pgt);

    return mm_new;
}

/* free a mm_struct */
/* FIXME: not tested yet */
void
mm_struct_free (
    mm_struct_t *mm_s
)
{
    /* FIXME: should free a mm_struct and all things inside it properly */ 
} /* mm_struct_free() */

/* free a mm_struct, which should be current mapping */
/* FIXME: not tested yet */
void
mm_struct_free_self (
    mm_struct_t *mm_s
)
{
	vma_t *vma_iter, *vma_tmp;

	/* unmap all user pages, and free some of them */
	for (vma_iter = mm_s->mmap; vma_iter != mm_s->mmap; vma_iter = vma_iter->next) {
		uint64_t va = vma_iter->vm_start;
		pgt_t *pgt = get_pgt_entry_lv4_self(va);
		vma_tmp = vma_tmp;
		pgt = pgt;
	}
#if DEBUG_MM_VMA
#endif
} /* mm_struct_free() */

void *sbrk(size_t incr)
{
	size_t brk_size = current->mm->brk_end - current->mm->brk_start;
	void *new_ptr = NULL;

	if ((brk_size + incr) > UBRK_LIMIT) {
		mm_error("User brk region limitation reached.\n");
		return (void *)(-1);
	}

	new_ptr = (void *)current->mm->brk_end;
	current->mm->brk_end += incr;

	return new_ptr;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
