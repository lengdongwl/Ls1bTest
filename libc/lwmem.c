/*
 * file: lwmem.c
 * Lightweight dynamic memory manager
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwMEM - Lightweight dynamic memory manager library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v1.3.0
 */

#include "bsp.h"

#if (BSP_USE_LWMEM)

#include <limits.h>

#include "lwmem.h"

#if defined(OS_RTTHREAD)
#include "rtthread.h"
static rt_mutex_t  lwmem_mutex;
#define LOCK(lw)   do { rt_mutex_take(lwmem_mutex, RT_WAITING_FOREVER); } while (0)
#define UNLOCK(lw) do { rt_mutex_release(lwmem_mutex); } while (0)

#elif defined(OS_UCOS)
#include "os.h"
static OS_EVENT         *lwmem_mutex;
#define LWMEM_MUTEX_PRIO 5
#define LOCK(lw)   do { unsigned char err; if (OSRunning == OS_TRUE) OSMutexPend(lwmem_mutex, ~0, &err); } while (0)
#define UNLOCK(lw) do { if (OSRunning == OS_TRUE) OSMutexPost(lwmem_mutex); } while (0)

#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
static SemaphoreHandle_t lwmem_mutex;
#define LOCK(lw)   do { xSemaphoreTake(lwmem_mutex, portMAX_DELAY); } while (0)
#define UNLOCK(lw) do { xSemaphoreGive(lwmem_mutex); } while (0)

#else
#define LOCK(lw)
#define UNLOCK(lw)

#endif

#define LWMEM_MEMSET        memset
#define LWMEM_MEMCPY        memcpy
#define LWMEM_MEMMOVE       memmove

/*
 * Transform alignment number (power of `2`) to bits
 */
#define LWMEM_ALIGN_BITS                ((size_t)(((size_t)LWMEM_CFG_ALIGN_NUM) - 1))

/*
 * Aligns input value to next alignment bits
 *
 * As an example, when \ref LWMEM_CFG_ALIGN_NUM is set to `4`:
 *
 *  - Input: `0`; Output: `0`
 *  - Input: `1`; Output: `4`
 *  - Input: `2`; Output: `4`
 *  - Input: `3`; Output: `4`
 *  - Input: `4`; Output: `4`
 *  - Input: `5`; Output: `8`
 *  - Input: `6`; Output: `8`
 *  - Input: `7`; Output: `8`
 *  - Input: `8`; Output: `8`
 */
#define LWMEM_ALIGN(x)                  (((x) + (LWMEM_ALIGN_BITS)) & ~(LWMEM_ALIGN_BITS))

/*
 * Size of metadata header for block information
 */
#define LWMEM_BLOCK_META_SIZE           LWMEM_ALIGN(sizeof(lwmem_block_t))

/*
 * Cast input pointer to byte
 * param[in]    p: Input pointer to cast to byte pointer
 */
#define LWMEM_TO_BYTE_PTR(p)            ((unsigned char *)(p))

/*
 * Bit indicating memory block is allocated
 */
#define LWMEM_ALLOC_BIT                 ((size_t)((size_t)1 << (sizeof(size_t) * CHAR_BIT - 1)))

/*
 * Mark written in `next` field when block is allocated
 */
#define LWMEM_BLOCK_ALLOC_MARK          (0xDEADBEEF)

/*
 * Set block as allocated
 * param[in]    block: Block to set as allocated
 */
#define LWMEM_BLOCK_SET_ALLOC(block)    do { \
    if ((block) != NULL) { \
        (block)->size |= LWMEM_ALLOC_BIT; \
        (block)->next = (void *)(LWMEM_TO_BYTE_PTR(0) + LWMEM_BLOCK_ALLOC_MARK); \
    }} while (0)

/*
 * Check if input block is properly allocated and valid
 * param[in]    block: Block to check if properly set as allocated
 */
#define LWMEM_BLOCK_IS_ALLOC(block)     ((block) != NULL && \
                                        ((block)->size & LWMEM_ALLOC_BIT) && \
                                         (block)->next == (void *)(LWMEM_TO_BYTE_PTR(0) + LWMEM_BLOCK_ALLOC_MARK))

/*
 * Get block handle from application pointer
 * param[in]    ptr: Input pointer to get block from
 */
#define LWMEM_GET_BLOCK_FROM_PTR(ptr)   (void *)((ptr) != NULL ? ((LWMEM_TO_BYTE_PTR(ptr)) - LWMEM_BLOCK_META_SIZE) : NULL)

/*
 * Get block handle from application pointer
 * param[in]    block: Input pointer to get block from
 */
#define LWMEM_GET_PTR_FROM_BLOCK(block) (void *)((block) != NULL ? ((LWMEM_TO_BYTE_PTR(block)) + LWMEM_BLOCK_META_SIZE) : NULL)

/*
 * Minimum amount of memory required to make new empty block
 *
 * Default size is size of meta block
 */
