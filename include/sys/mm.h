#ifndef __MM_H__
#define __MM_H__

#include <sys/mm_types.h>

#define __PAGE_SIZE_SHIFT           12
#define __PAGE_SIZE_MASK            0xFFF
#define __PAGE_SIZE                 (1<<__PAGE_SIZE_SHIFT)          /*bytes*/

#define PGT_ENTRY_LV1_SELFREF       256
#define PGT_ENTRY_LV1_KERNEL        511
#define PGT_ENTRY_LV2_KERNEL        510

#define __PAGE_STRUCT_SIZE_SHIFT    5                               /*bytes*/
#define __PAGE_STRUCT_SIZE          (1<<__PAGE_STRUCT_SIZE_SHIFT)   /*bytes*/

#define PGT_ENTRY_NUM               512

#define PGT_NP      0
#define PGT_P       1

#define PGT_EXE     0
#define PGT_NX      1

#define PGT_RO      0x00
#define PGT_RW      0x01

#define PGT_SUP     0x00
#define PGT_USR     0x02

#define PGT_PWT     0x04
#define PGT_PCD     0x08
#define PGT_PS      0x40
#define PGT_G       0x80

#define PGT_PAT     1

#define PG_FRE      0x0000 // free
#define PG_OCP      0x0001 // occupied

#define PG_SUP      0x0000 // kernel
#define PG_USR      0x0002 // user

#define PG_PGT      0x0010 // page table
#define PG_OBJ      0x0020 // object cache
#define PG_KMA      0x0040 // allocated by kmalloc
#define PG_VMA      0x0080 // allocated by vmalloc


#define OBJCACHE_HEADER_SIZE 80 /* in bytes */


/* initialize page structures */
int
init_page
(
    uint32_t    occupied    /* occupied page struct num */
);



/* Initialize page structure */
int
init_pgt
(
    addr_t      addr    /* the start physical address of page structure */
);


page_t *
get_page_from_va
(
    void    *va
);

void *
get_va_from_page
(
    page_t  *page
);

addr_t
get_pa_from_page
(
    page_t  *page
);


addr_t
get_pa_from_va
(
    void    *va
);


uint32_t
find_free_pages
(
    uint32_t    num
);

void *
get_zeroed_page
(
    uint16_t    flag        
);


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

pgt_t *
get_pgt_entry_lv4
(
    addr_t      addr      /* virtual address                        */
);

/* set a lv1 page table entry */
int
set_pgt_entry_lv1
(
    addr_t      entry   , /* entry in lv1 page table                */
    uint64_t    paddr   , /* page table entry index/physical address*/
    uint08_t    present , /* present bit                            */
    uint08_t    nx      , /* nx bit                                 */
    uint08_t    avl_1   , /* available to software                  */
    uint16_t    avl_2   , /* available to software                  */
    uint08_t    flag      /* flags                                  */
);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+   Get/Free Pages
*/

page_t *
alloc_free_pages
(
    uint16_t    flag        ,
    uint32_t    num 
);

page_t *
alloc_pages
(
    uint16_t    flag        ,
    uint32_t    order       
);


page_t *
alloc_page
(
    uint16_t    flag 
);


void *
__get_free_pages
(
    uint16_t    flag        ,
    uint32_t    order 
);

void *
__get_free_page
(
    uint16_t    flag        
);


void
__free_pages_anynumber
(
    page_t      *page       ,
    uint32_t    num
);

void
__free_pages
(
    page_t      *page       ,
    uint32_t    order
);

void
free_pages
(
    void        *va         ,
    uint32_t    order
);


void
free_page
(
    void        *va         
);

/*- Get/Free Pages 
 *--------------------------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+   malloc/free
*/

void *
kmalloc
( 
    uint64_t    size        ,
    uint16_t    flag        
);

void
kfree
(
    void*       ptr
);

/*- malloc/free 
 *--------------------------------------------------------*/


int
set_vma
(
    vma_t       *vma_p      ,        
    addr_t      vm_start    ,
    addr_t      vm_end      ,
    struct vma* next        ,
    addr_t      anon_vma    ,
    addr_t      file        ,
    addr_t      ofs         , 
    uint64_t    rsv_1       ,
    uint64_t    rsv_2       ,
    uint16_t    flag        
);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ Object Cache
*/

objcache_t *
create_objcache
(
    uint16_t    flag        , 
    uint16_t    size            /* size of the object */
);

void *
get_object
(
    objcache_t  *objcache_head
);

void
return_object
(
    void    *obj
);

/* lists of kernel object caches */
extern objcache_t *objcache_vma_head;
extern objcache_t *objcache_pcb_head;
extern objcache_t *objcache_gen_head[7];    /* 16B to 1024B  */
extern objcache_t *objcache_n4k_head;       /* near 4k      */

/*- Object Cache 
 *--------------------------------------------------------*/

extern vma_t kvma_head;
extern uint64_t kvma_end;


extern char kernmem;
extern char kernofs;
extern char physbase;

extern page_t *page_struct_begin;

extern addr_t def_pgt_paddr_lv1;
extern addr_t def_pgt_paddr_lv2;
extern addr_t def_pgt_paddr_lv3;

extern uint32_t page_num;

extern addr_t page_begin;
extern addr_t page_begin_addr;

extern uint32_t page_index_begin;

int mm_init(uint32_t *modulep, void *physbase, void *physfree);






#endif/*__MM_H__*/

/* vim: set ts=8 sw=4 tw=0 noet : expandtab : smarttab */
