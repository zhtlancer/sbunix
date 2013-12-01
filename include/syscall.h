#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <defs.h>
#include <syscall_no.h>

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

pid_t fork(void);
size_t read(int fd, void *buf, size_t nbyte);
size_t write(int fd, const void *buf, size_t nbyte);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
