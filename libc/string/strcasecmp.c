/*
 * strcasecmp.c
 */

#include <string.h>
#include <ctype.h>

int strcasecmp(const char *s1, const char *s2)
{
    const unsigned char *ucs1 = (const unsigned char *) s1;
    const unsigned char *ucs2 = (const unsigned char *) s2;
    int d = 0;

    for ( ; ; )
    {
        const int c1 = tolower(*ucs1++);
        const int c2 = tolower(*ucs2++);
        if (((d = c1 - c2) != 0) || (c2 == '\0'))
            break;
    }
    
    return d;
}

