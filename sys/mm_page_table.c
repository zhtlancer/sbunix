
#include <defs.h>
#include <sys/mm.h>
#include <sys/k_stdio.h>


/* initialize a page table */
int
init_pgt
(
    addr_t      addr        /* page table virtual address */
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


/* set a page table entry */
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

#if 0
/* map a page using self-reference technique */
int
map_page_self (
    addr_t      addr    , /* virtual address                        */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
)
{



} /* map_page_self */


/* map a page for another process */
int
map_page (
    addr_t      addr    , /* virtual address                        */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
)
{



} /* map_page */


#endif

/*----------------------------------------------------------
 * set/get page table entry using self-reference technique
 *----------------------------------------------------------
 */

/* set a lv4 page table entry */
int
set_pgt_entry_lv4
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
get_pgt_entry_lv4
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
set_pgt_entry_lv3
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
get_pgt_entry_lv3
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
set_pgt_entry_lv2
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
get_pgt_entry_lv2
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
set_pgt_entry_lv1
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
get_pgt_entry_lv1
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


/* vim: set ts=8 sw=4 tw=0 noet : expandtab : smarttab */