#define LWMEM_BLOCK_MIN_SIZE            (LWMEM_BLOCK_META_SIZE)

/*
 * Get LwMEM instance based on user input
 * param[in]    in_lw: LwMEM instance. Set to `NULL` for default instance
 */
#define LWMEM_GET_LW(in_lw)             ((in_lw) != NULL ? (in_lw) : (&lwmem_default))

/*
 * Gets block before input block (marked as prev) and its previous free block
 * param[in]    in_lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    in_b: Input block to find previous and its previous
 * param[in]    in_pp: Previous previous of input block
 * param[in]    in_p: Previous of input block
 */
#define LWMEM_GET_PREV_CURR_OF_BLOCK(in_lw, in_b, in_pp, in_p) do {     \
    for ((in_pp) = NULL, (in_p) = &(LWMEM_GET_LW(in_lw)->start_block);  \
        (in_p) != NULL && (in_p)->next < (in_b);                        \
        (in_pp) = (in_p), (in_p) = (in_p)->next                         \
    ) {}                                                                \
} while (0)

/*
 * LwMEM default structure used by application
 */
static lwmem_t lwmem_default;

/*
 * Get region aligned start address and aligned size
 * param[in]    region: Region to check for size and address
 * param[out]   msa: Memory start address output variable
 * param[out]   ms: Memory size output variable
 * return       `1` if region valid, `0` otherwise
 */
static unsigned char
prv_get_region_addr_size(const lwmem_region_t* region, unsigned char** msa, size_t* ms)
{
    size_t mem_size;
    unsigned char* mem_start_addr;

    if (region == NULL || msa == NULL || ms == NULL)
    {
        return 0;
    }

    *msa = NULL;
    *ms = 0;

    /* Check region size and align it to config bits */
    mem_size = region->size & ~LWMEM_ALIGN_BITS;/* Size does not include lower bits */
    if (mem_size < (2 * LWMEM_BLOCK_MIN_SIZE))
    {
        return 0;
    }

    /*
     * Start address must be aligned to configuration
     * Increase start address and decrease effective region size
     */
    mem_start_addr = region->start_addr;
    if (((size_t)mem_start_addr) & LWMEM_ALIGN_BITS)   /* Check alignment boundary */
    {
        mem_start_addr += ((size_t)LWMEM_CFG_ALIGN_NUM) - ((size_t)mem_start_addr & LWMEM_ALIGN_BITS);
        mem_size -= (size_t)(mem_start_addr - LWMEM_TO_BYTE_PTR(region->start_addr));
    }

    /* Check final memory size */
    if (mem_size >= (2 * LWMEM_BLOCK_MIN_SIZE))
    {
        *msa = mem_start_addr;
        *ms = mem_size;

        return 1;
    }

    return 0;
}

/*
 * Insert free block to linked list of free blocks
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    nb: New free block to insert into linked list
 */
static void
prv_insert_free_block(lwmem_t* const lw, lwmem_block_t* nb)
{
    lwmem_block_t* prev;

    /*
     * Try to find position to put new block in-between
     * Search until all free block addresses are lower than entry block
     */
    for (prev = &(LWMEM_GET_LW(lw)->start_block); prev != NULL && prev->next < nb; prev = prev->next)
    {
        //
    }

    /* This is hard error with wrong memory usage */
    if (prev == NULL)
    {
        return;
    }

    /*
     * At this point we have valid previous block
     * Previous block is last free block before input block
     */

    /*
     * Check if previous block and input block together create one big contiguous block
     * If this is the case, merge blocks together and increase previous block by input block size
     */
    if ((LWMEM_TO_BYTE_PTR(prev) + prev->size) == LWMEM_TO_BYTE_PTR(nb))
    {
        prev->size += nb->size;                 /* Increase current block by size of new block */
        nb = prev;                              /* New block and current are now the same thing */
        /*
         * It is important to set new block as current one
         * as this allows merging previous and next blocks together with new block
         * at the same time; follow next steps
         */
    }

    /*
     * Check if new block and next of previous create big contiguous block
     * Do not merge with "end of region" indication (commented part of if statement)
     */
    if (prev->next != NULL && prev->next->size > 0      /* Do not remove "end of region" indicator in each region */
        && (LWMEM_TO_BYTE_PTR(nb) + nb->size) == LWMEM_TO_BYTE_PTR(prev->next))
    {
        if (prev->next == LWMEM_GET_LW(lw)->end_block)  /* Does it points to the end? */
        {
            nb->next = LWMEM_GET_LW(lw)->end_block;     /* Set end block pointer */
        }
        else
        {
            nb->size += prev->next->size;       /* Expand of current block for size of next free block which is right behind new block */
            nb->next = prev->next->next;        /* Next free is pointed to the next one of previous next */
        }
    }
    else
    {
        nb->next = prev->next;                  /* Set next of input block as next of current one */
    }

    /*
     * If new block has not been set as current (and expanded),
     * then link them together, otherwise ignore as it would point to itself
     */
    if (prev != nb)
    {
        prev->next = nb;
    }
}

