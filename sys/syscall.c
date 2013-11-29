#include <sys/msr.h>
#include <sys/syscall.h>
#include <sys/sched.h>
#include <sys/k_stdio.h>
#include <sys/fs.h>

#define SYSCALL_CS	0x08
#define SYSRET_CS	0x1B

#define EFER_SCE	0x0000000000000001UL

#define RFLAGS_IF	0x200

#define syscall_error(fmt, ...)	\
	k_printf(1, "<SYSCALL> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_SYSCALL
#define syscall_db(fmt, ...)	\
	k_printf(1, "<SYSCALL DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define syscall_db(fmt, ...)
#endif

int syscall_init(void)
{
	uint64_t temp;
	/* Enable SCE in EFER */
	temp = rdmsr(MSR_ADDR_EFER);
	temp |= EFER_SCE;
	wrmsr(MSR_ADDR_EFER, (uint32_t)(temp & 0xFFFFFFFF), (uint32_t)(temp >> 32));

	/* Setup STAR */
	wrmsr(MSR_ADDR_STAR, 0x0, (SYSRET_CS << 16) | (SYSCALL_CS));

	/* Setup LSTAR */
	wrmsr(MSR_ADDR_LSTAR, (uint64_t)_syscall_lstar & 0xFFFFFFFF, (uint64_t)_syscall_lstar >> 32);

	/* Setup CSTAR */
	wrmsr(MSR_ADDR_CSTAR, (uint64_t)_syscall_cstar & 0xFFFFFFFF, (uint64_t)_syscall_cstar >> 32);

	/* Setup SFMASK */
	temp = 0;
	temp |= RFLAGS_IF;
	wrmsr(MSR_ADDR_SFMASK, temp & 0xFFFFFFFF, temp >> 32);

	return 0;
}

uint64_t sys_write(struct context *ctx)
{
	struct file *file = current->files[ctx->rdi];
	if (!file->writeable || file->f_ops->write == NULL)
		return 0;
	return file->f_ops->write(file, (void *)ctx->rsi, ctx->rdx);
}

uint64_t syscall_common(struct context *ctx)
{
	uint64_t syscall_no = ctx->rax;

	switch (syscall_no) {
	case SYS_write:
		return sys_write(ctx);
	default:
		syscall_error("Undefined syscall number (0x%x)\n", syscall_no);
		break;
	}
	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
