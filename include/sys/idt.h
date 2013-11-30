#ifndef _SYS_IDT_H
#define _SYS_IDT_H

/*------------------------------
 * IDT
 *----------------------------*/
#define EXP_PGF			14
#define IRQ_PIT         32
#define IRQ_KBD			33

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
/* vim: set ts=4 sw=0 tw=0 noet : */
