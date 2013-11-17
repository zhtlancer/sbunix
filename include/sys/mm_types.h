
#ifndef __MM_TYPE_H__
#define __MM_TYPE_H__

#include <defs.h>


/* one for each page: 32 bytes */
struct page {
    uint16_t flag;
    uint16_t count;
    uint32_t idx;
    addr_t   va;
    uint32_t next;
    uint32_t kmalloc_size;
    uint64_t reserv64;
}__attribute__((packed));
typedef struct page page_t;


/* page table entry */
struct pgt {
    uint08_t    present : 1;
    uint08_t    flag    : 8;
    uint08_t    avl_1   : 3;
    uint64_t    paddr   :40;
    uint16_t    avl_2   :11;
    uint08_t    nx      : 1;
}__attribute__((packed));
typedef struct pgt pgt_t;


/* object cache header: 80 bytes */
struct objcache {
    uint16_t    flag    : 16;
    uint16_t    size    : 16; /* size of the object */
    uint16_t    start   : 16; /* start offset of the object in the page */
    uint08_t    count   :  8; /* number of object in this page */
    uint08_t    free    :  8; /* number of free object in this page */
    uint64_t    bmap[8]     ; /* bit map of objects: 1->used, 0->free */
    struct objcache *next   ; /* pointer to next page whit the same type */
}__attribute__((packed));
typedef struct objcache objcache_t;


/* vm area structure: 64 bytes */
struct vma {
    uint16_t    flag    :16 ;
    uint64_t    rsv_1   :48 ;
    addr_t      vm_start    ; /* start address (included) */
    addr_t      vm_end      ; /* start address (excluded) */
    struct vma* next        ;
    struct vma* prev        ;
    addr_t      anon_vma    ;
    addr_t      file        ;
    addr_t      ofs         ; 
}__attribute__((packed));
typedef struct vma vma_t;


/* mm_struct: 256 bytes */
struct mm_struct {

    vma_t	*mmap;		/* list of vma */
    void	*pgt;		/* level 1 page table virtual address */

    addr_t	code_start;	
    addr_t	code_end;	
    addr_t	data_start;	
    addr_t	data_end;	
    addr_t	brk_start;	
    addr_t	brk;	
    addr_t	stack_start;	

    uint64_t	reserved11;
    uint64_t	reserved12;
    uint64_t	reserved13;
    uint64_t	reserved14;
    uint64_t	reserved15;
    uint64_t	reserved16;
    uint64_t	reserved17;

    uint64_t	reserved20;
    uint64_t	reserved21;
    uint64_t	reserved22;
    uint64_t	reserved23;
    uint64_t	reserved24;
    uint64_t	reserved25;
    uint64_t	reserved26;
    uint64_t	reserved27;

    uint64_t	reserved30;
    uint64_t	reserved31;
    uint64_t	reserved32;
    uint64_t	reserved33;
    uint64_t	reserved34;
    uint64_t	reserved35;
    uint64_t	reserved36;
    uint64_t	reserved37;
}__attribute__((packed));
typedef struct mm_struct mm_struct_t;


#endif /*__MM_TYPE_H__*/
