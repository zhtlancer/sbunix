#include <sys/elf.h>
#include <sys/error.h>
#include <sys/tarfs.h>
#include <sys/k_stdio.h>

#include <sys/mm.h>
#include <sys/mm_types.h>
#include <sys/mm_page_table.h>
#include <sys/sched.h>
#include <sys/string.h>
#include <sys/sched.h>
#include <sys/fs.h>

#define elf_error(fmt, ...)	\
	k_printf(1, "<ELF> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_ELF
#define elf_db(fmt, ...)	\
	k_printf(1, "<ELF DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define elf_db(fmt, ...)
#endif

#define TEST_ELF 0

static int verify_elf(struct elf64_header *hdr)
{
	return *(elf_magic_t *)hdr != ELF_MAGIC;
}

#if TEST_ELF
static int elf_test(const char *name)
{
	int i;
	struct elf64_header hdr;
	struct elf64_phdr phdr[10];

	TAR_FILE *fp = tarfs_fopen(name);
	if (fp == NULL) {
		k_printf(1, "Failed to open ELF file (%s)\n", name);
		return -ENOENT;
	}

	tarfs_fread(&hdr, sizeof(hdr), 1, fp);

	if (verify_elf(&hdr)) {
		k_printf(1, "Failed to verify ELF file (%s)\n", name);
		return -ENOEXEC;
	}

	k_printf(1, "type: %d ", hdr.type);
	k_printf(1, "machine: %d ", hdr.machine);
	k_printf(1, "version: %d ", hdr.version);
	k_printf(1, "entry: %x ", hdr.entry);
	k_printf(1, "phoff: %x ", hdr.phoff);
	k_printf(1, "shoff: %x ", hdr.shoff);
	k_printf(1, "flags: %x ", hdr.flags);
	k_printf(1, "ehsize: %d ", hdr.ehsize);
	k_printf(1, "phentisize: %d ", hdr.phentisize);
	k_printf(1, "phnum: %d ", hdr.phnum);
	k_printf(1, "shentsize: %d ", hdr.shentsize);
	k_printf(1, "shnum: %d ", hdr.shnum);
	k_printf(1, "shstrndx: %d ", hdr.shstrndx);
	k_printf(1, "\n");

	tarfs_fseek(fp, hdr.phoff, TARFS_SEEK_SET);
	tarfs_fread(&phdr, sizeof(struct elf64_phdr), hdr.phnum, fp);

	for (i = 0; i < hdr.phnum; i++) {
		k_printf(1, "PHDR[%d]: ", i);

		k_printf(1, "type: %x ", phdr[i].type);
		k_printf(1, "flags: %x ", phdr[i].flags);
		k_printf(1, "offset: %x ", phdr[i].offset);
		k_printf(1, "vaddr: %x ", phdr[i].vaddr);
		k_printf(1, "paddr: %x ", phdr[i].paddr);
		k_printf(1, "filesz: %x ", phdr[i].filesz);
		k_printf(1, "memsz: %x ", phdr[i].memsz);
		k_printf(1, "align: %x ", phdr[i].align);
		k_printf(1, "\n");
	}

	{
		volatile int d = 1;
		uint32_t *temp = (uint32_t *)phdr[0].vaddr;
		while (d);
		map_page_self(phdr[0].vaddr, 1, 0, PG_USR | PGT_RW | PGT_EXE, 0, 0, 0, PGT_RW | PGT_EXE | PGT_USR);
		temp[0] = 0xdeadbeef;
		k_printf(1, "%x\n", temp[0]);
		tarfs_fseek(fp, phdr[0].offset, TARFS_SEEK_SET);
		tarfs_fread((void *)phdr[0].vaddr, 1, phdr[0].filesz, fp);
		map_page_self(USTACK_TOP - __PAGE_SIZE, 1, 0, PG_USR | PGT_RW, 0, 0, 0, PGT_RW | PGT_USR);
		while (d);
		_jump_to_usermode((void *)0x4000B0, (void *)USTACK_TOP - 8);
	}

	return 0;
}
#endif

int parse_elf_executable(struct elf64_executable *exe)
{
	int i, rval = 0;
	struct elf64_header hdr;
	struct elf64_phdr phdr;
	struct inode *iexe;

	elf_db("Parsing ELF '%s'\n", exe->name);

	iexe = path_lookup(NULL, exe->name);
	if (iexe == NULL) {
		elf_error("Cannot open %s!\n", exe->name);
		panic("Failed to load the first user executable!\n");
	}
	iexe->fs_ops->read(iexe, &hdr, 0, sizeof(hdr));

	if (verify_elf(&hdr)) {
		elf_error("Verification error\n");
		rval = -ENOEXEC;
		goto fail;
	}

	if (hdr.type != 2) {
		elf_error("Unsupported ELF file (%d)\n", hdr.type);
		rval = -ENOEXEC;
		goto fail;
	}

	exe->entry = (void *)hdr.entry;

	elf_db("ELF entry: %p\n", exe->entry);

	for (i = 0; i < hdr.phnum; i++) {
		iexe->fs_ops->read(iexe, &phdr, hdr.phoff+i*sizeof(phdr), sizeof(phdr));
		elf_db("phdr[%x,%x]: %x\n", phdr.type, phdr.flags);
		switch (phdr.type) {
		case PT_LOAD:
			if (phdr.flags & ELF_PH_FLAG_X) {
				/* .text */
				elf_db(".text segment, filesz=%x, memsz=%x\n",
						phdr.filesz, phdr.memsz);
				exe->code_start = (void *)phdr.vaddr;
				exe->code_size = phdr.memsz;
				exe->code_offset = phdr.offset;
			} else {
				elf_db(".data segment, filesz=%x, memsz=%x bss=%x\n",
						phdr.filesz, phdr.memsz, phdr.memsz - phdr.filesz);
				exe->data_start = (void *)phdr.vaddr;
				exe->data_size = phdr.filesz;
				exe->data_offset = phdr.offset;
				exe->bss_size = phdr.memsz - phdr.filesz;
			}
			break;
		case PT_GNU_STACK:
			elf_db("Stack Segment, ignored (flags=%x offset=%x vaddr=%x paddr=%x filesz=%x memsz=%x)\n",
					phdr.flags, phdr.offset, phdr.vaddr, phdr.paddr, phdr.filesz, phdr.memsz);
			break;
		default:
			elf_error("Unexpected Segment type (%x)\n", phdr.type);
			rval = -ENOEXEC;
			goto fail;
		}
	}

#if TEST_ELF
	rval = elf_test(name);
#endif

fail:
	put_inode(iexe);
	return rval;
}

int load_elf(mm_struct_t *mm, struct elf64_executable *exe)
{
	int i, rval = 0;
	struct elf64_header hdr;
	struct elf64_phdr phdr;
	struct inode *iexe;

	elf_db("Loading ELF '%s'\n", exe->name);

	iexe = path_lookup(NULL, exe->name);
	if (iexe == NULL) {
		elf_error("Cannot open %s!\n", exe->name);
		panic("Failed to load the first user executable!\n");
	}
	iexe->fs_ops->read(iexe, &hdr, 0, sizeof(hdr));

	for (i = 0; i < hdr.phnum; i++) {
		uint8_t flags = PGT_USR;
		uint8_t nx = PGT_NX;

		void *usr_addr;
		size_t size;

		iexe->fs_ops->read(iexe, &phdr, hdr.phoff + i*sizeof(phdr), sizeof(phdr));

		if (phdr.type != PT_LOAD)
			continue;
		if (phdr.flags & ELF_PH_FLAG_W)
			flags |= PGT_RW;
		if (phdr.flags & ELF_PH_FLAG_X)
			nx = PGT_EXE;
		elf_db("[%d] flags = %x\n", i, flags);

		/* Allocate physical memory and load file content */
		usr_addr = (void *)phdr.vaddr;
		size = 0;
		{
			/* the first 4KiB data */
			size_t avail_size = __PAGE_SIZE - (phdr.vaddr & __PAGE_SIZE_MASK);
			page_t *page = alloc_page(flags);
			void *k_addr = (void *)page->va;
			memset(k_addr, 0, __PAGE_SIZE);
			k_addr += phdr.vaddr & __PAGE_SIZE_MASK; /* adjust to the vaddr */
			if (size < phdr.filesz) {
				size_t sec_size = phdr.filesz - size;
				sec_size = sec_size < avail_size ? sec_size : avail_size;
				iexe->fs_ops->read(iexe, k_addr, phdr.offset+size, sec_size);
			}
			/* FIXME: Is it OK to use the same flags for page and page-table? */
			elf_db("mapping %p, flags = %x\n", usr_addr + size, flags);
			map_page(mm->pgt, (addr_t)usr_addr+size,
					0, get_pa_from_page(page),
					PG_USR, nx, 0, 0, flags);
			size += avail_size;
		}
		while (size < phdr.memsz) {
			page_t *page = alloc_page(flags);
			/* The vaddr may be not at the start of page */
			void *k_addr = (void *)page->va + ((uint64_t)phdr.vaddr & __PAGE_SIZE_MASK);

			memset(k_addr, 0, __PAGE_SIZE);
			if (size < phdr.filesz) {
				size_t sec_size = phdr.filesz - size;
				sec_size = sec_size < __PAGE_SIZE ? sec_size : __PAGE_SIZE;
				iexe->fs_ops->read(iexe, k_addr, phdr.offset+size, sec_size);
			}

			/* FIXME: Is it OK to use the same flags for page and page-table? */
			map_page(mm->pgt, (addr_t)usr_addr+size,
					0, get_pa_from_page(page),
					PG_USR, nx, 0, 0, flags);
			size += __PAGE_SIZE;
		}
	}

	put_inode(iexe);

	return rval;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
