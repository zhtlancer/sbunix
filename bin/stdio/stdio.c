#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <fcntl.h>


#define TEST "TEST\n"

int temp;

int main(int argc, char *argv[], char *envp[])
{
	int i;
	char buf[512];


	printf("-------------------------\n");
	printf("|   printf/scanf demo   |\n");
	printf("-------------------------\n");


	printf( "Please input an integer: " );
	scanf( "%d", &i );
	printf( "You enter: %d\n", i );
	printf( "\n" );
	printf( "Please input an string (max 512 chars): " );
	scanf( "%s", buf );
	printf( "You enter: %s\n", buf );
	printf( "\n" );


	exit(0);

}

/* vim: set ts=4 sw=0 tw=0 noet : */
