
#ifndef __SYS_AHCI_H__ 
#define __SYS_AHCI_H__


#include <defs.h>
#include <sys/pci.h>
#include <sys/dev.h>


/*-------------------------------------------------------------------------
 * Definition
 *-------------------------------------------------------------------------
 */


#define SATA_SIG_ATA            0x00000101      // SATA drive
#define SATA_SIG_ATAPI          0xEB140101      // SATAPI drive
#define SATA_SIG_SEMB           0xC33C0101      // Enclosure management bridge
#define SATA_SIG_PM             0x96690101      // Port multiplier
 

#define AHCI_DEV_NULL           0
#define AHCI_DEV_SATA           1
#define AHCI_DEV_SATAPI         2
#define AHCI_DEV_SEMB           2
#define AHCI_DEV_PM             4

#define HBA_PxSSTS_DET_PRESENT  0x3
#define HBA_PxSSTS_IPM_ACTIVE   0x1

#define HBA_PxCMD_ST            0x00000001
#define HBA_PxCMD_FRE           0x00000010
#define HBA_PxCMD_FR            0x00004000
#define HBA_PxCMD_CR            0x00008000

#define HBA_PxIS_TFES           0x40008000

#define ATA_DEV_BUSY            0x80
#define ATA_DEV_DRQ             0x08

#define ATA_CMD_READ_DMA_EX     0x25
#define ATA_CMD_WRITE_DMA_EX    0x35

/*-----------------------------------------------------
 * For this project only
 *-----------------------------------------------------
 */

#define AHCI_DEFAULT_PORT       0
#define AHCI_DEV0_PA            0xFEBF0000
#define AHCI_DISK_SIZE          16*1024*1024
#define AHCI_SECT_SIZE          512
#define AHCI_DISK_SECT_NUM      (AHCI_DISK_SIZE/AHCI_SECT_SIZE)

/*-------------------------------------------------------------------------
 * Structure
 *-------------------------------------------------------------------------
 */


/*-----------------------------------------------------
 * FIS
 *-----------------------------------------------------
 */

typedef enum
{
    FIS_TYPE_REG_H2D        = 0x27, // Register FIS - host to device
    FIS_TYPE_REG_D2H        = 0x34, // Register FIS - device to host
    FIS_TYPE_DMA_ACT        = 0x39, // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP      = 0x41, // DMA setup FIS - bidirectional
    FIS_TYPE_DATA           = 0x46, // Data FIS - bidirectional
    FIS_TYPE_BIST           = 0x58, // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP      = 0x5F, // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS       = 0xA1, // Set device bits FIS - device to host
} FIS_TYPE;


struct hba_fis_reg_h2d
{
    // DWORD 0
    uint08_t    fis_type            ; // FIS_TYPE_REG_H2D
 
    uint08_t    pmport          :  4; // Port multiplier
    uint08_t    rsv0            :  3; // Reserved
    uint08_t    c               :  1; // 1: Command, 0: Control
 
    uint08_t    command             ; // Command register
    uint08_t    featurel            ; // Feature register, 7:0
 
    // DWORD 1
    uint08_t    lba0                ; // LBA low register, 7:0
    uint08_t    lba1                ; // LBA mid register, 15:8
    uint08_t    lba2                ; // LBA high register, 23:16
    uint08_t    device              ; // Device register
 
    // DWORD 2
    uint08_t    lba3                ; // LBA register, 31:24
    uint08_t    lba4                ; // LBA register, 39:32
    uint08_t    lba5                ; // LBA register, 47:40
    uint08_t    featureh            ; // Feature register, 15:8
 
    // DWORD 3
    uint08_t    countl              ; // Count register, 7:0
    uint08_t    counth              ; // Count register, 15:8
    uint08_t    icc                 ;     // Isochronous command completion
    uint08_t    control             ; // Control register
 
    // DWORD 4
    uint08_t    rsv1[4]             ; // Reserved
}__attribute__((packed));
typedef volatile struct hba_fis_reg_h2d hba_fis_reg_h2d_t;


struct hba_fis_reg_d2h
{
    // DWORD 0
    uint08_t    fis_type            ; // FIS_TYPE_REG_D2H
 
