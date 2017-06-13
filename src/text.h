/* BSD 2-Clause License, see github.com/ma16/hantek_dso_2090 */

#ifndef INCLUDE_text_h
#define INCLUDE_text_h

/* scan text */

#include <stdbool.h>
#include <stddef.h> /* size_t */

typedef struct 
{
    char const *buffer ; /* buffer given in init() */
    size_t      nbytes ; /* nbytes given in init() */
    char const      *p ; /* current buffer pointer */
    size_t         len ; /* length of found token */
    size_t         row ; /* number of passed \n +1 */
    size_t         col ; /* number of char since \n */
}
Text ;

/* scan bool (die if the next token doesn't match) */
bool text_bool(Text*) ;

/* skip over all white-spaces */
void text_clear(Text*) ;

/* scan next token (die if no argument matches)
   e.g. text_choice(text,"eny","meeny","miny","moe",NULL) returns 0..3 */
unsigned text_choice(Text*,...) ;

/* scan for double (die if there is no match) */
double text_double(Text*) ;

/* return true on end of buffer */
bool text_end(Text*) ;

/* die if not at end of buffer */
void text_finish(Text*) ;

/* similar to text_choice, returns 0 if no argument matches though 
   e.g. text_ifChoice(opt,"eny","meeny","miny","moe",NULL) returns 0..4 */
unsigned text_ifChoice(Text*,...) ;

/* init (the buffer must not be released by the caller) */
Text text_init(char const *buffer,size_t nbytes) ;

/* expect at least one white-space; skip all of them */
void text_space(Text*) ; 

/* scan for long long unsigned (die if there is no match) */
unsigned long long text_llu(Text*) ;

#endif /* INCLUDE_text_h */
