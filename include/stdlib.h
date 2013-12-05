#ifndef _STDLIB_H
#define _STDLIB_H
#include <fcntl.h>

void exit(int status);

typedef int DIR;

DIR *opendir(const char *name);

struct dirent *readdir(DIR *dirp);

int closedir(DIR *dirp);

void *malloc(size_t size);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
