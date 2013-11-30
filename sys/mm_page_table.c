
#include <defs.h>
#include <sys/mm.h>
#include <sys/k_stdio.h>


/* initialize a page table */
int
init_pgt
(
    void      *addr        /* page table virtual address */
)
{
    int i;
    volatile pgt_t *pgt_temp;
    for ( i=0; i<PGT_ENTRY_NUM; ++i ) {
        pgt_temp = (volatile void *)(addr+(i*8));
        pgt_temp->paddr     = 0x0   ;
        pgt_temp->present   = PGT_NP;
        pgt_temp->nx        = PGT_NX;
        pgt_temp->avl_1     = 0x0   ;
        pgt_temp->avl_2     = 0x0   ;
        pgt_temp->flag      = PGT_RO | PGT_SUP;
    }
    return 0;
}/* init_pgt() */


/* set a page table entry using VA */
int
set_pgt_entry
(
    addr_t      addr    , /* page table entry virtual address       */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
)
{
    volatile pgt_t *pgt_temp   = (volatile void *)(addr);

    pgt_temp->paddr     = paddr>>__PAGE_SIZE_SHIFT;
    pgt_temp->present   = present;
    pgt_temp->nx        = nx     ;
    pgt_temp->avl_1     = avl_1  ;
    pgt_temp->avl_2     = avl_2  ;
    pgt_temp->flag      = flag   ;

    return 0;
}/* set_pgt_entry */


/* get a page table entry using VA */
/* FIXME: not tested yet */
pgt_t *
get_pgt_entry
(
    addr_t      va
)
{
    return (pgt_t *)va;
} /* get_pgt_entry() */


/* map a page using self-reference technique */
/* FIXME: not tested yet */
int
map_page_self (
    addr_t      addr    , /* virtual address                                */
    uint08_t    newpage , /* 1-> mape a new page; 0-> mape above PA         */  
    addr_t      paddr   , /* page PA  which will be mapped to VA            */ 
    uint08_t    flag    , /* for page            : flags for page           */
    uint08_t    nx      , /* for page table entry: nx bit                   */
    uint08_t    avl_1   , /* for page table entry: available to software    */
    uint16_t    avl_2   , /* for page table entry: available to software    */
    uint08_t    flag_pgt  /* for page table entry: flags for page table     */
)
{
    pgt_t	*pgt_tmp    ;
    page_t	*page_tmp   ;
    addr_t	pa_tmp      ;
	void	*va_tmp;
    
    pgt_tmp         = get_pgt_entry_lv1_self( addr );
    if ( !(pgt_tmp->present)  ) {
	    page_tmp    = alloc_page( PG_PGT | PG_SUP | PG_OCP );
	    pa_tmp      = (addr_t)(get_pa_from_page( page_tmp ) );	
		va_tmp		= get_va_from_page(page_tmp);
	    init_pgt( va_tmp );
        set_pgt_entry_lv1_self( addr, pa_tmp, PGT_P, nx, avl_1, avl_2, flag_pgt );
    }

    pgt_tmp         = get_pgt_entry_lv2_self( addr );
    if ( !(pgt_tmp->present)  ) {
	    page_tmp    = alloc_page( PG_PGT | PG_SUP | PG_OCP );
	    pa_tmp      = (addr_t)(get_pa_from_page( page_tmp ) );	
		va_tmp		= get_va_from_page(page_tmp);
	    init_pgt( va_tmp );
        set_pgt_entry_lv2_self( addr, pa_tmp, PGT_P, nx, avl_1, avl_2, flag_pgt );
    }

    pgt_tmp         = get_pgt_entry_lv3_self( addr );
    if ( !(pgt_tmp->present)  ) {
	    page_tmp    = alloc_page( PG_PGT | PG_SUP | PG_OCP );
	    pa_tmp      = (addr_t)(get_pa_from_page( page_tmp ) );	
		va_tmp		= get_va_from_page(page_tmp);
	    init_pgt( va_tmp );
        set_pgt_entry_lv3_self( addr, pa_tmp, PGT_P, nx, avl_1, avl_2, flag_pgt );
    }

    pgt_tmp         = get_pgt_entry_lv4_self( addr );
    if ( !(pgt_tmp->present)  ) {
        if ( newpage ) {
	        page_tmp    = alloc_page( flag );
	        pa_tmp      = (addr_t)(get_pa_from_page( page_tmp ) );
        }
        else {
            pa_tmp      = paddr;
        }       
        set_pgt_entry_lv4_self( addr, pa_tmp, PGT_P, nx, avl_1, avl_2, flag_pgt );
    }

    return 0;
} /* map_page_self */


