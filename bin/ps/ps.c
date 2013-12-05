#include <ps.h>
#include <stdio.h>
#include <syscall.h>

#define PS_BUF_SZ	100
struct ps_ent ps_buf[PS_BUF_SZ];

const char *state_str [] = {
	[TASK_UNUSED] = "UNUSED",
	[TASK_EMBRYO] = "CREATED",
	[TASK_SLEEPING] = "SLEEPING",
	[TASK_RUNNABLE] = "RUNNABLE",
	[TASK_RUNNING] = "RUNNING",
	[TASK_ZOMBIE] = "ZOMBIE",
};

int main(int argc, char *argv[])
{
	int count;
	int i;

	count = ps(ps_buf, PS_BUF_SZ);
	if (count)
		printf("PID\tSTATE\n");
	for (i = 0; i < count; i++) {
		printf("%d\t", ps_buf[i].pid);
		printf("%s\n", state_str[ps_buf[i].state]);
	}
	return 0;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
