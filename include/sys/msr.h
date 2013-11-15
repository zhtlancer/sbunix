#ifndef _MSR_H
#define _MSR_H

#include <defs.h>

static inline uint64_t rdmsr(uint32_t addr)
{
	uint32_t low, high;

	asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" addr);

	return (low | ((uint64_t)high << 32));
}

static inline void wrmsr(uint32_t addr, uint32_t low, uint32_t high)
{
	asm volatile("wrmsr" : : "c" addr, "a" low, "d" high : "memory");
}

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
