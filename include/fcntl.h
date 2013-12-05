#ifndef _FCNTL_H
#define _FCNTL_H

#include <defs.h>

#define O_RDONLY	00000000
#define O_WRONLY	00000001
#define O_RDWR		00000002
#define O_CREAT		00000100	/* not fcntl */
#define O_APPEND	00002000
#define O_DIRECTORY	00200000	/* must be a directory */

#define O_SEEK_SET    1
#define O_SEEK_CUR    2
#define O_SEEK_END    3

#define DIRSIZ	12

struct dirent {
	uint32_t inum;
	char name[DIRSIZ];
};

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
