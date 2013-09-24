#include <defs.h>
#include <console.h>
#include <printf.h>
#include <sys/timer.h>
#include <sys/io.h>

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

static int video_io_base;

static void move_cursor(uint8_t row, uint8_t col)
{
	int pos;

	if (row > COL_SIZE || col > ROW_SIZE) {
		return;
	}

	pos = row * 80 + col;
	
/*#define BIOS_CURSOR 1*/
#define IO_CURSOR 1
#ifdef BIOS_CURSOR
	asm ("movb $0, %%bh\n\t"
			"movb %0, %%dh\n\t"
			"movb %1, %%dl\n\t"
			"movb $0x02, %%ah\n\t"
			"int $0x10"
			:
			: "g"(row), "g"(col)
			: "%ah", "%bh", "%dx");
#else
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));

	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
#endif
}

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
	{
		int i, j, k = 1;

		for (i = 0; i <= 25; i++)
			for (j = 0; j <= 80; j++) {
				move_cursor(i, j);
				k = 10000;
				while (k-- > 0);
			}
	}

	move_cursor(0, 0);
}

static void putchar_console(unsigned char asc_ctl,
		unsigned char ch)
{
	int is_visible = 0;

	switch (ch) {
		case '\b':
			if (buffer_pos > 0) {
				buffer_pos -= 1;
				buffer_base[buffer_pos] = ' ';
				buffer_base[buffer_pos+1] = asc_ctl;
			}
			break;
		case '\n':
			buffer_pos += COL_SIZE * UNIT_SIZE;
		case '\r':
			buffer_pos -= buffer_pos % (COL_SIZE * UNIT_SIZE);
			break;
		case '\t':
			putchar_console(asc_ctl, ' ');
			putchar_console(asc_ctl, ' ');
			putchar_console(asc_ctl, ' ');
			putchar_console(asc_ctl, ' ');
			break;
		default:
			is_visible = 1;
			break;

	}
	/* Check if we reach the bottom */
	if (buffer_pos >= CON_BUFFER_SIZE) {
		int i, j;
		for (i = 0, j = COL_SIZE * UNIT_SIZE;
				j < CON_BUFFER_SIZE;
				i += 1, j += 1) {
			buffer_base[i] = buffer_base[j];
		}

		/* Clear the last line */
		for ( ; i < CON_BUFFER_SIZE; i += 1)
			buffer_base[i] = 0;

		/* Put cursor to the beginning of last line */
		buffer_pos = CON_BUFFER_SIZE - COL_SIZE * UNIT_SIZE;
	}

	if (is_visible) {
		buffer_base[buffer_pos++] = ch;
		buffer_base[buffer_pos++] = asc_ctl;
	}

	move_cursor(buffer_pos / 2 / COL_SIZE, (buffer_pos / 2) % COL_SIZE);
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
	video_io_base = inw(0x0463);
	clear_console();

	printf("Video base addr = 0x%x\n", video_io_base);

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

#define KBD_STATUS_LENGTH	25
void update_kbd(const char *str)
{
	int i;
	
	for (i = 0; i < KBD_STATUS_LENGTH; i++)
		putchar_console_pos(SL_ASCII, str[i], SL_ROW_START, i);
}
