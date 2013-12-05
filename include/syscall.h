#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <defs.h>
#include <syscall_no.h>
#include <fcntl.h>

#define SYSCALL_PROTO(n) static __inline uint64_t __syscall##n

SYSCALL_PROTO(0)(uint64_t n) {
	uint64_t rval;
	__asm__ volatile("movq %1, %%rax\n\t"
			"syscall"
			: "=a"(rval) : "D"(n): "r10", "r11", "rcx");
	return rval;
}

SYSCALL_PROTO(1)(uint64_t n, uint64_t a1) {
	uint64_t rval;
	__asm__ volatile("movq %1, %%rax\n\t"
			"movq %2, %%rdi\n\t"
			"syscall"
			: "=a"(rval) : "D"(n), "S"(a1): "r10", "r11", "rcx");
	return rval;
}

SYSCALL_PROTO(2)(uint64_t n, uint64_t a1, uint64_t a2) {
	uint64_t rval;
	__asm__ volatile("movq %1, %%rax\n\t"
			"movq %2, %%rdi\n\t"
			"movq %3, %%rsi\n\t"
			"syscall"
			: "=a"(rval) : "D"(n), "S"(a1), "d"(a2): "r10", "r11", "rcx");
	return rval;
}

SYSCALL_PROTO(3)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	uint64_t rval;
	__asm__ volatile("movq %1, %%rax\n\t"
			"movq %2, %%rdi\n\t"
			"movq %3, %%rsi\n\t"
			"movq %4, %%rdx\n\t"
			"syscall"
			: "=a"(rval) : "D"(n), "S"(a1), "d"(a2), "c"(a3): "r10", "r11");
	return rval;
}

SYSCALL_PROTO(4)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
	uint64_t rval;
	__asm__ volatile("movq %1, %%rax\n\t"
			"movq %2, %%rdi\n\t"
			"movq %3, %%rsi\n\t"
			"movq %4, %%rdx\n\t"
			"movq %5, %%r8\n\t"
			"syscall"
			: "=a"(rval) : "D"(n), "S"(a1), "d"(a2), "c"(a3), "r"(a4): "r10", "r11");
	return rval;
}

SYSCALL_PROTO(5)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
	uint64_t rval;
	__asm__ volatile("movq %1, %%rax\n\t"
			"movq %2, %%rdi\n\t"
			"movq %3, %%rsi\n\t"
			"movq %4, %%rdx\n\t"
			"movq %5, %%r8\n\t"
			"movq %6, %%r9\n\t"
			"syscall"
			: "=a"(rval)
			: "D"(n), "S"(a1), "d"(a2), "c"(a3), "r"(a4), "r"(a5)
			: "r9", "r10", "r11");
	return rval;
}

SYSCALL_PROTO(6)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
	uint64_t rval;
	__asm__ volatile("movq %1, %%rax\n\t"
			"movq %2, %%rdi\n\t"
			"movq %3, %%rsi\n\t"
			"movq %4, %%rdx\n\t"
			"movq %5, %%r8\n\t"
			"movq %6, %%r9\n\t"
			"movq %7, %%r12\n\t"
			"syscall"
			: "=a"(rval)
			: "D"(n), "S"(a1), "d"(a2), "c"(a3), "r"(a4), "r"(a5), "r"(a6)
			: "r9", "r10", "r11", "r12");
	return rval;
}

pid_t fork(void);

int execve(const char *filename, char *const argv[], char *const envp[]);

unsigned int sleep(unsigned int seconds);

pid_t wait(int *status);

/* option values for waitpid */
#define WNOHANG 0x01
pid_t waitpid(pid_t pid, int *status, int options);

pid_t _exit(int status);

int kill(pid_t pid, int sig);

pid_t getpid(void);

int open(const char *pathname, int flags, mode_t mode);

int close(int fd);

size_t read(int fd, void *buf, size_t nbyte);

size_t write(int fd, const void *buf, size_t nbyte);

off_t lseek(int fd, off_t offset, int whence);

int getdents(int fd, struct dirent *dirp, int count);

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

int munmap(void *addr, size_t length);

void *sbrk(intptr_t increment);

int chdir(const char *path);

int ps(void *buf, int count);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
