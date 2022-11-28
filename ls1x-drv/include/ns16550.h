/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _NS16550_H_
#define _NS16550_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "termios.h"

#include "bsp.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define CFLAG_TO_BAUDRATE(flag)      \
        ((flag == B1200)   ? 1200 :  \
         (flag == B2400)   ? 2400 :  \
         (flag == B4800)   ? 4800 :  \
         (flag == B9600)   ? 9600 :  \
         (flag == B19200)  ? 19200 : \
         (flag == B38400)  ? 38400 : \
         (flag == B57600)  ? 57600 : \
         (flag == B115200) ? 115200 : 9600)

#define BAUDRATE_TO_CFLAG(baud)      \
        ((baud == 1200)   ? B1200 :  \
         (baud == 2400)   ? B2400 :  \
         (baud == 4800)   ? B4800 :  \
         (baud == 9600)   ? B9600 :  \
         (baud == 19200)  ? B19200 : \
         (baud == 38400)  ? B38400 : \
         (baud == 57600)  ? B57600 : \
         (baud == 115200) ? B115200 : B9600)

//-----------------------------------------------------------------------------
// UART devices
//-----------------------------------------------------------------------------

#if defined(LS1B)

#ifdef BSP_USE_UART2
extern void *devUART2;
#endif
#ifdef BSP_USE_UART3
extern void *devUART3;
#endif
#ifdef BSP_USE_UART4
extern void *devUART4;
#endif
#ifdef BSP_USE_UART5
extern void *devUART5;
#endif
#ifdef BSP_USE_UART0
extern void *devUART0;
#endif
#ifdef BSP_USE_UART01
extern void *devUART01;
#endif
#ifdef BSP_USE_UART02
extern void *devUART02;
#endif
#ifdef BSP_USE_UART03
extern void *devUART03;
#endif
#ifdef BSP_USE_UART1
extern void *devUART1;
#endif
#ifdef BSP_USE_UART11
extern void *devUART11;
#endif
#ifdef BSP_USE_UART12
extern void *devUART12;
#endif
#ifdef BSP_USE_UART13
extern void *devUART13;
#endif

#elif defined(LS1C)

#ifdef BSP_USE_UART0
extern void *devUART0;
#endif
#ifdef BSP_USE_UART1
extern void *devUART1;
#endif
#ifdef BSP_USE_UART2
extern void *devUART2;
#endif
#ifdef BSP_USE_UART3
extern void *devUART3;
#endif
#ifdef BSP_USE_UART4
extern void *devUART4;
#endif
#ifdef BSP_USE_UART5
extern void *devUART5;
#endif
#ifdef BSP_USE_UART6
extern void *devUART6;
#endif
#ifdef BSP_USE_UART7
extern void *devUART7;
#endif
#ifdef BSP_USE_UART8
extern void *devUART8;
#endif
#ifdef BSP_USE_UART9
extern void *devUART9;
#endif
#ifdef BSP_USE_UART10
extern void *devUART10;
#endif
#ifdef BSP_USE_UART11
extern void *devUART11;
#endif

#endif

//-----------------------------------------------------------------------------
// NS16550 io control command                   param type
//-----------------------------------------------------------------------------

#define IOCTL_NS16550_SET_MODE      0x1000      // arg: struct termios *

#define IOCTL_NS16550_USE_IRQ       0x1001      // arg: int *

//-----------------------------------------------------------------------------
// NS16550 driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_uart_drv_ops;

#define ls1x_uart_init(uart, arg)             ls1x_uart_drv_ops->init_entry(uart, arg)
#define ls1x_uart_open(uart, arg)             ls1x_uart_drv_ops->open_entry(uart, arg)
#define ls1x_uart_close(uart, arg)            ls1x_uart_drv_ops->close_entry(uart, arg)
#define ls1x_uart_read(uart, buf, size, arg)  ls1x_uart_drv_ops->read_entry(uart, buf, size, arg)
#define ls1x_uart_write(uart, buf, size, arg) ls1x_uart_drv_ops->write_entry(uart, buf, size, arg)
#define ls1x_uart_ioctl(uart, cmd, arg)       ls1x_uart_drv_ops->ioctl_entry(uart, cmd, arg)