    uint08_t    pmport          :  4; // Port multiplier
    uint08_t    rsv0            :  2; // Reserved
    uint08_t    i               :  1;    // Interrupt bit
    uint08_t    rsv1            :  1; // Reserved
 
    uint08_t    status              ; // Status register
    uint08_t    error               ; // Error register
 
    // DWORD 1
    uint08_t    lba0                ; // LBA low register, 7:0
    uint08_t    lba1                ; // LBA mid register, 15:8
    uint08_t    lba2                ; // LBA high register, 23:16
    uint08_t    device              ; // Device register
 
    // DWORD 2
    uint08_t    lba3                ; // LBA register, 31:24
    uint08_t    lba4                ; // LBA register, 39:32
    uint08_t    lba5                ; // LBA register, 47:40
    uint08_t    rsv2                ; // Reserved
 
    // DWORD 3
    uint08_t    countl              ; // Count register, 7:0
    uint08_t    counth              ; // Count register, 15:8
    uint08_t    rsv3[2]             ; // Reserved
 
    // DWORD 4
    uint08_t    rsv4[4]             ; // Reserved
}__attribute__((packed));
typedef volatile struct hba_fis_reg_d2h hba_fis_reg_d2h_t;


struct hba_fis_data
{
    // DWORD 0
    uint08_t    fis_type            ; // FIS_TYPE_DATA
 
    uint08_t    pmport          :  4; // Port multiplier
    uint08_t    rsv0            :  4; // Reserved
 
    uint08_t    rsv1[2]             ; // Reserved
 
    // DWORD 1 ~ N
    uint32_t    data[1]             ; // Payload
}__attribute__((packed));
typedef volatile struct hba_fis_data hba_fis_data_t;


struct hba_fis_dev_bits
{
    // DWORD 0
    uint08_t    fis_type            ; // FIS_TYPE_DATA
    uint08_t    pmport          :  4; // Port multiplier
    uint08_t    rsv0            :  2; // Reserved
    uint08_t    i               :  1; // Interrupt bit
    uint08_t    n               :  1; // Notification bit
    uint08_t    sts_l           :  3; // Status Low 
    uint08_t    rsv1            :  1; // Reserved
    uint08_t    sts_h           :  3; // Status High 
    uint08_t    rsv2            :  1; // Reserved
    uint08_t    error               ; // Error
 
    // DWORD 1
    uint32_t    proto               ; // Protocol Specific
}__attribute__((packed));
typedef volatile struct hba_fis_dev_bits hba_fis_dev_bits_t;


struct hba_fis_pio_setup
{
    // DWORD 0
    uint08_t    fis_type            ; // FIS_TYPE_PIO_SETUP
 
    uint08_t    pmport          :  4; // Port multiplier
    uint08_t    rsv0            :  1; // Reserved
    uint08_t    d               :  1; // Data transfer direction, 1 - device to host
    uint08_t    i               :  1; // Interrupt bit
    uint08_t    rsv1            :  1;
 
    uint08_t    status              ; // Status register
    uint08_t    error               ; // Error register
 
    // DWORD 1
    uint08_t    lba0                ; // LBA low register, 7:0
    uint08_t    lba1                ; // LBA mid register, 15:8
    uint08_t    lba2                ; // LBA high register, 23:16
    uint08_t    device              ; // Device register
 
    // DWORD 2
    uint08_t    lba3                ; // LBA register, 31:24
    uint08_t    lba4                ; // LBA register, 39:32
    uint08_t    lba5                ; // LBA register, 47:40
    uint08_t    rsv2                ; // Reserved
 
    // DWORD 3
    uint08_t    countl              ; // Count register, 7:0
    uint08_t    counth              ; // Count register, 15:8
    uint08_t    rsv3                ; // Reserved
    uint08_t    e_status            ; // New value of status register
 
    // DWORD 4
    uint16_t    tc                  ; // Transfer count
    uint08_t    rsv4[2]             ; // Reserved
}__attribute__((packed));
typedef volatile struct hba_fis_pio_setup hba_fis_pio_setup_t;


