#include <sys/dev.h>
#include <sys/ahci.h>
#include <sys/string.h>

struct dev devs[NDEV];

int dev_init(void)
{
	struct dev *dev;
	memset(devs, 0, sizeof(devs));

	/* Initialize console device */
	dev = &devs[DEV_CONSOLE];
	dev->type = DEV_TYPE_CHAR;
	dev->seek = NULL;
	/* TODO */

	/* Initialize TARFS device */
	dev = &devs[DEV_TARFS];
	dev->type = DEV_TYPE_BLOCK | DEV_TYPE_PSEUDO;
	/* TODO */

	/* Initialize AHCI device */
	dev = &devs[DEV_DISK];
	dev->type 			= DEV_TYPE_BLOCK;
	dev->super_block 	= NULL; // will be set in sbfs_init()
	dev->seek			= NULL;
	dev->read			= NULL;
	dev->write			= NULL;
	dev->readsect		= ahci_readsect;
	dev->writesect		= ahci_writesect;
	ahci_init();

	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
