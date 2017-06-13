/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_systime_h
#define INCLUDE_systime_h

/* current time in seconds since epoch */
double systime_get(void) ;

/* sleep seconds */
void systime_sleep(double) ;

#endif /* INCLUDE_systime_h */
