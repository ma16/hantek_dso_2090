/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

/* this implementation is based on POSIX.1 (not covered by C99 standard) */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#include <time.h>
#undef _POSIX_C_SOURCE
#else
#include <time.h>
#endif

#include "systime.h"
#include <errno.h>
#include <math.h>
#include <stdio.h> /* [debugging] */
#include <string.h>
#include "dod.h"

double systime_get()
{
    struct timespec spec ;
    int result = clock_gettime(CLOCK_MONOTONIC,&spec) ;
    if (result != 0)
	dod_exit("clock_gettime(CLOCK_MONOTONIC):%s",strerror(errno)) ;
    double t = 1e-9 * (double)spec.tv_nsec + (double)spec.tv_sec ;
    return t ;
}

void systime_sleep(double s)
{
    struct timespec spec ;
    double i = floor(s) ;
    spec.tv_sec = (time_t)i ;
    s -= i ;
    i = floor(1e+9 * s + .5) ;
    if (i >= 1e+9)
    {
	i -= 1.0 ;
	++spec.tv_sec ;
    }
    spec.tv_nsec = (long)i ;
    int result = nanosleep(&spec,NULL) ;
    if (result != 0)
	dod_exit("nanosleep():%s",strerror(errno)) ;
}
