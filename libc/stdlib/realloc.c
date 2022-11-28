/*
 * realloc.c
 */

#include "bsp.h"

#if BSP_USE_LWMEM

#include <stdlib.h>
#include "../lwmem.h"

void *realloc(void *ptr, size_t size)
{
    return lwmem_realloc(ptr, size);
}

#endif

