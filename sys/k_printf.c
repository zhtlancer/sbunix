#include <sys/k_stdio.h>
#include <sys/mm.h>

#include <sys/fs.h>
#include <sys/io.h>

unsigned char vgatext_x = 0;
unsigned char vgatext_y = 0;
addr_t vgatext_vbase;

int
k_puts(
    unsigned char   lvl,
    const char      *str
)
{
    unsigned long s_len = 0;
    char c;
    while( (c=str[s_len++]) != '\0' )
        vgatext_putchar(c);
    return s_len-1;
}

static void move_cursor(uint8_t row, uint8_t col)
{
	int pos;

	if (row >= __VGATEXT_MAX_Y || col >= __VGATEXT_MAX_X) {
		return;
	}

	pos = row * 80 + col;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));

	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

int vgatext_scroll()
{
    addr_t vgatext_pos = vgatext_vbase;

    /* scroll each line */
    for ( vgatext_pos=vgatext_vbase; 
          vgatext_pos<vgatext_vbase+((__VGATEXT_MAX_X*(__VGATEXT_MAX_Y-1))<<1);
          vgatext_pos++ 
        )
        __memput_char( vgatext_pos,
                       __memget_char(vgatext_pos+(__VGATEXT_MAX_X<<1)) );


    /* clean the least line */
    for ( ;
          vgatext_pos<vgatext_vbase+((__VGATEXT_MAX_X*(__VGATEXT_MAX_Y)  )<<1);
          vgatext_pos+=2
        ) {
		__memput_char( vgatext_pos, 0x07 );
		__memput_char( vgatext_pos, ' ' );
	}


    return 0;
}


int k_putchar( unsigned char lvl, const char c)
{
    return vgatext_putchar(c);
}

extern int is_kbd_buf_empty(void);
extern int is_kbd_buf_full(void);
extern void kbd_buf_backspace(void);

int vgatext_putchar(const char c)
{
    addr_t vgatext_pos = 0; 

    switch(c) {

    case '\n' : 
        vgatext_x = 0;
        if ( vgatext_y == __VGATEXT_MAX_Y-1 )
            vgatext_scroll();
        else
            vgatext_y++;
        break;
    
    case '\t' :
        vgatext_x = __TAB_SIZE * ((vgatext_x / __TAB_SIZE)+1);
        if ( vgatext_x >  __VGATEXT_MAX_X-1 ) {
            vgatext_x = 0;
            if ( vgatext_y == __VGATEXT_MAX_Y-1 )
                vgatext_scroll();
            else
                vgatext_y++;
        }
        break;
    
    case '\b' :
		if (!is_kbd_buf_empty()) {
			kbd_buf_backspace();
			if ( vgatext_x > 0 )
				vgatext_x -= 1;
			else if (vgatext_y > 0) {
				vgatext_x = __VGATEXT_MAX_X - 1;
				vgatext_y -= 1;
			}
			vgatext_pos = ((vgatext_y*__VGATEXT_MAX_X)+vgatext_x)<<1;
			__memput_char(vgatext_vbase+vgatext_pos  , ' ');
			__memput_char(vgatext_vbase+vgatext_pos+1, (char)7);
		}
        break;

    case '\r' :
            vgatext_x = 0;
        break;

    default   :
        vgatext_pos = ((vgatext_y*__VGATEXT_MAX_X)+vgatext_x)<<1;
        __memput_char(vgatext_vbase+vgatext_pos  , (char)c);
        __memput_char(vgatext_vbase+vgatext_pos+1, (char)7);
        if ( vgatext_x >= __VGATEXT_MAX_X-1 ) {
            vgatext_x = 0;
            if ( vgatext_y == __VGATEXT_MAX_Y-1 )
                vgatext_scroll();
            else
                vgatext_y++;
        }
        else
            vgatext_x++;
        break;
    }
	move_cursor(vgatext_y, vgatext_x);

    return 0; 
}


