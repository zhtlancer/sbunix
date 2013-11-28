#include <stdio.h>

#define TEST "TEST"

int temp;

int main()
{
	static volatile unsigned int d = 0xdeadbeef;
	while (d)
		;
	__asm__ ("syscall");

	printf("%s", TEST);
	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