/* map a new page for another process */
/* FIXME: not tested yet */
int
map_page (
    pgt_t       *pgt_lv1, /* top level page table start                     */
    addr_t      addr    , /* va to be mapped to                             */
    uint08_t    newpage , /* 1-> mape a new page; 0-> mape above PA         */  
    addr_t      paddr   , /* page PA  which will be mapped to VA            */ 
    uint08_t    flag    , /* for page            : flags for page           */
    uint08_t    nx      , /* for page table entry: nx bit                   */
    uint08_t    avl_1   , /* for page table entry: available to software    */
    uint16_t    avl_2   , /* for page table entry: available to software    */
    uint08_t    flag_pgt  /* for page table entry: flags for page table     */
)
{
    pgt_t	*pgt_tmp        ;
    page_t	*page_tmp       ;
    addr_t	pa_tmp          ;
    void	*va_tmp;
    addr_t  pgt_entry_tmp   ;

    pgt_entry_tmp   = (addr>>39) & 0x1FF;
    pgt_tmp         = pgt_lv1 + pgt_entry_tmp;
    if ( !(pgt_tmp->present)  ) {
	    page_tmp    = alloc_page( PG_PGT | PG_SUP | PG_OCP );
	    pa_tmp      = (addr_t)(get_pa_from_page( page_tmp ) );	
		va_tmp		= get_va_from_page(page_tmp);
	    init_pgt( va_tmp );
        set_pgt_entry( (addr_t)pgt_tmp, pa_tmp, PGT_P, nx, avl_1, avl_2, flag_pgt );
    }
    pgt_tmp = (pgt_t *)get_va_from_page( get_page_from_pgt(pgt_tmp) );


    pgt_entry_tmp   = (addr>>30) & 0x1FF;
    pgt_tmp         = pgt_tmp + pgt_entry_tmp;
    if ( !(pgt_tmp->present)  ) {
	    page_tmp    = alloc_page( PG_PGT | PG_SUP | PG_OCP );
	    pa_tmp      = (addr_t)(get_pa_from_page( page_tmp ) );	
		va_tmp		= get_va_from_page(page_tmp);
	    init_pgt( va_tmp );
        set_pgt_entry( (addr_t)pgt_tmp, pa_tmp, PGT_P, nx, avl_1, avl_2, flag_pgt );
    }
    pgt_tmp = (pgt_t *)get_va_from_page( get_page_from_pgt(pgt_tmp) );


    pgt_entry_tmp   = (addr>>21) & 0x1FF;
    pgt_tmp         = pgt_tmp + pgt_entry_tmp;
    if ( !(pgt_tmp->present)  ) {
	    page_tmp    = alloc_page( PG_PGT | PG_SUP | PG_OCP );
	    pa_tmp      = (addr_t)(get_pa_from_page( page_tmp ) );	
		va_tmp		= get_va_from_page(page_tmp);
	    init_pgt( va_tmp );
        set_pgt_entry( (addr_t)pgt_tmp, pa_tmp, PGT_P, nx, avl_1, avl_2, flag_pgt );
    }
    pgt_tmp = (pgt_t *)get_va_from_page( get_page_from_pgt(pgt_tmp) );


    pgt_entry_tmp   = (addr>>12) & 0x1FF;
    pgt_tmp         = pgt_tmp + pgt_entry_tmp;
    if ( !(pgt_tmp->present)  ) {
        if ( newpage ) {
	        page_tmp    = alloc_page( flag );
	        pa_tmp      = (addr_t)(get_pa_from_page( page_tmp ) );
        }
        else {
            pa_tmp      = paddr;
        }       
        set_pgt_entry( (addr_t)pgt_tmp, pa_tmp, PGT_P, nx, avl_1, avl_2, flag_pgt );
    }

    return 0;

} /* map_page */


#if 0
#endif

/*----------------------------------------------------------
 * set/get page table entry using self-reference technique
 *----------------------------------------------------------
 */

/* set a lv4 page table entry */
int
set_pgt_entry_lv4_self
(
    
    addr_t      addr    , /* virtual address                        */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
)
{
    addr_t addr_tmp = 0xFFFF000000000000
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<39);
    addr_tmp = addr_tmp | ((addr>> 9)&0x7FFFFFFFF8);
    return set_pgt_entry( addr_tmp, paddr, present, nx, avl_1, avl_2, flag );
}/* set_pgt_entry_lv4() */