struct hba_fis_dma_setup
{
    // DWORD 0
    uint08_t    fis_type            ; // FIS_TYPE_DMA_SETUP
    uint08_t    pmport          :  4; // Port multiplier
    uint08_t    rsv0            :  1; // Reserved
    uint08_t    d               :  1; // Data transfer direction, 1 - device to host
    uint08_t    i               :  1; // Interrupt bit
    uint08_t    a               :  1; // Auto-activate. Specifies if DMA Activate FIS is needed
    uint08_t    rsved[2]            ; // Reserved
 
    //DWORD 1&2
    uint64_t    buffer_id           ; // DMA Buffer Identifier. Used to Identify DMA buffer in host memory. SATA Spec says host specific and not in Spec. Trying AHCI spec might work.
 
    //DWORD 3
    uint32_t    rsvd                ; // More reserved
 
    //DWORD 4
    uint32_t    buffer_ofs          ; // Byte offset into buffer. First 2 bits must be 0
 
    //DWORD 5
    uint32_t    count               ; // Number of bytes to transfer. Bit 0 must be 0
 
    //DWORD 6
    uint32_t    resvd               ; // Reserved
 
}__attribute__((packed));
typedef volatile struct hba_fis_dma_setup hba_fis_dma_setup_t;

 
struct hba_fis_rec
{
    // 0x00
    hba_fis_dma_setup_t     dsfis   ; // DMA Setup FIS
    uint08_t                pad0[ 4];
 
    // 0x20
    hba_fis_pio_setup_t     psfis   ; // PIO Setup FIS
    uint08_t                pad1[12];
 
    // 0x40
    hba_fis_reg_d2h_t       rfis    ; // Register: Device to Host FIS
    uint08_t                pad2[ 4];
 
    // 0x58
    hba_fis_dev_bits_t      sdbfis  ; // Set Device Bit FIS
 
    // 0x60
    uint08_t                ufis[64];
 
    // 0xA0
    uint08_t                rsv[0x100-0xA0];
}__attribute__((packed));
typedef volatile struct hba_fis_rec hba_fis_rec_t;

/*-----------------------------------------------------
 * Command Table
 *-----------------------------------------------------
 */

struct hba_cmd_header
{
    // DW0
    uint08_t    cfl             :  5; // Command FIS length in DWORDS, 2 ~ 16
    uint08_t    a               :  1; // ATAPI
    uint08_t    w               :  1; // Write, 1: H2D, 0: D2H
    uint08_t    p               :  1; // Prefetchable
 
    uint08_t    r               :  1; // Reset
    uint08_t    b               :  1; // BIST
    uint08_t    c               :  1; // Clear busy upon R_OK
    uint08_t    rsv0            :  1; // Reserved
    uint08_t    pmp             :  4; // Port multiplier port

    uint16_t    prdtl               ; // Physical region descriptor table length in entries
 
    // DW1
    uint32_t    prdbc               ; // Physical region descriptor byte count transferred
 
    // DW2, 3
    uint32_t    ctba                ; // Command table descriptor base address
    uint32_t    ctbau               ; // Command table descriptor base address upper 32 bits
 
    // DW4 - 7
    uint32_t    rsv1[4]             ; // Reserved
}__attribute__((packed));
typedef volatile struct hba_cmd_header hba_cmd_header_t;


struct hba_prdt_entry
{
    uint32_t    dba                 ; // Data base address
    uint32_t    dbau                ; // Data base address upper 32 bits
    uint32_t    rsv0                ; // Reserved
 
    // DW3
    uint32_t    dbc             : 22; // Byte count, 4M max
    uint32_t    rsv1            :  9; // Reserved
    uint32_t    i               :  1; // Interrupt on completion
}__attribute__((packed));
typedef volatile struct hba_prdt_entry hba_prdt_entry_t;


struct hba_cmd_tbl
{
    // 0x00
    uint08_t    cfis[64]            ; // Command FIS
 
    // 0x40
    uint08_t    acmd[16]            ; // ATAPI command, 12 or 16 bytes
 
    // 0x50
    uint08_t    rsv[48]             ; // Reserved
 
    // 0x80
    hba_prdt_entry_t prdt_entry[1]  ; // Physical region descriptor table entries, 0 ~ 65535

}__attribute__((packed));
typedef volatile struct hba_cmd_tbl hba_cmd_tbl_t;


/*-----------------------------------------------------
 * ABAR
 *-----------------------------------------------------
 */
 
