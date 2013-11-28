#ifndef _MSR_H
#define _MSR_H

#include <defs.h>

#define MSR_ADDR_EFER	0xC0000080
#define MSR_ADDR_STAR	0xC0000081
#define MSR_ADDR_LSTAR	0xC0000082
#define MSR_ADDR_CSTAR	0xC0000083
#define MSR_ADDR_SFMASK	0xC0000084

static inline uint64_t rdmsr(uint32_t addr)
{
	uint32_t low, high;

	__asm__ volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (addr));

	return (low | ((uint64_t)high << 32));
}

static inline void wrmsr(uint32_t addr, uint32_t low, uint32_t high)
{
	__asm__ volatile("wrmsr" : : "c" (addr), "a" (low), "d" (high) : "memory");
}

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
