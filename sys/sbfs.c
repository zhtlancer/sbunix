#include <defs.h>
#include <sys/mm.h>
#include <sys/string.h>
#include <sys/dev.h>
#include <sys/pci.h>
#include <sys/ahci.h>
#include <sys/fs.h>
#include <sys/sbfs.h>
#include <sys/k_stdio.h>

#define sbfs_error(fmt, ...)	\
	k_printf(1, "<FS> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)

#if DEBUG_SBFS
#define sbfs_db(fmt, ...)	\
	k_printf(1, "<FS DEBUG> [%s (%s:%d)] " fmt, __func__, __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define sbfs_db(fmt, ...)
#endif

#define TEST_SBFS 0

volatile uint32_t *sbfs_buf;
sbfs_super_block_t sbfs_superblk;
static struct fs_operations sbfs_ops;

/*-------------------------------------------------------------------------
 * Function
 *-------------------------------------------------------------------------
 */

/*-----------------------------------------------------
 * Function: Find sector
 *-----------------------------------------------------
 */

int
sbfs_find_free_sect()
{
	struct dev 	*sbfs_dev = &devs[DEV_DISK];
	int 		free_sect = -1;
	int 		i, j, k;

	for ( i=SBFS_SECT_BM; i<SBFS_SECT_INODE; i++ ) {
		memset( (void *)sbfs_buf, 0, AHCI_SECT_SIZE );
		sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, i );
		for ( j=0; j<(AHCI_SECT_SIZE/4); j++ ) {

			if ( sbfs_buf[j] != 0x0 ) { /* find first 1 inside BM */
                for ( k=0; k<32; k++ ) {
                    if ( sbfs_buf[j] & ((uint32_t)1<<k) ) {
                        //sbfs_buf[j] = sbfs_buf[j] & (~((uint64_t)1<<k));
						//sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, i );
						k_printf( 0, "find free sect: %d, %d, %d\n", i, j, k );
						free_sect = ((i-SBFS_SECT_BM)*AHCI_SECT_SIZE*8)+(j*32)+k;
                        break;
                    }
                    else
                        continue;
                } /* for loop to find first 1 inside bmap[i] */
			}
			
			if ( free_sect != -1 )
				break;
		}
		if ( free_sect != -1 )
			break;
	}

	return free_sect;

} /* sbfs_find_free_sect() */

void
sbfs_set_bm_occupied
(
	uint32_t		sect
)
{
	struct dev 	*sbfs_dev = &devs[DEV_DISK];


	uint32_t	bm_sect	= (sect/(AHCI_SECT_SIZE*8))+SBFS_SECT_BM;
	uint32_t	bm		= (sect%(AHCI_SECT_SIZE*8))/32;
	uint32_t	bit		= sect&0xFFFF;

	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, bm_sect );
    sbfs_buf[bm] = sbfs_buf[bm] & (~((uint64_t)1<<bit));
	sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, bm_sect );
	
} /* sbfs_set_bm_occupied() */ 


void
sbfs_set_bm_free
(
	uint32_t		sect
)
{
	struct dev 	*sbfs_dev = &devs[DEV_DISK];


	uint32_t	bm_sect	= (sect/(AHCI_SECT_SIZE*8))+SBFS_SECT_BM;
	uint32_t	bm		= (sect%(AHCI_SECT_SIZE*8))/32;
	uint32_t	bit		= sect&0xFFFF;

	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, bm_sect );
    sbfs_buf[bm] = sbfs_buf[bm] | (((uint64_t)1<<bit));
	sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, bm_sect );
	
} /* sbfs_set_bm_free() */ 


/*-----------------------------------------------------
 * Function: find inode
 *-----------------------------------------------------
 */

