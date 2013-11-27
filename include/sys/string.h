#ifndef _SYS_STRING_H
#define _SYS_STRING_H
/*
 * String operations in Standard C Library
 * (Current implementation is based on GCC built-in functions)
 */

#define memmove	__builtin_memmove
#define memcpy	__builtin_memcpy
#define memset	__builtin_memset

#define strcpy	__builtin_strcpy
#define strncpy	__builtin_strncpy

#define strlen __builtin_strlen

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

size_t strlcpy(char *dst, const char *src, size_t n);

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