/*
 * Split too big block and add it to list of free blocks
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    block: Pointer to block with size already set
 * param[in]    new_block_size: New block size to be set
 * return       `1` if block splitted, `0` otherwise
 */
static unsigned char
prv_split_too_big_block(lwmem_t* const lw, lwmem_block_t* block, size_t new_block_size)
{
    lwmem_block_t* next;
    size_t block_size, is_alloc_bit;
    unsigned char success = 0;

    is_alloc_bit = block->size & LWMEM_ALLOC_BIT;   /* Check if allocation bit is set */
    block_size = block->size & ~LWMEM_ALLOC_BIT;    /* Use size without allocated bit */

    /*
     * If current block size is greater than requested size,
     * it is possible to create empty block at the end of existing one
     * and add it back to list of empty blocks
     */
    if ((block_size - new_block_size) >= LWMEM_BLOCK_MIN_SIZE)
    {
        next = (void *)(LWMEM_TO_BYTE_PTR(block) + new_block_size); /* Put next block after size of current allocation */
        next->size = block_size - new_block_size;                   /* Modify block data */
        block->size = new_block_size;                               /* Current size is now smaller */

        LWMEM_GET_LW(lw)->mem_available_bytes += next->size;        /* Increase available bytes by new block size */
        prv_insert_free_block(lw, next);                            /* Add new block to the free list */

        success = 1;
    }
    else
    {
        /* TODO: If next of current is free, check if we can increase next by at least some bytes */
        /* This can only happen during reallocation process when allocated block is reallocated to previous one */
        /* Very rare case, but may happen! */
    }

    /* If allocation bit was set before, set it now again */
    if (is_alloc_bit)
    {
        LWMEM_BLOCK_SET_ALLOC(block);
    }

    return success;
}

/*
 * Private allocation function
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    region: Pointer to region to allocate from.
 *                      Set to `NULL` for any region
 * param[in]    size: Application wanted size, excluding size of meta header
 * return       Pointer to allocated memory, `NULL` otherwise
 */
static void *
prv_alloc(lwmem_t* const lw, const lwmem_region_t* region, const size_t size)
{
    lwmem_block_t* prev, *curr;
    void* retval = NULL;

    /* Calculate final size including meta data size */
    const size_t final_size = LWMEM_ALIGN(size) + LWMEM_BLOCK_META_SIZE;

    /* Check if initialized and if size is in the limits */
    if (LWMEM_GET_LW(lw)->end_block == NULL || final_size == LWMEM_BLOCK_META_SIZE || (final_size & LWMEM_ALLOC_BIT) > 0)
    {
        return NULL;
    }

    /* Set default values */
    prev = &(LWMEM_GET_LW(lw)->start_block);    /* Use pointer from custom lwmem block */
    curr = prev->next;                          /* Curr represents first actual free block */

    /*
     * If region is not set to NULL,
     * request for memory allocation came from specific region:
     *
     * - Start at the beginning like normal (from very first region)
     * - Loop until free block is between region start addr and its size
     */
    if (region != NULL)
    {
        unsigned char* region_start_addr;
        size_t region_size;

        /* Get data about region */
        if (!prv_get_region_addr_size(region, &region_start_addr, &region_size))
        {
            return NULL;
        }

        /*
         * Scan all regions from lwmem and find first available block
         * which is within address of region and is big enough
         */
        for (; curr != NULL; prev = curr, curr = curr->next)
        {
            /* Check bounds */
            if (curr->next == NULL || curr == LWMEM_GET_LW(lw)->end_block)
            {
                return NULL;
            }
            if ((unsigned char*)curr < (unsigned char *)region_start_addr)      /* Check if we reached region */
            {
                continue;
            }
            if ((unsigned char*)curr >= (unsigned char *)(region_start_addr + region_size)) /* Check if we are out already */
            {
                return NULL;
            }
            if (curr->size >= final_size)
            {
                break;                          /* Free block identified */
            }
        }
    }
    else
    {
        /*
         * Try to find first block with at least `size` bytes of available memory
         * Loop until size of current block is smaller than requested final size
         */
        for (; curr != NULL && curr->size < final_size; prev = curr, curr = curr->next)
        {
            if (curr->next == NULL || curr == LWMEM_GET_LW(lw)->end_block)  /* If no more blocks available */
            {
                return NULL;                    /* No sufficient memory available to allocate block of memory */
            }
        }
    }

    /* Check curr pointer. During normal use, this should be always false */
    if (curr == NULL)
    {
        return NULL;
    }

    /* There is a valid block available */
    retval = LWMEM_GET_PTR_FROM_BLOCK(curr);    /* Return pointer does not include meta part */
    prev->next = curr->next;                    /* Remove this block from linked list by setting next of previous to next of current */

    /* curr block is now removed from linked list */

    LWMEM_GET_LW(lw)->mem_available_bytes -= curr->size;    /* Decrease available bytes by allocated block size */
    prv_split_too_big_block(lw, curr, final_size);          /* Split block if it is too big */
    LWMEM_BLOCK_SET_ALLOC(curr);                            /* Set block as allocated */

    return retval;
}

