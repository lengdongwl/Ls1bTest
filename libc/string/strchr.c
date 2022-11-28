/*
 * strchr.c
 */

#include <string.h>

char *strchr(const char *s1, int i)
{
    const unsigned char *s = (const unsigned char *)s1;
    unsigned char c = i;

    while (*s && *s != c)
        s++;
    if (*s == c)
        return (char *)s;

    return NULL;
}

