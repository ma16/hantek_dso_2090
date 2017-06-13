/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "dod.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dod_exit(char const *format,...)
{
    va_list ap ;
    va_start(ap,format) ;
    vfprintf(stderr,format,ap) ;
    va_end(ap) ;
    fprintf(stderr,"\n") ;
    exit(1) ;
}

void* dod_malloc(size_t nbytes)
{
    void *p = malloc(nbytes) ;
    if (p != NULL)
	return p ;
    if (nbytes == 0)
	return p ;
    dod_exit("dod:malloc(%zu) failed",nbytes) ; abort() ;
}

void dod_snprintf(char *p,size_t n,char const *format,...)
{
    va_list ap ;
    va_start(ap,format) ;
    int result = vsnprintf(p,n,format,ap) ;
    if (result < 0)
	dod_exit("dod:invalid printf format") ;
    unsigned len = (unsigned)result ;
    if (len >= n)
	dod_exit("dod:buffer would overflow") ;
}

double dod_strtod(char const *s)
{
    char *p ; errno = 0 ;
    double f = strtod(s,&p) ;
    if ((errno!=0) || p[0])
	dod_exit("dod:invalid double:<%s>",s) ;
    return f ;
}

unsigned long long dod_strtoull(char const *s)
{
    if (strchr(s,'-') != NULL)
	// ...stroull says that negative values are considered valid input
	// and are silently converted to the equivalent unsigned value.
	dod_exit("opt:integer must be non-negative:<%s>",s) ;
    char *p ; errno = 0 ;
    unsigned long long i = strtoull(s,&p,0) ;
    if ((errno!=0) || p[0])
	dod_exit("dod:invalid integer:<%s>",s) ;
    return i ;
}

size_t dod_loadFile(char const *path,char *buffer,size_t length)
{
    FILE *file = fopen(path,"rb") ;
    if (file == NULL)
	dod_exit("dod:fopen(%s):%s",path,strerror(errno)) ;

    /* the complete file contents must fit into the buffer with 
       exactly one read operation (that wouldn't work on sockets) */
    
    size_t nread = fread(buffer,1,length,file) ;
    if (ferror(file))
	dod_exit("dod:fread(%s):an error occurred",path) ;
    if (!feof(file))
	dod_exit("dod:fread(%s):file too large",path) ;
    
    int result = fclose(file) ;
    if (result != 0)
	dod_exit("dod:fclose(%s):%s",path,strerror(errno)) ;

    return nread ;
}

