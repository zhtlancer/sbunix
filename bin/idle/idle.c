#include <syscall.h>
#include <stdio.h>

#define DEBUG_IDLE 1

int main()
{
	pid_t sh_pid = 0;
	for ( ; ; ) {
		/* if sh is still alive, continue */
		if (sh_pid != 0)
			continue;

		sh_pid = fork();
		if (sh_pid == 0) {
			printf("[IDLE] child spawning new sh(%d)\n", sh_pid);
			execve("/bin/sh", NULL, NULL);
		} else {
			printf("[IDLE] spawn new sh(%d)\n", sh_pid);
		}
	}
	return 0;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
