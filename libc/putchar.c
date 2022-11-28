/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "console.h"

int putchar(int ch)
{
    console_putch(ch);
    return 0;
}

int _putchar(int ch)
{
    console_putch(ch);
    return 0;
}

int puts(const char *s)
{
    int count = 0;
    while (*s)
    {
        putchar(*s++);
        count++;
    }
    return count;
}


