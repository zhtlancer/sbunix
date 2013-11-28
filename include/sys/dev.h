#ifndef _SYS_DEV_H
#define _SYS_DEV_H

#include <defs.h>

#define DEV_CONSOLE	1
#define DEV_TARFS	2
#define DEV_DISK	3

enum {
	DEV_SEEK_SET = 1,
	DEV_SEEK_CUR = 2,
	DEV_SEEK_END = 3,
};

struct dev {

	/* Super block for filesystem on this device */
	void *super_block;

	/* device operation hook */
	size_t (*seek)(struct dev *dev, size_t offset, int pos);
	size_t (*read)(struct dev *dev, void *buf, size_t n);
	size_t (*write)(struct dev *dev, void *buf, size_t n);
};

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
