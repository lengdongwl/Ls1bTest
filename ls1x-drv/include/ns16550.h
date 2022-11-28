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
 * ��ʼ������
 * ����:    dev     �����涨��� UART �豸
 *          arg     ���� unsigned int, ���ڲ�����. ���ò���Ϊ 0 �� NULLʱ, ��������ΪĬ��ģʽ 115200,8N1
 *
 * ����:    0=�ɹ�
 */
int NS16550_init(void *dev, void *arg);

/*
 * �򿪴���. �����������Ϊ�ж�ģʽ, ��װ�ж�����
 * ����:    dev     �����涨��� UART �豸
 *          arg     ���� struct termios *, �Ѵ�������Ϊָ������ģʽ. �ò�����Ϊ 0 �� NULL.
 *
 * ����:    0=�ɹ�
 */
int NS16550_open(void *dev, void *arg);

/*
 * �رմ���. �����������Ϊ�ж�ģʽ, �Ƴ��ж�����
 * ����:    dev     �����涨��� UART �豸
 *          arg     ���� 0 �� NULL.
 *
 * ����:    0=�ɹ�
 */
int NS16550_close(void *dev, void *arg);

/*
 * �Ӵ��ڶ�����(����)
 * ����:    dev     �����涨��� UART �豸
 *          buf     ���� char *, ���ڴ�Ŷ�ȡ���ݵĻ�����
 *          size    ���� int, ����ȡ���ֽ���, ���Ȳ��ܳ��� buf ������
 *          arg     ���� int.
 *                  ������ڹ������ж�ģʽ:
 *                    >0:   ��ֵ�����������ĳ�ʱ�ȴ�������
 *                    =0:   ��������������
 *                  ������ڹ����ڲ�ѯģʽ:
 *                    !=0:  ����������������ģʽ, ֱ����ȡsize���ֽڲŷ���
 *                    =0:   ��������������
 *
 * ����:    ��ȡ���ֽ���
 *
 * ˵��:    ���ڹ������ж�ģʽ: ���������Ƕ����ڲ����ݽ��ջ�����
 *          ���ڹ����ڲ�ѯģʽ: ������ֱ�ӶԴ����豸���ж�
 */
int NS16550_read(void *dev, void *buf, int size, void *arg);

/*
 * �򴮿�д����(����)
 * ����:    dev     �����涨��� UART �豸
 *          buf     ���� char *, ���ڴ�Ŵ��������ݵĻ�����
 *          size    ���� int, �����͵��ֽ���, ���Ȳ����� buf ������
 *          arg     ���� 0 �� NULL
 *
 * ����:    ���͵��ֽ���
 *
 * ˵��:    ���ڹ������ж�ģʽ: д��������д���ڲ����ݷ��ͻ�����
 *          ���ڹ����ڲ�ѯģʽ: д����ֱ�ӶԴ����豸����д
 */
int NS16550_write(void *dev, void *buf, int size, void *arg);

/*
 * �򴮿ڷ��Ϳ�������
 * ����:    dev         �����涨��� UART �豸
 *          cmd         IOCTL_NS16550_SET_MODE
 *          arg         ���� struct termios *, �Ѵ�������Ϊָ������ģʽ.
 *
 * ����:    0=�ɹ�
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
 * ��������̨�Ĵ���
 */
#if defined(LS1B)
#define ConsolePort devUART5
#elif defined(LS1C)
#define ConsolePort devUART2
#endif

/*
 * �ӿ���̨���ڶ�һ���ַ�
 */
char Console_get_char(void *pUART);

/*
 * �����̨����дһ���ַ�
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

