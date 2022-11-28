/*
 * memset.c
 */

#include <string.h>

void *memset(void *m, int c, size_t n)
{
    char *s = (char *)m;
    while (n--)
        *s++ = (char) c;

    return m;
}

