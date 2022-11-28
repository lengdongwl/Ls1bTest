/*
 * malloc.c
 */

#include "bsp.h"

#if BSP_USE_LWMEM

#include <stdlib.h>
#include "../lwmem.h"

void *malloc(size_t size)
{
    return lwmem_malloc(size);
}

#endif

