
#include <defs.h>
#include <sys/string.h>
#include <sys/mm.h>
#include <sys/dev.h>
#include <sys/pci.h>
#include <sys/ahci.h>
#include <sys/k_stdio.h>


#define ahci_error(fmt, ...)	\
	k_printf(1, "<AHCI> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_AHCI
#define ahci_db(fmt, ...)	\
	k_printf(1, "<AHCI DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define ahci_db(fmt, ...)
#endif

#define TEST_AHCI 1


/*-------------------------------------------------------------------------
 * Global Variable
 *-------------------------------------------------------------------------
 */

hba_mem_t *hba_mem_0;
uint32_t ahci_bar;

/*-------------------------------------------------------------------------
 * Function
 *-------------------------------------------------------------------------
 */

int
ahci_check_type
(
    hba_port_t      *port
)
{
    uint32_t    ssts    = port->ssts;
 
    uint16_t    ipm     = (ssts >> 8) & 0x0F;
    uint16_t    det     = ssts & 0x0F;
 
    if (det != HBA_PxSSTS_DET_PRESENT)    // Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PxSSTS_IPM_ACTIVE)
        return AHCI_DEV_NULL;
 
    switch (port->sig) {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_SATA;
    }
} /* ahci_check_type() */


void
ahci_probe_port
( 
    hba_mem_t       *abar 
)
{
    // Search disk in impelemented ports
    uint32_t    pi  = abar->pi;
    int         i   = 0;
    while (i<32)
    {
        if (pi & 1)
        {
            int dt = ahci_check_type(&abar->ports[i]);

            if      (dt == AHCI_DEV_SATA)
            {
                ahci_db( "SATA drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                ahci_db( "SATAPI drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SEMB)
            {
                ahci_db( "SEMB drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_PM)
            {
                ahci_db( "PM drive found at port %d\n", i);
            }
            else
            {
                //ahci_db( "No drive found at port %d\n", i);
            }
        }
 
        pi >>= 1;
        i ++;
    }
} /* ahci_probe_port() */


// Start command engine
void
ahci_start_cmd
(
    hba_port_t      *port
)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);
 
    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST; 
} /* ahci_start_cmd */

 
// Stop command engine
void
ahci_stop_cmd
(
    hba_port_t      *port
)
{
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;

    // Clear FIS Receive Enable (FRE)
    port->cmd &= ~HBA_PxCMD_FRE; 


    // Wait until FR (bit14), CR (bit15) are cleared
    while(1)
    {
        if (port->cmd & HBA_PxCMD_FR)
            continue;
        if (port->cmd & HBA_PxCMD_CR)
            continue;
        break;
    }
 
} /* ahci_stop_cmd() */

 
void
ahci_port_rebase(
    hba_port_t      *port
)
{
    int     i;
    void    *cl_fis_va  = __get_free_page(  PG_SUP );
    void    *cmdtbl_va  = __get_free_pages( PG_SUP, 1 );
    addr_t   cl_fis_pa  = get_pa_from_va( cl_fis_va );
    addr_t   cmdtbl_pa  = get_pa_from_va( cmdtbl_va );

    ahci_stop_cmd(port); // Stop command engine

    // Command list entry size                 = 32
    // Command list entry maxim count per port = 32
    // Command list maxim size per port        = 32*32 = 1K
    port->clb   = (uint32_t)cl_fis_pa;
    port->clbu  = 0;
    memset(cl_fis_va, 0, 1024);
 
    // FIS offset: Command List maxim size per Port = 1K
    // FIS entry size per port                      = 256 bytes
    port->fb    = (uint32_t)cl_fis_pa + 1024;
    port->fbu   = 0;
    memset( get_va_from_pa((addr_t)(port->fb)), 0, 256);
 
    hba_cmd_header_t *cmdheader = (hba_cmd_header_t *)cl_fis_va;
    for ( i=0; i<32; i++)
    {
        cmdheader[i].prdtl = 8; // 8 prdt entries per command table
        // Command table size (with 8 prdt entries) = 256 byte
        cmdheader[i].ctba = (uint32_t)cmdtbl_pa + (i<<8);
        cmdheader[i].ctbau = 0;
        memset(get_va_from_pa((addr_t)(cmdheader[i].ctba)), 0, 256);
    }
 
    ahci_start_cmd(port);    // Start command engine
} /* hba_port_rebase() */


int
ahci_read(
    hba_port_t      *port   ,
    uint32_t        startl  ,
    uint32_t        starth  ,
    uint32_t        count   ,
    void            *buf
)
{
    int i;
    addr_t  buf_pa = get_pa_from_va( buf );
    port->is = (uint32_t)(-1);      // Clear pending interrupt bits
    int spin = 0;                   // Spin lock timeout counter
    int slot = ahci_find_cmdslot(port);
    if (slot == -1)
        return FALSE;
 
    //HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)port->clb;
    hba_cmd_header_t *cmdheader = (hba_cmd_header_t *)( get_va_from_pa((addr_t)(port->clb)) );
    cmdheader += slot;
    cmdheader->cfl = sizeof(hba_fis_reg_h2d_t)/sizeof(uint32_t); // Command FIS size
    cmdheader->w = 0;       // Read from device
    cmdheader->prdtl = (uint16_t)((count-1)>>4) + 1;    // PRDT entries count
 
    //HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t *)( get_va_from_pa((addr_t)(cmdheader->ctba)) );
    memset( (void *)cmdtbl, 0, sizeof(hba_cmd_tbl_t) + (cmdheader->prdtl-1)*sizeof(hba_prdt_entry_t));
 

    // 8K bytes (16 sectors) per PRDT
    for (i=0; i<cmdheader->prdtl-1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint32_t)buf_pa;
        cmdtbl->prdt_entry[i].dbc = (8*1024)-1; // 8K bytes
        cmdtbl->prdt_entry[i].i = 0; //FIXME: original 1, set to 0 because of QEMU bug
        buf_pa += 8*1024;  // 4K words
        count -= 16;    // 16 sectors
    }
    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint32_t)buf_pa;
    cmdtbl->prdt_entry[i].dbc = (count<<9)-1;   // 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 0; //FIXME: original 1, set to 0 because of QEMU bug
 
    // Setup command
    //FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
    hba_fis_reg_h2d_t *cmdfis = (hba_fis_reg_h2d_t *)(&cmdtbl->cfis);
 
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = ATA_CMD_READ_DMA_EX;
 
    cmdfis->lba0 = (uint16_t)startl;
    cmdfis->lba1 = (uint16_t)(startl>>8);
    cmdfis->lba2 = (uint16_t)(startl>>16);
    cmdfis->device = 1<<6;  // LBA mode
 
    cmdfis->lba3 = (uint16_t)(startl>>24);
    cmdfis->lba4 = (uint16_t)starth;
    cmdfis->lba5 = (uint16_t)(starth>>8);
 
    cmdfis->countl = (uint08_t)( count    &0xFF);
    cmdfis->counth = (uint08_t)((count>>8)&0xFF);
 
    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        ahci_error( "Port is hung\n");
        return FALSE;
    }
 
    port->ci = 1<<slot; // Issue command
 
    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit 
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1<<slot)) == 0) 
            break;
        if (port->is & HBA_PxIS_TFES)   // Task file error
        {
            ahci_error( "Read disk error\n");
            return FALSE;
        }
    }
 
    // Check again
    if (port->is & HBA_PxIS_TFES)
    {
        ahci_error( "Read disk error\n");
        return FALSE;
    }

    return TRUE;
} /* ahci_read() */