int
k_ltocstr
(
    long        i,
    char        *str,
    const char  cmd
)
{
    char num_X[]  = "0123456789ABCDEF";
    char num_x[]  = "0123456789abcdef";
    char *num;

    int  len = 0;
    int  base;

    uint64_t i_u;
    uint64_t temp_u;
  
    num = ( cmd == 'X' || cmd == 'P' ) ? num_X : num_x;

    if      ( cmd == 'd' || cmd == 'i' || cmd == 'u' )
        base = 10;
    else if ( cmd == 'o' )
        base =  8;
    else
        base = 16;

    if ( cmd == 'i' || cmd == 'd' ) 
        i = (i>0)?i:-i;

    i_u     = (uint64_t)i;
    temp_u  = i_u;

    do {
        ++str;
        ++len;
        temp_u /= base;
    } while ( temp_u );

    *str-- = '\0';
    
    do {
        *str-- = num[i_u%base];
        i_u /= base;
    } while ( i_u );
    
    return len;
} /* k_ltocstr */


/*
 * return c string length
 **/
int
k_strlen
(
    const char  *str
)
{
    int len;
    for ( len=0; str[len]!='\0'; len++ );
    return len;
} /* k_strlen */


int
k_strfill
(
    uint08_t    lvl,
    int         len,
    const char  c
)
{
    for ( ; len>0; len-- )
        k_putchar( lvl, c );
    return 0;   
}



