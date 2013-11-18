
#ifndef __MM_PAGE_TABLE_H__
#define __MM_PAGE_TABLE_H__

#include <defs.h>
#include <sys/mm_types.h>


/*-------------------------------------------------------------------------
 * Function
 *-------------------------------------------------------------------------
 */


/* Initialize a page table */
int
init_pgt
(
    addr_t      addr    /* the start physical address of page table */
);


/* set a page table entry */
int
set_pgt_entry
(
    addr_t      addr    ,
    uint64_t    paddr   ,
    uint08_t    present ,
    uint08_t    nx      ,
    uint08_t    avl_1   ,
    uint16_t    avl_2   ,
    uint08_t    flag 
);


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
);


/* get a lv4 page table entry */
pgt_t *
get_pgt_entry_lv4
(
    addr_t      addr      /* virtual address                        */
);


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
);


/* get a lv3 page table entry */
pgt_t *
get_pgt_entry_lv3
(
    addr_t      addr      /* virtual address                        */
);


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
);


/* get a lv2 page table entry */
pgt_t *
get_pgt_entry_lv2
(
    addr_t      addr      /* virtual address                        */
);


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
);


/* get a lv1 page table entry */
pgt_t *
get_pgt_entry_lv2
(
    addr_t      addr      /* virtual address                        */
);



#endif /* __MM_PAGE_TABLE_H__ */
