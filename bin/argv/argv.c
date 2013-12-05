#include <defs.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
	int i;

	printf("Totally %d args:\n", argc);
	for (i = 0; i < argc; i++)
		printf("\targv[%d]: %s\n", i, argv[i]);

	printf("envs:\n", argc);
	for (i = 0; envp[i] != NULL; i++)
		printf("\tenvp[%d]: %s\n", i, envp[i]);

	return 0;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