int
k_printf_formatter
(
    uint08_t        lvl,
    const char      cmd,
    const bool_t    flg_neg,
    const bool_t    flg_pos,
    const bool_t    flg_spc,
    const bool_t    flg_pnd,
    const bool_t    flg_zro,
    const bool_t    wid_str,
    const uint32_t  wid_num,
    const bool_t    prc_str,
    const uint32_t  prc_num,
    va_list         ap
)
{
    uint32_t    s_len = 0;     /* input string length */
    uint32_t    f_len = 0;     /* final string length */
    uint64_t    number_unsigned = 0;
    long        number = 0;
    char        num_str[K_NUM_STR_LEN_MAX+1];
    char        chr_str[2] = {'\0', '\0'};
    char        *str;
    char        numfill = 0;   /* fill char when print number */
    bool_t      altdisp = 0;   /* alternative display */

    uint32_t    wid = 0;       /* assigned print width */

    /* check if width is assigned by argument */
    if ( wid_str )
        wid = (uint32_t)va_arg(ap, int);
    else
        wid = wid_num;

    /* check fill char when print number */
    if ( flg_zro )
        numfill = '0';
    else
        numfill = ' ';

    /* get c string and check alternative display */
    if      ( cmd=='s' ) 
        str = va_arg(ap, char*);
    else if ( cmd=='c' ) {
        chr_str[0] = (char)va_arg(ap, int);
        str = chr_str;
    }
    else if (cmd=='i' || cmd=='d') {
        number = (long)va_arg(ap, int);
        if ( flg_spc || flg_pos || (number<0) )
            altdisp = 1; 
        k_ltocstr( (long)number, num_str, cmd );
        str = num_str;
    }
    else if ( cmd=='u' ) {  
        number_unsigned = (uint64_t)va_arg(ap, uint64_t);
        if ( flg_spc || flg_pos )
            altdisp = 1; 
        k_ltocstr( (long)number_unsigned, num_str, cmd );
        str = num_str;
    }
    else { /* x X o p P */
        number_unsigned = (uint64_t)va_arg(ap, uint64_t);
        if ( flg_pnd )
            altdisp = 1; 
        k_ltocstr( (long)number_unsigned, num_str, cmd );
        str = num_str;
    }

    /* get string length */
    s_len = k_strlen(str);

    /* print formatted string */
    switch ( cmd ) {

    case( 'c' ):
    case( 's' ):
        if ( s_len >= wid ) {
            k_puts( lvl, str);       
            f_len = s_len;
        }
        else {
            if ( !flg_neg )
                k_strfill( lvl, wid-s_len, ' ' );
            k_puts( lvl, str);
            if (  flg_neg )
                k_strfill( lvl, wid-s_len, ' ' );
            f_len = wid;
        }
        break;


    case( 'u' ):
        if ( s_len >= wid ) {
            k_puts( lvl, str );       
            f_len = s_len;
        }
        else {
            if ( !flg_neg )
                k_strfill( lvl, wid-s_len, numfill );
            k_puts( lvl, str);
            if (  flg_neg )
                k_strfill( lvl, wid-s_len, ' ' );
            f_len = wid;
        }
        break;


    case( 'i' ):
    case( 'd' ):
        if ( altdisp )
            s_len+=1;

        if ( s_len >= wid ) {
            if ( number < 0 )
                k_putchar( lvl, '-' );
            else if ( flg_pos )
                k_putchar( lvl, '+' );
            else if ( flg_spc )
                k_putchar( lvl, ' ' ); 
            k_puts( lvl, str );       
            f_len = s_len;
        }
        else {
            if ( altdisp &&  (flg_neg||flg_zro) ) {
                if ( number < 0 )
                    k_putchar( lvl, '-' );
                else if ( flg_pos )
                    k_putchar( lvl, '+' );
                else
                    k_putchar( lvl, ' ' ); 
            }
            if ( !flg_neg )
                k_strfill( lvl, wid-s_len, numfill );
            if ( altdisp && !(flg_neg||flg_zro) ) {
                if ( number < 0 )
                    k_putchar( lvl, '-' );
                else if ( flg_pos )
                    k_putchar( lvl, '+' );
                else
                    k_putchar( lvl, ' ' ); 
            }
            k_puts( lvl, str);       
            if (  flg_neg )
                k_strfill( lvl, wid-s_len, ' ' );
            f_len = wid;
        }
        break;


    case( 'o' ):
        if ( altdisp )
            s_len+=1;

        if ( s_len >= wid ) {
            if ( altdisp )
                k_putchar( lvl, '0' );
            k_puts( lvl, str );       
            f_len = s_len;
        }
        else {
            if ( altdisp &&  (flg_neg||flg_zro) )
                k_putchar( lvl, '0' );
            if ( !flg_neg )
                k_strfill( lvl, wid-s_len, numfill );
            if ( altdisp && !(flg_neg||flg_zro) )
                k_putchar( lvl, '0' );
            k_puts( lvl, str );
            if (  flg_neg )
                k_strfill( lvl, wid-s_len, ' ' );
            f_len = wid;
        }
        break;


    case( 'x' ):
    case( 'X' ):
        if ( altdisp )
            s_len+=2;
        
        if ( s_len >= wid ) {
            if ( altdisp )
                k_puts( lvl, "0x" );
            k_puts( lvl, str);       
            f_len = s_len;
        }
        else {
            if ( altdisp &&  (flg_neg||flg_zro) )
                k_puts( lvl, "0x" );
            if ( !flg_neg )
                k_strfill( lvl, wid-s_len, numfill );
            if ( altdisp && !(flg_neg||flg_zro) )
                k_puts( lvl, "0x" );
            k_puts( lvl, str);
            if (  flg_neg )
                k_strfill( lvl, wid-s_len, ' ' );
            f_len = wid;
        }
        break;


    case( 'p' ):
    case( 'P' ):
       
        s_len += 2; 
        if ( s_len >= wid ) {
            k_puts( lvl, "0x" );
            k_puts( lvl, str);       
            f_len = s_len;
        }
        else {
            if (             (flg_neg||flg_zro) )
                k_puts( lvl, "0x" );
            if ( !flg_neg )
                k_strfill( lvl, wid-s_len, numfill );
            if (            !(flg_neg||flg_zro) )
                k_puts( lvl, "0x" );
            k_puts( lvl, str );
            if (  flg_neg )
                k_strfill( lvl, wid-s_len, ' ' );
            f_len = wid;
        }
        break;

    default:
        break;

    } /* switch ( cmd ) */
    
    /* return length of printed  string */
    return f_len;
} /* k_printf_formatter */


