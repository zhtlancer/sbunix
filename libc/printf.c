#include <stdio.h>
#include <string.h>
#include <syscall.h>

#define OUTPUT	1
#define BUF_SIZE 512

#define PFMT_LEN_MAX 15
#define NUM_STR_LEN_MAX 22

/* k_printf: format decode state */
#define PFMT_ST_FLG 0
#define PFMT_ST_WID 1
#define PFMT_ST_PRC 2
#define PFMT_ST_MOD 3

#define PFMT_NEG 0
#define PFMT_POS 1
#define PFMT_STR 2
#define PFMT_PND 3
#define PFMT_PRC 4
#define PFMT_DOT 5


char buf[BUF_SIZE];
size_t pos;

static void flush_buf(void)
{
	if (pos > 0)
		write(1, buf, pos);
	pos = 0;
}

static void write_buf(char c)
{
	if (pos >= BUF_SIZE)
		flush_buf();

	buf[pos++] = (char)c;
}

int putchar(int c)
{
	write_buf((char)c);
	return c;
}

int puts(const char *s)
{
	size_t count = 0;
	while (s[count] != '\0')
		putchar(s[count++]);
	putchar('\n');
	return count;
}

/* puts with no newline */
int puts_nl(const char *s)
{
	size_t count = 0;
	while (s[count] != '\0')
		putchar(s[count++]);
	return count;
}

int ltocstr(long i, char *str, const char cmd)
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

int strfill(int len, const char c)
{
    for ( ; len>0; len-- )
        putchar( c );
    return 0;   
}

static int printf_formatter(
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
    va_list         ap)
{
    uint32_t    s_len = 0;     /* input string length */
    uint32_t    f_len = 0;     /* final string length */
    uint64_t    number_unsigned = 0;
    long        number = 0;
    char        num_str[NUM_STR_LEN_MAX+1];
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
        ltocstr( (long)number, num_str, cmd );
        str = num_str;
    }
    else if ( cmd=='u' ) {  
        number_unsigned = (uint64_t)va_arg(ap, uint64_t);
        if ( flg_spc || flg_pos )
            altdisp = 1; 
        ltocstr( (long)number_unsigned, num_str, cmd );
        str = num_str;
    }
    else { /* x X o p P */
        number_unsigned = (uint64_t)va_arg(ap, uint64_t);
        if ( flg_pnd )
            altdisp = 1; 
        ltocstr( (long)number_unsigned, num_str, cmd );
        str = num_str;
    }

    /* get string length */
    s_len = strlen(str);

    /* print formatted string */
    switch ( cmd ) {

    case( 'c' ):
    case( 's' ):
        if ( s_len >= wid ) {
            puts_nl( str);       
            f_len = s_len;
        }
        else {
            if ( !flg_neg )
                strfill( wid-s_len, ' ' );
            puts_nl( str);
            if (  flg_neg )
                strfill( wid-s_len, ' ' );
            f_len = wid;
        }
        break;


    case( 'u' ):
        if ( s_len >= wid ) {
            puts_nl( str );       
            f_len = s_len;
        }
        else {
            if ( !flg_neg )
                strfill( wid-s_len, numfill );
            puts_nl( str);
            if (  flg_neg )
                strfill( wid-s_len, ' ' );
            f_len = wid;
        }
        break;


    case( 'i' ):
    case( 'd' ):
        if ( altdisp )
            s_len+=1;

        if ( s_len >= wid ) {
            if ( number < 0 )
                putchar( '-' );
            else if ( flg_pos )
                putchar( '+' );
            else if ( flg_spc )
                putchar( ' ' ); 
            puts_nl( str );       
            f_len = s_len;
        }
        else {
            if ( altdisp &&  (flg_neg||flg_zro) ) {
                if ( number < 0 )
                    putchar( '-' );
                else if ( flg_pos )
                    putchar( '+' );
                else
                    putchar( ' ' ); 
            }
            if ( !flg_neg )
                strfill( wid-s_len, numfill );
            if ( altdisp && !(flg_neg||flg_zro) ) {
                if ( number < 0 )
                    putchar( '-' );
                else if ( flg_pos )
                    putchar( '+' );
                else
                    putchar( ' ' ); 
            }
            puts_nl( str);       
            if (  flg_neg )
                strfill( wid-s_len, ' ' );
            f_len = wid;
        }
        break;


    case( 'o' ):
        if ( altdisp )
            s_len+=1;

        if ( s_len >= wid ) {
            if ( altdisp )
                putchar( '0' );
            puts_nl( str );       
            f_len = s_len;
        }
        else {
            if ( altdisp &&  (flg_neg||flg_zro) )
                putchar( '0' );
            if ( !flg_neg )
                strfill( wid-s_len, numfill );
            if ( altdisp && !(flg_neg||flg_zro) )
                putchar( '0' );
            puts_nl( str );
            if (  flg_neg )
                strfill( wid-s_len, ' ' );
            f_len = wid;
        }
        break;


    case( 'x' ):
    case( 'X' ):
        if ( altdisp )
            s_len+=2;
        
        if ( s_len >= wid ) {
            if ( altdisp )
                puts_nl( "0x" );
            puts_nl( str);       
            f_len = s_len;
        }
        else {
            if ( altdisp &&  (flg_neg||flg_zro) )
                puts_nl( "0x" );
            if ( !flg_neg )
                strfill( wid-s_len, numfill );
            if ( altdisp && !(flg_neg||flg_zro) )
                puts_nl( "0x" );
            puts_nl( str);
            if (  flg_neg )
                strfill( wid-s_len, ' ' );
            f_len = wid;
        }
        break;


    case( 'p' ):
    case( 'P' ):
       
        s_len += 2; 
        if ( s_len >= wid ) {
            puts_nl( "0x" );
            puts_nl( str);       
            f_len = s_len;
        }
        else {
            if ( (flg_neg||flg_zro) )
                puts_nl( "0x" );
            if ( !flg_neg )
                strfill( wid-s_len, numfill );
            if (            !(flg_neg||flg_zro) )
                puts_nl( "0x" );
            puts_nl( str );
            if (  flg_neg )
                strfill( wid-s_len, ' ' );
            f_len = wid;
        }
        break;

    default:
        break;

    } /* switch ( cmd ) */
    
    /* return length of printed  string */
    return f_len;
} /* k_printf_formatter */


