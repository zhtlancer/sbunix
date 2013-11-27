#include <sys/msr.h>
#include <sys/syscall.h>

#define SYSRET_CS 0x1BUL

#define EFER_SCE	0x0000000000000001UL


int syscall_init(void)
{
	uint64_t temp;
	/* Enable SCE in EFER */
	temp = rdmsr(MSR_ADDR_EFER);
	temp |= EFER_SCE;
	wrmsr(MSR_ADDR_EFER, (uint32_t)(temp & 0xFFFFFFFF), (uint32_t)(temp >> 32));

	/* Setup STAR */
	wrmsr(MSR_ADDR_STAR, 0x0, (SYSRET_CS << 16));

	/* Setup LSTAR */
	/* Setup CSTAR */
	/* Setup SFMASK */
	return 0;
}


/* vim: set ts=4 sw=0 tw=0 noet : */
