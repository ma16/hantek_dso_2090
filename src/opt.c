/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "opt.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dod.h"

Opt opt_init(int argc,char *argv[])
{
    if (argc < 0)
	dod_exit("opt:invalid number of arguments") ;
    Opt self ;
    self.index = 0 ;
    self.optN = argc ;
    self.optV = argv ;
    return self ;
}

bool opt_bool(Opt *self)
{
    char const *s = opt_pop(self) ;
    
    if (0 == strcmp(s,    "0")) return false ;
    if (0 == strcmp(s,  "off")) return false ;
    if (0 == strcmp(s,"false")) return false ;
    if (0 == strcmp(s,   "no")) return false ;
    if (0 == strcmp(s,    "1")) return  true ;
    if (0 == strcmp(s,   "on")) return  true ;
    if (0 == strcmp(s, "true")) return  true ;
    if (0 == strcmp(s,  "yes")) return  true ;

    dod_exit("opt:not a bool:<%s>",s) ; abort() ;
}

static unsigned opt_ifChoiceVa(Opt *self,va_list *ap)
{
    if (opt_end(self))
	return 0 ;
    char const *s = opt_peek(self) ;
    unsigned i = 1 ;
    char const *t = va_arg(*ap,char const*) ;
    while (t != NULL)
    {
	if (0 == strcmp(s,t))
	{
	    va_end(*ap) ;
	    opt_pop(self) ;
	    return i ;
	}
	++i ;
	t = va_arg(*ap,char const*) ;
    }
    va_end(*ap) ;
    return 0 ;
}

unsigned opt_choice(Opt *self,...)
{
    va_list ap ;
    va_start(ap,self) ;
    unsigned i = opt_ifChoiceVa(self,&ap) ;
    if (i > 0)
      return (i-1) ;
    char buffer[0x100] = { '\0' } ;
    va_start(ap,self) ;
    char const *s = va_arg(ap,char const*);
    while (s != NULL)
    {
	size_t len = strlen(buffer) ;
	snprintf(buffer+len,sizeof(buffer)-len," <%s>",s) ;
	s = va_arg(ap,char const*) ;
    }
    if (strlen(buffer) == sizeof(buffer)-1)
	memcpy(buffer+sizeof(buffer)-4,"...",3) ;
    va_end(ap) ;
    dod_exit("option <%s> must be one of%s",opt_pop(self),buffer) ; abort() ;
}

unsigned opt_ifChoice(Opt *self,...)
{
    va_list ap ;
    va_start(ap,self) ;
    return opt_ifChoiceVa(self,&ap) ;
}

bool opt_end(Opt *self)
{
    return self->index == self->optN ;
}

void opt_finish(Opt *self)
{
    if (!opt_end(self))
	dod_exit("spurious option(s) at:<%s>",opt_peek(self)) ;
}

char const* opt_peek(Opt *self)
{
    if (opt_end(self))
	dod_exit("missing option") ;
    return self->optV[self->index] ;
}

char const* opt_pop(Opt *self)
{
    if (opt_end(self))
	dod_exit("missing option") ;
    return self->optV[self->index++] ;
}

bool opt_popIf(Opt *self,char const *arg)
{
    if (opt_end(self))
	return false ;
    char const *s = opt_peek(self) ;
    if (0 != strcmp(arg,s))
	return false ;
    opt_pop(self) ;
    return true ;
}

double opt_double(Opt *self)
{
    return dod_strtod(opt_pop(self)) ;
}

unsigned long long opt_llu(Opt *self)
{
    return dod_strtoull(opt_pop(self)) ;
}

uint16_t opt_uint16(Opt *self)
{
    long long unsigned i = dod_strtoull(opt_pop(self)) ;
    uint16_t j = (uint16_t)i ;
    if (i != j)
	dod_exit("16-bit unsigned integer too big:%llu",i) ;
    return j ;
}

char const *opt_preset(Opt *opt,char const *selector,char const *preset)
{
    return opt_popIf(opt,selector) ? opt_pop(opt) : preset ;
}
