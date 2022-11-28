/*
 * errno.c
 */

#include <errno.h>

static int _errno;

int *__errno(void)
{
    return &_errno;
}

