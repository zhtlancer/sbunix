#ifndef _SYS_CONFIG_H
#define _SYS_CONFIG_H

#include <sys/debug.h>

/*
 * Max number of processes
 */
#define NPROC	64

/*
 * Max number of opened file in whole system
 */
#define NFILE	1024

/*
 * Max number of inode structure opened in system
 */
#define NINODE 1024

/*
 * Max number of files opened per file
 */
#define NFILE_PER_PROC	16

/*
 * Max number of devices
 */
#define NDEV	16

/*
 * Physical disk mount point in rootfs
 */
#define DMOUNT "mnt"

#endif
/* vim: set ts=8 sw=4 tw=0 noet : */
