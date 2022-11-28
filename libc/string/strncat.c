/*
 * strncat.c
 */

#include <string.h>

char* strncat(char *s1, const char *s2, size_t n)
{
    char *s = s1;

    while (*s1)
        s1++;
        
    while (n-- != 0 && (*s1++ = *s2++))
    {
        if (n == 0)
            *s1 = '\0';
    }

    return s;
}

