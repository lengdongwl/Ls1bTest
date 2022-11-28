/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_gmac.h
 *
 * Created on: 2013/07/01
 *     Author: Bian
 */

#ifndef _LS1X_GMAC_H
#define _LS1X_GMAC_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// GMAC devices
//-----------------------------------------------------------------------------

#ifdef BSP_USE_GMAC0
extern void *devGMAC0;
#endif
#ifdef BSP_USE_GMAC1
extern void *devGMAC1;
#endif

//-----------------------------------------------------------------------------
// GMAC Interrupt Callback function
//-----------------------------------------------------------------------------

typedef int (*net_rx_callback_t)(void);
typedef int (*net_tx_callback_t)(void);

//-----------------------------------------------------------------------------
// Ioctl command
//-----------------------------------------------------------------------------

#define IOCTL_GMAC_START            0x0101      /* start GMAC hardware */
#define IOCTL_GMAC_STOP             0x0102      /* stop GMAC hardware */
#define IOCTL_GMAC_RESET            0x0103      /* reset GMAC hardware */

#define IOCTL_GMAC_SET_MACADDR      0x0105      /* set mac address */
#define IOCTL_GMAC_SET_TIMEOUT      0x0106      /* wait transmit/receive time out */

#define IOCTL_GMAC_IS_RUNNING       0x0107      /* GMAC is running? started or not */
#define IOCTL_GMAC_SHOW_STATS       0x0108

/*
 * TODO GMAC 链路状态
 */

//-----------------------------------------------------------------------------
// GMAC driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_gmac_drv_ops;

#define ls1x_gmac_init(gmac, arg)             ls1x_gmac_drv_ops->init_entry(gmac, arg)
#define ls1x_gmac_read(gmac, buf, size, arg)  ls1x_gmac_drv_ops->read_entry(gmac, buf, size, arg)
#define ls1x_gmac_write(gmac, buf, size, arg) ls1x_gmac_drv_ops->write_entry(gmac, buf, size, arg)
#define ls1x_gmac_ioctl(gmac, cmd, arg)       ls1x_gmac_drv_ops->ioctl_entry(gmac, cmd, arg)

#else

/*
 * GMAC初始化
 * 参数:    dev     devGMAC0/devGMAC1
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int LS1x_GMAC_initialize(void *dev, void *arg);

/*
 * 从GMAC读取接收到的网络数据
 * 参数:    dev     devGMAC0/devGMAC1
 *          buf     类型: char *, 用于存放读取数据的缓冲区
 *          size    类型: int, 待读取的字节数, 长度不能超过 buf 的容量
 *          arg     NULL
 *
 * 返回:    本次读取的字节数
 */
int LS1x_GMAC_read(void *dev, void *buf, int size, void *arg);

/*
 * 向GMAC写入待发送的网络数据
 * 参数:    dev     devGMAC0/devGMAC1
 *          buf     类型: char *, 用于存放待发送数据的缓冲区
 *          size    类型: int, 待发送的字节数, 长度不能超过 buf 的容量
 *          arg     NULL
 *
 * 返回:    本次发送的字节数
 */
int LS1x_GMAC_write(void *dev, void *buf, int size, void *arg);

/*
 * GMAC 控制命令
 * 参数:    dev     devGMAC0/devGMAC1
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                         |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_START            |   NULL, 启动GMAC设备
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_STOP             |   NULL, 停止GMAC设备
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_RESET            |   NULL, 复位GMAC设备
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_SET_TIMEOUT      |   类型: unsigned int
 *                                      |   用途: 设置接收/发送的超时等待时间(ms)
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_IS_RUNNING       |   NULL, GMAC设备是否运行
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_SHOW_STATS       |   NULL, 打印GMAC设备统计信息
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 */
int LS1x_GMAC_ioctl(void *dev, int cmd, void *arg);

#define ls1x_gmac_init(gmac, arg)             LS1x_GMAC_initialize(gmac, arg)
#define ls1x_gmac_read(gmac, buf, size, arg)  LS1x_GMAC_read(gmac, buf, size, arg)
#define ls1x_gmac_write(gmac, buf, size, arg) LS1x_GMAC_write(gmac, buf, size, arg)
#define ls1x_gmac_ioctl(gmac, cmd, arg)       LS1x_GMAC_ioctl(gmac, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// GMAC driver api
//-----------------------------------------------------------------------------

/*
 * 等待GMAC接收到网络数据
 * 参数:    dev     devGMAC0/devGMAC1
 *          bufptr  类型: unsigned char **, 返回GMAC驱动内部接收缓冲区地址
 *
 * 返回:    0=成功
 *
 * 说明:    1. RTOS下调用该函数时, 使用RTOS事件实现无限等待;
 *          2. 裸机下调用该函数时, 等待时间为IOCTL_GMAC_SET_TIMEOUT设置的超时毫秒数
 */
int LS1x_GMAC_wait_receive_packet(void *dev, unsigned char **bufptr);

/*
 * 等待GMAC空闲时发送数据
 * 参数:    dev     devGMAC0/devGMAC1
 *          bufptr  类型: unsigned char **, 返回GMAC驱动内部接收缓冲区地址
 *
 * 返回:    0=成功
 *
 * 说明:    1. RTOS下调用该函数时, 使用RTOS事件实现无限等待;
 *          2. 裸机下调用该函数时, 等待时间为IOCTL_GMAC_SET_TIMEOUT设置的超时毫秒数
 */
int LS1x_GMAC_wait_transmit_idle(void *dev, unsigned char **bufptr);

#define ls1x_gmac_wait_rx_packet(gmac, pbuf)  LS1x_GMAC_wait_receive_packet(gmac, pbuf)
#define ls1x_gmac_wait_tx_idle(gmac, pbuf)    LS1x_GMAC_wait_transmit_idle(gmac, pbuf)

/*
 * for RT-Thread
 */
#if defined(OS_RTTHREAD)
const char *ls1x_gmac_get_device_name(void *pMAC);
#endif

#ifdef __cplusplus
}
#endif

#endif // _LS1X_GMAC_H

