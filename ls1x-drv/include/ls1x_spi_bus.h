/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_spi.h
 *
 * this file contains the ls1x SPI driver declarations
 *
 *  Created on: 2013-11-1
 *      Author: Bian
 */

#ifndef _LS1x_SPI_H
#define _LS1x_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ls1x_io.h"

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#define LS1x_SPI0_BASE      LS1B_SPI0_BASE
#define LS1x_SPI1_BASE      LS1B_SPI1_BASE
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#define LS1x_SPI0_BASE      LS1C_SPI0_BASE
#define LS1x_SPI1_BASE      LS1C_SPI1_BASE
#else
#error "No Loongson1x SoC defined."
#endif

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#elif defined(OS_UCOS)
#include "os.h"
#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "semphr.h"
#endif

#include "bsp.h"

/******************************************************************************
 * LS1x SPI Definations
 ******************************************************************************/

typedef struct LS1x_SPI_regs;

/* 
 * SPI bus
 */
typedef struct
{
    struct LS1x_SPI_regs *hwSPI;    /* pointer to HW registers */
    /* hardware parameters */
    unsigned int base_frq;          /* input frq for baud rate divider */
    unsigned int chipsel_nums;      /* total chip select numbers */
    unsigned int chipsel_high;      /* cs high level - XXX: value from tfr-mode */
    unsigned int dummy_char;        /* this character will be continuously transmitted in read only functions */
    /* interrupt support*/
    unsigned int irqNum;            /* interrupt num */
    unsigned int int_ctrlr;         /* interrupt controller */
    unsigned int int_mask;          /* interrupt mask */
    /* mutex */
#if defined(OS_RTTHREAD)
    rt_mutex_t spi_mutex;
#elif defined(OS_UCOS)
    OS_EVENT  *spi_mutex;
#elif defined(OS_FREERTOS)
    SemaphoreHandle_t spi_mutex;
#else // defined(OS_NONE)
    int  spi_mutex;
#endif
    int  initialized;
    char dev_name[16];
#if (PACK_DRV_OPS)
    libspi_ops_t *ops;              /* bus operators */
#endif
} LS1x_SPI_bus_t;

/* 
 * SPI device communication mode 
 */
typedef struct
{
    unsigned int  baudrate;         /* maximum bits per second */
    unsigned char bits_per_char;    /* how many bits per byte/word/longword? */
    bool          lsb_first;        /* true: send LSB first */
    bool          clock_pha;        /* clock phase    - spi mode */
    bool          clock_pol;        /* clock polarity - spi mode */
    bool          clock_inv;        /* true: inverted clock (low active) - cs high or low */
    bool          clock_phs;        /* true: clock starts toggling at start of data tfr - interface mode */
} LS1x_SPI_mode_t;

/******************************************************************************
 * LS1x SPI BUS
 */
#ifdef BSP_USE_SPI0
extern LS1x_SPI_bus_t *busSPI0;
#endif
#ifdef BSP_USE_SPI1
extern LS1x_SPI_bus_t *busSPI1;
#endif

/******************************************************************************
 * bus operators
 */
#if (PACK_DRV_OPS)

#define ls1x_spi_initialize(spi)            spi->ops->init(spi)
#define ls1x_spi_send_start(spi, addr)      spi->ops->send_start(spi, addr)
#define ls1x_spi_send_stop(spi, addr)       spi->ops->send_stop(spi, addr)
#define ls1x_spi_send_addr(spi, addr, rw)   spi->ops->send_addr(spi, addr, rw)
#define ls1x_spi_read_bytes(spi, buf, len)  spi->ops->read_bytes(spi, buf, len)
#define ls1x_spi_write_bytes(spi, buf, len) spi->ops->write_bytes(spi, buf, len)
#define ls1x_spi_ioctl(spi, cmd, arg)       spi->ops->ioctl(spi, cmd, arg)

#else

/*
 * 初始化SPI总线
 * 参数:    bus     busSPI0 或者 busSPI1
 *
 * 返回:    0=成功
 *
 * 说明:    SPI总线在使用前, 必须要先调用该初始化函数
 */
int LS1x_SPI_initialize(void *bus);

