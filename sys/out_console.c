#include <defs.h>
#include <console.h>
#include <printf.h>
#include <sys/timer.h>

/* We currently assume we have an 80x25 console,
 * located at 0xB8000 */
#define BUFFER_BASE	(0xB8000)

#define COL_SIZE	(80)
#define ROW_SIZE	(25)
#define UNIT_SIZE	(2)

#define BUFFER_SIZE	(COL_SIZE * ROW_SIZE * UNIT_SIZE)

int console_inited = 0;

static volatile unsigned char *buffer_base;
static long buffer_pos;

void clear_console(void)
{
	int i;

	for (i = 0; i < BUFFER_SIZE; i++)
		buffer_base[i] = 0;

	buffer_pos = 0;
}

static void putchar_console(unsigned char asc_ctl,
		unsigned char ch)
{
	/* Check if we reach the bottom */
	if (buffer_pos == BUFFER_SIZE) {
		int i, j;
		for (i = 0, j = COL_SIZE;
				j < BUFFER_SIZE;
				i += 1, j += 1) {
			buffer_base[i] = buffer_base[j];
		}

		/* Clear the last line */
		for ( ; i < BUFFER_SIZE; i += 1)
			buffer_base[i] = 0;

		/* Put cursor to the beginning of last line */
		buffer_pos = BUFFER_SIZE - COL_SIZE;
	}

	buffer_base[buffer_pos++] = ch;
	buffer_base[buffer_pos++] = asc_ctl;
}

void putc_con(int ch)
{
	int asc_ctl;
	/* If ASC mode not set, use black on white */
	if ((asc_ctl = ch >> (sizeof(unsigned char)*8)) == 0)
		asc_ctl = 0x07;

	putchar_console(asc_ctl, ch & 0xFF);
}

void putchar_console_pos(int asc_ctl, unsigned char ch, int row, int col)
{
	int pos;
	
	if (row >= ROW_SIZE || col >= COL_SIZE)
		return;
	pos = (row * COL_SIZE + col) * 2;
	buffer_base[pos++] = ch;
	buffer_base[pos] = asc_ctl;
}

int console_init(void)
{
	buffer_base = (void *) BUFFER_BASE;
	clear_console();

	console_inited = 1;
	return 0;
}

void update_timer(void)
{
	uint64_t num = jiffies * 10000 / 182065;
	int buf[64];
	int w = 0, i;

	while (num >= 10) {
		buf[w++] = num % 10;
		num /= 10;
	}

	if (num)
		buf[w++] = num;

	if (w == 0)
		buf[w++] = 0;

	for (i = w; i > 0; i -= 1)
		putchar_console_pos(0x07, "0123456789"[buf[i-1]], 24, w-i);
}