int
sbfs_find_free_inode()
{
	uint32_t i, j, cnt;
	struct dev *sbfs_dev = &devs[DEV_DISK];
	struct p_inode *pinode = (struct p_inode *)(sbfs_buf);

	cnt=0;
	for ( i=SBFS_SECT_INODE; i<SBFS_SECT_DATA; i++ ) {
		sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, i );
		for ( j=0; j<(AHCI_SECT_SIZE/SBFS_INODE_SIZE); j++, cnt++ ) {
			if ( pinode[j].type == SBFS_IT_FREE )
				return cnt;
		}
	}

	return -1;

} /* sbfs_find_free_inode() */


int
sbfs_set_inode( uint32_t inode_nr, struct p_inode *pinode )
{
	struct dev *sbfs_dev = &devs[DEV_DISK];
	struct p_inode *pinode_list = (struct p_inode *)(sbfs_buf);

	uint32_t	sect	= inode_nr/(AHCI_SECT_SIZE/SBFS_INODE_SIZE)+SBFS_SECT_INODE;
	uint32_t	entry   = inode_nr%(AHCI_SECT_SIZE/SBFS_INODE_SIZE);

	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );
	pinode_list[entry] = *pinode;
	sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, sect );

	return 0;
} /* sbfs_set_inode() */


/*-----------------------------------------------------
 * Function: fs operations
 *-----------------------------------------------------
 */


static struct inode *sbfs_path_lookup(struct inode *parent, const char *path)
{
	int i, j, k, cnt;

	struct dev 		*sbfs_dev 	= &devs[DEV_DISK];
	struct p_inode 	pinode 		= parent->p_inode;
	uint32_t		pinode_nr	= 0;
	uint32_t 		sect		= pinode.addrs[0]; // only support 32 dir/file in a dir
	char 			path_tmp[SBFS_NAME_MAX+1];
	struct inode 	*inode;

	sbfs_dentry_t 	*dentry_list	= (sbfs_dentry_t *)sbfs_buf;
	struct p_inode 	*pinode_list	= (struct p_inode *)sbfs_buf;

	if 	( pinode.type != SBFS_IT_DIR ) {
		sbfs_error( "lookup path not from a  directory.\n" );
		return parent;
	}

	if ( path == '\0' ) {
		return parent;
	}

	if ( pinode.size > 32 ) {
		sbfs_error( "SBFS only support up to 30 files/dirs in a directory.\n" );
		return parent;
	}
		
	j=0;
	if ( path[j] == '/' )
		j++;
	while(1) {

		for ( i=0; i<SBFS_NAME_MAX+1; i++ )
			path_tmp[i] = 0;

		for ( i=0; path[j] != '\0' && path[j] != '/'; i++, j++) {
			path_tmp[i] = path[j];
		}
		path_tmp[i] = '\0';
		
		sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );

		cnt=-1;
		for ( k=0; k<pinode.size; k++ ) {
			if ( !strncmp( dentry_list[k].name, path_tmp, i ) ) {
				cnt	=	k;
				break;
			}
		}

		if ( cnt == -1 ) {
			sbfs_db( "SBFS path_not_found.\n" );
			return NULL;
		}
		else			{
			pinode_nr 	= dentry_list[cnt].pinode_nr;
			sect		= (pinode_nr/(AHCI_SECT_SIZE/SBFS_INODE_SIZE))+SBFS_SECT_INODE;
			sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );
			pinode		= pinode_list[ pinode_nr%(AHCI_SECT_SIZE/SBFS_INODE_SIZE) ];
			sect		= pinode.addrs[0];
		}
			
		if ( path[j] == '/' )
			j++;

		if ( path[j] == '\0')
			break;

	}

	inode = get_inode(NULL);
	inode->dev_num		= DEV_DISK;
	inode->fs_ops 		= &sbfs_ops;
	inode->priv_data 	= NULL;
	inode->p_inode 		= pinode;

	return inode;
} /* sbfs_path_lookup */


