#ifndef _SYS_X86_H
#define _SYS_X86_H

#include <defs.h>

static inline void lcr3(uint64_t cr3)
{
	__asm__ volatile("movq %0, %%cr3" : : "r" (cr3));
}

static inline uint64_t rcr2(void)
{
	uint64_t cr2;

	__asm__ volatile("movq %%cr2, %0" : "=r" (cr2));
	return cr2;
}

static inline void flush_tlb(void)
{
	__asm__ volatile("movq	%%cr3, %%rax\n\t"
			"movq	%%rax, %%cr3" : : : "rax");
}
#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
