#include <stdio.h>
#include <syscall.h>

#define TEST "TEST\n"

int temp;

int main()
{
	pid_t pid;
	static volatile unsigned int d = 0xdeadbeef;
	int test;
	test = printf("%s", TEST);

	pid = fork();

	if (pid == 0) {
		printf("I'm child\n");
	} else {
		printf("I'm parent, child id=%d\n", pid);
	}

	printf("%d\n", test);

	while (d)
		;

	printf("%s", TEST);
	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
