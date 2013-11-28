/*
 * Filesystem abstraction
 */

#include <sys/fs.h>

struct file files[NFILE];

/* The root inode for system */
struct inode *rootfs;

int vfs_init(void)
{
	return 0;
}


/* vim: set ts=4 sw=0 tw=0 noet : */
