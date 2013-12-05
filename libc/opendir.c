#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <syscall.h>

static DIR dirs[NFILE_PER_PROC];
static struct dirent dirents[NFILE_PER_PROC];

DIR *opendir(const char *name)
{
	int fd = open(name, O_DIRECTORY, 0);

	if (fd < 0)
		return NULL;

	dirs[fd] = 1;
	return &dirs[fd];
}

struct dirent *readdir(DIR *dirp)
{
	int fd = dirp - dirs;
	int rval;
	if (fd < 0 || fd >= NFILE_PER_PROC) {
		printf("Invalid fd(%d) for readdir\n", fd);
		return NULL;
	}
	rval = getdents(fd, &dirents[fd], 1);

	if (rval == 1)
		return &dirents[fd];
	else
		return NULL;
}

int closedir(DIR *dirp)
{
	int fd = dirp - dirs;

	if (fd < 0 || fd >= NFILE_PER_PROC) {
		printf("Invalid fd(%d) for readdir\n", fd);
		return NULL;
	}

	dirs[fd] = 0;

	return close(fd);
}
/* vim: set ts=4 sw=0 tw=0 noet : */
