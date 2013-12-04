#include <defs.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

#define BUF_SIZE 512

static char buf[BUF_SIZE+1];

/* get a line of input from STDIN, and the newline at tail will be replaced by '\0' */
int gets_l(char *buf, int size)
{
	int rval;
	rval = read(0, buf, size-1);
	buf[rval] = '\0';
	if (rval > 0 && buf[rval-1] == '\n')
		buf[--rval] = '\0';
	return rval;
}

static int vfdscanf(int fd, const char *fmt, va_list ap)
{
	gets_l(buf, BUF_SIZE);
	return 0;
}

int fdscanf(int fd, const char *fmt, ...)
{
	int rval;
	va_list ap;
	va_start(ap, fmt);

	rval = vfdscanf(fd, fmt, ap);
	va_end(ap);

	return rval;
}

int scanf(const char *fmt, ...)
{
	int rval;
	va_list ap;
	va_start(ap, fmt);

	rval = vfdscanf(FD_STDIN, fmt, ap);
	va_end(ap);

	return rval;
}


/* vim: set ts=4 sw=0 tw=0 noet : */
