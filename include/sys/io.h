#ifndef _IO_H
#define _IO_H

// IO inline functions

static inline
void outb( unsigned short port, unsigned char val )
{
    asm volatile( "outb %0, %1"
	    : : "a"(val), "Nd"(port) );
}

static inline
unsigned char inb( unsigned short port )
{
    unsigned char ret;
    asm volatile( "inb %1, %0"
	    : "=a"(ret) : "Nd"(port) );
    return ret;
}

static inline
void io_wait( void )
{
    asm volatile( "jmp 1f\n\t"
	    "1:jmp 2f\n\t"
	    "2:" );
}

#endif

/* vim: set ts=4 sw=0 tw=0 noet : */
