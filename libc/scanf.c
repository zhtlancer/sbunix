#include <defs.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

#define BUF_SIZE 512

static char buf[BUF_SIZE+1];

/* get a line of input from STDIN, and the newline at tail will be replaced by '\0' */
int gets_l(char *buf, int size)
{
	int rval;
	rval = read(0, buf, size-1);
	buf[rval] = '\0';
	if (rval > 0 && buf[rval-1] == '\n')
		buf[--rval] = '\0';
	return rval;
}


int
cstr2i
(
	const char *cstr
)
{
	int i=0;
	int neg=0;
	int cnt=0;

	if ( cstr[i] == '\0' )
		return 0;
	
	if ( cstr[i] == '-' ) {
		neg = 1;
		i++;
	}

 	for (;cstr[i] != '\0'; i++ )
		cnt = (cnt*10)+(cstr[i]-'0');
	
	if ( neg )
		cnt = 0 - cnt;	

	return cnt;

} /* cstr2i() */



static int vfdscanf(int fd, const char *fmt, va_list ap)
{
	char c 			= fmt[1];
	char *str 		= NULL;
	int	 *num	 	= NULL;
	int rval = 0;
	int i;

	if ( c == 'd' ) {
		num		= (int *)va_arg(ap, int	 *);
		rval 	= gets_l(buf, BUF_SIZE);
		*num	= cstr2i( buf );
	}
	else if ( c == 's' ) {
		str		= (char *)va_arg(ap, char *);
		rval 	= gets_l(buf, BUF_SIZE);
		for ( i=0; i<rval; i++ )
			str[i] = buf[i];
		str[i] = '\0';
	}

	return 0;
}

int fdscanf(int fd, const char *fmt, ...)
{
	int rval;
	va_list ap;
	va_start(ap, fmt);

	rval = vfdscanf(fd, fmt, ap);
	va_end(ap);

	return rval;
}

int scanf(const char *fmt, ...)
{
	int rval;
	va_list ap;
	va_start(ap, fmt);

	rval = vfdscanf(FD_STDIN, fmt, ap);
	va_end(ap);

	return rval;
}


/* vim: set ts=4 sw=0 tw=0 noet : */
