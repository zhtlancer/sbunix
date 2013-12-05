#include <string.h>
#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>

#define CMD_BUF_SIZE 512

#define MAXARGS 10

/* MAXARGS + possible '&' at tail + NULL */
#define MAXARGV_FIELD	12

const char *PATH[] = {
	"/bin",
	"/mnt/bin",
};
#define N_PATH (sizeof(PATH) / sizeof(char *))

char *const envp[] = {
	"PATH=/bin:/mnt/bin",
	"SHELL=/bin/sh",
	NULL,
};

struct command {
	const char *name;
	const char *desc;
	int (*func)(int argc, char **argv);
};

char cmd_buf_in[CMD_BUF_SIZE];
char cmd_buf_out[CMD_BUF_SIZE];

static int builtin_help(int argc, char **argv);
static int builtin_exit(int argc, char **argv);
static int builtin_cd(int argc, char **argv);

static struct command builtin_cmd[] = {
	{ "help", "Display this this list of commands", builtin_help },
	{ "exit", "Exit shell", builtin_exit },
	{ "cd", "Change directory", builtin_cd },
};

#define N_BUILTIN_CMD (sizeof(builtin_cmd) / sizeof(struct command))

static inline int is_space(char ch)
{
	return (ch == ' ') || (ch == '\t');
}

/* parse cmd_buf_in into cmd_buf_out, and record them in argv passed-in */
static int parse_cmd(char *argv[])
{
	int argc = 0;
	int i, j;

	j = 0;
	for (i = 0; argc < MAXARGS && cmd_buf_in[i] != '\0'; i++) {
		if (is_space(cmd_buf_in[i]))
				continue;

		argv[argc++] = &cmd_buf_out[j];
		while (cmd_buf_in[i] != '\0' && !is_space(cmd_buf_in[i]))
			cmd_buf_out[j++] = cmd_buf_in[i++];
		cmd_buf_out[j++] = '\0';
	}

	if (argc >= MAXARGS && cmd_buf_in[i] != '\0') {
		printf("[SH] Too many arguments! At most arguments acceptable.\n");
		return -1;
	}

	return argc;
}

/* return 0 if cmd not treated as builtin, or 1 otherwise */
static int run_builtin_cmd(int argc, char *argv[])
{
	int i;
	int rval;

	for (i = 0; i < N_BUILTIN_CMD; i++) {
		if (strcmp(builtin_cmd[i].name, argv[0]) == 0) {
			rval = builtin_cmd[i].func(argc, argv);
			printf("[SH] built-in command '%s' finished with (%d)\n", builtin_cmd[i].name, rval);
			return 1;
		}
	}
	return 0;
}

/* run external command */
static int run_ext_cmd(int bg, int argc, char *argv[])
{
	int i;
	int len;

	for (i = 0; i < N_PATH; i++) {
		/*len = strlcpy(cmd_buf_in, PATH[i], CMD_BUF_SIZE);*/
		/*cmd_buf_in[len++] = '/';*/
		/*strlcpy(cmd_buf_in+len, argv[0], CMD_BUF_SIZE-len);*/
		/*execve(cmd_buf_in, argv, envp);*/
	}
	strlcpy(cmd_buf_in, argv[0], CMD_BUF_SIZE);
	len = open(cmd_buf_in, 0, 0);
	printf("FD: %d\n", len);
	close(len);
	return 0;
}

static int run_cmd(char *s, int len)
{
	int bg = 0;
	int argc;
	char *argv[MAXARGV_FIELD];

	argc = parse_cmd(argv);

	if (argc < 0)
		return argc;

	/* First we try to run the command as builtin */
	if (run_builtin_cmd(argc, argv))
		return 0;

	/* check if last arg is "&", which indicate background exe */
	if (strcmp("&", argv[argc-1]) == 0) {
		bg = 1;
		argv[argc--] = NULL;
	}

	return run_ext_cmd(bg, argc, argv);
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
		len = gets_l(cmd_buf_in, CMD_BUF_SIZE);
		run_cmd(cmd_buf_in, len);
	}

	return 0;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
