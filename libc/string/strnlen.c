/*
 * strnlen.c
 */

#include <string.h>

size_t strnlen(const char *str, size_t n)
{
    const char *start = str;

    while (n-- > 0 && *str)
        str++;

    return str - start;
}