static size_t sbfs_read(struct inode *inode, void *dst, off_t off, size_t n)
{
	int i, idx;

	uint32_t		off_tmp	 	= (uint32_t)off;
	uint32_t     	off_sect 	= 0;

	uint32_t		size_tmp 	= (uint32_t)n;
	uint32_t     	size_sect	= 0;

	struct dev 		*sbfs_dev 	= &devs[DEV_DISK];
	struct p_inode 	pinode 		= inode->p_inode;
	uint32_t 		sect		= pinode.addrs[0];
	uint32_t 		sect_nr		= 0;

	uint08_t 		*buf_8		= (uint08_t *)sbfs_buf;
	uint08_t 		*dst_8		= (uint08_t *)dst;

	if ( pinode.type != SBFS_IT_FILE ) {
		sbfs_error( "SBFS can not read.\n" );
		return 0;
	}

	if ( pinode.size <= (uint32_t)off )
		return 0;	

	idx = 0;
	do {


		sect_nr		=	off_tmp/AHCI_SECT_SIZE;

		if ( sect_nr < NDIRECT ) {
			sect	= pinode.addrs[sect_nr];
			sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );
		}
		else {
			sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, pinode.addrs[NDIRECT] );
			sect	= sbfs_buf[sect_nr-NDIRECT];
			sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );
		}

		off_sect 	= ((uint32_t)off_tmp)%AHCI_SECT_SIZE;
		size_sect	= AHCI_SECT_SIZE-off_sect;
		size_sect	= size_sect > size_tmp ? size_tmp : size_sect;

		for ( i=0; i<size_sect; i++, idx++ ) {
			dst_8[idx]	= buf_8[off_sect+i];
		}

		off_tmp 	+= size_sect;
		size_tmp	-= size_sect;

	} while ( (pinode.size > off_tmp) && (size_tmp > 0) );


	return (size_t)idx;
} /* sbfs_read() */


static size_t sbfs_write(struct inode *inode, void *src, off_t off, size_t n)
{
	int i, idx;

	uint32_t		off_tmp	 	= (uint32_t)off;
	uint32_t     	off_sect 	= 0;

	uint32_t		size_tmp 	= (uint32_t)n;
	uint32_t     	size_sect	= 0;

	struct dev 		*sbfs_dev 	= &devs[DEV_DISK];
	struct p_inode 	pinode 		= inode->p_inode;
	uint32_t 		sect		= pinode.addrs[0];
	uint32_t 		sect_nr		= 0;

	uint08_t 		*buf_8		= (uint08_t *)sbfs_buf;
	uint08_t 		*src_8		= (uint08_t *)src;


	if ( pinode.type != SBFS_IT_FILE ) {
		sbfs_error( "SBFS can not write.\n" );
		return 0;
	}

	if ( pinode.size <= (uint32_t)off )
		return 0;	

	idx = 0;
	do {

		sect_nr		=	off_tmp/AHCI_SECT_SIZE;

		if ( sect_nr < NDIRECT ) {
			sect	= pinode.addrs[sect_nr];
			sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );
		}
		else {
			sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, pinode.addrs[NDIRECT] );
			sect	= sbfs_buf[sect_nr-NDIRECT];
			sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );
		}

		off_sect 	= ((uint32_t)off_tmp)%AHCI_SECT_SIZE;
		size_sect	= AHCI_SECT_SIZE-off_sect;
		size_sect	= size_sect > size_tmp ? size_tmp : size_sect;

		for ( i=0; i<size_sect; i++, idx++ ) {
			buf_8[off_sect+i]	= src_8[idx];
		}
		sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, sect );

		off_tmp 	+= size_sect;
		size_tmp	-= size_sect;

	} while ( (pinode.size > off_tmp) && (size_tmp > 0) );


	return (size_t)idx;
} /* sbfs_write() */ 


