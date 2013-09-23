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
		int asc_ctl)
{
	int buf[64];
	int w = 0;

	if (num < 0) {
		act(asc_ctl | '-', cnt);
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
		act(asc_ctl | pad, cnt);

	for ( ; w > 0; w -= 1)
		act(asc_ctl | "0123456789abcdef"[buf[w-1]], cnt);
}

void vprintfmt(void (*act)(int, void*),
		void *cnt,
		const char *fmt,
		va_list ap)
{
	register int ch = 0;
	register const char *p;
	unsigned long long num;
	unsigned int asc_ctl = 0;

	if (fmt == NULL) {
		return;
	}

	while (1) {
		while ((ch = *fmt++) != '%') {
			if (ch == '\0')
				return;

			/* Deal with ASC controlling string */
			if (ch == '\27') {
				asc_ctl = 0;
			} else {
				act(asc_ctl | ch, cnt);
			}
		}

		/* Deal with %-escaped string */
		switch (ch = *fmt++) {
			case 'c':
				act(asc_ctl | va_arg(ap, int), cnt);
				break;
			case 'd':
				num = (unsigned long long) va_arg(ap, int);
				printnum(num, act, cnt, 10, 0, 0, asc_ctl);
				break;
			case 'p':
				num = (unsigned long long) va_arg(ap, void *);
				act(asc_ctl | '0', cnt);
				act(asc_ctl | 'x', cnt);
				printnum(num, act, cnt, 16, '0', 16, asc_ctl);
				break;
			case 's':
				if ((p = va_arg(ap, char *)) == NULL)
					p = "(null)";
				for ( ; *p != '\0'; p += 1)
					act(asc_ctl | *p, cnt);
				break;
			case 'x':
				num = (unsigned long long) va_arg(ap, int);
				printnum(num, act, cnt, 16, 0, 0, asc_ctl);
				break;
			case '%':
				act(asc_ctl | '%', cnt);
				break;
			default:
				act(asc_ctl | '%', cnt);
				act(asc_ctl | ch, cnt);
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

