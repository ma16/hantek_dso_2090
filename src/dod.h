/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_dod_h
#define INCLUDE_dod_h

/* generic do-or-die wrappers */

#include "stdc.h"
#include <stddef.h> /* size_t */

void dod_exit(char const *format,...) STDC_FORMAT_PRINTF(1,2) ;

void* dod_malloc(size_t nbytes) ;

void dod_snprintf(char *p,size_t n,char const *format,...) STDC_FORMAT_PRINTF(3,4) ;

double dod_strtod(char const *s) ;

unsigned long long dod_strtoull(char const *s) ;

size_t dod_loadFile(char const *path,char *buffer,size_t length) ;
/* ...loads the file into the buffer */

#endif /* INCLUDE_dod_h */