static int sbfs_getdirents(struct inode *inode, void *buf, int offset, int count)
{
	int i, idx;

	struct dev 		*sbfs_dev 	= &devs[DEV_DISK];
	struct p_inode 	pinode 		= inode->p_inode;

	uint32_t 		sect		= pinode.addrs[0]; // only support 32 dir/file in a dir

	sbfs_dentry_t	*buf_dnt	= (sbfs_dentry_t *)sbfs_buf;
	sbfs_dentry_t	*dst_dnt	= (sbfs_dentry_t *)buf;

	if ( pinode.type != SBFS_IT_DIR ) {
		sbfs_error( "Not a dir.\n" );
		return 0;
	}

	if ( count > 32 ) {
		sbfs_error( "SBFS only support up to 30 files/dirs in a directory.\n" );
		return 0;
	}
		
	if ( pinode.size <= (uint32_t)offset )
		return 0;	

	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );

	for ( i=0, idx=(uint32_t)offset; i<count; i++, idx++ )
		dst_dnt[i] 	= buf_dnt[idx];

	return i;
} /* sbfs_getdirents() */ 


static void sbfs_close(struct inode *inode)
{
	/* TODO */
}


static struct fs_operations sbfs_ops = {
	.path_lookup	= sbfs_path_lookup,
	.read			= sbfs_read,
	.write			= sbfs_write,
	.getdirents		= sbfs_getdirents,
	.close			= sbfs_close 
};

/*-----------------------------------------------------
 * Function: Init
 *-----------------------------------------------------
 */

struct inode *
sbfs_create_dir
(
	struct inode	*parent,
	const char		*name 
)
{
	int	i, j;
	struct dev 		*sbfs_dev 	= &devs[DEV_DISK];
	struct p_inode 	pinode 		= parent->p_inode;

	uint32_t 		sect		= pinode.addrs[0];

	struct p_inode 	pinode_tmp;
	uint32_t 		pinode_tmp_nr	= 0;

	uint32_t		sect_tmp_nr		= 0;

	sbfs_dentry_t	*buf_dnt	= (sbfs_dentry_t *)sbfs_buf;

	if ( pinode.type != SBFS_IT_DIR ) {
		sbfs_error( "SBFS can not create DIR.\n" );
		return parent;
	}

	if ( pinode.size+1 > (uint32_t)32 ) {
		sbfs_error( "SBFS can not contain more than 30 files/dirs in a dir.\n" );
		return parent;	
	}

	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );

	for ( i=0; i<pinode.size; i++ ) {
		if ( !strcmp( buf_dnt[i].name, name) ) {
			sbfs_error( "Already has dir/file with the same name.\n" );
			return parent;
		}
	}
	
	pinode_tmp_nr	= sbfs_find_free_inode();
	sect_tmp_nr 	= sbfs_find_free_sect();
	sbfs_set_bm_occupied( sect_tmp_nr );	

	/* modify parent inode */
	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );
	buf_dnt[pinode.size].pinode_nr	= pinode_tmp_nr;
	for ( i=0; i<SBFS_NAME_MAX; i++ ) {
		buf_dnt[pinode.size].name[i] = name[i]; 
		if ( name[i] == '\0' )
			break;
	}
	sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, sect );
	pinode.size += 1;
	(parent->p_inode).size += 1;
	sbfs_set_inode( pinode.minor, &pinode );


	/* create child p_inode */
	pinode_tmp.type		= SBFS_IT_DIR;
	pinode_tmp.major	= 0;
	pinode_tmp.minor	= pinode_tmp_nr;
	pinode_tmp.nlink	= 0;
	pinode_tmp.size		= 2; /* . and .. */
	pinode_tmp.addrs[0]	= sect_tmp_nr;
	for ( i=1; i<NDIRECT+1; i++ )
		pinode_tmp.addrs[i]	= 0;
	sbfs_set_inode( pinode_tmp_nr, &pinode_tmp );

	/* create child dentries */
	for ( i=0; i<(AHCI_SECT_SIZE/SBFS_DENTRY_SIZE); i++ ) {
		buf_dnt[i].pinode_nr = 0;
		for ( j=1; j<SBFS_NAME_MAX+1; j++ )
			buf_dnt[i].name[j] = '\0';
	}
	buf_dnt[0].pinode_nr = pinode_tmp_nr;
	buf_dnt[0].name[0]   = '.';
	buf_dnt[0].name[1]   = '\0';
	buf_dnt[1].pinode_nr = pinode.minor;
	buf_dnt[1].name[0]   = '.';
	buf_dnt[1].name[1]   = '.';
	buf_dnt[1].name[2]   = '\0';
	sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, sect_tmp_nr );

	/* create child inode */
	struct inode *inode_tmp = get_inode(NULL);
	inode_tmp->dev_num		= DEV_DISK;
	inode_tmp->fs_ops 		= &sbfs_ops;
	inode_tmp->priv_data 	= NULL;
	inode_tmp->p_inode 		= pinode_tmp;

	return inode_tmp;
} /* sbfs_create_dir() */


