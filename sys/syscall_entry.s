# This file contains the entries for syscalls

.globl _syscall_lstar
_syscall_lstar:
	jmp .

/* Calling from compatible mode, actually we don't expect this */
.globl _syscall_cstar
_syscall_cstar:
	jmp .

/* vim: set ts=4 sw=0 tw=0 noet : */
