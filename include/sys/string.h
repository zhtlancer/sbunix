#ifndef _STRING_H
#define _STRING_H
/*
 * String operations in Standard C Library
 * (Current implementation is based on GCC built-in functions)
 */

#define memmove	__builtin_memmove
#define memcpy	__builtin_memcpy
#define memset	__builtin_memset

#define strcpy	__builtin_strcpy
#define strncpy	__builtin_strncpy
#define strcmp	__builtin_strcmp
#define strncmp	__builtin_strncmp

#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
