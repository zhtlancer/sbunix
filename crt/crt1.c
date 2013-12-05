#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]);

void _start(int argc, char *argv[], char *envp[]) {
	int res;
	res = main(argc, argv, envp);
	exit(res);
}
