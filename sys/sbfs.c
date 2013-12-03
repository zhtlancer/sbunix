
#include <defs.h>
#include <sys/mm.h>
#include <sys/string.h>
#include <sys/dev.h>
#include <sys/pci.h>
#include <sys/ahci.h>
#include <sys/fs.h>
#include <sys/sbfs.h>
#include <sys/k_stdio.h>

#define sbfs_error(fmt, ...)	\
	k_printf(1, "<FS> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_SBFS
#define sbfs_db(fmt, ...)	\
	k_printf(1, "<FS DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define sbfs_db(fmt, ...)
#endif

static struct inode *sbfs_path_lookup(struct inode *parent, const char *path)
{
	/* TODO */
	return NULL;
}
static size_t sbfs_read(struct inode *inode, void *dst, off_t off, size_t n)
{
	/* TODO */
	return 0;
}

static size_t sbfs_write(struct inode *inode, void *src, off_t off, size_t n)
{
	/* TODO */
	return 0;
}

static int sbfs_getdirents(struct inode *inode, void *buf, int offset, int count)
{
	/* TODO */
	return 0;
}

static void sbfs_close(struct inode *inode)
{
	/* TODO */
}

static struct fs_operations sbfs_ops = {
	.path_lookup	= sbfs_path_lookup,
	.read			= sbfs_read,
	.write			= sbfs_write,
	.getdirents		= sbfs_getdirents,
	.close			= sbfs_close 
};


uint32_t *sbfs_buf;
sbfs_super_block_t sbfs_superblk;

/* format disk */
int
sbfs_newfs()
{
	uint32_t i, j, cnt;
	struct dev *sbfs_dev = &devs[DEV_DISK];
	struct p_inode *pinode = (struct p_inode *)(sbfs_buf);

	/* write super block into disk */
	memset( (void *)sbfs_buf, 0, AHCI_SECT_SIZE );
	sbfs_buf[0] = SBFS_SECT_BM;
	sbfs_buf[1] = SBFS_SECT_INODE;
	sbfs_buf[2] = SBFS_SECT_DATA;
	sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, SBFS_SECT_SUPER );

	/* write bit map into disk */
	cnt=0;
	for ( i=SBFS_SECT_BM; i<SBFS_SECT_INODE; i++ ) {
		memset( (void *)sbfs_buf, 0, AHCI_SECT_SIZE );
		for ( j=0; j<(AHCI_SECT_SIZE/32); j++ ) {
			if ( cnt<SBFS_SECT_DATA )
				sbfs_buf[i]	= 0xFFFFFFFF;
			else
				sbfs_buf[i] = 0x00000000;
			
			cnt += 32;
		}
		sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, i );
	}

	/* write inode into disk */
	cnt=0;
	for ( i=SBFS_SECT_INODE; i<SBFS_SECT_DATA; i++ ) {
		memset( (void *)pinode, 0, AHCI_SECT_SIZE );
		for ( j=0; j<(AHCI_SECT_SIZE/SBFS_INODE_SIZE); j++ ) {
			cnt += 1;
		}
		sbfs_dev->writesect( sbfs_dev, (void *)pinode, i );
	}

	return 0;
}




int sbfs_init(void)
{
	struct dev *sbfs_dev = &devs[DEV_DISK];
	sbfs_super_block_t *dev_sbfs_superblk = (sbfs_super_block_t *)(sbfs_dev->super_block);
	dev_sbfs_superblk = &sbfs_superblk;	
	dev_sbfs_superblk->sect_nr_bm		= SBFS_SECT_BM;
	dev_sbfs_superblk->sect_nr_inode	= SBFS_SECT_INODE;
	dev_sbfs_superblk->sect_nr_data		= SBFS_SECT_DATA;

	sbfs_buf = (uint32_t *)kmalloc( 512, PG_SUP );

	/* TODO */
	sbfs_newfs();

	sbfs_db("Not implemented yet! %p\n", &sbfs_ops);


	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
