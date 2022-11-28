/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _APP_OS_PRIORITY_H
#define _APP_OS_PRIORITY_H

#if BSP_USE_OS

#define CAN0_STK_SIZE      (1024*1)
#define CAN1_STK_SIZE      (1024*1)
#define I2C0_STK_SIZE      (1024*1)
#define GUI_STK_SIZE       (1024*2)

#if defined(OS_RTTHREAD)
/*
 * RT-Thread 优先级说明：
 *
 *     1.RT_THREAD_PRIORITY_MAX == 32
 *     2.允许同优先级、时间片
 *     3.数值越小，优先级越高
 */
#define CAN0_TASK_PRIO      17
#define CAN0_TASK_SLICE     10

#define CAN1_TASK_PRIO      17
#define CAN1_TASK_SLICE     10

#define I2C0_TASK_PRIO      18
#define I2C0_TASK_SLICE     10

#define GUI_TASK_PRIO       19
#define GUI_TASK_SLICE      20

#elif defined(OS_UCOS)
/*
 * UCOSII 优先级说明：
 *
 *     1.OS_LOWEST_PRIO == 127
 *       32~63:  驱动程序使用
 *       64~127: 应用程序使用
 *     2.不允许同优先级、时间片
 *     3.数值越小，优先级越高
 */
#define CAN0_TASK_PRIO      64

#define CAN1_TASK_PRIO      65

#define I2C0_TASK_PRIO      66

#define GUI_MUTEX_PRIO      71
#define GUI_TASK_PRIO       72

#elif defined(OS_FREERTOS)
/*
 * FreeRTOS 优先级说明：
 *
 *     1.CONFIG_MAX_PRIORITIES == 31
 *     2.允许同优先级、时间片
 *     3.数值越大，优先级越高
 */
#define CAN0_TASK_PRIO      25

#define CAN1_TASK_PRIO      26

#define I2C0_TASK_PRIO      24

#define GUI_TASK_PRIO       10

#endif

#endif // BSP_USE_OS

#endif // _APP_OS_PRIORITY_H