struct inode *
sbfs_create_file
(
	struct inode	*parent,
	const char		*name,
	uint32_t		size
)
{
	int	i;
	struct dev 		*sbfs_dev 	= &devs[DEV_DISK];
	struct p_inode 	pinode 		= parent->p_inode;

	uint32_t 		sect		= pinode.addrs[0];
	uint32_t		sect_num	= 0;

	struct p_inode 	pinode_tmp;
	uint32_t 		pinode_tmp_nr	= 0;

	uint32_t		sect_tmp_nr		= 0;
	uint32_t		sect_tmp_nr_2nd	= 0;
	uint32_t 		local_buf[AHCI_SECT_SIZE/4];

	sbfs_dentry_t	*buf_dnt	= (sbfs_dentry_t *)sbfs_buf;

	if ( pinode.type != SBFS_IT_DIR ) {
		sbfs_error( "SBFS can not create file.\n" );
		return parent;
	}

	if ( pinode.size+1 > (uint32_t)32 ) {
		sbfs_error( "SBFS can not contain more than 30 files/dirs in a dir.\n" );
		return parent;	
	}

	if ( size > (((AHCI_SECT_SIZE/4)+8)*AHCI_SECT_SIZE) ) {
		sbfs_error( "SBFS can not create file: size too large.\n" );
		return parent;	
	}

	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );

	for ( i=0; i<pinode.size; i++ ) {
		if ( !strcmp( buf_dnt[i].name, name) ) {
			sbfs_error( "Already has dir/file with the same name.\n" );
			return parent;
		}
	}

	sect_num	= size/AHCI_SECT_SIZE;
	sect_num	= (size%AHCI_SECT_SIZE) ? sect_num+1 : sect_num;

	pinode_tmp_nr	= sbfs_find_free_inode();

	/* modify parent inode */
	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, sect );
	buf_dnt[pinode.size].pinode_nr	= pinode_tmp_nr;
	for ( i=0; i<SBFS_NAME_MAX; i++ ) {
		buf_dnt[pinode.size].name[i] = name[i]; 
		if ( name[i] == '\0' )
			break;
	}
	sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, sect );
	(parent->p_inode).size += 1;
	pinode.size += 1;
	sbfs_set_inode( pinode.minor, &pinode );


	/* create child p_inode */
	pinode_tmp.type		= SBFS_IT_FILE;
	pinode_tmp.major	= 0;
	pinode_tmp.minor	= pinode_tmp_nr;
	pinode_tmp.nlink	= 0;
	pinode_tmp.size		= size;

	for ( i=0; i<NDIRECT; i++ ) {
		sect_tmp_nr 	= sbfs_find_free_sect();
		sbfs_set_bm_occupied( sect_tmp_nr );	
		pinode_tmp.addrs[i]	= sect_tmp_nr;
		sect_num--;
		if ( sect_num==0 )
			break;
	}

	if ( sect_num > 0 ) {
		sect_tmp_nr_2nd 	= sbfs_find_free_sect();
		sbfs_set_bm_occupied( sect_tmp_nr_2nd );
		pinode_tmp.addrs[NDIRECT] = sect_tmp_nr_2nd;

		for ( i=0; ; i++ ) {
			sect_tmp_nr 	= sbfs_find_free_sect();
			sbfs_set_bm_occupied( sect_tmp_nr );	
			local_buf[i]	= sect_tmp_nr;
			sect_num--;
			if ( sect_num==0 )
				break;
		}

		for ( i=0; i<(AHCI_SECT_SIZE/4); i++ ) 
			sbfs_buf[i] = local_buf[i];

		sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, sect_tmp_nr_2nd );
	}

	sbfs_set_inode( pinode_tmp_nr, &pinode_tmp );


	/* create child inode */
	struct inode *inode_tmp = get_inode(NULL);
	inode_tmp->dev_num		= DEV_DISK;
	inode_tmp->fs_ops 		= &sbfs_ops;
	inode_tmp->priv_data 	= NULL;
	inode_tmp->p_inode 		= pinode_tmp;
	return inode_tmp;
} /* sbfs_create_file() */



