/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_pwm_hw.h
 *
 * Created on: 2013-3-12
 *     Author: Bian
 */

#ifndef _LS1X_PWM_HW_H
#define _LS1X_PWM_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PWM hardware Registers.
 */
typedef struct LS1x_PWM_regs
{
	volatile unsigned int counter;      /* 0x00 主计数器 */
	volatile unsigned int hrc;          /* 0x04 高脉冲定时参考寄存器 */
	volatile unsigned int lrc;          /* 0x08 低脉冲定时参考寄存器 */
	volatile unsigned int ctrl;         /* 0x0C 控制寄存器 */
} LS1x_PWM_regs_t;

/******************************************************************************
 * 1. 主计数器
 * 2. 高脉冲定时参考寄存器
 * 3. 低脉冲定时参考寄存器
 */
#if defined(LS1B)
#define PWM_REG_VALUE_MAX       0x007FFFFF      // XXX 实测值 0x003FF100
#elif defined(LS1C)
#define PWM_REG_VALUE_MAX       0x007FFFFF      // TODO
#endif

/******************************************************************************
 * 控制寄存器
 */
enum
{
#if defined(LS1C)
    //-----------------------------------------------------
    // TODO 在 LS1B 的数据手册中, 未见这两个说明.
    //      LS1B/LS1C 寄存器宽度 =8 明显是错误的.
    //-----------------------------------------------------
    pwm_ctrl_lrc_ien = (1<<11),     /* =1: 当INTEN为1时，CNTR计数到LRC后产生中断 */
    pwm_ctrl_hrc_ien = (1<<10),     /* =1: 当INTEN为1时，CNTR计数到HRC后产生中断 */
#endif

	pwm_ctrl_rest    = (1<<7),      /* =1: counter计数器清零, =0: 计数器正常工作 */
	pwm_ctrl_iflag   = (1<<6),      /* =1: 有中断产生. 写入 1: 清中断 */
	pwm_ctrl_ien     = (1<<5),      /* =1: 当counter计数到lrc或hrc后产生中断 */
	pwm_ctrl_single  = (1<<4),      /* =1: 脉冲仅产生一次, =0: 脉冲持续产生 */
	pwm_ctrl_oe_mask = (1<<3),      /* =1: 脉冲输出屏蔽, =0: 脉冲输出使能 */
	pwm_ctrl_cntr_en = (1<<0),      /* =1: counter使能计数, =0: counter停止计数 */
};

#ifdef __cplusplus
}
#endif

#endif // _LS1X_PWM_HW_H


