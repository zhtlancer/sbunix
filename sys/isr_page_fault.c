
#include <defs.h>
//#include <sys/io.h>
#include <sys/k_stdio.h>
//#include <sys/pic.h>

void isr_page_fault()
{
    k_printf( 0, "isr_page_fault called!!\n" );    
    while(1);
}
