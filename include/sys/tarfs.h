#ifndef _TARFS_H
#define _TARFS_H

#include <defs.h>

extern char _binary_tarfs_start;
extern char _binary_tarfs_end;

struct posix_header_ustar {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char pad[12];
};

struct tarfs_file {
	struct posix_header_ustar *_header;
	uint64_t size;
	uint8_t *_ptr;
	uint8_t *_end;
};

typedef struct tarfs_file TAR_FILE;

int tarfs_init(void);

TAR_FILE *tarfs_fopen(const char *name);

size_t tarfs_fread(void *ptr, size_t size, size_t nmemb, TAR_FILE *fp);

enum TARFS_SEEK {
	TARFS_SEEK_SET = 0,
	TARFS_SEEK_CUR = 1,
	TARFS_SEEK_END = 2
};

int tarfs_fseek(TAR_FILE *fp, long offset, int whence);

void tarfs_close(TAR_FILE *fp);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
