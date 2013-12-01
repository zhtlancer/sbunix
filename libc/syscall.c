#include <syscall.h>

size_t read(int fd, void *buf, size_t nbyte)
{
	return __syscall3(SYS_read, fd, (uint64_t)buf, nbyte);
}

size_t write(int fd, const void *buf, size_t nbyte)
{
	return __syscall3(SYS_write, fd, (uint64_t)buf, nbyte);
}

pid_t fork(void)
{
	return __syscall0(SYS_fork);
}
/* vim: set ts=4 sw=0 tw=0 noet : */
