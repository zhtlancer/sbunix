
#ifndef __K_STDIO_H__
#define __K_STDIO_H__

//#include "./gccbuiltin.h"
//#include "../cse506-git_new/include/defs.h"

#include <sys/gccbuiltin.h>
#include <defs.h>

#define __memput_char(addr, val)  (*(volatile char*)     (addr)) = val
#define __memput_int(addr, val)   (*(volatile int*)      (addr)) = val
#define __memput_long(addr, val)  (*(volatile long*)     (addr)) = val
#define __memput_llong(addr, val) (*(volatile long long*)(addr)) = val

#define __memget_char(addr)       (*(volatile char*)     (addr))
#define __memget_int(addr)        (*(volatile int*)      (addr))
#define __memget_long(addr)       (*(volatile long*)     (addr))
#define __memget_llong(addr)      (*(volatile long long*)(addr))

/*
typedef char            kt_bool;
typedef unsigned char   uchr;
typedef unsigned short  usrt;
typedef unsigned int    uint;
typedef unsigned long   ulng;

typedef unsigned char   kt_uint08; 
typedef unsigned short  kt_uint16;
typedef unsigned int    kt_uint32; 
typedef unsigned long`  kt_uint64; 

typedef char            kt_sint08; 
typedef short           kt_sint16;
typedef int             kt_sint32; 
typedef long`           kt_sint64; 

typedef uint64_t        kt_addr;
*/

/* k_printf: max length of a format string */
#define K_PFMT_LEN_MAX 15

/* maximum length of a string comverted from a number */
#define K_NUM_STR_LEN_MAX 22

/* k_printf: format decode state */
#define K_PFMT_ST_FLG 0
#define K_PFMT_ST_WID 1
#define K_PFMT_ST_PRC 2
#define K_PFMT_ST_MOD 3

#define K_PFMT_NEG 0
#define K_PFMT_POS 1
#define K_PFMT_STR 2
#define K_PFMT_PND 3
#define K_PFMT_PRC 4
#define K_PFMT_DOT 5

//#define k_putchar( lvl, x ) putchar(x)

#define K_PLVL_EMERG   0 /* print level: system is unusable*/
#define K_PLVL_ALERT   1 /* print level: action must be taken immediately*/
#define K_PLVL_CRIT    2 /* print level: critical conditions*/
#define K_PLVL_ERR     3 /* print level: error conditions*/
#define K_PLVL_WARNING 4 /* print level: warning conditions*/
#define K_PLVL_NOTICE  5 /* print level: normal but significant condition*/
#define K_PLVL_INFO    6 /* print level: informational*/
#define K_PLVL_DEBUG   7 /* print level: debug-level messages*/

#define VGATEXT_PBASE 0xB8000
#define __VGATEXT_MAX_X 80
#define __VGATEXT_MAX_Y 24

#define __TAB_SIZE      4

extern addr_t           vgatext_vbase;
extern unsigned char    vgatext_x;
extern unsigned char    vgatext_y;

/*=======================================================================*/

int
k_puts
( 
    unsigned char   lvl,
    const char      *str
);

int
k_ltocstr
(
    long            i,
    char            *str,
    const char      cmd
);


/*
 * return c string length
 **/
int
k_strlen
(
    const char      *str
);


int
k_strfill
(
    uint08_t        lvl,
    int             len,
    const char      c
);


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
);


int
k_printf
(
    uint08_t         lvl,
    const char      *fmt,
    ...
);

int vgatext_scroll();
int vgatext_putchar(const char);
int k_putchar( unsigned char lvl, const char);

void panic(const char *s);

#endif /* __K_STDIO_H__ */
