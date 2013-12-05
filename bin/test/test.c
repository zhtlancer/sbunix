#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>

#define TEST "TEST\n"

int temp;

int main(int argc, char *argv[], char *envp[])
{
	pid_t pid;
	static volatile unsigned int d = 0xdeadbeef;
	int test;
	test = printf("%s", TEST);
	int i;
	int fd;
	unsigned char buf[512];

	for (i = 0; i < argc; i++)
		printf("ARGV[%d]: %s\n", i, argv[i]);
	for (i = 0; envp[i] != NULL; i++)
		printf("ENVP[%d]: %s\n", i, envp[i]);

	/*fd = open("/mnt/test_dir2/test_file", 0, 0);*/
	fd = open("/bin/hello", 0, 0);

	if (fd < 0) {
		printf("Failed to open file\n");
		exit(-1);
	} else
		printf("file opened: %d\n", fd);

	read(fd, buf, 512);

	for (i = 0; i < 512; i++)
		printf(" %x", buf[i]);

	exit(0);

	while (d);

	pid = fork();

	if (pid == 0) {
		*(uint64_t *)0x7fffffffeff0 = 0xdeadbeef;
		d = 1;
		printf("I'm child, pid=%d\n", getpid());
		test = execve("/bin/hello", NULL, NULL);
		printf("execve %d\n", test);
	} else {
		*(uint64_t *)0x7fffffffeff0 = 0xdeadbeef;
		d = 2;
		printf("I'm parent, child id=%d\n", pid);
	}

	printf("%d\n", test);

	while (d)
		printf("TTTTTEEEESSSSTTTT\n");

	printf("%s", TEST);
	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
