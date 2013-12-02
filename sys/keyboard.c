#include <defs.h>
#include <sys/io.h>
#include <sys/pic.h>

uint64_t kbd_counter;

#define KBD_BUF_SZ	512

uint8_t kbd_buf[KBD_BUF_SZ];
int kbd_buf_pos = 0;

char scanCode2Ascii[128] = { /*0x00*/    0,  27, '1', '2',
    /*0x04*/  '3', '4', '5', '6',
    /*0x08*/  '7', '8', '9', '0',
    /*0x0C*/  '-', '=','\b','\t', 
    /*0x10*/  'q', 'w', 'e', 'r',
    /*0x14*/  't', 'y', 'u', 'i',
    /*0x18*/  'o', 'p', '[', ']',
    /*0x1C*/  '\n',  0, 'a', 's',
    /*0x20*/  'd', 'f', 'g', 'h',
    /*0x24*/  'j', 'k', 'l', ';',
    /*0x28*/  '\'','`',   0,'\\',
    /*0x2C*/  'z', 'x', 'c', 'v',
    /*0x30*/  'b', 'n', 'm', ',',
    /*0x34*/  '.', '/',   0, '*',
    /*0x38*/    0, ' ',   0,   0,
    /*0x3C*/    0,   0,   0,   0,
    /*0x40*/    0,   0,   0,   0,
    /*0x44*/    0,   0,   0,   0,
    /*0x48*/    0,   0, '-',   0,
    /*0x4C*/    0,   0, '+',   0,
    /*0x50*/    0,   0,   0,   0,
    /*0x54*/    0,   0,   0,   0,
    /*0x58*/    0,   0            }; 

char scanCode2AsciiShift[128] = {
    /*0x00*/    0,  27, '!', '@',
    /*0x04*/  '#', '$', '%', '^',
    /*0x08*/  '&', '*', '(', ')',
    /*0x0C*/  '_', '+','\b','\t', 
    /*0x10*/  'Q', 'W', 'E', 'R',
    /*0x14*/  'T', 'Y', 'U', 'I',
    /*0x18*/  'O', 'P', '{', '}',
    /*0x1C*/  '\n',  0, 'A', 'S',
    /*0x20*/  'D', 'F', 'G', 'H',
    /*0x24*/  'J', 'K', 'L', ':',
    /*0x28*/  '\"','~',   0, '|',
    /*0x2C*/  'Z', 'X', 'C', 'V',
    /*0x30*/  'B', 'N', 'M', '<',
    /*0x34*/  '>', '?',   0, '*',
    /*0x38*/    0, ' ',   0,   0,
    /*0x3C*/    0,   0,   0,   0,
    /*0x40*/    0,   0,   0,   0,
    /*0x44*/    0,   0,   0,   0,
    /*0x48*/    0,   0, '-',   0,
    /*0x4C*/    0,   0, '+',   0,
    /*0x50*/    0,   0,   0,   0,
    /*0x54*/    0,   0,   0,   0,
    /*0x58*/    0,   0            }; 

#define KBD_SC1_LSHIFT_P 0x2A 
#define KBD_SC1_RSHIFT_P 0x36 

#define KBD_SC1_LSHIFT_R 0xAA 
#define KBD_SC1_RSHIFT_R 0xB6 

uint08_t kbd_shift;

int k_putchar(unsigned char lvl, const char c);

int is_kbd_buf_empty(void)
{
	return kbd_buf_pos == 0;
}

int is_kbd_buf_full(void)
{
	return kbd_buf_pos >= KBD_BUF_SZ;
}

void kbd_buf_backspace(void)
{
	kbd_buf_pos -= 1;
}

void kbd_fill_echo(uint8_t ch)
{
	if (is_kbd_buf_full() && ch != '\b')
		return;
	if (ch != '\b')
		kbd_buf[kbd_buf_pos++] = ch;
	k_putchar(0, ch);
}

/* FIXME: this extern reference should be removed in the future */
extern addr_t vgatext_vbase;
void isr_kbd()
{
    register char *temp1;
    //register char *temp2;
    uint08_t scanCode[6];
    uint08_t release;
	uint8_t ch;

    scanCode[0] = inb( 0x60 );

    release = (scanCode[0]>>7) ? 1 : 0;

    if      ( ( scanCode[0]==KBD_SC1_LSHIFT_P ||
		scanCode[0]==KBD_SC1_RSHIFT_P    ) &&
	    kbd_shift==0
	    ) 
	kbd_shift = 1;
    else if ( ( scanCode[0]==KBD_SC1_LSHIFT_R ||
		scanCode[0]==KBD_SC1_RSHIFT_R    ) &&
	    kbd_shift==1
	    ) 
	kbd_shift = 0;

	if ( scanCode2Ascii[scanCode[0]]!=0 && !release ) {
		temp1  = (char*)(vgatext_vbase+0xF9A);
		if ( kbd_shift )
			ch = scanCode2AsciiShift[scanCode[0]];
		else
			ch = scanCode2Ascii[scanCode[0]];

		*temp1 = ch;
		kbd_fill_echo(ch);
    }


    kbd_counter++;


    PIC_eoi(1);
}

/* vim: set ts=4 sw=0 tw=0 noet : */
