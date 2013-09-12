#ifndef _CONSOLE_H
#define _CONSOLE_H

extern int console_init;

int init_console(void);
void clear_console(void);

void putc_con(int ch);

#endif
