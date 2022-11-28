/*
 * strdup.c
 */

#include <string.h>
#include <stdlib.h>

char *strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = (char *)malloc(len);

    if (copy)
    {
        memcpy(copy, s, len);
    }

    return copy;
}

