#ifndef _GDT_H
#define _GDT_H

#include <defs.h>

struct tss_t {
	uint32_t reserved;
	uint64_t rsp0;
	uint32_t unused[11];
}__attribute__((packed)) tss;

/* adapted from Chris Stones, shovelos */

#define GDT_CS        (0x00180000000000)  /*** code segment descriptor ***/
#define GDT_DS        (0x00100000000000)  /*** data segment descriptor ***/

#define C             (0x00040000000000)  /*** conforming ***/
#define DPL0          (0x00000000000000)  /*** descriptor privilege level 0 ***/
#define DPL1          (0x00200000000000)  /*** descriptor privilege level 1 ***/
#define DPL2          (0x00400000000000)  /*** descriptor privilege level 2 ***/
#define DPL3          (0x00600000000000)  /*** descriptor privilege level 3 ***/
#define P             (0x00800000000000)  /*** present ***/
#define L             (0x20000000000000)  /*** long mode ***/
#define D             (0x40000000000000)  /*** default op size ***/
#define W             (0x00020000000000)  /*** writable data segment ***/

extern uint64_t gdt[];
void reload_gdt();
void setup_tss();

/*------------------------------
 * IDT
 *----------------------------*/
#define IRQ_IPT         32

struct int_gate {
    uint16_t    offsetLo;
    uint16_t    segSel;
    uint16_t    attr;
    uint16_t    offsetMi; 
    uint32_t    offsetHi; 
    uint32_t    resZero;
}__attribute__((packed));
typedef struct int_gate int_gate_t;

#define TYPE_IG64       (0x0E00) /***` Type: 64 bits interrupt gate ***/
#define DESC_P          (0x8000) /***` Type: 64 bits interrupt gate ***/
#define DESC_DPL0       (0x0000)  /*** descriptor privilege level 0 ***/
#define DESC_DPL1       (0x2000)  /*** descriptor privilege level 1 ***/
#define DESC_DPL2       (0x4000)  /*** descriptor privilege level 2 ***/
#define DESC_DPL3       (0x6000)  /*** descriptor privilege level 3 ***/
extern uint64_t idt[];
void reload_idt();

#endif
