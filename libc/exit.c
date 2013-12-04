#include <syscall.h>

void exit(int status) {
	_exit(status);
}