/* format disk */
int
sbfs_newfs()
{
	uint32_t i, j, cnt;
	struct dev *sbfs_dev = &devs[DEV_DISK];

	struct inode *inode;
	struct p_inode *pinode 		= (struct p_inode *)(sbfs_buf);
	sbfs_dentry_t	*buf_dnt	= (sbfs_dentry_t *)sbfs_buf;

	/* write super block into disk */
	memset( (void *)sbfs_buf, 0, AHCI_SECT_SIZE );
	sbfs_buf[0] = SBFS_SECT_BM;
	sbfs_buf[1] = SBFS_SECT_INODE;
	sbfs_buf[2] = SBFS_SECT_DATA;
	sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, SBFS_SECT_SUPER );

	/* write bit map into disk: 1 for free, 0 for occupied */
	cnt=0;
	for ( i=SBFS_SECT_BM; i<SBFS_SECT_INODE; i++ ) {
		memset( (void *)sbfs_buf, 0, AHCI_SECT_SIZE );
		for ( j=0; j<(AHCI_SECT_SIZE/4); j++ ) {
			if ( cnt<SBFS_SECT_DATA )
				sbfs_buf[j]	= 0x00000000;
			else
				sbfs_buf[j] = 0xFFFFFFFF;
			
			cnt += 32;
		}
		sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, i );
	}

	/* write inode into disk */
	cnt=0;
	for ( i=SBFS_SECT_INODE; i<SBFS_SECT_DATA; i++ ) {
		memset( (void *)pinode, 0, AHCI_SECT_SIZE );
		for ( j=0; j<(AHCI_SECT_SIZE/SBFS_INODE_SIZE); j++, cnt++ ) {
			pinode[j].type	= SBFS_IT_FREE;
			pinode[j].major	= 0;
			pinode[j].minor	= cnt;
			pinode[j].nlink	= 0;
			pinode[j].size	= 0;
			for ( cnt=0; cnt<NDIRECT+1; cnt++ )
				pinode[j].addrs[cnt] = 0;
		}
		sbfs_dev->writesect( sbfs_dev, (void *)pinode, i );
	}

	/* create p_inode 0 as root dir */
	uint32_t	inod_tmp = sbfs_find_free_inode();
	uint32_t	sect_tmp = sbfs_find_free_sect();
	sbfs_set_bm_occupied( sect_tmp );	

	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, SBFS_SECT_INODE );
	pinode[0].type = SBFS_IT_DIR;
	pinode[0].major	= 0;
	pinode[0].minor	= inod_tmp;
	//pinode[0].minor	= 0;
	pinode[0].nlink	= 0;
	//pinode[0].size	= 2;
	pinode[0].size	= 3;
	pinode[0].addrs[0]	= sect_tmp;
	//pinode[0].addrs[0]	= SBFS_SECT_DATA;
	for ( i=1; i<NDIRECT+1; i++ )
		pinode[0].addrs[i]	= 0;
	sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, SBFS_SECT_INODE );

	/* create dentries for inode 0 */
	for ( i=0; i<(AHCI_SECT_SIZE/SBFS_DENTRY_SIZE); i++ ) {
		buf_dnt[i].pinode_nr = 0;
		for ( j=1; j<SBFS_NAME_MAX+1; j++ )
			buf_dnt[i].name[j] = '\0';
	}
	buf_dnt[0].pinode_nr = 0;
	buf_dnt[0].name[0]   = '.';
	buf_dnt[0].name[1]   = '\0';
	buf_dnt[1].pinode_nr = 0;
	buf_dnt[1].name[0]   = '.';
	buf_dnt[1].name[1]   = '.';
	buf_dnt[1].name[2]   = '\0';
	buf_dnt[2].pinode_nr = 0;
	buf_dnt[2].name[0]   = 't';
	buf_dnt[2].name[1]   = 'e';
	buf_dnt[2].name[2]   = 's';
	buf_dnt[2].name[3]   = 't';
	buf_dnt[2].name[4]   = '_';
	buf_dnt[2].name[5]   = 'd';
	buf_dnt[2].name[6]   = 'i';
	buf_dnt[2].name[7]   = 'r';
	buf_dnt[2].name[8]   = '\0';
	//sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, SBFS_SECT_DATA );
	sbfs_dev->writesect( sbfs_dev, (void *)sbfs_buf, sect_tmp );

	inode = get_inode(diskfs);
	inode->p_inode 		 = pinode[0];

	return 0;
} /* sbfs_newfs() */ 