struct hba_port
{
    uint32_t    clb                 ; // 0x00, command list base address, 1K-byte aligned
    uint32_t    clbu                ; // 0x04, command list base address upper 32 bits
    uint32_t    fb                  ; // 0x08, FIS base address, 256-byte aligned
    uint32_t    fbu                 ; // 0x0C, FIS base address upper 32 bits
    uint32_t    is                  ; // 0x10, interrupt status
    uint32_t    ie                  ; // 0x14, interrupt enable
    uint32_t    cmd                 ; // 0x18, command and status
    uint32_t    rsv0                ; // 0x1C, Reserved
    uint32_t    tfd                 ; // 0x20, task file data
    uint32_t    sig                 ; // 0x24, signature
    uint32_t    ssts                ; // 0x28, SATA status (SCR0:SStatus)
    uint32_t    sctl                ; // 0x2C, SATA control (SCR2:SControl)
    uint32_t    serr                ; // 0x30, SATA error (SCR1:SError)
    uint32_t    sact                ; // 0x34, SATA active (SCR3:SActive)
    uint32_t    ci                  ; // 0x38, command issue
    uint32_t    sntf                ; // 0x3C, SATA notification (SCR4:SNotification)
    uint32_t    fbs                 ; // 0x40, FIS-based switch control
    uint32_t    rsv1[11]            ; // 0x44 ~ 0x6F, Reserved
    uint32_t    vendor[4]           ; // 0x70 ~ 0x7F, vendor specific
}__attribute__((packed));
typedef volatile struct hba_port hba_port_t;


struct hba_mem
{
    // 0x00 - 0x2B, Generic Host Control
    uint32_t    cap                 ; // 0x00, Host capability
    uint32_t    ghc                 ; // 0x04, Global host control
    uint32_t    is                  ; // 0x08, Interrupt status
    uint32_t    pi                  ; // 0x0C, Port implemented
    uint32_t    vs                  ; // 0x10, Version
    uint32_t    ccc_ctl             ; // 0x14, Command completion coalescing control
    uint32_t    ccc_pts             ; // 0x18, Command completion coalescing ports
    uint32_t    em_loc              ; // 0x1C, Enclosure management location
    uint32_t    em_ctl              ; // 0x20, Enclosure management control
    uint32_t    cap2                ; // 0x24, Host capabilities extended
    uint32_t    bohc                ; // 0x28, BIOS/OS handoff control and status
 
    // 0x2C - 0x9F, Reserved
    uint08_t    rsv[0xA0-0x2C]      ;
 
    // 0xA0 - 0xFF, Vendor specific registers
    uint08_t    vendor[0x100-0xA0]  ;
 
    // 0x100 - 0x10FF, Port control registers
    hba_port_t  ports[1]            ;   // 1 ~ 32
}__attribute__((packed));
typedef volatile struct hba_mem hba_mem_t;


/*-------------------------------------------------------------------------
 * Global Variable
 *-------------------------------------------------------------------------
 */

extern hba_mem_t *hba_mem_0;


/*-------------------------------------------------------------------------
 * Function
 *-------------------------------------------------------------------------
 */

int
ahci_check_type
(
    hba_port_t      *port
);


void
ahci_probe_port
( 
    hba_mem_t       *abar
);

 
// Start command engine
void
ahci_start_cmd
(
    hba_port_t      *port
);


// Stop   command engine
void
ahci_stop_cmd
(
    hba_port_t      *port
);


void
ahci_port_rebase(
    hba_port_t      *port
);


int
ahci_read(
    hba_port_t      *port   ,
    uint32_t        startl  ,
    uint32_t        starth  ,
    uint32_t        count   ,
    void            *buf
);


int
ahci_write(
    hba_port_t      *port   ,
    uint32_t        startl  ,
    uint32_t        starth  ,
    uint32_t        count   ,
    void            *buf
);


int
ahci_readsect
(
    struct dev  *dev,
    void        *dst,
    uint64_t    lba
);


int
ahci_writesect
(
    struct dev  *dev,
    void        *src,
    uint64_t    lba
);


int
ahci_find_cmdslot
(
    hba_port_t      *port
);


void
ahci_init ();



#endif /* __SYS_AHCI_H */