/*
 * Free input pointer
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    ptr: Input pointer to free
 */
static void
prv_free(lwmem_t* const lw, void* const ptr)
{
    lwmem_block_t* const block = LWMEM_GET_BLOCK_FROM_PTR(ptr);
    if (LWMEM_BLOCK_IS_ALLOC(block))                            /* Check if block is valid */
    {
        block->size &= ~LWMEM_ALLOC_BIT;                        /* Clear allocated bit indication */

        LWMEM_GET_LW(lw)->mem_available_bytes += block->size;   /* Increase available bytes */
        prv_insert_free_block(lw, block);                       /* Put block back to list of free block */
    }
}

/*
 * Reallocates already allocated memory with new size
 *
 * Function behaves differently, depends on input parameter of `ptr` and `size`:
 *
 *  - `ptr == NULL; size == 0`: Function returns `NULL`, no memory is allocated or freed
 *  - `ptr == NULL; size > 0`: Function tries to allocate new block of memory with `size` length, equivalent to `malloc(size)`
 *  - `ptr != NULL; size == 0`: Function frees memory, equivalent to `free(ptr)`
 *  - `ptr != NULL; size > 0`: Function tries to allocate new memory of copy content before returning pointer on success
 *
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    region: Pointer to region to allocate from.
 *                      Set to `NULL` for any region
 * param[in]    ptr: Memory block previously allocated with one of allocation functions.
 *                   It may be set to `NULL` to create new clean allocation
 * param[in]    size: Size of new memory to reallocate
 * return       Pointer to allocated memory on success, `NULL` otherwise
 */
