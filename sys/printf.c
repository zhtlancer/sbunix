#include <defs.h>
#include <vaargs.h>
#include <console.h>
#include <serial.h>

#include <printf.h>

static void putc(int ch, int *cnt)
{
	putc_con(ch);

	*cnt += 1;
}

static void printnum(unsigned long long num,
		void (*act)(int, void*),
		void *cnt,
		int base,
		int pad,
		int width,
		int asc_mask)
{
	int buf[64];
	int w = 0;

	if (num < 0) {
		act(asc_mask | '-', cnt);
		num = -num;
	}
	while (num >= base) {
		buf[w++] = num % base;
		num /= base;
	}
	if (num)
		buf[w++] = num;

	if (width == 0 && w == 0)
		buf[w++] = 0;

	for (width -= w; width > 0; width -= 1)
		act(asc_mask | pad, cnt);

	for ( ; w > 0; w -= 1)
		act(asc_mask | "0123456789abcdef"[buf[w-1]], cnt);
}

void vprintfmt(void (*act)(int, void*),
		void *cnt,
		const char *fmt,
		va_list ap)
{
	register int ch = 0;
	register const char *p;
	unsigned long long num;
	unsigned int asc_mask = 0x0700;

	if (fmt == NULL) {
		return;
	}

	while (1) {
		while ((ch = *fmt++) != '%') {
			if (ch == '\0')
				return;

			/* Deal with ASC controlling string */
			if (ch == '\27' && (ch = *fmt++) == '[') {
				unsigned int asc_update = 0;
				while (1) {
					ch = *fmt++;
					if (ch == 'm') {
						if (!asc_update)
							asc_mask = 0x0700;
						break;
					} else if (ch == ';') {
						if (!asc_update)
							asc_mask = 0x0700;
					} else if (ch == '0') {
						asc_mask = 0;
						asc_update = 1;
					} else if (ch == '3') {
						ch = *fmt++;
						if (ch == '\0')
							return;
						asc_mask &= ~0x0F00;
						asc_mask |= (ch - '0') << 8;
						asc_update = 1;
					} else if (ch == '4') {
						ch = *fmt++;
						if (ch == '\0')
							return;
						asc_mask &= ~0xF000;
						asc_mask |= (ch - '0') << 12;
						asc_update = 1;
					} else {
						act(asc_mask | '?', cnt);
						act(asc_mask | '?', cnt);
						act(asc_mask | '?', cnt);
					}
				}
			} else {
				act(asc_mask | ch, cnt);
			}
		}

		/* Deal with %-escaped string */
		switch (ch = *fmt++) {
			case 'c':
				act(asc_mask | va_arg(ap, int), cnt);
				break;
			case 'd':
				num = (unsigned long long) va_arg(ap, int);
				printnum(num, act, cnt, 10, 0, 0, asc_mask);
				break;
			case 'p':
				num = (unsigned long long) va_arg(ap, void *);
				act(asc_mask | '0', cnt);
				act(asc_mask | 'x', cnt);
				printnum(num, act, cnt, 16, '0', 16, asc_mask);
				break;
			case 's':
				if ((p = va_arg(ap, char *)) == NULL)
					p = "(null)";
				for ( ; *p != '\0'; p += 1)
					act(asc_mask | *p, cnt);
				break;
			case 'x':
				num = (unsigned long long) va_arg(ap, int);
				printnum(num, act, cnt, 16, 0, 0, asc_mask);
				break;
			case '%':
				act(asc_mask | '%', cnt);
				break;
			default:
				act(asc_mask | '%', cnt);
				act(asc_mask | ch, cnt);
				break;
		}
	}
}

int vprintf(const char *fmt, va_list ap)
{
	int cnt = 0;

	vprintfmt((void *)putc, &cnt, fmt, ap);
	return cnt;
}

int printf(const char *fmt, ...)
{
	va_list ap;
	int cnt = 0;

	va_start(ap, fmt);
	cnt = vprintf(fmt, ap);
	va_end(ap);

	return cnt;
}

