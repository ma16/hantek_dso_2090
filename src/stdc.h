/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_stdc_h
#define INCLUDE_stdc_h

/* make sure we use the right compiler; 
   use compiler-specific features in a generic way */

#if __STDC__ != 1
#error "Not a standard-compliant compiler"
#endif

#ifdef __GNUC__
#define STDC_FORMAT_PRINTF(i,j) __attribute__ ((format (printf,i,j)))
#else
#define STDC_FORMAT_PRINTF(i,j)
#endif

#endif /* INCLUDE_stdc_h */
