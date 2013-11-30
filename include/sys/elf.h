#ifndef _SYS_ELF_H
#define _SYS_ELF_H

#include <defs.h>
#include <sys/sched.h>

#define EI_NIDENT	16

typedef uint64_t elf64_addr_t;
typedef uint16_t elf64_half_t;
typedef uint64_t elf64_off_t;
typedef int32_t elf64_sword_t;
typedef uint32_t elf64_word_t;
typedef uint64_t elf64_xword_t;
typedef int64_t elf64_sxword_t;

/* ELF magic number at start of file */
#define ELF_MAGIC	0x464C457F
typedef uint32_t elf_magic_t;

/*
 * Structure for ELF64 Header
 */
struct elf64_header {
	unsigned char	ident[EI_NIDENT];
	elf64_half_t	type;
	elf64_half_t	machine;
	elf64_word_t	version;
	elf64_addr_t	entry;
	elf64_off_t		phoff;
	elf64_off_t		shoff;
	elf64_word_t	flags;
	elf64_half_t	ehsize;
	elf64_half_t	phentisize;
	elf64_half_t	phnum;
	elf64_half_t	shentsize;
	elf64_half_t	shnum;
	elf64_half_t	shstrndx;
};

/*
 * Program Header types
 */
enum PH_TYPE {
	PT_NULL = 0x0,
	PT_LOAD	= 0x1,
	PT_DYNAMIC = 0x2,
	PT_GNU_STACK = 0x6474e551,
};

#define ELF_PH_FLAG_X	0x01
#define ELF_PH_FLAG_W	0x02
#define ELF_PH_FLAG_R	0x04

/*
 * Structure for ELF64 Program Header
 */
struct elf64_phdr {
	elf64_word_t	type;
	elf64_word_t	flags;
	elf64_off_t		offset;
	elf64_addr_t	vaddr;
	elf64_addr_t	paddr;
	elf64_xword_t	filesz;
	elf64_xword_t	memsz;
	elf64_xword_t	align;
};

/*
 * Structure for ELF64 Section Header
 */
struct elf64_shdr {
	elf64_word_t	name;
	elf64_word_t	type;
	elf64_xword_t	flags;
	elf64_addr_t	addr;
	elf64_off_t		offset;
	elf64_xword_t	size;
	elf64_word_t	link;
	elf64_word_t	info;
	elf64_xword_t	addralign;
	elf64_xword_t	entsize;
};

#define ELF_NAME_MAX 100

struct elf64_executable {
	char name[ELF_NAME_MAX];
	void *entry;
	void *code_start;
	size_t code_size;
	uint64_t code_offset;
	void *data_start;
	size_t data_size;
	uint64_t data_offset;
	size_t bss_size;
};

int parse_elf_executable(struct elf64_executable *exe);

int load_elf(struct task_struct *task, struct elf64_executable *exe);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
