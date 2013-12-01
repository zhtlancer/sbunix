#ifndef _PIT_H
#define _PIT_H

#include <defs.h>

#define SCHED_INTERVEL	100;
extern uint64_t jiffies;

void PIT_init(uint16_t freq);

#endif

/* vim: set ts=4 sw=0 tw=0 noet : */
