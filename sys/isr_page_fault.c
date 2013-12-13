
#include <defs.h>
//#include <sys/io.h>
#include <sys/k_stdio.h>
//#include <sys/pic.h>
#include <sys/sched.h>
#include <sys/x86.h>
#include <sys/string.h>

#define PF_EC_P		0x01
#define PF_EC_RW	0x02
#define PF_EC_PL	0x04
#define PF_EC_NX	0x10

#define pf_error(fmt, ...)	\
	k_printf(1, "<PF> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_PF
#define pf_db(fmt, ...)	\
	k_printf(1, "<PF DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define pf_db(fmt, ...)
#endif

void isr_page_fault(uint64_t ec, struct pt_regs *regs)
{
	uint64_t cr2 = rcr2();

	pf_db("Page fault at %p, ec %x\n", cr2, ec);

	/* Non-existing mapping */
	if (!(ec & PF_EC_P) && current != NULL) {
		if ((cr2 < USTACK_TOP) && (cr2 >= USTACK_BOTTOM)) {
			/* grow the user stack */
			pf_error("Allocate memory for user stack region (%p)\n", cr2);
			map_page_self(PGROUNDDOWN(cr2), 1, 0, PG_USR, 0, 0, 0, PGT_RW | PGT_USR);
			return;
		} else if ((cr2 >= current->mm->brk_start) && (cr2 < current->mm->brk_end)) {
			/* map brk region for user process */
			pf_error("Allocate memory for brk region (%p)\n", cr2);
			map_page_self(PGROUNDDOWN(cr2), 1, 0, PG_USR, 0, 0, 0, PGT_RW | PGT_USR);
			return;
		}
	}

	/* caused by mem write, COW page? */
	if ((ec & PF_EC_P) && (ec & PF_EC_RW)) {
		pgt_t *pgt = get_pgt_entry_lv4_self(cr2);
		if ((pgt->avl_1 & PGT_AVL_COW) && (pgt->flag & PGT_USR) && !(pgt->flag & PGT_RW)) {
			page_t *page = get_page_from_pgt(pgt);
			void *va = get_va_from_page(page);
			pf_db("COW page met, create new page.\n");
			remap_page_self(cr2, 1, 0, PG_USR, pgt->nx, pgt->avl_1 & ~PGT_AVL_COW,
					pgt->avl_2, pgt->flag | PGT_RW);
			flush_tlb();
			memcpy((void *)PGROUNDDOWN(cr2), va, __PAGE_SIZE);
		}
		return;
	}

	if (!(ec & PF_EC_PL)) {
		pf_error("Page fault in supervisor mode at %p, ec=%x\n", cr2, ec);
		if (regs)
			pf_error("RIP = %x\n", regs->rip);
		if (current->state == TASK_RUNNING)
			exit(-1);
		panic("Unhandled page fault\n");
	}

	/* Exit execution */
	if (current != NULL) {
		pf_error("SEGV at %p on process %d, killing it..\n", (void *)cr2, current->pid);
		exit(-1);
	}
}
/* vim: set ts=4 sw=0 tw=0 noet : */
