#include <console.h>
#include <printf.h>

/* We currently assume we have an 80x25 console,
 * located at 0xB8000 */
#define BUFFER_BASE	(0xB8000)

#define COL_SIZE	(80)
#define ROW_SIZE	(25)
#define UNIT_SIZE	(2)

#define BUFFER_SIZE	(COL_SIZE * ROW_SIZE * UNIT_SIZE)

int console_init = 0;

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

int init_console(void)
{
	buffer_base = (void *) BUFFER_BASE;
	clear_console();

	console_init = 1;
	return 0;
}