int sbfs_init(void)
{
	sbfs_buf = (uint32_t *)kmalloc( 512, PG_SUP );

	struct dev *sbfs_dev    = &devs[DEV_DISK];
	struct p_inode *pinode 	= (struct p_inode *)(sbfs_buf);
	struct inode *inode;

	//sbfs_newfs();

	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, 0 );
	sbfs_dev->super_block = &sbfs_superblk;
	sbfs_superblk.sect_nr_bm		= sbfs_buf[0];
	sbfs_superblk.sect_nr_inode		= sbfs_buf[1];
	sbfs_superblk.sect_nr_data		= sbfs_buf[2];
	k_printf( 0, "SBFS: %d, %d, %d\n", sbfs_superblk.sect_nr_bm, sbfs_superblk.sect_nr_inode, sbfs_superblk.sect_nr_data );

	/* get p_inode 0 and set to inode */
	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, SBFS_SECT_INODE );
	inode = get_inode(diskfs);
	inode->fs_ops 		= &sbfs_ops;
	inode->priv_data 	= NULL;
	inode->p_inode 		= pinode[0];
	k_printf( 0, "SBFS: %d, %d, %d, %d\n", (inode->p_inode).type, (inode->p_inode).minor, (inode->p_inode).size, (inode->p_inode).addrs[0] );

