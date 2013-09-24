#include <defs.h>
#include <printf.h>

#define SC_P_ESC	0x0100
#define SC_P_BS		0x0200
#define SC_P_TAB	0x0300
#define SC_P_ENTER	0x0400
#define SC_P_L_CTRL	0x0500
#define SC_P_L_SHIFT	0x0600
#define SC_P_R_SHIFT	0x0700
#define SC_P_L_ALT	0x0800
#define SC_P_SPACE	0x0900
#define SC_P_CAPSLOCK	0x0A00
#define SC_P_F1		0x0B00
#define SC_P_F2		0x0C00
#define SC_P_F3		0x0D00
#define SC_P_F4		0x0E00
#define SC_P_F5		0x0F00
#define SC_P_F6		0x1000
#define SC_P_F7		0x1100
#define SC_P_F8		0x1200
#define SC_P_F9		0x1300
#define SC_P_F10	0x1400
#define SC_P_F11	0x1500
#define SC_P_F12	0x1600
#define SC_P_NUMLOCK	0x1700
#define SC_P_SCLOCK	0x1800

#define SC_R_ESC	0x8100
#define SC_R_BS		0x8200
#define SC_R_TAB	0x8300
#define SC_R_ENTER	0x8400
#define SC_R_L_CTRL	0x8500
#define SC_R_L_SHIFT	0x8600
#define SC_R_R_SHIFT	0x8700
#define SC_R_L_ALT	0x8800
#define SC_R_SPACE	0x8900
#define SC_R_CAPSLOCK	0x8A00
#define SC_R_F1		0x8B00
#define SC_R_F2		0x8C00
#define SC_R_F3		0x8D00
#define SC_R_F4		0x8E00
#define SC_R_F5		0x8F00
#define SC_R_F6		0x9000
#define SC_R_F7		0x9100
#define SC_R_F8		0x9200
#define SC_R_F9		0x9300
#define SC_R_F10	0x9400
#define SC_R_F11	0x9500
#define SC_R_F12	0x9600
#define SC_R_NUMLOCK	0x9700
#define SC_R_SCLOCK	0x9800



static uint16_t scancode_table[] = {
	0, SC_P_ESC, '1', '2', '3',	/* 0x00 - 0x04 */
	'4', '5', '6', '7', '8',	/* 0x05 - 0x09 */
	'9', '0', '-', '=', SC_P_BS,	/* 0x0A - 0x0E */
	SC_P_TAB, 'q', 'w', 'e', 'r',	/* 0x0F - 0x13 */
	't', 'y', 'u', 'i', 'o',	/* 0x14 - 0x18 */
	'p', '[', ']', SC_P_ENTER, SC_P_L_CTRL,	/* 0x19 - 0x1D */
	'a', 's', 'd', 'f', 'g',	/* 0x1E - 0x22 */
	'h', 'j', 'k', 'l', ';',	/* 0x23 - 0x27 */
	'\'', '`', SC_P_L_SHIFT, '\\', 'z',	/* 0x28 - 0x2C */
	'x', 'c', 'v', 'b', 'n',	/* 0x2D - 0x31 */
	'm', ',', '.', '/', SC_P_R_SHIFT,	/* 0x32 - 0x36 */
	'*', SC_P_L_ALT, SC_P_SPACE, SC_P_CAPSLOCK, SC_P_F1,	/* 0x37 - 0x3B */
	SC_P_F2, SC_P_F3, SC_P_F4, SC_P_F5, SC_P_F6,	/* 0x3C - 0x40 */
	SC_P_F7, SC_P_F8, SC_P_F9, SC_P_F10, SC_P_NUMLOCK,	/* 0x41 - 0x45 */
	SC_P_SCLOCK, '7', '8', '9', '-',	/* 0x46 - 0x4A */
	'4', '5', '6', '+', '1',	/* 0x4B - 0x4F */
	'2', '3', '0', '.', 0,		/* 0x50 - 0x54 */
	0, 0, SC_P_F11, SC_P_F12, 0,	/* 0x55 - 0x59 */
	/*0, 0, 0, 0, 0,			[> 0x5A - 0x5E <]*/
	/*0, 0, 0, 0, 0,			[> 0x5F - 0x63 <]*/
	/*0, 0, 0, 0, 0,			[> 0x64 - 0x68 <]*/
	/*0, 0, 0, 0, 0,			[> 0x69 - 0x6D <]*/
	/*0, 0, 0, 0, 0,			[> 0x6E - 0x72 <]*/
	/*0, 0, 0, 0, 0,			[> 0x73 - 0x77 <]*/
	/*0, 0, 0, 0, 0,			[> 0x78 - 0x7C <]*/
	/*0, 0, 0, 0, SC_R_ESC,		[> 0x7D - 0x81 <]*/
	/*'1', '2', '3', '4', '5',	[> 0x82 - 0x86 <]*/
	/*'6', '7', '8', '9', '0',	[> 0x87 - 0x8B <]*/
	/*'-', '=', SC_R_BS, SC_R_TAB, 'q',	[> 0x8C - 0x90 <]*/
	/*'w', 'e', 'r', 't', 'y',	[> 0x91 - 0x95 <]*/
	/*'u', 'i', 'o', 'p', '[',	[> 0x96 - 0x9A <]*/
	/*']', SC_R_ENTER, SC_R_L_CTRL, 'a', 's',	[> 0x9B - 0x9F <]*/
	/*'d', 'f', 'g', 'h', 'j',	[> 0xA0 - 0xA4 <]*/
	/*'k', 'l', ';', , ,	[> 0xA5 - 0xA9 <]*/
	/*, , , , ,	[> 0xAA - 0xAE <]*/
	/*, , , , ,	[> 0xAF - 0xB3 <]*/
	/*, , , , ,	[> 0xB4 - 0xB8 <]*/
	/*, , , , ,	[> 0xB9 - 0xBD <]*/
	/*, , , , ,	[> 0xBE - 0xC2 <]*/
	/*, , , , ,	[> 0xC3 - 0xC7 <]*/
	/*, , , , ,	[> 0xC8 - 0xCC <]*/
	/*, , , , ,	[> 0xCD - 0xD1 <]*/
	/*, , , , ,	[> 0xD2 - 0xD6 <]*/
	/*, , , , ,	[> 0xD7 - 0xDB <]*/
	/*, , , , ,	[> 0xDC - 0xE0 <]*/
};

static int scancode_table_sz = sizeof(scancode_table) / sizeof(uint16_t);

void kbd_intr_handler(uint32_t scan_code)
{
	printf("Scancode %x\n", scan_code);

	if (scan_code >= scancode_table_sz) {
		printf("\tUnknown scancode\n");
	} else {
		printf("\tKey: %c\n", scancode_table[scan_code]);
	}
	return;
}
