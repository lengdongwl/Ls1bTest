/*
 * strncpy.c
 */

#include <string.h>

char *strncpy(char *dst0, const char *src0, size_t count)
{
    char *dscan = dst0;
    const char *sscan = src0;

    while (count > 0)
    {
        --count;
        if ((*dscan++ = *sscan++) == '\0')
            break;
    }
    
    while (count-- > 0)
        *dscan++ = '\0';

    return dst0;
}

