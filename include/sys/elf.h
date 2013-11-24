#ifndef _SYS_ELF_H
#define _SYS_ELF_H

#include <defs.h>

#define EI_NIDENT	16

typedef uint64_t elf64_addr_t;
typedef uint16_t elf64_half_t;
typedef uint64_t elf64_off_t;
typedef int32_t elf64_sword_t;
typedef uint32_t elf64_word_t;
typedef uint64_t elf64_xword_t;
typedef int64_t elf64_sxword_t;

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

int parse_elf_executable(const char *name);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
