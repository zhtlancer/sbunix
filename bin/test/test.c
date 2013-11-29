#include <stdio.h>
#include <syscall.h>

#define TEST "TEST"

int temp;

int main()
{
	static volatile unsigned int d = 0xdeadbeef;
	while (d)
		;
	__syscall3(16, 1, (uint64_t)TEST, 4);

	printf("%s", TEST);
	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
