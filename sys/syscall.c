#include <sys/msr.h>
#include <sys/syscall.h>
#include <sys/sched.h>
#include <sys/k_stdio.h>
#include <sys/fs.h>
#include <sys/pit.h>
#include <sys/mm_vma.h>

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

uint64_t sys_fork(struct pt_regs *regs)
{
	regs->rax = fork();
	return regs->rax;
}

uint64_t sys_execve(struct pt_regs *regs)
{
	regs->rax = execve((const char *)regs->rdi, (char *const *)regs->rsi, (char *const *)regs->rdx);
	return regs->rax;
}

uint64_t sys_sleep(struct pt_regs *regs)
{
	uint64_t tick0 = jiffies;
	uint64_t tick = jiffies + ((regs->rdi)*1000);
	while (jiffies < tick) {
		sleep(&jiffies);
	}
	regs->rax = jiffies - tick0;
	return regs->rax;
}

uint64_t sys_wait(struct pt_regs *regs)
{
	regs->rax = wait((int *)regs->rdi);
	return regs->rax;
}

uint64_t sys_waitpid(struct pt_regs *regs)
{
	regs->rax = waitpid(regs->rdi, (int *)regs->rsi, regs->rdx);
	return regs->rax;
}

uint64_t sys_exit(struct pt_regs *regs)
{
	exit(regs->rdi);
	return -1; /* actually we shouldn't return */
}

uint64_t sys_kill(struct pt_regs *regs)
{
	return 0;
}

uint64_t sys_getpid(struct pt_regs *regs)
{
	regs->rax = current->pid;
	return 0;
}

uint64_t sys_open(struct pt_regs *regs)
{
	regs->rax = fd_open((const char *)regs->rdi, regs->rsi, (mode_t)regs->rdx);
	return regs->rax;
}

uint64_t sys_close(struct pt_regs *regs)
{
	regs->rax = fd_close(regs->rdi);
	return regs->rax;
}

uint64_t sys_read(struct pt_regs *regs)
{
	struct file *file = current->files[regs->rdi];
	if (!file->readable || (file->f_ops->read == NULL))
		return 0;
	regs->rax = file->f_ops->read(file, (void *)regs->rsi, regs->rdx);
	return regs->rax;
}

uint64_t sys_write(struct pt_regs *regs)
{
	struct file *file = current->files[regs->rdi];
	if (!file->writeable || file->f_ops->write == NULL) {
		return 0;
	}
	regs->rax = file->f_ops->write(file, (void *)regs->rsi, regs->rdx);
	return regs->rax;
}

uint64_t sys_lseek(struct pt_regs *regs)
{
	struct file *file = current->files[regs->rdi];
	if (file->f_ops->seek == NULL)
		return 0;
	regs->rax = file->f_ops->seek( file, (off_t)(regs->rsi), (int)(regs->rdx) );
	return regs->rax;
}

uint64_t sys_getdents(struct pt_regs *regs)
{
	regs->rax = fd_getdents(regs->rdi, (struct dirent *)regs->rsi, regs->rdx);
	return regs->rax;
}

uint64_t sys_mmap(struct pt_regs *regs)
{
	return 0;
}

uint64_t sys_sbrk(struct pt_regs *regs)
{
	regs->rax = (uint64_t)sbrk(regs->rdi);
	return regs->rax;
}

uint64_t sys_chdir(struct pt_regs *regs)
{
	regs->rax = chdir((const char *)regs->rdi);
	return regs->rax;
}


/*
 * a1: rdi, a2: rsi, a3: rdx, a4: r8, a5: r9, a6: r12
 */
uint64_t syscall_common(struct pt_regs *regs)
{
	uint64_t syscall_no = regs->rax;

	current->tf = regs;

	switch (syscall_no) {
	case SYS_fork:
		return sys_fork(regs);
	case SYS_execve:
		return sys_execve(regs);
	case SYS_sleep:
		return sys_sleep(regs);
	case SYS_wait:
		return sys_wait(regs);
	case SYS_waitpid:
		return sys_waitpid(regs);
	case SYS_exit:
		return sys_exit(regs);
	case SYS_kill:
		return sys_kill(regs);
	case SYS_getpid:
		return sys_getpid(regs);
	case SYS_open:
		return sys_open(regs);
	case SYS_close:
		return sys_close(regs);
	case SYS_read:
		return sys_read(regs);
	case SYS_write:
		return sys_write(regs);
	case SYS_lseek:
		return sys_lseek(regs);
	case SYS_getdents:
		return sys_getdents(regs);
	case SYS_mmap:
		return sys_mmap(regs);
	case SYS_sbrk:
		return sys_sbrk(regs);
	case SYS_chdir:
		return sys_chdir(regs);
	default:
		syscall_error("Undefined syscall number (0x%x)\n", syscall_no);
		break;
	}
	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