void *
prv_realloc(lwmem_t* const lw, const lwmem_region_t* region, void* const ptr, const size_t size)
{
    lwmem_block_t* block, *prevprev, *prev;
    size_t block_size;                                                      /* Holds size of input block (ptr), including metadata size */
    const size_t final_size = LWMEM_ALIGN(size) + LWMEM_BLOCK_META_SIZE;    /* Holds size of new requested block size, including metadata size */
    void* retval;                                                           /* Return pointer, used with LWMEM_RETURN macro */

    /* Check optional input parameters */
    if (size == 0)
    {
        if (ptr != NULL)
        {
            prv_free(lw, ptr);
        }
        return NULL;
    }
    if (ptr == NULL)
    {
        return prv_alloc(lw, NULL, size);
    }

    /* Try to reallocate existing pointer */
    if ((size & LWMEM_ALLOC_BIT) || (final_size & LWMEM_ALLOC_BIT))
    {
        return NULL;
    }

    /* Process existing block */
    block = LWMEM_GET_BLOCK_FROM_PTR(ptr);
    if (LWMEM_BLOCK_IS_ALLOC(block))
    {
        block_size = block->size & ~LWMEM_ALLOC_BIT;    /* Get actual block size, without memory allocation bit */

        /* Check current block size is the same as new requested size */
        if (block_size == final_size)
        {
            return ptr;                                 /* Just return pointer, nothing to do */
        }

        /*
         * Abbreviations
         *
         * - "Current block" or "Input block" is allocated block from input variable "ptr"
         * - "Next free block" is next free block, on address space after input block
         * - "Prev free block" is last free block, on address space before input block
         * - "PrevPrev free block" is previous free block of "Prev free block"
         */

        /*
         * When new requested size is smaller than existing one,
         * it is enough to modify size of current block only.
         *
         * If new requested size is much smaller than existing one,
         * check if it is possible to create new empty block and add it to list of empty blocks
         *
         * Application returns same pointer
         */
        if (final_size < block_size)
        {
            if ((block_size - final_size) >= LWMEM_BLOCK_MIN_SIZE)
            {
                prv_split_too_big_block(lw, block, final_size); /* Split block if it is too big */
            }
            else
            {
                /*
                 * It is not possible to create new empty block at the end of input block
                 *
                 * But if next free block is just after input block,
                 * it is possible to find this block and increase it by "block_size - final_size" bytes
                 */

                /* Find free blocks before input block */
                LWMEM_GET_PREV_CURR_OF_BLOCK(lw, block, prevprev, prev);

                /* Check if current block and next free are connected */
                if ((LWMEM_TO_BYTE_PTR(block) + block_size) == LWMEM_TO_BYTE_PTR(prev->next)
                    && prev->next->size > 0)    /* Must not be end of region indicator */
                {
                    /* Make temporary variables as prev->next will point to different location */
                    const size_t tmp_size = prev->next->size;
                    void* const tmp_next = prev->next->next;

                    /* Shift block up, effectively increase its size */
                    prev->next = (void *)(LWMEM_TO_BYTE_PTR(prev->next) - (block_size - final_size));
                    prev->next->size = tmp_size + (block_size - final_size);
                    prev->next->next = tmp_next;
                    LWMEM_GET_LW(lw)->mem_available_bytes += block_size - final_size;   /* Increase available bytes by increase of free block */

                    block->size = final_size;   /* Block size is requested size */
                }
            }

            LWMEM_BLOCK_SET_ALLOC(block);       /* Set block as allocated */
            return ptr;                         /* Return existing pointer */
        }

        /* New requested size is bigger than current block size is */

        /* Find last free (and its previous) block, located just before input block */
        LWMEM_GET_PREV_CURR_OF_BLOCK(lw, block, prevprev, prev);

        /* If entry could not be found, there is a hard error */
        if (prev == NULL)
        {
            return NULL;
        }

        /* Order of variables is: | prevprev ---> prev --->--->--->--->--->--->--->--->--->---> prev->next  | */
        /*                        |                      (input_block, which is not on a list)              | */
        /* Input block points to address somewhere between "prev" and "prev->next" pointers                   */

        /* Check if "block" and next free "prev->next" create contiguous memory with size of at least new requested size */
        if ((LWMEM_TO_BYTE_PTR(block) + block_size) == LWMEM_TO_BYTE_PTR(prev->next)/* Blocks create contiguous block */
            && (block_size + prev->next->size) >= final_size)   /* Size is greater or equal to requested */
        {
            /*
             * Merge blocks together by increasing current block with size of next free one
             * and remove next free from list of free blocks
             */
            LWMEM_GET_LW(lw)->mem_available_bytes -= prev->next->size;  /* For now decrease effective available bytes */
            block->size = block_size + prev->next->size;                /* Increase effective size of new block */
            prev->next = prev->next->next;                              /* Set next to next's next, effectively remove
                                                                           expanded block from free list */

            prv_split_too_big_block(lw, block, final_size);             /* Split block if it is too big */
            LWMEM_BLOCK_SET_ALLOC(block);                               /* Set block as allocated */
            return ptr;                                                 /* Return existing pointer */
        }

        /*
         * Check if "block" and last free before "prev" create contiguous memory with size of at least new requested size.
         *
         * It is necessary to make a memory move and shift content up as new return pointer is now upper on address space
         */
        if ((LWMEM_TO_BYTE_PTR(prev) + prev->size) == LWMEM_TO_BYTE_PTR(block)  /* Blocks create contiguous block */
            && (prev->size + block_size) >= final_size)                         /* Size is greater or equal to requested */
        {
            /* Move memory from block to block previous to current */
            void* const old_data_ptr = LWMEM_GET_PTR_FROM_BLOCK(block);
            void* const new_data_ptr = LWMEM_GET_PTR_FROM_BLOCK(prev);

            /*
             * If memmove overwrites metadata of current block (when shifting content up),
             * it is not an issue as we know its size (block_size) and next is already NULL.
             *
             * Memmove must be used to guarantee move of data as addresses + their sizes may overlap
             *
             * Metadata of "prev" are not modified during memmove
             */
            LWMEM_MEMMOVE(new_data_ptr, old_data_ptr, block_size);

            LWMEM_GET_LW(lw)->mem_available_bytes -= prev->size;    /* For now decrease effective available bytes */
            prev->size += block_size;                               /* Increase size of input block size */
            prevprev->next = prev->next;                            /* Remove prev from free list as it is now being used
                                                                       for allocation together with existing block */
            block = prev;                                           /* Move block pointer to previous one */

            prv_split_too_big_block(lw, block, final_size);         /* Split block if it is too big */
            LWMEM_BLOCK_SET_ALLOC(block);                           /* Set block as allocated */
            return new_data_ptr;                                    /* Return new data ptr */
        }

        /*
         * At this point, it was not possible to expand existing block with free before or free after due to:
         * - Input block & next free block do not create contiguous block or its new size is too small
         * - Previous free block & input block do not create contiguous block or its new size is too small
         *
         * Last option is to check if previous free block "prev", input block "block" and next free block "prev->next" create contiguous block
         * and size of new block (from 3 contiguous blocks) together is big enough
         */
        if ((LWMEM_TO_BYTE_PTR(prev) + prev->size) == LWMEM_TO_BYTE_PTR(block)          /* Input block and free block before create contiguous block */
            && (LWMEM_TO_BYTE_PTR(block) + block_size) == LWMEM_TO_BYTE_PTR(prev->next) /* Input block and free block after create contiguous block */
            && (prev->size + block_size + prev->next->size) >= final_size)              /* Size is greater or equal to requested */
        {
            /* Move memory from block to block previous to current */
            void* const old_data_ptr = LWMEM_GET_PTR_FROM_BLOCK(block);
            void* const new_data_ptr = LWMEM_GET_PTR_FROM_BLOCK(prev);

            /*
             * If memmove overwrites metadata of current block (when shifting content up),
             * it is not an issue as we know its size (block_size) and next is already NULL.
             *
             * Memmove must be used to guarantee move of data as addresses and their sizes may overlap
             *
             * Metadata of "prev" are not modified during memmove
             */
            LWMEM_MEMMOVE(new_data_ptr, old_data_ptr, block_size);  /* Copy old buffer size to new location */

            LWMEM_GET_LW(lw)->mem_available_bytes -= prev->size + prev->next->size; /* Decrease effective available bytes for free blocks
                                                                                       before and after input block */
            prev->size += block_size + prev->next->size;                            /* Increase size of new block by size of 2 free blocks */
            prevprev->next = prev->next->next;                                      /* Remove free block before current one and block after
                                                                                       current one from linked list (remove 2) */
            block = prev;                                                           /* Previous block is now current */

            prv_split_too_big_block(lw, block, final_size);     /* Split block if it is too big */
            LWMEM_BLOCK_SET_ALLOC(block);                       /* Set block as allocated */
            return new_data_ptr;                                /* Return new data ptr */
        }
    }
    else
    {
        /* Hard error. Input pointer is not NULL and block is not considered allocated */
        return NULL;
    }

    /*
     * If application reaches this point, it means:
     * - New requested size is greater than input block size
     * - Input block & next free block do not create contiguous block or its new size is too small
     * - Last free block & input block do not create contiguous block or its new size is too small
     * - Last free block & input block & next free block do not create contiguous block or its size is too small
     *
     * Final solution is to find completely new empty block of sufficient size and copy content from old one to new one
     */
    retval = prv_alloc(lw, NULL, size);         /* Try to allocate new block */
    if (retval != NULL)
    {
        block_size = (block->size & ~LWMEM_ALLOC_BIT) - LWMEM_BLOCK_META_SIZE;  /* Get application size from input pointer */
        LWMEM_MEMCPY(retval, ptr, size > block_size ? block_size : size);       /* Copy content to new allocated block */
        prv_free(lw, ptr);                                                      /* Free input pointer */
    }

    return retval;
}

