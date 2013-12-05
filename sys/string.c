#include <defs.h>
#include <sys/string.h>

/* TODO: need a more powerful implementation */
void *memset(void *s, int c, size_t n)
{
	char *p;
	size_t m;

	p = s;
	m = n;
	while (m-- > 0)
		*p++ = c;

	return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
#if 0
	while (n-- > 0) {
		*(uint8_t *)dest = *(uint8_t *)src;
		++dest, ++src;
	}

	return dest;
#endif
	return memmove(dest, src, n);
}

void *memmove(void *dest, const void *src, size_t n)
{
	const char *s;
	char *d;

	s = src;
	d = dest;
	if (s < d && s + n > d) {
		s += n;
		d += n;
		while (n-- > 0)
			*--d = *--s;
	} else {
		while (n-- > 0)
			*d++ = *s++;
	}

	return dest;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s1 == *s2)
		++s1, ++s2;
	return (int) ((unsigned char)*s1 - (unsigned char)*s2);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	while (n > 0 && *s1 && *s1 == *s2)
		--n, ++s1, ++s2;

	if (n == 0)
		return 0;
	else
		return (int) ((unsigned char)*s1 - (unsigned char)*s2);
}

size_t strlcpy(char *dst, const char *src, size_t size)
{
	char *dst_in = dst;

	if (size > 0) {
		while (--size > 0 && *src != '\0')
			*dst++ = *src++;
		*dst = '\0';
	}

	return dst - dst_in;
}

size_t strnlen(const char *s, size_t maxlen)
{
	size_t len = 0;

	while (len < maxlen && s[len] != '\0')
		++len;

	return len;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
