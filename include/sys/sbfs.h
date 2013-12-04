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
#define SBFS_DENTRY_SIZE		16


#define SBFS_IT_FREE			0
#define SBFS_IT_FILE			1
#define SBFS_IT_DIR				2
#define SBFS_IT_DEV  			4

#define SBFS_NAME_MAX			13

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

struct sbfs_dentry { /* 16 bytes */
	uint32_t	pinode_nr				;
	char		name[SBFS_NAME_MAX+1]	;
}__attribute__((packed));
typedef struct sbfs_dentry sbfs_dentry_t;

/*-------------------------------------------------------------------------
 * Global Variable
 *-------------------------------------------------------------------------
 */


/*-------------------------------------------------------------------------
 * Function
 *-------------------------------------------------------------------------
 */

int sbfs_newfs();

int sbfs_init(void);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
