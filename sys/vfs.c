/*
 * Filesystem abstraction
 */

#include <sys/fs.h>
#include <sys/string.h>
#include <sys/error.h>
#include <sys/k_stdio.h>
#include <sys/dev.h>
#include <sys/sched.h>

#define fs_error(fmt, ...)	\
	k_printf(1, "<FS> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_FS
#define fs_db(fmt, ...)	\
	k_printf(1, "<FS DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define fs_db(fmt, ...)
#endif

struct file files[NFILE];

struct inode inodes[NINODE];

/* The root inode for system (also the root inode for tarfs) */
struct inode *rootfs;
/* The root inode for physical disk */
struct inode *diskfs;

static int is_rootfs(struct inode *inode)
{
	return inode == rootfs;
}

static struct inode *alloc_inode(void)
{
	int i;

	for (i = 0; i < NINODE; i++)
		if (inodes[i].ref == 0)
			return &inodes[i];

	panic("No available inode buffer!\n");
	return NULL;
}

static void free_inode(struct inode *inode)
{
	memset(inode, 0, sizeof(struct inode));
}
/* Add ref to inode, or allocate a new inode copy if inode==NULL */
struct inode *get_inode(struct inode *inode)
{
	if (inode == NULL)
		inode = alloc_inode();

	inode->ref += 1;
	return inode;
}

/* decrease inode reference, or free it if no reference exists */
void put_inode(struct inode *inode)
{
	if (inode->ref <= 0) {
		fs_error("Inode has no reference\n");
		panic("Unrecoverable FS error\n");
	}

	inode->ref -= 1;
	if (inode->ref == 0) {
		if (inode->fs_ops->close)
			inode->fs_ops->close(inode);
		free_inode(inode);
	}
}

int vfs_init(void)
{
	memset(files, 0, sizeof(files));
	memset(inodes, 0, sizeof(inodes));

	/* files for stdin/stdout/stderr will be initialized in console_init */

	/* TODO: initialize inode for rootfs, diskfs */
	rootfs = get_inode(NULL);
	rootfs->dev_num = DEV_TARFS;
	diskfs = get_inode(NULL);
	diskfs->dev_num = DEV_DISK;

	return 0;
}

/* parse the very first name in path, store in _name_buf,
 * return the index right after _name_buf in path */
static char _name_buf[NAME_MAX];
static int parse_name(const char *path)
{
	int i, j;

	_name_buf[0] = '\0';
	i = 0;
	for (i = 0; path[i] != '\0' && path[i] == '/'; i++)
		;

	if (path[i] == '\0')
		return 0;

	for (j = i; path[j] != '\0' && path[j] != '/'; j++)
		;

	if ((j-i) >= NAME_MAX)
		return 0;

	strlcpy(_name_buf, path+i, j-i+1);

	return j;
}

/* Special lookup routine for root, which may need to dispatch lookup into diskfs */
static struct inode *lookup_root(struct inode *parent, const char *path)
{
	int len = parse_name(path);

	if (len == 0)
		return NULL;

	/* dispatch diskfs */
	if (strcmp(_name_buf, DMOUNT) == 0) {
		struct inode *inode;
		parent = get_inode(diskfs);
		inode = parent->fs_ops->path_lookup(diskfs, path+len);
		put_inode(parent);
		return inode;
	} else
		return parent->fs_ops->path_lookup(parent, path);
}

struct inode *path_lookup(struct inode *parent, const char *path)
{
	struct inode *inode = NULL;
	int get_parent = 0;

	if (path[0] == '/') {
		get_parent = 1;
		parent = get_inode(rootfs);
	}

	/* If parent not provided, start from rootfs */
	if (parent == NULL) {
		return NULL;
	}

	if (is_rootfs(parent))
		inode = lookup_root(parent, path);
	else
		inode = inode->fs_ops->path_lookup(parent, path);

	if (get_parent)
		put_inode(parent);

	return inode;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
