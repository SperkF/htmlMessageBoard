#ifndef _ERR_MACRO_H
#define _ERR_MACRO_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <string.h> // strerror()

/*ERROR handling macros
 * we use a macro and not afunction to be able to print out line numer
 * macro uses conditional operator //(condition) ? (True-action) : (False-action)//
 * explanation to use of curly brackets: https://www.geeksforgeeks.org/multiline-macros-in-c/
 */
#define CHECK_EQUAL(X,Y) \
    ({ int vara = (X); int varb = (Y); vara == varb ? \
    ({ fprintf(stderr,"ERROR (file: "__FILE__" / line:%d) -- %s\n",__LINE__, strerror(errno));\
    exit(EXIT_FAILURE);}) : (X); })

#define CHECK_NONEQUAL(X,Y) \
    ({ int vara = (X); int varb = (Y); vara != varb ? \
    ({ fprintf(stderr,"ERROR (file: "__FILE__" / line:%d) -- %s\n",__LINE__, strerror(errno));\
    exit(EXIT_FAILURE);}) : (X); })

//a failed syscall will always return -1 (only few exceptions do otherwise)
#define CHECK_SYSCALL(X,Y) \
    ({ int vara = (X); vara == -1 ? \
    ({ fprintf(stderr,"ERROR (file: "__FILE__" / line:%d) -- %s\n",__LINE__, strerror(errno));\
    exit(EXIT_FAILURE);}) : (X); })

// if value of X is less than value of Y ->error
#define CHECK_LESS(X,Y) \
    ({ int vara = (X); int varb = (Y); vara < varb ? \
    ({ fprintf(stderr,"ERROR (file: "__FILE__" / line:%d) -- %s\n",__LINE__, strerror(errno));\
    exit(EXIT_FAILURE);}) : (X); })

// just write out error code and terminate program
//create as inline function??
#define ERR_EXIT() \
    ({ fprintf(stderr,"ERROR (file: "__FILE__" / line:%d) -- %s\n",__LINE__, strerror(errno));\
    exit(EXIT_FAILURE);})


#endif // _ERR_MACRO_H
