#include <sys/timer.h>
#include <console.h>

uint64_t jiffies;

int timer_init(void)
{
	jiffies = 0;

	return 0;
}

void update_jiffies(void)
{
	jiffies += 1;

	if (!(jiffies % 3))
		update_timer();
}

