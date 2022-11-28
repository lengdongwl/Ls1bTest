/*
 * free.c
 */

#include "bsp.h"

#if BSP_USE_LWMEM

#include <stdlib.h>
#include "../lwmem.h"

void free(void *ptr)
{
    lwmem_free(ptr);
}

#endif

