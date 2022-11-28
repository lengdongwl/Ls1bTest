/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bsp.h"

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_gpio.h"
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_gpio.h"
#else
#error "No Loongson1x SoC defined."
#endif

#include "console.h"
#include "ns16550.h"

#include "termios.h"

void console_init(unsigned int baudrate)
{
    struct termios t;
    
#if defined(LS1B)
    //
#elif defined(LS1C)
    /*
     * ConsolePort == UART2 ?
     */
    gpio_set_pinmux(36, 2);         // Pin: 149
    gpio_set_pinmux(37, 2);         // Pin: 150
#endif

    ls1x_uart_init(ConsolePort, (void *)baudrate);
    
    t.c_cflag = BAUDRATE_TO_CFLAG(baudrate) | CS8;  // 8N1
    ls1x_uart_open(ConsolePort, &t);
}

char console_getch(void)
{
   return Console_get_char(ConsolePort);
}

void console_putch(char ch)
{
    if (ch == '\n')
        Console_output_char(ConsolePort, '\r');
    Console_output_char(ConsolePort, ch);
}

void console_putstr(char *s)
{
    while (*s)
    {
    	console_putch(*s++);
    }
}

#ifdef OS_RTTHREAD

#include "rtconfig.h"

#if defined(RT_USING_CONSOLE) && !defined(RT_USING_SERIAL)

void rt_hw_console_output(const char *str)
{
    console_putstr((char *)str);
}

char rt_hw_console_getchar(void)
{
    return console_getch();
}

#endif

#endif


