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

struct inode {
	int dev_num;
};

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
