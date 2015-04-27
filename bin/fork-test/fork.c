#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>


int main(int argc, char *argv[], char *envp[])
{
	pid_t pid;

	printf("Fork test\n");

	pid = fork();

	if (pid == 0) {
		printf("\tI'm child, pid=%d\n", getpid());
	} else {
		int status;
		printf("\tI'm parent, child id=%d, waiting for child to exit...\n", pid);
		wait(&status);
		printf("\tChild exited with status (%d).\n", status);
	}
}
/* vim: set ts=4 sw=0 tw=0 noet : */
