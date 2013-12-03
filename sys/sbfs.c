#include <sys/k_stdio.h>
#include <sys/fs.h>

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

static void sbfs_close(struct inode *inode)
{
	/* TODO */
}

static struct fs_operations sbfs_ops = {
	.path_lookup	= sbfs_path_lookup,
	.read			= sbfs_read,
	.write			= sbfs_write,
	.close			= sbfs_close,
};

int sbfs_init(void)
{
	/* TODO */
	sbfs_db("Not implemented yet! %p\n", &sbfs_ops);

	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
