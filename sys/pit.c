#include <defs.h>
#include <sys/io.h>
#include <sys/k_stdio.h>
#include <sys/pic.h>
#include <sys/sched.h>

#define PIT_CH0     0x40  //PIT Channel 0's Data Register Port
#define PIT_CH1     0x41  //PIT Channels 1's Data Register Port, we wont be using this here
#define PIT_CH2     0x42  //PIT Channels 2's Data Register Port
#define PIT_CMD     0x43  //PIT Chip's Command Register Port
#define PIT_FREQ    1193182

uint64_t pit_cnt_m; /* millisecond counter */
uint64_t pit_cnt_s; /*      second counter */

uint64_t jiffies;

void PIT_init(uint16_t freq)
{
	int divider = PIT_FREQ / freq;       
	outb(PIT_CMD, 0x36);             
	outb(PIT_CH0, (uint08_t)(divider & 0xFF) );   
	outb(PIT_CH0, (uint08_t)(divider >>   8) );  
}

/* FIXME: this extern reference should be removed in the future */
extern addr_t vgatext_vbase;

void isr_pit()
{
	register char *temp1, *temp2;
	char num_str[K_NUM_STR_LEN_MAX+1];
	char temp[K_NUM_STR_LEN_MAX+1];
	uint32_t len;
	uint32_t i, j;

	pit_cnt_m++;
	if ( pit_cnt_m == 1000 ) {
		pit_cnt_s++;
		pit_cnt_m = 0;
		len = k_ltocstr( (long)pit_cnt_s, num_str, 'u' );
		for ( i=0; i<K_NUM_STR_LEN_MAX-len; i++ )
			temp[i] = ' '; 
		for ( j=0; j<len; j++ )
			temp[i+j] = num_str[j];
		temp[i+j] = '\0';
		for(
				temp1 = temp,
				temp2 = (char*)(vgatext_vbase+0xF68);
				//temp2 = (char*)((addr_t)&kernofs+0xB8EC8);
				*temp1;
				temp1 += 1, temp2 += 2
		   )
			*temp2 = *temp1;
	}
	PIC_eoi(0);

	jiffies++;
	if (!(jiffies % 100)) {
		wakeup_obj(&jiffies);
		yield();
	}
}

/* vim: set ts=4 sw=0 tw=0 noet : */
