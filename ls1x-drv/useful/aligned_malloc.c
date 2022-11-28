/*
 * aligned_malloc.c
 *
 * created: 2021/4/4
 *  author: Bian
 */

#if defined(OS_RTTHREAD)

#include "bsp.h"
#include "rtthread.h"

//-------------------------------------------------------------------------------------------------
// aligned malloc
//-------------------------------------------------------------------------------------------------

void *aligned_malloc(size_t size, unsigned int align)
{
    void *head;
    void **addr;

    head = (void*)rt_malloc(size + align - 1 + sizeof(void *));

    if (size == 0 || head == NULL)
        return NULL;

    unsigned int i = (unsigned int)head + sizeof(void *);

    while (i < (unsigned int)head + sizeof(void *) + align - 1)
    {
        if  (i % align == 0)
        {
            addr = (void **)i;
            break;
        }

        i++;
    }

    addr[-1] = head;

    //DBG_OUT("aligned_malloc(%i, %i), return 0x%08X\r\n", size, align, addr);

    return addr;
}

void aligned_free(void *addr)
{
    //DBG_OUT("aligned_free(0x%08X)\r\n", addr);
    
    void *ptr = ((void **)addr)[-1];
    rt_free(ptr);
}

#endif

