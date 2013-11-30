
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
    void      *addr    /* the start physical address of page table */
);


/* set a page table entry using VA */
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

/* get a page table entry using VA */
pgt_t *
get_pgt_entry
(
    addr_t      va
);


/* map a page using self-reference technique */
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
);


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
);


/* set a lv4 page table entry (self-reference) */
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
);


/* get a lv4 page table entry (self-reference) */
pgt_t *
get_pgt_entry_lv4_self
(
    addr_t      addr      /* virtual address                        */
);


/* set a lv3 page table entry (self-reference) */
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
);


/* get a lv3 page table entry (self-reference) */
pgt_t *
get_pgt_entry_lv3_self
(
    addr_t      addr      /* virtual address                        */
);


/* set a lv2 page table entry (self-reference) */
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
);


/* get a lv2 page table entry (self-reference) */
pgt_t *
get_pgt_entry_lv2_self
(
    addr_t      addr      /* virtual address                        */
);


/* set a lv1 page table entry (self-reference) */
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
);


/* get a lv1 page table entry (self-reference) */
pgt_t *
get_pgt_entry_lv1_self
(
    addr_t      addr      /* virtual address                        */
);

/* set a lv4 page table entry */
int
set_pgt_entry_lv4
(
	pgt_t		*pgt_lv4,
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
 	pgt_t		*pgt_lv4,
    addr_t      addr      /* virtual address                        */
);


/* set a lv3 page table entry */
int
set_pgt_entry_lv3
(
 	pgt_t		*pgt_lv3,
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
 	pgt_t		*pgt_lv3,
    addr_t      addr      /* virtual address                        */
);


/* set a lv2 page table entry */
int
set_pgt_entry_lv2
(
 	pgt_t		*pgt_lv2,
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
 	pgt_t		*pgt_lv2,
    addr_t      addr      /* virtual address                        */
);


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


/* get a lv1 page table entry */
pgt_t *
get_pgt_entry_lv1
(
 	pgt_t		*pgt_lv1,
    addr_t      addr      /* virtual address                        */
);

int
dup_upgt_self (
	pgt_t *dst
);

#endif /* __MM_PAGE_TABLE_H__ */
/* vim: set ts=4 sw=0 tw=0 noet : */
