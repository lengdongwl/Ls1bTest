/*
 * aligned_malloc.c
 */

#include <stdlib.h>

void *aligned_malloc(size_t size, unsigned int align)
{
    void *head;
    void **addr;

    head = (void*)malloc(size + align - 1 + sizeof(void *));

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

    return addr;
}
