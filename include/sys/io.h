#ifndef _IO_H
#define _IO_H

// IO inline functions

static inline
void outb( unsigned short port, unsigned char val )
{
    __asm__ volatile( "outb %0, %1"
	    : : "a"(val), "Nd"(port) );
}


static inline
unsigned char inb( unsigned short port )
{
    unsigned char ret;
    __asm__ volatile( "inb %1, %0"
	    : "=a"(ret) : "Nd"(port) );
    return ret;
}

static inline
void outl( uint16_t port, uint32_t val )
{
    __asm__ volatile( "outl %0, %1"
	    : : "a"(val), "Nd"(port) );
}

static inline
uint32_t inl( uint16_t port )
{
    uint32_t ret;
    __asm__ volatile( "inl %1, %0"
	    : "=a"(ret) : "Nd"(port) );
    return ret;
}

static inline
void io_wait( void )
{
    __asm__ volatile( "jmp 1f\n\t"
	    "1:jmp 2f\n\t"
	    "2:" );
}

#endif

/* vim: set ts=4 sw=0 tw=0 noet : */