int vprintf(const char *fmt, va_list ap)
{

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
            putchar( c );
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

            for ( i=0; i<PFMT_LEN_MAX; i++ ) {
            
                switch( c=*(fmt++) ) {

                case( '%' ):
                    if ( fmt_state == PFMT_ST_FLG ) {
                        putchar( '%' );
                        s_len++;
                    }
                    else 
                        goto PRINTF_ERROR;
                    fmt_end=1;
                    break;

                case( '-' ):
                    if ( fmt_state == PFMT_ST_FLG )
                        flg_neg = 1; 
                    else 
                        goto PRINTF_ERROR;
                    break; 

                case( '+' ):
                    if ( fmt_state == PFMT_ST_FLG ) {
                        flg_pos = 1;
                        flg_spc = 0;                     
                    }
                    else
                        goto PRINTF_ERROR;
                    break; 

                case( ' ' ):
                    if ( fmt_state == PFMT_ST_FLG ) {
                        if ( flg_pos == 0 )
                            flg_spc = 1;
                    }
                    else
                        goto PRINTF_ERROR;
                    break;
 
                case( '#' ):
                    if ( fmt_state == PFMT_ST_FLG )
                        flg_pnd = 1; 
                    else
                        goto PRINTF_ERROR;
                    break; 

                case( '.' ):
                    if      ( fmt_state == PFMT_ST_FLG )
                        fmt_state = PFMT_ST_PRC; 
                    else if ( fmt_state == PFMT_ST_WID )
                        fmt_state = PFMT_ST_PRC; 
                    else 
                        goto PRINTF_ERROR;
                    break; 

                case( '0' ):
                    if      ( fmt_state == PFMT_ST_FLG )
                        flg_zro = 1; 
                    else if ( fmt_state == PFMT_ST_WID ) {
                        if ( wid_str == 1 )
                            goto PRINTF_ERROR;
                        wid_num = wid_num * 10;
                    }
                    else if ( fmt_state == PFMT_ST_PRC ) {
                        if ( prc_str == 1 )
                            goto PRINTF_ERROR;
                        prc_num = prc_num * 10;
                    }
                    else
                        goto PRINTF_ERROR;
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
                    if      ( fmt_state == PFMT_ST_FLG ) {
                        fmt_state = PFMT_ST_WID;
                        wid_num = ((unsigned int)c) - '0';
                    }
                    else if ( fmt_state == PFMT_ST_WID ) {
                        if ( wid_str == 1 )
                            goto PRINTF_ERROR;
                        wid_num = wid_num * 10 + ((unsigned int)c) - '0';
                    }
                    else if ( fmt_state == PFMT_ST_PRC ) {
                        if ( prc_str == 1 )
                            goto PRINTF_ERROR;
                        prc_num = prc_num * 10 + ((unsigned int)c) - '0';
                    }
                    else
                        goto PRINTF_ERROR;
                    break; 

                case( '*' ):
                    if      ( fmt_state == PFMT_ST_FLG ) {
                        wid_str = 1;
                        fmt_state = PFMT_ST_WID;
                    }
                    else if ( fmt_state == PFMT_ST_WID )
                        goto PRINTF_ERROR;
                    else if ( fmt_state == PFMT_ST_PRC ) {
                        if ( prc_str == 1 )
                            goto PRINTF_ERROR;
                        prc_str = 1;
                    }
                    else
                        goto PRINTF_ERROR;
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

                    s_len += printf_formatter(
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
                    goto PRINTF_ERROR;
                    break;

                } /* switch( c=*(fmt++) ) */

                if ( fmt_end )
                    break;

            } /* for ( i=0; i<PFMT_LEN_MAX; i++ ) */

            if ( !fmt_end )
                goto PRINTF_ERROR;

        } /* end of format decode ( c == '%' ) */

    } /* while( (c=*(fmt++)) != '\0' ) */

    va_end(ap);
    return s_len;

PRINTF_ERROR:
    puts( "printf format error!!" ); 
    return -1;

}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
	int cnt = 0;

	cnt = vprintf(fmt, ap);

    va_end(ap);
	flush_buf();
	return cnt;
}
/* vim: set ts=4 sw=0 tw=0 noet : */