/* get a lv4 page table entry */
pgt_t *
get_pgt_entry_lv4_self
(
    addr_t      addr      /* virtual address                        */
)
{
    addr_t addr_tmp = 0xFFFF000000000000
                    | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<39);
    addr_tmp = addr_tmp | ((addr>> 9)&0x7FFFFFFFF8);
    return (pgt_t *)addr_tmp;
}/* get_pgt_entry_lv4() */


/* set a lv3 page table entry */
int
set_pgt_entry_lv3_self
(
    addr_t      addr    , /* virtual address                        */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
)
{
    addr_t addr_tmp = 0xFFFF000000000000
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<39)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<30);
    addr_tmp = addr_tmp | ((addr>>18)&0x3FFFFFF8 );
    return set_pgt_entry( addr_tmp, paddr, present, nx, avl_1, avl_2, flag );
}/* set_pgt_entry_lv3() */


/* get a lv3 page table entry */
pgt_t *
get_pgt_entry_lv3_self
(
    addr_t      addr      /* virtual address                        */
)
{
    addr_t addr_tmp = 0xFFFF000000000000
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<39)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<30);
    addr_tmp = addr_tmp | ((addr>>18)&0x3FFFFFF8 );
    return (pgt_t *)addr_tmp;
}/* get_pgt_entry_lv3() */


/* set a lv2 page table entry */
int
set_pgt_entry_lv2_self
(
    addr_t      addr    , /* virtual address                        */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
)
{
    addr_t addr_tmp = 0xFFFF000000000000
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<39)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<30)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<21);
    addr_tmp = addr_tmp | ((addr>>27)&0x1FFFF8   );
    return set_pgt_entry( addr_tmp, paddr, present, nx, avl_1, avl_2, flag );
}/* set_pgt_entry_lv2() */


/* get a lv2 page table entry */
pgt_t *
get_pgt_entry_lv2_self
(
    addr_t      addr      /* virtual address                        */
)
{
    addr_t addr_tmp = 0xFFFF000000000000
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<39)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<30)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<21);
    addr_tmp = addr_tmp | ((addr>>27)&0x1FFFF8   );
    return (pgt_t *)addr_tmp;
}/* get_pgt_entry_lv2() */


/* set a lv1 page table entry */
int
set_pgt_entry_lv1_self
(
    addr_t      addr    , /* virtual address                        */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
)
{
    addr_t addr_tmp = 0xFFFF000000000000
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<39)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<30)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<21)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<12);
    addr_tmp = addr_tmp | ((addr>>36)&0xFF8      );
    return set_pgt_entry( addr_tmp, paddr, present, nx, avl_1, avl_2, flag );
}/* set_pgt_entry_lv1() */

/* get a lv1 page table entry */
pgt_t *
get_pgt_entry_lv1_self
(
    addr_t      addr      /* virtual address                        */
)
{
    addr_t addr_tmp = 0xFFFF000000000000
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<39)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<30)
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<21) 
                      | ((uint64_t)PGT_ENTRY_LV1_SELFREF<<12);
    addr_tmp = addr_tmp | ((addr>>36)&0xFF8      );
    return (pgt_t *)addr_tmp;
}/* get_pgt_entry_lv1() */

/* get a lv1 page table entry */
pgt_t *
get_pgt_entry_lv1
(
 	pgt_t		*pgt_lv1,
    addr_t      addr      /* virtual address                        */
)
{
	pgt_t *pgt_tmp;

	addr = (addr >> 39) & 0x1FF;
	pgt_tmp = pgt_lv1 + addr;
	return (pgt_t *)get_va_from_page(get_page_from_pgt(pgt_tmp));
}

/* set a lv1 page table entry */
int
set_pgt_entry_lv1
(
 	pgt_t		*pgt_lv1,
    addr_t      addr    , /* virtual address                        */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
);

/* get a lv2 page table entry from lv2 table */
pgt_t *
get_pgt_entry_lv2_lv1
(
 	pgt_t		*pgt_lv2,
    addr_t      addr      /* virtual address                        */
)
{
	pgt_t *pgt_tmp;

	addr = (addr >> 30) & 0x1FF;
	pgt_tmp = pgt_lv2 + addr;
	return (pgt_t *)get_va_from_page(get_page_from_pgt(pgt_tmp));
}

pgt_t *
get_pgt_entry_lv2
(
 	pgt_t		*pgt_lv1,
    addr_t      addr      /* virtual address                        */
)
{
	pgt_t *pgt_tmp = get_pgt_entry_lv1(pgt_lv1, addr);

	return get_pgt_entry_lv2_lv1(pgt_tmp, addr);
}

