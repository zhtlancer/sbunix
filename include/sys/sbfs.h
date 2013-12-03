#ifndef _SYS_SBFS_H
#define _SYS_SBFS_H

#include <defs.h>
#include <sys/dev.h>
#include <sys/pci.h>
#include <sys/ahci.h>
#include <sys/fs.h>

/*-------------------------------------------------------------------------
 * Definition
 *-------------------------------------------------------------------------
 */

#define SBFS_SECT_SUPER         0
#define SBFS_SECT_BM            1
#define SBFS_SECT_INODE         SBFS_SECT_BM+((AHCI_DISK_SECT_NUM/AHCI_SECT_SIZE)/8)
#define SBFS_INODE_SIZE         sizeof(struct p_inode)
#define SBFS_INODE_NUM          2048*(AHCI_SECT_SIZE/SBFS_INODE_SIZE)
#define SBFS_INODE_START        (SBFS_SECT_BM*(AHCI_SECT_SIZE/SBFS_INODE_SIZE))
#define SBFS_SECT_DATA          2048


/*-------------------------------------------------------------------------
 * Structure
 *-------------------------------------------------------------------------
 */

struct sbfs_super_block {
	uint32_t	sect_nr_bm		;
	uint32_t	sect_nr_inode	;
	uint32_t	sect_nr_data	;
}__attribute__((packed));
typedef volatile struct sbfs_super_block sbfs_super_block_t;


/*-------------------------------------------------------------------------
 * Function
 *-------------------------------------------------------------------------
 */

int sbfs_newfs();

int sbfs_init(void);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
