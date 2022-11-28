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
 * RT-Thread ���ȼ�˵����
 *
 *     1.RT_THREAD_PRIORITY_MAX == 32
 *     2.����ͬ���ȼ���ʱ��Ƭ
 *     3.��ֵԽС�����ȼ�Խ��
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
 * UCOSII ���ȼ�˵����
 *
 *     1.OS_LOWEST_PRIO == 127
 *       32~63:  ��������ʹ��
 *       64~127: Ӧ�ó���ʹ��
 *     2.������ͬ���ȼ���ʱ��Ƭ
 *     3.��ֵԽС�����ȼ�Խ��
 */
#define CAN0_TASK_PRIO      64

#define CAN1_TASK_PRIO      65

#define I2C0_TASK_PRIO      66

#define GUI_MUTEX_PRIO      71
#define GUI_TASK_PRIO       72

#elif defined(OS_FREERTOS)
/*
 * FreeRTOS ���ȼ�˵����
 *
 *     1.CONFIG_MAX_PRIORITIES == 31
 *     2.����ͬ���ȼ���ʱ��Ƭ
 *     3.��ֵԽ�����ȼ�Խ��
 */
#define CAN0_TASK_PRIO      25

#define CAN1_TASK_PRIO      26

#define I2C0_TASK_PRIO      24

#define GUI_TASK_PRIO       10

#endif

#endif // BSP_USE_OS

#endif // _APP_OS_PRIORITY_H

