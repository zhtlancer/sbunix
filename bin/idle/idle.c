#include <syscall.h>
#include <stdio.h>

#define DEBUG_IDLE 1

#define SHELL_BIN "/bin/sh"

int main()
{
	pid_t sh_pid = 0;
	int status;

	for ( ; ; ) {
		pid_t tmp;
		/* if sh is still alive, continue */
		if (sh_pid > 0) {
			if ((tmp = waitpid(sh_pid, &status, WNOHANG)) == 0)
				continue;

			if (tmp == sh_pid) {
				printf("[IDLE] sh(%d) exited with status %d\n", sh_pid, status);
			} else {
				printf("[IDLE] sh(%d) exited by unknown reason\n", sh_pid);
			}
		}

		sh_pid = fork();
		if (sh_pid == 0) {
			printf("[IDLE] child spawning new sh(%d)\n", sh_pid);
			execve(SHELL_BIN, NULL, NULL);
		} else {
			printf("[IDLE] spawn new sh(%d)\n", sh_pid);
		}
	}
	return 0;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
