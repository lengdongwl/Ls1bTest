/*
 * memcpy.c
 */

#include <string.h>

void *memcpy(void *dst0, const void *src0, size_t len0)
{
    char *dst = (char *)dst0;
    char *src = (char *)src0;

    while (len0--)
    {
        *dst++ = *src++;
    }

    return dst0;
}

