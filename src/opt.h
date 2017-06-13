/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_opt_h
#define INCLUDE_opt_h

/* scan command line options */

#include <stdbool.h>
#include <stdint.h>

typedef struct 
{
    int   index ; /* current index to the option vector */
    int    optN ; /* number of options */
    char **optV ; /* vector with option strings */
}
Opt ;

/* init with main's arguments, e.g. opt_init(argc+1,argv-1) */
Opt opt_init(int argc,char *argv[]) ;

/* return true if no options are left */
bool opt_end(Opt *self) ;

/* die if any options are left */
void opt_finish(Opt *opt) ; 

/* return next option (die if there isn't one left) */
char const* opt_peek(Opt *opt) ;

/* pop and return next option (die if there isn't one left) */
char const* opt_pop(Opt *opt) ;

/* if arg matches: pop and return true  */
bool opt_popIf(Opt *opt,char const *arg) ;

/* pop bool (die if the next option doesn't match) */
bool opt_bool(Opt *opt) ;

/* pop double (die if the next option doesn't match) */
double opt_double(Opt *opt) ;

/* pop long long unsigned (die if the next option doesn't match) */
unsigned long long opt_llu(Opt *opt) ;

/* pop uint16_t (die if the next option doesn't match) */
uint16_t opt_uint16(Opt *opt) ;

/* pop next option (die if the next option doesn't match any argument) 
   e.g. opt_choice(opt,"eny","meeny","miny","moe",NULL) returns 0..3 */
unsigned opt_choice(Opt *opt,...) ;

/* similar to opt_choice, returns 0 if no argument matches though 
   e.g. opt_ifChoice(opt,"eny","meeny","miny","moe",NULL) returns 0..4 */
unsigned opt_ifChoice(Opt *opt,...) ;

/* return the option after the selector, or return the given preset 
   e.g. opt_preset(opt,"-i","100") checks if the next option is "-i",
   and if so returns the next option after; otherwise return "100" */
char const *opt_preset(Opt *opt,char const *selector,char const *preset) ;

#endif /* INCLUDE_opt_h */
