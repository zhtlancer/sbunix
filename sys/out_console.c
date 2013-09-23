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

#define CON_ROW_SIZE	(24)
#define CON_BUFFER_SIZE	(COL_SIZE * CON_ROW_SIZE * UNIT_SIZE)

/* Status line macros */
#define SL_ROW_START	24
#define SL_ROW_SIZE	1
#define SL_BUFFER_START	(CON_BUFFER_SIZE)
#define SL_BUFFER_SIZE	(COL_SIZE * SL_ROW_SIZE * UNIT_SIZE)
#define SL_BUFFER_END	(CON_BUFFER_SIZE + SL_BUFFER_SIZE)
#define SL_ASCII	0xF1


int console_inited = 0;

static volatile unsigned char *buffer_base;
static long buffer_pos;

void clear_console(void)
{
	int i;

	for (i = 0; i < CON_BUFFER_SIZE; i++)
		buffer_base[i] = 0;

	for (i = SL_BUFFER_START;i < SL_BUFFER_END; ) {
		buffer_base[i++] = ' ';
		buffer_base[i++] = SL_ASCII;
	}

	buffer_pos = 0;
}

static void putchar_console(unsigned char asc_ctl,
		unsigned char ch)
{
	/* Check if we reach the bottom */
	if (buffer_pos == CON_BUFFER_SIZE) {
		int i, j;
		for (i = 0, j = COL_SIZE;
				j < CON_BUFFER_SIZE;
				i += 1, j += 1) {
			buffer_base[i] = buffer_base[j];
		}

		/* Clear the last line */
		for ( ; i < CON_BUFFER_SIZE; i += 1)
			buffer_base[i] = 0;

		/* Put cursor to the beginning of last line */
		buffer_pos = CON_BUFFER_SIZE - COL_SIZE;
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
	uint64_t num[3];
	int buf[64];
	int w = 0, i;

	num[0] = jiffies * 10000 / 182065;
	num[2] = num[0] / 3600;
	num[0] %= 3600;
	num[1] = num[0] /60;
	num[0] %= 60;

	for (i = 0; i < 3; i++) {
		while (num[i] >= 10) {
			buf[w++] = num[i] % 10 + '0';
			num[i] /= 10;
		}

		if (num[i])
			buf[w++] = num[i] + '0';

		while (w < (i+1)*2+i)
			buf[w++] = '0';
		buf[w++] = ':';
	}

	w -= 1; /* Remove uneeded ':' */
	buf[w++] = ' '; buf[w++] = 'E'; buf[w++] = 'M'; buf[w++] = 'I'; buf[w++] = 'T'; buf[w++] = 'P'; buf[w++] = 'U';

	for (i = 0; i < w; i++)
		putchar_console_pos(SL_ASCII, buf[i], SL_ROW_START, COL_SIZE-i-1);
}

