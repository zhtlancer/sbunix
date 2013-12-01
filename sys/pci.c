#include <defs.h>
#include <sys/k_stdio.h>
#include <sys/mm.h>
#include <sys/io.h>
#include <sys/pci.h>


/*-------------------------------------------------------------------------
 * Global Variable
 *-------------------------------------------------------------------------
 */

uint32_t pci_dev_nr;
uint32_t pci_dev_type0_nr;
pci_t pci_dev_type0[16];


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
)
{
    uint32_t address;
    uint32_t lbus   = (uint32_t)bus;
    uint32_t lslot  = (uint32_t)slot;
    uint32_t lfunc  = (uint32_t)func;
    uint08_t tmp    = 0;
 
    /* create configuration address as per Figure 1 */
    address =   (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc)
              | ((uint32_t)0x80000000));
 
    /* write out the address */
    outl(0xCF8, address);

    /* read in the data */
    tmp = (uint08_t)(   ( (inl(0xcfc)) >> ((offset&0x3)*8) )&0xF   );

    return tmp;
}


uint16_t
pci_config_read_16
(
    uint16_t    bus ,
    uint16_t    slot,
    uint16_t    func,
    uint16_t    offset
)
{
    uint32_t address;
    uint32_t lbus   = (uint32_t)bus;
    uint32_t lslot  = (uint32_t)slot;
    uint32_t lfunc  = (uint32_t)func;
    uint16_t tmp    = 0;
 
    /* create configuration address as per Figure 1 */
    address =   (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc)
              | ((uint32_t)0x80000000));
 
    /* write out the address */
    outl(0xCF8, address);

    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the fisrt word of the 32 bits register */
    tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);

    return tmp;
}


uint32_t
pci_config_read_32
(
    uint16_t    bus ,
    uint16_t    slot,
    uint16_t    func,
    uint16_t    offset
)
{
    uint32_t address;
    uint32_t lbus   = (uint32_t)bus;
    uint32_t lslot  = (uint32_t)slot;
    uint32_t lfunc  = (uint32_t)func;
    uint32_t tmp    = 0;
 
    /* create configuration address as per Figure 1 */
    address =   (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc)
              | ((uint32_t)0x80000000));
 
    /* write out the address */
    outl(0xCF8, address);

    /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the fisrt word of the 32 bits register */
    tmp = inl(0xCFC);

    return tmp;
}


