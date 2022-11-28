/*
 * strcat.c
 */

#include <string.h>

char *strcat(char *s1, const char *s2)
{
    char *s = s1;

    while (*s1)
        s1++;

    while (*s1++ = *s2++)
        ;
    *s1 = '\0';

    return s;
}

