#include <sys/elf.h>
#include <sys/error.h>
#include <sys/tarfs.h>
#include <sys/k_stdio.h>

static int verify_elf(struct elf64_header *hdr)
{
	return *(elf_magic_t *)hdr != ELF_MAGIC;
}

int parse_elf_executable(const char *name)
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

	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
