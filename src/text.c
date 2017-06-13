/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "text.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dod.h"

Text text_init(char const *buffer,size_t nbytes)
{
    Text self ;
    self.buffer = buffer ;
    self.nbytes = nbytes ;
    self.p      = buffer ;
    self.len    =      0 ;
    self.row    =      1 ;
    self.col    =      0 ;
    return self ;
}

/* return the number of bytes that are left */
static size_t text_left(Text *self)
{
    return self->nbytes - (size_t)(self->p + self->len - self->buffer) ;
}

/* move pointer forward (by token-length bytes) */
static void text_forward(Text *self)
{
    for (size_t i=0 ; i<self->len ; ++i)
    {
	char c = *self->p++ ;
	if (c == '\n')
	{
	    ++self->row ;
	    self->col = 0 ;
	}
	else ++self->col ;
    }
    self->len = 0 ;
}

bool text_end(Text *self)
{
    return 0 == text_left(self) ;
}

bool text_bool(Text *self)
{
    text_forward(self) ;
    unsigned i = text_choice(
	self,"0","off","false","no","1","on","true","yes",NULL) ;
    return i >= 4 ;
}

void text_finish(Text *self)
{
    text_forward(self) ;
    if (!text_end(self))
    {
	dod_exit("text:unexpected character(s) at %zu:%zu <%.*s>",
		 self->row,self->col,(int)text_left(self),self->p) ;
	abort() ;
    }
}

/* expect one of the arguments as next token, 
   return (index+1) of the argument that matches */
static unsigned scanVa(Text *self,va_list *ap)
{
    unsigned i = 0 ;
    while (true)
    {
	++i ;
	char const *s = va_arg(*ap,char const*) ;
	if (s == NULL)
	    return 0 ;
	size_t n = strlen(s) ;
	if (n > text_left(self))
	    continue ;
	if (0 != memcmp(s,self->p,n))
	    continue ;
	self->len = n ;
	return i ;
    }
}

/* concatenate the string arguments */
static void appendVa(char *dst,size_t dlen,va_list *ap)
{
    assert(dlen > 0) ;
    size_t len = 0 ; dst[len] = '\0' ;
    while (true)
    {
	char const *s = va_arg(*ap,char const*) ;
	if (s == NULL)
	    break ;
	size_t left = (size_t)(dlen-len) ;
	int result = snprintf(dst+len,left," <%s>",s) ;
	if (result < 0)
	    dod_exit("text:snprintf failed") ;
	size_t n = (size_t)result ;
	len += (n >= left) ? left-1u : n ;
    }
    if (len == dlen-1 && len >= 3)
	memcpy(dst+len-3,"...",3) ;
}

unsigned text_choice(Text *self,...)
{
    text_forward(self) ;
    va_list ap ; va_start(ap,self) ;
    unsigned i = scanVa(self,&ap) ;
    va_end(ap) ;
    if (i > 0)
      return (i-1) ;

    char list[0x100] ; va_start(ap,self) ;
    appendVa(list,sizeof(list),&ap) ;
    va_end(ap) ;
    dod_exit("text:no match at %zu:%zu, valid tokens are:%s",
	     self->row,self->col,list) ;
    abort() ;
}

unsigned text_ifChoice(Text *self,...)
{
    text_forward(self) ;
    va_list ap ; va_start(ap,self) ;
    unsigned i = scanVa(self,&ap) ;
    va_end(ap) ;
    return i ;
}

void text_clear(Text *self)
{
    text_forward(self) ;
    bool comment = false ;
    while (!text_end(self))
    {
	char c = self->p[self->len] ;
	if (comment) 
	{
	    ++self->len ;
	    if (c == '\n')	    
		comment = false ;
	    continue ;
	}
	if (isspace((unsigned)c))
	{
	    ++self->len ;
	    continue ;
	}
	if (c == '#')
	{
	    comment = true ;
	    ++self->len ;
	    continue ;
	}
	break ;
    }
}

void text_space(Text *self)
{
    text_clear(self) ;
    if (self->len > 0)
	return ;
    dod_exit("text:expect white-space at %zu:%zu",
	     self->row,self->col) ;
    abort() ;
}

/* copy src string to dst; truncate if too long */
static size_t ncopy(char *dst,size_t dlen,char const *src,size_t slen)
{
    assert(dlen > 0) ;
    size_t len = (dlen-1 < slen) ? dlen-1 : slen ;
    memcpy(dst,src,len) ;
    dst[len] = '\0' ;
    return len ;
}

unsigned long long text_llu(Text *self)
{
    text_forward(self) ;
    if (text_end(self))
	dod_exit("text:premature end") ;
    if (*self->p == '-')
    {
	/* stroull says that negative values are considered valid input
	   and are silently converted to the equivalent unsigned value. */
	dod_exit("text:integer must be non-negative at %zu:%zu",
		 self->row,self->col) ;
    }
    char buffer[0x20] ;
    ncopy(buffer,sizeof(buffer),self->p,text_left(self)) ;
    
    char *p ; errno = 0 ;
    long long unsigned i = strtoull(buffer,&p,0) ;
    if ((errno!=0) || (p==buffer))
	dod_exit("text:invalid integer at %zu:%zu",self->row,self->col) ;
    
    self->len = (size_t)(p - buffer) ;
    return i ;
}

double text_double(Text *self)
{
    text_forward(self) ;
    char buffer[0x20] ;
    ncopy(buffer,sizeof(buffer),self->p,text_left(self)) ;
    
    char *p ; errno = 0 ;
    double f = strtod(buffer,&p) ;
    if ((errno!=0) || (p==buffer))
	dod_exit("text:invalid floating point number at %zu:%zu",
		 self->row,self->col) ;
    
    self->len = (size_t)(p - buffer) ;
    return f ;
}