/* get a lv3 page table entry */
pgt_t *
get_pgt_entry_lv3_lv2
(
 	pgt_t		*pgt_lv3,
    addr_t      addr      /* virtual address                        */
)
{
	pgt_t *pgt_tmp;

	addr = (addr >> 21) & 0x1FF;
	pgt_tmp = pgt_lv3 + addr;
	return (pgt_t *)get_va_from_page(get_page_from_pgt(pgt_tmp));
}

/* set a lv2 page table entry */
int
set_pgt_entry_lv2
(
 	pgt_t		*pgt_lv1,
    addr_t      addr    , /* virtual address                        */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
);

pgt_t *
get_pgt_entry_lv3
(
 	pgt_t		*pgt_lv1,
    addr_t      addr      /* virtual address                        */
)
{
	pgt_t *pgt_tmp = get_pgt_entry_lv2_lv1(pgt_lv1, addr);

	return get_pgt_entry_lv3_lv2(pgt_tmp, addr);
}

/* set a lv3 page table entry */
int
set_pgt_entry_lv3
(
 	pgt_t		*pgt_lv1,
    addr_t      addr    , /* virtual address                        */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
);

/* get a lv4 page table entry */
pgt_t *
get_pgt_entry_lv4_lv3
(
 	pgt_t		*pgt_lv4,
    addr_t      addr      /* virtual address                        */
)
{
	pgt_t *pgt_tmp;

	addr = (addr >> 12) & 0x1FF;
	pgt_tmp = pgt_lv4 + addr;
	return (pgt_t *)get_va_from_page(get_page_from_pgt(pgt_tmp));
}

pgt_t *
get_pgt_entry_lv4
(
 	pgt_t		*pgt_lv1,
    addr_t      addr      /* virtual address                        */
)
{
	pgt_t *pgt_tmp = get_pgt_entry_lv3(pgt_lv1, addr);
	return get_pgt_entry_lv4_lv3(pgt_tmp, addr);
}

/* set a lv4 page table entry */
int
set_pgt_entry_lv4
(
	pgt_t		*pgt_lv1,
    addr_t      addr    , /* virtual address                        */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
)
{
	return 0;
}

/* Duplicate user address space from *current (except the stack) with COW tech */
int
dup_upgt_self (
	pgt_t *dst
)
{
	uint64_t va;

	for (va = 0; va < UVMA_TOP; ) {
		uint64_t va_end1 = va + 1UL * PGT_ENTRY_NUM * PGT_ENTRY_NUM * PGT_ENTRY_NUM * __PAGE_SIZE;
		if (!(get_pgt_entry_lv1_self(va)->present)) {
			va = va_end1;
			continue;
		}

		for ( ;	va < va_end1; ) {
			uint64_t va_end2 = va + 1UL * PGT_ENTRY_NUM * PGT_ENTRY_NUM * __PAGE_SIZE;
			if (!(get_pgt_entry_lv2_self(va)->present)) {
				va = va_end2;
				continue;
			}

			for ( ;	va < va_end2; ) {
				uint64_t va_end3 = va + 1UL * PGT_ENTRY_NUM * __PAGE_SIZE;
				if (!(get_pgt_entry_lv3_self(va)->present)) {
					va = va_end3;
					continue;
				}

				for ( ; va < va_end3; va += __PAGE_SIZE) {
					pgt_t *pgt_tmp = get_pgt_entry_lv4_self(va);
					if (!(pgt_tmp->present))
						continue;
					if (!(pgt_tmp->flag & PGT_RW) && !(pgt_tmp->avl_1 & PGT_AVL_COW)) {
						/* read-only page */
						map_page(dst, va, 0, get_pa_from_va((void *)va), PG_USR,
								pgt_tmp->nx, pgt_tmp->avl_1, pgt_tmp->avl_2,
								pgt_tmp->flag);
					} else {
						/* mark both parent and child as COW */
						map_page(dst, va, 0, get_pa_from_va((void *)va), PG_USR,
								pgt_tmp->nx, pgt_tmp->avl_1 | PGT_AVL_COW,
								pgt_tmp->avl_2, pgt_tmp->flag & ~(PGT_RW));
						map_page_self(va, 0, get_pa_from_va((void *)va), PG_USR,
								pgt_tmp->nx, pgt_tmp->avl_1 | PGT_AVL_COW,
								pgt_tmp->avl_2, pgt_tmp->flag & ~(PGT_RW));
					}
				}
			}
		}
	}
	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
