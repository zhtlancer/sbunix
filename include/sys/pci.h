
#ifndef __SYS_PCI_H__ 
#define __SYS_PCI_H__


#include <defs.h>


/*-------------------------------------------------------------------------
 * Definition
 *-------------------------------------------------------------------------
 */

#define PCI_CFG_REG_VID         0x00
#define PCI_CFG_REG_DID         0x02
#define PCI_CFG_REG_CMD         0x04
#define PCI_CFG_REG_STS         0x06
#define PCI_CFG_REG_RID         0x08
#define PCI_CFG_REG_PIF         0x09
#define PCI_CFG_REG_CLS_SUB     0x0A
#define PCI_CFG_REG_CLS         0x0B
#define PCI_CFG_REG_CACHE_SIZE  0x0C
#define PCI_CFG_REG_TIMER       0x0D
#define PCI_CFG_REG_HTYPE       0x0E
#define PCI_CFG_REG_BIST        0x0F
#define PCI_CFG_REG_BAR0        0x10
#define PCI_CFG_REG_BAR1        0x14
#define PCI_CFG_REG_BAR2        0x18
#define PCI_CFG_REG_BAR3        0x1C
#define PCI_CFG_REG_BAR4        0x20
#define PCI_CFG_REG_BAR5        0x24
#define PCI_CFG_REG_CIS         0x28
#define PCI_CFG_REG_SUBSYS_VID  0x2C
#define PCI_CFG_REG_SUBSYS_ID   0x2E
#define PCI_CFG_REG_ROM_BASE    0x30
#define PCI_CFG_REG_CAP         0x34
#define PCI_CFG_REG_INTR_LINE   0x3C
#define PCI_CFG_REG_INTR_PIN    0x3D
#define PCI_CFG_REG_MIN_GRANT   0x3E
#define PCI_CFG_REG_MAX_LATENCY 0x3F


/*-------------------------------------------------------------------------
 * Structure
 *-------------------------------------------------------------------------
 */

struct pci_mem_bar
{
    uint08_t    zero    :  1; // Always 0
    uint08_t    type    :  2; // Type
    uint08_t    prefetch:  1; // Prefetchable
    uint32_t    addr    : 28; // 16-Byte Alighed Base Address
}__attribute__((packed));
typedef struct pci_mem_bar pci_mem_bar_t;


struct pci
{
    uint16_t    vid         : 16; // Vendor ID
    uint16_t    did         : 16; // Device ID

    uint16_t    cmd         : 16; // Command
    uint16_t    sts         : 16; // Status

    uint08_t    rid         :  8; // Revision ID 
    uint08_t    pif         :  8; // Prog IF
    uint08_t    cls_sub     :  8; // Subclass
    uint08_t    cls         :  8; // Class Code

    uint08_t    cache_size  :  8; // Cache Line Size
    uint08_t    timer       :  8; // Latency Timer
    uint08_t    htype       :  8; // Header type
    uint08_t    bist        :  8; // BIST

    uint32_t    bar0        : 32; // BAR0
    uint32_t    bar1        : 32; // BAR1
    uint32_t    bar2        : 32; // BAR2
    uint32_t    bar3        : 32; // BAR3
    uint32_t    bar4        : 32; // BAR4
    uint32_t    bar5        : 32; // BAR5

    uint32_t    cis         : 32; // Cardbus CIS pointer

    uint16_t    subsys_vid  : 16; // Subsystem Vender ID
    uint16_t    subsys_id   : 16; // Sybsystem ID

    uint32_t    rom_base    : 32; // Expansion ROM base address

    uint08_t    cap         :  8; // Capabilities Pointer
    uint32_t    rsvd0       : 24; // Reserved

    uint32_t    rsvd1       : 32; // Reserved

    uint08_t    intr_line   :  8; // Interrupt Line        
    uint08_t    intr_pin    :  8; // Interrupt PIN
    uint08_t    min_grant   :  8; // Min Grant
    uint08_t    max_latency :  8; // Max Latency

}__attribute__((packed));
typedef struct pci pci_t;


/*-------------------------------------------------------------------------
 * Global Variable
 *-------------------------------------------------------------------------
 */

extern uint32_t pci_dev_nr;
extern uint32_t pci_dev_type0_nr;
extern pci_t pci_dev_type0[16];
extern uint32_t ahci_bar;

/*-------------------------------------------------------------------------
 * Function
 *-------------------------------------------------------------------------
 */


uint08_t
pci_config_read_08
(
    uint16_t    bus ,
    uint16_t    slot,
    uint16_t    func,
    uint16_t    offset
);


uint16_t
pci_config_read_16
(
    uint16_t    bus ,
    uint16_t    slot,
    uint16_t    func,
    uint16_t    offset
);

uint32_t
pci_config_read_32
(
    uint16_t    bus ,
    uint16_t    slot,
    uint16_t    func,
    uint16_t    offset
);

void
pci_init();


#endif /* __SYS_PCI_H */
