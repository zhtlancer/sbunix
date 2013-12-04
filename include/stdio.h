#ifndef _STDIO_H
#define _STDIO_H

#include <gccbuiltin.h>

#define FD_STDIN	0
#define FD_STDOUT	1
#define FD_STDERR	2

int printf(const char *format, ...);

int gets_l(char *buf, int size);
int fdscanf(int fd, const char *format, ...);
int scanf(const char *format, ...);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
