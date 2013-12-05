#include <syscall.h>
#include <fcntl.h>

pid_t fork(void)
{
	return __syscall0(SYS_fork);
}

int execve(const char *filename, char *const argv[], char *const envp[])
{
	return __syscall3(SYS_execve, (uint64_t)filename, (uint64_t)argv, (uint64_t)envp);
}

unsigned int sleep(unsigned int mseconds)
{
	return __syscall1(SYS_sleep, mseconds);
}

pid_t wait(int *status)
{
	return __syscall1(SYS_wait, (uint64_t)status);
}

pid_t waitpid(pid_t pid, int *status, int options)
{
	return __syscall3(SYS_waitpid, pid, (uint64_t)status, options);
}

pid_t _exit(int status)
{
	return __syscall1(SYS_exit, status);
}

int kill(pid_t pid, int sig)
{
	return __syscall2(SYS_kill, pid, sig);
}

pid_t getpid(void)
{
	return __syscall0(SYS_getpid);
}

int open(const char *pathname, int flags, mode_t mode)
{
	return __syscall3(SYS_open, (uint64_t)pathname, flags, mode);
}

int close(int fd)
{
	return __syscall1(SYS_close, fd);
}

size_t read(int fd, void *buf, size_t nbyte)
{
	return __syscall3(SYS_read, fd, (uint64_t)buf, nbyte);
}

size_t write(int fd, const void *buf, size_t nbyte)
{
	return __syscall3(SYS_write, fd, (uint64_t)buf, nbyte);
}

off_t lseek(int fd, off_t offset, int whence)
{
	return __syscall3(SYS_lseek, fd, offset, whence);
}

int getdents(unsigned int fd, struct dirent *dirp, unsigned int count)
{
	return __syscall3(SYS_getdents, fd, (uint64_t)dirp, count);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	return (void *)__syscall6(SYS_mmap, (uint64_t)addr, length, prot, flags, fd, offset);
}

int munmap(void *addr, size_t length)
{
	return __syscall2(SYS_munmap, (uint64_t)addr, length);
}

void *sbrk(intptr_t increment)
{
	return (void *)__syscall1(SYS_sbrk, increment);
}

/* vim: set ts=4 sw=0 tw=0 noet : */