int
k_printf
(
    uint08_t lvl,
    const char *fmt,
    ...
)
{
    va_list ap;
    va_start(ap, fmt);

    int  i;

    char   fmt_state = 0; /* 0:flags, 1:width, 2:precision, 3:modifiers */
    bool_t flg_neg   = 0; /* flag      '-'   */
    bool_t flg_pos   = 0; /* flag      '+'   */
    bool_t flg_spc   = 0; /* flag      ' '   */
    bool_t flg_pnd   = 0; /* flag      '#'   */
    bool_t flg_zro   = 0; /* flag      '0'   */
    bool_t wid_str   = 0; /* width     '*'   */
    uint32_t wid_num   = 0; /* width     num   */
    bool_t prc_str   = 0; /* precision '*'   */
    uint32_t prc_num   = 0; /* precision num   */
    bool_t fmt_end   = 0;

    uint64_t s_len     = 0;
    char c;

    while( (c=*(fmt++)) != '\0' ) {
    
        if ( c != '%' ) { /* normal characters */
            k_putchar( lvl, c );
            s_len++;
        }
        else            { /* start with %, do format decode */

            /* clean out all attr */
            flg_neg = flg_pos = flg_spc = flg_pnd = flg_zro = 0;
            fmt_state  = 0;
            wid_str    = 0;
            wid_num    = 0;
            prc_str    = 0;
            prc_num    = 0;
            fmt_end    = 0;

            for ( i=0; i<K_PFMT_LEN_MAX; i++ ) {
            
                switch( c=*(fmt++) ) {

                case( '%' ):
                    if ( fmt_state == K_PFMT_ST_FLG ) {
                        k_putchar( lvl, '%' );
                        s_len++;
                    }
                    else 
                        goto K_PRINTF_ERROR;
                    fmt_end=1;
                    break;

                case( '-' ):
                    if ( fmt_state == K_PFMT_ST_FLG )
                        flg_neg = 1; 
                    else 
                        goto K_PRINTF_ERROR;
                    break; 

                case( '+' ):
                    if ( fmt_state == K_PFMT_ST_FLG ) {
                        flg_pos = 1;
                        flg_spc = 0;                     
                    }
                    else
                        goto K_PRINTF_ERROR;
                    break; 

                case( ' ' ):
                    if ( fmt_state == K_PFMT_ST_FLG ) {
                        if ( flg_pos == 0 )
                            flg_spc = 1;
                    }
                    else
                        goto K_PRINTF_ERROR;
                    break;
 
                case( '#' ):
                    if ( fmt_state == K_PFMT_ST_FLG )
                        flg_pnd = 1; 
                    else
                        goto K_PRINTF_ERROR;
                    break; 

                case( '.' ):
                    if      ( fmt_state == K_PFMT_ST_FLG )
                        fmt_state = K_PFMT_ST_PRC; 
                    else if ( fmt_state == K_PFMT_ST_WID )
                        fmt_state = K_PFMT_ST_PRC; 
                    else 
                        goto K_PRINTF_ERROR;
                    break; 

                case( '0' ):
                    if      ( fmt_state == K_PFMT_ST_FLG )
                        flg_zro = 1; 
                    else if ( fmt_state == K_PFMT_ST_WID ) {
                        if ( wid_str == 1 )
                            goto K_PRINTF_ERROR;
                        wid_num = wid_num * 10;
                    }
                    else if ( fmt_state == K_PFMT_ST_PRC ) {
                        if ( prc_str == 1 )
                            goto K_PRINTF_ERROR;
                        prc_num = prc_num * 10;
                    }
                    else
                        goto K_PRINTF_ERROR;
                    break; 

                case( '1' ):
                case( '2' ):
                case( '3' ):
                case( '4' ):
                case( '5' ):
                case( '6' ):
                case( '7' ):
                case( '8' ):
                case( '9' ):
                    if      ( fmt_state == K_PFMT_ST_FLG ) {
                        fmt_state = K_PFMT_ST_WID;
                        wid_num = ((unsigned int)c) - '0';
                    }
                    else if ( fmt_state == K_PFMT_ST_WID ) {
                        if ( wid_str == 1 )
                            goto K_PRINTF_ERROR;
                        wid_num = wid_num * 10 + ((unsigned int)c) - '0';
                    }
                    else if ( fmt_state == K_PFMT_ST_PRC ) {
                        if ( prc_str == 1 )
                            goto K_PRINTF_ERROR;
                        prc_num = prc_num * 10 + ((unsigned int)c) - '0';
                    }
                    else
                        goto K_PRINTF_ERROR;
                    break; 

                case( '*' ):
                    if      ( fmt_state == K_PFMT_ST_FLG ) {
                        wid_str = 1;
                        fmt_state = K_PFMT_ST_WID;
                    }
                    else if ( fmt_state == K_PFMT_ST_WID )
                        goto K_PRINTF_ERROR;
                    else if ( fmt_state == K_PFMT_ST_PRC ) {
                        if ( prc_str == 1 )
                            goto K_PRINTF_ERROR;
                        prc_str = 1;
                    }
                    else
                        goto K_PRINTF_ERROR;
                    break; 

                case( 'i' ):
                case( 'd' ):
                case( 'u' ):
                case( 'x' ):
                case( 'X' ):
                case( 'o' ):
                case( 'p' ):
                case( 'P' ):
                case( 'c' ):
                case( 's' ):

                    s_len += k_printf_formatter(
                                               lvl,
                                               c,
                                               flg_neg,
                                               flg_pos,
                                               flg_spc,
                                               flg_pnd,
                                               flg_zro,
                                               wid_str,
                                               wid_num,
                                               prc_str,
                                               prc_num,
                                               ap       );
                    fmt_end = 1;
                    break;

                default:
                    goto K_PRINTF_ERROR;
                    break;

                } /* switch( c=*(fmt++) ) */

                if ( fmt_end )
                    break;

            } /* for ( i=0; i<K_PFMT_LEN_MAX; i++ ) */

            if ( !fmt_end )
                goto K_PRINTF_ERROR;

        } /* end of format decode ( c == '%' ) */

    } /* while( (c=*(fmt++)) != '\0' ) */

    va_end(ap);
    return s_len;

K_PRINTF_ERROR:
    k_puts( lvl, "k_printf format error!!\n" ); 
    va_end(ap);
    return -1;

}

