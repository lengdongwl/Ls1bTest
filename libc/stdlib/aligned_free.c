/*
 * aligned_free.c
 */

#include <stdlib.h>

void aligned_free(void *addr)
{
    void *ptr = ((void **)addr)[-1];
    free(ptr);
}