int
ahci_write(
    hba_port_t      *port   ,
    uint32_t        startl  ,
    uint32_t        starth  ,
    uint32_t        count   ,
    void            *buf
)
{
    int i;
    addr_t  buf_pa = get_pa_from_va( buf );
    port->is = (uint32_t)(-1);      // Clear pending interrupt bits
    int spin = 0;                   // Spin lock timeout counter
    int slot = ahci_find_cmdslot(port);
    if (slot == -1)
        return FALSE;
 
    //HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)port->clb;
    hba_cmd_header_t *cmdheader = (hba_cmd_header_t *)( get_va_from_pa((addr_t)(port->clb)) );
    cmdheader += slot;
    cmdheader->cfl = sizeof(hba_fis_reg_h2d_t)/sizeof(uint32_t); // Command FIS size
    cmdheader->w = 0;       // Read from device
    cmdheader->prdtl = (uint16_t)((count-1)>>4) + 1;    // PRDT entries count
 
    //HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t *)( get_va_from_pa((addr_t)(cmdheader->ctba)) );
    memset( (void *)cmdtbl, 0, sizeof(hba_cmd_tbl_t) + (cmdheader->prdtl-1)*sizeof(hba_prdt_entry_t));
 

    // 8K bytes (16 sectors) per PRDT
    for (i=0; i<cmdheader->prdtl-1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint32_t)buf_pa;
        cmdtbl->prdt_entry[i].dbc = (8*1024)-1; // 8K bytes
        cmdtbl->prdt_entry[i].i = 0; //FIXME: original 1, set to 0 because of QEMU bug
        buf_pa += 8*1024;  // 4K words
        count -= 16;    // 16 sectors
    }
    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint32_t)buf_pa;
    cmdtbl->prdt_entry[i].dbc = (count<<9)-1;   // 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 0; //FIXME: original 1, set to 0 because of QEMU bug
 
    // Setup command
    //FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
    hba_fis_reg_h2d_t *cmdfis = (hba_fis_reg_h2d_t *)(&cmdtbl->cfis);
 
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = ATA_CMD_WRITE_DMA_EX;
 
    cmdfis->lba0 = (uint16_t)startl;
    cmdfis->lba1 = (uint16_t)(startl>>8);
    cmdfis->lba2 = (uint16_t)(startl>>16);
    cmdfis->device = 1<<6;  // LBA mode
 
    cmdfis->lba3 = (uint16_t)(startl>>24);
    cmdfis->lba4 = (uint16_t)starth;
    cmdfis->lba5 = (uint16_t)(starth>>8);
 
    cmdfis->countl = (uint08_t)( count    &0xFF);
    cmdfis->counth = (uint08_t)((count>>8)&0xFF);
 
    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        ahci_error( "Port is hung\n");
        return FALSE;
    }
 
    port->ci = 1<<slot; // Issue command
 
    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit 
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1<<slot)) == 0) 
            break;
        if (port->is & HBA_PxIS_TFES)   // Task file error
        {
            ahci_error( "Write disk error\n");
            return FALSE;
        }
    }
 
    // Check again
    if (port->is & HBA_PxIS_TFES)
    {
        ahci_error( "Write disk error\n");
        return FALSE;
    }

    return TRUE;
} /* ahci_write() */


 
// Find a free command list slot
int
ahci_find_cmdslot
(
    hba_port_t      *port
)
{
    // If not set in SACT and CI, the slot is free
    int i;
    uint32_t slots = (port->sact | port->ci);
    //for ( i=0; i<cmdslots; i++)
    for ( i=0; i<32; i++)
    {
        if ((slots&1) == 0)
            return i;
        slots >>= 1;
    }
    ahci_error(  "Cannot find free command list entry\n" );
    return -1;
}