void panic(const char *s)
{
	k_puts(0, s);
	k_putchar(0, '\n');

	while (1)
		;
}

size_t console_read(struct file *file, void *buf, size_t nbytes)
{
	//TODO
	return 0;
}

size_t console_write(struct file *file, void *buf, size_t nbytes)
{
	for (size_t i = 0; i < nbytes; i++)
		k_putchar(0, *(char *)buf++);

	return nbytes;
}

struct file_operations stdin_ops = {
	.open = NULL,
	.close = NULL,
	.seek = NULL,
	.read = console_read,
	.write = NULL,
};

struct file_operations stdout_ops = {
	.open = NULL,
	.close = NULL,
	.seek = NULL,
	.read = NULL,
	.write = console_write,
};

int console_init(void)
{
	/* stdin */
	files[0].ref = 1;
	files[0].readable = 1;
	files[0].writeable = 0;
	files[0].f_ops = &stdin_ops;

	/* stdout */
	files[1].ref = 1;
	files[1].readable = 0;
	files[1].writeable = 1;
	files[1].f_ops = &stdout_ops;

	/* stderr */
	files[2].ref = 1;
	files[2].readable = 0;
	files[2].writeable = 1;
	files[2].f_ops = &stdout_ops;

	return 0;
}

/* vim: set ts=4 sw=0 tw=0 noet : */
