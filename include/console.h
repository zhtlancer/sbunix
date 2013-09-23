#ifndef _CONSOLE_H
#define _CONSOLE_H

extern int console_inited;

int console_init(void);
void clear_console(void);

void putc_con(int ch);

void update_timer(void);

#endif
