#ifndef _SYS_FILE_H
#define _SYS_FILE_H

#include <defs.h>

enum {
	SEEK_SET = 1,
	SEEK_CUR = 2,
	SEEK_END = 3,
};

struct file;
struct inode;

struct file_operations {
	int (*open)(struct inode *inode, struct file *file);
	void (*close)(struct file *file);
	size_t (*seek)(struct file *file, size_t offset, int pos);
	size_t (*read)(struct file *file, void *buf, size_t nbytes);
	size_t (*write)(struct file *file, void *buf, size_t nbytes);
};

struct file {
	int ref;
	uint8_t readable;
	uint8_t writeable;
	size_t offset;
	struct inode *inode;

	struct file_operations *f_ops;
};

#define NDIRECT 12

struct p_inode {
	uint16_t type;		/* inode type */
	uint16_t major;
	uint16_t minor;
	uint16_t nlink;		/* number of hard links */
	uint32_t size;		/* file size (in bytes) */
	uint32_t addrs[NDIRECT+1];	/* file block addresses */
}__attribute__((packed));

struct inode {
	int dev_num;	/* index into system devs array */
	uint32_t inum;
	int ref;
	int flags;

	/* Copy of disk inode */
	struct p_inode p_inode;
};

extern struct file files[];

int vfs_init(void);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
