#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char *argv[], char *envp[])
{
	unsigned int *ptr;

	printf("malloc test:\n");
	while ((ptr = (unsigned int *)malloc(512)) != NULL) {
		ptr[0] = 0xdeadbeef;
		printf("%p: %x\n", ptr, ptr[0]);
	}

	return 0;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
