#include <stdlib.h>
#include <syscall.h>

void *malloc(size_t size)
{
	void *ptr = sbrk(size);

	if (ptr == (void *)(-1))
		return NULL;
	else
		return ptr;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
