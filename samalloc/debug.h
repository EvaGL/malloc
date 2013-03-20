#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "config.h"
#if MALLOC_DEBUG == 1
    #include <stdio.h>
    static __inline__ void writeForm (const char * format, ...)
    {
        va_list args;
        va_start (args, format);
        vfprintf (stderr, format, args);
        va_end (args);
    }

    #define MDEBUG(str, args...) printf(str, ## args)
#else
    #define MDEBUG(str, args...) (void) 0
#endif

#endif
