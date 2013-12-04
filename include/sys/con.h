#ifndef _SYS_KBD_H
#define _SYS_KBD_H

int is_kbd_buf_empty(void);
int is_kbd_buf_full(void);
void kbd_buf_backspace(void);

int kbd_copy_buf(void *buf, int size);
void console_read_commit(void);
#endif
/* vim: set ts=4 sw=0 tw=0 noet : */
