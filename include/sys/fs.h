#ifndef _SYS_FILE_H
#define _SYS_FILE_H

#include <defs.h>
#include <sys/k_stdio.h>
#include <fcntl.h>

/* MAX length of a name in dirent including '\0'
 * (which make sure the dirent is 16 bytes */
#define NAME_MAX	12

enum {
	SEEK_SET = 1,
	SEEK_CUR = 2,
	SEEK_END = 3,
};

struct file;
struct inode;

/* root inode for tarfs, also for system-wide root */
extern struct inode *rootfs;
/* root inode for disk */
extern struct inode *diskfs;

struct file_operations {
	int (*open)(struct inode *inode, struct file *file);
	void (*close)(struct file *file);
	off_t (*seek)(struct file *file, off_t offset, int pos);
	size_t (*read)(struct file *file, void *buf, size_t nbytes);
	size_t (*write)(struct file *file, void *buf, size_t nbytes);
};

struct file {
	int ref;
	uint8_t readable;
	uint8_t writeable;
	off_t offset;
	struct inode *inode;

	struct file_operations *f_ops;
};

struct fs_operations {
	struct inode *(*path_lookup)(struct inode *parent, const char *path);
	size_t (*read)(struct inode *inode, void *dst, off_t offset, size_t nbytes);
	size_t (*write)(struct inode *inode, void *src, off_t offset, size_t nbytes);
	/* offset is based on dirent, not bytes */
	int (*getdirents)(struct inode *inode, void *buf, int offset, int count);
	void (*close)(struct inode *inode);
};

#define NDIRECT 12

enum INODE_TYPE {
	IT_FILE	= 1,
	IT_DIR	= 2,
	IT_DEV	= 4,
};

/* inode struct on disk, with size of 64 bytes */
struct p_inode {
	uint16_t type;		/* inode type */
	uint16_t major;
	uint16_t minor;		/* FIXME: current used as p_inode number */
	uint16_t nlink;		/* number of hard links */
	uint32_t size;		/* file size (in bytes) */
	uint32_t addrs[NDIRECT+1];	/* file block addresses */
}__attribute__((packed));

struct inode {
	int dev_num;	/* index into system devs array */
	uint32_t inum;
	int ref;
	int flags;
	struct fs_operations *fs_ops;
	void *priv_data;

	/* Copy of disk inode */
	struct p_inode p_inode;
};


extern struct file files[];
extern struct inode inodes[];

struct inode *get_inode(struct inode *inode);
void put_inode(struct inode *inode);

static inline struct file *file_dup(struct file *file)
{
	file->ref += 1;
	return file;
}

void file_close(struct file *file);

#if 0
static inline void file_put(struct file *file)
{
	if (file->ref <= 0) {
		k_printf(0, "Putting an unreferenced file %p\n", file);
		panic("file_put error\n");
	}
	file->ref -= 1;
}
#endif

int vfs_init(void);

struct inode *path_lookup(struct inode *parent, const char *path);

int fd_open(const char *pathname, int flags, mode_t mode);

int fd_close(int fd);

int fd_getdents(int fd, struct dirent *dirp, int count);

int chdir(const char *path);

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

int munmap(void *addr, size_t length);

extern struct file files[];
extern struct inode inodes[];

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
