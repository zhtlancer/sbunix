#ifndef _IO_H
#define _IO_H

#include <defs.h>

static inline void outb(int port, uint8_t val)
{
	asm volatile ("outb %0, %w1": :"a"(val), "d"(port));
}

static inline uint8_t inb(int port)
{
	uint8_t val;

	asm volatile ("inb %w1, %0":"=a"(val):"d"(port));

	return val;
}

static inline uint16_t inw(int port)
{
	uint16_t val;

	asm volatile ("inw %w1, %0":"=a"(val):"d"(port));

	return val;
}

#endif
