/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * xpt2046.h
 *
 *  Created on: 2014-9-1
 *      Author: Bian
 */

#ifndef _XPT2046_H_
#define _XPT2046_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * TC-1B200开发板, 触摸中断信号通过 GPIO54(UART2_RX实现).
 */
#if defined(LS1B)
#if BSP_USE_OS
#define XPT2046_USE_GPIO_INT    1           /* touchscreen use interrupt */
#endif
#define XPT2046_USE_GPIO_NUM    54          /* interrupt gpio port */
#endif // #if BSP_USE_OS

#define TOUCHSCREEN_USE_MESSAGE 1

typedef struct touch_msg
{
    unsigned short x;           /* X坐标值 */
    unsigned short y;           /* Y坐标值  */
    unsigned short z;           /* Z方向的压力值 */
} ts_message_t;

/******************************************************************************
 * SPI0-XPT2046 driver operators
 */

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_xpt2046_drv_ops;

#define ls1x_xpt2046_init(spi, arg)             ls1x_xpt2046_drv_ops->init_entry(spi, arg)
#define ls1x_xpt2046_read(spi, buf, size, arg)  ls1x_xpt2046_drv_ops->read_entry(spi, buf, size, arg)

#else

/*
 * 初始化XPT2046芯片
 * 参数:    dev     busSPI0
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int XPT2046_initialize(void *bus, void *arg);

/*
 * 从XPT2046芯片读数据(触摸点)
 * 参数:    dev     busSPI0
 *          buf     类型: ts_message_t *, 数组, 用于存放读取数据的缓冲区
 *          size    类型: int, 待读取的字节数, 长度不能超过 buf 的容量, sizeof(ts_message_t)倍数
 *          arg     NULL
 *
 * 返回:    读取的字节数
 */
int XPT2046_read(void *bus, void *buf, int size, void *arg);

#define ls1x_xpt2046_init(spi, arg)             XPT2046_initialize(spi, arg)
#define ls1x_xpt2046_read(spi, buf, size, arg)  XPT2046_read(spi, buf, size, arg)

#endif

/******************************************************************************
 * user api in "touch_utils.c"
 */
/*
 * 触摸事件回调函数
 * 参数:    x, y    触摸坐标
 */
typedef void (*touch_callback_t)(int x, int y);

/*
 * 触摸屏校正函数
 */
extern int do_touchscreen_calibrate(void);

/*
 * 在RTOS编程下, 启动触摸屏任务
 * 参数:    cb      类型: touch_callback_t, 发生触摸事件时该回调函数供用户处理
 */
extern int start_touchscreen_task(touch_callback_t cb);
/*
 * 在RTOS编程下, 停止触摸屏任务
 */
extern int stop_touchscreen_task(void);

/*
 * 在裸机编程下, 获取发生触摸事件的坐标
 */
extern int bare_get_touch_point(int *x, int *y);

/*
 * touch screen calibrate file name. XXX 长文件名不能使用
 */
#define TOUCH_CALIBRATE_DIR        "/ndd/config"
#define TOUCH_CALIBRATE_INIFILE    "/ndd/config/touchcal.ini"
#define TOUCH_CALIBRATE_DATFILE    "/ndd/config/touchcal.dat"

#ifdef __cplusplus
}
#endif

#endif /* _XPT2046_H_ */


