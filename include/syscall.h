#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <defs.h>

#define SYSCALL_PROTO(n) static __inline uint64_t __syscall##n

SYSCALL_PROTO(0)(uint64_t n) {
	uint64_t rval;
	__asm__ volatile("syscall": "=a"(rval) : "D"(n): );
	return rval;
}

SYSCALL_PROTO(1)(uint64_t n, uint64_t a1) {
	uint64_t rval;
	__asm__ volatile("syscall": "=a"(rval) : "D"(n), "S"(a1): );
	return rval;
}

SYSCALL_PROTO(2)(uint64_t n, uint64_t a1, uint64_t a2) {
	uint64_t rval;
	__asm__ volatile("syscall": "=a"(rval) : "D"(n), "S"(a1), "d"(a2): );
	return rval;
}

SYSCALL_PROTO(3)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	uint64_t rval;
	__asm__ volatile("syscall": "=a"(rval) : "D"(n), "S"(a1), "d"(a2), "c"(a3): );
	return rval;
}

SYSCALL_PROTO(4)(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
	uint64_t rval;
	__asm__ volatile("syscall": "=a"(rval) : "D"(n), "S"(a1), "d"(a2), "c"(a3), "r"(a4): "r8");
	return rval;
}

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
