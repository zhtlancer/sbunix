#ifndef __MM_H__
#define __MM_H__

#include <defs.h>
#include <sys/mm_types.h>
#include <sys/mm_vma.h>
#include <sys/mm_page_table.h>

#define __PAGE_SIZE_SHIFT           12
#define __PAGE_SIZE_MASK            0xFFF
#define __PAGE_SIZE                 (1<<__PAGE_SIZE_SHIFT)          /*bytes*/

#define PGT_ENTRY_LV1_SELFREF       256
#define PGT_ENTRY_LV1_KERNEL        511
#define PGT_ENTRY_LV2_KERNEL        510

#define __PAGE_STRUCT_SIZE_SHIFT    5                               /*bytes*/
#define __PAGE_STRUCT_SIZE          (1<<__PAGE_STRUCT_SIZE_SHIFT)   /*bytes*/

#define PGT_ENTRY_NUM               512

#define PGT_NP      0		/* [0] */
#define PGT_P       1

#define PGT_EXE     0		/* [63]? */
#define PGT_NX      1

#define PGT_RO      0x00	/* [1] */
#define PGT_RW      0x01

#define PGT_SUP     0x00	/* [2] */
#define PGT_USR     0x02

#define PGT_PWT     0x04	/* [3] */
#define PGT_PCD     0x08	/* [4] */
#define PGT_A  	    0x10	/* [5] accessed */
#define PGT_D	    0x20	/* [6] dirty */
#define PGT_PS      0x40 	/* [7] only for lv2,lv3 */
#define PGT_G       0x80	/* [8] only for last level */

#define PGT_PAT     1

/* flags values for avl_1 */
#define PGT_AVL_SHARE	0x2	/* [10] Shared pte */
#define PGT_AVL_COW	0x4	/* [11] Copy-on-write pte */

#define PG_FRE      0x0000 // free
#define PG_OCP      0x0001 // occupied

#define PG_SUP      0x0000 // kernel
#define PG_USR      0x0002 // user

#define PG_PGT      0x0010 // page table
#define PG_OBJ      0x0020 // object cache
#define PG_KMA      0x0040 // allocated by kmalloc
#define PG_VMA      0x0080 // allocated by vmalloc


#define OBJCACHE_HEADER_SIZE 80 /* in bytes */

/* Top of user address space(excluding) */
#define UVMA_TOP	(0x0000800000000000UL)
/* Top of user stack */
#define USTACK_TOP		(UVMA_TOP - __PAGE_SIZE)
#define USTACK_LIMIT	(3 * __PAGE_SIZE)
#define USTACK_BOTTOM	(USTACK_TOP - USTACK_LIMIT)
/* Top of normal user address space */
#define UNORMAL_TOP		(USTACK_BOTTOM - __PAGE_SIZE)

/* TODO: Maybe a illustration of our VM space here? */

/* initialize page structures */
int
init_page
(
    uint32_t    occupied    /* occupied page struct num */
);



/* Initialize a page table */
//int
//init_pgt
//(
//    addr_t      addr    /* the start physical address of page structure */
//);


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


page_t *
get_page_from_pgt
(
    pgt_t   *pgt_tmp
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
extern objcache_t *objcache_pcb_head;
extern objcache_t *objcache_gen_head[7];    /* 16B to 1024B  */
extern objcache_t *objcache_n4k_head;       /* near 4k      */

/*- Object Cache 
 *--------------------------------------------------------*/


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
