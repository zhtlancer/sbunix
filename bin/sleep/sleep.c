#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>


int main(int argc, char *argv[], char *envp[])
{
	int second=0;

	if ( argc != 2 ) {
		printf("wrong parameter.\n" );
		exit(0);
	}
	
	second = cstr2i( argv[1] );
	printf( "sleep %d second(s).\n", second );
	sleep( second );
	printf( "I'm back!\n" );

	exit(0);
}

/* vim: set ts=4 sw=0 tw=0 noet : */