void
pci_init()
{
    uint32_t i, j;
    //uint16_t vid;
    //uint08_t pif;
    //uint16_t class;
    //uint08_t header_type;

    uint08_t temp_08;
    uint16_t temp_16;
    uint32_t idx = 0;

    pci_dev_nr          = 0;
    pci_dev_type0_nr    = 0;

    k_printf( 0, "\nPCI Enumerating...\n" );
    for ( i=0; i<256; i++ ) { /* Enumerate 256 buses */
        for ( j=0; j<32; j++ ) { /* Enumerate 32 devices */

            temp_16 = pci_config_read_16( i, j, 0, PCI_CFG_REG_VID );
            if ( temp_16 != 0xFFFF ) { /* found device */
                pci_dev_nr++;

                temp_08 = pci_config_read_08( i, j, 0, PCI_CFG_REG_HTYPE );
                if ( temp_08 == 0x0 ) { /* found device with header type 0 */
                    idx = pci_dev_type0_nr;
                    pci_dev_type0_nr++;

                    pci_dev_type0[idx].vid          = pci_config_read_16( i, j, 0, PCI_CFG_REG_VID          );
                    pci_dev_type0[idx].did          = pci_config_read_16( i, j, 0, PCI_CFG_REG_DID          );
                    pci_dev_type0[idx].cmd          = pci_config_read_16( i, j, 0, PCI_CFG_REG_CMD          );
                    pci_dev_type0[idx].sts          = pci_config_read_16( i, j, 0, PCI_CFG_REG_STS          );
                    pci_dev_type0[idx].rid          = pci_config_read_08( i, j, 0, PCI_CFG_REG_RID          );
                    pci_dev_type0[idx].pif          = pci_config_read_08( i, j, 0, PCI_CFG_REG_PIF          );
                    pci_dev_type0[idx].cls_sub      = pci_config_read_08( i, j, 0, PCI_CFG_REG_CLS_SUB      );
                    pci_dev_type0[idx].cls          = pci_config_read_08( i, j, 0, PCI_CFG_REG_CLS          );
                    pci_dev_type0[idx].cache_size   = pci_config_read_08( i, j, 0, PCI_CFG_REG_CACHE_SIZE   );
                    pci_dev_type0[idx].timer        = pci_config_read_08( i, j, 0, PCI_CFG_REG_TIMER        );
                    pci_dev_type0[idx].htype        = pci_config_read_08( i, j, 0, PCI_CFG_REG_HTYPE        );
                    pci_dev_type0[idx].bist         = pci_config_read_08( i, j, 0, PCI_CFG_REG_BIST         );
                    pci_dev_type0[idx].bar0         = pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR0         );
                    pci_dev_type0[idx].bar1         = pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR1         );
                    pci_dev_type0[idx].bar2         = pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR2         );
                    pci_dev_type0[idx].bar3         = pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR3         );
                    pci_dev_type0[idx].bar4         = pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR4         );
                    pci_dev_type0[idx].bar5         = pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR5         );
                    pci_dev_type0[idx].cis          = pci_config_read_32( i, j, 0, PCI_CFG_REG_CIS          );
                    pci_dev_type0[idx].subsys_vid   = pci_config_read_16( i, j, 0, PCI_CFG_REG_SUBSYS_VID   );
                    pci_dev_type0[idx].subsys_id    = pci_config_read_16( i, j, 0, PCI_CFG_REG_SUBSYS_ID    );
                    pci_dev_type0[idx].rom_base     = pci_config_read_32( i, j, 0, PCI_CFG_REG_ROM_BASE     );
                    pci_dev_type0[idx].cap          = pci_config_read_08( i, j, 0, PCI_CFG_REG_CAP          );
                    pci_dev_type0[idx].intr_line    = pci_config_read_08( i, j, 0, PCI_CFG_REG_INTR_LINE    );
                    pci_dev_type0[idx].intr_pin     = pci_config_read_08( i, j, 0, PCI_CFG_REG_INTR_PIN     );
                    pci_dev_type0[idx].min_grant    = pci_config_read_08( i, j, 0, PCI_CFG_REG_MIN_GRANT    );
                    pci_dev_type0[idx].max_latency  = pci_config_read_08( i, j, 0, PCI_CFG_REG_MAX_LATENCY  );

                    pci_dev_type0[idx].rsvd0        = 0x0;
                    pci_dev_type0[idx].rsvd1        = 0x0;
     
                    //vid         = pci_config_read_16( i, j, 0, PCI_CFG_REG_VID );
                    //class       = pci_config_read_16( i, j, 0, PCI_CFG_REG_CLS     );
                    //header_type = pci_config_read_08( i, j, 0, PCI_CFG_REG_HTYPE   );
                    //pif         = pci_config_read_08( i, j, 0, PCI_CFG_REG_PIF   );
                    //k_printf( 0, "Bus %3d, Dev %2d, VID=0x%4X, Class/Sub=0x%4X, HType=0x%2X, ProgIF=0x%2X\n", i, j, vid, class, header_type, pif );
                    //k_printf( 0, "BAR: 0x%8X, 0x%8X, 0x%8X, 0x%8X, 0x%8X, 0x%8X\n", 
                    //          pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR0 ), 
                    //          pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR1 ),
                    //          pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR2 ),
                    //          pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR3 ),
                    //          pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR4 ),
                    //          pci_config_read_32( i, j, 0, PCI_CFG_REG_BAR5 )
                    //        );
                    //k_printf( 0, "\n" );
                    
                } /* if found device with header type 0 */
            } /* if found device */
        } /* enumerate 32 slots */


    } /* enumerate 256 buses */

    //k_printf( 0, "Total %2d PCI devices, %2d of them with header type 0\n", pci_dev_nr, pci_dev_type0_nr );


} /* init_pci() */