#else

/*
 * 初始化串口
 * 参数:    dev     见上面定义的 UART 设备
 *          arg     类型 unsigned int, 串口波特率. 当该参数为 0 或 NULL时, 串口设置为默认模式 115200,8N1
 *
 * 返回:    0=成功
 */
int NS16550_init(void *dev, void *arg);

/*
 * 打开串口. 如果串口配置为中断模式, 安装中断向量
 * 参数:    dev     见上面定义的 UART 设备
 *          arg     类型 struct termios *, 把串口配置为指定参数模式. 该参数可为 0 或 NULL.
 *
 * 返回:    0=成功
 */
int NS16550_open(void *dev, void *arg);

/*
 * 关闭串口. 如果串口配置为中断模式, 移除中断向量
 * 参数:    dev     见上面定义的 UART 设备
 *          arg     总是 0 或 NULL.
 *
 * 返回:    0=成功
 */
int NS16550_close(void *dev, void *arg);

/*
 * 从串口读数据(接收)
 * 参数:    dev     见上面定义的 UART 设备
 *          buf     类型 char *, 用于存放读取数据的缓冲区
 *          size    类型 int, 待读取的字节数, 长度不能超过 buf 的容量
 *          arg     类型 int.
 *                  如果串口工作在中断模式:
 *                    >0:   该值用作读操作的超时等待毫秒数
 *                    =0:   读操作立即返回
 *                  如果串口工作在查询模式:
 *                    !=0:  读操作工作在阻塞模式, 直到读取size个字节才返回
 *                    =0:   读操作立即返回
 *
 * 返回:    读取的字节数
 *
 * 说明:    串口工作在中断模式: 读操作总是读的内部数据接收缓冲区
 *          串口工作在查询模式: 读操作直接对串口设备进行读
 */
int NS16550_read(void *dev, void *buf, int size, void *arg);

/*
 * 向串口写数据(发送)
 * 参数:    dev     见上面定义的 UART 设备
 *          buf     类型 char *, 用于存放待发送数据的缓冲区
 *          size    类型 int, 待发送的字节数, 长度不超过 buf 的容量
 *          arg     总是 0 或 NULL
 *
 * 返回:    发送的字节数
 *
 * 说明:    串口工作在中断模式: 写操作总是写的内部数据发送缓冲区
 *          串口工作在查询模式: 写操作直接对串口设备进行写
 */
int NS16550_write(void *dev, void *buf, int size, void *arg);

/*
 * 向串口发送控制命令
 * 参数:    dev         见上面定义的 UART 设备
 *          cmd         IOCTL_NS16550_SET_MODE
 *          arg         类型 struct termios *, 把串口配置为指定参数模式.
 *
 * 返回:    0=成功
 */
int NS16550_ioctl(void *dev, int cmd, void *arg);

#define ls1x_uart_init(uart, arg)             NS16550_init(uart, arg)
#define ls1x_uart_open(uart, arg)             NS16550_open(uart, arg)
#define ls1x_uart_close(uart, arg)            NS16550_close(uart, arg)
#define ls1x_uart_read(uart, buf, size, arg)  NS16550_read(uart, buf, size, arg)
#define ls1x_uart_write(uart, buf, size, arg) NS16550_write(uart, buf, size, arg)
#define ls1x_uart_ioctl(uart, cmd, arg)       NS16550_ioctl(uart, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// Console Support
//-----------------------------------------------------------------------------

/*
 * 用作控制台的串口
 */
#if defined(LS1B)
#define ConsolePort devUART5
#elif defined(LS1C)
#define ConsolePort devUART2
#endif

/*
 * 从控制台串口读一个字符
 */
char Console_get_char(void *pUART);

/*
 * 向控制台串口写一个字符
 */
void Console_output_char(void *pUART, char ch);

/*
 * for RT-Thread
 */
const char *ls1x_uart_get_device_name(void *pUART);

#ifdef __cplusplus
}
#endif

#endif /* _NS16550_H_ */