/*
 * Initializes and assigns user regions for memory used by allocator algorithm
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    regions: Array of regions with address and its size.
 *                       Regions must be in increasing order (start address) and must not overlap in-between
 * param[in]    len: Number of regions in array
 * return       `0` on failure, number of final regions used for memory manager on success
 * note         This function is not thread safe when used with operating system.
 *              It must be called only once to setup memory regions
 */
size_t
lwmem_assignmem_ex(lwmem_t* const lw, const lwmem_region_t* regions, const size_t len)
{
    unsigned char* mem_start_addr;
    size_t i, mem_size;
    lwmem_block_t* first_block, *prev_end_block;

    if (LWMEM_GET_LW(lw)->end_block != NULL     /* Init function may only be called once per lwmem instance */
        || ((((size_t)LWMEM_CFG_ALIGN_NUM) & (((size_t)LWMEM_CFG_ALIGN_NUM) - 1)) > 0)  /* Must be power of 2 */
        || regions == NULL || len == 0
#if LWMEM_CFG_OS
        || lwmem_sys_mutex_isvalid(&(LWMEM_GET_LW(lw)->mutex))  /* Check if mutex valid already */
#endif /* LWMEM_CFG_OS */
        )       /* Check inputs */
    {
        return 0;
    }

#if LWMEM_CFG_OS
    if (!lwmem_sys_mutex_create(&(LWMEM_GET_LW(lw)->mutex)))
    {
        return 0;
    }
#endif /* LWMEM_CFG_OS */

    /* Ensure regions are growing linearly and do not overlap in between */
    mem_start_addr = (void *)0;
    mem_size = 0;
    for (i = 0; i < len; ++i)
    {
        /* New region(s) must be higher (in address space) than previous one */
        if ((mem_start_addr + mem_size) > LWMEM_TO_BYTE_PTR(regions[i].start_addr))
        {
            return 0;
        }

        /* Save new values for next try */
        mem_start_addr = regions[i].start_addr;
        mem_size = regions[i].size;
    }

    for (i = 0; i < len; ++i, ++regions)
    {
        /* Get region start address and size */
        if (!prv_get_region_addr_size(regions, &mem_start_addr, &mem_size))
        {
            continue;
        }

        /*
         * If end_block == NULL, this indicates first iteration.
         * In first indication application shall set start_block and never again
         * end_block value holds
         */
        if (LWMEM_GET_LW(lw)->end_block == NULL)
        {
            /*
             * Next entry of start block is first region
             * It points to beginning of region data
             * In the later step(s) first block is manually set on top of memory region
             */
            LWMEM_GET_LW(lw)->start_block.next = (void *)mem_start_addr;
            LWMEM_GET_LW(lw)->start_block.size = 0;   /* Size of dummy start block is zero */
        }

        /* Save current end block status as it is used later for linked list insertion */
        prev_end_block = LWMEM_GET_LW(lw)->end_block;

        /* Put end block to the end of the region with size = 0 */
        LWMEM_GET_LW(lw)->end_block = (void *)(mem_start_addr + mem_size - LWMEM_BLOCK_META_SIZE);
        LWMEM_GET_LW(lw)->end_block->next = NULL;   /* End block in region does not have next entry */
        LWMEM_GET_LW(lw)->end_block->size = 0;      /* Size of end block is zero */

        /*
         * Create memory region first block.
         *
         * First block meta size includes size of metadata too
         * Subtract MEM_BLOCK_META_SIZE as there is one more block (end_block) at the end of region
         *
         * Actual maximal available size for application in the region is mem_size - 2 * MEM_BLOCK_META_SIZE
         */
        first_block = (void *)mem_start_addr;
        first_block->next = LWMEM_GET_LW(lw)->end_block;            /* Next block of first is last block */
        first_block->size = mem_size - LWMEM_BLOCK_META_SIZE;

        /* Check if previous regions exist by checking previous end block state */
        if (prev_end_block != NULL)
        {
            prev_end_block->next = first_block;                     /* End block of previous region now points to start of current region */
        }

        LWMEM_GET_LW(lw)->mem_available_bytes += first_block->size; /* Increase number of available bytes */
        ++LWMEM_GET_LW(lw)->mem_regions_count;                      /* Increase number of used regions */
    }

    return LWMEM_GET_LW(lw)->mem_regions_count;                     /* Return number of regions used by manager */
}

