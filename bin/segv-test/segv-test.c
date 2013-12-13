#include <stdio.h>

int main(int argc, char *argv[])
{
	int *ptr = (int *)0xdeadbeef;

	printf("Before accessing %p\n", ptr);
	*ptr = 1;
	printf("After accessing %p\n", ptr);
	return 0;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
