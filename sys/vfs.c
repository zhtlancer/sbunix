/*
 * Filesystem abstraction
 */

#include <sys/fs.h>
#include <sys/string.h>

struct file files[NFILE];

/* The root inode for system */
struct inode *rootfs;

int vfs_init(void)
{
	memset(files, 0, sizeof(files));

	/* files for stdin/stdout/stderr will be initialized in console_init */

	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