/*
 * 开始SPI总线操作. 本函数获取SPI总线的控制权
 * 参数:    bus     busSPI0 或者 busSPI1
 *          Addr    片选. 取值范围0~3, 表示将操作SPI总线上挂接的某个从设备
 *
 * 返回:    0=成功
 */
int LS1x_SPI_send_start(void *bus, unsigned int Addr);

/*
 * 结束SPI总线操作. 本函数释放SPI总线的控制权
 * 参数:    bus     busSPI0 或者 busSPI1
 *          Addr    片选. 取值范围0~3, 表示将操作SPI总线上挂接的某个从设备
 *
 * 返回:    0=成功
 */
int LS1x_SPI_send_stop(void *bus, unsigned int Addr);

/*
 * 读写SPI总线前发送片选信号
 * 参数:    bus     busSPI0 或者 busSPI1
 *          Addr    片选. 取值范围0~3, 表示将操作SPI总线上挂接的某个从设备
 *          rw      未使用
 *
 * 返回:    0=成功
 */
int LS1x_SPI_send_addr(void *bus, unsigned int Addr, int rw);

/*
 * 从SPI从设备读取数据
 * 参数:    bus     busSPI0 或者 busSPI1
 *          buf     类型 unsigned char *, 用于存放读取数据的缓冲区
 *          len     类型 int, 待读取的字节数, 长度不能超过 buf 的容量
 *
 * 返回:    本次读操作的字节数
 */
int LS1x_SPI_read_bytes(void *bus, unsigned char *buf, int len);

/*
 * 向SPI从设备写入数据
 * 参数:    bus     busSPI0 或者 busSPI1
 *          buf     类型 unsigned char *, 用于存放待写数据的缓冲区
 *          len     类型 int, 待写的字节数, 长度不能超过 buf 的容量
 *
 * 返回:    本次写操作的字节数
 */
int LS1x_SPI_write_bytes(void *bus, unsigned char *buf, int len);

/*
 * 向SPI总线发送控制命令
 * 参数:    bus     busSPI0 或者 busSPI1
 *      ---------------------------------------------------------------------------------
 *          cmd                             |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_SPI_I2C_SET_TFRMODE       |   类型: LS1x_SPI_mode_t *
 *                                          |   用途: 设置SPI总线的通信模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_FAST_READ_ENABLE    |   NULL, 设置SPI控制器为 Flash快速读模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_FAST_READ_DISABLE   |   NULL, 取消SPI控制器的 Flash快速读模式
 *      ---------------------------------------------------------------------------------
 *          IOCTL_FLASH_GET_FAST_READ_MODE  |   类型: unsigned int *
 *                                          |   用途: 读取SPI控制器是否处于 Flash快速读模式
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 *
 * 说明:    该函数调用的时机是: SPI设备已经初始化且空闲, 或者已经获取总线控制权
 */
int LS1x_SPI_ioctl(void *bus, int cmd, void *arg);

#define ls1x_spi_initialize(spi)            LS1x_SPI_initialize(spi)
#define ls1x_spi_send_start(spi, addr)      LS1x_SPI_send_start(spi, addr)
#define ls1x_spi_send_stop(spi, addr)       LS1x_SPI_send_stop(spi, addr)
#define ls1x_spi_send_addr(spi, addr, rw)   LS1x_SPI_send_addr(spi, addr, rw)
#define ls1x_spi_read_bytes(spi, buf, len)  LS1x_SPI_read_bytes(spi, buf, len)
#define ls1x_spi_write_bytes(spi, buf, len) LS1x_SPI_write_bytes(spi, buf, len)
#define ls1x_spi_ioctl(spi, cmd, arg)       LS1x_SPI_ioctl(spi, cmd, arg)

#endif

/******************************************************************************
 * bus api
 */

/*
 * 设置SPI控制器为 Flash快速读模式
 * 参数:    bus     busSPI0 或者 busSPI1
 */
int LS1x_enable_spiflash_fastread(LS1x_SPI_bus_t *pSPI);

/*
 * 取消SPI控制器的 Flash快速读模式
 * 参数:    bus     busSPI0 或者 busSPI1
 */
int LS1x_disable_spiflash_fastread(LS1x_SPI_bus_t *pSPI);

#ifdef __cplusplus
}
#endif

#endif /* _LS1x_I2C_H */
