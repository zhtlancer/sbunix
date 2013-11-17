
#ifndef __MM_VMA_H__
#define __MM_VMA_H__


#include <defs.h>
#include <sys/mm_types.h>


/* --------------------------------------------------------
 * Global Variable
 * --------------------------------------------------------
 */

/* kernal space vma */
extern vma_t kvma_head;
extern uint64_t kvma_end;

/* object cache for user space */
extern objcache_t *objcache_vma_head;
extern objcache_t *objcache_mm_struct_head;


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


#endif /* __MM_VMA_H__ */
