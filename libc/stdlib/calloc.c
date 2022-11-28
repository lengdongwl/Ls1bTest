/*
 * calloc.c
 */

#include "bsp.h"

#if BSP_USE_LWMEM

#include <stdlib.h>
#include "../lwmem.h"

void *calloc(size_t nitems, size_t size)
{
    return lwmem_calloc(nitems, size);
}

#endif

