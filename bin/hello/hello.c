#include <stdio.h>

int main(int argc, char* argv[]) {
	printf("Hello World!\n");
	*(int *)0xdeadbeef = 1;
	return 0;
}
