/*
 * atoi.c
 */

#include <stdlib.h>

int atoi(const char *s)
{
    return (int) strtol(s, NULL, 10);
}

