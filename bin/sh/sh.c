#include <string.h>
#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>

#define CMD_BUF_SIZE 512

#define MAXARGS 10

struct command {
	const char *name;
	const char *desc;
	int (*func)(int argc, char **argv);
};

char cmd_buf[CMD_BUF_SIZE];

static int builtin_help(int argc, char **argv);
static int builtin_exit(int argc, char **argv);
static int builtin_cd(int argc, char **argv);

static struct command builtin_cmd[] = {
	{ "help", "Display this this list of commands", builtin_help },
	{ "exit", "Exit shell", builtin_exit },
	{ "cd", "Change directory", builtin_cd },
};

#define N_BUILTIN_CMD (sizeof(builtin_cmd) / sizeof(struct command))

static int run_builtin_cmd()
{
	return 0;
}

static int run_cmd(char *s, int len)
{
	int i;

	for (i = 0; i < N_BUILTIN_CMD; i++)
		if (strcmp(builtin_cmd[i].name, s) == 0)
			return builtin_cmd[i].func(0, NULL);

	run_builtin_cmd();
	return 0;
}

static int builtin_help(int argc, char **argv)
{
	return 0;
}

static int builtin_exit(int argc, char **argv)
{
	exit(0);
	return 0;
}

static int builtin_cd(int argc, char **argv)
{
	return 0;
}

int main(int argc, char *argv[], char *envp[])
{
	int len;

	printf("\n>> Welcome to sbsh, the shell for SBUNIX\n");

	while (1) {
		printf("# ");
		len = gets_l(cmd_buf, CMD_BUF_SIZE);
		run_cmd(cmd_buf, len);
	}

	return 0;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
