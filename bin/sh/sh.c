#include <stdio.h>

#define BUF_SIZE 512

char buf[BUF_SIZE];

int main()
{
	int len;

	printf("\n>> Welcome to sbsh, the shell for SBUNIX\n");

	while (1) {
		printf("# ");
		len = gets_l(buf, BUF_SIZE);
		printf("Got(%d): %s\n", len, buf);
	}

	return 0;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
