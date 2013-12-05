
#ifndef __MM_VMA_H__
#define __MM_VMA_H__


#include <defs.h>
#include <sys/mm_types.h>


/*-------------------------------------------------------------------------
 * Global Variable
 *-------------------------------------------------------------------------
 */

/* kernal space vma */
extern vma_t kvma_head;
extern uint64_t kvma_end;

/* object cache for user space */
extern objcache_t *objcache_vma_head;
extern objcache_t *objcache_mm_struct_head;

vma_t *vma_alloc(vma_t *vma_head, uint64_t start, uint64_t length);

void vma_insert(vma_t *vma_head, vma_t *vma_new);
void vma_delete(vma_t *vma);

/*-------------------------------------------------------------------------
 * Function
 *-------------------------------------------------------------------------
 */

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
);

vma_t *
vma_find
(
    vma_t       *vma_head       ,
    void        *addr
);


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
);

mm_struct_t *
mm_struct_dup(void);

void
mm_struct_free (
    mm_struct_t *mm_s
);

void
mm_struct_free_self (
    mm_struct_t *mm_s
);

void *sbrk(size_t incr);

#endif /* __MM_VMA_H__ */
/* vim: set ts=4 sw=0 tw=0 noet : */
