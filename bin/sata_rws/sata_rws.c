#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <fcntl.h>


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
	uint08_t buf[512];

	//for (i = 0; i < argc; i++)
	//	printf("ARGV[%d]: %s\n", i, argv[i]);
	//for (i = 0; envp[i] != NULL; i++)
	//	printf("ENVP[%d]: %s\n", i, envp[i]);

	printf("---------------------------------\n");
	printf("|   SATA Read/Write/Seek demo   |\n");
	printf("---------------------------------\n");

	printf("Open file from SATA disk:  /mnt/test_dir2/test_file\n");
	fd = open("/mnt/test_dir2/test_file", O_RDWR, 0);

	if (fd < 0) {
		printf("Failed to open file\n");
		exit(-1);
	} else
		printf("File opened: fd %d\n", fd);

	printf("Read 512 byte from file.\n");
	read(fd, buf, 512);

	printf("Print first 16 bytes:\n");
	for (i = 0; i < 16; i++) {
		if ( !(i%16) && i )
			printf(" \n" );
		printf(" %2x", buf[i]);
	}
	printf(" \n" );

	printf("Seek to the beginning of the file.\n");
	lseek( fd, 0,  SEEK_SET );

	printf("Write 512 byte different data to file.\n");
	for (i = 0; i < 512; i++ ) {
		buf[i] = ~(buf[i]);
	}
	write(fd, buf, 512);

	printf("Seek to the beginning of the file again.\n");
	lseek( fd, 0,  SEEK_SET );

	printf("Read from file again.\n");
	read(fd, buf, 512);

	printf("Print first 16 bytes again:\n");
	for (i = 0; i < 16; i++) {
		if ( !(i%16) && i )
			printf(" \n" );
		printf(" %2x", buf[i]);
	}
	printf("\n");

	printf("New content has been wrote to disk.\n");


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
