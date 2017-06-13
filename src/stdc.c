/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#include "stdc.h"
#include <limits.h>

/* make sure we have the right hardware */
#if CHAR_BIT != 8
#error "char must consist of 8 bits"
#endif

/* ISO C forbids an empty translation unit */
char const stdc_buildId[] = "build_id:" __DATE__ "." __TIME__ ;
