#include <sys/dev.h>

#define DEV_CONSOLE	1
#define DEV_TARFS	2
#define DEV_DISK	3

struct dev devs[NDEV];

int dev_init(void)
{
	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