#if TEST_SBFS
	struct inode *inode2;

	k_printf( 0, "SBFS: %d, %d, %d\n", sbfs_superblk.sect_nr_bm, sbfs_superblk.sect_nr_inode, sbfs_superblk.sect_nr_data );
	k_printf( 0, "SBFS: %d, %d, %d, %d\n", (inode->p_inode).type, (inode->p_inode).minor, (inode->p_inode).size, (inode->p_inode).addrs[0] );
	int i;
	sbfs_dentry_t *buf_dnt	= (sbfs_dentry_t *)(sbfs_buf);
	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, (inode->p_inode).addrs[0] );
	for ( i=0; i<(inode->p_inode).size; i++ )
		k_printf( 0, "SBFS: 0 rootdir file %d: pinode=%d, name=%s\n", i, buf_dnt[i].pinode_nr, buf_dnt[i].name );

	uint32_t	inod_tmp = sbfs_find_free_inode();
	uint32_t	sect_tmp = sbfs_find_free_sect();
	k_printf( 0, "SBFS: get_free: inode=%d: sect=%d\n", inod_tmp, sect_tmp );
	
	struct inode *inode_tmp = sbfs_path_lookup( inode, "test_dir" );
	k_printf( 0, "SBFS: path_lookup pinode_nr=%d, pinode_addrs0=%d\n", (inode_tmp->p_inode).minor, (inode_tmp->p_inode).addrs[0] );

	sbfs_dentry_t *dirs = (sbfs_dentry_t *)kmalloc( 512, PG_SUP);
	sbfs_getdirents( inode, (void *)dirs, 0, (inode->p_inode).size );
	for ( i=0; i<(inode->p_inode).size; i++ )
		k_printf( 0, "SBFS: 1 rootdir file %d: pinode=%d, name=%s\n", i, dirs[i].pinode_nr, dirs[i].name );

	inode2 = sbfs_create_dir( inode, "test_dir2" );	
	sbfs_getdirents( inode, (void *)dirs, 0, (inode->p_inode).size );
	for ( i=0; i<(inode->p_inode).size; i++ )
		k_printf( 0, "SBFS: 2 rootdir file %d: pinode=%d, name=%s\n", i, dirs[i].pinode_nr, dirs[i].name );

	inode_tmp = sbfs_path_lookup( inode, "test_dir2" );
	sbfs_getdirents( inode_tmp, (void *)dirs, 0, (inode_tmp->p_inode).size );
	k_printf( 0, "SBFS: path_lookup pinode_nr=%d, pinode_addrs0=%d\n", (inode_tmp->p_inode).minor, (inode_tmp->p_inode).addrs[0] );
	for ( i=0; i<(inode_tmp->p_inode).size; i++ )
		k_printf( 0, "SBFS: 3 rootdir file %d: pinode=%d, name=%s\n", i, dirs[i].pinode_nr, dirs[i].name );

	inode_tmp = sbfs_create_file( inode2, "test_file", 8192 );
	sbfs_getdirents( inode2, (void *)dirs, 0, (inode2->p_inode).size );
	k_printf( 0, "SBFS: get_dentry rootfs=%d, pinode_addrs0=%d\n", (inode2->p_inode).minor, (inode2->p_inode).size );
	for ( i=0; i<(inode2->p_inode).size; i++ )
		k_printf( 0, "SBFS: 3 rootdir file %d: pinode=%d, name=%s\n", i, dirs[i].pinode_nr, dirs[i].name );

	k_printf( 0, "SBFS: created_file=%d, size=%d\n", (inode_tmp->p_inode).minor, (inode_tmp->p_inode).size );
	for ( i=0; i<NDIRECT; i++ )
		k_printf( 0, "SBFS: created file sect=%d = %d\n", i, (inode_tmp->p_inode).addrs[i] );
	sbfs_dev->readsect( sbfs_dev, (void *)sbfs_buf, (inode_tmp->p_inode).addrs[NDIRECT] );
	for ( i=0; i<4; i++ )
		k_printf( 0, "SBFS: created file sect=%d = %d\n", i+NDIRECT, sbfs_buf[i] );

	inode2 = sbfs_path_lookup( inode, "test_dir2/test_file" );
	k_printf( 0, "SBFS: path_lookup file pinode_nr=%d, pinode_addrs0=%d\n", (inode2->p_inode).minor, (inode2->p_inode).addrs[0] );

	uint08_t *write_buf = (uint08_t *)kmalloc( 8192, PG_SUP);
	int j;
	for ( i=0; i<16; i++ )
		for ( j=0; j<512; j++ )	
			write_buf[(i*512)+j] = i;

	sbfs_write( inode_tmp, (void *)write_buf, 0, 8192 );
	
	for ( i=0; i<16; i++ )
		for ( j=0; j<512; j++ )	
			write_buf[(i*512)+j] = 0;

	k_printf( 0, "SBFS: print after clean \n" );
	for ( i=0; i<16; i++ )
		k_printf( 0, "%d ", write_buf[512*i] );
	k_printf( 0, "\n" );
	sbfs_read( inode_tmp, (void *)write_buf, 0, 8192 );
	k_printf( 0, "SBFS: print after read  \n");
	for ( i=0; i<16; i++ )
		k_printf( 0, "%d ", write_buf[512*i] );
	k_printf( 0, "\n" );

	for ( i=0; i<16; i++ )
		for ( j=0; j<512; j++ )	
			write_buf[(i*512)+j] = (uint32_t)((i*512)+j);
	sbfs_write( inode_tmp, (void *)write_buf, 0, 8192 );


	kfree( write_buf );
#endif
	

	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
