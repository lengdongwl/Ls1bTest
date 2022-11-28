/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * drv_os_priority.h
 *
 * created: 2021/1/2
 *  author: Bian
 *
 */

#ifndef _DRV_OS_PRIORITY_H
#define _DRV_OS_PRIORITY_H

#if BSP_USE_OS

#define CAN_RX_EVENT        0x0001      /* CAN 接收事件 */
#define CAN_TX_EVENT        0x0002      /* CAN 发送事件 */

#define GMAC_RX_EVENT       0x0004      /* GMAC 接收事件 */
#define GMAC_TX_EVENT       0x0008      /* GMAC 发送事件 */

#define PWM_TIMER_EVENT     0x0010      /* PWM Timer 事件 */
#define RTC_TIMER_EVENT     0x0020      /* RTC Timer 事件 */

#define AC97_DMA1_EVENT     0x0040      /* AC97 DMA1 tx 事件 */
#define AC97_DMA2_EVENT     0x0080      /* AC97 DMA2 rx 事件 */

#define TOUCH_CLICK_EVENT   0x0100      /* 触摸事件 */

/*
 * Stack size
 */
#define TOUCH_STK_SIZE     (1024*2)

#define AC97_STK_SIZE      (1024*2)     // AC97 use DMA1/DMA2

//-----------------------------------------------------------------------------

#if defined(OS_RTTHREAD)
/*
 * RT-Thread 优先级说明：
 *
 *     1.RT_THREAD_PRIORITY_MAX == 32
 *     2.允许同优先级、时间片
 *     3.数值越小，优先级越高
 */
#define TOUCH_TASK_PRIO     15
#define TOUCH_TASK_SLICE    10

#define AC97_TASK_PRIO       6          // AC97 use DMA1/DMA2
#define AC97_TASK_SLICE     10

//-----------------------------------------------------------------------------

#elif defined(OS_UCOS)
/*
 * UCOSII 优先级说明：
 *
 *     1.OS_LOWEST_PRIO == 127
 *       10~63:  驱动程序使用
 *       64~127: 应用程序使用
 *     2.不允许同优先级、时间片
 *     3.数值越小，优先级越高
 */
#define DMA0_MUTEX_PRIO     10          // NAND use DMA0
#define AC97_TASK_PRIO      11          // AC97 use DMA1/DMA2

#define SPI0_MUTEX_PRIO     23
#define SPI1_MUTEX_PRIO     24
#define I2C0_MUTEX_PRIO     25
#define I2C1_MUTEX_PRIO     26
#define I2C2_MUTEX_PRIO     27

#define FB_MUTEX_PRIO       61
#define TOUCH_TASK_PRIO     62

//-----------------------------------------------------------------------------

#elif defined(OS_FREERTOS)
/*
 * FreeRTOS 优先级说明：
 *
 *     1.CONFIG_MAX_PRIORITIES == 31
 *     2.允许同优先级、时间片
 *     3.数值越大，优先级越高
 */
#define TOUCH_TASK_PRIO     11
#define AC97_TASK_PRIO      21          // AC97 use DMA1/DMA2

#endif

#endif // #if BSP_USE_OS

#endif // _DRV_OS_PRIORITY_H