/*
 * Allocate memory of requested size in specific lwmem instance and optional region.
 * This is an extended malloc version function declaration to support advanced features
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    region: Optional region instance within LwMEM instance to force allocation from.
 *                      Set to `NULL` to use any region within LwMEM instance
 * param[in]    size: Number of bytes to allocate
 * return       Pointer to allocated memory on success, `NULL` otherwise
 * note         This function is thread safe when \ref LWMEM_CFG_OS is enabled
 */
void *
lwmem_malloc_ex(lwmem_t* const lw, const lwmem_region_t* region, const size_t size)
{
    void* ptr;
    LOCK(lw);
    ptr = prv_alloc(lw, region, size);
    UNLOCK(lw);
    return ptr;
}

/*
 * Allocate contiguous block of memory for requested number of items and its size
 * in specific lwmem instance and region.
 *
 * It resets allocated block of memory to zero if allocation is successful
 *
 * This is an extended calloc version function declaration to support advanced features
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    region: Optional region instance within LwMEM instance to force allocation from.
 *                      Set to `NULL` to use any region within LwMEM instance
 * param[in]    nitems: Number of elements to be allocated
 * param[in]    size: Size of each element, in units of bytes
 * return       Pointer to allocated memory on success, `NULL` otherwise
 * note         This function is thread safe when \ref LWMEM_CFG_OS is enabled
 */
void *
lwmem_calloc_ex(lwmem_t* const lw, const lwmem_region_t* region, const size_t nitems, const size_t size)
{
    void* ptr;
    const size_t s = size * nitems;

    LOCK(lw);
    if ((ptr = prv_alloc(lw, region, s)) != NULL)
    {
        LWMEM_MEMSET(ptr, 0x00, s);
    }

    UNLOCK(lw);
    return ptr;
}

/*
 * Reallocates already allocated memory with new size in specific lwmem instance and region.
 *
 * This function may only be used with allocations returned by any of `_from` API functions
 *
 * Function behaves differently, depends on input parameter of `ptr` and `size`:
 *
 *  - `ptr == NULL; size == 0`: Function returns `NULL`, no memory is allocated or freed
 *  - `ptr == NULL; size > 0`: Function tries to allocate new block of memory with `size` length, equivalent to `malloc(region, size)`
 *  - `ptr != NULL; size == 0`: Function frees memory, equivalent to `free(ptr)`
 *  - `ptr != NULL; size > 0`: Function tries to allocate new memory of copy content before returning pointer on success
 *
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    region: Pointer to region to allocate from.
 *                      Set to `NULL` to use any region within LwMEM instance.
 *                      Instance must be the same as used during allocation procedure
 * param[in]    ptr: Memory block previously allocated with one of allocation functions.
 *                   It may be set to `NULL` to create new clean allocation
 * param[in]    size: Size of new memory to reallocate
 * return       Pointer to allocated memory on success, `NULL` otherwise
 * note        This function is thread safe when \ref LWMEM_CFG_OS is enabled
 */
void *
lwmem_realloc_ex(lwmem_t* const lw, const lwmem_region_t* region, void* const ptr, const size_t size)
{
    void* p;
    LOCK(lw);
    p = prv_realloc(lw, region, ptr, size);
    UNLOCK(lw);
    return p;
}