int
ahci_readsect
(
    struct dev  *dev,
    void        *dst,
    uint64_t    lba
)
{
    return ahci_read(
                        &(hba_mem_0->ports[AHCI_DEFAULT_PORT]),
                        (uint32_t)( lba     &0xFFFFFFFF), 
                        (uint32_t)((lba>>32)&0xFFFFFFFF),
                        1,
                        dst
                    );
} /* readsect() */


int
ahci_writesect
(
    struct dev  *dev,
    void        *src,
    uint64_t    lba
)
{
    return ahci_write(
                        &(hba_mem_0->ports[AHCI_DEFAULT_PORT]),
                        (uint32_t)( lba     &0xFFFFFFFF), 
                        (uint32_t)((lba>>32)&0xFFFFFFFF),
                        1,
                        src
                     );
} /* writesect() */


void
ahci_init ()
{

    //uint32_t volatile *hba_DW = (uint32_t volatile *)(map_pa_kernel(0xFEBF0000, 0, 0, 0, PGT_SUP | PGT_EXE | PGT_RW ) );
    hba_mem_0 = (hba_mem_t *)( map_pa_kernel(ahci_bar, 0, 0, 0, PGT_SUP | PGT_EXE | PGT_RW ) );  
 


    //uint32_t volatile *hba_DW = (uint32_t volatile *)(0xFFFFFFFFFEBF0000UL);
    //ahci_db( "%8P, HBA.cap=0x%8X\n", hba_DW, 0x0 );
    //ahci_db(  "%8P, HBA.cap=0x%8X\n", hba_mem_0, hba_mem_0->cap );
    //ahci_db( "hba_mem_0->ports[0]->clb(u) = 0x%08X%08X\n", hba_mem_0->ports[0].clbu, hba_mem_0->ports[0].clb );
    //ahci_db( "hba_mem_0->ports[0]->fb (u) = 0x%08X%08X\n", hba_mem_0->ports[0].fbu , hba_mem_0->ports[0].fb  );

    ahci_probe_port( hba_mem_0 ); 
    
    ahci_port_rebase( &(hba_mem_0->ports[AHCI_DEFAULT_PORT]) );

    //ahci_db( "hba_mem_0->ports[0]->clb(u) = 0x%08X%08X\n", hba_mem_0->ports[0].clbu, hba_mem_0->ports[0].clb );
    //ahci_db( "hba_mem_0->ports[0]->fb (u) = 0x%08X%08X\n", hba_mem_0->ports[0].fbu , hba_mem_0->ports[0].fb  );

#if TEST_AHCI

    int i, j;
    // 1 sector = 512 byte, 8 sector = 4k = 1page
    void *ahci_buf = __get_free_pages( PG_SUP, 4 );
    if ( ahci_read( &(hba_mem_0->ports[0]), 0, 0, 128, ahci_buf ) )
        k_printf( 0, "AHCI read OK!\n" );
    else
        k_printf( 0, "AHCI read Error!\n" );

    uint08_t *ahci_buf_byte = (uint08_t *)ahci_buf;
    for ( i=0; i<1; i++ )
        for ( j=0; j<16; j++ )  {
            if ( (j%16)==0 ) 
                k_printf( 0, "\n" );
            k_printf( 0, "%02X ", *(ahci_buf_byte++) );
        }   
    k_printf( 0, "\n" );

    ahci_buf_byte = (uint08_t *)ahci_buf;
    *(ahci_buf_byte++) = '0';
    *(ahci_buf_byte++) = '1';
    *(ahci_buf_byte++) = '2';
    *(ahci_buf_byte++) = '3';
    *(ahci_buf_byte++) = '4';
    *(ahci_buf_byte++) = '5';
    *(ahci_buf_byte++) = '6';
    *(ahci_buf_byte++) = '7';
    *(ahci_buf_byte++) = '8';
    *(ahci_buf_byte++) = '9';
    *(ahci_buf_byte++) = 'A';
    *(ahci_buf_byte++) = 'B';
    *(ahci_buf_byte++) = 'C';
    *(ahci_buf_byte++) = 'D';
    *(ahci_buf_byte++) = 'E';
    *(ahci_buf_byte++) = 'F';

    if ( ahci_write( &(hba_mem_0->ports[0]), 0, 0, 128, ahci_buf ) )
        k_printf( 0, "AHCI write OK!\n" );
    else
        k_printf( 0, "AHCI write Error!\n" );

    ahci_buf_byte = (uint08_t *)ahci_buf;
    for ( i=0; i<1; i++ )
        for ( j=0; j<16; j++ )  {
            *(ahci_buf_byte++)=0;
        }   

    //ahci_buf_byte = (uint08_t *)ahci_buf;
    //for ( i=0; i<1; i++ )
    //    for ( j=0; j<16; j++ )  {
    //        if ( (j%16)==0 ) 
    //            k_printf( 0, "\n" );
    //        k_printf( 0, "%02X ", *(ahci_buf_byte++) );
    //    }   
    //k_printf( 0, "\n" );

    //if ( ahci_read( &(hba_mem_0->ports[0]), 0, 0, 128, ahci_buf ) )
    //if ( ahci_readsect( &devs[DEV_DISK], ahci_buf, 0 ) )
    if ( devs[DEV_DISK].readsect( &devs[DEV_DISK], ahci_buf, 0 ) )
        k_printf( 0, "AHCI read OK!\n" );
    else
        k_printf( 0, "AHCI read Error!\n" );

    ahci_buf_byte = (uint08_t *)ahci_buf;
    for ( i=0; i<1; i++ )
        for ( j=0; j<16; j++ )  {
            if ( (j%16)==0 ) 
                k_printf( 0, "\n" );
            k_printf( 0, "%02X ", *(ahci_buf_byte++) );
        }   
    k_printf( 0, "\n" );
#endif    
}
