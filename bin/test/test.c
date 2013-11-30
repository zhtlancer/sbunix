#include <stdio.h>
#include <syscall.h>

#define TEST "TEST\n"

int temp;

int main()
{
	static volatile unsigned int d = 0xdeadbeef;
	int test;
	*(uint64_t *)0xdeadbeef = 0;
	test = printf("%s", TEST);

	printf("%d\n", test);

	while (d)
		;

	printf("%s", TEST);
	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