/*
 * Safe version of realloc_ex function.
 *
 * After memory is reallocated, input pointer automatically points to new memory
 * to prevent use of dangling pointers. When reallocation is not successful,
 * original pointer is not modified and application still has control of it.
 *
 * It is advised to use this function when reallocating memory.
 *
 * Function behaves differently, depends on input parameter of `ptr` and `size`:
 *
 *  - `ptr == NULL`: Invalid input, function returns `0`
 *  - `*ptr == NULL; size == 0`: Function returns `0`, no memory is allocated or freed
 *  - `*ptr == NULL; size > 0`: Function tries to allocate new block of memory with `size` length, equivalent to `malloc(size)`
 *  - `*ptr != NULL; size == 0`: Function frees memory, equivalent to `free(ptr)`, sets input pointer pointing to `NULL`
 *  - `*ptr != NULL; size > 0`: Function tries to reallocate existing pointer with new size and copy content to new block
 *
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance
 * param[in]    region: Pointer to region to allocate from.
 *                      Set to `NULL` to use any region within LwMEM instance.
 *                      Instance must be the same as used during allocation procedure
 * param[in]    ptr: Pointer to pointer to allocated memory. Must not be set to `NULL`.
 *                   If reallocation is successful, it modified where pointer points to,
 *                   or sets it to `NULL` in case of `free` operation
 * param[in]    size: New requested size
 * return       `1` if successfully reallocated, `0` otherwise
 * note         This function is thread safe when \ref LWMEM_CFG_OS is enabled
 */
unsigned char
lwmem_realloc_s_ex(lwmem_t* const lw, const lwmem_region_t* region, void** const ptr, const size_t size)
{
    void* new_ptr;

    /*
     * Input pointer must not be NULL otherwise,
     * in case of successful allocation, we have memory leakage
     * aka. allocated memory where noone is pointing to it
     */
    if (ptr == NULL)
{
        return 0;
    }

    new_ptr = lwmem_realloc_ex(lw, region, *ptr, size);     /* Try to reallocate existing pointer */
    if (new_ptr != NULL)
    {
        *ptr = new_ptr;
    }
    else if (size == 0)                                     /* size == 0 means free input memory */
    {
        *ptr = NULL;
        return 1;
    }

    return new_ptr != NULL;
}

/*
 * Free previously allocated memory using one of allocation functions in specific lwmem instance.
 *
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance.
 *                  Instance must be the same as used during allocation procedure
 * note         This is an extended free version function declaration to support advanced features
 * param[in]    ptr: Memory to free. `NULL` pointer is valid input
 * note         This function is thread safe when \ref LWMEM_CFG_OS is enabled
 */
void
lwmem_free_ex(lwmem_t* const lw, void* const ptr)
{
    LOCK(lw);
    prv_free(lw, ptr);
    UNLOCK(lw);
}

/*
 * Safe version of free function
 *
 * After memory is freed, input pointer is safely set to `NULL`
 * to prevent use of dangling pointers.
 *
 * It is advised to use this function when freeing memory.
 *
 * param[in]    lw: LwMEM instance. Set to `NULL` to use default instance.
 *                  Instance must be the same as used during allocation procedure
 * param[in]    ptr: Pointer to pointer to allocated memory.
 *                   When set to non `NULL`, pointer is freed and set to `NULL`
 * note         This function is thread safe when \ref LWMEM_CFG_OS is enabled
 */
void
lwmem_free_s_ex(lwmem_t* const lw, void** const ptr)
{
    if (ptr != NULL && *ptr != NULL)
    {
        LOCK(lw);
        prv_free(lw, *ptr);
        UNLOCK(lw);
        *ptr = NULL;
    }
}

//-------------------------------------------------------------------------------------
// Initialize Application HEAP
//-------------------------------------------------------------------------------------

#include "cpu.h"

static lwmem_region_t m_region;

extern unsigned int get_memory_size(void);

/*
 * 把全部剩余空间用作 heap, 可以改成部分RAM...
 */
void lwmem_initialize(unsigned int size)
{
    extern char end[];              /* end in "ld.script" */
    unsigned int endAddr;

    if (m_region.size > 0)
        return;

#if defined(OS_RTTHREAD)
    lwmem_mutex = rt_mutex_create("LWMEM", RT_IPC_FLAG_FIFO);
#elif defined(OS_UCOS)
    unsigned char err;
    lwmem_mutex = OSMutexCreate(LWMEM_MUTEX_PRIO, &err);
#elif defined(OS_FREERTOS)
    lwmem_mutex = xSemaphoreCreateMutex();
#endif

#if BSP_USE_OS
    if (NULL == lwmem_mutex)
    {
        printk("create memory manager mutex fail.\r\n");
    }
#endif

    endAddr             = ((unsigned int)end + 4) & ~0x03;
    m_region.start_addr = (void *)endAddr;
    m_region.size       = get_memory_size() - K0_TO_PHYS(endAddr);

    lwmem_assignmem(&m_region, 1);
}

#endif // #if (BSP_USE_LWMEM)

/*
 * @@ END
 */
